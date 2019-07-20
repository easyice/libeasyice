/*****************************************************************************
 * dr_0a.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 * \file <dr_0a.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the MPEG 2 "ISO 639 language"
 * descriptor decoder and generator.
 *
 * Application interface for the MPEG 2 "ISO 639 language" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ISO/IEC 13818-1 section 2.6.18.
 */

#ifndef _DVBPSI_DR_0A_H_
#define _DVBPSI_DR_0A_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DR_0A_API_VER 2
typedef uint8_t iso_639_language_code_t[3];

/*****************************************************************************
 * dvbpsi_iso639_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_iso639_dr_s
 * \brief "ISO 639 language" descriptor structure.
 *
 * This structure is used to store a decoded "ISO 639 language"
 * descriptor. (ISO/IEC 13818-1 section 2.6.18).
 */
/*!
 * \typedef struct dvbpsi_iso639_dr_s dvbpsi_iso639_dr_t
 * \brief dvbpsi_iso639_dr_t type definition.
 */
typedef struct dvbpsi_iso639_dr_s
{
  uint8_t       i_code_count;           /*!< length of the i_iso_639_code
                                             array */
  struct {
    iso_639_language_code_t  iso_639_code;    /*!< ISO_639_language_code */
    uint8_t                  i_audio_type;    /*!< audio_type */
  } code[64];

} dvbpsi_iso639_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeISO639Dr
 *****************************************************************************/
/*!
 * \fn dvbpsi_iso639_dr_t * dvbpsi_DecodeISO639Dr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "ISO 639 language" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "ISO 639 language" descriptor structure which
 * contains the decoded data.
 */
dvbpsi_iso639_dr_t* dvbpsi_DecodeISO639Dr(dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenISO639Dr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenISO639Dr(
                        dvbpsi_iso639_dr_t * p_decoded, int b_duplicate)
 * \brief "ISO 639 language" descriptor generator.
 * \param p_decoded pointer to a decoded "ISO 639 language" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenISO639Dr(dvbpsi_iso639_dr_t * p_decoded,
                                         int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_0a.h"
#endif

