/*
Copyright (C) 2006-2012 Adam Charrett

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

atsc_ett.c

Decode PSIP Extended Text Table.

*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../psi.h"
#include "../descriptor.h"
#include "../demux.h"

#include "atsc_ett.h"

/*****************************************************************************
 * dvbpsi_atsc_ett_decoder_s
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
typedef struct dvbpsi_atsc_ett_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_atsc_ett_callback      pf_ett_callback;
    void *                        p_cb_data;

    dvbpsi_atsc_ett_t             current_ett;
    dvbpsi_atsc_ett_t *           p_building_ett;

} dvbpsi_atsc_ett_decoder_t;

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
static void dvbpsi_atsc_GatherETTSections(dvbpsi_t * p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSections
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeETTSections(dvbpsi_atsc_ett_t* p_ett,
                                          dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachETT
 *****************************************************************************
 * Initialize a ETT decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_atsc_AttachETT(dvbpsi_t * p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_atsc_ett_decoder_t* p_ett_decoder;
    p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*) dvbpsi_decoder_new(NULL,
                                                  0, true, sizeof(dvbpsi_atsc_ett_decoder_t));
    if (p_ett_decoder == NULL)
        return false;

    /* PSI decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachETT,
                                         dvbpsi_atsc_GatherETTSections, DVBPSI_DECODER(p_ett_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_ett_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* ETT decoder information */
    p_ett_decoder->pf_ett_callback = pf_callback;
    p_ett_decoder->p_cb_data = p_cb_data;
    p_ett_decoder->p_building_ett = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachETT
 *****************************************************************************
 * Close a ETT decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_atsc_DetachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT Decoder",
                     "No such ETT decoder (table_id == 0x%02x,"
                     "extension == 0x%04x)",
                     i_table_id, i_extension);
        return;
    }

    dvbpsi_atsc_ett_decoder_t* p_ett_decoder;
    p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)p_subdec->p_decoder;
    if (!p_ett_decoder)
        return;

    if (p_ett_decoder->p_building_ett)
        dvbpsi_atsc_DeleteETT(p_ett_decoder->p_building_ett);
    p_ett_decoder->p_building_ett = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitETT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_ett_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett, uint8_t i_table_id,
                         uint16_t i_extension, uint8_t i_version, uint8_t i_protocol,
                         uint32_t i_etm_id, bool b_current_next)
{
    assert(p_ett);

    p_ett->i_table_id = i_table_id;
    p_ett->i_extension = i_extension;

    p_ett->i_version = i_version;
    p_ett->b_current_next = b_current_next;
    p_ett->i_protocol = i_protocol;
    p_ett->i_etm_id = i_etm_id;
    p_ett->i_etm_length = 0;
    p_ett->p_etm_data = NULL;
    p_ett->p_first_descriptor = NULL;
}

dvbpsi_atsc_ett_t *dvbpsi_atsc_NewETT(uint8_t i_table_id, uint16_t i_extension,
                                      uint8_t i_version, uint8_t i_protocol,
                                      uint32_t i_etm_id, bool b_current_next)
{
    dvbpsi_atsc_ett_t *p_ett;
    p_ett = (dvbpsi_atsc_ett_t*)malloc(sizeof(dvbpsi_atsc_ett_t));
    if (p_ett != NULL)
        dvbpsi_atsc_InitETT(p_ett, i_table_id, i_extension, i_version,
                            i_protocol, i_etm_id, b_current_next);
    return p_ett;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptyETT
 *****************************************************************************
 * Clean a dvbpsi_atsc_ett_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t *p_ett)
{
    assert(p_ett);

    dvbpsi_DeleteDescriptors(p_ett->p_first_descriptor);

    free(p_ett->p_etm_data);
    p_ett->i_etm_length = 0;
    p_ett->p_etm_data = NULL;
    p_ett->p_first_descriptor = NULL;
}

void dvbpsi_atsc_DeleteETT(dvbpsi_atsc_ett_t *p_ett)
{
    if (p_ett)
        dvbpsi_atsc_EmptyETT(p_ett);
    free(p_ett);
    p_ett = NULL;
}

/*****************************************************************************
 * dvbpsi_ReInitETT                                                          *
 *****************************************************************************/
static void dvbpsi_ReInitETT(dvbpsi_atsc_ett_decoder_t *p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_ett)
            dvbpsi_atsc_DeleteETT(p_decoder->p_building_ett);
    }
    p_decoder->p_building_ett = NULL;
}

static bool dvbpsi_CheckETT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_ett_decoder_t *p_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;

    assert(p_dvbpsi);
    assert(p_decoder);

    if (p_decoder->p_building_ett->i_protocol != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "'protocol version' differs"
                     " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->p_building_ett->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "'version_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder",
                     "'last_section_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionETT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_ett_decoder_t *p_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_decoder->p_building_ett)
    {
        uint32_t i_etm_id = ((uint32_t)p_section->p_payload_start[1] << 24) |
                ((uint32_t)p_section->p_payload_start[2] << 16) |
                ((uint32_t)p_section->p_payload_start[3] << 8)  |
                ((uint32_t)p_section->p_payload_start[4] << 0);

        p_decoder->p_building_ett = dvbpsi_atsc_NewETT(p_section->i_table_id,
                                                       p_section->i_extension,
                                                       p_section->i_version,
                                                       p_section->p_payload_start[0],
                                                       i_etm_id,
                                                       p_section->b_current_next);
        if (!p_decoder->p_building_ett)
            return false;

        p_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "ATSC ETT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherETTSections
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
static void dvbpsi_atsc_GatherETTSections(dvbpsi_t* p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0xCC, "ATSC ETT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* We have a valid ETT section */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_atsc_ett_decoder_t* p_ett_decoder = (dvbpsi_atsc_ett_decoder_t*)p_decoder;
    if (!p_ett_decoder)
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder", "No decoder specified");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitETT(p_ett_decoder, true);
        p_ett_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_ett_decoder->p_building_ett)
        {
            if (dvbpsi_CheckETT(p_dvbpsi, p_ett_decoder, p_section))
                dvbpsi_ReInitETT(p_ett_decoder, true);
        }
        else
        {
            if (   (p_ett_decoder->b_current_valid)
                && (p_ett_decoder->current_ett.i_version == p_section->i_version)
                && (p_ett_decoder->current_ett.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "ATSC ETT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
        }
    }

    /* Add section to ETT */
    if (!dvbpsi_AddSectionETT(p_dvbpsi, p_ett_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "ATSC ETT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_ett_decoder)))
    {
        assert(p_ett_decoder->pf_ett_callback);

        /* Save the current information */
        p_ett_decoder->current_ett = *p_ett_decoder->p_building_ett;
        p_ett_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_atsc_DecodeETTSections(p_ett_decoder->p_building_ett,
                                      p_ett_decoder->p_sections);
        /* signal the new ETT */
        p_ett_decoder->pf_ett_callback(p_ett_decoder->p_cb_data,
                                       p_ett_decoder->p_building_ett);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitETT(p_ett_decoder, false);
        assert(p_ett_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_atsc_DecodeETTSections
 *****************************************************************************
 * ETT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeETTSections(dvbpsi_atsc_ett_t* p_ett,
                                          dvbpsi_psi_section_t* p_section)
{
    while (p_section)
    {
        uint16_t i_etm_length = p_section->i_length - 14;

        /* NOTE: p_etm_data should be NULL if not then,
         * the PSI table is spread over multiple PSI sections */
        if (p_ett->p_etm_data)
            abort();
        p_ett->p_etm_data = calloc(i_etm_length, sizeof(uint8_t));
        if (!p_ett->p_etm_data)
            continue;
        /* FIXME: Decode the separate strings. For now copy the data in the
         * decoded table struct. */
        memcpy(p_ett->p_etm_data, p_section->p_payload_start + 5, i_etm_length);
        p_ett->i_etm_length = i_etm_length;

        p_section = p_section->p_next;
    }
}
