/*
Copyright (C) 2012  Yonathan Yusim

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

dr_a1.c

Service Location Descriptor.

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

#include "dr_a1.h"


/*****************************************************************************
 * dvbpsi_DecodeServiceLocationDr
 *****************************************************************************/
dvbpsi_service_location_dr_t *
dvbpsi_DecodeServiceLocationDr (dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_service_location_dr_t *p_decoded;
    uint8_t *buf = p_descriptor->p_data;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0xa1))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Check length */
    if ((p_descriptor->i_length - 3) % 6)
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_service_location_dr_t *)
            malloc (sizeof (dvbpsi_service_location_dr_t));
    if (!p_decoded)
        return NULL;

    memset (p_decoded, 0, sizeof (dvbpsi_service_location_dr_t));

    p_descriptor->p_decoded = (void *) p_decoded;

    p_decoded->i_pcr_pid = ((uint16_t) (buf[0] & 0x1f) << 8) | buf[1];
    p_decoded->i_number_elements = buf[2];

    buf += 3;

    for (int i = 0; i < p_decoded->i_number_elements; i++)
    {
        dvbpsi_service_location_element_t *p_element = &p_decoded->elements[i];

        p_element->i_stream_type = buf[0];
        p_element->i_elementary_pid = ((uint16_t) (buf[1] & 0x1f) << 8) | buf[2];
        memcpy (p_element->i_iso_639_code, &buf[3], 3);

        buf += 6;
    }

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenServiceLocationDr
 *****************************************************************************/
dvbpsi_descriptor_t* dvbpsi_GenServiceLocationDr(
                                        dvbpsi_service_location_dr_t* p_decoded,
                                        bool b_duplicate)
{
    if (p_decoded->i_number_elements > 42)
        p_decoded->i_number_elements = 42;

    uint8_t i_desc_length = 3 + p_decoded->i_number_elements * 6;
    dvbpsi_descriptor_t* p_descriptor = dvbpsi_NewDescriptor(0xa1, i_desc_length, NULL);
    if (!p_descriptor)
        return NULL;

    uint8_t* p_data = p_descriptor->p_data;
    p_data[0] = p_decoded->i_pcr_pid >> 8;
    p_data[1] = p_decoded->i_pcr_pid;
    p_data[2] = p_decoded->i_number_elements;

    p_data += 3;
    for (uint8_t i = 0; i < p_decoded->i_number_elements; ++i)
    {
        dvbpsi_service_location_element_t p_es = p_decoded->elements[i];
        uint8_t* p_es_data = p_data;
        p_es_data[0] = p_es.i_stream_type;
        p_es_data[1] = p_es.i_elementary_pid >> 8;
        p_es_data[2] = p_es.i_elementary_pid;
        p_es_data[3] = p_es.i_iso_639_code[0];
        p_es_data[4] = p_es.i_iso_639_code[1];
        p_es_data[5] = p_es.i_iso_639_code[2];

        p_data += 6;
    }

    if (b_duplicate)
    {
        p_descriptor->p_decoded = dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                         sizeof(dvbpsi_service_location_dr_t));
    }

    return p_descriptor;
}
