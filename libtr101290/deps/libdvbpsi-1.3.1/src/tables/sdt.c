/*****************************************************************************
 * sdt.c: SDT decoder/generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>\
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
#include "sdt.h"
#include "sdt_private.h"

/*****************************************************************************
 * dvbpsi_sdt_attach
 *****************************************************************************
 * Initialize a SDT subtable decoder.
 *****************************************************************************/
bool dvbpsi_sdt_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                      dvbpsi_sdt_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t* p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "SDT decoder",
                     "Already a decoder for (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_sdt_decoder_t*  p_sdt_decoder;
    p_sdt_decoder = (dvbpsi_sdt_decoder_t*) dvbpsi_decoder_new(NULL,
                                             0, true, sizeof(dvbpsi_sdt_decoder_t));
    if (p_sdt_decoder == NULL)
        return false;

    /* subtable decoder configuration */
    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_sdt_detach,
                                         dvbpsi_sdt_sections_gather, DVBPSI_DECODER(p_sdt_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_sdt_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* SDT decoder information */
    p_sdt_decoder->pf_sdt_callback = pf_callback;
    p_sdt_decoder->p_cb_data = p_cb_data;
    p_sdt_decoder->p_building_sdt = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_sdt_detach
 *****************************************************************************
 * Close a SDT decoder.
 *****************************************************************************/
void dvbpsi_sdt_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "SDT Decoder",
                     "No such SDT decoder (table_id == 0x%02x,"
                     "extension == 0x%02x)",
                     i_table_id, i_extension);
        return;
    }

    assert(p_subdec->p_decoder);

    dvbpsi_sdt_decoder_t* p_sdt_decoder;
    p_sdt_decoder = (dvbpsi_sdt_decoder_t*)p_subdec->p_decoder;
    if (p_sdt_decoder->p_building_sdt)
        dvbpsi_sdt_delete(p_sdt_decoder->p_building_sdt);
    p_sdt_decoder->p_building_sdt = NULL;

    /* Free sub table decoder */
    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_sdt_init
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_sdt_t structure.
 *****************************************************************************/
void dvbpsi_sdt_init(dvbpsi_sdt_t* p_sdt, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next, uint16_t i_network_id)
{
    assert(p_sdt);

    p_sdt->i_table_id = i_table_id;
    p_sdt->i_extension = i_extension;

    p_sdt->i_version = i_version;
    p_sdt->b_current_next = b_current_next;
    p_sdt->i_network_id = i_network_id;
    p_sdt->p_first_service = NULL;
}

/*****************************************************************************
 * dvbpsi_sdt_new
 *****************************************************************************
 * Allocate and Initialize a new dvbpsi_sdt_t structure.
 *****************************************************************************/
dvbpsi_sdt_t *dvbpsi_sdt_new(uint8_t i_table_id, uint16_t i_extension, uint8_t i_version,
                             bool b_current_next, uint16_t i_network_id)
{
    dvbpsi_sdt_t *p_sdt = (dvbpsi_sdt_t*)malloc(sizeof(dvbpsi_sdt_t));
    if (p_sdt != NULL)
        dvbpsi_sdt_init(p_sdt, i_table_id, i_extension, i_version,
                        b_current_next, i_network_id);
    return p_sdt;
}

/*****************************************************************************
 * dvbpsi_sdt_empty
 *****************************************************************************
 * Clean a dvbpsi_sdt_t structure.
 *****************************************************************************/
void dvbpsi_sdt_empty(dvbpsi_sdt_t* p_sdt)
{
    dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;

    while (p_service != NULL)
    {
        dvbpsi_sdt_service_t* p_tmp = p_service->p_next;
        dvbpsi_DeleteDescriptors(p_service->p_first_descriptor);
        free(p_service);
        p_service = p_tmp;
    }
    p_sdt->p_first_service = NULL;
}

/*****************************************************************************
 * dvbpsi_sdt_delete
 *****************************************************************************
 * Clean and Delete dvbpsi_sdt_t structure.
 *****************************************************************************/
void dvbpsi_sdt_delete(dvbpsi_sdt_t *p_sdt)
{
    if (p_sdt)
        dvbpsi_sdt_empty(p_sdt);
    free(p_sdt);
}

/*****************************************************************************
 * dvbpsi_sdt_service_add
 *****************************************************************************
 * Add a service description at the end of the SDT.
 *****************************************************************************/
dvbpsi_sdt_service_t *dvbpsi_sdt_service_add(dvbpsi_sdt_t* p_sdt,
                                           uint16_t i_service_id,
                                           bool b_eit_schedule,
                                           bool b_eit_present,
                                           uint8_t i_running_status,
                                           bool b_free_ca)
{
    dvbpsi_sdt_service_t * p_service;
    p_service = (dvbpsi_sdt_service_t*)calloc(1, sizeof(dvbpsi_sdt_service_t));
    if (p_service == NULL)
        return NULL;

    p_service->i_service_id = i_service_id;
    p_service->b_eit_schedule = b_eit_schedule;
    p_service->b_eit_present = b_eit_present;
    p_service->i_running_status = i_running_status;
    p_service->b_free_ca = b_free_ca;
    p_service->p_next = NULL;
    p_service->p_first_descriptor = NULL;

    if (p_sdt->p_first_service == NULL)
        p_sdt->p_first_service = p_service;
    else
    {
        dvbpsi_sdt_service_t * p_last_service = p_sdt->p_first_service;
        while(p_last_service->p_next != NULL)
            p_last_service = p_last_service->p_next;
        p_last_service->p_next = p_service;
    }

    return p_service;
}

/*****************************************************************************
 * dvbpsi_sdt_service_descriptor_add
 *****************************************************************************
 * Add a descriptor in the SDT service description.
 *****************************************************************************/
dvbpsi_descriptor_t *dvbpsi_sdt_service_descriptor_add(
                                               dvbpsi_sdt_service_t *p_service,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor;
    p_descriptor = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_service->p_first_descriptor = dvbpsi_AddDescriptor(p_service->p_first_descriptor,
                                                         p_descriptor);
    assert(p_service->p_first_descriptor);
    if (p_service->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/* */
static void dvbpsi_ReInitSDT(dvbpsi_sdt_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_sdt)
            dvbpsi_sdt_delete(p_decoder->p_building_sdt);
    }
    p_decoder->p_building_sdt = NULL;
}

static bool dvbpsi_CheckSDT(dvbpsi_t *p_dvbpsi, dvbpsi_sdt_decoder_t *p_sdt_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;
    assert(p_dvbpsi);
    assert(p_sdt_decoder);

    if (p_sdt_decoder->p_building_sdt->i_extension != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "SDT decoder",
                "'transport_stream_id' differs"
                " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_sdt_decoder->p_building_sdt->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "SDT decoder",
                "'version_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_sdt_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "SDT decoder",
                "'last_section_number' differs"
                " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionSDT(dvbpsi_t *p_dvbpsi, dvbpsi_sdt_decoder_t *p_sdt_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_sdt_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_sdt_decoder->p_building_sdt)
    {
        p_sdt_decoder->p_building_sdt =
                dvbpsi_sdt_new(p_section->i_table_id, p_section->i_extension,
                             p_section->i_version, p_section->b_current_next,
                             ((uint16_t)(p_section->p_payload_start[0]) << 8)
                                         | p_section->p_payload_start[1]);

        if (p_sdt_decoder->p_building_sdt == NULL)
            return false;
        p_sdt_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_sdt_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "SDT decoder", "overwrite section number %d",
                     p_section->i_number);

    return true;
}

/*****************************************************************************
 * dvbpsi_sdt_sections_gather
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
void dvbpsi_sdt_sections_gather(dvbpsi_t *p_dvbpsi,
                                dvbpsi_decoder_t *p_private_decoder,
                                dvbpsi_psi_section_t * p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    const uint8_t i_table_id = (p_section->i_table_id == 0x42 ||
                                p_section->i_table_id == 0x46) ?
                                    p_section->i_table_id : 0x42;

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, i_table_id, "SDT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* We have a valid SDT section */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *)p_dvbpsi->p_decoder;
    dvbpsi_sdt_decoder_t *p_sdt_decoder
                        = (dvbpsi_sdt_decoder_t*)p_private_decoder;

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitSDT(p_sdt_decoder, true);
        p_sdt_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_sdt_decoder->p_building_sdt)
        {
            if (dvbpsi_CheckSDT(p_dvbpsi, p_sdt_decoder, p_section))
                dvbpsi_ReInitSDT(p_sdt_decoder, true);
        }
        else
        {
            if(    (p_sdt_decoder->b_current_valid)
                && (p_sdt_decoder->current_sdt.i_version == p_section->i_version)
                && (p_sdt_decoder->current_sdt.b_current_next == p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "SDT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
        }
    }

    /* Add section to SDT */
    if (!dvbpsi_AddSectionSDT(p_dvbpsi, p_sdt_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "SDT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_sdt_decoder)))
    {
        assert(p_sdt_decoder->pf_sdt_callback);

        /* Save the current information */
        p_sdt_decoder->current_sdt = *p_sdt_decoder->p_building_sdt;
        p_sdt_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_sdt_sections_decode(p_sdt_decoder->p_building_sdt,
                                   p_sdt_decoder->p_sections);
        /* signal the new SDT */
        p_sdt_decoder->pf_sdt_callback(p_sdt_decoder->p_cb_data,
                                       p_sdt_decoder->p_building_sdt);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitSDT(p_sdt_decoder, false);
        assert(p_sdt_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_sdt_sections_decode
 *****************************************************************************
 * SDT decoder.
 *****************************************************************************/
void dvbpsi_sdt_sections_decode(dvbpsi_sdt_t* p_sdt,
                                dvbpsi_psi_section_t* p_section)
{
    uint8_t *p_byte, *p_end;

    while (p_section)
    {
        for (p_byte = p_section->p_payload_start + 3;
             p_byte + 4 < p_section->p_payload_end;)
        {
            uint16_t i_service_id = ((uint16_t)(p_byte[0]) << 8) | p_byte[1];
            bool b_eit_schedule = ((p_byte[2] & 0x2) >> 1);
            bool b_eit_present =  ((p_byte[2]) & 0x1);
            uint8_t i_running_status = (uint8_t)(p_byte[3]) >> 5;
            bool b_free_ca = ((p_byte[3] & 0x10) >> 4);
            uint16_t i_srv_length = ((uint16_t)(p_byte[3] & 0xf) <<8) | p_byte[4];
            dvbpsi_sdt_service_t* p_service = dvbpsi_sdt_service_add(p_sdt,
                    i_service_id, b_eit_schedule, b_eit_present,
                    i_running_status, b_free_ca);

            /* Service descriptors */
            p_byte += 5;
            p_end = p_byte + i_srv_length;
            if( p_end > p_section->p_payload_end ) break;

            while(p_byte + 2 <= p_end)
            {
                uint8_t i_tag = p_byte[0];
                uint8_t i_length = p_byte[1];
                if (i_length + 2 <= p_end - p_byte)
                    dvbpsi_sdt_service_descriptor_add(p_service, i_tag, i_length, p_byte + 2);
                p_byte += 2 + i_length;
            }
        }
        p_section = p_section->p_next;
    }
}

/*****************************************************************************
 * dvbpsi_sdt_sections_generate
 *****************************************************************************
 * Generate SDT sections based on the dvbpsi_sdt_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_sdt_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_sdt_t* p_sdt)
{
    dvbpsi_psi_section_t *p_result = dvbpsi_NewPSISection(1024);
    dvbpsi_psi_section_t *p_current = p_result;
    dvbpsi_psi_section_t *p_prev;

    dvbpsi_sdt_service_t *p_service = p_sdt->p_first_service;

    p_current->i_table_id = 0x42;
    p_current->b_syntax_indicator = true;
    p_current->b_private_indicator = true;
    p_current->i_length = 12;                    /* header + CRC_32 */
    p_current->i_extension = p_sdt->i_extension; /* is transport_stream_id */
    p_current->i_version = p_sdt->i_version;
    p_current->b_current_next = p_sdt->b_current_next;
    p_current->i_number = 0;
    p_current->p_payload_end += 11;               /* just after the header */
    p_current->p_payload_start = p_current->p_data + 8;

    /* Original Network ID */
    p_current->p_data[8] = (p_sdt->i_network_id >> 8) ;
    p_current->p_data[9] = p_sdt->i_network_id;
    p_current->p_data[10] = 0xff;

    /* SDT service */
    while (p_service != NULL)
    {
        uint8_t * p_service_start = p_current->p_payload_end;
        uint16_t i_service_length = 5;

        dvbpsi_descriptor_t * p_descriptor = p_service->p_first_descriptor;

        while ((p_descriptor != NULL)&& ((p_service_start - p_current->p_data) + i_service_length <= 1020))
        {
            i_service_length += p_descriptor->i_length + 2;
            p_descriptor = p_descriptor->p_next;
        }

        if ((p_descriptor != NULL) && (p_service_start - p_current->p_data != 11) && (i_service_length <= 1009))
        {
            /* will put more descriptors in an empty section */
            dvbpsi_debug(p_dvbpsi, "SDT generator","create a new section to carry more Service descriptors");

            p_prev = p_current;
            p_current = dvbpsi_NewPSISection(1024);
            p_prev->p_next = p_current;

            p_current->i_table_id = 0x42;
            p_current->b_syntax_indicator = true;
            p_current->b_private_indicator = true;
            p_current->i_length = 12;                 /* header + CRC_32 */
            p_current->i_extension = p_sdt->i_extension;;
            p_current->i_version = p_sdt->i_version;
            p_current->b_current_next = p_sdt->b_current_next;
            p_current->i_number = p_prev->i_number + 1;
            p_current->p_payload_end += 11;           /* just after the header */
            p_current->p_payload_start = p_current->p_data + 8;

            /* Original Network ID */
            p_current->p_data[8] = (p_sdt->i_network_id >> 8) ;
            p_current->p_data[9] = p_sdt->i_network_id;
            p_current->p_data[10] = 0xff;

            p_service_start = p_current->p_payload_end;
        }

        p_service_start[0] = (p_service->i_service_id >>8);
        p_service_start[1] = (p_service->i_service_id );
        p_service_start[2] = 0xfc | (p_service-> b_eit_schedule  ? 0x2 : 0x0) | (p_service->b_eit_present ? 0x01 : 0x00);
        p_service_start[3] = ((p_service->i_running_status & 0x07) << 5 ) | ((p_service->b_free_ca & 0x1) << 4);

        /* Increase the length by 5 */
        p_current->p_payload_end += 5;
        p_current->i_length += 5;

        /* ES descriptors */
        p_descriptor = p_service->p_first_descriptor;
        while ((p_descriptor != NULL) && ( (p_current->p_payload_end - p_current->p_data) + p_descriptor->i_length <= 1018))
        {
            /* p_payload_end is where the descriptor begins */
            p_current->p_payload_end[0] = p_descriptor->i_tag;
            p_current->p_payload_end[1] = p_descriptor->i_length;
            memcpy(p_current->p_payload_end + 2, p_descriptor->p_data, p_descriptor->i_length);
            /* Increase length by descriptor_length + 2 */
            p_current->p_payload_end += p_descriptor->i_length + 2;
            p_current->i_length += p_descriptor->i_length + 2;
            p_descriptor = p_descriptor->p_next;
        }

        if (p_descriptor != NULL)
            dvbpsi_error(p_dvbpsi, "SDT generator", "unable to carry all the descriptors");

        /* ES_info_length */
        i_service_length = p_current->p_payload_end - p_service_start - 5;
        p_service_start[3] |= ((i_service_length  >> 8) & 0x0f);
        p_service_start[4] = i_service_length;

        p_service = p_service->p_next;
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
