/*****************************************************************************
 * dr_0e.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: dr_0e.c,v 1.3 2003/07/25 20:20:40 fenrir Exp $
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

#include "dr_0e.h"


/*****************************************************************************
 * dvbpsi_DecodeMaxBitrateDr
 *****************************************************************************/
dvbpsi_max_bitrate_dr_t * dvbpsi_DecodeMaxBitrateDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_max_bitrate_dr_t * p_decoded;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x0e))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if (p_descriptor->i_length != 3)
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_max_bitrate_dr_t*)malloc(sizeof(dvbpsi_max_bitrate_dr_t));
    if (!p_decoded)
        return NULL;

    p_decoded->i_max_bitrate =
            ((uint32_t)(p_descriptor->p_data[0] & 0x3f) << 16)
            | ((uint32_t)(p_descriptor->p_data[1]) << 8)
            | p_descriptor->p_data[2];

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenMaxBitrateDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenMaxBitrateDr(dvbpsi_max_bitrate_dr_t * p_decoded,
                                             bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x0e, 3, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = 0xc0 | ((p_decoded->i_max_bitrate >> 16) & 0x3f);
    p_descriptor->p_data[1] = p_decoded->i_max_bitrate >> 8;
    p_descriptor->p_data[2] = p_decoded->i_max_bitrate;

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_max_bitrate_dr_t));
    }

    return p_descriptor;
}
