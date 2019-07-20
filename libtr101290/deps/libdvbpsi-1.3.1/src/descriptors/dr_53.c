/*****************************************************************************
 * dr_53.c
 * Copyright (C) 2013, M2X BV
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
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

#include "dr_53.h"

/*****************************************************************************
 * dvbpsi_DecodeCAIdentifierDr
 *****************************************************************************/
dvbpsi_ca_identifier_dr_t * dvbpsi_DecodeCAIdentifierDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_ca_identifier_dr_t * p_decoded;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x53))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if (p_descriptor->i_length < 1)
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_ca_identifier_dr_t*)calloc(1, sizeof(dvbpsi_ca_identifier_dr_t));
    if (!p_decoded)
        return NULL;

    int i_number = p_descriptor->i_length / 2;
    if (i_number > DVBPSI_CA_SYSTEM_ID_DR_MAX)
        i_number = DVBPSI_CA_SYSTEM_ID_DR_MAX;
    p_decoded->i_number = i_number;

    for (int i = 0; i < i_number; i++)
    {
        /* TODO: decode CA system identifier values */
        p_decoded->p_system[i].i_ca_system_id = p_descriptor->p_data[2 * i];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenCAIdentifierDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenCAIdentifierDr(dvbpsi_ca_identifier_dr_t *p_decoded,
                                               bool b_duplicate)
{
    if (p_decoded->i_number > DVBPSI_CA_SYSTEM_ID_DR_MAX)
        p_decoded->i_number = DVBPSI_CA_SYSTEM_ID_DR_MAX;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x53, p_decoded->i_number * 2, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    for (int i = 0; i < p_decoded->i_number; i++ )
    {
        p_descriptor->p_data[2 * i] = p_decoded->p_system[i].i_ca_system_id >> 8;
        p_descriptor->p_data[2 * i + 1] = p_decoded->p_system[i].i_ca_system_id;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_ca_identifier_dr_t));
    }

    return p_descriptor;
}
