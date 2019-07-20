/*****************************************************************************
 * tot.c: TDT/TOT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
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
#include "tot.h"
#include "tot_private.h"

/*****************************************************************************
 * dvbpsi_tot_attach
 *****************************************************************************
 * Initialize a TDT/TOT subtable decoder.
 *****************************************************************************/
bool dvbpsi_tot_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                       dvbpsi_tot_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    i_extension = 0; /* NOTE: force to 0 when handling TDT/TOT */
    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "TDT/TOT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_tot_decoder_t *p_tot_decoder;
    p_tot_decoder = (dvbpsi_tot_decoder_t *) dvbpsi_decoder_new(NULL,
                                                0, true, sizeof(dvbpsi_tot_decoder_t));
    if (p_tot_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_tot_detach,
                                         dvbpsi_tot_sections_gather, DVBPSI_DECODER(p_tot_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_tot_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* TDT/TOT decoder information */
    p_tot_decoder->pf_tot_callback = pf_callback;
    p_tot_decoder->p_cb_data = p_cb_data;
    p_tot_decoder->p_building_tot = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_tot_detach
 *****************************************************************************
 * Close a TDT/TOT decoder.
 *****************************************************************************/
void dvbpsi_tot_detach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id,
                      uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_decoder;
    dvbpsi_demux_subdec_t* p_subdec;

    i_extension = 0; /* NOTE: force to 0 when handling TDT/TOT */
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "TDT/TOT Decoder",
                     "No such TDT/TOT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return;
    }

    assert(p_subdec->p_decoder);

    dvbpsi_tot_decoder_t* p_tot_decoder;
    p_tot_decoder = (dvbpsi_tot_decoder_t*)p_subdec->p_decoder;
    if (p_tot_decoder->p_building_tot)
        dvbpsi_tot_delete(p_tot_decoder->p_building_tot);
    p_tot_decoder->p_building_tot = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_tot_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_tot_t structure.
 *****************************************************************************/
void dvbpsi_tot_init(dvbpsi_tot_t* p_tot, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next, uint64_t i_utc_time)
{
    assert(p_tot);

    p_tot->i_table_id = i_table_id;
    p_tot->i_extension = i_extension;

    p_tot->i_version = i_version;
    p_tot->b_current_next = b_current_next;

    p_tot->i_utc_time = i_utc_time;
    p_tot->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_tot_new
 *****************************************************************************
 * Allocate and Initialize a new dvbpsi_tot_t structure.
 *****************************************************************************/
dvbpsi_tot_t *dvbpsi_tot_new(uint8_t i_table_id, uint16_t i_extension, uint8_t i_version,
                             bool b_current_next, uint64_t i_utc_time)
{
  dvbpsi_tot_t *p_tot = (dvbpsi_tot_t*)malloc(sizeof(dvbpsi_tot_t));
  if (p_tot != NULL)
        dvbpsi_tot_init(p_tot, i_table_id, i_extension, i_version,
                        b_current_next, i_utc_time);
  return p_tot;
}

/*****************************************************************************
 * dvbpsi_tot_empty
 *****************************************************************************
 * Clean a dvbpsi_tot_t structure.
 *****************************************************************************/
void dvbpsi_tot_empty(dvbpsi_tot_t* p_tot)
{
    dvbpsi_DeleteDescriptors(p_tot->p_first_descriptor);
    p_tot->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_tot_delete
 *****************************************************************************
 * Clean and Delete dvbpsi_tot_t structure.
 *****************************************************************************/
void dvbpsi_tot_delete(dvbpsi_tot_t* p_tot)
{
    if (p_tot)
        dvbpsi_tot_empty(p_tot);
    free(p_tot);
}

/*****************************************************************************
 * dvbpsi_tot_descriptor_add
 *****************************************************************************
 * Add a descriptor in the TOT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_tot_descriptor_add(dvbpsi_tot_t* p_tot,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_tot->p_first_descriptor = dvbpsi_AddDescriptor(p_tot->p_first_descriptor,
                                                     p_descriptor);
    assert(p_tot->p_first_descriptor);
    if (p_tot->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/* */
static void dvbpsi_ReInitTOT(dvbpsi_tot_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_tot)
            dvbpsi_tot_delete(p_decoder->p_building_tot);
    }
    p_decoder->p_building_tot = NULL;
}

static bool dvbpsi_CheckTOT(dvbpsi_t *p_dvbpsi, dvbpsi_tot_decoder_t *p_tot_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi);
    assert(p_tot_decoder);

    if (p_tot_decoder->p_building_tot->i_extension != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "TDT/TOT decoder",
                "'transport_stream_id' differs"
                " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_tot_decoder->p_building_tot->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "TDT/TOT decoder",
                "'version_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_tot_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "TDT/TOT decoder",
                "'last_section_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionTOT(dvbpsi_t *p_dvbpsi, dvbpsi_tot_decoder_t *p_tot_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_tot_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_tot_decoder->p_building_tot)
    {
        p_tot_decoder->p_building_tot = dvbpsi_tot_new(
                             p_section->i_table_id, p_section->i_extension,
                             p_section->i_version, p_section->b_current_next,
                             ((uint64_t)p_section->p_payload_start[0] << 32)
                           | ((uint64_t)p_section->p_payload_start[1] << 24)
                           | ((uint64_t)p_section->p_payload_start[2] << 16)
                           | ((uint64_t)p_section->p_payload_start[3] <<  8)
                           |  (uint64_t)p_section->p_payload_start[4]);
        if (p_tot_decoder->p_building_tot == NULL)
            return false;
        p_tot_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_tot_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "TOT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_tot_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_tot_sections_gather(dvbpsi_t* p_dvbpsi,
                                dvbpsi_decoder_t* p_decoder,
                                dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    const uint8_t i_table_id = ((p_section->i_table_id == 0x70 ||  /* TDT */
                                 p_section->i_table_id == 0x73)) ? /* TOT */
                                    p_section->i_table_id : 0x70;

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, i_table_id, "TDT/TOT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Valid TDT/TOT section */
    dvbpsi_tot_decoder_t* p_tot_decoder = (dvbpsi_tot_decoder_t*)p_decoder;

    /* TS discontinuity check */
    if (p_tot_decoder->b_discontinuity)
    {
        /* We don't care about discontinuities with the TDT/TOT as it
           only consists of one section anyway */
        //dvbpsi_ReInitTOT(p_tot_decoder, true);
        p_tot_decoder->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_tot_decoder->p_building_tot)
        {
            if (dvbpsi_CheckTOT(p_dvbpsi, p_tot_decoder, p_section))
                dvbpsi_ReInitTOT(p_tot_decoder, true);
        }
#if 0
/* FIXME: Check TDT/TOT table definition for how the version numbering works for this table */
        else
        {
            if(    (p_tot_decoder->b_current_valid)
                && (p_tot_decoder->current_tot.i_version == p_section->i_version)
                && (p_tot_decoder->current_tot.b_current_next == p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "TOT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
        }
#endif
    }

    /* Add section to TOT */
    if (!dvbpsi_AddSectionTOT(p_dvbpsi, p_tot_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "TOT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_tot_decoder)))
    {
        assert(p_tot_decoder->pf_tot_callback);

        /* Save the current information */
        p_tot_decoder->current_tot = *p_tot_decoder->p_building_tot;
        p_tot_decoder->b_current_valid = true;

        /* Decode the sections */
        dvbpsi_tot_sections_decode(p_dvbpsi, p_tot_decoder->p_building_tot,
                                   p_tot_decoder->p_sections);
        /* signal the new TOT */
        p_tot_decoder->pf_tot_callback(p_tot_decoder->p_cb_data,
                                       p_tot_decoder->p_building_tot);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitTOT(p_tot_decoder, false);
        assert(p_tot_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_tot_section_valid
 *****************************************************************************
 * Check the CRC_32 if the section has b_syntax_indicator set.
 *****************************************************************************/
static bool dvbpsi_tot_section_valid(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t* p_section)
{
    /* TDT table */
    if (p_section->i_table_id == 0x70)
    {
        /* A TDT (table_id 0x70) always has a length of 5 bytes (which is only the UTC time) */
        if (p_section->i_length != 5)
        {
            dvbpsi_error(p_dvbpsi, "TDT decoder",
                         "TDT has an invalid payload size (%d bytes) !!!",
                          p_section->i_length);
            return false;
        }
    }

    /* CRC32 has already been checked by dvbpsi_packet_push()
     * and by dvbpsi_BuildPSISection(). */
    return true;
}

/*****************************************************************************
 * dvbpsi_tot_sections_decode
 *****************************************************************************
 * TDT/TOT decoder.
 *****************************************************************************/
void dvbpsi_tot_sections_decode(dvbpsi_t* p_dvbpsi, dvbpsi_tot_t* p_tot,
                                dvbpsi_psi_section_t* p_section)
{
    if (p_section)
    {
        uint8_t* p_byte;

        if (!dvbpsi_tot_section_valid(p_dvbpsi, p_section))
            return;

        /* points at first byte of UTC time */
        p_byte = p_section->p_payload_start;
        if (p_byte + 5 <= p_section->p_payload_end)
        {
            /* 16-bit MJD and 24 bits coded as 6 digits in 4-bit BCD */
            p_tot->i_utc_time = ((uint64_t)p_byte[0] << 32) |
                                ((uint64_t)p_byte[1] << 24) |
                                ((uint64_t)p_byte[2] << 16) |
                                ((uint64_t)p_byte[3] << 8) |
                                 (uint64_t)p_byte[4];
            p_byte += 5;
        }

        /* If we have a TOT, extract the descriptors */
        if (p_section->i_table_id == 0x73)
        {
            uint16_t i_loop_length;
            uint8_t* p_end;

            i_loop_length = (((uint16_t)(p_byte[0] & 0x0f) << 8) | (p_byte[1]));
            p_end = p_byte + i_loop_length;

            p_byte += 2; /* start of descriptors loop */
            while (p_byte + 2 <= p_end)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_length = p_byte[1];
                if (i_length + 2 <= p_section->p_payload_end - p_byte)
                    dvbpsi_tot_descriptor_add(p_tot, i_tag, i_length, p_byte + 2);
                p_byte += 2 + i_length;
            }
        }
    }
}

/*****************************************************************************
 * dvbpsi_tot_sections_generate
 *****************************************************************************
 * Generate TDT/TOT sections based on the dvbpsi_tot_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_tot_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_tot_t* p_tot)
{
    dvbpsi_psi_section_t* p_result;
    dvbpsi_descriptor_t* p_descriptor = p_tot->p_first_descriptor;

    /* If it has descriptors, it must be a TOT, otherwise a TDT */
    p_result = dvbpsi_NewPSISection((p_descriptor != NULL) ? 4096 : 8);

    p_result->i_table_id = (p_descriptor != NULL) ? 0x73 : 0x70;
    p_result->b_syntax_indicator = false;
    p_result->b_private_indicator = false;
    p_result->i_length = 5;
    p_result->p_payload_start = p_result->p_data + 3;
    p_result->p_payload_end = p_result->p_data + 8;

    p_result->p_data[3] = (p_tot->i_utc_time >> 32) & 0xff;
    p_result->p_data[4] = (p_tot->i_utc_time >> 24) & 0xff;
    p_result->p_data[5] = (p_tot->i_utc_time >> 16) & 0xff;
    p_result->p_data[6] = (p_tot->i_utc_time >>  8) & 0xff;
    p_result->p_data[7] =  p_tot->i_utc_time        & 0xff;

    if (p_result->i_table_id == 0x73)
    {
        /* Special handling for TOT only (A TDT doesn't have descriptors!) */
        /* Reserve two bytes for descriptors_loop_length */
        p_result->p_payload_end += 2;
        p_result->i_length += 2;

        /* TOT descriptors */
        while (p_descriptor != NULL)
        {
            /* A TOT cannot have multiple sections! */
            if(   (p_result->p_payload_end - p_result->p_data)
                                        + p_descriptor->i_length > 4090)
            {
                dvbpsi_error(p_dvbpsi, "TDT/TOT generator",
                             "TOT does not fit into one section as it ought to be !!!");
                break;
            }

            /* p_payload_end is where the descriptor begins */
            p_result->p_payload_end[0] = p_descriptor->i_tag;
            p_result->p_payload_end[1] = p_descriptor->i_length;
            memcpy(p_result->p_payload_end + 2,
                   p_descriptor->p_data,
                    p_descriptor->i_length);

            /* Increase length by descriptor_length + 2 */
            p_result->p_payload_end += p_descriptor->i_length + 2;
            p_result->i_length += p_descriptor->i_length + 2;

            p_descriptor = p_descriptor->p_next;
        }

        /* descriptors_loop_length */
        p_result->p_payload_start[5] = ((p_result->i_length - 7) << 8) | 0xf0;
        p_result->p_payload_start[6] =  (p_result->i_length - 7)       & 0xff;
    }

    /* Build the PSI section including the CRC32 on the playload.
     * NOTE: The p_payload_end pointer should point to the last byte
     * of the payload without the CRC32 field.
     */
    dvbpsi_BuildPSISection(p_dvbpsi, p_result);

    if (p_result->i_table_id == 0x73)
    {
        /* A TOT has a CRC_32 although it's a private section,
           but the CRC_32 is part of the payload! */
        p_result->p_payload_end += 4;
        p_result->i_length += 4;
    }

    if (!dvbpsi_tot_section_valid(p_dvbpsi, p_result))
    {
        dvbpsi_error(p_dvbpsi, "TDT/TOT generator", "********************************************");
        dvbpsi_error(p_dvbpsi, "TDT/TOT generator", "*  Generated TDT/TOT section is invalid.   *");
        dvbpsi_error(p_dvbpsi, "TDT/TOT generator", "* THIS IS A BUG, PLEASE REPORT TO THE LIST *");
        dvbpsi_error(p_dvbpsi, "TDT/TOT generator", "*  ---  libdvbpsi-devel@videolan.org  ---  *");
        dvbpsi_error(p_dvbpsi, "TDT/TOT generator", "********************************************");
    }

    return p_result;
}
