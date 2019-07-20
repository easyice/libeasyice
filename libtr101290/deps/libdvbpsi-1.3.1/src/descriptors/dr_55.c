/*****************************************************************************
 * dr_55.c
 * Copyright (C) 2004-2011 VideoLAN
 * $Id: dr_55.c 89 2004-06-28 19:17:23Z gbazin $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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

#include "dr_55.h"


/*****************************************************************************
 * dvbpsi_DecodeParentalRatingDr
 *****************************************************************************/
dvbpsi_parental_rating_dr_t * dvbpsi_DecodeParentalRatingDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x55))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Decode data and check the length */
    if(p_descriptor->i_length % 4)
        return NULL;

    /* Allocate memory */
    dvbpsi_parental_rating_dr_t * p_decoded;
    p_decoded = (dvbpsi_parental_rating_dr_t*)malloc(sizeof(dvbpsi_parental_rating_dr_t));
    if (!p_decoded)
        return NULL;

    int i_ratings_number = p_descriptor->i_length / 4;
    if (i_ratings_number > DVBPSI_PARENTAL_RATING_DR_MAX)
        i_ratings_number = DVBPSI_PARENTAL_RATING_DR_MAX;
    p_decoded->i_ratings_number = i_ratings_number;

    for (int i = 0; i < i_ratings_number; i++)
    {
        p_decoded->p_parental_rating[i].i_country_code =
                ((uint32_t)p_descriptor->p_data[4 * i] << 16)
                | ((uint32_t)p_descriptor->p_data[4 * i + 1] << 8)
                | p_descriptor->p_data[4 * i + 2];

        p_decoded->p_parental_rating[i].i_rating = p_descriptor->p_data[4 * i + 3];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenParentalRatingDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenParentalRatingDr(
                                        dvbpsi_parental_rating_dr_t * p_decoded,
                                        bool b_duplicate)
{
    uint8_t i_length;
    if (p_decoded->i_ratings_number >= DVBPSI_PARENTAL_RATING_DR_MAX)
    {
        i_length = (DVBPSI_PARENTAL_RATING_DR_MAX - 1) * 4;
        p_decoded->i_ratings_number = DVBPSI_PARENTAL_RATING_DR_MAX;
    }
    else
        i_length = p_decoded->i_ratings_number * 4;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x55, i_length, NULL);

    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (int i = 0; i < p_decoded->i_ratings_number; i++ )
    {
        p_descriptor->p_data[8 * i] =
                p_decoded->p_parental_rating[i].i_country_code >> 16;
        p_descriptor->p_data[8 * i + 1] =
                (p_decoded->p_parental_rating[i].i_country_code >> 8) & 0xff;
        p_descriptor->p_data[8 * i + 2] =
                p_decoded->p_parental_rating[i].i_country_code & 0xff;

        p_descriptor->p_data[8 * i + 3] =
                p_decoded->p_parental_rating[i].i_rating;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_parental_rating_dr_t));
    }

    return p_descriptor;
}
