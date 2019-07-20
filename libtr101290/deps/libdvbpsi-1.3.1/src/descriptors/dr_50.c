/*
 * dr_50.c
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

#include "dr_50.h"

/*****************************************************************************
 * dvbpsi_DecodeComponentDr
 *****************************************************************************/
dvbpsi_component_dr_t* dvbpsi_DecodeComponentDr(dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (p_descriptor->i_tag != 0x50)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check the length */
    if (p_descriptor->i_length < 6)
        return NULL;

    /* Allocate memory */
    dvbpsi_component_dr_t * p_decoded;
    p_decoded = (dvbpsi_component_dr_t*)calloc(1, sizeof(dvbpsi_component_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_stream_content = p_descriptor->p_data[0] & 0x0F;
    p_decoded->i_component_type = p_descriptor->p_data[1];
    p_decoded->i_component_tag = p_descriptor->p_data[2];
    memcpy( &p_decoded->i_iso_639_code[0], &p_descriptor->p_data[3], 3 );
    if (p_descriptor->i_length > 6)
    {
    	p_decoded->i_text_length = p_descriptor->i_length - 6;
        p_decoded->i_text = calloc(1, p_decoded->i_text_length);
        if (!p_decoded->i_text)
        {
        	free(p_decoded);
            return NULL;
        }
    	memcpy( p_decoded->i_text, &p_descriptor->p_data[6], p_decoded->i_text_length );
    }
    else
    {
        p_decoded->i_text_length = 0;
        p_decoded->i_text = NULL;
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenComponentDr
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_GenComponentDr(dvbpsi_component_dr_t * p_decoded,
                                                  bool b_duplicate)
{
    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x50, 6+p_decoded->i_text_length, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
	p_descriptor->p_data[0] = p_decoded->i_stream_content+0xF0;
    p_descriptor->p_data[1] = p_decoded->i_component_type;
    p_descriptor->p_data[2] = p_decoded->i_component_tag;
    memcpy( &p_descriptor->p_data[3], p_decoded->i_iso_639_code, 3 );
    if (p_decoded->i_text_length)
      memcpy(&p_descriptor->p_data[6],p_decoded->i_text,p_decoded->i_text_length);

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                      sizeof(dvbpsi_component_dr_t));
    }

    return p_descriptor;
}
