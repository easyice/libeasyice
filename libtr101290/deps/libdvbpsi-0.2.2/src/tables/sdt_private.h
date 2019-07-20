/*****************************************************************************
 * sdt_private.h: private SDT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: sdt_private.h,v 1.1 2002/12/11 13:04:57 jobi Exp $
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef _DVBPSI_SDT_PRIVATE_H_
#define _DVBPSI_SDT_PRIVATE_H_


/*****************************************************************************
 * dvbpsi_sdt_decoder_t
 *****************************************************************************
 * SDT decoder.
 *****************************************************************************/
typedef struct dvbpsi_sdt_decoder_s
{
  dvbpsi_sdt_callback           pf_callback;
  void *                        p_cb_data;

  dvbpsi_sdt_t                  current_sdt;
  dvbpsi_sdt_t *                p_building_sdt;

  int                           b_current_valid;

  uint8_t                       i_last_section_number;
  dvbpsi_psi_section_t *        ap_sections [256];

} dvbpsi_sdt_decoder_t;


/*****************************************************************************
 * dvbpsi_GatherSDTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_GatherSDTSections(dvbpsi_decoder_t* p_psi_decoder,
                      void* p_private_decoder,
                              dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_DecodeSDTSection
 *****************************************************************************
 * SDT decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_DecodeSDTSections(dvbpsi_sdt_t* p_sdt,
                              dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of sdt_private.h"
#endif

