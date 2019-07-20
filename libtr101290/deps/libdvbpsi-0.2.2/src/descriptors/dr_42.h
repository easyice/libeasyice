/*****************************************************************************
 * dr_42.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Johan Bilien <jobi@via.ecp.fr>
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
 * \file <dr_42.h>
 * \author Johan Bilien <jobi@via.ecp.fr>
 * \brief Application interface for the DVB "stuffing"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "stuffing" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.35.
 */

#ifndef _DVBPSI_DR_42_H_
#define _DVBPSI_DR_42_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_stuffing_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_stuffing_dr_s
 * \brief "stuffing" descriptor structure.
 *
 * This structure is used to store a decoded "stuffing"
 * descriptor. (ETSI EN 300 468 section 6.2.35).
 */
/*!
 * \typedef struct dvbpsi_stuffing_dr_s dvbpsi_stuffing_dr_t
 * \brief dvbpsi_stuffing_dr_t type definition.
 */
typedef struct dvbpsi_stuffing_dr_s
{
  uint8_t      i_stuffing_length;            /*!< length of the i_stuffing_byte
                                                  array */
  uint8_t      i_stuffing_byte[255];         /*!< stuffing_bytes */

} dvbpsi_stuffing_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeStuffingDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_stuffing_dr_t * dvbpsi_DecodeStuffingDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "stuffing" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "stuffing" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_stuffing_dr_t* dvbpsi_DecodeStuffingDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenStuffingDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenStuffingDr(
                        dvbpsi_stuffing_data_dr_t * p_decoded, int b_duplicate)
 * \brief "stuffing" descriptor generator.
 * \param p_decoded pointer to a decoded "stuffing" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenStuffingDr(
                                        dvbpsi_stuffing_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_42.h"
#endif

