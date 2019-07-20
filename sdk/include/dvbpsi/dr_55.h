/*****************************************************************************
 * dr_55.h
 * Copyright (C) 2004-2010 VideoLAN
 * $Id: dr_55.h 88 2004-02-24 14:31:18Z sam $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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
 * \file <dr_55.h>
 * \author Christophe Massiot <massiot@via.ecp.fr>
 * \brief Parental rating descriptor parsing.
 *
 * Parental rating descriptor parsing, according to ETSI EN 300 468
 * section 6.2.26.
 */

#ifndef _DVBPSI_DR_55_H_
#define _DVBPSI_DR_55_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_parental_rating_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_parental_rating_s
 * \brief  one subtitle structure.
 *
 * This structure is used since parental_rating_descriptor will contain several
 * coutry/rating pairs.
 */
/*!
 * \typedef struct dvbpsi_parental_rating_s dvbpsi_parental_rating_t
 * \brief dvbpsi_parental_rating_t type definition.
 */
typedef struct dvbpsi_parental_rating_s
{
  uint32_t      i_country_code;
  uint8_t       i_rating;

} dvbpsi_parental_rating_t;


/*****************************************************************************
 * dvbpsi_parental_rating_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_parental_rating_dr_s
 * \brief "parental_rating" descriptor structure.
 *
 * This structure is used to store a decoded "parental_rating"
 * descriptor. (ETSI EN 300 468 section 6.2.26).
 */
/*!
 * \typedef struct dvbpsi_parental_rating_dr_s dvbpsi_parental_rating_dr_t
 * \brief dvbpsi_parental_rating_dr_t type definition.
 */
typedef struct dvbpsi_parental_rating_dr_s
{
  uint8_t       i_ratings_number;
  dvbpsi_parental_rating_t p_parental_rating[64];

} dvbpsi_parental_rating_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeParentalRatingDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_parental_rating_dr_t * dvbpsi_DecodeParentalRatingDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "parental_rating" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "parental_rating" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_parental_rating_dr_t* dvbpsi_DecodeParentalRatingDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenParentalRatingDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenParentalRatingDr(
                        dvbpsi_parental_rating_dr_t * p_decoded, int b_duplicate)
 * \brief "parental_rating" descriptor generator.
 * \param p_decoded pointer to a decoded "parental_rating" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenParentalRatingDr(
                                        dvbpsi_parental_rating_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_55.h"
#endif

