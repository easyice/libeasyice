/*****************************************************************************
 * dr_47.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: dr_47.h,v 1.2 2002/12/12 10:19:32 jobi Exp $
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
 * \file <dr_47.h>
 * \author Johan Bilien <jobi@via.ecp.fr>
 * \brief Application interface for the DVB "bouquet name"
 * descriptor decoder and generator.
 *
 * Application interface for the MPEG 2 "bouquet name" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.3.
 */

#ifndef _DVBPSI_DR_47_H_
#define _DVBPSI_DR_47_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_bouquet_name_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_bouquet_name_dr_s
 * \brief "bouquet name" descriptor structure.
 *
 * This structure is used to store a decoded "bouquet name"
 * descriptor. (ETSI EN 300 468 section 6.2.3).
 */
/*!
 * \typedef struct dvbpsi_bouquet_name_dr_s dvbpsi_bouquet_name_dr_t
 * \brief dvbpsi_bouquet_name_dr_t type definition.
 */
typedef struct dvbpsi_bouquet_name_dr_s
{
  uint8_t      i_name_length;       /*!< length of thr i_char array */
  uint8_t      i_char[255];         /*!< char */

} dvbpsi_bouquet_name_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeBouquetNameDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_bouquet_name_dr_t * dvbpsi_DecodeBouquetNameDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "bouquet name" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "bouquet name" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_bouquet_name_dr_t* dvbpsi_DecodeBouquetNameDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenBouquetNameDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenStuffingDr(
                        dvbpsi_bouquet_name_dr_t * p_decoded, int b_duplicate)
 * \brief "bouquet name" descriptor generator.
 * \param p_decoded pointer to a decoded "bouquet name" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenBouquetNameDr(
                                        dvbpsi_bouquet_name_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_47.h"
#endif

