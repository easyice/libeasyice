/*****************************************************************************
 * dr_8a.h
 * Copyright (c) 2010 VideoLAN
 * $Id$
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
 * \file <dr_8a.h>
 * \author Jean-Paul Saman <jpsaman@videolan.org>
 * \brief CUE Identifier descriptor parsing.
 *
 * CUE Identifier descriptor parsing, according to SCTE 35 2004
 * section 6.2.
 */

#ifndef _DVBPSI_DR_8A_H_
#define _DVBPSI_DR_8A_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_cuei_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_cuei_dr_s
 * \brief "CUE Identifier" descriptor structure.
 *
 * The CUE Identifier descriptor is used to label the PIDs
 * that carry splice commands. (SCTE 35 2004 section 6.2.)
 */
/*!
 * \typedef struct dvbpsi_cuei_dr_s dvbpsi_cuei_dr_t
 * \brief dvbpsi_cuei_dr_t type definition.
 */
typedef struct dvbpsi_cuei_dr_s
{
  uint8_t      i_cue_stream_type; /*!< indicate type of splice commands */

} dvbpsi_cuei_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeCUEIDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_cuei_dr_t * dvbpsi_DecodeCUEIDr(dvbpsi_descriptor_t * p_descriptor)
 * \brief "CUEI" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "CUEI" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_cuei_dr_t* dvbpsi_DecodeCUEIDr(dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenCUEIDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenCUEIDr(
                        dvbpsi_cuei_dr_t * p_decoded, int b_duplicate)
 * \brief "CUEI" descriptor generator.
 * \param p_decoded pointer to a decoded "CUEI" descriptor
 * structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenCUEIDr(dvbpsi_cuei_dr_t * p_decoded);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_8a.h"
#endif

