/*****************************************************************************
 * pat.c: PAT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "pat.h"
#include "pat_private.h"


/*****************************************************************************
 * dvbpsi_AttachPAT
 *****************************************************************************
 * Initialize a PAT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPAT(dvbpsi_pat_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  dvbpsi_pat_decoder_t* p_pat_decoder;

  if(h_dvbpsi == NULL)
    return NULL;

  p_pat_decoder = (dvbpsi_pat_decoder_t*)malloc(sizeof(dvbpsi_pat_decoder_t));

  if(p_pat_decoder == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }

  /* PSI decoder configuration */
  h_dvbpsi->pf_callback = &dvbpsi_GatherPATSections;
  h_dvbpsi->p_private_decoder = p_pat_decoder;
  h_dvbpsi->i_section_max_size = 1024;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* PAT decoder information */
  p_pat_decoder->pf_callback = pf_callback;
  p_pat_decoder->p_cb_data = p_cb_data;
  /* PAT decoder initial state */
  p_pat_decoder->b_current_valid = 0;
  p_pat_decoder->p_building_pat = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_pat_decoder->ap_sections[i] = NULL;

  return h_dvbpsi;
}


/*****************************************************************************
 * dvbpsi_DetachPAT
 *****************************************************************************
 * Close a PAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_DetachPAT(dvbpsi_handle h_dvbpsi)
{
  dvbpsi_pat_decoder_t* p_pat_decoder
                        = (dvbpsi_pat_decoder_t*)h_dvbpsi->p_private_decoder;

  free(p_pat_decoder->p_building_pat);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_pat_decoder->ap_sections[i])
      free(p_pat_decoder->ap_sections[i]);
  }

  free(h_dvbpsi->p_private_decoder);
  if(h_dvbpsi->p_current_section)
    dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
  free(h_dvbpsi);
}


/*****************************************************************************
 * dvbpsi_InitPAT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_InitPAT(dvbpsi_pat_t* p_pat, uint16_t i_ts_id, uint8_t i_version,
                    int b_current_next)
{
  p_pat->i_ts_id = i_ts_id;
  p_pat->i_version = i_version;
  p_pat->b_current_next = b_current_next;
  p_pat->p_first_program = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptyPAT
 *****************************************************************************
 * Clean a dvbpsi_pat_t structure.
 *****************************************************************************/
void dvbpsi_EmptyPAT(dvbpsi_pat_t* p_pat)
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
 * dvbpsi_PATAddProgram
 *****************************************************************************
 * Add a program at the end of the PAT.
 *****************************************************************************/
dvbpsi_pat_program_t* dvbpsi_PATAddProgram(dvbpsi_pat_t* p_pat,
                                           uint16_t i_number, uint16_t i_pid)
{
  dvbpsi_pat_program_t* p_program
                = (dvbpsi_pat_program_t*)malloc(sizeof(dvbpsi_pat_program_t));

  if(p_program)
  {
    p_program->i_number = i_number;
    p_program->i_pid = i_pid;
    p_program->p_next = NULL;

    if(p_pat->p_first_program == NULL)
    {
      p_pat->p_first_program = p_program;
    }
    else
    {
      dvbpsi_pat_program_t* p_last_program = p_pat->p_first_program;
      while(p_last_program->p_next != NULL)
        p_last_program = p_last_program->p_next;
      p_last_program->p_next = p_program;
    }
  }

  return p_program;
}


/*****************************************************************************
 * dvbpsi_GatherPATSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_GatherPATSections(dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section)
{
  dvbpsi_pat_decoder_t* p_pat_decoder
                        = (dvbpsi_pat_decoder_t*)p_decoder->p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("PAT decoder",
                   "Table version %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(p_section->i_table_id != 0x00)
  {
    /* Invalid table_id value */
    DVBPSI_ERROR_ARG("PAT decoder",
                     "invalid section (table_id == 0x%02x)",
                     p_section->i_table_id);
    b_append = 0;
  }

  if(b_append && !p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("PAT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid PAT section */
  if(b_append)
  {
    /* TS discontinuity check */
    if(p_decoder->b_discontinuity)
    {
      b_reinit = 1;
      p_decoder->b_discontinuity = 0;
    }
    else
    {
      /* Perform a few sanity checks */
      if(p_pat_decoder->p_building_pat)
      {
        if(p_pat_decoder->p_building_pat->i_ts_id != p_section->i_extension)
        {
          /* transport_stream_id */
          DVBPSI_ERROR("PAT decoder",
                       "'transport_stream_id' differs"
                       " whereas no TS discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_pat_decoder->p_building_pat->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("PAT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_pat_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("PAT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_pat_decoder->b_current_valid)
            && (p_pat_decoder->current_pat.i_version == p_section->i_version)
            && (p_pat_decoder->current_pat.b_current_next ==
                                           p_section->b_current_next))
        {
          /* Don't decode since this version is already decoded */
          b_append = 0;
        }
      }
    }
  }

  /* Reinit the decoder if wanted */
  if(b_reinit)
  {
    /* Force redecoding */
    p_pat_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_pat_decoder->p_building_pat)
    {
      free(p_pat_decoder->p_building_pat);
      p_pat_decoder->p_building_pat = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_pat_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_pat_decoder->ap_sections[i]);
        p_pat_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_pat_decoder->p_building_pat)
    {
      p_pat_decoder->p_building_pat =
                                (dvbpsi_pat_t*)malloc(sizeof(dvbpsi_pat_t));
      dvbpsi_InitPAT(p_pat_decoder->p_building_pat,
                     p_section->i_extension,
                     p_section->i_version,
                     p_section->b_current_next);
      p_pat_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_pat_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("PAT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_pat_decoder->ap_sections[p_section->i_number]);
    }
    p_pat_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(unsigned int i = 0; i <= p_pat_decoder->i_last_section_number; i++)
    {
      if(!p_pat_decoder->ap_sections[i])
        break;

      if(i == p_pat_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_pat_decoder->current_pat = *p_pat_decoder->p_building_pat;
      p_pat_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_pat_decoder->i_last_section_number)
      {
        for(unsigned int i = 0; (int)i <= p_pat_decoder->i_last_section_number - 1; i++)
          p_pat_decoder->ap_sections[i]->p_next =
                                        p_pat_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_DecodePATSections(p_pat_decoder->p_building_pat,
                               p_pat_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_pat_decoder->ap_sections[0]);
      /* signal the new PAT */
      p_pat_decoder->pf_callback(p_pat_decoder->p_cb_data,
                                 p_pat_decoder->p_building_pat);
      /* Reinitialize the structures */
      p_pat_decoder->p_building_pat = NULL;
      for(unsigned int i = 0; i <= p_pat_decoder->i_last_section_number; i++)
        p_pat_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodePATSection
 *****************************************************************************
 * PAT decoder.
 *****************************************************************************/
void dvbpsi_DecodePATSections(dvbpsi_pat_t* p_pat,
                               dvbpsi_psi_section_t* p_section)
{
  while(p_section)
  {
    for(uint8_t *p_byte = p_section->p_payload_start;
        p_byte < p_section->p_payload_end;
        p_byte += 4)
    {
      uint16_t i_program_number = ((uint16_t)(p_byte[0]) << 8) | p_byte[1];
      uint16_t i_pid = ((uint16_t)(p_byte[2] & 0x1f) << 8) | p_byte[3];
      dvbpsi_PATAddProgram(p_pat, i_program_number, i_pid);
    }

    p_section = p_section->p_next;
  }
}


/*****************************************************************************
 * dvbpsi_GenPATSections
 *****************************************************************************
 * Generate PAT sections based on the dvbpsi_pat_t structure. The third
 * argument is used to limit the number of program in each section (max: 253).
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenPATSections(dvbpsi_pat_t* p_pat,
                                            int i_max_pps)
{
  dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
  dvbpsi_psi_section_t* p_current = p_result;
  dvbpsi_psi_section_t* p_prev;
  dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
  int i_count = 0;

  /* A PAT section can carry up to 253 programs */
  if((i_max_pps <= 0) || (i_max_pps > 253))
    i_max_pps = 253;

  p_current->i_table_id = 0;
  p_current->b_syntax_indicator = 1;
  p_current->b_private_indicator = 0;
  p_current->i_length = 9;                      /* header + CRC_32 */
  p_current->i_extension = p_pat->i_ts_id;
  p_current->i_version = p_pat->i_version;
  p_current->b_current_next = p_pat->b_current_next;
  p_current->i_number = 0;
  p_current->p_payload_end += 8;                /* just after the header */
  p_current->p_payload_start = p_current->p_payload_end;

  /* PAT programs */
  while(p_program != NULL)
  {
    /* New section if needed */
    if(++i_count > i_max_pps)
    {
      p_prev = p_current;
      p_current = dvbpsi_NewPSISection(1024);
      p_prev->p_next = p_current;
      i_count = 1;

      p_current->i_table_id = 0;
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
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
  while(p_prev != NULL)
  {
    p_prev->i_last_number = p_current->i_number;
    dvbpsi_BuildPSISection(p_prev);
    p_prev = p_prev->p_next;
  }

  return p_result;
}

