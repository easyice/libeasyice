/*****************************************************************************
 * rst_private.h: private RST structures
 *----------------------------------------------------------------------------
 * Copyright (c) 2012 VideoLAN
 * $Id$
 *
 * Authors: Corno Roberto <corno.roberto@gmail.com>
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

#ifndef _DVBPSI_RST_PRIVATE_H_
#define _DVBPSI_RST_PRIVATE_H_

/*****************************************************************************
 * dvbpsi_rst_decoder_t
 *****************************************************************************
 * RST decoder.
 *****************************************************************************/
typedef struct dvbpsi_rst_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_rst_callback           pf_rst_callback;
    void *                        p_cb_data;

    dvbpsi_rst_t                  current_rst;
    dvbpsi_rst_t *                p_building_rst;

} dvbpsi_rst_decoder_t;

/*****************************************************************************
 * dvbpsi_GatherRSTSections
 *****************************************************************************
 * Callback for dvbpsi_rst_sections_gather PSI decoder.
 *****************************************************************************/
void dvbpsi_rst_sections_gather(dvbpsi_t* p_dvbpsi, dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_rst_sections_decode
 *****************************************************************************
 * RST decoder.
 *****************************************************************************/
void dvbpsi_rst_sections_decode(dvbpsi_rst_t* p_rst,
                              dvbpsi_psi_section_t* p_section);


#else
#error "Multiple inclusions of rst_private.h"
#endif


