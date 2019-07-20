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

vct.c

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
#include "atsc_vct.h"

typedef struct dvbpsi_atsc_vct_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_atsc_vct_callback      pf_vct_callback;
    void *                        p_cb_data;

    dvbpsi_atsc_vct_t             current_vct;
    dvbpsi_atsc_vct_t *           p_building_vct;

} dvbpsi_atsc_vct_decoder_t;

static dvbpsi_descriptor_t *dvbpsi_atsc_VCTAddDescriptor(
                                               dvbpsi_atsc_vct_t *p_vct,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static dvbpsi_atsc_vct_channel_t *dvbpsi_atsc_VCTAddChannel(dvbpsi_atsc_vct_t* p_vct,
                                            uint8_t *p_short_name,
                                            uint16_t i_major_number,
                                            uint16_t i_minor_number,
                                            uint8_t  i_modulation,
                                            uint32_t i_carrier_freq,
                                            uint16_t i_channel_tsid,
                                            uint16_t i_program_number,
                                            uint8_t  i_etm_location,
                                            int      b_access_controlled,
                                            int      b_hidden,
                                            int      b_path_select,
                                            int      b_out_of_band,
                                            int      b_hide_guide,
                                            uint8_t  i_service_type,
                                            uint16_t i_source_id);

static dvbpsi_descriptor_t *dvbpsi_atsc_VCTChannelAddDescriptor(
                                               dvbpsi_atsc_vct_channel_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static void dvbpsi_atsc_GatherVCTSections(dvbpsi_t * p_dvbpsi,
                dvbpsi_decoder_t *p_decoder, dvbpsi_psi_section_t * p_section);

static void dvbpsi_atsc_DecodeVCTSections(dvbpsi_atsc_vct_t* p_vct,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachVCT
 *****************************************************************************
 * Initialize a VCT subtable decoder.
 *****************************************************************************/
bool dvbpsi_atsc_AttachVCT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                          dvbpsi_atsc_vct_callback pf_vct_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "ATSC VCT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_atsc_vct_decoder_t*  p_vct_decoder;
    p_vct_decoder = (dvbpsi_atsc_vct_decoder_t*) dvbpsi_decoder_new(NULL,
                                                  0, true, sizeof(dvbpsi_atsc_vct_decoder_t));
    if (p_vct_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachVCT,
                                         dvbpsi_atsc_GatherVCTSections, DVBPSI_DECODER(p_vct_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_vct_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* VCT decoder information */
    p_vct_decoder->pf_vct_callback = pf_vct_callback;
    p_vct_decoder->p_cb_data = p_cb_data;
    p_vct_decoder->p_building_vct = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachVCT
 *****************************************************************************
 * Close a VCT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachVCT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if(p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC VCT Decoder",
                         "No such VCT decoder (table_id == 0x%02x,"
                         "extension == 0x%04x)",
                         i_table_id, i_extension);
        return;
    }

    dvbpsi_atsc_vct_decoder_t* p_vct_decoder;
    p_vct_decoder = (dvbpsi_atsc_vct_decoder_t*)p_subdec->p_decoder;
    if (!p_vct_decoder)
        return;
    if (p_vct_decoder->p_building_vct)
        dvbpsi_atsc_DeleteVCT(p_vct_decoder->p_building_vct);
    p_vct_decoder->p_building_vct = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_NewVCT
 *****************************************************************************
 * Allocate a new dvbpsi_atsc_vct_t structure and initialize it.
 *****************************************************************************/
dvbpsi_atsc_vct_t *dvbpsi_atsc_NewVCT(uint8_t i_table_id, uint16_t i_extension,
        uint8_t i_protocol, bool b_cable_vct, uint8_t i_version, bool b_current_next)
{
    dvbpsi_atsc_vct_t *p_vct = (dvbpsi_atsc_vct_t*)malloc(sizeof(dvbpsi_atsc_vct_t));
    if (p_vct != NULL)
        dvbpsi_atsc_InitVCT(p_vct, i_table_id, i_extension,  i_protocol,
                            b_cable_vct, i_version, b_current_next);
    return p_vct;
}

/*****************************************************************************
 * dvbpsi_atsc_InitVCT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_vct_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitVCT(dvbpsi_atsc_vct_t* p_vct, uint8_t i_table_id,
                         uint16_t i_extension, uint8_t i_protocol, bool b_cable_vct,
                         uint8_t i_version, bool b_current_next)
{
    assert(p_vct);
    p_vct->i_table_id = i_table_id;
    p_vct->i_extension = i_extension;

    p_vct->i_version = i_version;
    p_vct->b_current_next = b_current_next;
    p_vct->i_protocol = i_protocol;
    p_vct->b_cable_vct = b_cable_vct;
    p_vct->p_first_channel = NULL;
    p_vct->p_first_descriptor = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptyVCT
 *****************************************************************************
 * Clean a dvbpsi_atsc_vct_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyVCT(dvbpsi_atsc_vct_t* p_vct)
{
    dvbpsi_atsc_vct_channel_t* p_channel = p_vct->p_first_channel;
    dvbpsi_DeleteDescriptors(p_vct->p_first_descriptor);
    p_vct->p_first_descriptor = NULL;

    while(p_channel != NULL)
    {
        dvbpsi_atsc_vct_channel_t* p_tmp = p_channel->p_next;
        dvbpsi_DeleteDescriptors(p_channel->p_first_descriptor);
        free(p_channel);
        p_channel = p_tmp;
    }
    p_vct->p_first_channel = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_DeleteVCT
 *****************************************************************************
 * Empty and Delere a dvbpsi_atsc_vct_t structure.
 *****************************************************************************/
void dvbpsi_atsc_DeleteVCT(dvbpsi_atsc_vct_t *p_vct)
{
    if (p_vct)
        dvbpsi_atsc_EmptyVCT(p_vct);
    free(p_vct);
}

/*****************************************************************************
 * dvbpsi_atsc_VCTAddDescriptor
 *****************************************************************************
 * Add a descriptor to the VCT table.
 *****************************************************************************/
static dvbpsi_descriptor_t *dvbpsi_atsc_VCTAddDescriptor(dvbpsi_atsc_vct_t *p_vct,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
            = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_vct->p_first_descriptor = dvbpsi_AddDescriptor(p_vct->p_first_descriptor,
                                                     p_descriptor);
    assert(p_vct->p_first_descriptor);
    if (p_vct->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_atsc_VCTAddChannel
 *****************************************************************************
 * Add a Channel description at the end of the VCT.
 *****************************************************************************/
static dvbpsi_atsc_vct_channel_t *dvbpsi_atsc_VCTAddChannel(dvbpsi_atsc_vct_t* p_vct,
                                            uint8_t *p_short_name,
                                            uint16_t i_major_number,
                                            uint16_t i_minor_number,
                                            uint8_t  i_modulation,
                                            uint32_t i_carrier_freq,
                                            uint16_t i_channel_tsid,
                                            uint16_t i_program_number,
                                            uint8_t  i_etm_location,
                                            int      b_access_controlled,
                                            int      b_hidden,
                                            int      b_path_select,
                                            int      b_out_of_band,
                                            int      b_hide_guide,
                                            uint8_t  i_service_type,
                                            uint16_t i_source_id)
{
    dvbpsi_atsc_vct_channel_t * p_channel
            = (dvbpsi_atsc_vct_channel_t*)malloc(sizeof(dvbpsi_atsc_vct_channel_t));
    if(p_channel)
    {
        memcpy(p_channel->i_short_name, p_short_name, sizeof(uint16_t) * 7);
        p_channel->i_major_number = i_major_number;
        p_channel->i_minor_number = i_minor_number;
        p_channel->i_modulation = i_modulation;
        p_channel->i_carrier_freq = i_carrier_freq;
        p_channel->i_channel_tsid = i_channel_tsid;
        p_channel->i_program_number = i_program_number;
        p_channel->i_etm_location = i_etm_location;
        p_channel->b_access_controlled = b_access_controlled;
        p_channel->b_path_select = b_path_select;
        p_channel->b_out_of_band = b_out_of_band;
        p_channel->b_hidden = b_hidden;
        p_channel->b_hide_guide = b_hide_guide;
        p_channel->i_service_type = i_service_type;
        p_channel->i_source_id = i_source_id;

        p_channel->p_first_descriptor = NULL;
        p_channel->p_next = NULL;

        if(p_vct->p_first_channel== NULL)
        {
            p_vct->p_first_channel = p_channel;
        }
        else
        {
            dvbpsi_atsc_vct_channel_t * p_last_channel = p_vct->p_first_channel;
            while(p_last_channel->p_next != NULL)
                p_last_channel = p_last_channel->p_next;
            p_last_channel->p_next = p_channel;
        }
    }

    return p_channel;
}

/*****************************************************************************
 * dvbpsi_VCTTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the VCT table description.
 *****************************************************************************/
static dvbpsi_descriptor_t *dvbpsi_atsc_VCTChannelAddDescriptor(
                                               dvbpsi_atsc_vct_channel_t *p_channel,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
            = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if(p_descriptor)
    {
        if(p_channel->p_first_descriptor == NULL)
        {
            p_channel->p_first_descriptor = p_descriptor;
        }
        else
        {
            dvbpsi_descriptor_t * p_last_descriptor = p_channel->p_first_descriptor;
            while(p_last_descriptor->p_next != NULL)
                p_last_descriptor = p_last_descriptor->p_next;
            p_last_descriptor->p_next = p_descriptor;
        }
    }

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_ReInitVCT                                                          *
 *****************************************************************************/
static void dvbpsi_ReInitVCT(dvbpsi_atsc_vct_decoder_t *p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_vct)
            dvbpsi_atsc_DeleteVCT(p_decoder->p_building_vct);
    }
    p_decoder->p_building_vct = NULL;
}

static bool dvbpsi_CheckVCT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_vct_decoder_t *p_vct_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;

    assert(p_dvbpsi);
    assert(p_vct_decoder);

    if (p_vct_decoder->p_building_vct->i_extension != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "ATSC VCT decoder",
                     "'transport_stream_id' differs"
                     " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_vct_decoder->p_building_vct->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "ATSC VCT decoder",
                     "'version_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_vct_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "ATSC VCT decoder",
                     "'last_section_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionVCT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_vct_decoder_t *p_vct_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_vct_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_vct_decoder->p_building_vct)
    {
        p_vct_decoder->p_building_vct = dvbpsi_atsc_NewVCT(
                              p_section->i_table_id, p_section->i_extension,
                              p_section->p_payload_start[0], p_section->i_table_id == 0xC9,
                              p_section->i_version, p_section->b_current_next);
        if (!p_vct_decoder->p_building_vct)
            return false;

        p_vct_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_vct_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "ATSC VCT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherVCTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
static void dvbpsi_atsc_GatherVCTSections(dvbpsi_t *p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t *p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    const uint8_t i_table_id = (p_section->i_table_id == 0xC8 ||
                                p_section->i_table_id == 0xC9) ?
                                p_section->i_table_id : 0xC8;

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, i_table_id, "ATSC VCT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_atsc_vct_decoder_t *p_vct_decoder = (dvbpsi_atsc_vct_decoder_t*)p_decoder;

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitVCT(p_vct_decoder, true);
        p_vct_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_vct_decoder->p_building_vct)
        {
            if (dvbpsi_CheckVCT(p_dvbpsi, p_vct_decoder, p_section))
                dvbpsi_ReInitVCT(p_vct_decoder, true);
        }
        else
        {
            if (   (p_vct_decoder->b_current_valid)
                && (p_vct_decoder->current_vct.i_version == p_section->i_version)
                && (p_vct_decoder->current_vct.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "ATSC VCT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
#if 0 /* FIXME: when to signal new table? */
            if ((p_vct_decoder->b_current_valid)
                && (p_vct_decoder->current_vct.i_version == p_section->i_version))
            {
                /* Signal a new VCT if the previous one wasn't active */
                if ((!p_vct_decoder->current_vct.b_current_next)
                    && (p_section->b_current_next))
                {
                    dvbpsi_atsc_vct_t * p_vct = (dvbpsi_atsc_vct_t*)malloc(sizeof(dvbpsi_atsc_vct_t));
                    if (p_vct)
                    {
                        p_vct_decoder->current_vct.b_current_next = 1;
                        *p_vct = p_vct_decoder->current_vct;
                        p_vct_decoder->pf_vct_callback(p_vct_decoder->p_cb_data, p_vct);
                    }
                }
                else
                    dvbpsi_error(p_dvbpsi, "ATSC VCT decoder", "Could not signal new ATSC VCT.");
            }
            dvbpsi_DeletePSISections(p_section);
            return;
#endif
        }
    }

    /* Add section to VCT */
    if (!dvbpsi_AddSectionVCT(p_dvbpsi, p_vct_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "ATSC_VCT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_vct_decoder)))
    {
        assert(p_vct_decoder->pf_vct_callback);

        /* Save the current information */
        p_vct_decoder->current_vct = *p_vct_decoder->p_building_vct;
        p_vct_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_atsc_DecodeVCTSections(p_vct_decoder->p_building_vct,
                                      p_vct_decoder->p_sections);
        /* signal the new VCT */
        p_vct_decoder->pf_vct_callback(p_vct_decoder->p_cb_data,
                                       p_vct_decoder->p_building_vct);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitVCT(p_vct_decoder, false);
        assert(p_vct_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeVCTSection
 *****************************************************************************
 * VCT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeVCTSections(dvbpsi_atsc_vct_t* p_vct,
                              dvbpsi_psi_section_t* p_section)
{
    uint8_t *p_byte, *p_end;

    while(p_section)
    {
        uint16_t i_channels_defined = p_section->p_payload_start[1];
        uint16_t i_channels_count = 0;
        uint16_t i_length = 0;

        for(p_byte = p_section->p_payload_start + 2;
            ((p_byte + 6) < p_section->p_payload_end) && (i_channels_count < i_channels_defined);
            i_channels_count ++)
        {
            dvbpsi_atsc_vct_channel_t* p_channel;
            uint16_t i_major_number      = ((uint16_t)(p_byte[14] & 0xf) << 6) | ((uint16_t)(p_byte[15] & 0xfc) >> 2);
            uint16_t i_minor_number      = ((uint16_t)(p_byte[15] & 0x3) << 8) | ((uint16_t) p_byte[16]);
            uint8_t  i_modulation        = p_byte[17];
            uint32_t i_carrier_freq      = ((uint32_t)(p_byte[18] << 24)) |
                    ((uint32_t)(p_byte[19] << 16)) |
                    ((uint32_t)(p_byte[20] <<  8)) |
                    ((uint32_t)(p_byte[21]));
            uint16_t i_channel_tsid      = ((uint16_t)(p_byte[22] << 8)) |  ((uint16_t)p_byte[23]);
            uint16_t i_program_number    = ((uint16_t)(p_byte[24] << 8)) |  ((uint16_t)p_byte[25]);
            uint8_t  i_etm_location      = (uint8_t)((p_byte[26] & 0xC0) >> 6);
            int      b_access_controlled = (p_byte[26] & 0x20) >> 5;
            int      b_hidden            = (p_byte[26] & 0x10) >> 4;
            int      b_path_select       = (p_byte[26] & 0x08) >> 3;
            int      b_out_of_band       = (p_byte[26] & 0x04) >> 2;
            int      b_hide_guide        = (p_byte[26] & 0x02) >> 1;
            uint8_t  i_service_type      = (uint8_t)(p_byte[27] & 0x3f);
            uint16_t i_source_id         = ((uint16_t)(p_byte[28] << 8)) |  ((uint16_t)p_byte[29]);
            i_length = ((uint16_t)(p_byte[30] & 0x3) <<8) | p_byte[31];

            p_channel = dvbpsi_atsc_VCTAddChannel(p_vct, p_byte,
                                                  i_major_number, i_minor_number,
                                                  i_modulation, i_carrier_freq,
                                                  i_channel_tsid, i_program_number,
                                                  i_etm_location, b_access_controlled,
                                                  b_hidden, b_path_select, b_out_of_band,
                                                  b_hide_guide, i_service_type, i_source_id);

            /* Table descriptors */
            p_byte += 32;
            p_end = p_byte + i_length;
            if( p_end > p_section->p_payload_end ) break;

            while(p_byte + 2 <= p_end)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_len = p_byte[1];
                if(i_len + 2 <= p_end - p_byte)
                    dvbpsi_atsc_VCTChannelAddDescriptor(p_channel, i_tag, i_len, p_byte + 2);
                p_byte += 2 + i_len;
            }
        }

        /* Table descriptors */
        i_length = ((uint16_t)(p_byte[0] & 0x3) <<8) | p_byte[1];
        p_byte += 2;
        p_end = p_byte + i_length;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_len = p_byte[1];
            if(i_len + 2 <= p_end - p_byte)
                dvbpsi_atsc_VCTAddDescriptor(p_vct, i_tag, i_len, p_byte + 2);
            p_byte += 2 + i_len;
        }
        p_section = p_section->p_next;
    }
}
