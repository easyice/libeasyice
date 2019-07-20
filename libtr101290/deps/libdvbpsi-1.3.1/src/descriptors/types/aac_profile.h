/*****************************************************************************
 * aac_profile.h
 * Copyright (c) 2012 VideoLAN
 * $Id$
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
 * Daniel Kamil Kozar <dkk089@gmail.com>
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

/*!
 * \file <aac_profile.h>
 * \author Jean-Paul Saman <jpsaman@videolan.org>
 * \author Daniel Kamil Kozar <dkk089@gmail.com>
 * \brief AAC Audio Profile and Level values
 *
 * Definitions of the values used for specifying the AAC audio profile and level,
 * as specified in ISO/IEC 13818-1:2015 table 2.71.
 */

#ifndef _DVBPSI_AAC_PROFILE_H_
#define _DVBPSI_AAC_PROFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_aac_dr_t
 *****************************************************************************/
/*!
 * \enum dvbpsi_aac_profile_and_level_s
 * \brief enumeration of AAC profile and levels as specified in ISO/IEC 13818-1:2015 table 2.71
 */
/*!
 * \typedef enum dvbpsi_aac_profile_and_level_s dvbpsi_aac_profile_and_level_t
 * \brief AAC profile and level as specified in ISO/IEC 13818-1:2015 table 2.71.
 */
typedef enum dvbpsi_aac_profile_and_level_s
{
    DVBPSI_AAC_PROFILE_RESERVED = 0x00, /*!< 0x00-0x0E Reserved */

    /** No audio profile and level defined for the associated MPEG-4 audio stream */
    DVBPSI_AAC_PROFILE_NOT_DEFINED = 0x0F,

    DVBPSI_AAC_PROFILE_MAIN_LEVEL_1 = 0x10, /*!< Main profile, level 1 */
    DVBPSI_AAC_PROFILE_MAIN_LEVEL_2 = 0x11, /*!< Main profile, level 2 */
    DVBPSI_AAC_PROFILE_MAIN_LEVEL_3 = 0x12, /*!< Main profile, level 3 */
    DVBPSI_AAC_PROFILE_MAIN_LEVEL_4 = 0x13, /*!< Main profile, level 4 */
    /** 0x14-0x17 Reserved */
    DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_1 = 0x18, /*!< Scalable Profile, level 1 */
    DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_2 = 0x19, /*!< Scalable Profile, level 2 */
    DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_3 = 0x1A, /*!< Scalable Profile, level 3 */
    DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_4 = 0x1B, /*!< Scalable Profile, level 4 */
    /** 0x1C-0x1F Reserved */
    DVBPSI_AAC_PROFILE_SPEECH_LEVEL_1 = 0x20, /*!< Speech profile, level 1 */
    DVBPSI_AAC_PROFILE_SPEECH_LEVEL_2 = 0x21, /*!< Speech profile, level 2 */
    /** 0x22-0x27 Reserved */
    DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_1 = 0x28, /*!< Synthesis profile, level 1 */
    DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_2 = 0x29, /*!< Synthesis profile, level 2 */
    DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_3 = 0x2A, /*!< Synthesis profile, level 3 */
    /** 0x2B-0x2F Reserved */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_1 = 0x30, /*!< High quality audio profile, level 1 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_2 = 0x31, /*!< High quality audio profile, level 2 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_3 = 0x32, /*!< High quality audio profile, level 3 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_4 = 0x33, /*!< High quality audio profile, level 4 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_5 = 0x34, /*!< High quality audio profile, level 5 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_6 = 0x35, /*!< High quality audio profile, level 6 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_7 = 0x36, /*!< High quality audio profile, level 7 */
    DVBPSI_AAC_PROFILE_HQ_LEVEL_8 = 0x37, /*!< High quality audio profile, level 8 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_1 = 0x38, /*!< Low delay audio profile, level 1 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_2 = 0x39, /*!< Low delay audio profile, level 2 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_3 = 0x3A, /*!< Low delay audio profile, level 3 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_4 = 0x3B, /*!< Low delay audio profile, level 4 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_5 = 0x3C, /*!< Low delay audio profile, level 5 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_6 = 0x3D, /*!< Low delay audio profile, level 6 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_7 = 0x3E, /*!< Low delay audio profile, level 7 */
    DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_8 = 0x3F, /*!< Low delay audio profile, level 8 */
    DVBPSI_AAC_PROFILE_NATURAL_LEVEL_1 = 0x40, /*!< Natural audio profile, level 1 */
    DVBPSI_AAC_PROFILE_NATURAL_LEVEL_2 = 0x41, /*!< Natural audio profile, level 2 */
    DVBPSI_AAC_PROFILE_NATURAL_LEVEL_3 = 0x42, /*!< Natural audio profile, level 3 */
    DVBPSI_AAC_PROFILE_NATURAL_LEVEL_4 = 0x43, /*!< Natural audio profile, level 4 */
    /** 0x44-0x47 Reserved */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_1 = 0x48, /*!< Mobile audio internetworking profile, level 1 */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_2 = 0x49, /*!< Mobile audio internetworking profile, level 2 */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_3 = 0x4A, /*!< Mobile audio internetworking profile, level 3 */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_4 = 0x4B, /*!< Mobile audio internetworking profile, level 4 */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_5 = 0x4C, /*!< Mobile audio internetworking profile, level 5 */
    DVBPSI_AAC_PROFILE_MOBILE_LEVEL_6 = 0x4D, /*!< Mobile audio internetworking profile, level 6 */
    /** 0x4E-0x4F Reserved */
    DVBPSI_AAC_PROFILE_LEVEL_1 = 0x50, /*!< AAC profile, level 1 */
    DVBPSI_AAC_PROFILE_LEVEL_2 = 0x51, /*!< AAC profile, level 2 */
    DVBPSI_AAC_PROFILE_LEVEL_4 = 0x52, /*!< AAC profile, level 4 */
    DVBPSI_AAC_PROFILE_LEVEL_5 = 0x53, /*!< AAC profile, level 5 */
    /** 0x54-0x57 RESERVED */
    DVBPSI_HE_AAC_PROFILE_LEVEL_2 = 0x58, /*!< High efficiency AAC profile, level 2 */
    DVBPSI_HE_AAC_PROFILE_LEVEL_3 = 0x59, /*!< High efficiency AAC profile, level 3 */
    DVBPSI_HE_AAC_PROFILE_LEVEL_4 = 0x5A, /*!< High efficiency AAC profile, level 4 */
    DVBPSI_HE_AAC_PROFILE_LEVEL_5 = 0x5B, /*!< High efficiency AAC profile, level 5 */
    /** 0x5C-0x5F RESERVED */
    DVBPSI_HE_AAC_V2_PROFILE_LEVEL_2 = 0x60, /*!< High efficiency AAC v2 profile, level 2 */
    DVBPSI_HE_AAC_V2_PROFILE_LEVEL_3 = 0x61, /*!< High efficiency AAC v2 profile, level 3 */
    DVBPSI_HE_AAC_V2_PROFILE_LEVEL_4 = 0x62, /*!< High efficiency AAC v2 profile, level 4 */
    DVBPSI_HE_AAC_V2_PROFILE_LEVEL_5 = 0x63, /*!< High efficiency AAC v2 profile, level 5 */
    /** 0x64-0xFE RESERVED */

    /** Audio profile and level not specified by the MPEG-4_audio_profile_and_level
     * field in this descriptor. */
    DVBPSI_AAC_PROFILE_NOT_SPECIFIED = 0xFF
} dvbpsi_aac_profile_and_level_t;

#ifdef __cplusplus
};
#endif

#endif
