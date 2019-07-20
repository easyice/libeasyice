/*****************************************************************************
 * dr_49.h
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
 * \file <dr_49.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the DVB "country availability"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "country availability" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.10.
 */

#ifndef DR_49_H_
#define DR_49_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_country_availability_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_country_availability_dr_t
 * \brief "country availability" descriptor structure.
 *
 * This structure is used to store a decoded "country availability"
 * descriptor. (ETSI EN 300 468 section 6.2.10).
 */
/*!
 * \typedef struct dvbpsi_country_availability_dr_s dvbpsi_country_availability_dr_t
 * \brief dvbpsi_country_availability_dr_t type definition.
 */
/*!
 * \struct dvbpsi_country_availability_dr_s
 * \brief dvbpsi_country_availability_dr_s type definition @see dvbpsi_country_availability_dr_t
 */
typedef struct dvbpsi_country_availability_dr_s
{
  bool          b_country_availability_flag;    /*!< country availability flag */
  uint8_t       i_code_count;                   /*!< length of the i_iso_639_code
                                                array */
  struct {
    iso_639_language_code_t  iso_639_code;      /*!< ISO_639 language code */
  } code[84];                                   /*!< ISO_639_language_code array */

} dvbpsi_country_availability_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeCountryAvailabilityDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_country_availability_dr_t * dvbpsi_DecodeCountryAvailability(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "country availability" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "country availability" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_country_availability_dr_t* dvbpsi_DecodeCountryAvailability(
                                        dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenCountryAvailabilityDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenCountryAvailabilityDr(
                        dvbpsi_country_availability_dr_t * p_decoded,
                        bool b_duplicate)
 * \brief "country availability" descriptor generator.
 * \param p_decoded pointer to a decoded "country availability" descriptor
 * structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenCountryAvailabilityDr(
		                        dvbpsi_country_availability_dr_t * p_decoded,
                                        bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_49.h"
#endif /* DR_49_H_ */
