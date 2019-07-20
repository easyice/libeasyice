/*****************************************************************************
 * dr_02.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: dr_02.c,v 1.7 2003/07/25 20:20:40 fenrir Exp $
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
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"

#include "dr_02.h"

/*****************************************************************************
 * dvbpsi_DecodeVStreamDr
 *****************************************************************************/
dvbpsi_vstream_dr_t * dvbpsi_DecodeVStreamDr(dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_vstream_dr_t * p_decoded;

  /* Check the tag */
  if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x02))
     return NULL;

  /* Don't decode twice */
  if (dvbpsi_IsDescriptorDecoded(p_descriptor))
     return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded = (dvbpsi_vstream_dr_t*)malloc(sizeof(dvbpsi_vstream_dr_t));
  if(!p_decoded) return NULL;

  /* Decode data and check the length */
  p_decoded->b_mpeg2 = !((p_descriptor->p_data[0] & 0x04) ? true : false);
  if(    (!p_decoded->b_mpeg2 && (p_descriptor->i_length != 1))
      || (p_decoded->b_mpeg2 && (p_descriptor->i_length != 3)))
  {
    free(p_decoded);
    return NULL;
  }

  p_decoded->b_multiple_frame_rate = (p_descriptor->p_data[0] & 0x80) ? true : false;
  p_decoded->i_frame_rate_code = (p_descriptor->p_data[0] & 0x78) >> 3;
  p_decoded->b_constrained_parameter = (p_descriptor->p_data[0] & 0x02) ? true : false;
  p_decoded->b_still_picture = (p_descriptor->p_data[0] & 0x01) ? true : false;

  if(p_decoded->b_mpeg2)
  {
    p_decoded->i_profile_level_indication = p_descriptor->p_data[1];
    p_decoded->i_chroma_format = (p_descriptor->p_data[2] & 0xc0) >> 6;
    p_decoded->b_frame_rate_extension =
                                (p_descriptor->p_data[2] & 0x20) ? true : false;
  }

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenVStreamDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenVStreamDr(dvbpsi_vstream_dr_t * p_decoded,
                                          bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x02, p_decoded->b_mpeg2 ? 3 : 1, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = (p_decoded->b_mpeg2) ? 0 : 0x04;
    if (p_decoded->b_multiple_frame_rate)
        p_descriptor->p_data[0] |= 0x80;
    p_descriptor->p_data[0] |= (p_decoded->i_frame_rate_code & 0x0f) << 3;
    if (p_decoded->b_constrained_parameter)
        p_descriptor->p_data[0] |= 0x02;
    if (p_decoded->b_still_picture)
        p_descriptor->p_data[0] |= 0x01;

    if (p_decoded->b_mpeg2)
    {
        p_descriptor->p_data[1] = p_decoded->i_profile_level_indication;
        p_descriptor->p_data[2] = 0x1f;
        p_descriptor->p_data[2] |= (p_decoded->i_chroma_format & 0x03) << 6;
        if (p_decoded->b_frame_rate_extension)
            p_descriptor->p_data[2] |= 0x20;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_vstream_dr_t));
    }

    return p_descriptor;
}
