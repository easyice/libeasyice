/*****************************************************************************
 * dr_5a.c
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

#include "dr_5a.h"


/*****************************************************************************
 * dvbpsi_DecodeTerrDelivSysDr
 *****************************************************************************/
dvbpsi_terr_deliv_sys_dr_t * dvbpsi_DecodeTerrDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_terr_deliv_sys_dr_t * p_decoded;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x5a)
  {
    DVBPSI_ERROR_ARG("dr_5a decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_terr_deliv_sys_dr_t*)malloc(sizeof(dvbpsi_terr_deliv_sys_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_5a decoder", "out of memory");
    return NULL;
  }

  /* Decode data */
  p_decoded->i_centre_frequency      =    (uint32_t)(p_descriptor->p_data[0] << 24)
                                        | (uint32_t)(p_descriptor->p_data[1] << 16)
                                        | (uint32_t)(p_descriptor->p_data[2] <<  8)
                                        | (uint32_t)(p_descriptor->p_data[3]);
  p_decoded->i_bandwidth             =    (p_descriptor->p_data[4] >> 5) & 0x07;
  p_decoded->i_priority              =    (p_descriptor->p_data[4] >> 4) & 0x01;
  p_decoded->i_time_slice_indicator  =    (p_descriptor->p_data[4] >> 3) & 0x01;
  p_decoded->i_mpe_fec_indicator     =    (p_descriptor->p_data[4] >> 2) & 0x01;
  p_decoded->i_constellation         =    (p_descriptor->p_data[5] >> 6) & 0x03;
  p_decoded->i_hierarchy_information =    (p_descriptor->p_data[5] >> 3) & 0x07;
  p_decoded->i_code_rate_hp_stream   =     p_descriptor->p_data[5]       & 0x07;
  p_decoded->i_code_rate_lp_stream   =    (p_descriptor->p_data[6] >> 5) & 0x07;
  p_decoded->i_guard_interval        =    (p_descriptor->p_data[6] >> 3) & 0x03;
  p_decoded->i_transmission_mode     =    (p_descriptor->p_data[6] >> 1) & 0x03;
  p_decoded->i_other_frequency_flag  =     p_descriptor->p_data[6]       & 0x01;

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenTerrDelivSysDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenTerrDelivSysDr(
                                        dvbpsi_terr_deliv_sys_dr_t * p_decoded,
                                        int b_duplicate)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x5a, 11, NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0]  =   (p_decoded->i_centre_frequency >> 24) & 0xff;
    p_descriptor->p_data[1]  =   (p_decoded->i_centre_frequency >> 16) & 0xff;
    p_descriptor->p_data[2]  =   (p_decoded->i_centre_frequency >>  8) & 0xff;
    p_descriptor->p_data[3]  =    p_decoded->i_centre_frequency        & 0xff;
    p_descriptor->p_data[4]  =   (p_decoded->i_bandwidth               & 0x07) << 5
                               | (p_decoded->i_priority                & 0x01) << 4
                               | (p_decoded->i_time_slice_indicator    & 0x01) << 3
                               | (p_decoded->i_mpe_fec_indicator       & 0x01) << 2
                               | 0x03;
    p_descriptor->p_data[5]  =   (p_decoded->i_constellation           & 0x03) << 6
                               | (p_decoded->i_hierarchy_information   & 0x07) << 3
                               | (p_decoded->i_code_rate_hp_stream     & 0x07);
    p_descriptor->p_data[6]  =   (p_decoded->i_code_rate_lp_stream     & 0x07) << 5
                               | (p_decoded->i_guard_interval          & 0x03) << 3
                               | (p_decoded->i_transmission_mode       & 0x03) << 1
                               | (p_decoded->i_other_frequency_flag    & 0x01);
    p_descriptor->p_data[7]  =   0xff;
    p_descriptor->p_data[8]  =   0xff;
    p_descriptor->p_data[9]  =   0xff;
    p_descriptor->p_data[10] =   0xff;

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_terr_deliv_sys_dr_t * p_dup_decoded =
        (dvbpsi_terr_deliv_sys_dr_t*)malloc(sizeof(dvbpsi_terr_deliv_sys_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_terr_deliv_sys_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}
