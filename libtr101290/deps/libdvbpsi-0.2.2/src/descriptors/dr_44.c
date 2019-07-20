/*****************************************************************************
 * dr_44.c
 * Copyright (C) 2011 VideoLAN
 * $Id$
 *
 * Authors: Ilkka Ollakka
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

#include "dr_44.h"


/*****************************************************************************
 * dvbpsi_DecodeCableDelivSysDr
 *****************************************************************************/
dvbpsi_cable_deliv_sys_dr_t * dvbpsi_DecodeCableDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_cable_deliv_sys_dr_t * p_decoded;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x44)
  {
    DVBPSI_ERROR_ARG("dr_44 decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_cable_deliv_sys_dr_t*)malloc(sizeof(dvbpsi_cable_deliv_sys_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_44 decoder", "out of memory");
    return NULL;
  }

  /* Decode data */
  p_decoded->i_frequency         =   (uint32_t)(p_descriptor->p_data[0] << 24)
                                   | (uint32_t)(p_descriptor->p_data[1] << 16)
                                   | (uint32_t)(p_descriptor->p_data[2] <<  8)
                                   | (uint32_t)(p_descriptor->p_data[3] );
  p_decoded->i_fec_outer         =   (uint8_t)(p_descriptor->p_data[5] & 0x0f);
  p_decoded->i_modulation        =   (uint8_t)(p_descriptor->p_data[6] );
  p_decoded->i_symbol_rate       =   (uint32_t)(p_descriptor->p_data[7] << 20)
                                   | (uint32_t)(p_descriptor->p_data[8] << 12)
                                   | (uint32_t)(p_descriptor->p_data[9] <<  4)
                                   | (uint32_t)((p_descriptor->p_data[10] & 0xf0) >> 4);
  p_decoded->i_fec_inner         =   (uint8_t)(p_descriptor->p_data[10] & 0x0f);

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenCableDelivSysDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenCableDelivSysDr(
                                        dvbpsi_cable_deliv_sys_dr_t * p_decoded,
                                        int b_duplicate)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x44, 11, NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0]  =     (p_decoded->i_frequency >> 24)       & 0xff;
    p_descriptor->p_data[1]  =     (p_decoded->i_frequency >> 16)       & 0xff;
    p_descriptor->p_data[2]  =     (p_decoded->i_frequency >>  8)       & 0xff;
    p_descriptor->p_data[3]  =      p_decoded->i_frequency              & 0xff;
    p_descriptor->p_data[5]  =     (p_decoded->i_fec_outer              & 0x0f);
    p_descriptor->p_data[6]  =     (p_decoded->i_modulation);
    p_descriptor->p_data[7]  =     (p_decoded->i_symbol_rate >> 20)     & 0xff;
    p_descriptor->p_data[8]  =     (p_decoded->i_symbol_rate >> 12)     & 0xff;
    p_descriptor->p_data[9]  =     (p_decoded->i_symbol_rate >>  4)     & 0xff;
    p_descriptor->p_data[10] =    ((p_decoded->i_symbol_rate <<  4)     & 0xf0)
                                  |(p_decoded->i_fec_inner              & 0x0f);

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_cable_deliv_sys_dr_t * p_dup_decoded =
        (dvbpsi_cable_deliv_sys_dr_t*)malloc(sizeof(dvbpsi_cable_deliv_sys_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_cable_deliv_sys_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}
