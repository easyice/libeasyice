/*****************************************************************************
 * dr_5a.h
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
 * \file <dr_5a.h>
 * \author Johann Hanne
 * \brief Application interface for the DVB terrestrial delivery system
 *        descriptor decoder and generator.
 *
 * Application interface for the DVB terrestrial delivery system descriptor
 * decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.13.4.
 */

#ifndef _DVBPSI_DR_5A_H_
#define _DVBPSI_DR_5A_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_terr_deliv_sys_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_terr_deliv_sys_dr_s
 * \brief terrestrial delivery system descriptor structure.
 *
 * This structure is used to store a decoded terrestrial delivery system
 * descriptor. (ETSI EN 300 468 section 6.2.13.4).
 */
/*!
 * \typedef struct dvbpsi_terr_deliv_sys_dr_s dvbpsi_terr_deliv_sys_dr_t
 * \brief dvbpsi_terr_deliv_sys_dr_t type definition.
 */
typedef struct dvbpsi_terr_deliv_sys_dr_s
{
  uint32_t     i_centre_frequency;              /*!< centre frequency */
  uint8_t      i_bandwidth;                     /*!< bandwidth */
  uint8_t      i_priority;                      /*!< priority */
  uint8_t      i_time_slice_indicator;          /*!< Time_Slicing_indicator */
  uint8_t      i_mpe_fec_indicator;             /*!< MPE-FEC_indicator */
  uint8_t      i_constellation;                 /*!< constellation */
  uint8_t      i_hierarchy_information;         /*!< hierarchy_information */
  uint8_t      i_code_rate_hp_stream;           /*!< code_rate-HP_stream */
  uint8_t      i_code_rate_lp_stream;           /*!< code_rate-LP_stream */
  uint8_t      i_guard_interval;                /*!< guard_interval */
  uint8_t      i_transmission_mode;             /*!< transmission_mode */
  uint8_t      i_other_frequency_flag;          /*!< other_frequency_flag */

} dvbpsi_terr_deliv_sys_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeTerrDelivSysDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_terr_deliv_sys_dr_t * dvbpsi_DecodeTerrDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief terrestrial delivery system descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new terrestrial delivery system descriptor structure
 * which contains the decoded data.
 */
dvbpsi_terr_deliv_sys_dr_t* dvbpsi_DecodeTerrDelivSysDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenTerrDelivSysDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenTerrDelivSysDr(
                        dvbpsi_terr_deliv_sys_dr_t * p_decoded, int b_duplicate)
 * \brief terrestrial delivery system descriptor generator.
 * \param p_decoded pointer to a decoded terrestrial delivery system descriptor
 * descriptor structure
 * \param b_duplicate if non zero then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenTerrDelivSysDr(
                                        dvbpsi_terr_deliv_sys_dr_t * p_decoded,
                                        int b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_5a.h"
#endif
