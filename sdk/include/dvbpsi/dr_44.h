/*****************************************************************************
 * dr_44.h
 * Copyright (C) 2011 VideoLAN
 * $Id$
 *
 * Authors: Ilkka Ollakka
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
 * \file <dr_44.h>
 * \author Ilkka Ollakka
 * \brief Application interface for the DVB cable delivery system
 *        descriptor decoder and generator.
 *
 * Application interface for the DVB cable delivery system descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.13.1.
 * dr_44.h is heavily based on dr_43.h file
 */

#ifndef _DVBPSI_DR_44_H_
#define _DVBPSI_DR_44_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_cable_deliv_sys_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_cable_deliv_sys_dr_s
 * \brief cable delivery system descriptor structure.
 *
 * This structure is used to store a decoded cable delivery system
 * descriptor. (ETSI EN 300 468 section 6.2.13.1).
 */
/*!
 * \typedef struct dvbpsi_cable_deliv_sys_dr_s dvbpsi_cable_deliv_sys_dr_t
 * \brief dvbpsi_cable_deliv_sys_dr_t type definition.
 */
typedef struct dvbpsi_cable_deliv_sys_dr_s
{
  uint32_t     i_frequency;                         /*!< frequency */
  uint8_t      i_modulation;                   /*!< modulation type */
  uint32_t     i_symbol_rate;                       /*!< symbol rate */
  uint8_t      i_fec_inner;                         /*!< FEC inner */
  uint8_t      i_fec_outer;                         /*!< FEC outer*/

} dvbpsi_cable_deliv_sys_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeCableDelivSysDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_cable_deliv_sys_dr_t * dvbpsi_DecodeCableDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief cable delivery system descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new cable delivery system descriptor structure
 * which contains the decoded data.
 */
dvbpsi_cable_deliv_sys_dr_t* dvbpsi_DecodeCableDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenCableDelivSysDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenCableDelivSysDr(
                        dvbpsi_cable_deliv_sys_dr_t * p_decoded, int b_duplicate)
 * \brief cable delivery system descriptor generator.
 * \param p_decoded pointer to a decoded cable delivery system descriptor
 * descriptor structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenCableDelivSysDr(
                                        dvbpsi_cable_deliv_sys_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_44.h"
#endif
