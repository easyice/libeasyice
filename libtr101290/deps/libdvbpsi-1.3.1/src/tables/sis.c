/*****************************************************************************
 * sis.c: SIS decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2010-2011 VideoLAN
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
#include <stdbool.h>
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
 * dvbpsi_sis_attach
 *****************************************************************************
 * Initialize a SIS subtable decoder.
 *****************************************************************************/
bool dvbpsi_sis_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                      dvbpsi_sis_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    i_extension = 0;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                         "Already a decoder for (table_id == 0x%02x,"
                         "extension == 0x%02x)",
                         i_table_id, i_extension);
        return false;
    }

    dvbpsi_sis_decoder_t*  p_sis_decoder;
    p_sis_decoder = (dvbpsi_sis_decoder_t*) dvbpsi_decoder_new(NULL,
                                             0, true, sizeof(dvbpsi_sis_decoder_t));
    if (p_sis_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_sis_detach,
                                         dvbpsi_sis_sections_gather, DVBPSI_DECODER(p_sis_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_sis_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* SIS decoder information */
    p_sis_decoder->pf_sis_callback = pf_callback;
    p_sis_decoder->p_cb_data = p_cb_data;
    p_sis_decoder->p_building_sis = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_sis_detach
 *****************************************************************************
 * Close a SIS decoder.
 *****************************************************************************/
void dvbpsi_sis_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    i_extension = 0;
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "SIS Decoder",
                         "No such SIS decoder (table_id == 0x%02x,"
                         "extension == 0x%02x)",
                         i_table_id, i_extension);
        return;
    }

    assert(p_subdec->p_decoder);

    dvbpsi_sis_decoder_t* p_sis_decoder;
    p_sis_decoder = (dvbpsi_sis_decoder_t*)p_subdec->p_decoder;
    if (p_sis_decoder->p_building_sis)
        dvbpsi_sis_delete(p_sis_decoder->p_building_sis);
    p_sis_decoder->p_building_sis = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_sis_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_sis_t structure.
 *****************************************************************************/
void dvbpsi_sis_init(dvbpsi_sis_t *p_sis, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next, uint8_t i_protocol_version)
{
    p_sis->i_table_id = i_table_id;
    p_sis->i_extension = i_extension;

    p_sis->i_version = i_version;
    p_sis->b_current_next = b_current_next;

    assert(i_protocol_version == 0);
    p_sis->i_protocol_version = 0; /* must be 0 */

    /* encryption */
    p_sis->b_encrypted_packet = false;
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
 * dvbpsi_sis_new
 *****************************************************************************
 * Allocate and Initialize a new dvbpsi_sis_t structure.
 *****************************************************************************/
dvbpsi_sis_t* dvbpsi_sis_new(uint8_t i_table_id, uint16_t i_extension, uint8_t i_version,
                             bool b_current_next, uint8_t i_protocol_version)
{
    dvbpsi_sis_t* p_sis = (dvbpsi_sis_t*)malloc(sizeof(dvbpsi_sis_t));
    if (p_sis != NULL)
        dvbpsi_sis_init(p_sis, i_table_id, i_extension, i_version,
                        b_current_next, i_protocol_version);
    return p_sis;
}

/*****************************************************************************
 * dvbpsi_sis_empty
 *****************************************************************************
 * Clean a dvbpsi_sis_t structure.
 *****************************************************************************/
void dvbpsi_sis_empty(dvbpsi_sis_t* p_sis)
{
    /* FIXME: free splice_command_sections */

    dvbpsi_DeleteDescriptors(p_sis->p_first_descriptor);
    p_sis->p_first_descriptor = NULL;

    /* FIXME: free alignment stuffing */
}

/*****************************************************************************
 * dvbpsi_sis_delete
 *****************************************************************************
 * Clean and Delete a dvbpsi_sis_t structure.
 *****************************************************************************/
void dvbpsi_sis_delete(dvbpsi_sis_t *p_sis)
{
    if (p_sis)
        dvbpsi_sis_empty(p_sis);
    free(p_sis);
}

/*****************************************************************************
 * dvbpsi_sis_descriptor_add
 *****************************************************************************
 * Add a descriptor in the SIS service description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_sis_descriptor_add(dvbpsi_sis_t *p_sis,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor;
    p_descriptor = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_sis->p_first_descriptor = dvbpsi_AddDescriptor(p_sis->p_first_descriptor,
                                                     p_descriptor);
    assert(p_sis->p_first_descriptor);
    if (p_sis->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/* */
static void dvbpsi_ReInitSIS(dvbpsi_sis_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_sis)
            dvbpsi_sis_delete(p_decoder->p_building_sis);
    }
    p_decoder->p_building_sis = NULL;
}

static bool dvbpsi_CheckSIS(dvbpsi_t *p_dvbpsi, dvbpsi_sis_decoder_t* p_sis_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi);
    assert(p_sis_decoder);

    if (p_sis_decoder->p_building_sis->i_protocol_version != 0)
    {
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                     "'protocol_version' differs"
                     " while no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_sis_decoder->p_building_sis->i_extension != p_section->i_extension)
    {
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                "'transport_stream_id' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_sis_decoder->p_building_sis->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                "'version_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_sis_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                "'last_section_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionSIS(dvbpsi_t *p_dvbpsi, dvbpsi_sis_decoder_t *p_sis_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_sis_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_sis_decoder->p_building_sis)
    {
        p_sis_decoder->p_building_sis = dvbpsi_sis_new(
                            p_section->i_table_id, p_section->i_extension,
                            p_section->i_version, p_section->b_current_next, 0);
        if (p_sis_decoder->p_building_sis == NULL)
            return false;
        p_sis_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_sis_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "SDT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_sis_sections_gather
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_sis_sections_gather(dvbpsi_t *p_dvbpsi,
                              dvbpsi_decoder_t *p_decoder,
                              dvbpsi_psi_section_t * p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0xFC, "SIS decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_sis_decoder_t * p_sis_decoder = (dvbpsi_sis_decoder_t*)p_decoder;

    if (p_section->b_private_indicator)
    {
        /* Invalid private_syntax_indicator */
        dvbpsi_error(p_dvbpsi, "SIS decoder",
                     "invalid private section (private_syntax_indicator != false)");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitSIS(p_sis_decoder, true);
        p_sis_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_sis_decoder->p_building_sis)
        {
            if (dvbpsi_CheckSIS(p_dvbpsi, p_sis_decoder, p_section))
                dvbpsi_ReInitSIS(p_sis_decoder, true);
        }
        else
        {
            if(     (p_sis_decoder->b_current_valid)
                 && (p_sis_decoder->current_sis.i_version == p_section->i_version)
                 && (p_sis_decoder->current_sis.b_current_next == p_section->b_current_next))
             {
                 /* Don't decode since this version is already decoded */
                 dvbpsi_debug(p_dvbpsi, "SIT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                 dvbpsi_DeletePSISections(p_section);
                 return;
             }
        }
    }

    /* Add section to SIS */
    if (!dvbpsi_AddSectionSIS(p_dvbpsi, p_sis_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "SIS decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_sis_decoder)))
    {
        assert(p_sis_decoder->pf_sis_callback);

        /* Save the current information */
        p_sis_decoder->current_sis = *p_sis_decoder->p_building_sis;
        p_sis_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_sis_sections_decode(p_dvbpsi, p_sis_decoder->p_building_sis,
                                   p_sis_decoder->p_sections);
        /* signal the new SDT */
        p_sis_decoder->pf_sis_callback(p_sis_decoder->p_cb_data,
                                       p_sis_decoder->p_building_sis);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitSIS(p_sis_decoder, false);
        assert(p_sis_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_sis_sections_decode
 *****************************************************************************
 * SIS decoder.
 *****************************************************************************/
void dvbpsi_sis_sections_decode(dvbpsi_t* p_dvbpsi, dvbpsi_sis_t* p_sis,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t *p_byte, *p_end;

    while (p_section)
    {
        for (p_byte = p_section->p_payload_start + 3;
             p_byte < p_section->p_payload_end; )
        {
            p_sis->i_protocol_version = p_byte[3];
            p_sis->b_encrypted_packet = ((p_byte[4] & 0x80) == 0x80);
            /* NOTE: cannot handle encrypted packet */
            assert(p_sis->b_encrypted_packet);
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
            switch(p_sis->i_splice_command_type)
            {
                case 0x00: /* splice_null */
                case 0x04: /* splice_schedule */
                case 0x05: /* splice_insert */
                case 0x06: /* time_signal */
                case 0x07: /* bandwidth_reservation */
                    break;
                default:
                    dvbpsi_error(p_dvbpsi, "SIS decoder", "invalid SIS Command found");
                    break;
            }

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
                    dvbpsi_sis_descriptor_add(p_sis, i_tag, i_length, p_desc + 2);
                p_desc += 2 + i_length;
            }

            if (p_sis->b_encrypted_packet)
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
 * dvbpsi_sis_sections_generate
 *****************************************************************************
 * Generate SIS sections based on the dvbpsi_sis_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_sis_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_sis_t* p_sis)
{
    dvbpsi_psi_section_t * p_current = dvbpsi_NewPSISection(1024);

    p_current->i_table_id = 0xFC;
    p_current->b_syntax_indicator = false;
    p_current->b_private_indicator = false;
    p_current->i_length = 3;                     /* header + CRC_32 */

    /* FIXME: looks weird */
    p_current->p_payload_end += 2;               /* just after the header */
    p_current->p_payload_start = p_current->p_data + 3;

    p_current->p_data[3] = p_sis->i_protocol_version;
    p_current->p_data[4] = p_sis->b_encrypted_packet ? 0x80 : 0x0;
    /* NOTE: cannot handle encrypted packet */
    assert(p_sis->b_encrypted_packet);
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
    dvbpsi_BuildPSISection(p_dvbpsi, p_current);
    return p_current;
}
