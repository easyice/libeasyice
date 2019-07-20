/*****************************************************************************
 * dr_48.c
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#include "dr_48.h"


/*****************************************************************************
 * dvbpsi_DecodeServiceDr
 *****************************************************************************/
dvbpsi_service_dr_t * dvbpsi_DecodeServiceDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_service_dr_t * p_decoded;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x48)
  {
    DVBPSI_ERROR_ARG("dr_48 decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_service_dr_t*)malloc(sizeof(dvbpsi_service_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_48 decoder", "out of memory");
    return NULL;
  }

  /* Decode data and check the length */
  if(p_descriptor->i_length < 3)
  {
    DVBPSI_ERROR_ARG("dr_07 decoder", "bad length (%d)",
                     p_descriptor->i_length);
    free(p_decoded);
    return NULL;
  }

  p_descriptor->p_decoded = (void*)p_decoded;

  p_decoded->i_service_type = p_descriptor->p_data[0];
  p_decoded->i_service_provider_name_length = p_descriptor->p_data[1];
  p_decoded->i_service_name_length = 0;
  p_decoded->i_service_provider_name[0] = 0;
  p_decoded->i_service_name[0] = 0;

  if(p_decoded->i_service_provider_name_length + 2 > p_descriptor->i_length)
      return p_decoded;

  if(p_decoded->i_service_provider_name_length)
    memcpy(p_decoded->i_service_provider_name,
           p_descriptor->p_data + 2,
           p_decoded->i_service_provider_name_length);

  if(p_decoded->i_service_provider_name_length + 3 > p_descriptor->i_length)
      return p_decoded;

  p_decoded->i_service_name_length =
    p_descriptor->p_data[2+p_decoded->i_service_provider_name_length];

  if(p_decoded->i_service_provider_name_length + 3 +
     p_decoded->i_service_name_length > p_descriptor->i_length)
      return p_decoded;

  if(p_decoded->i_service_name_length)
    memcpy(p_decoded->i_service_name,
           p_descriptor->p_data + 3 + p_decoded->i_service_provider_name_length,
           p_decoded->i_service_name_length);

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenServiceDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenServiceDr(
                                        dvbpsi_service_dr_t * p_decoded,
                                        int b_duplicate)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x48, 3 + p_decoded->i_service_name_length +
               p_decoded->i_service_provider_name_length , NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_service_type;
    p_descriptor->p_data[1] = p_decoded->i_service_provider_name_length;
    if(p_decoded->i_service_provider_name_length)
      memcpy(p_descriptor->p_data + 2,
             p_decoded->i_service_provider_name,
             p_decoded->i_service_provider_name_length);
    p_descriptor->p_data[2+p_decoded->i_service_provider_name_length] =
      p_decoded->i_service_name_length;
    if(p_decoded->i_service_name_length)
      memcpy(p_descriptor->p_data + 3 + p_decoded->i_service_provider_name_length,
             p_decoded->i_service_name,
             p_decoded->i_service_name_length);

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_service_dr_t * p_dup_decoded =
        (dvbpsi_service_dr_t*)malloc(sizeof(dvbpsi_service_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_service_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}

