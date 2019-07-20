/*****************************************************************************
 * nit.c: NIT decoder/generator
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
#include "../demux.h"
#include "nit.h"
#include "nit_private.h"


/*****************************************************************************
 * dvbpsi_AttachNIT
 *****************************************************************************
 * Initialize a NIT subtable decoder.
 *****************************************************************************/
int dvbpsi_AttachNIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
                     uint16_t i_extension, dvbpsi_nit_callback pf_callback,
                     void* p_cb_data)
{
  dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_psi_decoder->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_nit_decoder_t*  p_nit_decoder;

  if(dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
  {
    DVBPSI_ERROR_ARG("NIT decoder",
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

  p_nit_decoder = (dvbpsi_nit_decoder_t*)malloc(sizeof(dvbpsi_nit_decoder_t));

  if(p_nit_decoder == NULL)
  {
    free(p_subdec);
    return 1;
  }

  /* subtable decoder configuration */
  p_subdec->pf_callback = &dvbpsi_GatherNITSections;
  p_subdec->p_cb_data = p_nit_decoder;
  p_subdec->i_id = (uint32_t)i_table_id << 16 | (uint32_t)i_extension;
  p_subdec->pf_detach = dvbpsi_DetachNIT;

  /* Attach the subtable decoder to the demux */
  p_subdec->p_next = p_demux->p_first_subdec;
  p_demux->p_first_subdec = p_subdec;

  /* NIT decoder information */
  p_nit_decoder->i_network_id = i_extension;
  p_nit_decoder->pf_callback = pf_callback;
  p_nit_decoder->p_cb_data = p_cb_data;
  /* NIT decoder initial state */
  p_nit_decoder->b_current_valid = 0;
  p_nit_decoder->p_building_nit = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_nit_decoder->ap_sections[i] = NULL;

  return 0;
}


/*****************************************************************************
 * dvbpsi_DetachNIT
 *****************************************************************************
 * Close a NIT decoder.
 *****************************************************************************/
void dvbpsi_DetachNIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                      uint16_t i_extension)
{
  dvbpsi_demux_subdec_t* p_subdec;
  dvbpsi_demux_subdec_t** pp_prev_subdec;
  dvbpsi_nit_decoder_t* p_nit_decoder;

  p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);

  if(p_demux == NULL)
  {
    DVBPSI_ERROR_ARG("NIT Decoder",
                     "No such NIT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
    return;
  }

  p_nit_decoder = (dvbpsi_nit_decoder_t*)p_subdec->p_cb_data;

  free(p_nit_decoder->p_building_nit);

  for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_nit_decoder->ap_sections[i])
      dvbpsi_DeletePSISections(p_nit_decoder->ap_sections[i]);
  }

  free(p_subdec->p_cb_data);

  pp_prev_subdec = &p_demux->p_first_subdec;
  while(*pp_prev_subdec != p_subdec)
    pp_prev_subdec = &(*pp_prev_subdec)->p_next;

  *pp_prev_subdec = p_subdec->p_next;
  free(p_subdec);
}


/*****************************************************************************
 * dvbpsi_InitNIT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_nit_t structure.
 *****************************************************************************/
void dvbpsi_InitNIT(dvbpsi_nit_t* p_nit, uint16_t i_network_id,
                    uint8_t i_version, int b_current_next)
{
  p_nit->i_network_id = i_network_id;
  p_nit->i_version = i_version;
  p_nit->b_current_next = b_current_next;
  p_nit->p_first_descriptor = NULL;
  p_nit->p_first_ts = NULL;
}


/*****************************************************************************
 * dvbpsi_EmptyNIT
 *****************************************************************************
 * Clean a dvbpsi_nit_t structure.
 *****************************************************************************/
void dvbpsi_EmptyNIT(dvbpsi_nit_t* p_nit)
{
  dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;

  dvbpsi_DeleteDescriptors(p_nit->p_first_descriptor);

  while(p_ts != NULL)
  {
    dvbpsi_nit_ts_t* p_tmp = p_ts->p_next;
    dvbpsi_DeleteDescriptors(p_ts->p_first_descriptor);
    free(p_ts);
    p_ts = p_tmp;
  }

  p_nit->p_first_descriptor = NULL;
  p_nit->p_first_ts = NULL;
}


/*****************************************************************************
 * dvbpsi_NITAddDescriptor
 *****************************************************************************
 * Add a descriptor in the NIT.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_NITAddDescriptor(dvbpsi_nit_t* p_nit,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_nit->p_first_descriptor == NULL)
    {
      p_nit->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_nit->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_NITAddTS
 *****************************************************************************
 * Add an TS in the NIT.
 *****************************************************************************/
dvbpsi_nit_ts_t* dvbpsi_NITAddTS(dvbpsi_nit_t* p_nit,
                                 uint16_t i_ts_id, uint16_t i_orig_network_id)
{
  dvbpsi_nit_ts_t* p_ts = (dvbpsi_nit_ts_t*)malloc(sizeof(dvbpsi_nit_ts_t));

  if(p_ts)
  {
    p_ts->i_ts_id = i_ts_id;
    p_ts->i_orig_network_id = i_orig_network_id;
    p_ts->p_first_descriptor = NULL;
    p_ts->p_next = NULL;

    if(p_nit->p_first_ts == NULL)
    {
      p_nit->p_first_ts = p_ts;
    }
    else
    {
      dvbpsi_nit_ts_t* p_last_ts = p_nit->p_first_ts;
      while(p_last_ts->p_next != NULL)
        p_last_ts = p_last_ts->p_next;
      p_last_ts->p_next = p_ts;
    }
  }

  return p_ts;
}


/*****************************************************************************
 * dvbpsi_NITTSAddDescriptor
 *****************************************************************************
 * Add a descriptor in the NIT TS.
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_NITTSAddDescriptor(dvbpsi_nit_ts_t* p_ts,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
{
  dvbpsi_descriptor_t* p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);

  if(p_descriptor)
  {
    if(p_ts->p_first_descriptor == NULL)
    {
      p_ts->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t* p_last_descriptor = p_ts->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}


/*****************************************************************************
 * dvbpsi_GatherNITSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_GatherNITSections(dvbpsi_decoder_t * p_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t * p_section)
{
  dvbpsi_nit_decoder_t* p_nit_decoder
                        = (dvbpsi_nit_decoder_t*)p_private_decoder;
  int b_append = 1;
  int b_reinit = 0;

  DVBPSI_DEBUG_ARG("NIT decoder",
                   "Table version %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);

  if(p_section->i_table_id != 0x40 && p_section->i_table_id != 0x41)
  {
    /* Invalid table_id value */
    DVBPSI_ERROR_ARG("NIT decoder",
                     "invalid section (table_id == 0x%02x)",
                     p_section->i_table_id);
    b_append = 0;
  }

  if(b_append && !p_section->b_syntax_indicator)
  {
    /* Invalid section_syntax_indicator */
    DVBPSI_ERROR("NIT decoder",
                 "invalid section (section_syntax_indicator == 0)");
    b_append = 0;
  }

  /* Now if b_append is true then we have a valid NIT section */
  if(b_append && (p_nit_decoder->i_network_id != p_section->i_extension))
  {
    /* Invalid program_number */
#if 0
    DVBPSI_ERROR("NIT decoder", "'network_id' don't match");
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
      if(p_nit_decoder->p_building_nit)
      {
        if(p_nit_decoder->p_building_nit->i_version != p_section->i_version)
        {
          /* version_number */
          DVBPSI_ERROR("NIT decoder",
                       "'version_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
        else if(p_nit_decoder->i_last_section_number
                                                != p_section->i_last_number)
        {
          /* last_section_number */
          DVBPSI_ERROR("NIT decoder",
                       "'last_section_number' differs"
                       " whereas no discontinuity has occured");
          b_reinit = 1;
        }
      }
      else
      {
        if(    (p_nit_decoder->b_current_valid)
            && (p_nit_decoder->current_nit.i_version == p_section->i_version)
            && (p_nit_decoder->current_nit.b_current_next ==
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
    p_nit_decoder->b_current_valid = 0;
    /* Free structures */
    if(p_nit_decoder->p_building_nit)
    {
      free(p_nit_decoder->p_building_nit);
      p_nit_decoder->p_building_nit = NULL;
    }
    /* Clear the section array */
    for(unsigned int i = 0; i <= 255; i++)
    {
      if(p_nit_decoder->ap_sections[i] != NULL)
      {
        dvbpsi_DeletePSISections(p_nit_decoder->ap_sections[i]);
        p_nit_decoder->ap_sections[i] = NULL;
      }
    }
  }

  /* Append the section to the list if wanted */
  if(b_append)
  {
    int b_complete;

    /* Initialize the structures if it's the first section received */
    if(!p_nit_decoder->p_building_nit)
    {
      p_nit_decoder->p_building_nit =
                                (dvbpsi_nit_t*)malloc(sizeof(dvbpsi_nit_t));
      dvbpsi_InitNIT(p_nit_decoder->p_building_nit,
                     p_nit_decoder->i_network_id,
                     p_section->i_version,
                     p_section->b_current_next);
      p_nit_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Fill the section array */
    if(p_nit_decoder->ap_sections[p_section->i_number] != NULL)
    {
      DVBPSI_DEBUG_ARG("NIT decoder", "overwrite section number %d",
                       p_section->i_number);
      dvbpsi_DeletePSISections(p_nit_decoder->ap_sections[p_section->i_number]);
    }
    p_nit_decoder->ap_sections[p_section->i_number] = p_section;

    /* Check if we have all the sections */
    b_complete = 0;
    for(unsigned int i = 0; i <= p_nit_decoder->i_last_section_number; i++)
    {
      if(!p_nit_decoder->ap_sections[i])
        break;

      if(i == p_nit_decoder->i_last_section_number)
        b_complete = 1;
    }

    if(b_complete)
    {
      /* Save the current information */
      p_nit_decoder->current_nit = *p_nit_decoder->p_building_nit;
      p_nit_decoder->b_current_valid = 1;
      /* Chain the sections */
      if(p_nit_decoder->i_last_section_number)
      {
        for(unsigned int i = 0; (int)i <= p_nit_decoder->i_last_section_number - 1; i++)
          p_nit_decoder->ap_sections[i]->p_next =
                                        p_nit_decoder->ap_sections[i + 1];
      }
      /* Decode the sections */
      dvbpsi_DecodeNITSections(p_nit_decoder->p_building_nit,
                               p_nit_decoder->ap_sections[0]);
      /* Delete the sections */
      dvbpsi_DeletePSISections(p_nit_decoder->ap_sections[0]);
      /* signal the new NIT */
      p_nit_decoder->pf_callback(p_nit_decoder->p_cb_data,
                                 p_nit_decoder->p_building_nit);
      /* Reinitialize the structures */
      p_nit_decoder->p_building_nit = NULL;
      for(unsigned int i = 0; i <= p_nit_decoder->i_last_section_number; i++)
        p_nit_decoder->ap_sections[i] = NULL;
    }
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}


/*****************************************************************************
 * dvbpsi_DecodeNITSections
 *****************************************************************************
 * NIT decoder.
 *****************************************************************************/
void dvbpsi_DecodeNITSections(dvbpsi_nit_t* p_nit,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t* p_byte, * p_end, * p_end2;

  while(p_section)
  {
    /* - NIT descriptors */
    p_byte = p_section->p_payload_start + 2;
    p_end = p_byte + (   ((uint16_t)(p_section->p_payload_start[0] & 0x0f) << 8)
                       | p_section->p_payload_start[1]);

    while(p_byte + 2 <= p_end)
    {
      uint8_t i_tag = p_byte[0];
      uint8_t i_length = p_byte[1];
      if(i_length + 2 <= p_end - p_byte)
        dvbpsi_NITAddDescriptor(p_nit, i_tag, i_length, p_byte + 2);
      p_byte += 2 + i_length;
    }

    p_end = p_byte + (   ((uint16_t)(p_byte[0] & 0x0f) << 8)
                       | p_byte[1]);
    if(p_end > p_section->p_payload_end)
    {
        p_end = p_section->p_payload_end;
    }
    p_byte += 2;

    /* - TSs */
    for(; p_byte + 6 <= p_end;)
    {
      uint16_t i_ts_id = ((uint16_t)p_byte[0] << 8) | p_byte[1];
      uint16_t i_orig_network_id = ((uint16_t)p_byte[2] << 8) | p_byte[3];
      uint16_t i_ts_length = ((uint16_t)(p_byte[4] & 0x0f) << 8) | p_byte[5];
      dvbpsi_nit_ts_t* p_ts = dvbpsi_NITAddTS(p_nit, i_ts_id, i_orig_network_id);
      /* - TS descriptors */
      p_byte += 6;
      p_end2 = p_byte + i_ts_length;
      if( p_end2 > p_section->p_payload_end )
      {
            p_end2 = p_section->p_payload_end;
      }
      while(p_byte + 2 <= p_end2)
      {
        uint8_t i_tag = p_byte[0];
        uint8_t i_length = p_byte[1];
        if(i_length + 2 <= p_end2 - p_byte)
          dvbpsi_NITTSAddDescriptor(p_ts, i_tag, i_length, p_byte + 2);
        p_byte += 2 + i_length;
      }
    }

    p_section = p_section->p_next;
  }
}


/*****************************************************************************
 * dvbpsi_GenNITSections
 *****************************************************************************
 * Generate NIT sections based on the dvbpsi_nit_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_GenNITSections(dvbpsi_nit_t* p_nit,
                                            uint8_t i_table_id)
{
  dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
  dvbpsi_psi_section_t* p_current = p_result;
  dvbpsi_psi_section_t* p_prev;
  dvbpsi_descriptor_t* p_descriptor = p_nit->p_first_descriptor;
  dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;
  uint16_t i_network_descriptors_length, i_transport_stream_loop_length;
  uint8_t * p_transport_stream_loop_length;

  p_current->i_table_id = i_table_id;
  p_current->b_syntax_indicator = 1;
  p_current->b_private_indicator = 0;
  p_current->i_length = 13;                     /* including CRC_32 */
  p_current->i_extension = p_nit->i_network_id;
  p_current->i_version = p_nit->i_version;
  p_current->b_current_next = p_nit->b_current_next;
  p_current->i_number = 0;
  p_current->p_payload_end += 10;
  p_current->p_payload_start = p_current->p_data + 8;

  /* NIT descriptors */
  while(p_descriptor != NULL)
  {
    /* New section if needed */
    /* written_data_length + descriptor_length + 2 > 1024 - CRC_32_length */
    if(   (p_current->p_payload_end - p_current->p_data)
                                + p_descriptor->i_length > 1018)
    {
      /* network_descriptors_length */
      i_network_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
      p_current->p_data[8] = (i_network_descriptors_length >> 8) | 0xf0;
      p_current->p_data[9] = i_network_descriptors_length;

      /* transport_stream_loop_length */
      p_current->p_payload_end[0] = 0;
      p_current->p_payload_end[1] = 0;
      p_current->p_payload_end += 2;

      p_prev = p_current;
      p_current = dvbpsi_NewPSISection(1024);
      p_prev->p_next = p_current;

      p_current->i_table_id = i_table_id;
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
      p_current->i_length = 13;                 /* including CRC_32 */
      p_current->i_extension = p_nit->i_network_id;
      p_current->i_version = p_nit->i_version;
      p_current->b_current_next = p_nit->b_current_next;
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

  /* network_descriptors_length */
  i_network_descriptors_length = (p_current->p_payload_end - p_current->p_payload_start) - 2;
  p_current->p_data[8] = (i_network_descriptors_length >> 8) | 0xf0;
  p_current->p_data[9] = i_network_descriptors_length;

  /* Store the position of the transport_stream_loop_length field
     and reserve two bytes for it */
  p_transport_stream_loop_length = p_current->p_payload_end;
  p_current->p_payload_end += 2;

  /* NIT TSs */
  while(p_ts != NULL)
  {
    uint8_t* p_ts_start = p_current->p_payload_end;
    uint16_t i_ts_length = 5;

    /* Can the current section carry all the descriptors ? */
    p_descriptor = p_ts->p_first_descriptor;
    while(    (p_descriptor != NULL)
           && ((p_ts_start - p_current->p_data) + i_ts_length <= 1020))
    {
      i_ts_length += p_descriptor->i_length + 2;
      p_descriptor = p_descriptor->p_next;
    }

    /* If _no_ and the current section isn't empty and an empty section
       may carry one more descriptor
       then create a new section */
    if(    (p_descriptor != NULL)
        && (p_ts_start - p_current->p_data != 12)
        && (i_ts_length <= 1008))
    {
      /* transport_stream_loop_length */
      i_transport_stream_loop_length = (p_current->p_payload_end - p_transport_stream_loop_length) - 2;
      p_transport_stream_loop_length[0] = (i_transport_stream_loop_length >> 8) | 0xf0;
      p_transport_stream_loop_length[1] = i_transport_stream_loop_length;

      /* will put more descriptors in an empty section */
      DVBPSI_DEBUG("NIT generator",
                   "create a new section to carry more TS descriptors");
      p_prev = p_current;
      p_current = dvbpsi_NewPSISection(1024);
      p_prev->p_next = p_current;

      p_current->i_table_id = i_table_id;
      p_current->b_syntax_indicator = 1;
      p_current->b_private_indicator = 0;
      p_current->i_length = 13;                 /* including CRC_32 */
      p_current->i_extension = p_nit->i_network_id;
      p_current->i_version = p_nit->i_version;
      p_current->b_current_next = p_nit->b_current_next;
      p_current->i_number = p_prev->i_number + 1;
      p_current->p_payload_end += 10;
      p_current->p_payload_start = p_current->p_data + 8;

      /* network_descriptors_length = 0 */
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

    if(p_descriptor != NULL)
      DVBPSI_ERROR("NIT generator", "unable to carry all the TS descriptors");

    /* TS_info_length */
    i_ts_length = p_current->p_payload_end - p_ts_start - 5;
    p_ts_start[4] = (i_ts_length >> 8) | 0xf0;
    p_ts_start[5] = i_ts_length;

    p_ts = p_ts->p_next;
  }

  /* transport_stream_loop_length */
  i_transport_stream_loop_length = (p_current->p_payload_end - p_transport_stream_loop_length) - 2;
  p_transport_stream_loop_length[0] = (i_transport_stream_loop_length >> 8) | 0xf0;
  p_transport_stream_loop_length[1] = i_transport_stream_loop_length;

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
