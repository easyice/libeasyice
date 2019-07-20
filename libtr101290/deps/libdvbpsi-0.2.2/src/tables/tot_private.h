/*****************************************************************************
 * tot_private.h: private TDT/TOT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#ifndef _DVBPSI_TOT_PRIVATE_H_
#define _DVBPSI_TOT_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_tot_decoder_t
 *****************************************************************************
 * TDT/TOT decoder.
 *****************************************************************************/
typedef struct dvbpsi_tot_decoder_s
{
  dvbpsi_tot_callback           pf_callback;
  void *                        p_cb_data;

} dvbpsi_tot_decoder_t;


/*****************************************************************************
 * dvbpsi_GatherTOTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_GatherTOTSections(dvbpsi_decoder_t* p_decoder,
                              void * p_private_decoder,
                              dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_ValidTOTSection
 *****************************************************************************
 * Check the CRC_32 if the section has b_syntax_indicator set.
 *****************************************************************************/
__attribute__((deprecated))
int dvbpsi_ValidTOTSection(dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_DecodeTOTSections
 *****************************************************************************
 * TDT/TOT decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_DecodeTOTSections(dvbpsi_tot_t* p_tot,
                              dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of tot_private.h"
#endif
