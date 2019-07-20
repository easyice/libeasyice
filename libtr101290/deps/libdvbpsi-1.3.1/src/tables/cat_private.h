/*****************************************************************************
 * cat_private.h: private CAT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt_private.h which was written by
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

#ifndef _DVBPSI_CAT_PRIVATE_H_
#define _DVBPSI_CAT_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_cat_decoder_t
 *****************************************************************************
 * CAT decoder.
 *****************************************************************************/
typedef struct dvbpsi_cat_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_cat_callback           pf_cat_callback;
    void *                        p_cb_data;

    dvbpsi_cat_t                  current_cat;
    dvbpsi_cat_t *                p_building_cat;

} dvbpsi_cat_decoder_t;

/*****************************************************************************
 * dvbpsi_cat_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_cat_sections_gather(dvbpsi_t* p_dvbpsi, dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_cat_sections_decode
 *****************************************************************************
 * CAT decoder.
 *****************************************************************************/
void dvbpsi_cat_sections_decode(dvbpsi_cat_t* p_cat,
                                dvbpsi_psi_section_t* p_section);

#else
#error "Multiple inclusions of cat_private.h"
#endif

