/*****************************************************************************
 * cat.c: CAT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "../descriptor.h"
#include "cat.h"
#include "cat_private.h"


/*****************************************************************************
 * dvbpsi_AttachCAT
 *****************************************************************************
 * Initialize a CAT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachCAT(dvbpsi_cat_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  dvbpsi_cat_decoder_t* p_cat_decoder;

  if(h_dvbpsi == NULL)
    return NULL;

  p_cat_decoder = (dvbpsi_cat_decoder_t*)malloc(sizeof(dvbpsi_cat_decoder_t));

  if(p_cat_decoder == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }

  /* PSI decoder configuration */
  h_dvbpsi->pf_callback = &dvbpsi_GatherCATSections;
  h_dvbpsi->p_private_decoder = p_cat_decoder;
  h_dvbpsi->i_section_max_size = 1024;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* CAT decoder configuration */
  p_cat_decoder->pf_callback = pf_callback;
  p_cat_decoder->p_cb_data = p_cb_data;
  /* CAT decoder initial state */
  p_cat_decoder->b_current_valid = 0;
  p_cat_decoder->p_building_cat = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_cat_decoder->ap_sections[i] = NULL;

  return h_dvbpsi;
}


/*****************************************************************************
 * dvbpsi_DetachCAT
 *****************************************************************************
 * Close a CAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_DetachCAT(dvbpsi_handle h_dvbpsi)
{
  dvbpsi_cat_decoder_t* p_cat_decoder
                        = (dvbpsi_cat_decoder_t*)h_dvbpsi->p_private_decoder;

  free(p_cat_decoder->p_building_cat);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_cat_decoder->ap_sections[i])
      free(p_cat_decoder->ap_sections[i]);
  }

  free(h_dvbpsi->p_private_decoder);
  if(h_dvbpsi->p_current_section)
    dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
  free(h_dvbpsi);
}


/*****************************************************************************
 * dvbpsi_InitCAT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_cat_t structure.
 *****************************************************************************/
void dvbpsi_InitCAT(dvbpsi_cat_t* p_cat,
                    uint8_t i_version, int b_current_next)
{
  p_cat->i_version = i_version;
  p_cat->b_current_next = b_current_next;
  p_cat->p_first_descriptor = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptyCAT
 *****************************************************************************
 * Clean a dvbpsi_cat_t structure.
 *****************************************************************************/
void dvbpsi_EmptyCAT(dvbpsi_cat_t* p_cat)
{
  dvbpsi_DeleteDescriptors(p_cat->p_first_descriptor);

  p_cat->p_first_descriptor = NULL;
}


/*****************************************************************************
 * dvbpsi_CATAddDescriptor
 *****************************************************************************
 * Add a descriptor in the CAT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_CATAddDescriptor(dvbpsi_cat_t* p_cat,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_cat->p_first_descriptor == NULL)
    {
      p_cat->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_cat->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_GatherCATSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_GatherCATSections(dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section)
{
  dvbpsi_cat_decoder_t* p_cat_decoder
                        = (dvbpsi_cat_decoder_t*)p_decoder->p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("CAT decoder",
                   "Table version %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(p_section->i_table_id != 0x01)
  {
    /* Invalid table_id value */
    DVBPSI_ERROR_ARG("CAT decoder",
                     "invalid section (table_id == 0x%02x)",
                     p_section->i_table_id);
    b_append = 0;
  }

  if(b_append && !p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("CAT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

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
      /* Perform some few sanity checks */
      if(p_cat_decoder->p_building_cat)
      {
        if(p_cat_decoder->p_building_cat->i_version != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("CAT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_cat_decoder->i_last_section_number
                                                != p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("CAT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_cat_decoder->b_current_valid)
            && (p_cat_decoder->current_cat.i_version == p_section->i_version)
            && (p_cat_decoder->current_cat.b_current_next ==
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
    p_cat_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_cat_decoder->p_building_cat)
    {
      free(p_cat_decoder->p_building_cat);
      p_cat_decoder->p_building_cat = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_cat_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_cat_decoder->ap_sections[i]);
        p_cat_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_cat_decoder->p_building_cat)
    {
      p_cat_decoder->p_building_cat =
                                (dvbpsi_cat_t*)malloc(sizeof(dvbpsi_cat_t));
      dvbpsi_InitCAT(p_cat_decoder->p_building_cat,
                     p_section->i_version,
                     p_section->b_current_next);
      p_cat_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_cat_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("CAT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_cat_decoder->ap_sections[p_section->i_number]);
    }
    p_cat_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(unsigned int i = 0; i <= p_cat_decoder->i_last_section_number; i++)
    {
      if(!p_cat_decoder->ap_sections[i])
        break;

      if(i == p_cat_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_cat_decoder->current_cat = *p_cat_decoder->p_building_cat;
      p_cat_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_cat_decoder->i_last_section_number)
      {
        for(unsigned int i = 0; (int)i <= p_cat_decoder->i_last_section_number - 1; i++)
          p_cat_decoder->ap_sections[i]->p_next =
                                        p_cat_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_DecodeCATSections(p_cat_decoder->p_building_cat,
                               p_cat_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_cat_decoder->ap_sections[0]);
      /* signal the new CAT */
      p_cat_decoder->pf_callback(p_cat_decoder->p_cb_data,
                                 p_cat_decoder->p_building_cat);
      /* Reinitialize the structures */
      p_cat_decoder->p_building_cat = NULL;
      for(unsigned int i = 0; i <= p_cat_decoder->i_last_section_number; i++)
        p_cat_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeCATSections
 *****************************************************************************
 * CAT decoder.
 *****************************************************************************/
void dvbpsi_DecodeCATSections(dvbpsi_cat_t* p_cat,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t* p_byte;

  while(p_section)
  {
    /* CAT descriptors */
    p_byte = p_section->p_payload_start;
    while(p_byte + 5 <= p_section->p_payload_end)
    {
      uint8_t i_tag = p_byte[0];
      uint8_t i_length = p_byte[1];
      if(i_length + 2 <= p_section->p_payload_end - p_byte)
        dvbpsi_CATAddDescriptor(p_cat, i_tag, i_length, p_byte + 2);
      p_byte += 2 + i_length;
    }

    p_section = p_section->p_next;
  }
}


/*****************************************************************************
 * dvbpsi_GenCATSections
 *****************************************************************************
 * Generate CAT sections based on the dvbpsi_cat_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenCATSections(dvbpsi_cat_t* p_cat)
{
  dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
  dvbpsi_psi_section_t* p_current = p_result;
  dvbpsi_psi_section_t* p_prev;
  dvbpsi_descriptor_t* p_descriptor = p_cat->p_first_descriptor;

  p_current->i_table_id = 0x01;
  p_current->b_syntax_indicator = 1;
  p_current->b_private_indicator = 0;
  p_current->i_length = 9;                      /* header + CRC_32 */
  p_current->i_extension = 0;                   /* Not used in the CAT */
  p_current->i_version = p_cat->i_version;
  p_current->b_current_next = p_cat->b_current_next;
  p_current->i_number = 0;
  p_current->p_payload_end += 8;                /* just after the header */
  p_current->p_payload_start = p_current->p_data + 8;

  /* CAT descriptors */
  while(p_descriptor != NULL)
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
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
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
           p_descriptor->p_data,
           p_descriptor->i_length);

    /* Increase length by descriptor_length + 2 */
    p_current->p_payload_end += p_descriptor->i_length + 2;
    p_current->i_length += p_descriptor->i_length + 2;

    p_descriptor = p_descriptor->p_next;
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

