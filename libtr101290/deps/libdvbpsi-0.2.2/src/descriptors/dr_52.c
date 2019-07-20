/*****************************************************************************
 * dr_52.c
 * Copyright (C) 2005-2010 Andrew John Hughes
 *
 * Authors: Andrew John Hughes <gnu_andrew@member.fsf.org>
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

#include "dr_52.h"


/*****************************************************************************
 * dvbpsi_DecodeStreamIdentifierDr
 *****************************************************************************/
dvbpsi_stream_identifier_dr_t * dvbpsi_DecodeStreamIdentifierDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_stream_identifier_dr_t * p_decoded;

  /* Check the tag */
  if(p_descriptor->i_tag != 0x52)
  {
    DVBPSI_ERROR_ARG("dr_52 decoder", "bad tag (0x%x)", p_descriptor->i_tag);
    return NULL;
  }

  /* Don't decode twice */
  if(p_descriptor->p_decoded)
    return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded =
        (dvbpsi_stream_identifier_dr_t*)malloc(sizeof(dvbpsi_stream_identifier_dr_t));
  if(!p_decoded)
  {
    DVBPSI_ERROR("dr_52 decoder", "out of memory");
    return NULL;
  }

  /* Decode data and check the length */
  if(p_descriptor->i_length < 1)
  {
    DVBPSI_ERROR_ARG("dr_52 decoder", "bad length (%d)",
                     p_descriptor->i_length);
    free(p_decoded);
    return NULL;
  }

  p_decoded->i_component_tag = p_descriptor->p_data[0];

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenStreamIdentifierDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenStreamIdentifierDr(
                                        dvbpsi_stream_identifier_dr_t * p_decoded,
                                        int b_duplicate)
{
  /* Create the descriptor */
  dvbpsi_descriptor_t * p_descriptor =
        dvbpsi_NewDescriptor(0x52, 1, NULL);

  if(p_descriptor)
  {
    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_component_tag;

    if(b_duplicate)
    {
      /* Duplicate decoded data */
      dvbpsi_stream_identifier_dr_t * p_dup_decoded =
        (dvbpsi_stream_identifier_dr_t*)malloc(sizeof(dvbpsi_stream_identifier_dr_t));
      if(p_dup_decoded)
        memcpy(p_dup_decoded, p_decoded, sizeof(dvbpsi_stream_identifier_dr_t));

      p_descriptor->p_decoded = (void*)p_dup_decoded;
    }
  }

  return p_descriptor;
}

