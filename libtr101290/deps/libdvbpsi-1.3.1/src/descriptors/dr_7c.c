/*****************************************************************************
 * dr_7c.c
 * Copyright (c) 2012 VideoLAN
 * $Id$
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

#include "dr_7c.h"

/* */
struct dvbpsi_aac_profile_and_level_table_s
{
    uint8_t hex;
    dvbpsi_aac_profile_and_level_t profile_and_level;
};
static struct dvbpsi_aac_profile_and_level_table_s aac_profile_and_level_table[] =
{
    { 0x00, DVBPSI_AAC_PROFILE_RESERVED },
    /* 0x00-0x0E Reserved */
    { 0x0F, DVBPSI_AAC_PROFILE_NOT_DEFINED },
    { 0x10, DVBPSI_AAC_PROFILE_MAIN_LEVEL_1 },
    { 0x11, DVBPSI_AAC_PROFILE_MAIN_LEVEL_2 },
    { 0x12, DVBPSI_AAC_PROFILE_MAIN_LEVEL_3 },
    { 0x13, DVBPSI_AAC_PROFILE_MAIN_LEVEL_4 },
    /* 0x14-0x17 Reserved */
    { 0x18, DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_1 },
    { 0x19, DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_2 },
    { 0x1A, DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_3 },
    { 0x1B, DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_4 },
    /* 0x1C-0x1F Reserved */
    { 0x20, DVBPSI_AAC_PROFILE_SPEECH_LEVEL_1 },
    { 0x21, DVBPSI_AAC_PROFILE_SPEECH_LEVEL_2 },
    /* 0x22-0x27 Reserved */
    { 0x28, DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_1 },
    { 0x29, DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_2 },
    { 0x2A, DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_3 },
    /* 0x2B-0x2F Reserved */
    { 0x30, DVBPSI_AAC_PROFILE_HQ_LEVEL_1 },
    { 0x31, DVBPSI_AAC_PROFILE_HQ_LEVEL_2 },
    { 0x32, DVBPSI_AAC_PROFILE_HQ_LEVEL_3 },
    { 0x33, DVBPSI_AAC_PROFILE_HQ_LEVEL_4 },
    { 0x34, DVBPSI_AAC_PROFILE_HQ_LEVEL_5 },
    { 0x35, DVBPSI_AAC_PROFILE_HQ_LEVEL_6 },
    { 0x36, DVBPSI_AAC_PROFILE_HQ_LEVEL_7 },
    { 0x37, DVBPSI_AAC_PROFILE_HQ_LEVEL_8 },
    { 0x38, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_1 },
    { 0x39, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_2 },
    { 0x3A, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_3 },
    { 0x3B, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_4 },
    { 0x3C, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_5 },
    { 0x3D, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_6 },
    { 0x3E, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_7 },
    { 0x3F, DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_8 },
    { 0x40, DVBPSI_AAC_PROFILE_NATURAL_LEVEL_1 },
    { 0x41, DVBPSI_AAC_PROFILE_NATURAL_LEVEL_2 },
    { 0x42, DVBPSI_AAC_PROFILE_NATURAL_LEVEL_3 },
    { 0x43, DVBPSI_AAC_PROFILE_NATURAL_LEVEL_4 },
    /* 0x44-0x47 Reserved */
    { 0x48, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_1 },
    { 0x49, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_2 },
    { 0x4A, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_3 },
    { 0x4B, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_4 },
    { 0x4C, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_5 },
    { 0x4D, DVBPSI_AAC_PROFILE_MOBILE_LEVEL_6 },
    /* 0x4E-0x4F Reserved */
    { 0x50, DVBPSI_AAC_PROFILE_LEVEL_1  },
    { 0x51, DVBPSI_AAC_PROFILE_LEVEL_2  },
    { 0x52, DVBPSI_AAC_PROFILE_LEVEL_4  },
    { 0x53, DVBPSI_AAC_PROFILE_LEVEL_5  },
    /* 0x54-0x57 RESERVED */
    { 0x58, DVBPSI_HE_AAC_PROFILE_LEVEL_2 },
    { 0x59, DVBPSI_HE_AAC_PROFILE_LEVEL_3 },
    { 0x5A, DVBPSI_HE_AAC_PROFILE_LEVEL_4 },
    { 0x5B, DVBPSI_HE_AAC_PROFILE_LEVEL_5 },
    /* 0x5C-0x5F RESERVED */
    { 0x60, DVBPSI_HE_AAC_V2_PROFILE_LEVEL_2 },
    { 0x61, DVBPSI_HE_AAC_V2_PROFILE_LEVEL_3 },
    { 0x62, DVBPSI_HE_AAC_V2_PROFILE_LEVEL_4 },
    { 0x63, DVBPSI_HE_AAC_V2_PROFILE_LEVEL_5 },
    /* 0x64-0xFE RESERVED */
    { 0xFF, DVBPSI_AAC_PROFILE_NOT_SPECIFIED }
};
static dvbpsi_aac_profile_and_level_t dvbpsi_aac_profile_and_level_lookup(const uint8_t value)
{
    dvbpsi_aac_profile_and_level_t profile_and_level = DVBPSI_AAC_PROFILE_RESERVED;

    for (unsigned int i = 0; i < ARRAY_SIZE(aac_profile_and_level_table); i++)
    {
        if (aac_profile_and_level_table[i].hex == value)
            profile_and_level = aac_profile_and_level_table[i].profile_and_level;
    }

    return profile_and_level;
}

static uint8_t dvbpsi_aac_profile_and_level_to_hex(const dvbpsi_aac_profile_and_level_t profile_and_level)
{
    uint8_t value = 0x00; /* Reserved */

    if (profile_and_level == DVBPSI_AAC_PROFILE_RESERVED)
        value = 0x56; /* FIXME: levels 0x56..0x57 and 0x5C..0xFF are mapped onto 0x56 here */
    else
    {
        for (unsigned int i = 0; i < ARRAY_SIZE(aac_profile_and_level_table); i++)
        {
            if (aac_profile_and_level_table[i].profile_and_level == profile_and_level)
                value = aac_profile_and_level_table[i].hex;
        }
    }

    return value;
}

/* */
struct dvbpsi_aac_type_table_s
{
    uint8_t hex;
    dvbpsi_aac_type_t type;
};
static struct dvbpsi_aac_type_table_s aac_type_table[] =
{
    { 0x00, DVBPSI_AAC_RESERVED0 },
    { 0x01, DVBPSI_HE_AAC_MONO   },
    { 0x02, DVBPSI_AAC_RESERVED1 },
    { 0x03, DVBPSI_HE_AAC_STEREO },
    { 0x04, DVBPSI_AAC_RESERVED2 },
    { 0x05, DVBPSI_HE_AAC_SURROUND },

    { 0x40, DVBPSI_HE_AAC_IMPAIRED },
    { 0x41, DVBPSI_HE_AAC_HEARING },
    { 0x42, DVBPSI_HE_AAC_MIXED },
    { 0x43, DVBPSI_HE_AAC_V2_STEREO },
    { 0x44, DVBPSI_HE_AAC_V2_IMPAIRED },
    { 0x45, DVBPSI_HE_AAC_V2_HEARING },
    { 0x46, DVBPSI_HE_AAC_V2_MIXED },
    { 0x47, DVBPSI_HE_AAC_MIXED_IMPAIRED },
    { 0x48, DVBPSI_HE_AAC_BROADCAST_MIXED_IMPAIRED },
    { 0x49, DVBPSI_HE_AAC_V2_MIXED_IMPAIRED },
    { 0x4A, DVBPSI_HE_AAC_V2_BROADCAST_MIXED_IMPAIRED }
};

static dvbpsi_aac_type_t dvbpsi_aac_type_lookup(const uint8_t value)
{
    dvbpsi_aac_type_t type = 0;

    if ((value >= 0x06) && (value <= 0x3F))
        type = DVBPSI_AAC_RESERVED3;
    else if ((value >= 0x4B) && (value <= 0xAF))
        type = DVBPSI_AAC_RESERVED4;
    else if ((value >= 0xB0) && (value <= 0xFE))
        type = DVBPSI_AAC_USER;
    else if (value == 0xFF)
        type = DVBPSI_AAC_RESERVED5;
    else
    {
        for (unsigned int i = 0; i < ARRAY_SIZE(aac_type_table); i++)
        {
            if (aac_type_table[i].hex == value)
                type = aac_type_table[i].type;
        }
    }
    return type;
}

static uint8_t dvbpsi_aac_type_to_hex(const dvbpsi_aac_type_t type)
{
    uint8_t value = 0;

    if (type == DVBPSI_AAC_RESERVED3)
        value = 0x06; /* FIXME: 0x06..0x3F */
    else if (type == DVBPSI_AAC_RESERVED4)
        value = 0x4B; /* FIXME: 0x4B..0xAF */
    else if (type == DVBPSI_AAC_USER)
        value = 0xB0; /* FIXME: 0xB0..0xFE */
    else if (type == DVBPSI_AAC_RESERVED5)
        value = 0xFF;
    else
    {
        for (unsigned int i = 0; i < ARRAY_SIZE(aac_type_table); i++)
        {
            if (aac_type_table[i].type == type)
                value = aac_type_table[i].hex;
        }
    }
    return value;
}

/*****************************************************************************
 * dvbpsi_DecodeAACDr
 *****************************************************************************/
dvbpsi_aac_dr_t *dvbpsi_DecodeAACDr(dvbpsi_descriptor_t *p_descriptor)
{
    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x7c))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    if (p_descriptor->i_length == 0x01)
        return NULL;

    /* Allocate memory */
    dvbpsi_aac_dr_t *p_decoded;
    p_decoded = (dvbpsi_aac_dr_t*)calloc(1, sizeof(dvbpsi_aac_dr_t));
    if (!p_decoded)
        return NULL;

    /* AAC Audio descriptor
     * ETSI EN 300 468 V1.13.1 (2012-04) Annex H
     */
    p_decoded->i_profile_and_level = dvbpsi_aac_profile_and_level_lookup(p_descriptor->p_data[0]);
    if (p_descriptor->i_length > 1)
        p_decoded->b_type = ((p_descriptor->p_data[1]>>7) == 0x01);
    if (p_decoded->b_type)
        p_decoded->i_type = dvbpsi_aac_type_lookup(p_descriptor->p_data[2]);

    /* Keep additional info bytes field */
    if (p_descriptor->i_length > 1)
    {
        uint8_t i_info_length = p_descriptor->i_length - (p_decoded->b_type ? 3 : 2);
        dvbpsi_aac_dr_t *p_tmp = realloc(p_decoded, sizeof(dvbpsi_aac_dr_t) + i_info_length);
        if (!p_tmp)
        {
            free(p_decoded);
            return NULL;
        }
        p_decoded->p_additional_info = ((uint8_t*)p_tmp + sizeof(dvbpsi_aac_dr_t));
        p_decoded->i_additional_info_length = i_info_length;

        uint8_t i_data = p_decoded->b_type ? 3 : 2;
        uint8_t *p = &p_descriptor->p_data[i_data];
        memcpy(p_decoded->p_additional_info, p, i_info_length);
    }

    p_descriptor->p_decoded = (void*)p_decoded;

    return p_decoded;
}

/*****************************************************************************
 * dvbpsi_GenAACDr
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_GenAACDr(dvbpsi_aac_dr_t *p_decoded, bool b_duplicate)
{
    /* Create the descriptor */
    uint8_t i_length = p_decoded->b_type ? 3 + p_decoded->i_additional_info_length : 1;
    dvbpsi_descriptor_t *p_descriptor = dvbpsi_NewDescriptor(0x7c, i_length, NULL);
    if (!p_descriptor)
        return NULL;

    /* Encode data */
    p_descriptor->p_data[0] = dvbpsi_aac_profile_and_level_to_hex(p_decoded->i_profile_and_level);

    if (p_descriptor->i_length > 1)
    {
        p_descriptor->p_data[1]  = 0x00;
        p_descriptor->p_data[1] |= ((p_decoded->b_type ? 1: 0) << 7);
    }

    if (p_decoded->b_type)
        p_descriptor->p_data[2] = dvbpsi_aac_type_to_hex(p_decoded->i_type);

    /* Store additional info bytes field */
    if (p_descriptor->i_length > 1)
    {
        uint8_t *p = &p_descriptor->p_data[p_decoded->b_type ? 3 : 2];
        memcpy(p, p_decoded->p_additional_info, p_decoded->i_additional_info_length);
    }

    if (b_duplicate)
    {
        /* Duplicate decoded data */
        p_descriptor->p_decoded =
                dvbpsi_DuplicateDecodedDescriptor(p_decoded, sizeof(dvbpsi_aac_dr_t));
    }

    return p_descriptor;
}
