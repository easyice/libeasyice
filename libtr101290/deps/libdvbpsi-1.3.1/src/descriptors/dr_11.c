/*
Copyright (C) 2015 Daniel Kamil Kozar

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
*/

#include "config.h"

#include <stdlib.h>
#include <stdbool.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"

#include "dr_11.h"

dvbpsi_std_dr_t* dvbpsi_DecodeSTDDr(dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_std_dr_t * p_decoded;

    /* check the tag. */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x11))
        return NULL;

    /* don't decode twice. */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* all descriptors of this type have 1 byte of payload. */
    if (p_descriptor->i_length != 1)
        return NULL;

    p_decoded = (dvbpsi_std_dr_t*)malloc(sizeof(*p_decoded));
    if (!p_decoded)
        return NULL;

    p_decoded->b_leak_valid_flag = p_descriptor->p_data[0] & 0x01;

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

dvbpsi_descriptor_t * dvbpsi_GenSTDDr(dvbpsi_std_dr_t * p_decoded)
{
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x11, 1, NULL);
    if (!p_descriptor)
        return NULL;

    /* encode the data, making sure the reserved fields are set to all ones. */
    p_descriptor->p_data[0] = (0xfe | p_decoded->b_leak_valid_flag);

    return p_descriptor;
}
