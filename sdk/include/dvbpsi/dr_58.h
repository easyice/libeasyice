/*****************************************************************************
 * dr_58.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
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
 * \file <dr_58.h>
 * \author Johann Hanne
 * \brief Application interface for the DVB "local time offset"
 * descriptor decoder and generator.
 *
 * Application interface for the DVB "local time offset" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.19.
 */

#ifndef _DVBPSI_DR_58_H_
#define _DVBPSI_DR_58_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_local_time_offset_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_local_time_offset_s
 * \brief one local time offset structure.
 *
 * This structure is used since local_time_offset will contain information
 * for several countries.
 */
/*!
 * \typedef struct dvbpsi_local_time_offset_s dvbpsi_local_time_offset_t
 * \brief dvbpsi_local_time_offset_t type definition.
 */
typedef struct dvbpsi_local_time_offset_s
{
  uint8_t       i_country_code[3];            /*!< country_code */
  uint8_t       i_country_region_id;          /*!< country_region_id */
  uint8_t       i_local_time_offset_polarity; /*!< local_time_offset_polarity */
  uint16_t      i_local_time_offset;          /*!< local_time_offset */
  uint64_t      i_time_of_change;             /*!< time_of_change */
  uint16_t      i_next_time_offset;           /*!< next_time_offset */

} dvbpsi_local_time_offset_t;


/*****************************************************************************
 * dvbpsi_local_time_offset_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_local_time_offset_dr_s
 * \brief "local time offset" descriptor structure.
 *
 * This structure is used to store a decoded "local time offset"
 * descriptor. (ETSI EN 300 468 section 6.2.19).
 */
/*!
 * \typedef struct dvbpsi_local_time_offset_dr_s dvbpsi_local_time_offset_dr_t
 * \brief dvbpsi_local_time_offset_dr_t type definition.
 */
typedef struct dvbpsi_local_time_offset_dr_s
{
  uint8_t      i_local_time_offsets_number;
  dvbpsi_local_time_offset_t p_local_time_offset[19];

} dvbpsi_local_time_offset_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeLocalTimeOffsetDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_local_time_offset_dr_t * dvbpsi_DecodeLocalTimeOffsetDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "local time offset" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "local time offset" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_local_time_offset_dr_t* dvbpsi_DecodeLocalTimeOffsetDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenLocalTimeOffsetDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenLocalTimeOffsetDr(
                        dvbpsi_local_time_offset_data_dr_t * p_decoded, int b_duplicate)
 * \brief "local time offset" descriptor generator.
 * \param p_decoded pointer to a decoded "local time offset" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenLocalTimeOffsetDr(
                                        dvbpsi_local_time_offset_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_58.h"
#endif
