/*****************************************************************************
 * rst.c: RST decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2012 VideoLAN
 * $Id:$
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

#include "rst.h"
#include "rst_private.h"

/*****************************************************************************
 * dvbpsi_rst_attach
 *****************************************************************************
 * Initialize a RST decoder and return a handle on it.
 *****************************************************************************/
bool dvbpsi_rst_attach(dvbpsi_t *p_dvbpsi, dvbpsi_rst_callback pf_callback,
                      void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder == NULL);

    dvbpsi_rst_decoder_t* p_rst_decoder;
    p_rst_decoder = (dvbpsi_rst_decoder_t*) dvbpsi_decoder_new(&dvbpsi_rst_sections_gather,
                                                1024, true, sizeof(dvbpsi_rst_decoder_t));
    if (p_rst_decoder == NULL)
        return false;

    /* RST decoder configuration */
    p_rst_decoder->pf_rst_callback = pf_callback;
    p_rst_decoder->p_cb_data = p_cb_data;
    p_rst_decoder->p_building_rst = NULL;

    p_dvbpsi->p_decoder = DVBPSI_DECODER(p_rst_decoder);
    return true;
}

/*****************************************************************************
 * dvbpsi_rst_detach
 *****************************************************************************
 * Close a RST decoder. The handle isn't valid any more.
 *****************************************************************************/
void dvbpsi_rst_detach(dvbpsi_t *p_dvbpsi)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_rst_decoder_t* p_rst_decoder
                        = (dvbpsi_rst_decoder_t*)p_dvbpsi->p_decoder;
    if (p_rst_decoder->p_building_rst)
        dvbpsi_rst_delete(p_rst_decoder->p_building_rst);
    p_rst_decoder->p_building_rst = NULL;

    dvbpsi_decoder_delete(p_dvbpsi->p_decoder);
    p_dvbpsi->p_decoder = NULL;
}

/*****************************************************************************
 * dvbpsi_rst_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_rst_t structure.
 *****************************************************************************/
void dvbpsi_rst_init(dvbpsi_rst_t* p_rst)
{
    assert(p_rst);

    p_rst->p_first_event = NULL;
}

/*****************************************************************************
 * dvbpsi_rst_new
 *****************************************************************************
 * Allocate and Initialize a dvbpsi_rst_t structure.
 *****************************************************************************/
dvbpsi_rst_t *dvbpsi_rst_new(void)
{
    dvbpsi_rst_t *p_rst = (dvbpsi_rst_t*)malloc(sizeof(dvbpsi_rst_t));
    if (p_rst != NULL)
        dvbpsi_rst_init(p_rst);
    return p_rst;
}

/*****************************************************************************
 * dvbpsi_rst_empty
 *****************************************************************************
 * Clean a dvbpsi_rst_t structure.
 *****************************************************************************/
void dvbpsi_rst_empty(dvbpsi_rst_t* p_rst)
{
	dvbpsi_rst_event_t * p_rst_event = p_rst->p_first_event;

    while(p_rst_event != NULL)
    {
    	dvbpsi_rst_event_t* p_next = p_rst_event->p_next;

        free(p_rst_event);

        p_rst_event = p_next;
    }

    p_rst->p_first_event = NULL;
}

/*****************************************************************************
 * dvbpsi_rst_delete
 *****************************************************************************
 * Clean a dvbpsi_rst_t structure.
 *****************************************************************************/
void dvbpsi_rst_delete(dvbpsi_rst_t *p_rst)
{
    if (p_rst)
        dvbpsi_rst_empty(p_rst);
    free(p_rst);
}

/*****************************************************************************
 * dvbpsi_rst_event_add
 *****************************************************************************
 * Add an event in the RST.
 *****************************************************************************/

dvbpsi_rst_event_t* dvbpsi_rst_event_add(dvbpsi_rst_t* p_rst,
                                            uint16_t i_ts_id,
                                            uint16_t i_orig_network_id,
                                            uint16_t i_service_id,
                                            uint16_t i_event_id,
                                            uint8_t i_running_status)
{
	dvbpsi_rst_event_t* p_rst_event
                        = (dvbpsi_rst_event_t*)malloc(sizeof(dvbpsi_rst_event_t));

    if (p_rst_event == NULL)
        return NULL;

    p_rst_event->i_ts_id = i_ts_id;
    p_rst_event->i_orig_network_id = i_orig_network_id;
    p_rst_event->i_service_id = i_service_id;
    p_rst_event->i_event_id = i_event_id;
    p_rst_event->i_running_status = i_running_status;
    p_rst_event->p_next = NULL;

    if (p_rst->p_first_event == NULL)
    	p_rst->p_first_event = p_rst_event;
    else
    {
    	dvbpsi_rst_event_t *p_last = p_rst->p_first_event;
    	while (p_last->p_next != NULL)
    		p_last = p_last->p_next;
    	p_last->p_next = p_rst_event;
    }

    return p_rst_event;
}

/*****************************************************************************
 * dvbpsi_rst_sections_generate
 *****************************************************************************
 * Generate RST sections based on the dvbpsi_rst_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t* dvbpsi_rst_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_rst_t* p_rst)
{
    dvbpsi_psi_section_t* p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t* p_current = p_result;
    dvbpsi_psi_section_t* p_prev;
    dvbpsi_rst_event_t* p_event = p_rst->p_first_event;
    int i_count = 0;

    if (p_current == NULL)
    {
        dvbpsi_error(p_dvbpsi, "RST encoder", "failed to allocate new PSI section");
        return NULL;
    }

    p_current->i_table_id = 0x71;
    p_current->b_syntax_indicator = false;
    p_current->b_private_indicator = false;
    p_current->i_length = 3;                      /* header */
    p_current->i_extension = 0;                   /* Not used in the RST */
    p_current->i_version = 0;                     /* Not used in the RST */
    p_current->b_current_next = true;             /* Not used in the RST */
    p_current->i_number = 0;                      /* Not used in the RST */
    p_current->p_payload_end += 3;                /* just after the header */
    p_current->p_payload_start = p_current->p_payload_end;

    /* RST events */
    while (p_event != NULL)
    {
        /* The section_length shall not exceed 1 021 so that the entire section has a maximum length of 1 024 bytes */
        if((p_current->p_payload_end - p_current->p_data) < 1021)
        {
        	p_current->p_data[0+i_count] = (p_event->i_ts_id >> 8);
        	p_current->p_data[1+i_count] = (p_event->i_ts_id & 0xFF);
        	p_current->p_data[2+i_count] = (p_event->i_orig_network_id >> 8);
        	p_current->p_data[3+i_count] = (p_event->i_orig_network_id & 0xFF);
        	p_current->p_data[4+i_count] = (p_event->i_service_id >> 8);
        	p_current->p_data[5+i_count] = (p_event->i_service_id & 0xFF);
        	p_current->p_data[6+i_count] = (p_event->i_event_id >> 8);
        	p_current->p_data[7+i_count] = (p_event->i_event_id & 0xFF);
        	p_current->p_data[8+i_count] = 0xF8 + p_event->i_running_status;
        	/* Increase length by event_length */
        	p_current->i_length += 9;
        	p_current->p_payload_end += 9;
        }
        p_event = p_event->p_next;
        i_count++;
    }

    /* Finalization */
    p_prev = p_result;
    while (p_prev != NULL)
    {
        p_prev->i_last_number = p_current->i_number;
        dvbpsi_BuildPSISection(p_dvbpsi, p_prev);
        p_prev = p_prev->p_next;
    }
    return p_result;
}

/* */
static void dvbpsi_rst_reset(dvbpsi_rst_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_rst)
            dvbpsi_rst_delete(p_decoder->p_building_rst);
    }
    p_decoder->p_building_rst = NULL;
}

static bool dvbpsi_rst_section_add(dvbpsi_t *p_dvbpsi, dvbpsi_rst_decoder_t *p_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (p_decoder->p_building_rst == NULL)
    {
        p_decoder->p_building_rst = dvbpsi_rst_new();
        if (p_decoder->p_building_rst == NULL)
            return false;

        p_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "RST decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_rst_section_check
 *****************************************************************************
 * Check if RST section has the expected table_id and it the syntax indicator
 * is set to 0 according to ETSI EN 300 468 §5.2.7.
 *****************************************************************************/
static bool dvbpsi_rst_section_check(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section,
                            const uint8_t table_id, const char *psz_table_name)
{
    assert(p_dvbpsi);
    assert(p_section);

    if (p_section->i_table_id != table_id)
    {
        /* Invalid table_id value */
        dvbpsi_error(p_dvbpsi, psz_table_name,
                     "invalid section (table_id == 0x%02x expected 0x%02)",
                     p_section->i_table_id, table_id);
        goto error;
    }

    if (p_section->b_syntax_indicator)
    {
        /* Invalid section_syntax_indicator */
        dvbpsi_error(p_dvbpsi, psz_table_name,
                     "invalid section (section_syntax_indicator != 0)");
        goto error;
    }

    dvbpsi_debug(p_dvbpsi, psz_table_name,
                   "Table version %2d, " "i_extension %5d, "
                   "section %3d up to %3d, " "current %1d",
                   p_section->i_version, p_section->i_extension,
                   p_section->i_number, p_section->i_last_number,
                   p_section->b_current_next);
    return true;

error:
    return false;
}

/*****************************************************************************
 * dvbpsi_rst_sections_gather
 *****************************************************************************
 * Callback for the PSI decoder.
 *****************************************************************************/
void dvbpsi_rst_sections_gather(dvbpsi_t *p_dvbpsi,
                              dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_rst_section_check(p_dvbpsi, p_section, 0x71, "RST decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_rst_decoder_t* p_rst_decoder
                          = (dvbpsi_rst_decoder_t*)p_dvbpsi->p_decoder;

    /* TS discontinuity check */
    if (p_rst_decoder->b_discontinuity)
    {
    	dvbpsi_rst_reset(p_rst_decoder, true);
        p_rst_decoder->b_discontinuity = false;
    }

    /* Add section to RST */
    if (!dvbpsi_rst_section_add(p_dvbpsi, p_rst_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "RST decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_rst_decoder)))
    {
        assert(p_rst_decoder->pf_rst_callback);

        /* Save the current information */
        p_rst_decoder->current_rst = *p_rst_decoder->p_building_rst;
        p_rst_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_rst_sections_decode(p_rst_decoder->p_building_rst,
                                   p_rst_decoder->p_sections);
        /* signal the new CAT */
        p_rst_decoder->pf_rst_callback(p_rst_decoder->p_cb_data,
                                       p_rst_decoder->p_building_rst);
        /* Delete sectioins and Reinitialize the structures */
        dvbpsi_rst_reset(p_rst_decoder, false);
        assert(p_rst_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_rst_sections_decode
 *****************************************************************************
 * RST decoder.
 *****************************************************************************/
void dvbpsi_rst_sections_decode(dvbpsi_rst_t* p_rst,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t* p_byte;

    while (p_section)
    {
        /* RST events */
        p_byte = p_section->p_payload_start;
        while (p_byte + 9 <= p_section->p_payload_end)
        {
            uint16_t i_transport_stream_id = (p_byte[0] << 8) + p_byte[1];
            uint16_t i_original_network_id = (p_byte[2] << 8) + p_byte[3];
            uint16_t i_service_id = (p_byte[4] << 8) + p_byte[5];
            uint16_t i_event_id = (p_byte[6] << 8) + p_byte[7];
            uint8_t i_running_status = (p_byte[8] & 0x07);

            dvbpsi_rst_event_add(p_rst, i_transport_stream_id, i_original_network_id, i_service_id, i_event_id, i_running_status);
            p_byte += 9;
        }
        p_section = p_section->p_next;
    }
}
