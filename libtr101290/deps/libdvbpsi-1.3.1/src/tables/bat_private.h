/*****************************************************************************
 * bat_private.h: private BAT structures
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: bat_private.h 88 2004-02-24 14:31:18Z sam $
 *
 * Authors: Zhu zhenglu <zhuzlu@gmail.com>
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
 *
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/

#ifndef _DVBPSI_BAT_PRIVATE_H_
#define _DVBPSI_BAT_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_bat_decoder_t
 *****************************************************************************
 * BAT decoder.
 *****************************************************************************/
typedef struct dvbpsi_bat_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_bat_callback           pf_bat_callback;
    void *                        p_cb_data;

    dvbpsi_bat_t                  current_bat;
    dvbpsi_bat_t *                p_building_bat;

} dvbpsi_bat_decoder_t;

/*****************************************************************************
 * dvbpsi_bat_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_bat_sections_gather(dvbpsi_t* p_dvbpsi, dvbpsi_decoder_t* p_decoder,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_bat_sections_decode
 *****************************************************************************
 * BAT decoder.
 *****************************************************************************/
void dvbpsi_bat_sections_decode(dvbpsi_bat_t* p_bat,
                              dvbpsi_psi_section_t* p_section);

#else
#error "Multiple inclusions of bat_private.h"
#endif
