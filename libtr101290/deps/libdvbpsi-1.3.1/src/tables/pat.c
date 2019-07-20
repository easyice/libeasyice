/*****************************************************************************
 * pat.c: PAT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2012 VideoLAN
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

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "pat.h"
#include "pat_private.h"

/*****************************************************************************
 * dvbpsi_pat_attach
 *****************************************************************************
 * Initialize a PAT decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_pat_attach(dvbpsi_t *p_dvbpsi, dvbpsi_pat_callback pf_callback,
                      void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder == NULL);

    /* PSI decoder configuration and initial state */
    dvbpsi_pat_decoder_t *p_pat_decoder;
    p_pat_decoder = (dvbpsi_pat_decoder_t*) dvbpsi_decoder_new(&dvbpsi_pat_sections_gather,
                                                1024, true, sizeof(dvbpsi_pat_decoder_t));
    if (p_pat_decoder == NULL)
        return false;

    /* PAT decoder information */
    p_pat_decoder->pf_pat_callback = pf_callback;
    p_pat_decoder->p_cb_data = p_cb_data;
    p_pat_decoder->p_building_pat = NULL;

    p_dvbpsi->p_decoder = DVBPSI_DECODER(p_pat_decoder);
    return true;
}

/*****************************************************************************
 * dvbpsi_pat_detach
 *****************************************************************************
 * Close a PAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_pat_detach(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_pat_decoder_t* p_pat_decoder = (dvbpsi_pat_decoder_t*)p_dvbpsi->p_decoder;
    if (p_pat_decoder->p_building_pat)
        dvbpsi_pat_delete(p_pat_decoder->p_building_pat);
    p_pat_decoder->p_building_pat = NULL;

    dvbpsi_decoder_delete(p_dvbpsi->p_decoder);
    p_dvbpsi->p_decoder = NULL;
}

/*****************************************************************************
 * dvbpsi_pat_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_pat_init(dvbpsi_pat_t* p_pat, uint16_t i_ts_id, uint8_t i_version,
                    bool b_current_next)
{
    assert(p_pat);

    p_pat->i_ts_id = i_ts_id;
    p_pat->i_version = i_version;
    p_pat->b_current_next = b_current_next;
    p_pat->p_first_program = NULL;
}

/*****************************************************************************
 * dvbpsi_pat_new
 *****************************************************************************
 * Allocate and Initialize a newly allocated dvbpsi_pat_t structure.
 *****************************************************************************/
dvbpsi_pat_t *dvbpsi_pat_new(uint16_t i_ts_id, uint8_t i_version,
                            bool b_current_next)
{
    dvbpsi_pat_t *p_pat = (dvbpsi_pat_t*)malloc(sizeof(dvbpsi_pat_t));
    if (p_pat)
        dvbpsi_pat_init(p_pat, i_ts_id, i_version, b_current_next);
    return p_pat;
}

/*****************************************************************************
 * dvbpsi_pat_empty
 *****************************************************************************
 * Clean a dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_pat_empty(dvbpsi_pat_t* p_pat)
{
    dvbpsi_pat_program_t* p_program = p_pat->p_first_program;

    while(p_program != NULL)
    {
        dvbpsi_pat_program_t* p_tmp = p_program->p_next;
        free(p_program);
        p_program = p_tmp;
    }
    p_pat->p_first_program = NULL;
}

/*****************************************************************************
 * dvbpsi_pat_delete
 *****************************************************************************
 * Clean and Delete dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_pat_delete(dvbpsi_pat_t *p_pat)
{
    if (p_pat)
        dvbpsi_pat_empty(p_pat);
    free(p_pat);
}

/*****************************************************************************
 * dvbpsi_pat_program_add
 *****************************************************************************
 * Add a program at the end of the PAT.
 *****************************************************************************/
dvbpsi_pat_program_t* dvbpsi_pat_program_add(dvbpsi_pat_t* p_pat,
                                             uint16_t i_number, uint16_t i_pid)
{
    dvbpsi_pat_program_t* p_program;

    /* PID = 0 is invalid for programs
     * FIXME: check for other fixed PIDs too. */
    if (i_pid == 0)
        return NULL;

    p_program = (dvbpsi_pat_program_t*) malloc(sizeof(dvbpsi_pat_program_t));
    if (p_program == NULL)
        return NULL;

    p_program->i_number = i_number;
    p_program->i_pid = i_pid;
    p_program->p_next = NULL;

    if (p_pat->p_first_program == NULL)
        p_pat->p_first_program = p_program;
    else
    {
        dvbpsi_pat_program_t* p_last_program = p_pat->p_first_program;
        while (p_last_program->p_next != NULL)
            p_last_program = p_last_program->p_next;
        p_last_program->p_next = p_program;
    }

    return p_program;
}

/* */
static void dvbpsi_ReInitPAT(dvbpsi_pat_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_pat)
            dvbpsi_pat_delete(p_decoder->p_building_pat);
    }
    p_decoder->p_building_pat = NULL;
}

static bool dvbpsi_CheckPAT(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi->p_decoder);

    dvbpsi_pat_decoder_t* p_pat_decoder;
    p_pat_decoder = (dvbpsi_pat_decoder_t *)p_dvbpsi->p_decoder;

    /* Perform a few sanity checks */
    if (p_pat_decoder->p_building_pat->i_ts_id != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "PAT decoder",
                        "'transport_stream_id' differs"
                        " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_pat_decoder->p_building_pat->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "PAT decoder",
                        "'version_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_pat_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "PAT decoder",
                        "'last_section_number' differs"
                        " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionPAT(dvbpsi_t *p_dvbpsi, dvbpsi_pat_decoder_t *p_pat_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_pat_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (p_pat_decoder->p_building_pat == NULL)
    {
        p_pat_decoder->p_building_pat = dvbpsi_pat_new(p_section->i_extension,
                              p_section->i_version, p_section->b_current_next);
        if (p_pat_decoder->p_building_pat == NULL)
            return false;

        p_pat_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_pat_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "PAT decoder", "overwrite section number %d",
                     p_section->i_number);
    return true;
}

/*****************************************************************************
 * dvbpsi_pat_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_pat_sections_gather(dvbpsi_t* p_dvbpsi, dvbpsi_psi_section_t* p_section)
{
    dvbpsi_pat_decoder_t* p_pat_decoder;

    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0x00, "PAT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Now we have a valid PAT section */
    p_pat_decoder = (dvbpsi_pat_decoder_t *)p_dvbpsi->p_decoder;

    /* TS discontinuity check */
    if (p_pat_decoder->b_discontinuity)
    {
        dvbpsi_ReInitPAT(p_pat_decoder, true);
        p_pat_decoder->b_discontinuity = false;
    }
    else
    {
        if (p_pat_decoder->p_building_pat)
        {
            if (dvbpsi_CheckPAT(p_dvbpsi, p_section))
                dvbpsi_ReInitPAT(p_pat_decoder, true);
        }
        else
        {
            if(    (p_pat_decoder->b_current_valid)
                && (p_pat_decoder->current_pat.i_version == p_section->i_version)
                && (p_pat_decoder->current_pat.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "PAT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
        }
    }

    /* Add section to PAT */
    if (!dvbpsi_AddSectionPAT(p_dvbpsi, p_pat_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "PAT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_pat_decoder)))
    {
        assert(p_pat_decoder->pf_pat_callback);

        /* Save the current information */
        p_pat_decoder->current_pat = *p_pat_decoder->p_building_pat;

        /* Decode the sections */
        if (dvbpsi_pat_sections_decode(p_pat_decoder->p_building_pat,
                                       p_pat_decoder->p_sections))
            p_pat_decoder->b_current_valid = true;

        /* signal the new PAT */
        if (p_pat_decoder->b_current_valid)
            p_pat_decoder->pf_pat_callback(p_pat_decoder->p_cb_data,
                                           p_pat_decoder->p_building_pat);

        /* Delete sectioins and Reinitialize the structures */
        dvbpsi_ReInitPAT(p_pat_decoder, !p_pat_decoder->b_current_valid);
        assert(p_pat_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_DecodePATSection
 *****************************************************************************
 * Decode sections into a PAT table. The function returns 'true' on success,
 * 'false' otherwise.
 *****************************************************************************/
bool dvbpsi_pat_sections_decode(dvbpsi_pat_t* p_pat, dvbpsi_psi_section_t* p_section)
{
    bool b_valid = false;
    while (p_section)
    {
        for (uint8_t *p_byte = p_section->p_payload_start;
            p_byte < p_section->p_payload_end;
            p_byte += 4)
        {
            uint16_t i_program_number = ((uint16_t)(p_byte[0]) << 8) | p_byte[1];
            uint16_t i_pid = ((uint16_t)(p_byte[2] & 0x1f) << 8) | p_byte[3];
            dvbpsi_pat_program_t* p_program = dvbpsi_pat_program_add(p_pat, i_program_number, i_pid);
            if (p_program)
                b_valid = true;
        }

        p_section = p_section->p_next;
    }
    return b_valid;
}

/*****************************************************************************
 * dvbpsi_pat_sections_generate
 *****************************************************************************
 * Generate PAT sections based on the dvbpsi_pat_t structure. The third
 * argument is used to limit the number of program in each section (max: 253).
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_pat_sections_generate(dvbpsi_t *p_dvbpsi,
                                            dvbpsi_pat_t* p_pat, int i_max_pps)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
    int i_count = 0;

    if (p_current == NULL)
    {
        dvbpsi_error(p_dvbpsi, "PAT encoder", "failed to allocate new PSI section");
        return NULL;
    }

    /* A PAT section can carry up to 253 programs */
    if((i_max_pps <= 0) || (i_max_pps > 253))
        i_max_pps = 253;

    p_current->i_table_id = 0;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = false;
    p_current->i_length = 9;                      /* header + CRC_32 */
    p_current->i_extension = p_pat->i_ts_id;
    p_current->i_version = p_pat->i_version;
    p_current->b_current_next = p_pat->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 8;                /* just after the header */
    p_current->p_payload_start = p_current->p_payload_end;

    /* PAT programs */
    while (p_program != NULL)
    {
        /* New section if needed */
        if (++i_count > i_max_pps)
        {
            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            if (p_current ==  NULL)
            {
                dvbpsi_error(p_dvbpsi, "PAT encoder", "failed to allocate new PSI section");
                goto error;
            }
            p_prev->p_next = p_current;
            i_count = 1;

            p_current->i_table_id = 0;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = false;
            p_current->i_length = 9;                  /* header + CRC_32 */
            p_current->i_extension = p_pat->i_ts_id;
            p_current->i_version = p_pat->i_version;
            p_current->b_current_next = p_pat->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 8;            /* just after the header */
            p_current->p_payload_start = p_current->p_payload_end;
        }

        /* p_payload_end is where the program begins */
        p_current->p_payload_end[0] = p_program->i_number >> 8;
        p_current->p_payload_end[1] = p_program->i_number;
        p_current->p_payload_end[2] = (p_program->i_pid >> 8) | 0xe0;
        p_current->p_payload_end[3] = p_program->i_pid;

        /* Increase length by 4 */
        p_current->p_payload_end += 4;
        p_current->i_length += 4;

        p_program = p_program->p_next;
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

error:
    /* Cleanup on error */
    p_prev = p_result;
    dvbpsi_DeletePSISections(p_prev);
    return NULL;
}
