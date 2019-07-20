/*****************************************************************************
 * pmt.c: PMT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
#include "pmt.h"
#include "pmt_private.h"

/*****************************************************************************
 * dvbpsi_pmt_attach
 *****************************************************************************
 * Initialize a PMT decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_pmt_attach(dvbpsi_t *p_dvbpsi, uint16_t i_program_number,
                      dvbpsi_pmt_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder == NULL);

    dvbpsi_pmt_decoder_t* p_pmt_decoder;
    p_pmt_decoder = (dvbpsi_pmt_decoder_t*) dvbpsi_decoder_new(&dvbpsi_pmt_sections_gather,
                                            1024, true, sizeof(dvbpsi_pmt_decoder_t));
    if (p_pmt_decoder == NULL)
        return false;

    p_dvbpsi->p_decoder = DVBPSI_DECODER(p_pmt_decoder);

    /* PMT decoder configuration */
    p_pmt_decoder->i_program_number = i_program_number;
    p_pmt_decoder->pf_pmt_callback = pf_callback;
    p_pmt_decoder->p_cb_data = p_cb_data;
    p_pmt_decoder->p_building_pmt = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_pmt_detach
 *****************************************************************************
 * Close a PMT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_pmt_detach(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_pmt_decoder_t* p_pmt_decoder;
    p_pmt_decoder = (dvbpsi_pmt_decoder_t*)p_dvbpsi->p_decoder;
    if (p_pmt_decoder->p_building_pmt)
        dvbpsi_pmt_delete(p_pmt_decoder->p_building_pmt);
    p_pmt_decoder->p_building_pmt = NULL;

    dvbpsi_decoder_delete(p_dvbpsi->p_decoder);
    p_dvbpsi->p_decoder = NULL;
}

/*****************************************************************************
 * dvbpsi_pmt_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_pmt_init(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, bool b_current_next, uint16_t i_pcr_pid)
{
    assert(p_pmt);

    p_pmt->i_program_number = i_program_number;
    p_pmt->i_version = i_version;
    p_pmt->b_current_next = b_current_next;
    p_pmt->i_pcr_pid = i_pcr_pid;
    p_pmt->p_first_descriptor = NULL;
    p_pmt->p_first_es = NULL;
}

/*****************************************************************************
 * dvbpsi_pmt_new
 *****************************************************************************
 * Allocate and Initialize a new dvbpsi_pmt_t structure.
 *****************************************************************************/
dvbpsi_pmt_t* dvbpsi_pmt_new(uint16_t i_program_number, uint8_t i_version,
                            bool b_current_next, uint16_t i_pcr_pid)
{
    dvbpsi_pmt_t *p_pmt = (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));
    if(p_pmt != NULL)
        dvbpsi_pmt_init(p_pmt, i_program_number, i_version,
                        b_current_next, i_pcr_pid);
    return p_pmt;
}

/*****************************************************************************
 * dvbpsi_pmt_empty
 *****************************************************************************
 * Clean a dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_pmt_empty(dvbpsi_pmt_t* p_pmt)
{
    dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;

    dvbpsi_DeleteDescriptors(p_pmt->p_first_descriptor);

    while(p_es != NULL)
    {
        dvbpsi_pmt_es_t* p_tmp = p_es->p_next;
        dvbpsi_DeleteDescriptors(p_es->p_first_descriptor);
        free(p_es);
        p_es = p_tmp;
    }

    p_pmt->p_first_descriptor = NULL;
    p_pmt->p_first_es = NULL;
}

/*****************************************************************************
 * dvbpsi_pmt_delete
 *****************************************************************************
 * Clean a dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_pmt_delete(dvbpsi_pmt_t* p_pmt)
{
    if (p_pmt)
        dvbpsi_pmt_empty(p_pmt);
    free(p_pmt);
}

/*****************************************************************************
 * dvbpsi_pmt_descriptor_add
 *****************************************************************************
 * Add a descriptor in the PMT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_pmt_descriptor_add(dvbpsi_pmt_t* p_pmt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor;
    p_descriptor = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_pmt->p_first_descriptor = dvbpsi_AddDescriptor(p_pmt->p_first_descriptor,
                                                     p_descriptor);
    assert(p_pmt->p_first_descriptor);
    if (p_pmt->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_pmt_es_add
 *****************************************************************************
 * Add an ES in the PMT.
 *****************************************************************************/
dvbpsi_pmt_es_t* dvbpsi_pmt_es_add(dvbpsi_pmt_t* p_pmt,
                                   uint8_t i_type, uint16_t i_pid)
{
    dvbpsi_pmt_es_t* p_es = (dvbpsi_pmt_es_t*)malloc(sizeof(dvbpsi_pmt_es_t));
    if (p_es == NULL)
        return NULL;

    p_es->i_type = i_type;
    p_es->i_pid = i_pid;
    p_es->p_first_descriptor = NULL;
    p_es->p_next = NULL;

    if (p_pmt->p_first_es == NULL)
       p_pmt->p_first_es = p_es;
    else
    {
        dvbpsi_pmt_es_t* p_last_es = p_pmt->p_first_es;
        while (p_last_es->p_next != NULL)
            p_last_es = p_last_es->p_next;
        p_last_es->p_next = p_es;
    }
    return p_es;
}

/*****************************************************************************
 * dvbpsi_pmt_es_descriptor_add
 *****************************************************************************
 * Add a descriptor in the PMT ES.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_pmt_es_descriptor_add(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor;
    p_descriptor = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    if (p_es->p_first_descriptor == NULL)
        p_es->p_first_descriptor = p_descriptor;
    else
    {
        dvbpsi_descriptor_t* p_last_descriptor = p_es->p_first_descriptor;
        while(p_last_descriptor->p_next != NULL)
            p_last_descriptor = p_last_descriptor->p_next;
        p_last_descriptor->p_next = p_descriptor;
    }
    return p_descriptor;
}

/* */
static void dvbpsi_ReInitPMT(dvbpsi_pmt_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_pmt)
            dvbpsi_pmt_delete(p_decoder->p_building_pmt);
    }
    p_decoder->p_building_pmt = NULL;
}

static bool dvbpsi_CheckPMT(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi->p_decoder);

    dvbpsi_pmt_decoder_t* p_pmt_decoder;
    p_pmt_decoder = (dvbpsi_pmt_decoder_t *)p_dvbpsi->p_decoder;

    if (p_pmt_decoder->p_building_pmt->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "PMT decoder",
                        "'version_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_pmt_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "PMT decoder",
                        "'last_section_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionPMT(dvbpsi_t *p_dvbpsi, dvbpsi_pmt_decoder_t *p_pmt_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_pmt_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (p_pmt_decoder->p_building_pmt == NULL)
    {
        p_pmt_decoder->p_building_pmt = dvbpsi_pmt_new(p_pmt_decoder->i_program_number,
                              p_section->i_version, p_section->b_current_next,
                              ((uint16_t)(p_section->p_payload_start[0] & 0x1f) << 8)
                                          | p_section->p_payload_start[1]);
        if (p_pmt_decoder->p_building_pmt == NULL)
            return false;

        p_pmt_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_pmt_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "PMT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_GatherPMTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_pmt_sections_gather(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0x02, "PMT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_pmt_decoder_t* p_pmt_decoder = (dvbpsi_pmt_decoder_t*)p_dvbpsi->p_decoder;
    assert(p_pmt_decoder);

    /* We have a valid PMT section */
    if (p_pmt_decoder->i_program_number != p_section->i_extension)
    {
        /* Invalid program_number */
        dvbpsi_debug(p_dvbpsi, "PMT decoder", "ignoring section %d not belonging to 'program_number' %d",
                     p_section->i_extension, p_pmt_decoder->i_program_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_pmt_decoder->b_discontinuity)
    {
        dvbpsi_ReInitPMT(p_pmt_decoder, true);
        p_pmt_decoder->b_discontinuity = false;
    }
    else
    {
        /* Perform some few sanity checks */
        if (p_pmt_decoder->p_building_pmt)
        {
            if (dvbpsi_CheckPMT(p_dvbpsi, p_section))
                dvbpsi_ReInitPMT(p_pmt_decoder, true);
        }
        else
        {
            if(    (p_pmt_decoder->b_current_valid)
                && (p_pmt_decoder->current_pmt.i_version == p_section->i_version)
                && (p_pmt_decoder->current_pmt.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "PMT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
        }
    }

    /* Add section to PMT */
    if (!dvbpsi_AddSectionPMT(p_dvbpsi, p_pmt_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "PMT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_pmt_decoder)))
    {
        assert(p_pmt_decoder->pf_pmt_callback);

        /* Save the current information */
        p_pmt_decoder->current_pmt = *p_pmt_decoder->p_building_pmt;
        p_pmt_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_pmt_sections_decode(p_pmt_decoder->p_building_pmt,
                                   p_pmt_decoder->p_sections);
        /* signal the new PMT */
        p_pmt_decoder->pf_pmt_callback(p_pmt_decoder->p_cb_data,
                                       p_pmt_decoder->p_building_pmt);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitPMT(p_pmt_decoder, false);
        assert(p_pmt_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_pmt_sections_decode
 *****************************************************************************
 * PMT decoder.
 *****************************************************************************/
void dvbpsi_pmt_sections_decode(dvbpsi_pmt_t* p_pmt,
                                dvbpsi_psi_section_t* p_section)
{
    uint8_t* p_byte, * p_end;

    while (p_section)
    {
        /* - PMT descriptors */
        p_byte = p_section->p_payload_start + 4;
        p_end = p_byte + (   ((uint16_t)(p_section->p_payload_start[2] & 0x0f) << 8)
                           | p_section->p_payload_start[3]);
        while (p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if (i_length + 2 <= p_end - p_byte)
                dvbpsi_pmt_descriptor_add(p_pmt, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }

        /* - ESs */
        for (p_byte = p_end; p_byte + 5 <= p_section->p_payload_end;)
        {
            uint8_t i_type = p_byte[0];
            uint16_t i_pid = ((uint16_t)(p_byte[1] & 0x1f) << 8) | p_byte[2];
            uint16_t i_es_length = ((uint16_t)(p_byte[3] & 0x0f) << 8) | p_byte[4];
            dvbpsi_pmt_es_t* p_es = dvbpsi_pmt_es_add(p_pmt, i_type, i_pid);
            /* - ES descriptors */
            p_byte += 5;
            p_end = p_byte + i_es_length;
            if (p_end > p_section->p_payload_end)
            {
                p_end = p_section->p_payload_end;
            }
            while (p_byte + 2 <= p_end)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_length = p_byte[1];
                if (i_length + 2 <= p_end - p_byte)
                    dvbpsi_pmt_es_descriptor_add(p_es, i_tag, i_length, p_byte + 2);
                p_byte += 2 + i_length;
            }
        }
        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_pmt_sections_generate
 *****************************************************************************
 * Generate PMT sections based on the dvbpsi_pmt_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_pmt_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_pmt_t* p_pmt)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_descriptor_t* p_descriptor = p_pmt->p_first_descriptor;
    dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
    uint16_t i_info_length;

    p_current->i_table_id = 0x02;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = false;
    p_current->i_length = 13;                     /* header + CRC_32 */
    p_current->i_extension = p_pmt->i_program_number;
    p_current->i_version = p_pmt->i_version;
    p_current->b_current_next = p_pmt->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 12;               /* just after the header */
    p_current->p_payload_start = p_current->p_data + 8;

    /* PCR_PID */
    p_current->p_data[8] = (p_pmt->i_pcr_pid >> 8) | 0xe0;
    p_current->p_data[9] = p_pmt->i_pcr_pid;

    /* PMT descriptors */
    while (p_descriptor != NULL)
    {
        /* New section if needed */
        /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
        if ((p_current->p_payload_end - p_current->p_data)
                                      + p_descriptor->i_length > 1018)
        {
            /* program_info_length */
            i_info_length = (p_current->p_payload_end - p_current->p_data) - 12;
            p_current->p_data[10] = (i_info_length >> 8) | 0xf0;
            p_current->p_data[11] = i_info_length;

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x02;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 13;                 /* header + CRC_32 */
            p_current->i_extension = p_pmt->i_program_number;
            p_current->i_version = p_pmt->i_version;
            p_current->b_current_next = p_pmt->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 12;           /* just after the header */
            p_current->p_payload_start = p_current->p_data + 8;

            /* PCR_PID */
            p_current->p_data[8] = (p_pmt->i_pcr_pid >> 8) | 0xe0;
            p_current->p_data[9] = p_pmt->i_pcr_pid;
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

    /* program_info_length */
    i_info_length = (p_current->p_payload_end - p_current->p_data) - 12;
    p_current->p_data[10] = (i_info_length >> 8) | 0xf0;
    p_current->p_data[11] = i_info_length;

    /* PMT ESs */
    while (p_es != NULL)
    {
        uint8_t* p_es_start = p_current->p_payload_end;
        uint16_t i_es_length = 5;

        /* Can the current section carry all the descriptors ? */
        p_descriptor = p_es->p_first_descriptor;
        while(    (p_descriptor != NULL)
               && ((p_es_start - p_current->p_data) + i_es_length <= 1020))
        {
            i_es_length += p_descriptor->i_length + 2;
            p_descriptor = p_descriptor->p_next;
        }

        /* If _no_ and the current section isn't empty and an empty section
           may carry one more descriptor
           then create a new section */
        if(    (p_descriptor != NULL)
            && (p_es_start - p_current->p_data != 12)
            && (i_es_length <= 1008))
        {
            /* will put more descriptors in an empty section */
            dvbpsi_debug(p_dvbpsi, "PMT generator",
                         "create a new section to carry more ES descriptors");

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x02;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 13;                 /* header + CRC_32 */
            p_current->i_extension = p_pmt->i_program_number;
            p_current->i_version = p_pmt->i_version;
            p_current->b_current_next = p_pmt->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 12;           /* just after the header */
            p_current->p_payload_start = p_current->p_data + 8;

            /* PCR_PID */
            p_current->p_data[8] = (p_pmt->i_pcr_pid >> 8) | 0xe0;
            p_current->p_data[9] = p_pmt->i_pcr_pid;

            /* program_info_length */
            i_info_length = 0;
            p_current->p_data[10] = 0xf0;
            p_current->p_data[11] = 0x00;

            p_es_start = p_current->p_payload_end;
        }

        /* p_es_start is where the ES begins */
        p_es_start[0] = p_es->i_type;
        p_es_start[1] = (p_es->i_pid >> 8) | 0xe0;
        p_es_start[2] = p_es->i_pid;

        /* Increase the length by 5 */
        p_current->p_payload_end += 5;
        p_current->i_length += 5;

        /* ES descriptors */
        p_descriptor = p_es->p_first_descriptor;
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
            dvbpsi_error(p_dvbpsi, "PMT generator", "unable to carry all the ES descriptors");

        /* ES_info_length */
        i_es_length = p_current->p_payload_end - p_es_start - 5;
        p_es_start[3] = (i_es_length >> 8) | 0xf0;
        p_es_start[4] = i_es_length;

        p_es = p_es->p_next;
    }

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
