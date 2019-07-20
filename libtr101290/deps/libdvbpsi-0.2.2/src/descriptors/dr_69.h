/*****************************************************************************
 * dr_69.h
 * Copyright (C) 2007-2010 VideoLAN
 * $Id$
 *
 * Authors: Pinkava Jiri <master_up@post.cz>
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
 * \file <dr_69.h>
 * \author Jiri Pinkava <master_up@post.cz>
 * \brief Application interface for the PDC (Programme Delivery Control)
 * descriptor decoder and generator.
 *
 * Application interface for the MPEG 2 TS PDC descriptor decoder and generator
 * This descriptor's definition can be found in ETSC EN 300 231
 */

#ifndef _DVBPSI_DR_69_H_
#define _DVBPSI_DR_69_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_PDC_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_PDC_dr_s
 * \brief PDC descriptor structure.
 *
 * This structure is used to store a decoded PDC descriptor.
 * (ETSI EN 300 231).
 */
/*!
 * \typedef struct dvbpsi_PDC_dr_s dvbpsi_PDC_dr_t
 * \brief dvbpsi_PDC_dr_t type definition.
 */
typedef struct dvbpsi_PDC_dr_s
{
  /* pdc[0] ~= day; PDC[1] ~= month; PDC[2] ~= hour; PDC[3] ~= min; */
  uint8_t   i_PDC[4];          /*!< PDC */
} dvbpsi_PDC_dr_t;


/*****************************************************************************
 * dvbpsi_DecodePDCDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_PDC_dr_t * dvbpsi_DecodePDCDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief PDC descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "video stream" descriptor structure which
 * contains the decoded data.
 */
dvbpsi_PDC_dr_t* dvbpsi_DecodePDCDr(dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenPDCDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenVStreamDr(
                        dvbpsi_PDC_dr_t * p_decoded, int b_duplicate)
 * \brief PDC descriptor generator.
 * \param p_decoded pointer to a decoded PDC descriptor structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenPDCDr(dvbpsi_PDC_dr_t * p_decoded,
                                          int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_69.h"
#endif

