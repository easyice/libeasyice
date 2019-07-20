/*****************************************************************************
 * sis_private.h: private SIS structures
 *----------------------------------------------------------------------------
 * Copyright (c) 2010-2011 VideoLAN
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
    DVBPSI_DECODER_COMMON

    dvbpsi_sis_callback           pf_sis_callback;
    void *                        p_cb_data;

    /* */
    dvbpsi_sis_t                  current_sis;
    dvbpsi_sis_t                  *p_building_sis;

} dvbpsi_sis_decoder_t;

/*****************************************************************************
 * dvbpsi_sis_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_sis_sections_gather(dvbpsi_t* p_dvbpsi,
                              dvbpsi_decoder_t * p_decoder,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_sis_sections_decode
 *****************************************************************************
 * SIS decoder.
 *****************************************************************************/
void dvbpsi_sis_sections_decode(dvbpsi_t* p_dvbpsi, dvbpsi_sis_t* p_sis,
                               dvbpsi_psi_section_t* p_section);

#else
#error "Multiple inclusions of sis_private.h"
#endif
