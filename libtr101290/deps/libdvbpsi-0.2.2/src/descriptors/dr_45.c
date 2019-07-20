/*****************************************************************************
 * dr_45.c
 * Copyright (C) 2004-2010 VideoLAN
 * $Id$
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
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

#include "dr_45.h"

/*****************************************************************************
 * dvbpsi_DecodeVBIDataDr
 *****************************************************************************/
dvbpsi_vbi_dr_t * dvbpsi_DecodeVBIDataDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  int i_services_number, i;
  dvbpsi_vbi_dr_t * p_decoded;

  /* Check the tag */
  if( p_descriptor->i_tag != 0x45 )
  {
    DVBPSI_ERROR_ARG("dr_45 decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Decode data and check the length */
  if(p_descriptor->i_length < 3)
  {
    DVBPSI_ERROR_ARG("dr_45 decoder", "bad length (%d)",
                     p_descriptor->i_length);
    return NULL;
  }

  if(p_descriptor->i_length % 2)
  {
    DVBPSI_ERROR_ARG("dr_45 decoder", "length not multiple of 3(%d)",
                     p_descriptor->i_length);
    return NULL;
  }

  i_services_number = p_descriptor->i_length / 2;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_vbi_dr_t*)malloc(sizeof(dvbpsi_vbi_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_45 decoder", "out of memory");
    return NULL;
  }

  p_decoded->i_services_number = i_services_number;

  for(i=0; i < i_services_number; i++)
  {
    int n, i_lines = 0, i_data_service_id;

    i_data_service_id = ((uint8_t)(p_descriptor->p_data[3 * i + 2 + i_lines]));
    p_decoded->p_services[i].i_data_service_id = i_data_service_id;

    i_lines = ((uint8_t)(p_descriptor->p_data[3 * i + 3]));
    p_decoded->p_services[i].i_lines = i_lines;
    for(n=0; n < i_lines; n++ )
    {
      if( (i_data_service_id >= 0x01) && (i_data_service_id <= 0x07) )
      {
        p_decoded->p_services[i].p_lines[n].i_parity =
               ((uint8_t)((p_descriptor->p_data[3 * i + 3 + n])&0x20)>>5);
        p_decoded->p_services[i].p_lines[n].i_line_offset =
               ((uint8_t)(p_descriptor->p_data[3 * i + 3 + n])&0x1f);
      }
    }
  }

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenVBIDataDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenVBIDataDr(
                                        dvbpsi_vbi_dr_t * p_decoded,
                                        int b_duplicate)
{
  int i;

  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
      dvbpsi_NewDescriptor(0x45, p_decoded->i_services_number * 5 , NULL);

  if(p_descriptor)
  {
    /* Encode data */
    for (i=0; i < p_decoded->i_services_number; i++ )
    {
      int n;
      p_descriptor->p_data[5 * i + 3] =
                    ( (uint8_t) p_decoded->p_services[i].i_data_service_id );

      p_descriptor->p_data[5 * i + 4] = p_decoded->p_services[i].i_lines;
      for (n=0; n < p_decoded->p_services[i].i_lines; n++ )
      {
         if( (p_decoded->p_services[i].i_data_service_id >= 0x01) &&
             (p_decoded->p_services[i].i_data_service_id <= 0x07) )
         {
            p_descriptor->p_data[5 * i + 4 + n] = (uint8_t)
                ( (((uint8_t) p_decoded->p_services[i].p_lines[n].i_parity)&0x20)<<5) |
                ( ((uint8_t) p_decoded->p_services[i].p_lines[n].i_line_offset)&0x1f);
         }
         else p_descriptor->p_data[5 * i + 3 + n] = 0xFF; /* Stuffing byte */
      }
    }

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_vbi_dr_t * p_dup_decoded =
        (dvbpsi_vbi_dr_t*)malloc(sizeof(dvbpsi_vbi_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_vbi_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}
