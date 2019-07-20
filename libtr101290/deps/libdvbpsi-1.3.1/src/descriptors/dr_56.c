/*****************************************************************************
 * dr_56.c
 * Copyright (C) 2004-2011 VideoLAN
 * $Id: dr_56.c 93 2004-10-19 19:17:49Z massiot $
 *
 * Authors: Derk-Jan Hartman <hartman at videolan dot org>
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

#include "dr_56.h"

/*****************************************************************************
 * dvbpsi_DecodeTeletextDr
 *****************************************************************************/
dvbpsi_teletext_dr_t * dvbpsi_DecodeTeletextDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x56) &&
            !dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x46))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Decode data and check the length */
    if(p_descriptor->i_length < 3)
        return NULL;

    if(p_descriptor->i_length % 5)
        return NULL;

    /* Allocate memory */
    dvbpsi_teletext_dr_t * p_decoded;
    p_decoded = (dvbpsi_teletext_dr_t*)malloc(sizeof(dvbpsi_teletext_dr_t));
    if (!p_decoded)
        return NULL;

    int i_pages_number;
    i_pages_number = p_descriptor->i_length / 5;
    if (i_pages_number > DVBPSI_TELETEXT_DR_MAX)
        i_pages_number = DVBPSI_TELETEXT_DR_MAX;
    p_decoded->i_pages_number = i_pages_number;

    for (int i = 0; i < i_pages_number; i++)
    {
        memcpy(p_decoded->p_pages[i].i_iso6392_language_code,
               p_descriptor->p_data + 5 * i, 3);

        p_decoded->p_pages[i].i_teletext_type =
                ((uint8_t)(p_descriptor->p_data[5 * i + 3]) >> 3);

        p_decoded->p_pages[i].i_teletext_magazine_number =
                ((uint16_t)(p_descriptor->p_data[5 * i + 3]) & 0x07);

        p_decoded->p_pages[i].i_teletext_page_number = p_descriptor->p_data[5 * i + 4];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenTeletextDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenTeletextDr(dvbpsi_teletext_dr_t * p_decoded,
                                           bool b_duplicate)
{
    if (p_decoded->i_pages_number > DVBPSI_TELETEXT_DR_MAX)
        p_decoded->i_pages_number = DVBPSI_TELETEXT_DR_MAX;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x56, p_decoded->i_pages_number * 5 , NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (int i = 0; i < p_decoded->i_pages_number; i++ )
    {
        memcpy( p_descriptor->p_data + 8 * i,
                p_decoded->p_pages[i].i_iso6392_language_code,
                3);

        p_descriptor->p_data[8 * i + 3] =
                (uint8_t) ( ( (uint8_t) p_decoded->p_pages[i].i_teletext_type << 3 ) |
                            ( (uint8_t) p_decoded->p_pages[i].i_teletext_magazine_number & 0x07 ) );

        p_descriptor->p_data[8 * i + 4] =
                p_decoded->p_pages[i].i_teletext_page_number;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_teletext_dr_t));
    }

    return p_descriptor;
}
