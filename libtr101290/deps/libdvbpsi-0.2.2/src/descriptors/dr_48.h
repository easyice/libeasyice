/*****************************************************************************
 * dr_48.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: dr_48.h,v 1.2 2002/12/12 10:19:32 jobi Exp $
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
 * \file <dr_48.h>
 * \author Johan Bilien <jobi@via.ecp.fr>
 * \brief Application interface for the DVB "service"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "service" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.30.
 */

#ifndef _DVBPSI_DR_48_H_
#define _DVBPSI_DR_48_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_service_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_dr_s
 * \brief "service" descriptor structure.
 *
 * This structure is used to store a decoded "service"
 * descriptor. (ETSI EN 300 468 section 6.2.30).
 */
/*!
 * \typedef struct dvbpsi_service_dr_s dvbpsi_service_dr_t
 * \brief dvbpsi_service_dr_t type definition.
 */
typedef struct dvbpsi_service_dr_s
{
  uint8_t      i_service_type;              /*!< service_type*/
  uint8_t      i_service_provider_name_length; /*!< length of the
                                                i_service_provider_name array*/
  uint8_t      i_service_provider_name[252];/*!< name of the service provider */
  uint8_t      i_service_name_length;       /*!< length of the
                                              i_service_name array*/
  uint8_t      i_service_name[252];         /*!< name of the service */

} dvbpsi_service_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeServiceDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_service_dr_t * dvbpsi_DecodeServiceDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "service" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "service" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_service_dr_t* dvbpsi_DecodeServiceDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenServiceDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenServiceDr(
                        dvbpsi_service_dr_t * p_decoded, int b_duplicate)
 * \brief "service" descriptor generator.
 * \param p_decoded pointer to a decoded "service" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenServiceDr(
                                        dvbpsi_service_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_48.h"
#endif

