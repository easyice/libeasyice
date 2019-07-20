/*
Copyright (C) 2010-2012  Adam Charrett

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

dr_13.c

Decode Carousel Id Descriptor.

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

#include "dr_13.h"

static dvbpsi_carousel_id_dr_t *NewCarouselDr(const size_t i_private)
{
    dvbpsi_carousel_id_dr_t *p_carousel;
    if (i_private <= 0)
        return NULL;
    p_carousel = (dvbpsi_carousel_id_dr_t *)
                    calloc(1, sizeof(dvbpsi_carousel_id_dr_t) + i_private);
    if (p_carousel)
    {
        p_carousel->p_private_data = ((uint8_t *)p_carousel + sizeof(dvbpsi_carousel_id_dr_t));
        p_carousel->i_private_data_len = i_private;
    }
    return p_carousel;
}

/*****************************************************************************
 * dvbpsi_DecodeCarouselIdDr
 *****************************************************************************/
dvbpsi_carousel_id_dr_t *dvbpsi_DecodeCarouselIdDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_carousel_id_dr_t *p_decoded;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x13)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check length */
    if (p_descriptor->i_length < 4)
        return NULL;

    p_decoded = NewCarouselDr(p_descriptor->i_length - 4);
    if (!p_decoded)
        return NULL;

    p_decoded->i_carousel_id = ((p_descriptor->p_data[0] & 0xff) << 24) | ((p_descriptor->p_data[1] & 0xff) << 16)|
                               ((p_descriptor->p_data[2] & 0xff) <<  8) |  (p_descriptor->p_data[3] & 0xff);

    memcpy(p_decoded->p_private_data, &p_descriptor->p_data[4], p_decoded->i_private_data_len);
    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}
