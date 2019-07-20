/*****************************************************************************
 * dr_0a.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#include "dr_0a.h"


/*****************************************************************************
 * dvbpsi_DecodeISO639Dr
 *****************************************************************************/
dvbpsi_iso639_dr_t * dvbpsi_DecodeISO639Dr(dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_iso639_dr_t * p_decoded;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x0a))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if ((p_descriptor->i_length < 1) ||
        (p_descriptor->i_length % 4 != 0))
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_iso639_dr_t*)malloc(sizeof(dvbpsi_iso639_dr_t));
    if (!p_decoded)
        return NULL;

    p_decoded->i_code_count = p_descriptor->i_length / 4;
    if (p_decoded->i_code_count > 64)
        p_decoded->i_code_count = 64;

    int i = 0;
    while( i < p_decoded->i_code_count )
    {
        p_decoded->code[i].iso_639_code[0] = p_descriptor->p_data[i*4];
        p_decoded->code[i].iso_639_code[1] = p_descriptor->p_data[i*4+1];
        p_decoded->code[i].iso_639_code[2] = p_descriptor->p_data[i*4+2];
        p_decoded->code[i].i_audio_type = p_descriptor->p_data[i*4+3];
        i++;
    }
    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenISO639Dr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenISO639Dr(dvbpsi_iso639_dr_t * p_decoded,
                                         bool b_duplicate)
{
    if (p_decoded->i_code_count > 64)
        p_decoded->i_code_count = 64;

    /* Create the descriptor */
    uint8_t i_size = (p_decoded->i_code_count * 4) > UINT8_MAX ? 255 : p_decoded->i_code_count * 4;
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x0a, i_size, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    int i = 0;
    while( i < p_decoded->i_code_count )
    {
        p_descriptor->p_data[i*4] = p_decoded->code[i].iso_639_code[0];
        p_descriptor->p_data[i*4+1] = p_decoded->code[i].iso_639_code[1];
        p_descriptor->p_data[i*4+2] = p_decoded->code[i].iso_639_code[2];
        p_descriptor->p_data[i*4+3] = p_decoded->code[i].i_audio_type;
        i++;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_iso639_dr_t));
    }

    return p_descriptor;
}

