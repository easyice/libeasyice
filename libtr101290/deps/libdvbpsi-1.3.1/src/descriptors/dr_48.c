/*****************************************************************************
 * dr_48.c
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Johan Bilien <jobi@via.ecp.fr>
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

#include "dr_48.h"


/*****************************************************************************
 * dvbpsi_DecodeServiceDr
 *****************************************************************************/
dvbpsi_service_dr_t * dvbpsi_DecodeServiceDr(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x48))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if (p_descriptor->i_length < 3)
        return NULL;

    /* Allocate memory */
    dvbpsi_service_dr_t * p_decoded;
    p_decoded = (dvbpsi_service_dr_t*)calloc(1, sizeof(dvbpsi_service_dr_t));
    if (!p_decoded)
        return NULL;

    p_descriptor->p_decoded = (void*)p_decoded;

    p_decoded->i_service_type = p_descriptor->p_data[0];
    p_decoded->i_service_provider_name_length = p_descriptor->p_data[1];
    p_decoded->i_service_name_length = 0;
    p_decoded->i_service_provider_name[0] = 0;
    p_decoded->i_service_name[0] = 0;

    if (p_decoded->i_service_provider_name_length > 252)
        p_decoded->i_service_provider_name_length = 252;

    if (p_decoded->i_service_provider_name_length + 2 > p_descriptor->i_length)
        return p_decoded;

    if (p_decoded->i_service_provider_name_length)
        memcpy(p_decoded->i_service_provider_name,
               p_descriptor->p_data + 2,
               p_decoded->i_service_provider_name_length);

    if (p_decoded->i_service_provider_name_length + 3 > p_descriptor->i_length)
        return p_decoded;

    p_decoded->i_service_name_length =
            p_descriptor->p_data[2+p_decoded->i_service_provider_name_length];

    if (p_decoded->i_service_name_length > 252)
        p_decoded->i_service_name_length = 252;

    if (p_decoded->i_service_provider_name_length + 3 +
            p_decoded->i_service_name_length > p_descriptor->i_length)
        return p_decoded;

    if (p_decoded->i_service_name_length)
        memcpy(p_decoded->i_service_name,
               p_descriptor->p_data + 3 + p_decoded->i_service_provider_name_length,
               p_decoded->i_service_name_length);

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenServiceDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenServiceDr(dvbpsi_service_dr_t * p_decoded,
                                          bool b_duplicate)
{
    if (p_decoded->i_service_provider_name_length > 252)
        p_decoded->i_service_provider_name_length = 252;
    if (p_decoded->i_service_name_length > 252)
        p_decoded->i_service_name_length = 252;

    /* FIXME: is this correct? A descriptor cannot be more then 255 bytes due to
     * the function prototype definition */
    uint8_t i_size = 0;
    int i_length = 3 + p_decoded->i_service_name_length + p_decoded->i_service_provider_name_length;
    i_size = (i_length >= UINT8_MAX) ? 255 : i_length;

    /* Create the descriptor */
    dvbpsi_descriptor_t *p_descriptor = dvbpsi_NewDescriptor(0x48, i_size, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_service_type;
    p_descriptor->p_data[1] = p_decoded->i_service_provider_name_length;
    if (p_decoded->i_service_provider_name_length)
        memcpy(p_descriptor->p_data + 2,
               p_decoded->i_service_provider_name,
               p_decoded->i_service_provider_name_length);
    p_descriptor->p_data[2+p_decoded->i_service_provider_name_length] =
            p_decoded->i_service_name_length;
    if (p_decoded->i_service_name_length)
        memcpy(p_descriptor->p_data + 3 + p_decoded->i_service_provider_name_length,
               p_decoded->i_service_name,
               p_decoded->i_service_name_length);

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                                                  sizeof(dvbpsi_service_dr_t));
    }

    return p_descriptor;
}
