/*****************************************************************************
 * dr_52.h
 * Copyright (C) 2005-2010 Andrew John Hughes
 *
 * Authors: Andrew John Hughes <gnu_andrew@member.fsf.org>
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
 * \file <dr_52.h>
 * \author Andrew John Hughes <gnu_andrew@member.fsf.org>
 * \brief Application interface for the DVB "stream identifier"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "stream identifier" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.37.
 */

#ifndef _DVBPSI_DR_52_H_
#define _DVBPSI_DR_52_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_service_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_dr_s
 * \brief "stream identifier" descriptor structure.
 *
 * This structure is used to store a decoded "stream identifier"
 * descriptor. (ETSI EN 300 468 section 6.2.37).
 */
/*!
 * \typedef struct dvbpsi_service_dr_s dvbpsi_stream_identifier_dr_t
 * \brief dvbpsi_stream_identifier_dr_t type definition.
 */
typedef struct dvbpsi_stream_identifier_dr_s
{
  uint8_t      i_component_tag;             /*!< component tag*/
} dvbpsi_stream_identifier_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeStreamIdentifierDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_stream_identifier_dr_t * dvbpsi_DecodeStreamIdentifierDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "stream identifier" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "stream identifier" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_stream_identifier_dr_t* dvbpsi_DecodeStreamIdentifierDr(
                              dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenStreamIdentifierDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenStreamIdentifierDr(
                        dvbpsi_service_dr_t * p_decoded, int b_duplicate)
 * \brief "stream identifier" descriptor generator.
 * \param p_decoded pointer to a decoded "stream identifier" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenStreamIdentifierDr(
                                        dvbpsi_stream_identifier_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_52.h"
#endif

