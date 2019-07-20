/*
 * dr_4a.c
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

#include "dr_4a.h"

/*****************************************************************************
 * dvbpsi_DecodeLinkageDr
 *****************************************************************************/
dvbpsi_linkage_dr_t* dvbpsi_DecodeLinkageDr(dvbpsi_descriptor_t * p_descriptor)
{
    /* Check the tag */
    if (p_descriptor->i_tag != 0x4A)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check the length */
    int handover_type = 0, origin_type = 0;
    if (p_descriptor->p_data[6] == 0x08)
    {
        handover_type = p_descriptor->p_data[7] & 0xF0 >> 4;
        origin_type = p_descriptor->p_data[7] & 0x01;
        if ((( handover_type > 0 ) && ( handover_type < 4 )
                && ( origin_type == 0 ) && ( p_descriptor->i_length > 243 )) ||
            (( handover_type > 0 ) && ( handover_type < 4 )
                && ( origin_type == 1 ) && ( p_descriptor->i_length > 245 )))
            return NULL;
    }
    if (p_descriptor->p_data[6] == 0x0D &&
        p_descriptor->i_length > 245)
        return NULL;
    if (p_descriptor->p_data[6] != 0x08 &&
        p_descriptor->p_data[6] != 0x0D &&
        p_descriptor->i_length > 248)
        return NULL;

    /* Allocate memory */
    dvbpsi_linkage_dr_t * p_decoded;
    p_decoded = (dvbpsi_linkage_dr_t*)calloc(1, sizeof(dvbpsi_linkage_dr_t));
    if (!p_decoded)
        return NULL;

    /* Decode data */
    int i = 7;
    p_decoded->i_transport_stream_id = p_descriptor->p_data[0] << 8
                                       | p_descriptor->p_data[1];
    p_decoded->i_original_network_id = p_descriptor->p_data[2] << 8
                                       | p_descriptor->p_data[3];
    p_decoded->i_service_id = p_descriptor->p_data[4] << 8
                              | p_descriptor->p_data[5];
    p_decoded->i_linkage_type = p_descriptor->p_data[6];

    if (p_descriptor->p_data[6] == 0x08)
    {
        p_decoded->i_handover_type = handover_type;
        p_decoded->i_origin_type = origin_type;
        if (handover_type > 0 && handover_type < 4)
        {
            p_decoded->i_network_id = p_descriptor->p_data[8] << 8
                                      | p_descriptor->p_data[9];
            i = 10;
        }
        if (origin_type == 0)
        {
            if (handover_type > 0 && handover_type < 4)
            {
                p_decoded->i_initial_service_id = p_descriptor->p_data[10] << 8
                                                  | p_descriptor->p_data[11];
                i = 12;
            }
            else
            {
                p_decoded->i_initial_service_id = p_descriptor->p_data[8] << 8
                                                  | p_descriptor->p_data[9];
                i = 10;
            }
        }
    }
    if (p_descriptor->p_data[6] == 0x0D)
    {
       p_decoded->i_target_event_id = p_descriptor->p_data[7] << 8
                                      | p_descriptor->p_data[8];
       p_decoded->b_target_listed = (p_descriptor->p_data[9] & 0x80) ? true : false;
       p_decoded->b_event_simulcast = (p_descriptor->p_data[9] & 0x40) ? true : false;
       i = 10;
    }
    p_decoded->i_private_data_length = p_descriptor->i_length - i;
    if (p_decoded->i_private_data_length > 248)
        p_decoded->i_private_data_length = 248;
    memcpy(p_decoded->i_private_data, &p_descriptor->p_data[i], p_decoded->i_private_data_length);

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenCountryAvailabilityDr
 *****************************************************************************/
dvbpsi_descriptor_t * dvbpsi_GenLinkageDr(dvbpsi_linkage_dr_t * p_decoded,
                                          bool b_duplicate)
{
    /* Check the length */
    int last_pos;
    int length = 7;

    if (p_decoded->i_linkage_type == 0x08)
    {
        length++;
        if ((p_decoded->i_handover_type > 0) &&
            (p_decoded->i_handover_type < 3))
        {
            length+=2;
	    if (p_decoded->i_origin_type == 0)
                length+=2;
	}
    }
    if (p_decoded->i_linkage_type == 0x0D)
        length+=3;
    if (length+p_decoded->i_private_data_length > 255)
        return NULL;

    /* Create the descriptor */
    dvbpsi_descriptor_t * p_descriptor =
            dvbpsi_NewDescriptor(0x4a, p_decoded->i_private_data_length+length, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = p_decoded->i_transport_stream_id >> 8;
    p_descriptor->p_data[1] = p_decoded->i_transport_stream_id;
    p_descriptor->p_data[2] = p_decoded->i_original_network_id >> 8;
    p_descriptor->p_data[3] = p_decoded->i_original_network_id;
    p_descriptor->p_data[4] = p_decoded->i_service_id >> 8;
    p_descriptor->p_data[5] = p_decoded->i_service_id;
    p_descriptor->p_data[6] = p_decoded->i_linkage_type;
    last_pos = 6;
    if (p_decoded->i_linkage_type == 0x08)
    {
    	p_descriptor->p_data[7] = ( (p_decoded->i_handover_type & 0x0F) << 4 )
    			| 0x0E | ( p_decoded->i_origin_type & 0x01 );
        if ((p_decoded->i_handover_type > 0) &&
            (p_decoded->i_handover_type < 3 ))
        {
    		p_descriptor->p_data[8] = p_decoded->i_network_id >> 8;
    		p_descriptor->p_data[9] = p_decoded->i_network_id;
    		last_pos = 9;
    	}
        if (p_decoded->i_origin_type == 0)
        {
                if ((p_decoded->i_handover_type > 0) &&
                    (p_decoded->i_handover_type < 3 ))
                {
        		p_descriptor->p_data[10] = p_decoded->i_initial_service_id >> 8;
        		p_descriptor->p_data[11] = p_decoded->i_initial_service_id;
        		last_pos = 11;
    		}
                else
                {
        		p_descriptor->p_data[8] = p_decoded->i_initial_service_id >> 8;
        		p_descriptor->p_data[9] = p_decoded->i_initial_service_id;
        		last_pos = 9;
    		}
    	}
    }

    if (p_decoded->i_linkage_type == 0x0D)
    {
    	p_descriptor->p_data[7] = p_decoded->i_target_event_id >> 8;
        p_descriptor->p_data[8] = p_decoded->i_target_event_id;
        p_descriptor->p_data[9] = ((p_decoded->b_target_listed) ? 0x80 : 0x00 )
                        | ((p_decoded->b_event_simulcast) ? 0x40 : 0x00) | 0x3F;
        last_pos = 9;
    }

    memcpy(&p_descriptor->p_data[last_pos+1], p_decoded->i_private_data, p_decoded->i_private_data_length);

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
               dvbpsi_DuplicateDecodedDescriptor(p_decoded,
                      sizeof(dvbpsi_linkage_dr_t));
    }

    return p_descriptor;
}
