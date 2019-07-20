/*****************************************************************************
 * dr_09.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: dr_09.h,v 1.2 2002/05/10 23:50:36 bozo Exp $
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
 * \file <dr_09.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the MPEG 2 "conditional access"
 * descriptor decoder and generator.
 *
 * Application interface for the MPEG 2 "conditional access" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ISO/IEC 13818-1 section 2.6.16.
 */

#ifndef _DVBPSI_DR_09_H_
#define _DVBPSI_DR_09_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_ca_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_ca_dr_s
 * \brief "conditional access" descriptor structure.
 *
 * This structure is used to store a decoded "conditional access"
 * descriptor. (ISO/IEC 13818-1 section 2.6.16).
 */
/*!
 * \typedef struct dvbpsi_ca_dr_s dvbpsi_ca_dr_t
 * \brief dvbpsi_ca_dr_t type definition.
 */
typedef struct dvbpsi_ca_dr_s
{
  uint16_t      i_ca_system_id;         /*!< CA_system_ID */
  uint16_t      i_ca_pid;               /*!< CA_PID */
  uint8_t       i_private_length;       /*!< length of the i_private_data
                                             array */
  uint8_t       i_private_data[251];    /*!< private_data_byte */

} dvbpsi_ca_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeCADr
 *****************************************************************************/
/*!
 * \fn dvbpsi_ca_dr_t * dvbpsi_DecodeCADr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "conditional access" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "conditional access" descriptor structure which
 * contains the decoded data.
 */
dvbpsi_ca_dr_t* dvbpsi_DecodeCADr(dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenCADr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenCADr(
                                dvbpsi_ca_dr_t * p_decoded, int b_duplicate)
 * \brief "conditional access" descriptor generator.
 * \param p_decoded pointer to a decoded "conditional access" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenCADr(dvbpsi_ca_dr_t * p_decoded,
                                     int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_09.h"
#endif

