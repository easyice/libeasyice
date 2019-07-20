/*****************************************************************************
 * dr_4c.c
 * Copyright (C) 2013, VideoLAN Association
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

#include "dr_4c.h"

/*****************************************************************************
 * dvbpsi_DecodeTimeShiftedServiceDr
 *****************************************************************************/
dvbpsi_tshifted_service_dr_t* dvbpsi_DecodeTimeShiftedServiceDr(dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x4c))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if (p_descriptor->i_length < 2)
        return NULL;

    /* Allocate memory */
    dvbpsi_tshifted_service_dr_t *p_decoded;
    p_decoded = (dvbpsi_tshifted_service_dr_t*)calloc(1, sizeof(dvbpsi_tshifted_service_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_ref_service_id = p_descriptor->p_data[0] << 8
                                | p_descriptor->p_data[1];

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenTimeShiftedServiceDr
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_GenTimeShiftedServiceDr(dvbpsi_tshifted_service_dr_t *p_decoded,
                                                    bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x4c, 2, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_ref_service_id >> 8;
    p_descriptor->p_data[1] = p_decoded->i_ref_service_id;

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                      sizeof(dvbpsi_tshifted_service_dr_t));
    }

    return p_descriptor;
}
