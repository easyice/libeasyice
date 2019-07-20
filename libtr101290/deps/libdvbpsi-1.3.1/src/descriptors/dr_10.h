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
 * \file <dr_10.h>
 * \author Daniel Kamil Kozar <dkk089@gmail.com>
 * \brief Application interface for the MPEG-2 smoothing buffer descriptor
 * decoder and generator.
 *
 * Application interface for the MPEG-2 smoothing buffer descriptor decoder and
 * generator. This descriptor's definition can be found in ISO/IEC 13818-1
 * revision 2014/10 section 2.6.30.
 */

#ifndef _DVBPSI_DR_10_H_
#define _DVBPSI_DR_10_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \struct dvbpsi_smoothing_buffer_dr_s
 * \brief Smoothing buffer descriptor structure.
 *
 * This structure is used to store a decoded smoothing buffer descriptor.
 * (ISO/IEC 13818-1 section 2.6.30).
 */

/*!
 * \typedef struct dvbpsi_smoothing_buffer_dr_s dvbpsi_smoothing_buffer_dr_t
 * \brief dvbpsi_smoothing_buffer_dr_t type definition.
 */
typedef struct dvbpsi_smoothing_buffer_dr_s
{
  /*! Value of the leak rate out of the SBn buffer, in units of 400 bits/s. */
  uint32_t      i_sb_leak_rate;
  
  /*! Value of the size of the multiplexing buffer smoothing buffer SBn. */
  uint32_t      i_sb_size;
} dvbpsi_smoothing_buffer_dr_t;

/*!
 * \brief Smoothing buffer descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return A pointer to a new smoothing buffer descriptor structure which
 * contains the decoded data.
 */
dvbpsi_smoothing_buffer_dr_t* dvbpsi_DecodeSmoothingBufferDr(
                                      dvbpsi_descriptor_t * p_descriptor);

/*!
 * \brief Smoothing buffer descriptor generator.
 * \param p_decoded pointer to a decoded smoothing buffer descriptor structure.
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenSmoothingBufferDr(
                                      dvbpsi_smoothing_buffer_dr_t * p_decoded);

#ifdef __cplusplus
}
#endif

#else
#error "Multiple inclusions of dr_10.h"
#endif
