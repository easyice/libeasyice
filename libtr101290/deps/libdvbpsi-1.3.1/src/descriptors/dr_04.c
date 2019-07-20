/*****************************************************************************
 * dr_04.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: dr_04.c,v 1.3 2003/07/25 20:20:40 fenrir Exp $
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

#include "dr_04.h"


/*****************************************************************************
 * dvbpsi_DecodeHierarchyDr
 *****************************************************************************/
dvbpsi_hierarchy_dr_t * dvbpsi_DecodeHierarchyDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
  dvbpsi_hierarchy_dr_t * p_decoded;

  /* Check the tag */
  if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x04))
     return NULL;

  /* Don't decode twice */
  if (dvbpsi_IsDescriptorDecoded(p_descriptor))
     return p_descriptor->p_decoded;

  /* Allocate memory */
  p_decoded = (dvbpsi_hierarchy_dr_t*)malloc(sizeof(dvbpsi_hierarchy_dr_t));
  if(!p_decoded) return NULL;

  /* Decode data and check the length */
  if(p_descriptor->i_length != 4)
  {
    free(p_decoded);
    return NULL;
  }

  p_decoded->i_h_type = p_descriptor->p_data[0] & 0x0f;
  p_decoded->i_h_layer_index = p_descriptor->p_data[1] & 0x3f;
  p_decoded->i_h_embedded_layer = p_descriptor->p_data[2] & 0x3f;
  p_decoded->i_h_priority = p_descriptor->p_data[3] & 0x3f;

  p_descriptor->p_decoded = (void*)p_decoded;

  return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenHierarchyDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenHierarchyDr(dvbpsi_hierarchy_dr_t * p_decoded,
                                            bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x04, 4, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = 0xf0 | p_decoded->i_h_type;
    p_descriptor->p_data[1] = 0xc0 | p_decoded->i_h_layer_index;
    p_descriptor->p_data[2] = 0xc0 | p_decoded->i_h_embedded_layer;
    p_descriptor->p_data[3] = 0xc0 | p_decoded->i_h_priority;

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_hierarchy_dr_t));
    }

    return p_descriptor;
}

