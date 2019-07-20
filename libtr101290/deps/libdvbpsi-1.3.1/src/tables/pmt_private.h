/*****************************************************************************
 * pmt_private.h: private PMT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: pmt_private.h,v 1.1 2002/01/22 20:30:16 bozo Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Jean-Paul Saman <jpsaman@videolan.org>
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

#ifndef _DVBPSI_PMT_PRIVATE_H_
#define _DVBPSI_PMT_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_pmt_decoder_t
 *****************************************************************************
 * PMT decoder.
 *****************************************************************************/
typedef struct dvbpsi_pmt_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_pmt_callback           pf_pmt_callback;
    void *                        p_cb_data;

    dvbpsi_pmt_t                  current_pmt;
    dvbpsi_pmt_t *                p_building_pmt;

    uint16_t                      i_program_number;

} dvbpsi_pmt_decoder_t;

/*****************************************************************************
 * dvbpsi_pmt_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_pmt_sections_gather(dvbpsi_t *p_dvbpsi,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_pmt_sections_decode
 *****************************************************************************
 * PMT decoder.
 *****************************************************************************/
void dvbpsi_pmt_sections_decode(dvbpsi_pmt_t* p_pmt,
                                dvbpsi_psi_section_t* p_section);

#else
#error "Multiple inclusions of pmt_private.h"
#endif

