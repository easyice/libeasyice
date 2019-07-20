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

dr_86.c

Decode Caption Service Descriptor.

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

#include "dr_86.h"

/*****************************************************************************
 * dvbpsi_DecodeCaptionServiceDr
 *****************************************************************************/
dvbpsi_caption_service_dr_t *dvbpsi_DecodeCaptionServiceDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_caption_service_dr_t *p_decoded;
    uint8_t * buf = p_descriptor->p_data;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x86))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Check length */
    if ((p_descriptor->i_length - 1) % 6)
        return NULL;

    p_decoded = (dvbpsi_caption_service_dr_t*)malloc(sizeof(dvbpsi_caption_service_dr_t));
    if (!p_decoded)
        return NULL;

    p_descriptor->p_decoded = (void*)p_decoded;

    p_decoded->i_number_of_services = 0x1f & buf[0];
    buf++;

    for (int i = 0; i < p_decoded->i_number_of_services; i++)
    {
        dvbpsi_caption_service_t * p_service = &p_decoded->services[i];

        memcpy(p_service->i_iso_639_code, buf, 3);
        buf += 3;
        p_service->b_digital_cc             = 0x01 & (buf[0] >> 7);
        p_service->b_line21_field           = 0x01 &  buf[0];
        p_service->i_caption_service_number = (p_service->b_digital_cc) ? 0x3F &  buf[0] : 0;
        buf++;
        p_service->b_easy_reader            = 0x01 & (buf[0] >> 7);
        p_service->b_wide_aspect_ratio      = 0x01 & (buf[0] >> 6);

        buf += 2;
    }
    return p_decoded;
}
