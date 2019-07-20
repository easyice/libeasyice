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

dr_76.c

Decode Content Identifier Descriptor.

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

#include "dr_76.h"

/*****************************************************************************
 * dvbpsi_DecodeLCNDr
 *****************************************************************************/
dvbpsi_content_id_dr_t *dvbpsi_DecodeContentIdDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_content_id_dr_t *p_decoded;
    int byte;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x76)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check boundaries */
    if (p_descriptor->i_length > ARRAY_SIZE(p_decoded->p_entries))
        p_descriptor->i_length = ARRAY_SIZE(p_decoded->p_entries);

    p_decoded = (dvbpsi_content_id_dr_t*)malloc(sizeof(dvbpsi_content_id_dr_t));
    if (!p_decoded)
        return NULL;

    p_decoded->i_number_of_entries = 0;
    for (byte = 0;
         (byte < p_descriptor->i_length) && (p_decoded->i_number_of_entries < DVBPSI_CRID_ENTRY_DR_MAX);
         p_decoded->i_number_of_entries ++)
    {
        dvbpsi_crid_entry_t *entry = &p_decoded->p_entries[p_decoded->i_number_of_entries];

        entry->i_type = (p_descriptor->p_data[byte] >> 2) & 0x3f;
        entry->i_location = p_descriptor->p_data[byte] & 3;
        byte ++;

        if (entry->i_location == CRID_LOCATION_DESCRIPTOR)
        {
            uint8_t len = p_descriptor->p_data[byte];
            if (len > 253)
                len = 253;

            unsigned int i;
            byte ++;
            for (i = 0; i < len; i ++)
            {
                entry->value.path[i] = p_descriptor->p_data[byte + i];
            }
            byte += len;
            /* Properly terminate the string */
            unsigned int last = (i < len) ? i : len - 1U;
            entry->value.path[last] = 0;
        }
        else if (entry->i_location == CRID_LOCATION_CIT)
        {
            entry->value.ref = (p_descriptor->p_data[byte] << 8) | p_descriptor->p_data[byte + 1];
            byte += 2;
        }
        else
        {
            /* Unknown location */
            free(p_decoded);
            return NULL;
        }
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}
