/*****************************************************************************
 * dr_45.h
 * Copyright (C) 2004-2010 VideoLAN
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

/*!
 * \file <dr_45.h>
 * \author Jean-Paul Saman <jpsaman at videolan dot org>
 * \brief VBI data descriptor parsing.
 *
 * DVB VBI data descriptor parsing, according to ETSI EN 300 468
 * version 1.7.1 section 6.2.46
 *
 * NOTE: this descriptor is known by tag value 0x45
 */

#ifndef _DVBPSI_DR_45_H_
#define _DVBPSI_DR_45_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_vbidata_line_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_vbidata_line_t
 * \brief  one VBI Data line structure.
 *
 * This structure is used since vbidata_t structure will contain several
 * of these structures
 */
/*!
 * \typedef struct dvbpsi_vbidata_line_s dvbpsi_vbidata_line_t
 * \brief dvbpsi_vbidata_line_t type definition.
 */
typedef struct dvbpsi_vbidata_line_s
{
  uint8_t   i_parity;      /* 1 bits */
  uint8_t   i_line_offset; /* 5 bits */

} dvbpsi_vbidata_line_t;

/*****************************************************************************
 * dvbpsi_vbidata_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_vbidata_t
 * \brief  one VBI data structure.
 *
 * This structure is used since vbi_descriptor will contain several
 * of these structures
 */
/*!
 * \typedef struct dvbpsi_vbidata_s dvbpsi_vbidata_t
 * \brief dvbpsi_vbidata_t type definition.
 */
typedef struct dvbpsi_vbidata_s
{
  uint8_t               i_data_service_id;  /* 8 bits */
  uint8_t               i_lines;
  dvbpsi_vbidata_line_t p_lines[255];

} dvbpsi_vbidata_t;

/*****************************************************************************
 * dvbpsi_vbi_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_vbi_dr_s
 * \brief "teletext" descriptor structure.
 *
 * This structure is used to store a decoded "VBI data"
 * descriptor. (ETSI EN 300 468 version 1.7.1 section 6.2.46).
 */
/*!
 * \typedef struct dvbpsi_vbi_dr_s dvbpsi_vbi_dr_t
 * \brief dvbpsi_vbi_dr_t type definition.
 */
typedef struct dvbpsi_vbi_dr_s
{
  uint8_t          i_services_number;
  dvbpsi_vbidata_t p_services[85];

} dvbpsi_vbi_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeVBIDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_vbi_dr_t * dvbpsi_DecodeVBIDataDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "VBI data" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "VBI data" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_vbi_dr_t* dvbpsi_DecodeVBIDataDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenVBIDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenVBIDataDr(
                        dvbpsi_vbi_dr_t * p_decoded, int b_duplicate)
 * \brief "VBI data" descriptor generator.
 * \param p_decoded pointer to a decoded "VBI data" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenVBIDataDr(
                                        dvbpsi_vbi_dr_t * p_decoded,
                                        int b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_45.h"
#endif
