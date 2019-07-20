/*****************************************************************************
 * dr_54.c
 * Copyright (C) 2013 VideoLAN
 *
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

#include "dr_54.h"


/*****************************************************************************
 * dvbpsi_DecodeContentDr
 *****************************************************************************/
dvbpsi_content_dr_t * dvbpsi_DecodeContentDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x54))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Decode data and check the length */
    if(p_descriptor->i_length % 2)
        return NULL;

    /* Allocate memory */
    dvbpsi_content_dr_t * p_decoded;
    p_decoded = (dvbpsi_content_dr_t*)malloc(sizeof(dvbpsi_content_dr_t));
    if (!p_decoded)
        return NULL;

    int i_contents_number = p_descriptor->i_length / 2;
    if (i_contents_number > DVBPSI_CONTENT_DR_MAX)
        i_contents_number = DVBPSI_CONTENT_DR_MAX;
    p_decoded->i_contents_number = i_contents_number;

    for (int i = 0; i < i_contents_number; i++)
    {
        p_decoded->p_content[i].i_type = p_descriptor->p_data[2 * i];
        p_decoded->p_content[i].i_user_byte = p_descriptor->p_data[2 * i + 1];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenContentDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenContentDr(
                                        dvbpsi_content_dr_t * p_decoded,
                                        bool b_duplicate)
{
    if (p_decoded->i_contents_number > DVBPSI_CONTENT_DR_MAX)
        p_decoded->i_contents_number = DVBPSI_CONTENT_DR_MAX;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x54, p_decoded->i_contents_number * 2 , NULL);

    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (int i = 0; i < p_decoded->i_contents_number; i++ )
    {
        p_descriptor->p_data[8 * i] = p_decoded->p_content[i].i_type;
        p_descriptor->p_data[8 * i + 1] = p_decoded->p_content[i].i_user_byte;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_content_dr_t));
    }

    return p_descriptor;
}
