/*
 * dr_41.c
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: rcorno (May 21, 2012)
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

#include "dr_41.h"

/*****************************************************************************
 * dvbpsi_DecodeServiceListDr
 *****************************************************************************/
dvbpsi_service_list_dr_t* dvbpsi_DecodeServiceListDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_service_list_dr_t * p_decoded;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x41)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check the length */
    unsigned int service_count = p_descriptor->i_length / 3;
    if ((p_descriptor->i_length < 1) ||
        (p_descriptor->i_length % 3 != 0) ||
        (service_count>63))
      return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_service_list_dr_t*)calloc(1, sizeof(dvbpsi_service_list_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_service_count = service_count;

    for (uint8_t i = 0; i < p_decoded->i_service_count; i++ )
    {
    	p_decoded->i_service[i].i_service_id = ((uint16_t)(p_descriptor->p_data[i*3]) << 8)
                                      | p_descriptor->p_data[i*3+1];
    	p_decoded->i_service[i].i_service_type = p_descriptor->p_data[i*3+2];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenServiceListDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenServiceListDr(
		                        dvbpsi_service_list_dr_t * p_decoded,
                                        bool b_duplicate)
{
    /* Check the length */
    if (p_decoded->i_service_count > 63)
        return NULL;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x83, p_decoded->i_service_count*3, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (uint8_t i = 0; i < p_decoded->i_service_count; i++)
    {
        p_descriptor->p_data[i*3] = p_decoded->i_service[i].i_service_id >> 8;
        p_descriptor->p_data[i*3+1] = p_decoded->i_service[i].i_service_id;
        p_descriptor->p_data[i*3+2] = p_decoded->i_service[i].i_service_type;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                       sizeof(dvbpsi_service_list_dr_t));
    }

    return p_descriptor;
}
