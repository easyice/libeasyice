/*****************************************************************************
 * eit.c: EIT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2004-2010 VideoLAN
 * $Id: eit.c 88 2004-02-24 14:31:18Z sam $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
 *          Johan Bilien <jobi@via.ecp.fr>
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
#include "../demux.h"
#include "eit.h"
#include "eit_private.h"


/*****************************************************************************
 * dvbpsi_AttachEIT
 *****************************************************************************
 * Initialize a EIT subtable decoder.
 *****************************************************************************/
int dvbpsi_AttachEIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_eit_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_eit_decoder_t*  p_eit_decoder;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("EIT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);

    return 1;
  }

  p_subdec = (dvbpsi_demux_subdec_t*)malloc(sizeof(dvbpsi_demux_subdec_t));
  if(p_subdec == NULL)
  {
    return 1;
  }

  p_eit_decoder = (dvbpsi_eit_decoder_t*)malloc(sizeof(dvbpsi_eit_decoder_t));

  if(p_eit_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_GatherEITSections;
  p_subdec->p_cb_data = p_eit_decoder;
  p_subdec->i_id = (uint32_t)i_table_id << 16 | (uint32_t)i_extension;
  p_subdec->pf_detach = dvbpsi_DetachEIT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* EIT decoder information */
  p_eit_decoder->pf_callback = pf_callback;
  p_eit_decoder->p_cb_data = p_cb_data;
  /* EIT decoder initial state */
  p_eit_decoder->b_current_valid = 0;
  p_eit_decoder->p_building_eit = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_eit_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_DetachEIT
 *****************************************************************************
 * Close a EIT decoder.
 *****************************************************************************/
void dvbpsi_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_eit_decoder_t* p_eit_decoder;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("EIT Decoder",
                     "No such EIT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
    return;
  }

  p_eit_decoder = (dvbpsi_eit_decoder_t*)p_subdec->p_cb_data;

  free(p_eit_decoder->p_building_eit);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_eit_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_InitEIT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_eit_t structure.
 *****************************************************************************/
void dvbpsi_InitEIT(dvbpsi_eit_t* p_eit, uint16_t i_service_id, uint8_t i_version,
                    int b_current_next, uint16_t i_ts_id, uint16_t i_network_id,
                    uint8_t i_segment_last_section_number,
                    uint8_t i_last_table_id)
{
  p_eit->i_service_id = i_service_id;
  p_eit->i_version = i_version;
  p_eit->b_current_next = b_current_next;
  p_eit->i_ts_id = i_ts_id;
  p_eit->i_network_id = i_network_id;
  p_eit->i_segment_last_section_number = i_segment_last_section_number;
  p_eit->i_last_table_id = i_last_table_id;
  p_eit->p_first_event = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptyEIT
 *****************************************************************************
 * Clean a dvbpsi_eit_t structure.
 *****************************************************************************/
void dvbpsi_EmptyEIT(dvbpsi_eit_t* p_eit)
{
  dvbpsi_eit_event_t* p_event = p_eit->p_first_event;

  while(p_event != NULL)
  {
    dvbpsi_eit_event_t* p_tmp = p_event->p_next;
    dvbpsi_DeleteDescriptors(p_event->p_first_descriptor);
    free(p_event);
    p_event = p_tmp;
  }

  p_eit->p_first_event = NULL;
}


/*****************************************************************************
 * dvbpsi_EITAddEvent
 *****************************************************************************
 * Add an event description at the end of the EIT.
 *****************************************************************************/
dvbpsi_eit_event_t* dvbpsi_EITAddEvent(dvbpsi_eit_t* p_eit,
    uint16_t i_event_id, uint64_t i_start_time, uint32_t i_duration,
    uint8_t i_running_status,int b_free_ca)
{
  dvbpsi_eit_event_t* p_event
                = (dvbpsi_eit_event_t*)malloc(sizeof(dvbpsi_eit_event_t));

  if(p_event)
  {
    p_event->i_event_id = i_event_id;
    p_event->i_start_time = i_start_time;
    p_event->i_duration = i_duration;
    p_event->i_running_status = i_running_status;
    p_event->b_free_ca = b_free_ca;
    p_event->p_next = NULL;
    p_event->p_first_descriptor = NULL;

    if(p_eit->p_first_event == NULL)
    {
      p_eit->p_first_event = p_event;
    }
    else
    {
      dvbpsi_eit_event_t* p_last_event = p_eit->p_first_event;
      while(p_last_event->p_next != NULL)
        p_last_event = p_last_event->p_next;
      p_last_event->p_next = p_event;
    }
  }

  return p_event;
}


/*****************************************************************************
 * dvbpsi_EITEventAddDescriptor
 *****************************************************************************
 * Add a descriptor in the EIT event description.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_EITEventAddDescriptor(
                                               dvbpsi_eit_event_t* p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_event->p_first_descriptor == NULL)
    {
      p_event->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_event->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_GatherEITSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_GatherEITSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_eit_decoder_t* p_eit_decoder
                        = (dvbpsi_eit_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("EIT decoder",
                   "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_table_id,
                   p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("EIT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid EIT section */
  if(b_append)
  {
    /* TS discontinuity check */
    if(p_psi_decoder->b_discontinuity)
    {
      b_reinit = 1;
      p_psi_decoder->b_discontinuity = 0;
    }
    else
    {
      /* Perform a few sanity checks */
      if(p_eit_decoder->p_building_eit)
      {
        if(p_eit_decoder->p_building_eit->i_service_id != p_section->i_extension)
        {
          /* service_id */
          DVBPSI_ERROR("EIT decoder",
                       "'service_id' differs"
                       " whereas no TS discontinuity has occurred");
          b_reinit = 1;
        }
        else if(p_eit_decoder->p_building_eit->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("EIT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occurred");
          b_reinit = 1;
        }
        else if(p_eit_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("EIT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_eit_decoder->b_current_valid)
            && (p_eit_decoder->current_eit.i_version == p_section->i_version)
            && (p_eit_decoder->current_eit.b_current_next ==
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
    p_eit_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_eit_decoder->p_building_eit)
    {
      free(p_eit_decoder->p_building_eit);
      p_eit_decoder->p_building_eit = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_eit_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[i]);
        p_eit_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_eit_decoder->p_building_eit)
    {
      p_eit_decoder->p_building_eit =
                                (dvbpsi_eit_t*)malloc(sizeof(dvbpsi_eit_t));
      dvbpsi_InitEIT(p_eit_decoder->p_building_eit,
                     p_section->i_extension,
                     p_section->i_version,
                     p_section->b_current_next,
                     ((uint16_t)(p_section->p_payload_start[0]) << 8)
                     | p_section->p_payload_start[1],
                     ((uint16_t)(p_section->p_payload_start[2]) << 8)
                     | p_section->p_payload_start[3],
                     p_section->p_payload_start[4],
                     p_section->p_payload_start[5]);
      p_eit_decoder->i_last_section_number = p_section->i_last_number;
      p_eit_decoder->i_first_received_section_number = p_section->i_number;
    }

    /* Fill the section array */
    if(p_eit_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("EIT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[p_section->i_number]);
    }
    p_eit_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;

    /* As there may be gaps in the section_number fields (see below), we
     * have to wait until we have received a section_number twice or
     * until we have a received a section_number which is
     * first_received_section_number - 1;
     * if the first_received_section_number is 0, it's enough to wait
     * until the last_section_number has been received;
     * this is the only way to be sure that a complete table has been
     * sent! */
    if((p_eit_decoder->i_first_received_section_number > 0 &&
        (p_section->i_number ==
           p_eit_decoder->i_first_received_section_number ||
         p_section->i_number ==
           p_eit_decoder->i_first_received_section_number - 1)) ||
       (p_eit_decoder->i_first_received_section_number == 0 &&
        p_section->i_number == p_eit_decoder->i_last_section_number))
    {
      for(unsigned int i = 0; i <= p_eit_decoder->i_last_section_number; i++)
      {
        if(!p_eit_decoder->ap_sections[i])
          break;

        if(i == p_eit_decoder->i_last_section_number)
        {
          b_complete = 1;
          break;
        }

        /* ETSI EN 300 468 V1.5.1 section 5.2.4 says that the EIT
         * sections may be structured into a number of segments and
         * that there may be a gap in the section_number between
         * two segments (but not within a single segment); thus at
         * the end of a segment (indicated by
         * section_number == segment_last_section_number)
         * we have to search for the beginning of the next segment) */
        if(i == p_eit_decoder->ap_sections[i]->p_payload_start[4])
        {
          while(!p_eit_decoder->ap_sections[i + 1] &&
                (i + 1 < p_eit_decoder->i_last_section_number))
          {
            i++;
          }
        }
      }
    }

    if(b_complete)
    {
      /* Save the current information */
      p_eit_decoder->current_eit = *p_eit_decoder->p_building_eit;
      p_eit_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_eit_decoder->i_last_section_number)
      {
        dvbpsi_psi_section_t * p_prev_section;

        p_prev_section = p_eit_decoder->ap_sections[0];
        for(unsigned int i = 1; i <= p_eit_decoder->i_last_section_number; i++)
        {
          if(p_eit_decoder->ap_sections[i] != NULL)
          {
            p_prev_section->p_next = p_eit_decoder->ap_sections[i];
            p_prev_section = p_eit_decoder->ap_sections[i];
          }
        }
      }
      /* Decode the sections */
      dvbpsi_DecodeEITSections(p_eit_decoder->p_building_eit,
                               p_eit_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_eit_decoder->ap_sections[0]);
      /* signal the new EIT */
      p_eit_decoder->pf_callback(p_eit_decoder->p_cb_data,
                                 p_eit_decoder->p_building_eit);
      /* Reinitialize the structures */
      p_eit_decoder->p_building_eit = NULL;
      for(unsigned int i = 0; i <= p_eit_decoder->i_last_section_number; i++)
        p_eit_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeEITSections
 *****************************************************************************
 * EIT decoder.
 *****************************************************************************/
void dvbpsi_DecodeEITSections(dvbpsi_eit_t* p_eit,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t* p_byte, * p_end;

  while(p_section)
  {
    for(p_byte = p_section->p_payload_start + 6;
        p_byte < p_section->p_payload_end - 12;)
    {
      uint16_t i_event_id = ((uint16_t)(p_byte[0]) << 8) | p_byte[1];
      uint64_t i_start_time = ((uint64_t)(p_byte[2]) << 32)
                               | ((uint64_t)(p_byte[3]) << 24)
                               | ((uint64_t)(p_byte[4]) << 16)
                               | ((uint64_t)(p_byte[5]) << 8) | p_byte[6];
      uint32_t i_duration = ((uint32_t)(p_byte[7]) << 16)
                               | ((uint32_t)(p_byte[8]) << 8) | p_byte[9];
      uint8_t i_running_status = (uint8_t)(p_byte[10]) >> 5;
      int b_free_ca = (int)(p_byte[10] & 0x10) >> 4;
      uint16_t i_ev_length = ((uint16_t)(p_byte[10] & 0xf) << 8) | p_byte[11];
      dvbpsi_eit_event_t* p_event = dvbpsi_EITAddEvent(p_eit,
          i_event_id, i_start_time, i_duration,
          i_running_status, b_free_ca);
      /* Event descriptors */
      p_byte += 12;
      p_end = p_byte + i_ev_length;
      while(p_byte + 2 <= p_end)
      {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_EITEventAddDescriptor(p_event, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
      }
    }

    p_section = p_section->p_next;
  }
}
