/*****************************************************************************
 * dr_0f.c
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: dr_0f.c,v 1.3 2003/07/25 20:20:40 fenrir Exp $
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
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"

#include "dr_0f.h"


/*****************************************************************************
 * dvbpsi_DecodePrivateDataDr
 *****************************************************************************/
dvbpsi_private_data_dr_t * dvbpsi_DecodePrivateDataDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_private_data_dr_t * p_decoded;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x0f)
  {
    DVBPSI_ERROR_ARG("dr_0f decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_private_data_dr_t*)malloc(sizeof(dvbpsi_private_data_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_0f decoder", "out of memory");
    return NULL;
  }

  /* Decode data and check the length */
  if(p_descriptor->i_length != 4)
  {
    DVBPSI_ERROR_ARG("dr_0f decoder", "bad length (%d)",
                     p_descriptor->i_length);
    free(p_decoded);
    return NULL;
  }

  p_decoded->i_private_data =   ((uint32_t)(p_descriptor->p_data[0]) << 24)
                              | ((uint32_t)(p_descriptor->p_data[1]) << 16)
                              | ((uint32_t)(p_descriptor->p_data[2]) << 8)
                              | p_descriptor->p_data[3];

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenPrivateDataDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenPrivateDataDr(
                                        dvbpsi_private_data_dr_t * p_decoded,
                                        int b_duplicate)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x0f, 4, NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_private_data >> 24;
    p_descriptor->p_data[1] = p_decoded->i_private_data >> 16;
    p_descriptor->p_data[2] = p_decoded->i_private_data >> 8;
    p_descriptor->p_data[3] = p_decoded->i_private_data;

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_private_data_dr_t * p_dup_decoded =
        (dvbpsi_private_data_dr_t*)malloc(sizeof(dvbpsi_private_data_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_private_data_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}

