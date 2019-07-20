/*****************************************************************************
 * tot_private.h: private TDT/TOT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#ifndef _DVBPSI_TOT_PRIVATE_H_
#define _DVBPSI_TOT_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_tot_decoder_t
 *****************************************************************************
 * TDT/TOT decoder.
 *****************************************************************************/
typedef struct dvbpsi_tot_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_tot_callback           pf_tot_callback;
    void *                        p_cb_data;

    /* */
    dvbpsi_tot_t                  current_tot;
    dvbpsi_tot_t                  *p_building_tot;

} dvbpsi_tot_decoder_t;


/*****************************************************************************
 * dvbpsi_tot_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_tot_sections_gather(dvbpsi_t* p_dvbpsi,
                              dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_tot_sections_decode
 *****************************************************************************
 * TDT/TOT decoder.
 *****************************************************************************/
void dvbpsi_tot_sections_decode(dvbpsi_t* p_dvbpsi, dvbpsi_tot_t* p_tot,
                              dvbpsi_psi_section_t* p_section);

#else
#error "Multiple inclusions of tot_private.h"
#endif
