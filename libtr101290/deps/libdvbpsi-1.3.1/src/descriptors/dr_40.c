/*
 * dr_40.c
 * Copyright (C) 2001-2012 VideoLAN
 *
 * Authors: rcorno (Nov 22, 2011)
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

#include "dr_40.h"

/*****************************************************************************
 * dvbpsi_DecodeNetworkNameDr
 *****************************************************************************/
dvbpsi_network_name_dr_t* dvbpsi_DecodeNetworkNameDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_network_name_dr_t * p_decoded;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x40))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
       return p_descriptor->p_decoded;

    /* Allocate memory */
    p_decoded = (dvbpsi_network_name_dr_t*)calloc(1, sizeof(dvbpsi_network_name_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_name_length = p_descriptor->i_length;

    if (p_decoded->i_name_length)
        memcpy(p_decoded->i_name_byte,
               p_descriptor->p_data,
               p_decoded->i_name_length);

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenNetworkNameDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenNetworkNameDr(
                                        dvbpsi_network_name_dr_t * p_decoded,
                                        bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x40, p_decoded->i_name_length, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    if (p_decoded->i_name_length)
        memcpy(p_descriptor->p_data,
               p_decoded->i_name_byte,
               p_decoded->i_name_length);

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_network_name_dr_t));
    }

    return p_descriptor;
}
