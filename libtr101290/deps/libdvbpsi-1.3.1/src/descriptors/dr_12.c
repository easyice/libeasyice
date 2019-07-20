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

#include "dr_12.h"

dvbpsi_ibp_dr_t* dvbpsi_DecodeIBPDr(dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_ibp_dr_t * p_decoded;
    
    /* check the tag. */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x12))
        return NULL;
    
    /* don't decode twice. */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;
    
    /* all descriptors of this type have 2 bytes of payload. */
    if (p_descriptor->i_length != 2)
        return NULL;
    
    p_decoded = (dvbpsi_ibp_dr_t*)malloc(sizeof(*p_decoded));
    if (!p_decoded)
        return NULL;
    
    p_decoded->b_closed_gop_flag = p_descriptor->p_data[0] & 0x80;
    p_decoded->b_identical_gop_flag = p_descriptor->p_data[0] & 0x40;
    p_decoded->i_max_gop_length = 
        (((uint16_t)p_descriptor->p_data[0] & 0x3f) << 8)
        | p_descriptor->p_data[1];
    
    /* a value of 0 is forbidden for max_gop_length. */
    if(p_decoded->i_max_gop_length == 0)
    {
        free(p_decoded);
        return NULL;
    }
    
    p_descriptor->p_decoded = (void*)p_decoded;
    
    return p_decoded;
}

dvbpsi_descriptor_t * dvbpsi_GenIBPDr(dvbpsi_ibp_dr_t * p_decoded)
{
    dvbpsi_descriptor_t * p_descriptor = dvbpsi_NewDescriptor(0x12, 2, NULL);
    if (!p_descriptor)
        return NULL;
    
    /* encode the data. */
    p_descriptor->p_data[0]  = p_decoded->b_closed_gop_flag << 7;
    p_descriptor->p_data[0] |= p_decoded->b_identical_gop_flag << 6;
    p_descriptor->p_data[0] |= p_decoded->i_max_gop_length >> 8;
    
    p_descriptor->p_data[1] = p_decoded->i_max_gop_length;
    
    return p_descriptor;
}
