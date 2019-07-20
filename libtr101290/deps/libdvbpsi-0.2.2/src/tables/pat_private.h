/*****************************************************************************
 * pat_private.h: private PAT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: pat_private.h,v 1.1 2002/01/22 20:30:16 bozo Exp $
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
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef _DVBPSI_PAT_PRIVATE_H_
#define _DVBPSI_PAT_PRIVATE_H_


/*****************************************************************************
 * dvbpsi_pat_decoder_t
 *****************************************************************************
 * PAT decoder.
 *****************************************************************************/
typedef struct dvbpsi_pat_decoder_s
{
  dvbpsi_pat_callback           pf_callback;
  void *                        p_cb_data;

  dvbpsi_pat_t                  current_pat;
  dvbpsi_pat_t *                p_building_pat;

  int                           b_current_valid;

  uint8_t                       i_last_section_number;
  dvbpsi_psi_section_t *        ap_sections [256];

} dvbpsi_pat_decoder_t;


/*****************************************************************************
 * dvbpsi_GatherPATSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_GatherPATSections(dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_DecodePATSection
 *****************************************************************************
 * PAT decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_DecodePATSections(dvbpsi_pat_t* p_pat,
                              dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of pat_private.h"
#endif

