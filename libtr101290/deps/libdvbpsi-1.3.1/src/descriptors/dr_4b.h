/*****************************************************************************
 * dr_4a.h
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
 * \file <dr_4b.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the Near Video On Demand (NVOD) reference
 * descriptor decoder and generator.
 *
 * Application interface for the DVB Near Video On Demand (NVOD) reference
 * descriptor decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.26.
 */
#ifndef DR_4B_H_
#define DR_4B_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_nvod_ref_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nvod_ref_t
 * \brief one "NVOD reference" structure.
 *
 * This structure is used since dvbpsi_nvod_ref_dr_t structure will contain
 * several of these structures
 */
/*!
 * \typedef struct dvbpsi_nvod_ref_s dvbpsi_nvod_ref_t
 * \brief dvbpsi_nvod_ref_dr_t type definition.
 */
/*!
 * \struct dvbpsi_nvod_ref_s
 * \brief struct dvbpsi_nvod_ref_s @see dvbpsi_nvod_ref_t
 */
typedef struct dvbpsi_nvod_ref_s
{
  uint16_t       i_transport_stream_id;         /*!< transport stream id */
  uint16_t       i_original_network_id;         /*!< original network id */
  uint16_t       i_service_id;                  /*!< service id */
} dvbpsi_nvod_ref_t;

/*****************************************************************************
 * dvbpsi_nvod_ref_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nvod_ref_dr_s
 * \brief "NVOD reference" descriptor structure.
 *
 * This structure is used to store a decoded "Near Video On Demand (NVOD) reference"
 * descriptor. (ETSI EN 300 468 section 6.2.26).
 */
/*!
 * \typedef struct dvbpsi_nvod_ref_dr_s dvbpsi_nvod_ref_dr_t
 * \brief dvbpsi_nvod_ref_dr_t type definition.
 */
typedef struct dvbpsi_nvod_ref_dr_s
{
  uint8_t               i_references;           /*!< number of nvod references */
  dvbpsi_nvod_ref_t     p_nvod_refs[43];        /*!< NVOD references */
} dvbpsi_nvod_ref_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeNVODReferenceDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_nvod_ref_dr_t * dvbpsi_DecodeNVODReferenceDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "NVOD reference" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "NVOD reference" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_nvod_ref_dr_t* dvbpsi_DecodeNVODReferenceDr(dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenNVODReferenceDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_GenNVODReferenceDr(dvbpsi_nvod_ref_dr_t *p_decoded,
                                          bool b_duplicate);
 * \brief "NVOD reference" descriptor generator.
 * \param p_decoded pointer to a decoded "NVOD reference" descriptor structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t *dvbpsi_GenNVODReferenceDr(dvbpsi_nvod_ref_dr_t * p_decoded,
                                         bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_4b.h"
#endif /* DR_4B_H_ */
