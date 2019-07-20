/*****************************************************************************
 * pmt.c: PMT decoder/generator
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
#include "pmt.h"
#include "pmt_private.h"


/*****************************************************************************
 * dvbpsi_AttachPMT
 *****************************************************************************
 * Initialize a PMT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachPMT(uint16_t i_program_number,
                               dvbpsi_pmt_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  dvbpsi_pmt_decoder_t* p_pmt_decoder;

  if(h_dvbpsi == NULL)
    return NULL;

  p_pmt_decoder = (dvbpsi_pmt_decoder_t*)malloc(sizeof(dvbpsi_pmt_decoder_t));

  if(p_pmt_decoder == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }

  /* PSI decoder configuration */
  h_dvbpsi->pf_callback = &dvbpsi_GatherPMTSections;
  h_dvbpsi->p_private_decoder = p_pmt_decoder;
  h_dvbpsi->i_section_max_size = 1024;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* PMT decoder configuration */
  p_pmt_decoder->i_program_number = i_program_number;
  p_pmt_decoder->pf_callback = pf_callback;
  p_pmt_decoder->p_cb_data = p_cb_data;
  /* PMT decoder initial state */
  p_pmt_decoder->b_current_valid = 0;
  p_pmt_decoder->p_building_pmt = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_pmt_decoder->ap_sections[i] = NULL;

  return h_dvbpsi;
}


/*****************************************************************************
 * dvbpsi_DetachPMT
 *****************************************************************************
 * Close a PMT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_DetachPMT(dvbpsi_handle h_dvbpsi)
{
  dvbpsi_pmt_decoder_t* p_pmt_decoder
                        = (dvbpsi_pmt_decoder_t*)h_dvbpsi->p_private_decoder;

  free(p_pmt_decoder->p_building_pmt);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_pmt_decoder->ap_sections[i])
      free(p_pmt_decoder->ap_sections[i]);
  }

  free(h_dvbpsi->p_private_decoder);
  if(h_dvbpsi->p_current_section)
    dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
  free(h_dvbpsi);
}


/*****************************************************************************
 * dvbpsi_InitPMT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_InitPMT(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, int b_current_next, uint16_t i_pcr_pid)
{
  p_pmt->i_program_number = i_program_number;
  p_pmt->i_version = i_version;
  p_pmt->b_current_next = b_current_next;
  p_pmt->i_pcr_pid = i_pcr_pid;
  p_pmt->p_first_descriptor = NULL;
  p_pmt->p_first_es = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptyPMT
 *****************************************************************************
 * Clean a dvbpsi_pmt_t structure.
 *****************************************************************************/
void dvbpsi_EmptyPMT(dvbpsi_pmt_t* p_pmt)
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
 * dvbpsi_PMTAddDescriptor
 *****************************************************************************
 * Add a descriptor in the PMT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTAddDescriptor(dvbpsi_pmt_t* p_pmt,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_pmt->p_first_descriptor == NULL)
    {
      p_pmt->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_pmt->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_PMTAddES
 *****************************************************************************
 * Add an ES in the PMT.
 *****************************************************************************/
dvbpsi_pmt_es_t* dvbpsi_PMTAddES(dvbpsi_pmt_t* p_pmt,
                                 uint8_t i_type, uint16_t i_pid)
{
  dvbpsi_pmt_es_t* p_es = (dvbpsi_pmt_es_t*)malloc(sizeof(dvbpsi_pmt_es_t));

  if(p_es)
  {
    p_es->i_type = i_type;
    p_es->i_pid = i_pid;
    p_es->p_first_descriptor = NULL;
    p_es->p_next = NULL;

    if(p_pmt->p_first_es == NULL)
    {
      p_pmt->p_first_es = p_es;
    }
    else
    {
      dvbpsi_pmt_es_t* p_last_es = p_pmt->p_first_es;
      while(p_last_es->p_next != NULL)
        p_last_es = p_last_es->p_next;
      p_last_es->p_next = p_es;
    }
  }

  return p_es;
}


/*****************************************************************************
 * dvbpsi_PMTESAddDescriptor
 *****************************************************************************
 * Add a descriptor in the PMT ES.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_PMTESAddDescriptor(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_es->p_first_descriptor == NULL)
    {
      p_es->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_es->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_GatherPMTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_GatherPMTSections(dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section)
{
  dvbpsi_pmt_decoder_t* p_pmt_decoder
                        = (dvbpsi_pmt_decoder_t*)p_decoder->p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("PMT decoder",
                   "Table version %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(p_section->i_table_id != 0x02)
  {
    /* Invalid table_id value */
    DVBPSI_ERROR_ARG("PMT decoder",
                     "invalid section (table_id == 0x%02x)",
                     p_section->i_table_id);
    b_append = 0;
  }

  if(b_append && !p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("PMT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid PMT section */
  if(b_append && (p_pmt_decoder->i_program_number != p_section->i_extension))
  {
    /* Invalid program_number */
#if 0
    DVBPSI_ERROR("PMT decoder", \
                 "'program_number' don't match");
#endif
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
      if(p_pmt_decoder->p_building_pmt)
      {
        if(p_pmt_decoder->p_building_pmt->i_version != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("PMT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_pmt_decoder->i_last_section_number
                                                != p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("PMT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_pmt_decoder->b_current_valid)
            && (p_pmt_decoder->current_pmt.i_version == p_section->i_version)
            && (p_pmt_decoder->current_pmt.b_current_next ==
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
    p_pmt_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_pmt_decoder->p_building_pmt)
    {
      free(p_pmt_decoder->p_building_pmt);
      p_pmt_decoder->p_building_pmt = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_pmt_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_pmt_decoder->ap_sections[i]);
        p_pmt_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_pmt_decoder->p_building_pmt)
    {
      p_pmt_decoder->p_building_pmt =
                                (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));
      dvbpsi_InitPMT(p_pmt_decoder->p_building_pmt,
                     p_pmt_decoder->i_program_number,
                     p_section->i_version,
                     p_section->b_current_next,
                       ((uint16_t)(p_section->p_payload_start[0] & 0x1f) << 8)
                     | p_section->p_payload_start[1]);
      p_pmt_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_pmt_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("PMT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_pmt_decoder->ap_sections[p_section->i_number]);
    }
    p_pmt_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(unsigned int i = 0; i <= p_pmt_decoder->i_last_section_number; i++)
    {
      if(!p_pmt_decoder->ap_sections[i])
        break;

      if(i == p_pmt_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_pmt_decoder->current_pmt = *p_pmt_decoder->p_building_pmt;
      p_pmt_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_pmt_decoder->i_last_section_number)
      {
        for(unsigned int i = 0; (int)i <= p_pmt_decoder->i_last_section_number - 1; i++)
          p_pmt_decoder->ap_sections[i]->p_next =
                                        p_pmt_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_DecodePMTSections(p_pmt_decoder->p_building_pmt,
                               p_pmt_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_pmt_decoder->ap_sections[0]);
      /* signal the new PMT */
      p_pmt_decoder->pf_callback(p_pmt_decoder->p_cb_data,
                                 p_pmt_decoder->p_building_pmt);
      /* Reinitialize the structures */
      p_pmt_decoder->p_building_pmt = NULL;
      for(unsigned int i = 0; i <= p_pmt_decoder->i_last_section_number; i++)
        p_pmt_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodePMTSections
 *****************************************************************************
 * PMT decoder.
 *****************************************************************************/
void dvbpsi_DecodePMTSections(dvbpsi_pmt_t* p_pmt,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t* p_byte, * p_end;

  while(p_section)
  {
    /* - PMT descriptors */
    p_byte = p_section->p_payload_start + 4;
    p_end = p_byte + (   ((uint16_t)(p_section->p_payload_start[2] & 0x0f) << 8)
                       | p_section->p_payload_start[3]);
    while(p_byte + 2 <= p_end)
    {
      uint8_t i_tag = p_byte[0];
      uint8_t i_length = p_byte[1];
      if(i_length + 2 <= p_end - p_byte)
        dvbpsi_PMTAddDescriptor(p_pmt, i_tag, i_length, p_byte + 2);
      p_byte += 2 + i_length;
    }

    /* - ESs */
    for(p_byte = p_end; p_byte + 5 <= p_section->p_payload_end;)
    {
      uint8_t i_type = p_byte[0];
      uint16_t i_pid = ((uint16_t)(p_byte[1] & 0x1f) << 8) | p_byte[2];
      uint16_t i_es_length = ((uint16_t)(p_byte[3] & 0x0f) << 8) | p_byte[4];
      dvbpsi_pmt_es_t* p_es = dvbpsi_PMTAddES(p_pmt, i_type, i_pid);
      /* - ES descriptors */
      p_byte += 5;
      p_end = p_byte + i_es_length;
      if( p_end > p_section->p_payload_end )
      {
            p_end = p_section->p_payload_end;
      }
      while(p_byte + 2 <= p_end)
      {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_PMTESAddDescriptor(p_es, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
      }
    }

    p_section = p_section->p_next;
  }
}


/*****************************************************************************
 * dvbpsi_GenPMTSections
 *****************************************************************************
 * Generate PMT sections based on the dvbpsi_pmt_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenPMTSections(dvbpsi_pmt_t* p_pmt)
{
  dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
  dvbpsi_psi_section_t* p_current = p_result;
  dvbpsi_psi_section_t* p_prev;
  dvbpsi_descriptor_t* p_descriptor = p_pmt->p_first_descriptor;
  dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
  uint16_t i_info_length;

  p_current->i_table_id = 0x02;
  p_current->b_syntax_indicator = 1;
  p_current->b_private_indicator = 0;
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
  while(p_descriptor != NULL)
  {
    /* New section if needed */
    /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
    if(   (p_current->p_payload_end - p_current->p_data)
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
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
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
  while(p_es != NULL)
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
      DVBPSI_DEBUG("PMT generator",
                   "create a new section to carry more ES descriptors");
      p_prev = p_current;
      p_current = dvbpsi_NewPSISection(1024);
      p_prev->p_next = p_current;

      p_current->i_table_id = 0x02;
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
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

    if(p_descriptor != NULL)
      DVBPSI_ERROR("PMT generator", "unable to carry all the ES descriptors");

    /* ES_info_length */
    i_es_length = p_current->p_payload_end - p_es_start - 5;
    p_es_start[3] = (i_es_length >> 8) | 0xf0;
    p_es_start[4] = i_es_length;

    p_es = p_es->p_next;
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

