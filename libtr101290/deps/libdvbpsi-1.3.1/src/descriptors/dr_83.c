/*
Copyright (C) 2006  Adam Charrett

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

dr_83.c

Decode Logical Channel Number Descriptor.

*/
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

#include "dr_83.h"

/*****************************************************************************
 * dvbpsi_DecodeLCNDr
 *****************************************************************************/
dvbpsi_lcn_dr_t *dvbpsi_DecodeLCNDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_lcn_dr_t *p_decoded;
    int i;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x83)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check length */
    if (p_descriptor->i_length % 4)
        return NULL;

    p_decoded = (dvbpsi_lcn_dr_t*)malloc(sizeof(dvbpsi_lcn_dr_t));
    if (!p_decoded)
        return NULL;

    p_decoded->i_number_of_entries = p_descriptor->i_length / 4;
    if (p_decoded->i_number_of_entries > ARRAY_SIZE(p_decoded->p_entries))
        p_decoded->i_number_of_entries = ARRAY_SIZE(p_decoded->p_entries);

    for (i = 0; i < p_decoded->i_number_of_entries; i ++)
    {
        p_decoded->p_entries[i].i_service_id = (p_descriptor->p_data[i * 4] << 8) |
                                                p_descriptor->p_data[(i * 4) + 1];

        p_decoded->p_entries[i].b_visible_service_flag = (p_descriptor->p_data[(i * 4) + 2] >> 7) & 1;

        p_decoded->p_entries[i].i_logical_channel_number = ((p_descriptor->p_data[(i * 4) + 2] << 8) |
                                                             p_descriptor->p_data[(i * 4) + 3]) & 0x3ff;

    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenLCNDr
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_GenLCNDr(dvbpsi_lcn_dr_t* p_decoded,
                                       bool b_duplicate)
{
    if (p_decoded->i_number_of_entries > 63)
        p_decoded->i_number_of_entries = 63;

    dvbpsi_descriptor_t* p_descriptor = dvbpsi_NewDescriptor(0x83, p_decoded->i_number_of_entries * 4, 0);
    if (NULL == p_descriptor)
        return NULL;

    for (int i = 0; i < p_decoded->i_number_of_entries; i++ )
    {
        p_descriptor->p_data[4 * i] = p_decoded->p_entries[i].i_service_id >> 8;
        p_descriptor->p_data[4 * i + 1] = p_decoded->p_entries[i].i_service_id;
        p_descriptor->p_data[4 * i + 2] = (p_decoded->p_entries[i].b_visible_service_flag << 7);
        p_descriptor->p_data[4 * i + 2] |= (p_decoded->p_entries[i].i_logical_channel_number >> 8);
        p_descriptor->p_data[4 * i + 3] = p_decoded->p_entries[i].i_logical_channel_number;
    }

    if (b_duplicate)
    {
        dvbpsi_lcn_dr_t * p_dup = (dvbpsi_lcn_dr_t*) malloc(sizeof (dvbpsi_lcn_dr_t));
        if (NULL != p_dup)
            memcpy(p_dup, p_decoded, sizeof(dvbpsi_lcn_dr_t));
        p_descriptor->p_decoded = (void*)p_dup;
    }

    return p_descriptor;
}

