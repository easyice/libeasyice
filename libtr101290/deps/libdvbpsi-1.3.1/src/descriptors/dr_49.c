/*
 * dr_49.c
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: rcorno (May 21, 2012)
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

#include "dr_49.h"

/*****************************************************************************
 * dvbpsi_DecodeCountryAvailability
 *****************************************************************************/
dvbpsi_country_availability_dr_t* dvbpsi_DecodeCountryAvailability(
                                        dvbpsi_descriptor_t * p_descriptor)
{
    dvbpsi_country_availability_dr_t * p_decoded;

    /* Check the tag */
    if (p_descriptor->i_tag != 0x49)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check the length */
    unsigned int code_count = (p_descriptor->i_length-1) / 3;
    if ((p_descriptor->i_length < 1) ||
        ((p_descriptor->i_length-1) % 3 != 0) ||
         (code_count > 83))
        return NULL;

    /* Allocate memory */
    p_decoded = (dvbpsi_country_availability_dr_t*)calloc(1, sizeof(dvbpsi_country_availability_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    p_decoded->i_code_count = code_count;
    p_decoded->b_country_availability_flag = p_descriptor->p_data[0] & 0x80;

    for (uint8_t i = 0; i < p_decoded->i_code_count; i++)
    {
    	p_decoded->code[i].iso_639_code[0] = p_descriptor->p_data[1+i*3];
    	p_decoded->code[i].iso_639_code[1] = p_descriptor->p_data[2+i*3];
    	p_decoded->code[i].iso_639_code[2] = p_descriptor->p_data[3+i*3];
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}


/*****************************************************************************
 * dvbpsi_GenCountryAvailabilityDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenCountryAvailabilityDr(
		                        dvbpsi_country_availability_dr_t * p_decoded,
                                        bool b_duplicate)
{
    /* Check the length */    
    if (p_decoded->i_code_count > 83) 
        return NULL;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x83, 1+p_decoded->i_code_count*3, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = (p_decoded->b_country_availability_flag) ? 0x80: 0x00;

    for (uint8_t i = 0; i < p_decoded->i_code_count; i++)
    {
        p_descriptor->p_data[1+i*3] = p_decoded->code[i].iso_639_code[0];
        p_descriptor->p_data[2+i*3] = p_decoded->code[i].iso_639_code[1];
        p_descriptor->p_data[3+i*3] = p_decoded->code[i].iso_639_code[2];
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                      sizeof(dvbpsi_country_availability_dr_t));
    }

    return p_descriptor;
}
