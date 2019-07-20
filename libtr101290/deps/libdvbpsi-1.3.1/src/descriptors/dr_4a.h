/*****************************************************************************
 * dr_4a.h
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: Corno Roberto <corno.roberto@gmail.com>
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
 * \file <dr_4a.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the DVB "country availability"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "linkage" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.19.
 */

#ifndef DR_4A_H_
#define DR_4A_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_linkage_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_linkage_dr_t
 * \brief "linkage" descriptor structure.
 *
 * This structure is used to store a decoded "linkage"
 * descriptor. (ETSI EN 300 468 section 6.2.10).
 */
/*!
 * \typedef struct dvbpsi_linkage_dr_s dvbpsi_linkage_dr_t
 * \brief dvbpsi_linkage_dr_t type definition.
 */
/*!
 * \struct dvbpsi_linkage_dr_s
 * \brief struct dvbpsi_linkage_dr_s @see dvbpsi_linkage_dr_t
 */
typedef struct dvbpsi_linkage_dr_s
{
  uint16_t       i_transport_stream_id;         /*!< transport stream id */
  uint16_t       i_original_network_id;         /*!< original network id */
  uint16_t       i_service_id;                  /*!< service id */
  uint8_t        i_linkage_type;                /*!< linkage type */

  /* used if i_linkage_type ==0x08 */
  uint8_t        i_handover_type;               /*!< hand-over type */
  uint8_t        i_origin_type;                 /*!< origin type */
  /* used if i_linkage_type ==0x08 &&
   * (i_handover_type ==0x01 || i_handover_type ==0x02|| i_handover_type ==0x03)
   */
  uint16_t       i_network_id;                  /*!< network id */
  /* used if i_linkage_type ==0x08 && i_origin_type ==0 */
  uint16_t       i_initial_service_id;          /*!< initial service id */

  /* used if i_linkage_type ==0x0D */
  uint16_t       i_target_event_id;             /*!< target event id */
  bool           b_target_listed;               /*!< target listed */
  bool           b_event_simulcast;             /*!< event simulcast */

  uint8_t       i_private_data_length;         /*!< length of the i_private_data
                                                    array */
  uint8_t       i_private_data[248];           /*!< private data */

} dvbpsi_linkage_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeLinkageDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_linkage_dr_t * dvbpsi_DecodeLinkageDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "linkage" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "linkage" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_linkage_dr_t* dvbpsi_DecodeLinkageDr(dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenLinkageDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_GenLinkageDr(dvbpsi_linkage_dr_t *p_decoded,
                                          bool b_duplicate);
 * \brief "linkage" descriptor generator.
 * \param p_decoded pointer to a decoded "linkage" descriptor structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t *dvbpsi_GenLinkageDr(dvbpsi_linkage_dr_t * p_decoded,
                                         bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_4a.h"
#endif /* DR_4A_H_ */
