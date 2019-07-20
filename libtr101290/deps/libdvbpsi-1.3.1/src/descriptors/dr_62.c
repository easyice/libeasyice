/*
Copyright (C) 2006  Adam Charrett

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

dr_62.c

Decode Frequency List Descriptor.

*/
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

#include "dr_62.h"

/*****************************************************************************
 * dvbpsi_DecodeFrequencyListDr
 *****************************************************************************/
dvbpsi_frequency_list_dr_t *dvbpsi_DecodeFrequencyListDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_frequency_list_dr_t *p_decoded;
    int i;
    /* Check the tag */
    if (p_descriptor->i_tag != 0x62)
        return NULL;

    /* Don't decode twice */
    if (p_descriptor->p_decoded)
        return p_descriptor->p_decoded;

    /* Check length */
    if ((p_descriptor->i_length - 1) % 4)
        return NULL;

    p_decoded = (dvbpsi_frequency_list_dr_t*)malloc(sizeof(dvbpsi_frequency_list_dr_t));
    if (!p_decoded)
        return NULL;

    p_decoded->i_number_of_frequencies = (p_descriptor->i_length - 1) / 4;
    if (p_decoded->i_number_of_frequencies > ARRAY_SIZE(p_decoded->p_center_frequencies))
        p_decoded->i_number_of_frequencies = ARRAY_SIZE(p_decoded->p_center_frequencies);

    p_decoded->i_coding_type = p_descriptor->p_data[0] & 0x3;

    for (i = 0; i < p_decoded->i_number_of_frequencies; i ++)
    {
        p_decoded->p_center_frequencies[i] = (p_descriptor->p_data[(i * 4) + 1] << 24) |
                                             (p_descriptor->p_data[(i * 4) + 2] << 16) |
                                             (p_descriptor->p_data[(i * 4) + 3] <<  8) |
                                              p_descriptor->p_data[(i * 4) + 4];

        if ((p_decoded->i_coding_type == 1) || (p_decoded->i_coding_type == 2))
        {
            p_decoded->p_center_frequencies[i] = dvbpsi_Bcd8ToUint32(p_decoded->p_center_frequencies[i]);
        }

    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

uint32_t dvbpsi_Bcd8ToUint32(uint32_t bcd)
{
    uint32_t i_decoded;

    i_decoded  = ((bcd >> 28) & 0xf) * 10000000;
    i_decoded += ((bcd >> 24) & 0xf) *  1000000;
    i_decoded += ((bcd >> 20) & 0xf) *   100000;
    i_decoded += ((bcd >> 16) & 0xf) *    10000;
    i_decoded += ((bcd >> 12) & 0xf) *     1000;
    i_decoded += ((bcd >>  8) & 0xf) *      100;
    i_decoded += ((bcd >>  4) & 0xf) *       10;
    i_decoded += ((bcd >>  0) & 0xf) *        1;

    return i_decoded;
}
