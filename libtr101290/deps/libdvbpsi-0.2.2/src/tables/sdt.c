/*****************************************************************************
 * sdt.c: SDT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
#include "sdt.h"
#include "sdt_private.h"


/*****************************************************************************
 * dvbpsi_AttachSDT
 *****************************************************************************
 * Initialize a SDT subtable decoder.
 *****************************************************************************/
int dvbpsi_AttachSDT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sdt_callback pf_callback,
                               void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_sdt_decoder_t*  p_sdt_decoder;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("SDT decoder",
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

  p_sdt_decoder = (dvbpsi_sdt_decoder_t*)malloc(sizeof(dvbpsi_sdt_decoder_t));

  if(p_sdt_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_GatherSDTSections;
  p_subdec->p_cb_data = p_sdt_decoder;
  p_subdec->i_id = (uint32_t)i_table_id << 16 | (uint32_t)i_extension;
  p_subdec->pf_detach = dvbpsi_DetachSDT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* SDT decoder information */
  p_sdt_decoder->pf_callback = pf_callback;
  p_sdt_decoder->p_cb_data = p_cb_data;
  /* SDT decoder initial state */
  p_sdt_decoder->b_current_valid = 0;
  p_sdt_decoder->p_building_sdt = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_sdt_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_DetachSDT
 *****************************************************************************
 * Close a SDT decoder.
 *****************************************************************************/
void dvbpsi_DetachSDT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_sdt_decoder_t* p_sdt_decoder;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("SDT Decoder",
                     "No such SDT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
    return;
  }

  p_sdt_decoder = (dvbpsi_sdt_decoder_t*)p_subdec->p_cb_data;

  free(p_sdt_decoder->p_building_sdt);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_sdt_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_sdt_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_InitSDT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_sdt_t structure.
 *****************************************************************************/
void dvbpsi_InitSDT(dvbpsi_sdt_t* p_sdt, uint16_t i_ts_id, uint8_t i_version,
                    int b_current_next, uint16_t i_network_id)
{
  p_sdt->i_ts_id = i_ts_id;
  p_sdt->i_version = i_version;
  p_sdt->b_current_next = b_current_next;
  p_sdt->i_network_id = i_network_id;
  p_sdt->p_first_service = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptySDT
 *****************************************************************************
 * Clean a dvbpsi_sdt_t structure.
 *****************************************************************************/
void dvbpsi_EmptySDT(dvbpsi_sdt_t* p_sdt)
{
  dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;

  while(p_service != NULL)
  {
    dvbpsi_sdt_service_t* p_tmp = p_service->p_next;
    dvbpsi_DeleteDescriptors(p_service->p_first_descriptor);
    free(p_service);
    p_service = p_tmp;
  }

  p_sdt->p_first_service = NULL;
}


/*****************************************************************************
 * dvbpsi_SDTAddService
 *****************************************************************************
 * Add a service description at the end of the SDT.
 *****************************************************************************/
dvbpsi_sdt_service_t *dvbpsi_SDTAddService(dvbpsi_sdt_t* p_sdt,
                                           uint16_t i_service_id,
                                           int b_eit_schedule,
                                           int b_eit_present,
                                           uint8_t i_running_status,
                                           int b_free_ca)
{
  dvbpsi_sdt_service_t * p_service
                = (dvbpsi_sdt_service_t*)malloc(sizeof(dvbpsi_sdt_service_t));

  if(p_service)
  {
    p_service->i_service_id = i_service_id;
    p_service->b_eit_schedule = b_eit_schedule;
    p_service->b_eit_present = b_eit_present;
    p_service->i_running_status = i_running_status;
    p_service->b_free_ca = b_free_ca;
    p_service->p_next = NULL;
    p_service->p_first_descriptor = NULL;

    if(p_sdt->p_first_service == NULL)
    {
      p_sdt->p_first_service = p_service;
    }
    else
    {
      dvbpsi_sdt_service_t * p_last_service = p_sdt->p_first_service;
      while(p_last_service->p_next != NULL)
        p_last_service = p_last_service->p_next;
      p_last_service->p_next = p_service;
    }
  }

  return p_service;
}


/*****************************************************************************
 * dvbpsi_SDTServiceAddDescriptor
 *****************************************************************************
 * Add a descriptor in the SDT service description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_SDTServiceAddDescriptor(
                                               dvbpsi_sdt_service_t *p_service,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_service->p_first_descriptor == NULL)
    {
      p_service->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_service->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_GatherSDTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_GatherSDTSections(dvbpsi_decoder_t * p_psi_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_sdt_decoder_t * p_sdt_decoder
                        = (dvbpsi_sdt_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("SDT decoder",
                   "Table version %2d, " "i_table_id %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_table_id,
                   p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(!p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("SDT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid SDT section */
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
      if(p_sdt_decoder->p_building_sdt)
      {
        if(p_sdt_decoder->p_building_sdt->i_ts_id != p_section->i_extension)
        {
          /* transport_stream_id */
          DVBPSI_ERROR("SDT decoder",
                       "'transport_stream_id' differs"
                       " whereas no TS discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_sdt_decoder->p_building_sdt->i_version
                                                != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("SDT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_sdt_decoder->i_last_section_number !=
                                                p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("SDT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_sdt_decoder->b_current_valid)
            && (p_sdt_decoder->current_sdt.i_version == p_section->i_version)
            && (p_sdt_decoder->current_sdt.b_current_next ==
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
    p_sdt_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_sdt_decoder->p_building_sdt)
    {
      free(p_sdt_decoder->p_building_sdt);
      p_sdt_decoder->p_building_sdt = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_sdt_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_sdt_decoder->ap_sections[i]);
        p_sdt_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_sdt_decoder->p_building_sdt)
    {
      p_sdt_decoder->p_building_sdt =
                                (dvbpsi_sdt_t*)malloc(sizeof(dvbpsi_sdt_t));
      dvbpsi_InitSDT(p_sdt_decoder->p_building_sdt,
                     p_section->i_extension,
                     p_section->i_version,
                     p_section->b_current_next,
                     ((uint16_t)(p_section->p_payload_start[0]) << 8)
                     | p_section->p_payload_start[1]);
      p_sdt_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_sdt_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("SDT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_sdt_decoder->ap_sections[p_section->i_number]);
    }
    p_sdt_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(unsigned int i = 0; i <= p_sdt_decoder->i_last_section_number; i++)
    {
      if(!p_sdt_decoder->ap_sections[i])
        break;

      if(i == p_sdt_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_sdt_decoder->current_sdt = *p_sdt_decoder->p_building_sdt;
      p_sdt_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_sdt_decoder->i_last_section_number)
      {
        for(unsigned int i = 0; (int)i <= p_sdt_decoder->i_last_section_number - 1; i++)
          p_sdt_decoder->ap_sections[i]->p_next =
                                        p_sdt_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_DecodeSDTSections(p_sdt_decoder->p_building_sdt,
                               p_sdt_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_sdt_decoder->ap_sections[0]);
      /* signal the new SDT */
      p_sdt_decoder->pf_callback(p_sdt_decoder->p_cb_data,
                                 p_sdt_decoder->p_building_sdt);
      /* Reinitialize the structures */
      p_sdt_decoder->p_building_sdt = NULL;
      for(unsigned int i = 0; i <= p_sdt_decoder->i_last_section_number; i++)
        p_sdt_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeSDTSection
 *****************************************************************************
 * SDT decoder.
 *****************************************************************************/
void dvbpsi_DecodeSDTSections(dvbpsi_sdt_t* p_sdt,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    for(p_byte = p_section->p_payload_start + 3;
        p_byte + 4 < p_section->p_payload_end;)
    {
      uint16_t i_service_id = ((uint16_t)(p_byte[0]) << 8) | p_byte[1];
      int b_eit_schedule = (int)((p_byte[2] & 0x2) >> 1);
      int b_eit_present = (int)((p_byte[2]) & 0x1);
      uint8_t i_running_status = (uint8_t)(p_byte[3]) >> 5;
      int b_free_ca = (int)((p_byte[3] & 0x10) >> 4);
      uint16_t i_srv_length = ((uint16_t)(p_byte[3] & 0xf) <<8) | p_byte[4];
      dvbpsi_sdt_service_t* p_service = dvbpsi_SDTAddService(p_sdt,
          i_service_id, b_eit_schedule, b_eit_present,
          i_running_status, b_free_ca);

      /* Service descriptors */
      p_byte += 5;
      p_end = p_byte + i_srv_length;
      if( p_end > p_section->p_payload_end ) break;

      while(p_byte + 2 <= p_end)
      {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_SDTServiceAddDescriptor(p_service, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
      }
    }

    p_section = p_section->p_next;
  }
}


/*****************************************************************************
 * dvbpsi_GenSDTSections
 *****************************************************************************
 * Generate SDT sections based on the dvbpsi_sdt_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_GenSDTSections(dvbpsi_sdt_t* p_sdt)
{
  dvbpsi_psi_section_t * p_result = dvbpsi_NewPSISection(1024);
  dvbpsi_psi_section_t * p_current = p_result;
  dvbpsi_psi_section_t * p_prev;

  dvbpsi_sdt_service_t *    p_service = p_sdt->p_first_service;

  p_current->i_table_id = 0x42;
  p_current->b_syntax_indicator = 1;
  p_current->b_private_indicator = 1;
  p_current->i_length = 12;                     /* header + CRC_32 */
  p_current->i_extension = p_sdt->i_ts_id;
  p_current->i_version = p_sdt->i_version;
  p_current->b_current_next = p_sdt->b_current_next;
  p_current->i_number = 0;
  p_current->p_payload_end += 11;               /* just after the header */
  p_current->p_payload_start = p_current->p_data + 8;

  /* Original Network ID */
  p_current->p_data[8] = (p_sdt->i_network_id >> 8) ;
  p_current->p_data[9] = p_sdt->i_network_id;
  p_current->p_data[10] = 0xff;

  /* SDT service */
  while(p_service != NULL)
  {
    uint8_t * p_service_start = p_current->p_payload_end;

    uint16_t i_service_length = 5;

    dvbpsi_descriptor_t * p_descriptor = p_service->p_first_descriptor;

    while ( (p_descriptor != NULL)&& ((p_service_start - p_current->p_data) + i_service_length <= 1020) )
    {
      i_service_length += p_descriptor->i_length + 2;
      p_descriptor = p_descriptor->p_next;
    }

    if ( (p_descriptor != NULL) && (p_service_start - p_current->p_data != 11) && (i_service_length <= 1009) )
    {
      /* will put more descriptors in an empty section */
      DVBPSI_DEBUG("SDT generator","create a new section to carry more Service descriptors");
      p_prev = p_current;
      p_current = dvbpsi_NewPSISection(1024);
      p_prev->p_next = p_current;

      p_current->i_table_id = 0x42;
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 1;
      p_current->i_length = 12;                 /* header + CRC_32 */
      p_current->i_extension = p_sdt->i_ts_id;;
      p_current->i_version = p_sdt->i_version;
      p_current->b_current_next = p_sdt->b_current_next;
      p_current->i_number = p_prev->i_number + 1;
      p_current->p_payload_end += 11;           /* just after the header */
      p_current->p_payload_start = p_current->p_data + 8;


      /* Original Network ID */
      p_current->p_data[8] = (p_sdt->i_network_id >> 8) ;
      p_current->p_data[9] = p_sdt->i_network_id;
      p_current->p_data[10] = 0xff;

      p_service_start = p_current->p_payload_end;
    }

    p_service_start[0] = (p_service->i_service_id >>8);
    p_service_start[1] = (p_service->i_service_id );
    p_service_start[2] = 0xfc | (p_service-> b_eit_schedule  ? 0x2 : 0x0) | (p_service->b_eit_present ? 0x01 : 0x00);
    p_service_start[3] = ((p_service->i_running_status & 0x07) << 5 ) | ((p_service->b_free_ca & 0x1) << 4);

    /* Increase the length by 5 */
    p_current->p_payload_end += 5;
    p_current->i_length += 5;

    /* ES descriptors */
    p_descriptor = p_service->p_first_descriptor;
    while ( (p_descriptor != NULL) && ( (p_current->p_payload_end - p_current->p_data) + p_descriptor->i_length <= 1018) )
    {
      /* p_payload_end is where the descriptor begins */
      p_current->p_payload_end[0] = p_descriptor->i_tag;
      p_current->p_payload_end[1] = p_descriptor->i_length;
      memcpy(p_current->p_payload_end + 2, p_descriptor->p_data, p_descriptor->i_length);
      /* Increase length by descriptor_length + 2 */
      p_current->p_payload_end += p_descriptor->i_length + 2;
      p_current->i_length += p_descriptor->i_length + 2;
      p_descriptor = p_descriptor->p_next;
    }

    if(p_descriptor != NULL)
      DVBPSI_ERROR("SDT generator", "unable to carry all the descriptors");

    /* ES_info_length */
    i_service_length = p_current->p_payload_end - p_service_start - 5;
    p_service_start[3] |= ((i_service_length  >> 8) & 0x0f);
    p_service_start[4] = i_service_length;

    p_service = p_service->p_next;
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

