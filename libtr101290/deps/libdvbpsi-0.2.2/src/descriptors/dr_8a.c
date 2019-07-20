/*****************************************************************************
 * dr_8a.c
 * Copyright (c) 2010 VideoLAN
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

#include "dr_8a.h"

/*****************************************************************************
 * dvbpsi_DecodeCUEIDr
 *****************************************************************************/
dvbpsi_cuei_dr_t * dvbpsi_DecodeCUEIDr(dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_cuei_dr_t *p_decoded;

  /* Check the tag */
  if (p_descriptor->i_tag != 0x8a)
  {
    DVBPSI_ERROR_ARG("dr_8a decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if (p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded = (dvbpsi_cuei_dr_t*)malloc(sizeof(dvbpsi_cuei_dr_t));
  if (!p_decoded)
  {
    DVBPSI_ERROR("dr_8a decoder", "out of memory");
    return NULL;
  }

  /* Decode data and check the length */
  if (p_descriptor->i_length == 0x01)
  {
    DVBPSI_ERROR_ARG("dr_8a decoder", "bad length (%d)",
                     p_descriptor->i_length);
    free(p_decoded);
    return NULL;
  }

  /* cue_stream_type according to: SCTE 35 2004 - p15 - table 6.3
   * cue_stream_type     PID usage
   *   0x00              splice_insert, splice_null, splice_schedule
   *   0x01              All Commands
   *   0x02              Segmentation
   *   0x03              Tiered Splicing
   *   0x04              Tiered Segmentation
   *   0x05 - 0x7f       Reserved
   *   0x80 - 0xff       User Defined
   */
  p_decoded->i_cue_stream_type = p_descriptor->p_data[0];

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenCUEIDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenCUEIDr(dvbpsi_cuei_dr_t * p_decoded)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
      dvbpsi_NewDescriptor(0x8a, 0x01, NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_cue_stream_type;
  }

  return p_descriptor;
}

