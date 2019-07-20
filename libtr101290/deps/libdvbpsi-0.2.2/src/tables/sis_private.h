/*****************************************************************************
 * sis_private.h: private SIS structures
 *----------------------------------------------------------------------------
 * Copyright (c) 2011 VideoLAN
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
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef _DVBPSI_SIS_PRIVATE_H_
#define _DVBPSI_SIS_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_sis_decoder_t
 *****************************************************************************
 * SIS decoder.
 *****************************************************************************/
typedef struct dvbpsi_sis_decoder_s
{
  dvbpsi_sis_callback           pf_callback;
  void *                        p_cb_data;

  /* */
  dvbpsi_sis_t                  *current_sis;
  dvbpsi_sis_t                  *p_building_sis;

  int                           b_current_valid;

} dvbpsi_sis_decoder_t;


/*****************************************************************************
 * dvbpsi_GatherSISSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_GatherSISSections(dvbpsi_decoder_t* p_psi_decoder,
                              void* p_private_decoder,
                              dvbpsi_psi_section_t* p_section);


/*****************************************************************************
 * dvbpsi_DecodeSISSection
 *****************************************************************************
 * SIS decoder.
 *****************************************************************************/
__attribute__((deprecated))
void dvbpsi_DecodeSISSections(dvbpsi_sis_t* p_sis,
                              dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of sis_private.h"
#endif

