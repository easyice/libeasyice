/*****************************************************************************
 * dr_08.h
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: dr_08.h,v 1.3 2002/05/10 23:50:36 bozo Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 * \file <dr_08.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the MPEG 2 "video window"
 * descriptor decoder and generator.
 *
 * Application interface for the MPEG 2 "video window" descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ISO/IEC 13818-1 section 2.6.14.
 */

#ifndef _DVBPSI_DR_08_H_
#define _DVBPSI_DR_08_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_vwindow_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_vwindow_dr_s
 * \brief "video window" descriptor structure.
 *
 * This structure is used to store a decoded "video window"
 * descriptor. (ISO/IEC 13818-1 section 2.6.14).
 */
/*!
 * \typedef struct dvbpsi_vwindow_dr_s dvbpsi_vwindow_dr_t
 * \brief dvbpsi_vwindow_dr_t type definition.
 */
typedef struct dvbpsi_vwindow_dr_s
{
  uint16_t      i_horizontal_offset;    /*!< horizontal_offset */
  uint16_t      i_vertical_offset;      /*!< vertical_offset */
  uint8_t       i_window_priority;      /*!< window_priority */

} dvbpsi_vwindow_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeVWindowDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_vwindow_dr_t * dvbpsi_DecodeVWindowDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "video window" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "video window" descriptor structure which
 * contains the decoded data.
 */
dvbpsi_vwindow_dr_t* dvbpsi_DecodeVWindowDr(dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenVWindowDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenVWindowDr(
                        dvbpsi_vwindow_dr_t * p_decoded, int b_duplicate)
 * \brief "video window" descriptor generator.
 * \param p_decoded pointer to a decoded "video window" descriptor structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenVWindowDr(dvbpsi_vwindow_dr_t * p_decoded,
                                          int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_08.h"
#endif

