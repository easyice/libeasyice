/*****************************************************************************
 * dr_4d.c
 * Copyright (C) 2005-2010 VideoLAN
 * $Id: dr_4d.c,v 1.7 2003/07/25 20:20:40 fenrir Exp $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
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

#include "dr_4d.h"

/*****************************************************************************
 * dvbpsi_DecodeShortEventDr
 *****************************************************************************/
dvbpsi_short_event_dr_t * dvbpsi_DecodeShortEventDr(dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_short_event_dr_t * p_decoded;
  int i_len1;
  int i_len2;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x4d ||
     p_descriptor->i_length < 5 )
  {
    DVBPSI_ERROR_ARG("dr_4d decoder", "bad tag or corrupted(0x%x)", p_descriptor->i_tag);
    return NULL;
  }
  /* Check length */
  i_len1 = p_descriptor->p_data[3];
  i_len2 = p_descriptor->p_data[4+i_len1];
  if( p_descriptor->i_length < 5 + i_len1 + i_len2 )
  {
    DVBPSI_ERROR_ARG("dr_4d decoder", "invalid length/content (tag=0x%x)", p_descriptor->i_tag );
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded = malloc(sizeof(dvbpsi_short_event_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_4d decoder", "out of memory");
    return NULL;
  }

  /* Decode data and check the length */
  memcpy( p_decoded->i_iso_639_code, &p_descriptor->p_data[0], 3 );
  p_decoded->i_event_name_length = i_len1;
  if( i_len1 > 0 )
      memcpy( p_decoded->i_event_name, &p_descriptor->p_data[3+1], i_len1 );
  p_decoded->i_text_length = i_len2;
  if( i_len2 > 0 )
      memcpy( p_decoded->i_text, &p_descriptor->p_data[4+i_len1+1], i_len2 );

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenShortEventDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenShortEventDr(dvbpsi_short_event_dr_t * p_decoded,
                                          int b_duplicate)
{
  int i_len1 = p_decoded->i_event_name_length;
  int i_len2 = p_decoded->i_text_length;

  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
                dvbpsi_NewDescriptor(0x4d, 5 + i_len1 + i_len2, NULL );

  if(p_descriptor)
  {
    /* Encode data */
    memcpy( &p_descriptor->p_data[0], p_decoded->i_iso_639_code, 3 );
    p_descriptor->p_data[3] = i_len1;
    if( i_len1 )
        memcpy( &p_descriptor->p_data[4], p_decoded->i_event_name, i_len1 );
    p_descriptor->p_data[3+i_len1] = i_len2;
    if( i_len2 )
        memcpy( &p_descriptor->p_data[3+i_len1+1], p_decoded->i_text, i_len2 );

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_short_event_dr_t * p_dup_decoded =
                (dvbpsi_short_event_dr_t*)malloc(sizeof(dvbpsi_short_event_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_short_event_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}
