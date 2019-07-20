/*****************************************************************************
 * dr_56.h
 * Copyright (C) 2004-2010 VideoLAN
 * $Id: dr_56.h 93 2004-10-19 19:17:49Z massiot $
 *
 * Authors: Derk-Jan Hartman <hartman at videolan dot org>
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
 * \file <dr_56.h>
 * \author Derk-Jan Hartman <hartman at videolan dot org>
 * \brief EBU Teletext descriptor parsing.
 *
 * DVB EBU Teletext descriptor parsing, according to ETSI EN 300 468
 * version 1.7.1 section 6.2.42 and 6.2.47.
 *
 * NOTE: this descriptor is known by tag value 0x56 AND 0x46
 */

#ifndef _DVBPSI_DR_56_H_
#define _DVBPSI_DR_56_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_teletext_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_teletextpage_t
 * \brief  one teletext page structure.
 *
 * This structure is used since teletext_descriptor will contain several
 * pages
 */
/*!
 * \typedef struct dvbpsi_teletextpage_s dvbpsi_teletextpage_t
 * \brief dvbpsi_teletextpage_t type definition.
 */
typedef struct dvbpsi_teletextpage_s
{
  uint8_t      i_iso6392_language_code[3];  /* 24 bits */
  uint8_t      i_teletext_type;             /*  5 bits */
  uint8_t      i_teletext_magazine_number;  /*  3 bits */
  uint8_t      i_teletext_page_number;      /*  8 bits */

} dvbpsi_teletextpage_t;


/*****************************************************************************
 * dvbpsi_teletext_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_teletext_dr_s
 * \brief "teletext" descriptor structure.
 *
 * This structure is used to store a decoded "teletext"
 * descriptor. (ETSI EN 300 468 version 1.7.1 section 6.2.42 and 6.2.47).
 */
/*!
 * \typedef struct dvbpsi_teletext_dr_s dvbpsi_teletext_dr_t
 * \brief dvbpsi_teletext_dr_t type definition.
 */
typedef struct dvbpsi_teletext_dr_s
{
  uint8_t      i_pages_number;
  dvbpsi_teletextpage_t p_pages[64];

} dvbpsi_teletext_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeTeletextDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_teletext_dr_t * dvbpsi_DecodeTeletextDataDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "teletext" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "teletext" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_teletext_dr_t* dvbpsi_DecodeTeletextDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenTeletextDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenTeletextDr(
                        dvbpsi_teletext_dr_t * p_decoded, int b_duplicate)
 * \brief "teletext" descriptor generator.
 * \param p_decoded pointer to a decoded "teletext" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenTeletextDr(
                                        dvbpsi_teletext_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_56.h"
#endif
