/*
Copyright (C) 2006-2012  Adam Charrett

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

eit.c

Decode PSIP Virtual Channel Table.

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

#include "atsc_eit.h"

typedef struct dvbpsi_atsc_eit_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_atsc_eit_callback      pf_eit_callback;
    void *                        p_cb_data;

    dvbpsi_atsc_eit_t             current_eit;
    dvbpsi_atsc_eit_t *           p_building_eit;

} dvbpsi_atsc_eit_decoder_t;


static dvbpsi_atsc_eit_event_t *dvbpsi_atsc_EITAddEvent(dvbpsi_atsc_eit_t* p_eit,
                                            uint16_t i_event_id,
                                            uint32_t i_start_time,
                                            uint8_t  i_etm_location,
                                            uint32_t i_length_seconds,
                                            uint8_t i_title_length,
                                            uint8_t *p_title);

static dvbpsi_descriptor_t *dvbpsi_atsc_EITChannelAddDescriptor(
                                               dvbpsi_atsc_eit_event_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static void dvbpsi_atsc_GatherEITSections(dvbpsi_t* p_dvbpsi,
                      dvbpsi_decoder_t* p_decoder, dvbpsi_psi_section_t* p_section);

static void dvbpsi_atsc_DecodeEITSections(dvbpsi_atsc_eit_t* p_eit,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachEIT
 *****************************************************************************
 * Initialize a EIT subtable decoder.
 *****************************************************************************/
bool dvbpsi_atsc_AttachEIT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_eit_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_atsc_eit_decoder_t* p_eit_decoder;
    p_eit_decoder = (dvbpsi_atsc_eit_decoder_t*) dvbpsi_decoder_new(NULL,
                                                    0, true, sizeof(dvbpsi_atsc_eit_decoder_t));
    if (p_eit_decoder == NULL)
        return false;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachEIT,
                                         dvbpsi_atsc_GatherEITSections, DVBPSI_DECODER(p_eit_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_eit_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* EIT decoder information */
    p_eit_decoder->pf_eit_callback = pf_callback;
    p_eit_decoder->p_cb_data = p_cb_data;
    p_eit_decoder->p_building_eit = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachEIT
 *****************************************************************************
 * Close a EIT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachEIT(dvbpsi_t * p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC EIT Decoder",
                            "No such EIT decoder (table_id == 0x%02x,"
                            "extension == 0x%04x)",
                            i_table_id, i_extension);
        return;
    }

    dvbpsi_atsc_eit_decoder_t* p_eit_decoder;
    p_eit_decoder = (dvbpsi_atsc_eit_decoder_t*)p_subdec->p_decoder;
    if (!p_eit_decoder)
        return;

    if (p_eit_decoder->p_building_eit)
        dvbpsi_atsc_DeleteEIT(p_eit_decoder->p_building_eit);
    p_eit_decoder->p_building_eit = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitEIT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_eit_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitEIT(dvbpsi_atsc_eit_t* p_eit, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_version, uint8_t i_protocol,
                         uint16_t i_source_id, bool b_current_next)
{
    assert(p_eit);

    p_eit->i_table_id = i_table_id;
    p_eit->i_extension = i_extension;

    p_eit->i_version = i_version;
    p_eit->b_current_next = b_current_next;
    p_eit->i_protocol = i_protocol;
    p_eit->i_source_id = i_source_id;
    p_eit->p_first_event = NULL;
    p_eit->p_first_descriptor = NULL;
}

dvbpsi_atsc_eit_t *dvbpsi_atsc_NewEIT(uint8_t i_table_id, uint16_t i_extension,
                                      uint8_t i_version, uint8_t i_protocol,
                                      uint16_t i_source_id, bool b_current_next)
{
    dvbpsi_atsc_eit_t *p_eit;
    p_eit = (dvbpsi_atsc_eit_t*) malloc(sizeof(dvbpsi_atsc_eit_t));
    if (p_eit != NULL)
        dvbpsi_atsc_InitEIT(p_eit, i_table_id, i_extension, i_version,
                            i_protocol, i_source_id, b_current_next);
    return p_eit;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptyEIT
 *****************************************************************************
 * Clean a dvbpsi_atsc_eit_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyEIT(dvbpsi_atsc_eit_t* p_eit)
{
  dvbpsi_atsc_eit_event_t* p_event = p_eit->p_first_event;

  while(p_event != NULL)
  {
    dvbpsi_atsc_eit_event_t* p_tmp = p_event->p_next;
    dvbpsi_DeleteDescriptors(p_event->p_first_descriptor);
    free(p_event);
    p_event = p_tmp;
  }
  p_eit->p_first_event = NULL;

  dvbpsi_DeleteDescriptors(p_eit->p_first_descriptor);
  p_eit->p_first_descriptor = NULL;
}

void dvbpsi_atsc_DeleteEIT(dvbpsi_atsc_eit_t *p_eit)
{
    if (p_eit)
        dvbpsi_atsc_EmptyEIT(p_eit);
    free(p_eit);
    p_eit = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_EITAddChannel
 *****************************************************************************
 * Add a Channel description at the end of the EIT.
 *****************************************************************************/
static dvbpsi_atsc_eit_event_t *dvbpsi_atsc_EITAddEvent(dvbpsi_atsc_eit_t* p_eit,
                                            uint16_t i_event_id,
                                            uint32_t i_start_time,
                                            uint8_t  i_etm_location,
                                            uint32_t i_length_seconds,
                                            uint8_t i_title_length,
                                            uint8_t *p_title)
{
  dvbpsi_atsc_eit_event_t * p_event
                = (dvbpsi_atsc_eit_event_t*)malloc(sizeof(dvbpsi_atsc_eit_event_t));
  if(p_event)
  {
    p_event->i_event_id = i_event_id;
    p_event->i_start_time = i_start_time;
    p_event->i_etm_location = i_etm_location;
    p_event->i_length_seconds = i_length_seconds;
    p_event->i_title_length = i_title_length;

    memcpy(p_event->i_title, p_title, i_title_length);

    p_event->p_first_descriptor = NULL;
    p_event->p_next = NULL;

    if(p_eit->p_first_event== NULL)
    {
      p_eit->p_first_event = p_event;
    }
    else
    {
      dvbpsi_atsc_eit_event_t * p_last_event = p_eit->p_first_event;
      while(p_last_event->p_next != NULL)
        p_last_event = p_last_event->p_next;
      p_last_event->p_next = p_event;
    }
  }

  return p_event;
}

/*****************************************************************************
 * dvbpsi_EITTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the EIT table description.
 *****************************************************************************/
static dvbpsi_descriptor_t *dvbpsi_atsc_EITChannelAddDescriptor(
                                               dvbpsi_atsc_eit_event_t *p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
                            = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_event->p_first_descriptor = dvbpsi_AddDescriptor(p_event->p_first_descriptor,
                                                       p_descriptor);
    assert(p_event->p_first_descriptor);
    if (p_event->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_ReInitEIT                                                          *
 *****************************************************************************/
static void dvbpsi_ReInitEIT(dvbpsi_atsc_eit_decoder_t *p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_eit)
            dvbpsi_atsc_DeleteEIT(p_decoder->p_building_eit);
    }
    p_decoder->p_building_eit = NULL;
}

static bool dvbpsi_CheckEIT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_eit_decoder_t *p_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;

    assert(p_dvbpsi);
    assert(p_decoder);

    if (p_decoder->p_building_eit->i_source_id != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder",
                     "'transport_stream_id' differs"
                     " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->p_building_eit->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder",
                     "'version_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder",
                     "'last_section_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionEIT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_eit_decoder_t *p_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_decoder->p_building_eit)
    {
        p_decoder->p_building_eit = dvbpsi_atsc_NewEIT(p_section->i_table_id,
                                                p_section->i_extension,
                                                p_section->i_version,
                                                p_section->p_payload_start[0],
                                                p_section->i_extension,
                                                p_section->b_current_next);
        if (!p_decoder->p_building_eit)
            return false;

        p_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "ATSC EIT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherEITSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
static void dvbpsi_atsc_GatherEITSections(dvbpsi_t * p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t * p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0xCB, "ATSC EIT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* We have a valid EIT section */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_atsc_eit_decoder_t *p_eit_decoder = (dvbpsi_atsc_eit_decoder_t*)p_decoder;
    if (!p_eit_decoder)
    {
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder", "No decoder specified");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitEIT(p_eit_decoder, true);
        p_eit_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_eit_decoder->p_building_eit)
        {
            if (dvbpsi_CheckEIT(p_dvbpsi, p_eit_decoder, p_section))
                dvbpsi_ReInitEIT(p_eit_decoder, true);
        }
        else
        {
            if (   (p_eit_decoder->b_current_valid)
                && (p_eit_decoder->current_eit.i_version == p_section->i_version)
                && (p_eit_decoder->current_eit.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "ATSC EIT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
#if 0
            if ((p_eit_decoder->b_current_valid) &&
                (p_eit_decoder->current_eit.i_version == p_section->i_version))
            {
                /* Signal a new EIT if the previous one wasn't active */
                if ((!p_eit_decoder->current_eit.b_current_next) &&
                     (p_section->b_current_next))
                {
                    dvbpsi_atsc_eit_t * p_eit = (dvbpsi_atsc_eit_t*)malloc(sizeof(dvbpsi_atsc_eit_t));
                    if (p_eit)
                    {
                        p_eit_decoder->current_eit.b_current_next = true;
                        *p_eit = p_eit_decoder->current_eit;
                        p_eit_decoder->pf_eit_callback(p_eit_decoder->p_cb_data, p_eit);
                    }
                    else
                        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder", "Could not signal new ATSC EIT.");
                }
            }
            dvbpsi_DeletePSISections(p_section);
            return;
#endif
        }
    }

    /* Add section to EIT */
    if (!dvbpsi_AddSectionEIT(p_dvbpsi, p_eit_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "ATSC EIT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_eit_decoder)))
    {
        assert(p_eit_decoder->pf_eit_callback);

        /* Save the current information */
        p_eit_decoder->current_eit = *p_eit_decoder->p_building_eit;
        p_eit_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_atsc_DecodeEITSections(p_eit_decoder->p_building_eit,
                                      p_eit_decoder->p_sections);
        /* signal the new EIT */
        p_eit_decoder->pf_eit_callback(p_eit_decoder->p_cb_data,
                                       p_eit_decoder->p_building_eit);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitEIT(p_eit_decoder, false);
        assert(p_eit_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeEITSection
 *****************************************************************************
 * EIT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeEITSections(dvbpsi_atsc_eit_t* p_eit,
                              dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    uint16_t i_number_events = p_section->p_payload_start[1];
    uint16_t i_events_count = 0;
    uint16_t i_length = 0;

    for(p_byte = p_section->p_payload_start + 2;
        ((p_byte + 4) < p_section->p_payload_end) && (i_events_count < i_number_events);
        i_events_count ++)
    {
        dvbpsi_atsc_eit_event_t* p_event;
        uint16_t i_event_id          = ((uint16_t)(p_byte[0] & 0x3f) << 8) | ((uint16_t) p_byte[1]);
        uint32_t i_start_time        = ((uint32_t)(p_byte[2] << 24)) |
                                       ((uint32_t)(p_byte[3] << 16)) |
                                       ((uint32_t)(p_byte[4] <<  8)) |
                                       ((uint32_t)(p_byte[5]));
        uint8_t  i_etm_location      = (uint8_t)((p_byte[6] & 0x30) >> 4);
        uint32_t i_length_seconds    = ((uint32_t)((p_byte[6] & 0x0f) << 16)) |
                                       ((uint32_t)(p_byte[7] << 8)) |
                                       ((uint32_t)(p_byte[8]));
        uint8_t  i_title_length      = p_byte[9];

        p_byte += 10;
        p_event = dvbpsi_atsc_EITAddEvent(p_eit, i_event_id, i_start_time,
                                i_etm_location, i_length_seconds, i_title_length,
                                p_byte);
        p_byte += i_title_length;
        i_length = ((uint16_t)(p_byte[0] & 0xf) <<8) | p_byte[1];
        /* Table descriptors */
        p_byte += 2;
        p_end = p_byte + i_length;
        if( p_end > p_section->p_payload_end ) break;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_len = p_byte[1];
            if(i_len + 2 <= p_end - p_byte)
              dvbpsi_atsc_EITChannelAddDescriptor(p_event, i_tag, i_len, p_byte + 2);
            p_byte += 2 + i_len;
        }
    }

    p_section = p_section->p_next;
  }
}
