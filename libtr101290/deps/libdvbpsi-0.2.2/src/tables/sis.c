/*****************************************************************************
 * sis.c: SIS decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2010 VideoLAN
 * $Id:$
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "../descriptor.h"
#include "../demux.h"

#include "sis.h"
#include "sis_private.h"

/*****************************************************************************
 * dvbpsi_AttachSIS
 *****************************************************************************
 * Initialize a SIS subtable decoder.
 *****************************************************************************/
int dvbpsi_AttachSIS(dvbpsi_decoder_t *p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sis_callback pf_callback,
                               void* p_cb_data)
{
    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
    dvbpsi_demux_subdec_t* p_subdec;
    dvbpsi_sis_decoder_t*  p_sis_decoder;

    i_extension = 0;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        DVBPSI_ERROR_ARG("SIS decoder",
                         "Already a decoder for (table_id == 0x%02x,"
                         "extension == 0x%02x)",
                         i_table_id, i_extension);

        return 1;
    }

    p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
    if (p_subdec == NULL)
    {
        return 1;
    }

    p_sis_decoder = (dvbpsi_sis_decoder_t*)malloc(sizeof(dvbpsi_sis_decoder_t));
    if (p_sis_decoder == NULL)
    {
        free(p_subdec);
        return 1;
    }

    /* subtable decoder configuration */
    p_subdec->pf_callback = &dvbpsi_GatherSISSections;
    p_subdec->p_cb_data = p_sis_decoder;
    p_subdec->i_id = (uint32_t)i_table_id << 16 | (uint32_t)i_extension;
    p_subdec->pf_detach = dvbpsi_DetachSIS;

    /* Attach the subtable decoder to the demux */
    p_subdec->p_next = p_demux->p_first_subdec;
    p_demux->p_first_subdec = p_subdec;

    /* SIS decoder information */
    p_sis_decoder->pf_callback = pf_callback;
    p_sis_decoder->p_cb_data = p_cb_data;

    return 0;
}

/*****************************************************************************
 * dvbpsi_DetachSIS
 *****************************************************************************
 * Close a SIS decoder.
 *****************************************************************************/
void dvbpsi_DetachSIS(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
{
    dvbpsi_demux_subdec_t* p_subdec;
    dvbpsi_demux_subdec_t** pp_prev_subdec;
    dvbpsi_sis_decoder_t* p_sis_decoder;

    i_extension = 0;

    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_demux == NULL)
    {
        DVBPSI_ERROR_ARG("SIS Decoder",
                         "No such SIS decoder (table_id == 0x%02x,"
                         "extension == 0x%02x)",
                         i_table_id, i_extension);
        return;
    }

    p_sis_decoder = (dvbpsi_sis_decoder_t*)p_subdec->p_cb_data;

    free(p_subdec->p_cb_data);

    pp_prev_subdec = &p_demux->p_first_subdec;
    while(*pp_prev_subdec != p_subdec)
        pp_prev_subdec = &(*pp_prev_subdec)->p_next;

    *pp_prev_subdec = p_subdec->p_next;
    free(p_subdec);
}

/*****************************************************************************
 * dvbpsi_InitSIS
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_sis_t structure.
 *****************************************************************************/
void dvbpsi_InitSIS(dvbpsi_sis_t *p_sis, uint8_t i_protocol_version)
{
    assert(i_protocol_version == 0);
    p_sis->i_protocol_version = 0; /* must be 0 */

    /* encryption */
    p_sis->b_encrypted_packet = 0;
    p_sis->i_encryption_algorithm = 0;

    p_sis->i_pts_adjustment = (uint64_t)0;
    p_sis->cw_index = 0;

    /* splice command */
    p_sis->i_splice_command_length = 0;
    p_sis->i_splice_command_type = 0x00;

    /* FIXME: splice_info_section comes here */

    /* descriptors */
    p_sis->i_descriptors_length = 0;
    p_sis->p_first_descriptor = NULL;

    /* FIXME: alignment stuffing */

    p_sis->i_ecrc = 0;
}

/*****************************************************************************
 * dvbpsi_EmptySIS
 *****************************************************************************
 * Clean a dvbpsi_sis_t structure.
 *****************************************************************************/
void dvbpsi_EmptySIS(dvbpsi_sis_t* p_sis)
{
    /* FIXME: free splice_command_sections */

    dvbpsi_DeleteDescriptors(p_sis->p_first_descriptor);
    p_sis->p_first_descriptor = NULL;

    /* FIXME: free alignment stuffing */
}

/*****************************************************************************
 * dvbpsi_SISAddDescriptor
 *****************************************************************************
 * Add a descriptor in the SIS service description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_SISAddDescriptor( dvbpsi_sis_t *p_sis,
                                              uint8_t i_tag, uint8_t i_length,
                                              uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor;

    p_descriptor = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor)
    {
        if (p_sis->p_first_descriptor == NULL)
        {
            p_sis->p_first_descriptor = p_descriptor;
        }
        else
        {
            dvbpsi_descriptor_t * p_last_descriptor = p_sis->p_first_descriptor;
            while (p_last_descriptor->p_next != NULL)
                p_last_descriptor = p_last_descriptor->p_next;
            p_last_descriptor->p_next = p_descriptor;
        }
    }

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_GatherSISSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_GatherSISSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
    dvbpsi_sis_decoder_t * p_sis_decoder
                        = (dvbpsi_sis_decoder_t*)p_private_decoder;
    int b_append = 1;
    int b_reinit = 0;

    DVBPSI_DEBUG_ARG("SIS decoder",
                     "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                     "section %3d up to %3d, " "current %1d",
                     p_section->i_version, p_section->i_table_id,
                     p_section->i_extension,
                     p_section->i_number, p_section->i_last_number,
                     p_section->b_current_next);

    if (p_section->i_table_id != 0xFC)
    {
        /* Invalid table_id value */
        DVBPSI_ERROR_ARG("SIS decoder",
                         "invalid section (table_id == 0x%02x)",
                          p_section->i_table_id);
        b_append = 0;
    }

    if (p_section->b_syntax_indicator != 0)
    {
        /* Invalid section_syntax_indicator */
        DVBPSI_ERROR("SIS decoder",
                     "invalid section (section_syntax_indicator != 0)");
        b_append = 0;
    }

    if (p_section->b_private_indicator != 0)
    {
        /* Invalid private_syntax_indicator */
        DVBPSI_ERROR("SIS decoder",
                     "invalid private section (private_syntax_indicator != 0)");
        b_append = 0;
    }

    /* Now if b_append is true then we have a valid SIS section */
    if (b_append)
    {
        /* TS discontinuity check */
        if (p_psi_decoder->b_discontinuity)
        {
            b_reinit = 1;
            p_psi_decoder->b_discontinuity = 0;
        }
        else
        {
            /* Perform a few sanity checks */
            if (p_sis_decoder->p_building_sis)
            {
                if (p_sis_decoder->p_building_sis->i_protocol_version != 0)
                {
                    /* transport_stream_id */
                    DVBPSI_ERROR("SIS decoder",
                                 "'protocol_version' differs");\
                    b_reinit = 1;
                }
            }
            else
            {
                if (p_sis_decoder->b_current_valid)
                {
                    /* Don't decode since this version is already decoded */
                    b_append = 0;
                }
            }
        }
    }

    /* Reinit the decoder if wanted */
    if (b_reinit)
    {
        /* Force redecoding */
        p_sis_decoder->b_current_valid = 0;

        /* Free structures */
        if (p_sis_decoder->p_building_sis)
        {
            free(p_sis_decoder->p_building_sis);
            p_sis_decoder->p_building_sis = NULL;
        }
    }

    /* Append the section to the list if wanted */
    if (b_append)
    {
        /* Initialize the structures if it's the first section received */
        if (!p_sis_decoder->p_building_sis)
        {
            p_sis_decoder->p_building_sis =
                                (dvbpsi_sis_t*)malloc(sizeof(dvbpsi_sis_t));
            // FIXME: potiential crash on OUT OF MEMORY
            dvbpsi_InitSIS(p_sis_decoder->p_building_sis, 0);
        }
    }
    else
    {
        dvbpsi_DeletePSISections(p_section);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeSISSection
 *****************************************************************************
 * SIS decoder.
 *****************************************************************************/
void dvbpsi_DecodeSISSections(dvbpsi_sis_t* p_sis,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t *p_byte, *p_end;

    while (p_section)
    {
        for (p_byte = p_section->p_payload_start + 3;
             p_byte < p_section->p_payload_end; )
        {
            p_sis->i_protocol_version = p_byte[3];
            p_sis->b_encrypted_packet = ((p_byte[4] & 0x80)>>8);
            /* NOTE: cannot handle encrypted packet */
            assert(p_sis->b_encrypted_packet == 1);
            p_sis->i_encryption_algorithm = ((p_byte[4] & 0x7E) >> 1);
            p_sis->i_pts_adjustment = ((((uint64_t)p_byte[4] & 0x01) << 32) |
                                        ((uint64_t)p_byte[5] << 24) |
                                        ((uint64_t)p_byte[6] << 16) |
                                        ((uint64_t)p_byte[7] << 8)  |
                                         (uint64_t)p_byte[8]);
            p_sis->cw_index = p_byte[9];
            p_sis->i_splice_command_length = ((p_byte[11] & 0x0F) << 8) | p_byte[12];
            p_sis->i_splice_command_type = p_byte[13];

            uint32_t i_splice_command_length = p_sis->i_splice_command_length;
            if (p_sis->i_splice_command_length == 0xfff)
            {
                /* FIXME: size 0xfff of splice_command_section is undefined */
                assert(p_sis->i_splice_command_length != 0xfff);
            }
            /* FIXME: handle splice_command_sections */

            /* Service descriptors */
            uint8_t *p_desc = p_byte + 13 + i_splice_command_length;
            p_sis->i_descriptors_length = (p_desc[0] << 8) | p_desc[1];

            p_desc += 1;
            p_end = p_desc + p_sis->i_descriptors_length;
            if (p_end > p_section->p_payload_end) break;

            while (p_desc + 2 <= p_end)
            {
                uint8_t i_tag = p_desc[0];
                uint8_t i_length = p_desc[1];
                if ((i_length <= 254) &&
                    (i_length + 2 <= p_end - p_desc))
                    dvbpsi_SISAddDescriptor(p_sis, i_tag, i_length, p_desc + 2);
                p_desc += 2 + i_length;
            }

            if (p_sis->b_encrypted_packet == 1)
            {
                /* FIXME: Currently ignored */
                /* Calculate crc32 over decoded
                 * p_sis->i_splice_command_type till p_sis->i_ecrc,
                 * the result should be exactly p_sis->i_ecrc and indicates
                 * a successfull decryption.
                 */
                p_desc += 4; /* E CRC 32 */
            }

            /* point to next section */
            p_byte = p_desc + 4 /* CRC 32 */;
        }

        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_GenSISSections
 *****************************************************************************
 * Generate SIS sections based on the dvbpsi_sis_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_GenSISSections(dvbpsi_sis_t* p_sis)
{
    dvbpsi_psi_section_t * p_current = dvbpsi_NewPSISection(1024);

    p_current->i_table_id = 0xFC;
    p_current->b_syntax_indicator = 0;
    p_current->b_private_indicator = 0;
    p_current->i_length = 3;                     /* header + CRC_32 */

    /* FIXME: looks weird */
    p_current->p_payload_end += 2;               /* just after the header */
    p_current->p_payload_start = p_current->p_data + 3;

    p_current->p_data[3] = p_sis->i_protocol_version;
    p_current->p_data[4] = p_sis->b_encrypted_packet ? 0x80 : 0x0;
    /* NOTE: cannot handle encrypted packet */
    assert(p_sis->b_encrypted_packet == 1);
    p_current->p_data[4] |= ((p_sis->i_encryption_algorithm << 1) & 0x7E);

    p_current->p_data[4] |= ((p_sis->i_pts_adjustment & 0x00800) >> 32);
    p_current->p_data[5] = (p_sis->i_pts_adjustment >> 24);
    p_current->p_data[6] = (p_sis->i_pts_adjustment >> 16);
    p_current->p_data[7] = (p_sis->i_pts_adjustment >> 8);
    p_current->p_data[8] =  p_sis->i_pts_adjustment;

    p_current->p_data[9]  = p_sis->cw_index;
    p_current->p_data[11] = 0x00;
    p_current->p_data[11]|= ((p_sis->i_splice_command_length >> 8) & 0x0F);
    p_current->p_data[12] = (uint8_t) (p_sis->i_splice_command_length & 0xFF);
    p_current->p_data[13] = p_sis->i_splice_command_type;

    uint32_t i_desc_start = 13 + p_sis->i_splice_command_length;
    if (p_sis->i_splice_command_length == 0xfff)
    {
        /* FIXME: size 0xfff of splice_command_section is undefined */
        assert(p_sis->i_splice_command_length != 0xfff);
    }

    /* FIXME: handle splice_command_sections */

    /* Service descriptors */
    p_current->p_data[i_desc_start] = (p_sis->i_descriptors_length >> 8);
    p_current->p_data[i_desc_start+1] = (uint8_t)(p_sis->i_descriptors_length & 0xFF);

    p_current->p_payload_end += (i_desc_start + 1);

    uint32_t i_desc_length = 0;

    dvbpsi_descriptor_t * p_descriptor = p_sis->p_first_descriptor;
    while ((p_descriptor != NULL) && (p_current->i_length <= 1018))
    {
        i_desc_length += p_descriptor->i_length + 2;
        p_descriptor = p_descriptor->p_next;

        /* p_payload_end is where the descriptor begins */
        p_current->p_payload_end[0] = p_descriptor->i_tag;
        p_current->p_payload_end[1] = p_descriptor->i_length;
        memcpy(p_current->p_payload_end + 2, p_descriptor->p_data, p_descriptor->i_length);
        /* Increase length by descriptor_length + 2 */
        p_current->p_payload_end += p_descriptor->i_length + 2;
        p_current->i_length += p_descriptor->i_length + 2;

    }
    /* Coding error if this condition is not met */
    assert( i_desc_length == p_sis->i_descriptors_length);

    /* Finalization */
    dvbpsi_BuildPSISection(p_current);
    return p_current;
}

