/*****************************************************************************
 * dr_58.c
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
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
#include "../descriptor.h"

#include "dr_58.h"


/*****************************************************************************
 * dvbpsi_DecodeLocalTimeOffsetDr
 *****************************************************************************/
dvbpsi_local_time_offset_dr_t * dvbpsi_DecodeLocalTimeOffsetDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_local_time_offset_dr_t * p_decoded;
  uint8_t * p_data, * p_end;
  dvbpsi_local_time_offset_t * p_current;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x58)
  {
    DVBPSI_ERROR_ARG("dr_58 decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_local_time_offset_dr_t*)malloc(sizeof(dvbpsi_local_time_offset_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_58 decoder", "out of memory");
    return NULL;
  }

  /* Decode data */
  p_decoded->i_local_time_offsets_number = 0;
  p_current = p_decoded->p_local_time_offset;
  p_end = p_descriptor->p_data + p_descriptor->i_length;
  p_data = p_descriptor->p_data;
  while(p_data + 13 <= p_end) {
    memcpy(p_current->i_country_code, p_data, 3);
    p_current->i_country_region_id          =   (p_data[3] >> 2) & 0x3f;
    p_current->i_local_time_offset_polarity =   p_data[3] & 0x01;
    p_current->i_local_time_offset          =   ((uint16_t)p_data[4] << 8)
                                              |  (uint16_t)p_data[5];
    p_current->i_time_of_change             =   ((uint64_t)p_data[6] << 32)
                                              | ((uint64_t)p_data[7] << 24)
                                              | ((uint64_t)p_data[8] << 16)
                                              | ((uint64_t)p_data[9] << 8)
                                              |  (uint64_t)p_data[10];
    p_current->i_next_time_offset           =   ((uint16_t)p_data[11] << 8)
                                              |  (uint16_t)p_data[12];

    p_decoded->i_local_time_offsets_number++;
    p_data += 13;
    p_current++;
  }

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenLocalTimeOffsetDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenLocalTimeOffsetDr(
                                        dvbpsi_local_time_offset_dr_t * p_decoded,
                                        int b_duplicate)
{
  uint8_t i_num;
  dvbpsi_local_time_offset_t * p_current;
  uint8_t * p_data;

  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x58, p_decoded->i_local_time_offsets_number * 13, NULL);

  if(p_descriptor)
  {
    /* Encode data */

    p_current = p_decoded->p_local_time_offset;
    p_data = p_descriptor->p_data;

    for(i_num = 0; i_num < p_decoded->i_local_time_offsets_number; i_num++) {
      memcpy(p_data, p_current->i_country_code, 3);
      p_data[3]  =   ((p_current->i_country_region_id & 0x3f) << 2)
                   | 0x02
                   | (p_current->i_local_time_offset_polarity & 0x01);
      p_data[4]  =   (p_current->i_local_time_offset >> 8)    & 0xff;
      p_data[5]  =    p_current->i_local_time_offset          & 0xff;
      p_data[6]  =   (p_current->i_time_of_change   >> 32)    & 0xff;
      p_data[7]  =   (p_current->i_time_of_change   >> 24)    & 0xff;
      p_data[8]  =   (p_current->i_time_of_change   >> 16)    & 0xff;
      p_data[9]  =   (p_current->i_time_of_change   >>  8)    & 0xff;
      p_data[10] =    p_current->i_time_of_change             & 0xff;
      p_data[11] =   (p_current->i_next_time_offset >>  8)    & 0xff;
      p_data[12] =    p_current->i_next_time_offset           & 0xff;

      p_data += 13;
      p_current++;
    }

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_local_time_offset_dr_t * p_dup_decoded =
        (dvbpsi_local_time_offset_dr_t*)malloc(sizeof(dvbpsi_local_time_offset_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_local_time_offset_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}
