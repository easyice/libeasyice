/*****************************************************************************
 * dr_53.h
 * Copyright (C) 2013, M2X BV
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
 * \file <dr_53.h>
 * \author Jean-Paul Saman <jpsaman@videolan.org>
 * \brief Application interface for the DVB "CA identifier"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "CA identifier" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.5.
 */

#ifndef _DVBPSI_DR_53_H_
#define _DVBPSI_DR_53_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_ca_system_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_ca_system_s
 * \brief  CA system identifier structure
 *
 * This structure is used to identify the individual CA system types.
 */
/*!
 * \typedef struct dvbpsi_ca_system_s dvbpsi_ca_system_t
 * \brief dvbpsi_ca_system_t type definition. The CA system identifier
 * field is allocated using values found in ETSI TS 101 162.
 */
typedef struct dvbpsi_ca_system_s
{
  uint16_t      i_ca_system_id;            /*!< CA system identifier */

} dvbpsi_ca_system_t;

/*!
 * \def DVBPSI_CA_SYSTEM_ID_DR_MAX
 * \brief Maximum number of dvbpsi_ca_system_t entries present in @see dvbpsi_ca_identifier_dr_t
 */
#define DVBPSI_CA_SYSTEM_ID_DR_MAX 127

/*****************************************************************************
 * dvbpsi_ca_identifier_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_ca_identifier_dr_s
 * \brief "CA identifier" descriptor structure.
 *
 * This structure is used to store a decoded "CA identifier"
 * descriptor. (ETSI EN 300 468 section 6.2.5).
 */
/*!
 * \typedef struct dvbpsi_ca_identifier_dr_s dvbpsi_ca_identifier_dr_t
 * \brief dvbpsi_ca_identifier_dr_t type definition.
 */
typedef struct dvbpsi_ca_identifier_dr_s
{
    uint8_t            i_number;                /*!< number of CA system identifiers */
    dvbpsi_ca_system_t p_system[DVBPSI_CA_SYSTEM_ID_DR_MAX]; /*!< CA system identifiers */

} dvbpsi_ca_identifier_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeCAIdentifierDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_da_identifier_dr_t * dvbpsi_DecodeCAIdentifierDr(
                                        dvbpsi_descriptor_t *p_descriptor)
 * \brief "DA identifier" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "CA identifier" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_ca_identifier_dr_t* dvbpsi_DecodeCAIdentifierDr(dvbpsi_descriptor_t *p_descriptor);


/*****************************************************************************
 * dvbpsi_GenStreamIdentifierDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_GenCAIdentifierDr(
                        dvbpsi_ca_identifier_dr_t *p_decoded, bool b_duplicate)
 * \brief "CA identifier" descriptor generator.
 * \param p_decoded pointer to a decoded "CA identifier" descriptor
 * structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t *dvbpsi_GenCAIdentifierDr(dvbpsi_ca_identifier_dr_t *p_decoded,
                                              bool b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_53.h"
#endif
