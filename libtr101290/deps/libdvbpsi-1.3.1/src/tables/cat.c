/*****************************************************************************
 * cat.c: CAT decoder/generator
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
#include "cat.h"
#include "cat_private.h"

/*****************************************************************************
 * dvbpsi_cat_attach
 *****************************************************************************
 * Initialize a CAT decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_cat_attach(dvbpsi_t *p_dvbpsi, dvbpsi_cat_callback pf_callback,
                      void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder == NULL);

    dvbpsi_cat_decoder_t* p_cat_decoder;
    p_cat_decoder = (dvbpsi_cat_decoder_t*) dvbpsi_decoder_new(&dvbpsi_cat_sections_gather,
                                                1024, true, sizeof(dvbpsi_cat_decoder_t));
    if (p_cat_decoder == NULL)
        return false;

    /* CAT decoder configuration */
    p_cat_decoder->pf_cat_callback = pf_callback;
    p_cat_decoder->p_cb_data = p_cb_data;
    p_cat_decoder->p_building_cat = NULL;

    p_dvbpsi->p_decoder = DVBPSI_DECODER(p_cat_decoder);
    return true;
}

/*****************************************************************************
 * dvbpsi_cat_detach
 *****************************************************************************
 * Close a CAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_cat_detach(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_cat_decoder_t* p_cat_decoder
                        = (dvbpsi_cat_decoder_t*)p_dvbpsi->p_decoder;
    if (p_cat_decoder->p_building_cat)
        dvbpsi_cat_delete(p_cat_decoder->p_building_cat);
    p_cat_decoder->p_building_cat = NULL;

    dvbpsi_decoder_delete(p_dvbpsi->p_decoder);
    p_dvbpsi->p_decoder = NULL;
}

/*****************************************************************************
 * dvbpsi_cat_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_cat_t structure.
 *****************************************************************************/
void dvbpsi_cat_init(dvbpsi_cat_t* p_cat, uint8_t i_version, bool b_current_next)
{
    assert(p_cat);

    p_cat->i_version = i_version;
    p_cat->b_current_next = b_current_next;
    p_cat->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_cat_new
 *****************************************************************************
 * Allocate and Initialize a dvbpsi_cat_t structure.
 *****************************************************************************/
dvbpsi_cat_t *dvbpsi_cat_new(uint8_t i_version, bool b_current_next)
{
    dvbpsi_cat_t *p_cat = (dvbpsi_cat_t*)malloc(sizeof(dvbpsi_cat_t));
    if (p_cat != NULL)
        dvbpsi_cat_init(p_cat, i_version, b_current_next);
    return p_cat;
}

/*****************************************************************************
 * dvbpsi_cat_empty
 *****************************************************************************
 * Clean a dvbpsi_cat_t structure.
 *****************************************************************************/
void dvbpsi_cat_empty(dvbpsi_cat_t* p_cat)
{
    dvbpsi_DeleteDescriptors(p_cat->p_first_descriptor);
    p_cat->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_cat_delete
 *****************************************************************************
 * Clean a dvbpsi_cat_t structure.
 *****************************************************************************/
void dvbpsi_cat_delete(dvbpsi_cat_t *p_cat)
{
    if (p_cat)
        dvbpsi_cat_empty(p_cat);
    free(p_cat);
}

/*****************************************************************************
 * dvbpsi_cat_descriptor_add
 *****************************************************************************
 * Add a descriptor in the CAT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_cat_descriptor_add(dvbpsi_cat_t* p_cat,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data)
{
    dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_cat->p_first_descriptor = dvbpsi_AddDescriptor(p_cat->p_first_descriptor,
                                                     p_descriptor);
    assert(p_cat->p_first_descriptor);
    if (p_cat->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/* */
static void dvbpsi_ReInitCAT(dvbpsi_cat_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_cat)
            dvbpsi_cat_delete(p_decoder->p_building_cat);
    }
    p_decoder->p_building_cat = NULL;
}

static bool dvbpsi_CheckCAT(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi->p_decoder);

    dvbpsi_cat_decoder_t* p_cat_decoder;
    p_cat_decoder = (dvbpsi_cat_decoder_t *)p_dvbpsi->p_decoder;

    /* Perform a few sanity checks */
#if 0
    if (p_pat_decoder->p_building_pat->i_ts_id != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "PAT decoder",
                        "'transport_stream_id' differs"
                        " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else
#endif
    if (p_cat_decoder->p_building_cat->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "CAT decoder",
                        "'version_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_cat_decoder->i_last_section_number != p_section->i_last_number)
    {
         /* last_section_number */
         dvbpsi_error(p_dvbpsi, "CAT decoder",
                        "'last_section_number' differs"
                        " whereas no discontinuity has occured");
         b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionCAT(dvbpsi_t *p_dvbpsi, dvbpsi_cat_decoder_t *p_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (p_decoder->p_building_cat == NULL)
    {
        p_decoder->p_building_cat = dvbpsi_cat_new(p_section->i_version,
                                                   p_section->b_current_next);
        if (p_decoder->p_building_cat == NULL)
            return false;

        p_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "CAT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_cat_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_cat_sections_gather(dvbpsi_t *p_dvbpsi,
                              dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0x01, "CAT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_cat_decoder_t* p_cat_decoder
                          = (dvbpsi_cat_decoder_t*)p_dvbpsi->p_decoder;

    /* TS discontinuity check */
    if (p_cat_decoder->b_discontinuity)
    {
        dvbpsi_ReInitCAT(p_cat_decoder, true);
        p_cat_decoder->b_discontinuity = false;
    }
    else
    {
        /* Perform some few sanity checks */
        if (p_cat_decoder->p_building_cat)
        {
            if (dvbpsi_CheckCAT(p_dvbpsi, p_section))
                dvbpsi_ReInitCAT(p_cat_decoder, true);
        }
        else
        {
             if (   (p_cat_decoder->b_current_valid)
                 && (p_cat_decoder->current_cat.i_version == p_section->i_version)
                 && (p_cat_decoder->current_cat.b_current_next ==
                                                   p_section->b_current_next))
             {
                 /* Don't decode since this version is already decoded */
                 dvbpsi_debug(p_dvbpsi, "CAT decoder",
                              "ignoring already decoded section %d",
                              p_section->i_number);
                 dvbpsi_DeletePSISections(p_section);
                 return;
             }
        }
    }

    /* Add section to CAT */
    if (!dvbpsi_AddSectionCAT(p_dvbpsi, p_cat_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "CAT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_cat_decoder)))
    {
        assert(p_cat_decoder->pf_cat_callback);

        /* Save the current information */
        p_cat_decoder->current_cat = *p_cat_decoder->p_building_cat;
        p_cat_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_cat_sections_decode(p_cat_decoder->p_building_cat,
                                   p_cat_decoder->p_sections);
        /* signal the new CAT */
        p_cat_decoder->pf_cat_callback(p_cat_decoder->p_cb_data,
                                       p_cat_decoder->p_building_cat);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitCAT(p_cat_decoder, false);
        assert(p_cat_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_cat_sections_decode
 *****************************************************************************
 * CAT decoder.
 *****************************************************************************/
void dvbpsi_cat_sections_decode(dvbpsi_cat_t* p_cat, dvbpsi_psi_section_t* p_section)
{
    uint8_t* p_byte;

    while (p_section)
    {
        /* CAT descriptors */
        p_byte = p_section->p_payload_start;
        while (p_byte <= p_section->p_payload_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_length = p_byte[1];
            if (i_length + 2 <= p_section->p_payload_end - p_byte)
                dvbpsi_cat_descriptor_add(p_cat, i_tag, i_length, p_byte + 2);
            p_byte += 2 + i_length;
        }
        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_cat_sections_generate
 *****************************************************************************
 * Generate CAT sections based on the dvbpsi_cat_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_cat_sections_generate(dvbpsi_t* p_dvbpsi, dvbpsi_cat_t* p_cat)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_descriptor_t* p_descriptor = p_cat->p_first_descriptor;

    p_current->i_table_id = 0x01;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = false;
    p_current->i_length = 9;                      /* header + CRC_32 */
    p_current->i_extension = 0;                   /* Not used in the CAT */
    p_current->i_version = p_cat->i_version;
    p_current->b_current_next = p_cat->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 8;                /* just after the header */
    p_current->p_payload_start = p_current->p_data + 8;

    /* CAT descriptors */
    while (p_descriptor != NULL)
    {
        /* New section if needed */
        /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
        if(   (p_current->p_payload_end - p_current->p_data)
                                + p_descriptor->i_length > 1018)
        {
            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x01;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 9;                  /* header + CRC_32 */
            p_current->i_extension = 0;               /* Not used in the CAT */
            p_current->i_version = p_cat->i_version;
            p_current->b_current_next = p_cat->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 8;            /* just after the header */
            p_current->p_payload_start = p_current->p_data + 8;
        }

        /* p_payload_end is where the descriptor begins */
        p_current->p_payload_end[0] = p_descriptor->i_tag;
        p_current->p_payload_end[1] = p_descriptor->i_length;
        memcpy(p_current->p_payload_end + 2,
                p_descriptor->p_data, p_descriptor->i_length);

        /* Increase length by descriptor_length + 2 */
        p_current->p_payload_end += p_descriptor->i_length + 2;
        p_current->i_length += p_descriptor->i_length + 2;

        p_descriptor = p_descriptor->p_next;
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
