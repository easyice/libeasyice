/*
Copyright (C) 2010  Adam Charrett

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

dr_66.c

Decode Data Broadcast Id Descriptor.

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

#include "dr_66.h"

static dvbpsi_data_broadcast_id_dr_t *NewDataBroadcastDr(const size_t i_private)
{
    dvbpsi_data_broadcast_id_dr_t *p_bcast;

    if (i_private <= 0)
        return NULL;

    p_bcast = (dvbpsi_data_broadcast_id_dr_t *)
                calloc(1, sizeof(dvbpsi_data_broadcast_id_dr_t) + i_private);
    if (p_bcast)
    {
        p_bcast->p_id_selector = ((uint8_t *)p_bcast + sizeof(dvbpsi_data_broadcast_id_dr_t));
        p_bcast->i_id_selector_len = i_private;
    }
    return p_bcast;
}

/*****************************************************************************
 * dvbpsi_DecodeDataBroadcastIdDr
 *****************************************************************************/
dvbpsi_data_broadcast_id_dr_t *dvbpsi_DecodeDataBroadcastIdDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_data_broadcast_id_dr_t *p_decoded;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x66)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check length */
    if (p_descriptor->i_length < 2)
        return NULL;

    p_decoded = NewDataBroadcastDr(p_descriptor->i_length - 2);
    if (!p_decoded)
        return NULL;

    p_decoded->i_data_broadcast_id = ((p_descriptor->p_data[0] & 0xff) << 8) | (p_descriptor->p_data[1] & 0xff);
    memcpy(p_decoded->p_id_selector, &p_descriptor->p_data[2], p_decoded->i_id_selector_len);
    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}
