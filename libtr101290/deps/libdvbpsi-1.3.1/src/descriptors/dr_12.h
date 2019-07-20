/*
Copyright (C) 2015 Daniel Kamil Kozar

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*!
 * \file <dr_12.h>
 * \author Daniel Kamil Kozar <dkk089 at gmail.com>
 * \brief Application interface for the MPEG-2 IBP descriptor decoder and
 * generator.
 *
 * Application interface for the MPEG-2 IBP descriptor decoder and generator.
 * This descriptor's definition can be found in ISO/IEC 13818-1 revision 2014/10
 * section 2.6.34.
 */

#ifndef _DVBPSI_DR_12_H_
#define _DVBPSI_DR_12_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \struct dvbpsi_ibp_dr_s
 * \brief IBP descriptor structure.
 *
 * This structure is used to store a decoded IBP descriptor. (ISO/IEC 13818-1
 * section 2.6.34).
 */

/*!
 * \typedef struct dvbpsi_ibp_dr_s dvbpsi_ibp_dr_t
 * \brief dvbpsi_ibp_dr_s type definition.
 */
typedef struct dvbpsi_ibp_dr_s
{
  bool          b_closed_gop_flag; /*!< closed_gop_flag */
  bool          b_identical_gop_flag; /*!< identical_gop_flag */
  uint16_t      i_max_gop_length; /*!< max_gop_length */
} dvbpsi_ibp_dr_t;

/*!
 * \brief IBP descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return A pointer to a new IBP descriptor structure which contains the
 * decoded data.
 */
dvbpsi_ibp_dr_t* dvbpsi_DecodeIBPDr(dvbpsi_descriptor_t * p_descriptor);

/*!
 * \brief IBP descriptor generator.
 * \param p_decoded pointer to a decoded IBP descriptor structure.
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenIBPDr(dvbpsi_ibp_dr_t * p_decoded);

#ifdef __cplusplus
}
#endif

#else
#error "Multiple inclusions of dr_12.h"
#endif
