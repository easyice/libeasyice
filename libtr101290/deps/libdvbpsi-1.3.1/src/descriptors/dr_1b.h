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

/*!
 * \file <dr_1b.h>
 * \author Daniel Kamil Kozar <dkk089@gmail.com>
 * \brief Application interface for the MPEG-4 video descriptor decoder and
 * generator.
 *
 * Application interface for the MPEG-4 video descriptor decoder and generator.
 * This descriptor's definition can be found in ISO/IEC 13818-1 revision 2014/10
 * section 2.6.36.
 */

#ifndef _DVBPSI_DR_1B_H_
#define _DVBPSI_DR_1B_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \enum dvbpsi_mpeg4_visual_profile_and_level_s
 * \brief Enumeration of MPEG-4 video profile and levels as specified in
 * ISO/IEC 14496-2:2001 Table G-1.
 */
/*!
 * \typedef enum dvbpsi_mpeg4_visual_profile_and_level_s dvbpsi_mpeg4_visual_profile_and_level_t
 * \note Values not present in this enumeration were marked by the specification
 * as reserved at the time of writing.
 * \brief MPEG-4 video profile and level as specified in ISO/IEC 14496-2:2001
 * Table G-1.
 */
typedef enum dvbpsi_mpeg4_visual_profile_and_level_s
{
    DVBPSI_MPEG4V_PROFILE_SIMPLE_L1 = 0x01, /*!< Simple Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_L2 = 0x02, /*!< Simple Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_L3 = 0x03, /*!< Simple Profile/Level 3 */
    /* 0x04 - 0x10 : Reserved */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_SCALABLE_L1 = 0x11, /*!< Simple Scalable Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_SCALABLE_L2 = 0x12, /*!< Simple Scalable Profile/Level 2 */
    /* 0x13 - 0x20 : Reserved */
    DVBPSI_MPEG4V_PROFILE_CORE_L1 = 0x21, /*!< Core Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_CORE_L2 = 0x22, /*!< Core Profile/Level 2 */
    /* 0x23 - 0x31 : Reserved */
    DVBPSI_MPEG4V_PROFILE_MAIN_L2 = 0x32, /*!< Main Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_MAIN_L3 = 0x33, /*!< Main Profile/Level 3 */
    DVBPSI_MPEG4V_PROFILE_MAIN_L4 = 0x34, /*!< Main Profile/Level 4 */
    /* 0x35 - 0x41 : Reserved */
    DVBPSI_MPEG4V_PROFILE_N_BIT_L2 = 0x42, /*!< N-bit Profile/Level 2 */
    /* 0x43 - 0x50 : Reserved */
    DVBPSI_MPEG4V_PROFILE_SCALABLE_TEXTURE_L1 = 0x51, /*!< Scalable Texture Profile/Level 1 */
    /* 0x52 - 0x60 : Reserved */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_FACE_ANIMATION_L1 = 0x61, /*!< Simple Face Animation Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_FACE_ANIMATION_L2 = 0x62, /*!< Simple Face Animation Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_FBA_L1 = 0x63, /*!< Simple FBA Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_SIMPLE_FBA_L2 = 0x64, /*!< Simple FBA Profile/Level 2 */
    /* 0x65 - 0x70 : Reserved */
    DVBPSI_MPEG4V_PROFILE_BASIC_ANIMATED_TEXTURE_L1 = 0x71, /*!< Basic Animated Texture Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_BASIC_ANIMATED_TEXTURE_L2 = 0x72, /*!< Basic Animated Texture Profile/Level 2 */
    /* 0x73 - 0x80 : Reserved */
    DVBPSI_MPEG4V_PROFILE_HYBRID_L1 = 0x81, /*!< Hybrid Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_HYBRID_L2 = 0x82, /*!< Hybrid Profile/Level 2 */
    /* 0x83 - 0x90 : Reserved */
    DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L1 = 0x91, /*!< Advanced Real Time Simple Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L2 = 0x92, /*!< Advanced Real Time Simple Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L3 = 0x93, /*!< Advanced Real Time Simple Profile/Level 3 */
    DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L4 = 0x94, /*!< Advanced Real Time Simple Profile/Level 4 */
    /* 0x95 - 0xa0 : Reserved */
    DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L1 = 0xa1, /*!< Core Scalable Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L2 = 0xa2, /*!< Core Scalable Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L3 = 0xa3, /*!< Core Scalable Profile/Level 3 */
    /* 0xa4 - 0xb0 : Reserved */
    DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L1 = 0xb1, /*!< Advanced Coding Efficiency Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L2 = 0xb2, /*!< Advanced Coding Efficiency Profile/Level 2 */
    DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L3 = 0xb3, /*!< Advanced Coding Efficiency Profile/Level 3 */
    DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L4 = 0xb4, /*!< Advanced Coding Efficiency Profile/Level 4 */
    /* 0xb5 - 0xc0 : Reserved */
    DVBPSI_MPEG4V_PROFILE_ADV_CORE_L1 = 0xc1, /*!< Advanced Core Profile/Level 1 */
    DVBPSI_MPEG4V_PROFILE_ADV_CORE_L2 = 0xc2, /*!< Advanced Core Profile/Level 2 */
    /* 0xc3 - 0xd0 : Reserved */
    DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L1 = 0xd1, /*!< Advanced Scalable Texture/Level 1 */
    DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L2 = 0xd2, /*!< Advanced Scalable Texture/Level 2 */
    DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L3 = 0xd3, /*!< Advanced Scalable Texture/Level 3 */
    /* 0xd4 - 0xff : Reserved */
    DVBPSI_MPEG4V_PROFILE_LAST = 0xff, /* enforce enum size. */
} dvbpsi_mpeg4_visual_profile_and_level_t;

/*!
 * \struct dvbpsi_mpeg4_video_dr_s
 * \brief MPEG-4 video descriptor structure.
 *
 * This structure is used to store a decoded MPEG-4 video descriptor. (ISO/IEC
 * 13818-1 section 2.6.36).
 */

/*!
 * \typedef struct dvbpsi_mpeg4_video_dr_s dvbpsi_mpeg4_video_dr_t
 * \brief dvbpsi_mpeg4_video_dr_t type definition.
 */
typedef struct dvbpsi_mpeg4_video_dr_s
{
    /*! MPEG-4_visual_profile_and_level */
    dvbpsi_mpeg4_visual_profile_and_level_t    i_mpeg4_visual_profile_and_level;
} dvbpsi_mpeg4_video_dr_t;

/*!
 * \brief MPEG-4 video descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return A pointer to a new MPEG-4 video descriptor structure which contains
 * the decoded data.
 */
dvbpsi_mpeg4_video_dr_t* dvbpsi_DecodeMPEG4VideoDr(
                                      dvbpsi_descriptor_t * p_descriptor);

/*!
 * \brief MPEG-4 video descriptor generator.
 * \param p_decoded pointer to a decoded MPEG-4 video descriptor structure.
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenMPEG4VideoDr(
                                      dvbpsi_mpeg4_video_dr_t * p_decoded);

#ifdef __cplusplus
}
#endif

#else
#error "Multiple inclusions of dr_1b.h"
#endif
