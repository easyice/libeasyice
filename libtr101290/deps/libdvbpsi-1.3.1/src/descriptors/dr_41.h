/*****************************************************************************
 * dr_41.h
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: rcorno (May 21, 2012)
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
 * \file <dr_41.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the DVB "service list"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "service list" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.35.
 */

#ifndef DR_41_H_
#define DR_41_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_network_name_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_list_dr_s
 * \brief "service list" descriptor structure.
 *
 * This structure is used to store a decoded "service list"
 * descriptor. (ETSI EN 300 468 section 6.2.35).
 */
/*!
 * \typedef struct dvbpsi_service_list_dr_s dvbpsi_service_list_dr_t
 * \brief dvbpsi_service_list_dr_t type definition.
 */
typedef struct dvbpsi_service_list_dr_s
{
  uint8_t       i_service_count;            /*!< length of the i_service_list
  	                                             array */
  struct {
      uint16_t     i_service_id;            /*!< service id */
      uint8_t      i_service_type;          /*!< service type */
  } i_service[64];                          /*!< array of services */

} dvbpsi_service_list_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeServiceListDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_service_list_dr_t * dvbpsi_DecodeServiceListDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "service list" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "service list" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_service_list_dr_t* dvbpsi_DecodeServiceListDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenServiceListDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenServiceListDr(
                        dvbpsi_service_list_dr_t * p_decoded, bool b_duplicate)
 * \brief "service list" descriptor generator.
 * \param p_decoded pointer to a decoded "service list" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenServiceListDr(
		                        dvbpsi_service_list_dr_t * p_decoded,
                                        bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_41.h"
#endif /* DR_41_H_ */
