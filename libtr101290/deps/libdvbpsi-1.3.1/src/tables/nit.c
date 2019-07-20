/*****************************************************************************
 * nit.c: NIT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2012 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Jean-Paul Saman <jpsaman@videolan.org>
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
#include "nit.h"
#include "nit_private.h"

/*****************************************************************************
 * dvbpsi_nit_attach
 *****************************************************************************
 * Initialize a NIT subtable decoder.
 *****************************************************************************/
bool dvbpsi_nit_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id,
                      uint16_t i_extension, dvbpsi_nit_callback pf_callback,
                      void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "NIT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_nit_decoder_t*  p_nit_decoder;
    p_nit_decoder = (dvbpsi_nit_decoder_t*) dvbpsi_decoder_new(NULL,
                                             0, true, sizeof(dvbpsi_nit_decoder_t));
    if (p_nit_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_nit_detach,
                                         dvbpsi_nit_sections_gather, DVBPSI_DECODER(p_nit_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_nit_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* NIT decoder information */
    p_nit_decoder->i_network_id = i_extension;
    p_nit_decoder->pf_nit_callback = pf_callback;
    p_nit_decoder->p_cb_data = p_cb_data;
    p_nit_decoder->p_building_nit = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_nit_detach
 *****************************************************************************
 * Close a NIT decoder.
 *****************************************************************************/
void dvbpsi_nit_detach(dvbpsi_t * p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "NIT Decoder",
                     "No such NIT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return;
    }

    dvbpsi_nit_decoder_t* p_nit_decoder;
    p_nit_decoder = (dvbpsi_nit_decoder_t*)p_subdec->p_decoder;
    if (p_nit_decoder->p_building_nit)
        dvbpsi_nit_delete(p_nit_decoder->p_building_nit);
    p_nit_decoder->p_building_nit = NULL;

    /* Free demux sub table decoder */
    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/****************************************************************************
 * dvbpsi_nit_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_nit_t structure.
 *****************************************************************************/
void dvbpsi_nit_init(dvbpsi_nit_t* p_nit, uint8_t i_table_id, uint16_t i_extension,
                     uint16_t i_network_id, uint8_t i_version, bool b_current_next)
{
    p_nit->i_table_id = i_table_id;
    p_nit->i_extension = i_extension;

    p_nit->i_network_id = i_network_id;
    p_nit->i_version = i_version;
    p_nit->b_current_next = b_current_next;
    p_nit->p_first_descriptor = NULL;
    p_nit->p_first_ts = NULL;
}

/****************************************************************************
 * dvbpsi_nit_new
 *****************************************************************************
 * Allocate and initialize a dvbpsi_nit_t structure.
 *****************************************************************************/
dvbpsi_nit_t *dvbpsi_nit_new(uint8_t i_table_id, uint16_t i_extension,
                             uint16_t i_network_id, uint8_t i_version,
                             bool b_current_next)
{
    dvbpsi_nit_t*p_nit = (dvbpsi_nit_t*)malloc(sizeof(dvbpsi_nit_t));
    if (p_nit != NULL)
       dvbpsi_nit_init(p_nit, i_table_id, i_extension, i_network_id,
                       i_version, b_current_next);
    return p_nit;
}

/*****************************************************************************
 * dvbpsi_nit_empty
 *****************************************************************************
 * Clean a dvbpsi_nit_t structure.
 *****************************************************************************/
void dvbpsi_nit_empty(dvbpsi_nit_t* p_nit)
{
    dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;

    dvbpsi_DeleteDescriptors(p_nit->p_first_descriptor);

    while (p_ts != NULL)
    {
        dvbpsi_nit_ts_t* p_tmp = p_ts->p_next;
        dvbpsi_DeleteDescriptors(p_ts->p_first_descriptor);
        free(p_ts);
        p_ts = p_tmp;
    }

    p_nit->p_first_descriptor = NULL;
    p_nit->p_first_ts = NULL;
}

/****************************************************************************
 * dvbpsi_nit_delete
 *****************************************************************************
 * Clean and delete a dvbpsi_nit_t structure.
 *****************************************************************************/
void dvbpsi_nit_delete(dvbpsi_nit_t *p_nit)
{
    if (p_nit)
        dvbpsi_nit_empty(p_nit);
    free(p_nit);
}

/*****************************************************************************
 * dvbpsi_nit_descriptor_add
 *****************************************************************************
 * Add a descriptor in the NIT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_nit_descriptor_add(dvbpsi_nit_t* p_nit,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    if (p_nit->p_first_descriptor == NULL)
        p_nit->p_first_descriptor = p_descriptor;
    else
    {
        dvbpsi_descriptor_t* p_last_descriptor = p_nit->p_first_descriptor;
        while(p_last_descriptor->p_next != NULL)
            p_last_descriptor = p_last_descriptor->p_next;
        p_last_descriptor->p_next = p_descriptor;
    }
    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_nit_ts_add
 *****************************************************************************
 * Add an TS in the NIT.
 *****************************************************************************/
dvbpsi_nit_ts_t* dvbpsi_nit_ts_add(dvbpsi_nit_t* p_nit,
                                   uint16_t i_ts_id, uint16_t i_orig_network_id)
{
    dvbpsi_nit_ts_t* p_ts = (dvbpsi_nit_ts_t*)malloc(sizeof(dvbpsi_nit_ts_t));
    if (p_ts == NULL)
        return NULL;

    p_ts->i_ts_id = i_ts_id;
    p_ts->i_orig_network_id = i_orig_network_id;
    p_ts->p_first_descriptor = NULL;
    p_ts->p_next = NULL;

    if (p_nit->p_first_ts == NULL)
        p_nit->p_first_ts = p_ts;
    else
    {
        dvbpsi_nit_ts_t* p_last_ts = p_nit->p_first_ts;
        while(p_last_ts->p_next != NULL)
            p_last_ts = p_last_ts->p_next;
        p_last_ts->p_next = p_ts;
    }
    return p_ts;
}

/*****************************************************************************
 * dvbpsi_nit_ts_descriptor_add
 *****************************************************************************
 * Add a descriptor in the NIT TS.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_nit_ts_descriptor_add(dvbpsi_nit_ts_t* p_ts,
                                                  uint8_t i_tag, uint8_t i_length,
                                                  uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_ts->p_first_descriptor = dvbpsi_AddDescriptor(p_ts->p_first_descriptor,
                                                    p_descriptor);
    assert(p_ts->p_first_descriptor);
    if (p_ts->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/* */
static void dvbpsi_ReInitNIT(dvbpsi_nit_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_nit)
            dvbpsi_nit_delete(p_decoder->p_building_nit);
    }
    p_decoder->p_building_nit = NULL;
}

static bool dvbpsi_CheckNIT(dvbpsi_t *p_dvbpsi, dvbpsi_nit_decoder_t *p_nit_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    assert(p_dvbpsi);
    assert(p_nit_decoder);

    bool b_reinit = false;

    if (p_nit_decoder->p_building_nit->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "NIT decoder",
                "'version_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_nit_decoder->i_last_section_number
                                    != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "NIT decoder",
                "'last_section_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionNIT(dvbpsi_t *p_dvbpsi, dvbpsi_nit_decoder_t *p_nit_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_nit_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (p_nit_decoder->p_building_nit == NULL)
    {
        p_nit_decoder->p_building_nit = dvbpsi_nit_new(p_section->i_table_id,
                p_section->i_extension, p_nit_decoder->i_network_id,
                p_section->i_version, p_section->b_current_next);
        if (p_nit_decoder->p_building_nit == NULL)
            return false;
        p_nit_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_nit_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "NIT decoder", "overwrite section number %d",
                               p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_nit_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_nit_sections_gather(dvbpsi_t *p_dvbpsi,
                                dvbpsi_decoder_t *p_private_decoder,
                                dvbpsi_psi_section_t *p_section)
{
    assert(p_dvbpsi);

    const uint8_t i_table_id = ((p_section->i_table_id == 0x40) ||
                                (p_section->i_table_id == 0x41)) ?
                                    p_section->i_table_id : 0x40;

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, i_table_id, "NIT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_nit_decoder_t* p_nit_decoder
                        = (dvbpsi_nit_decoder_t*)p_private_decoder;

    /* We have a valid NIT section */
    if (p_nit_decoder->i_network_id != p_section->i_extension)
    {
        /* Invalid program_number */
        dvbpsi_error(p_dvbpsi, "NIT decoder", "'network_id' don't match");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_nit_decoder->b_discontinuity)
    {
        dvbpsi_ReInitNIT(p_nit_decoder, true);
        p_nit_decoder->b_discontinuity = false;
    }
    else
    {
        /* Perform some few sanity checks */
        if (p_nit_decoder->p_building_nit)
        {
            if (dvbpsi_CheckNIT(p_dvbpsi, p_nit_decoder, p_section))
                dvbpsi_ReInitNIT(p_nit_decoder, true);
        }
        else
        {
            if (   (p_nit_decoder->b_current_valid)
                && (p_nit_decoder->current_nit.i_version == p_section->i_version)
                && (p_nit_decoder->current_nit.b_current_next == p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "NIT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;;
            }
        }
    }

    /* Add section to NIT */
    if (!dvbpsi_AddSectionNIT(p_dvbpsi, p_nit_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "NIT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_nit_decoder)))
    {
        assert(p_nit_decoder->pf_nit_callback);

        /* Save the current information */
        p_nit_decoder->current_nit = *p_nit_decoder->p_building_nit;
        p_nit_decoder->b_current_valid = true;

        /* Decode the sections */
        dvbpsi_nit_sections_decode(p_nit_decoder->p_building_nit,
                                   p_nit_decoder->p_sections);
        /* signal the new NIT */
        p_nit_decoder->pf_nit_callback(p_nit_decoder->p_cb_data,
                                       p_nit_decoder->p_building_nit);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitNIT(p_nit_decoder, false);
        assert(p_nit_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_nit_sections_decode
 *****************************************************************************
 * NIT decoder.
 *****************************************************************************/
void dvbpsi_nit_sections_decode(dvbpsi_nit_t* p_nit,
                                dvbpsi_psi_section_t* p_section)
{
    uint8_t* p_byte, * p_end;

    while (p_section)
    {
        /* - NIT descriptors */
        p_byte = p_section->p_payload_start + 2;
        p_end = p_byte + (((uint16_t)(p_section->p_payload_start[0] & 0x0f) << 8)
                          | p_section->p_payload_start[1]);

        while (p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if (i_length + 2 <= p_end - p_byte)
                dvbpsi_nit_descriptor_add(p_nit, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }

        /* Transport stream loop length */
        p_end = 2 + p_byte + (((uint16_t)(p_byte[0] & 0x0f) << 8) | p_byte[1]);
        if (p_end > p_section->p_payload_end)
            p_end = p_section->p_payload_end;

        p_byte += 2;

        /* - TSs */
        while(p_byte + 6 <= p_end)
        {
            uint8_t *p_end2; /* descriptor loop end */

            uint16_t i_ts_id = ((uint16_t)p_byte[0] << 8) | p_byte[1];
            uint16_t i_orig_network_id = ((uint16_t)p_byte[2] << 8) | p_byte[3];
            uint16_t i_ts_length = ((uint16_t)(p_byte[4] & 0x0f) << 8) | p_byte[5];

            dvbpsi_nit_ts_t* p_ts = dvbpsi_nit_ts_add(p_nit, i_ts_id, i_orig_network_id);
            if (!p_ts)
                break;

            /* - TS descriptors */
            p_byte += 6;
            p_end2 = p_byte + i_ts_length;
            if (p_end2 > p_section->p_payload_end)
                p_end2 = p_section->p_payload_end;

            while (p_byte + 2 <= p_end2)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_length = p_byte[1];
                if (i_length + 2 <= p_end2 - p_byte)
                    dvbpsi_nit_ts_descriptor_add(p_ts, i_tag, i_length, p_byte + 2);
                p_byte += 2 + i_length;
            }
        }
        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_nit_sections_generate
 *****************************************************************************
 * Generate NIT sections based on the dvbpsi_nit_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_nit_sections_generate(dvbpsi_t *p_dvbpsi,
                                            dvbpsi_nit_t* p_nit, uint8_t i_table_id)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_descriptor_t* p_descriptor = p_nit->p_first_descriptor;
    dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;
    uint16_t i_network_descriptors_length, i_transport_stream_loop_length;
    uint8_t * p_transport_stream_loop_length;

    p_current->i_table_id = i_table_id;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = false;
    p_current->i_length = 13;                     /* including CRC_32 */
    p_current->i_extension = p_nit->i_network_id;
    p_current->i_version = p_nit->i_version;
    p_current->b_current_next = p_nit->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 10;
    p_current->p_payload_start = p_current->p_data + 8;

    /* NIT descriptors */
    while (p_descriptor != NULL)
    {
        /* New section if needed */
        /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
        if ((p_current->p_payload_end - p_current->p_data)
                                + p_descriptor->i_length > 1018)
        {
            /* network_descriptors_length */
            i_network_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
            p_current->p_data[8] = (i_network_descriptors_length >> 8) | 0xf0;
            p_current->p_data[9] = i_network_descriptors_length;

            /* transport_stream_loop_length */
            p_current->p_payload_end[0] = 0;
            p_current->p_payload_end[1] = 0;
            p_current->p_payload_end += 2;

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = i_table_id;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 13;                 /* including CRC_32 */
            p_current->i_extension = p_nit->i_network_id;
            p_current->i_version = p_nit->i_version;
            p_current->b_current_next = p_nit->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 10;
            p_current->p_payload_start = p_current->p_data + 8;
        }

        /* p_payload_end is where the descriptor begins */
        p_current->p_payload_end[0] = p_descriptor->i_tag;
        p_current->p_payload_end[1] = p_descriptor->i_length;
        memcpy(p_current->p_payload_end + 2,
               p_descriptor->p_data,
               p_descriptor->i_length);

        /* Increase length by descriptor_length + 2 */
        p_current->p_payload_end += p_descriptor->i_length + 2;
        p_current->i_length += p_descriptor->i_length + 2;

        p_descriptor = p_descriptor->p_next;
    }

    /* network_descriptors_length */
    i_network_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
    p_current->p_data[8] = (i_network_descriptors_length >> 8) | 0xf0;
    p_current->p_data[9] = i_network_descriptors_length;

    /* Store the position of the transport_stream_loop_length field
       and reserve two bytes for it */
    p_transport_stream_loop_length = p_current->p_payload_end;
    p_current->p_payload_end += 2;

    /* NIT TSs */
    while (p_ts != NULL)
    {
        uint8_t* p_ts_start = p_current->p_payload_end;
        uint16_t i_ts_length = 5;

        /* Can the current section carry all the descriptors ? */
        p_descriptor = p_ts->p_first_descriptor;
        while(    (p_descriptor != NULL)
               && ((p_ts_start - p_current->p_data) + i_ts_length <= 1020))
        {
            i_ts_length += p_descriptor->i_length + 2;
            p_descriptor = p_descriptor->p_next;
        }

        /* If _no_ and the current section isn't empty and an empty section
           may carry one more descriptor
           then create a new section */
        if(    (p_descriptor != NULL)
            && (p_ts_start - p_current->p_data != 12)
            && (i_ts_length <= 1008))
        {
            /* transport_stream_loop_length */
            i_transport_stream_loop_length = (p_current->p_payload_end - p_transport_stream_loop_length) - 2;
            p_transport_stream_loop_length[0] = (i_transport_stream_loop_length >> 8) | 0xf0;
            p_transport_stream_loop_length[1] = i_transport_stream_loop_length;

            /* will put more descriptors in an empty section */
            dvbpsi_debug(p_dvbpsi, "NIT generator",
                                   "create a new section to carry more TS descriptors");

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = i_table_id;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 13;                 /* including CRC_32 */
            p_current->i_extension = p_nit->i_network_id;
            p_current->i_version = p_nit->i_version;
            p_current->b_current_next = p_nit->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 10;
            p_current->p_payload_start = p_current->p_data + 8;

            /* network_descriptors_length = 0 */
            p_current->p_data[8] = 0xf0;
            p_current->p_data[9] = 0x00;

            /* Store the position of the transport_stream_loop_length field
               and reserve two bytes for it */
            p_transport_stream_loop_length = p_current->p_payload_end;
            p_current->p_payload_end += 2;

            p_ts_start = p_current->p_payload_end;
        }

        /* p_ts_start is where the TS begins */
        p_ts_start[0] = p_ts->i_ts_id >> 8;
        p_ts_start[1] = p_ts->i_ts_id & 0xff;
        p_ts_start[2] = p_ts->i_orig_network_id >> 8;
        p_ts_start[3] = p_ts->i_orig_network_id & 0xff;

        /* Increase the length by 6 */
        p_current->p_payload_end += 6;
        p_current->i_length += 6;

        /* TS descriptors */
        p_descriptor = p_ts->p_first_descriptor;
        while (   (p_descriptor != NULL)
           && (   (p_current->p_payload_end - p_current->p_data)
                + p_descriptor->i_length <= 1018))
        {
            /* p_payload_end is where the descriptor begins */
            p_current->p_payload_end[0] = p_descriptor->i_tag;
            p_current->p_payload_end[1] = p_descriptor->i_length;
            memcpy(p_current->p_payload_end + 2,
                    p_descriptor->p_data,
                    p_descriptor->i_length);

            /* Increase length by descriptor_length + 2 */
            p_current->p_payload_end += p_descriptor->i_length + 2;
            p_current->i_length += p_descriptor->i_length + 2;

            p_descriptor = p_descriptor->p_next;
        }

        if (p_descriptor != NULL)
            dvbpsi_error(p_dvbpsi, "NIT generator", "unable to carry all the TS descriptors");

        /* TS_info_length */
        i_ts_length = p_current->p_payload_end - p_ts_start - 6;
        p_ts_start[4] = (i_ts_length >> 8) | 0xf0;
        p_ts_start[5] = i_ts_length;

        p_ts = p_ts->p_next;
    }

    /* transport_stream_loop_length */
    i_transport_stream_loop_length = (p_current->p_payload_end - p_transport_stream_loop_length) - 2;
    p_transport_stream_loop_length[0] = (i_transport_stream_loop_length >> 8) | 0xf0;
    p_transport_stream_loop_length[1] = i_transport_stream_loop_length;

    /* Finalization */
    p_prev = p_result;
    while (p_prev != NULL)
    {
        p_prev->i_last_number = p_current->i_number;
        dvbpsi_BuildPSISection(p_dvbpsi, p_prev);
        p_prev = p_prev->p_next;
    }

    return p_result;
}
