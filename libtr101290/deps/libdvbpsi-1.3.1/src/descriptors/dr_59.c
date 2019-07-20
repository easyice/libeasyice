/*****************************************************************************
 * dr_59.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Tristan Leteurtre <tristan.leteurtre@anevia.com>
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

#include "dr_59.h"


/*****************************************************************************
 * dvbpsi_DecodeSubtitlingDr
 *****************************************************************************/
dvbpsi_subtitling_dr_t * dvbpsi_DecodeSubtitlingDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    int i_subtitles_number;
    dvbpsi_subtitling_dr_t * p_decoded;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x59))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Decode data and check the length */
    if (p_descriptor->i_length < 3)
        return NULL;

    if (p_descriptor->i_length % 8)
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_subtitling_dr_t*)malloc(sizeof(dvbpsi_subtitling_dr_t));
    if (!p_decoded)
        return NULL;

    i_subtitles_number = p_descriptor->i_length / 8;
    if (i_subtitles_number > DVBPSI_SUBTITLING_DR_MAX)
        i_subtitles_number = DVBPSI_SUBTITLING_DR_MAX;
    p_decoded->i_subtitles_number = i_subtitles_number;

    for (int i = 0; i < i_subtitles_number; i++)
    {
        memcpy(p_decoded->p_subtitle[i].i_iso6392_language_code,
               p_descriptor->p_data + 8 * i, 3);

        p_decoded->p_subtitle[i].i_subtitling_type =
                p_descriptor->p_data[8 * i + 3];

        p_decoded->p_subtitle[i].i_composition_page_id =
                ((uint16_t)(p_descriptor->p_data[8 * i + 4]) << 8)
                | p_descriptor->p_data[8 * i + 5];

        p_decoded->p_subtitle[i].i_ancillary_page_id =
                ((uint16_t)(p_descriptor->p_data[8 * i + 6]) << 8)
                | p_descriptor->p_data[8 * i + 7];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenSubtitlingDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenSubtitlingDr(
                                        dvbpsi_subtitling_dr_t * p_decoded,
                                        bool b_duplicate)
{
    if (p_decoded->i_subtitles_number > DVBPSI_SUBTITLING_DR_MAX)
        p_decoded->i_subtitles_number = DVBPSI_SUBTITLING_DR_MAX;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x59, p_decoded->i_subtitles_number * 8 , NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (int i = 0; i < p_decoded->i_subtitles_number; i++ )
    {
        memcpy( p_descriptor->p_data + 8 * i,
                p_decoded->p_subtitle[i].i_iso6392_language_code,
                3);

        p_descriptor->p_data[8 * i + 3] =
                p_decoded->p_subtitle[i].i_subtitling_type;

        p_descriptor->p_data[8 * i + 4] =
                p_decoded->p_subtitle[i].i_composition_page_id >> 8;
        p_descriptor->p_data[8 * i + 5] =
                p_decoded->p_subtitle[i].i_composition_page_id % 0xFF;

        p_descriptor->p_data[8 * i + 6] =
                p_decoded->p_subtitle[i].i_ancillary_page_id >> 8;
        p_descriptor->p_data[8 * i + 7] =
                p_decoded->p_subtitle[i].i_ancillary_page_id % 0xFF;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_subtitling_dr_t));
    }

    return p_descriptor;
}
