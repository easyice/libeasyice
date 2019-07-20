/*
 * dr_4b.c
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: Roberto Corno <corno.roberto@gmail.com>
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

#include "dr_4b.h"

/*****************************************************************************
 * dvbpsi_DecodeNVODReferenceDr
 *****************************************************************************/
dvbpsi_nvod_ref_dr_t* dvbpsi_DecodeNVODReferenceDr(dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (p_descriptor->i_tag != 0x4B)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check the length */
    if (p_descriptor->i_length < 6)
        return NULL;
    if (p_descriptor->i_length % 6!=0)
        return NULL;

    /* Allocate memory */
    dvbpsi_nvod_ref_dr_t * p_decoded;
    p_decoded = (dvbpsi_nvod_ref_dr_t*)calloc(1, sizeof(dvbpsi_nvod_ref_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_references = p_descriptor->i_length % 6;
    if (p_decoded->i_references > 43)
	p_decoded->i_references = 43;

    for (int i = 0; i < p_decoded->i_references; i++)
    {
      int pos = i*6;
      p_decoded->p_nvod_refs[i].i_transport_stream_id = p_descriptor->p_data[pos] << 8
                                                      | p_descriptor->p_data[pos+1];
      p_decoded->p_nvod_refs[i].i_original_network_id = p_descriptor->p_data[pos+2] << 8
                                                      | p_descriptor->p_data[pos+3];
      p_decoded->p_nvod_refs[i].i_service_id = p_descriptor->p_data[pos+4] << 8
                                                      | p_descriptor->p_data[pos+5];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenNVODReferenceDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenNVODReferenceDr(dvbpsi_nvod_ref_dr_t * p_decoded,
                                          bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x4b, p_decoded->i_references * 6, NULL);
    if (!p_descriptor)
        return NULL;

    if (p_decoded->i_references > 43)
	p_decoded->i_references = 43;

    /* Encode data */
    int pos = 0;
    for (int i = 0; i < p_decoded->i_references; i++ )
    {
    	p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_transport_stream_id >> 8;
        p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_transport_stream_id;
        p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_original_network_id >> 8;
        p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_original_network_id;
        p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_service_id >> 8;
        p_descriptor->p_data[pos++] = p_decoded->p_nvod_refs[i].i_service_id;
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                      sizeof(dvbpsi_nvod_ref_dr_t));
    }

    return p_descriptor;
}
