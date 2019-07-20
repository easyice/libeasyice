/*****************************************************************************
 * bat.c: BAT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: bat.c 110 2010-04-01 12:52:02Z gbazin $
 *
 * Authors: Zhu zhenglu <zhuzlu@gmail.com>
 *          heavily based on nit.c which was written by
 *          Johann Hanne
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
#include "bat.h"
#include "bat_private.h"

/*****************************************************************************
 * dvbpsi_bat_attach
 *****************************************************************************
 * Initialize a BAT subtable decoder.
 *****************************************************************************/
bool dvbpsi_bat_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_bat_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;
    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "BAT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_bat_decoder_t*  p_bat_decoder;
    p_bat_decoder = (dvbpsi_bat_decoder_t*) dvbpsi_decoder_new(NULL,
                                             0, true, sizeof(dvbpsi_bat_decoder_t));
    if (p_bat_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_bat_detach,
                                         dvbpsi_bat_sections_gather, DVBPSI_DECODER(p_bat_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_bat_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* BAT decoder information */
    p_bat_decoder->pf_bat_callback = pf_callback;
    p_bat_decoder->p_cb_data = p_cb_data;
    p_bat_decoder->p_building_bat = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_bat_detach
 *****************************************************************************
 * Close a BAT decoder.
 *****************************************************************************/
void dvbpsi_bat_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "BAT Decoder",
                     "No such BAT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return;
    }

    dvbpsi_bat_decoder_t* p_bat_decoder;
    p_bat_decoder = (dvbpsi_bat_decoder_t*)p_subdec->p_decoder;
    if (p_bat_decoder->p_building_bat)
        dvbpsi_bat_delete(p_bat_decoder->p_building_bat);
    p_bat_decoder->p_building_bat = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_bat_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_bat_t structure.
 *****************************************************************************/
void dvbpsi_bat_init(dvbpsi_bat_t* p_bat, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next)
{
    assert(p_bat);
    p_bat->i_table_id = i_table_id;
    p_bat->i_extension = i_extension;

    p_bat->i_version = i_version;
    p_bat->b_current_next = b_current_next;
    p_bat->p_first_ts = NULL;
    p_bat->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_bat_new
 *****************************************************************************
 * Allocate and initialize a dvbpsi_bat_t structure.
 *****************************************************************************/
dvbpsi_bat_t *dvbpsi_bat_new(uint8_t i_table_id, uint16_t i_extension,
                             uint8_t i_version, bool b_current_next)
{
    dvbpsi_bat_t *p_bat = (dvbpsi_bat_t*)malloc(sizeof(dvbpsi_bat_t));
    if(p_bat != NULL)
        dvbpsi_bat_init(p_bat, i_table_id, i_extension, i_version, b_current_next);
    return p_bat;
}

/*****************************************************************************
 * dvbpsi_bat_empty
 *****************************************************************************
 * Clean a dvbpsi_bat_t structure.
 *****************************************************************************/
void dvbpsi_bat_empty(dvbpsi_bat_t* p_bat)
{
    dvbpsi_bat_ts_t* p_ts = p_bat->p_first_ts;

    dvbpsi_DeleteDescriptors(p_bat->p_first_descriptor);
    p_bat->p_first_descriptor = NULL;

    while (p_ts != NULL)
    {
        dvbpsi_bat_ts_t* p_tmp = p_ts->p_next;
        dvbpsi_DeleteDescriptors(p_ts->p_first_descriptor);
        free(p_ts);
        p_ts = p_tmp;
    }
    p_bat->p_first_ts = NULL;
}

/*****************************************************************************
 * dvbpsi_bat_delete
 *****************************************************************************
 * Empty and Delere a dvbpsi_bat_t structure.
 *****************************************************************************/
void dvbpsi_bat_delete(dvbpsi_bat_t *p_bat)
{
    if (p_bat)
        dvbpsi_bat_empty(p_bat);
    free(p_bat);
}

/*****************************************************************************
 * dvbpsi_bat_bouquet_descriptor_add
 *****************************************************************************
 * Add a descriptor in the BAT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_bat_bouquet_descriptor_add(dvbpsi_bat_t* p_bat,
                                                       uint8_t i_tag, uint8_t i_length,
                                                       uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_bat->p_first_descriptor = dvbpsi_AddDescriptor(p_bat->p_first_descriptor,
                                                     p_descriptor);
    assert(p_bat->p_first_descriptor);
    if (p_bat->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_bat_ts_add
 *****************************************************************************
 * Add a TS description at the end of the BAT.
 *****************************************************************************/
dvbpsi_bat_ts_t *dvbpsi_bat_ts_add(dvbpsi_bat_t* p_bat,
                                 uint16_t i_ts_id, uint16_t i_orig_network_id)
{
    dvbpsi_bat_ts_t * p_ts
                = (dvbpsi_bat_ts_t*)malloc(sizeof(dvbpsi_bat_ts_t));
    if (p_ts == NULL)
        return NULL;

    p_ts->i_ts_id = i_ts_id;
    p_ts->i_orig_network_id = i_orig_network_id;
    p_ts->p_next = NULL;
    p_ts->p_first_descriptor = NULL;

    if (p_bat->p_first_ts == NULL)
        p_bat->p_first_ts = p_ts;
    else
    {
        dvbpsi_bat_ts_t * p_last_ts = p_bat->p_first_ts;
        while(p_last_ts->p_next != NULL)
            p_last_ts = p_last_ts->p_next;
        p_last_ts->p_next = p_ts;
    }

    return p_ts;
}


/*****************************************************************************
 * dvbpsi_bat_ts_descriptor_add
 *****************************************************************************
 * Add a descriptor in the BAT TS descriptors, which is in the second loop of BAT.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_bat_ts_descriptor_add(dvbpsi_bat_ts_t *p_bat,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    if (p_bat->p_first_descriptor == NULL)
        p_bat->p_first_descriptor = p_descriptor;
    else
    {
        dvbpsi_descriptor_t *p_last_descriptor = p_bat->p_first_descriptor;
        while(p_last_descriptor->p_next != NULL)
            p_last_descriptor = p_last_descriptor->p_next;
        p_last_descriptor->p_next = p_descriptor;
    }
    return p_descriptor;
}

/* */
static void dvbpsi_ReInitBAT(dvbpsi_bat_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_bat)
            dvbpsi_bat_delete(p_decoder->p_building_bat);
    }
    p_decoder->p_building_bat = NULL;
}

static bool dvbpsi_CheckBAT(dvbpsi_t *p_dvbpsi, dvbpsi_bat_decoder_t *p_bat_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi);
    assert(p_bat_decoder);

    if (p_bat_decoder->p_building_bat->i_extension != p_section->i_extension)
    {
        /* bouquet_id */
        dvbpsi_error(p_dvbpsi, "BAT decoder", "'bouquet_id' differs"
                        " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_bat_decoder->p_building_bat->i_version
                                        != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "BAT decoder", "'version_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_bat_decoder->i_last_section_number !=
                                        p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "BAT decoder", "'last_section_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionBAT(dvbpsi_t *p_dvbpsi, dvbpsi_bat_decoder_t *p_bat_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_bat_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_bat_decoder->p_building_bat)
    {
        p_bat_decoder->p_building_bat = dvbpsi_bat_new(
                              p_section->i_table_id, p_section->i_extension,
                              p_section->i_version, p_section->b_current_next);
        if (!p_bat_decoder->p_building_bat)
            return false;

        p_bat_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_bat_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "BAT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_bat_sections_gather
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_bat_sections_gather(dvbpsi_t *p_dvbpsi,
                              dvbpsi_decoder_t *p_decoder,
                              dvbpsi_psi_section_t * p_section)
{
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_bat_decoder_t * p_bat_decoder = (dvbpsi_bat_decoder_t *) p_decoder;

    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0x4a, "BAT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* We have a valid BAT section */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitBAT(p_bat_decoder, true);
        p_bat_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_bat_decoder->p_building_bat)
        {
            if (dvbpsi_CheckBAT(p_dvbpsi, p_bat_decoder, p_section))
                dvbpsi_ReInitBAT(p_bat_decoder, true);
        }
        else
        {
            if (   (p_bat_decoder->b_current_valid)
                && (p_bat_decoder->current_bat.i_version == p_section->i_version)
                && (p_bat_decoder->current_bat.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "BAT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
#if 0 /* FIXME: Is this really needed ? */
            else if (  (!p_bat_decoder->current_bat.b_current_next)
                     && (p_section->b_current_next))
            {
                /* Signal a new BAT if the previous one wasn't active */
                dvbpsi_bat_t *p_bat = (dvbpsi_bat_t*)malloc(sizeof(dvbpsi_bat_t));
                if (p_bat)
                {
                    p_bat_decoder->current_bat.b_current_next = true;
                    memcpy(p_bat, &p_bat_decoder->current_bat, sizeof(dvbpsi_bat_t));
                    p_bat_decoder->pf_bat_callback(p_bat_decoder->p_cb_data, p_bat);
                }
                else
                    dvbpsi_error(p_dvbpsi, "BAT decoder", "Could not signal new BAT.");
            }
            dvbpsi_DeletePSISections(p_section);
            return;
#endif
        }
    }

    /* Add section to BAT */
    if (!dvbpsi_AddSectionBAT(p_dvbpsi, p_bat_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "BAT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_bat_decoder)))
    {
        assert(p_bat_decoder->pf_bat_callback);

        /* Save the current information */
        p_bat_decoder->current_bat = *p_bat_decoder->p_building_bat;
        p_bat_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_bat_sections_decode(p_bat_decoder->p_building_bat,
                                   p_bat_decoder->p_sections);
        /* signal the new BAT */
        p_bat_decoder->pf_bat_callback(p_bat_decoder->p_cb_data,
                                       p_bat_decoder->p_building_bat);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitBAT(p_bat_decoder, false);
        assert(p_bat_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeBATSection
 *****************************************************************************
 * BAT decoder.
 * p_bat as the output parameter
 * p_section as the input parameter
 * similar to dvbpsi_DecodeNITSection
 *****************************************************************************/
void dvbpsi_bat_sections_decode(dvbpsi_bat_t* p_bat,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t* p_byte, * p_end;

    while(p_section)
    {
        /* - first loop descriptors */
        p_byte = p_section->p_payload_start + 2;
        p_end = p_byte + (((uint16_t)(p_section->p_payload_start[0] & 0x0f) << 8)
                          | p_section->p_payload_start[1]);
        if (p_end > p_section->p_payload_end)
            p_end = p_section->p_payload_end;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if (i_length + 2 <= p_end - p_byte)
                dvbpsi_bat_bouquet_descriptor_add(p_bat, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }

        p_end = 2 + p_byte + (((uint16_t)(p_byte[0] & 0x0f) << 8) | p_byte[1]);
        if (p_end > p_section->p_payload_end)
            p_end = p_section->p_payload_end;

        /* - TSs */
        p_byte += 2;
        while(p_byte + 6 <= p_end)
        {
            uint8_t *p_end2;
            uint16_t i_ts_id = ((uint16_t)p_byte[0] << 8) | p_byte[1];
            uint16_t i_orig_network_id = ((uint16_t)p_byte[2] << 8) | p_byte[3];
            uint16_t i_transport_descriptors_length = ((uint16_t)(p_byte[4] & 0x0f) << 8) | p_byte[5];

            dvbpsi_bat_ts_t* p_ts = dvbpsi_bat_ts_add(p_bat, i_ts_id, i_orig_network_id);
            if (!p_ts)
                break;

            /* - TS descriptors */
            p_byte += 6;
            p_end2 = p_byte + i_transport_descriptors_length;
            if (p_end2 > p_section->p_payload_end)
                p_end2 = p_section->p_payload_end;

            while (p_byte + 2 <= p_end2)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_length = p_byte[1];
                if (i_length + 2 <= p_end2 - p_byte)
                    dvbpsi_bat_ts_descriptor_add(p_ts, i_tag, i_length, p_byte + 2);
                p_byte += 2 + i_length;
            }
        }

        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_bat_sections_generate
 *****************************************************************************
 * Generate BAT sections based on the dvbpsi_bat_t structure.
 * similar to dvbpsi_nit_sections_generate
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_bat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_bat_t* p_bat)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_descriptor_t* p_descriptor = p_bat->p_first_descriptor;
    dvbpsi_bat_ts_t* p_ts = p_bat->p_first_ts;
    uint16_t i_bouquet_descriptors_length, i_transport_stream_loop_length;
    uint8_t * p_transport_stream_loop_length;

    if (p_current == NULL)
    {
        dvbpsi_error(p_dvbpsi, "BAT encoder", "failed to allocate new PSI section");
        return NULL;
    }

    p_current->i_table_id = 0x4a;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = true;
    p_current->i_length = 13;                     /* including CRC_32 */
    p_current->i_extension = p_bat->i_extension;
    p_current->i_version = p_bat->i_version;
    p_current->b_current_next = p_bat->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 10;
    p_current->p_payload_start = p_current->p_data + 8;

    /* first loop descriptors */
    while (p_descriptor != NULL)
    {
        /* New section if needed */
        /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
        if(   (p_current->p_payload_end - p_current->p_data)
                                + p_descriptor->i_length > 1018)
        {
            /* bouquet_descriptors_length */
            i_bouquet_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
            p_current->p_data[8] = (i_bouquet_descriptors_length >> 8) | 0xf0;
            p_current->p_data[9] = i_bouquet_descriptors_length;

            /* transport_stream_loop_length */
            p_current->p_payload_end[0] = 0;
            p_current->p_payload_end[1] = 0;
            p_current->p_payload_end += 2;

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            if (p_current ==  NULL)
            {
                dvbpsi_error(p_dvbpsi, "BAT encoder", "failed to allocate new PSI section");
                goto error;
            }
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x4a;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = true;
            p_current->i_length = 13;                 /* including CRC_32 */
            p_current->i_extension = p_bat->i_extension;
            p_current->i_version = p_bat->i_version;
            p_current->b_current_next = p_bat->b_current_next;
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

    /* bouquet_descriptors_length */
    i_bouquet_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
    p_current->p_data[8] = (i_bouquet_descriptors_length >> 8) | 0xf0;
    p_current->p_data[9] = i_bouquet_descriptors_length;

    /* Store the position of the transport_stream_loop_length field
       and reserve two bytes for it */
    p_transport_stream_loop_length = p_current->p_payload_end;
    p_current->p_payload_end += 2;

    /* second loop: BAT TSs */
    while (p_ts != NULL)
    {
        uint8_t* p_ts_start = p_current->p_payload_end;
        uint16_t i_transport_descriptors_length = 5;

        /* Can the current section carry all the descriptors ? */
        p_descriptor = p_ts->p_first_descriptor;
        while(    (p_descriptor != NULL)
               && ((p_ts_start - p_current->p_data) + i_transport_descriptors_length <= 1020))
        {
            i_transport_descriptors_length += p_descriptor->i_length + 2;
            p_descriptor = p_descriptor->p_next;
        }

        /* If _no_ and the current section isn't empty and an empty section
           may carry one more descriptor
           then create a new section */
        if(    (p_descriptor != NULL)
            && (p_ts_start - p_current->p_data != 12)
            && (i_transport_descriptors_length <= 1008))
        {
            /* transport_stream_loop_length */
            i_transport_stream_loop_length = (p_current->p_payload_end - p_transport_stream_loop_length) - 2;
            p_transport_stream_loop_length[0] = (i_transport_stream_loop_length >> 8) | 0xf0;
            p_transport_stream_loop_length[1] = i_transport_stream_loop_length;

            /* will put more descriptors in an empty section */
            dvbpsi_debug(p_dvbpsi, "BAT generator",
                        "create a new section to carry more TS descriptors");

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x4a;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = true;
            p_current->i_length = 13;                 /* including CRC_32 */
            p_current->i_extension = p_bat->i_extension;
            p_current->i_version = p_bat->i_version;
            p_current->b_current_next = p_bat->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 10;
            p_current->p_payload_start = p_current->p_data + 8;

            /* bouquet_descriptors_length = 0 */
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
        while(    (p_descriptor != NULL)
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
            dvbpsi_error(p_dvbpsi, "BAT generator", "unable to carry all the TS descriptors");

        /* transport_descriptors_length */
        i_transport_descriptors_length = p_current->p_payload_end - p_ts_start - 5;
        p_ts_start[4] = (i_transport_descriptors_length >> 8) | 0xf0;
        p_ts_start[5] = i_transport_descriptors_length;

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

error:
    /* Cleanup on error */
    p_prev = p_result;
    dvbpsi_DeletePSISections(p_prev);
    return NULL;
}
