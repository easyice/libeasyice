/*
Copyright (C) 2013-2014  Michael Ira Krufky

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

dr_a0.c

Decode Extended Channel Name Descriptor.

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

#include "dr_a0.h"

/*****************************************************************************
 * dvbpsi_ExtendedChannelNameDr
 *****************************************************************************/
dvbpsi_extended_channel_name_dr_t *dvbpsi_ExtendedChannelNameDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_extended_channel_name_dr_t *p_decoded;

    /* Check the tag */
    if (p_descriptor->i_tag != 0xA0)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check length */
    if (!p_descriptor->i_length)
        return NULL;

    p_decoded = (dvbpsi_extended_channel_name_dr_t*)malloc(sizeof(dvbpsi_extended_channel_name_dr_t));
    if (!p_decoded)
        return NULL;

    p_descriptor->p_decoded = (void*)p_decoded;

    p_decoded->i_long_channel_name_length = p_descriptor->i_length;
    memcpy(p_decoded->i_long_channel_name, p_descriptor->p_data, p_descriptor->i_length);

    return p_decoded;
}
