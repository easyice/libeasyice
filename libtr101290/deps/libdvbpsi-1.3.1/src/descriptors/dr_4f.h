/*****************************************************************************
 * dr_4f.h
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: Roberto Corno <corno.roberto@gmail.com>
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
 * \file <dr_4f.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the time shifted event
 * descriptor decoder and generator.
 *
 * Application interface for the DVB time shifted event descriptor
 * descriptor decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.44.
 */

#ifndef DR_4F_H_
#define DR_4F_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_tshifted_ev_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_tshifted_ev_dr_t
 * \brief "time shifted event" descriptor structure.
 *
 * This structure is used to store a decoded "time shifted event"
 * descriptor. (ETSI EN 300 468 section 6.2.44).
 */
/*!
 * \typedef struct dvbpsi_tshifted_ev_dr_s dvbpsi_tshifted_ev_dr_t
 * \brief dvbpsi_tshifted_ev_dr_t type definition.
 */
/*!
 * \struct dvbpsi_tshifted_ev_dr_s
 * \brief struct dvbpsi_tshifted_ev_dr_s @see dvbpsi_tshifted_ev_dr_t
 */
typedef struct dvbpsi_tshifted_ev_dr_s
{
  uint16_t       i_ref_service_id;         /*!< reference service id */
  uint16_t       i_ref_event_id;           /*!< reference service id */
} dvbpsi_tshifted_ev_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeTimeShiftedEventDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_tshifted_ev_dr_t * dvbpsi_DecodeTimeShiftedEventDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "time shifted event" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "time shifted event" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_tshifted_ev_dr_t* dvbpsi_DecodeTimeShiftedEventDr(dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenTimeShiftedEventDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_GenTimeShiftedEventDr(dvbpsi_tshifted_ev_dr_t *p_decoded,
                                                         bool b_duplicate);
 * \brief "time shifted event" descriptor generator.
 * \param p_decoded pointer to a decoded "time shifted event" descriptor structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t *dvbpsi_GenTimeShiftedEventDr(dvbpsi_tshifted_ev_dr_t * p_decoded,
                                                  bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_4f.h"
#endif /* DR_4F_H_ */
