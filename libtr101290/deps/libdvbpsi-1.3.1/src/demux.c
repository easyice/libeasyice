/*****************************************************************************
 * demux.c: DVB subtables demux functions.
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"
#include "demux.h"

/*****************************************************************************
 * dvbpsi_AttachDemux
 *****************************************************************************
 * Creation of the demux structure
 *****************************************************************************/
bool dvbpsi_AttachDemux(dvbpsi_t *            p_dvbpsi,
                        dvbpsi_demux_new_cb_t pf_new_cb,
                        void *                p_new_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder == NULL);

    dvbpsi_demux_t *p_demux;
    p_demux = (dvbpsi_demux_t*) dvbpsi_decoder_new(&dvbpsi_Demux, 4096, true,
                                                   sizeof(dvbpsi_demux_t));
    if (p_demux == NULL)
        return false;

    /* Subtables demux configuration */
    p_demux->p_first_subdec = NULL;
    p_demux->pf_new_callback = pf_new_cb;
    p_demux->p_new_cb_data = p_new_cb_data;

    p_dvbpsi->p_decoder = DVBPSI_DECODER(p_demux);
    return true;
}

/*****************************************************************************
 * dvbpsi_demuxGetSubDec
 *****************************************************************************
 * Finds a subtable decoder given the table id and extension
 *****************************************************************************/
dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t * p_demux,
                                              uint8_t i_table_id,
                                              uint16_t i_extension)
{
    uint32_t i_id = (uint32_t)i_table_id << 16 |(uint32_t)i_extension;
    dvbpsi_demux_subdec_t * p_subdec = p_demux->p_first_subdec;

    while (p_subdec)
    {
        if (p_subdec->i_id == i_id)
            break;

        p_subdec = p_subdec->p_next;
    }

    return p_subdec;
}

/*****************************************************************************
 * dvbpsi_Demux
 *****************************************************************************
 * Sends a PSI section to the right subtable decoder
 *****************************************************************************/
void dvbpsi_Demux(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t * p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_decoder;
    dvbpsi_demux_subdec_t * p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
                                                             p_section->i_extension);
    if (p_subdec == NULL)
    {
        /* Tell the application we found a new subtable, so that it may attach a
         * subtable decoder */
        p_demux->pf_new_callback(p_dvbpsi, p_section->i_table_id, p_section->i_extension,
                                 p_demux->p_new_cb_data);

        /* Check if a new subtable decoder is available */
        p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
                                         p_section->i_extension);
    }

    if (p_subdec)
        p_subdec->pf_gather(p_dvbpsi, p_subdec->p_decoder, p_section);
    else
        dvbpsi_DeletePSISections(p_section);
}

/*****************************************************************************
 * dvbpsi_DetachDemux
 *****************************************************************************
 * Destroys a demux structure
 *****************************************************************************/
void dvbpsi_DetachDemux(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_decoder;
    dvbpsi_demux_subdec_t* p_subdec = p_demux->p_first_subdec;

    while (p_subdec)
    {
        dvbpsi_demux_subdec_t* p_subdec_temp = p_subdec;
        p_subdec = p_subdec->p_next;

        if (p_subdec_temp->pf_detach)
            p_subdec_temp->pf_detach(p_dvbpsi, (p_subdec_temp->i_id >> 16) & 0xFFFF,
                                     p_subdec_temp->i_id & 0xFFFF);
        else free(p_subdec_temp);
    }

    dvbpsi_decoder_delete(p_dvbpsi->p_decoder);
    p_dvbpsi->p_decoder = NULL;
}

/*****************************************************************************
 * dvbpsi_NewDemuxSubDecoder
 *****************************************************************************
 * Allocate a new demux sub table decoder
 *****************************************************************************/
dvbpsi_demux_subdec_t *dvbpsi_NewDemuxSubDecoder(const uint8_t i_table_id,
                                                 const uint16_t i_extension,
                                                 dvbpsi_demux_detach_cb_t pf_detach,
                                                 dvbpsi_demux_gather_cb_t pf_gather,
                                                 dvbpsi_decoder_t *p_decoder)
{
    assert(pf_gather);
    assert(pf_detach);

    dvbpsi_demux_subdec_t *p_subdec = calloc(1, sizeof(dvbpsi_demux_subdec_t));
    if (p_subdec == NULL)
        return NULL;

    uint32_t i_id = (uint32_t)i_table_id << 16 | (uint32_t)i_extension;

    /* subtable decoder configuration */
    p_subdec->i_id = i_id;
    p_subdec->p_decoder = p_decoder;
    p_subdec->pf_gather = pf_gather;
    p_subdec->pf_detach = pf_detach;

    return p_subdec;
}

/*****************************************************************************
 * dvbpsi_DeleteDemuxSubDecoder
 *****************************************************************************
 * Free a demux sub table decoder
 *****************************************************************************/
void dvbpsi_DeleteDemuxSubDecoder(dvbpsi_demux_subdec_t *p_subdec)
{
    if (!p_subdec)
        return;
    /* FIXME: find a saner way to release private decoder resources */
    dvbpsi_decoder_delete(p_subdec->p_decoder);
    free(p_subdec);
    p_subdec = NULL;
}

/*****************************************************************************
 * dvbpsi_AttachDemuxSubDecoder
 *****************************************************************************/
void dvbpsi_AttachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec)
{
    assert(p_demux);
    assert(p_subdec);

    if (!p_demux || !p_subdec)
        abort();

    p_subdec->p_next = p_demux->p_first_subdec;
    p_demux->p_first_subdec = p_subdec;
}

/*****************************************************************************
 * dvbpsi_DetachDemuxSubDecoder
 *****************************************************************************/
void dvbpsi_DetachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec)
{
    assert(p_demux);
    assert(p_subdec);

    if (!p_demux || !p_subdec)
        abort();

    assert(p_demux->p_first_subdec);

    dvbpsi_demux_subdec_t** pp_prev_subdec;
    pp_prev_subdec = &p_demux->p_first_subdec;
    while(*pp_prev_subdec != p_subdec)
        pp_prev_subdec = &(*pp_prev_subdec)->p_next;

    *pp_prev_subdec = p_subdec->p_next;
}
