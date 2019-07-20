/*
Copyright (C) 2006  Adam Charrett
Copyright (C) 2011-2012  Michael Krufky

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

mgt.c

Decode PSIP Master Guide Table.

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

#include "atsc_mgt.h"

typedef struct dvbpsi_atsc_mgt_decoder_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_atsc_mgt_callback      pf_mgt_callback;
    void *                        p_cb_data;

    dvbpsi_atsc_mgt_t             current_mgt;
    dvbpsi_atsc_mgt_t *           p_building_mgt;

} dvbpsi_atsc_mgt_decoder_t;

static dvbpsi_descriptor_t *dvbpsi_atsc_MGTAddDescriptor(
                                               dvbpsi_atsc_mgt_t *p_mgt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static dvbpsi_atsc_mgt_table_t *dvbpsi_atsc_MGTAddTable(dvbpsi_atsc_mgt_t* p_mgt,
						 uint16_t i_table_type,
						 uint16_t i_table_type_pid,
						 uint8_t  i_table_type_version,
						 uint32_t i_number_bytes);

static dvbpsi_descriptor_t *dvbpsi_atsc_MGTTableAddDescriptor(
                                               dvbpsi_atsc_mgt_table_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

static void dvbpsi_atsc_GatherMGTSections(dvbpsi_t * p_dvbpsi,
                                          dvbpsi_decoder_t *p_decoder,
                                          dvbpsi_psi_section_t * p_section);

static void dvbpsi_atsc_DecodeMGTSections(dvbpsi_atsc_mgt_t* p_mgt,
                              dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_atsc_AttachMGT
 *****************************************************************************
 * Initialize a MGT subtable decoder.
 *****************************************************************************/
bool dvbpsi_atsc_AttachMGT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t*)p_dvbpsi->p_decoder;

    if (dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension))
    {
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder",
                     "Already a decoder for (table_id == 0x%02x extension == 0x%04x)",
                     i_table_id, i_extension);
        return false;
    }

    dvbpsi_atsc_mgt_decoder_t*  p_mgt_decoder;
    p_mgt_decoder = (dvbpsi_atsc_mgt_decoder_t*) dvbpsi_decoder_new(NULL,
                                                  0, true, sizeof(dvbpsi_atsc_mgt_decoder_t));
    if(p_mgt_decoder == NULL)
        return false;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_NewDemuxSubDecoder(i_table_id, i_extension, dvbpsi_atsc_DetachMGT,
                                         dvbpsi_atsc_GatherMGTSections, DVBPSI_DECODER(p_mgt_decoder));
    if (p_subdec == NULL)
    {
        dvbpsi_decoder_delete(DVBPSI_DECODER(p_mgt_decoder));
        return false;
    }

    /* Attach the subtable decoder to the demux */
    dvbpsi_AttachDemuxSubDecoder(p_demux, p_subdec);

    /* MGT decoder information */
    p_mgt_decoder->pf_mgt_callback = pf_callback;
    p_mgt_decoder->p_cb_data = p_cb_data;
    p_mgt_decoder->p_building_mgt = NULL;

    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_DetachMGT
 *****************************************************************************
 * Close a MGT decoder.
 *****************************************************************************/
void dvbpsi_atsc_DetachMGT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;

    dvbpsi_demux_subdec_t* p_subdec;
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, i_table_id, i_extension);
    if (p_subdec == NULL)
    {
        dvbpsi_error(p_dvbpsi, "ATSC MGT Decoder",
                         "No such MGT decoder (table_id == 0x%02x,"
                         "extension == 0x%04x)",
                         i_table_id, i_extension);
        return;
    }

    dvbpsi_atsc_mgt_decoder_t* p_mgt_decoder;
    p_mgt_decoder = (dvbpsi_atsc_mgt_decoder_t*)p_subdec->p_decoder;
    if (!p_mgt_decoder)
        return;

    if (p_mgt_decoder->p_building_mgt)
        dvbpsi_atsc_DeleteMGT(p_mgt_decoder->p_building_mgt);
    p_mgt_decoder->p_building_mgt = NULL;

    dvbpsi_DetachDemuxSubDecoder(p_demux, p_subdec);
    dvbpsi_DeleteDemuxSubDecoder(p_subdec);
}

/*****************************************************************************
 * dvbpsi_atsc_InitMGT
 *****************************************************************************
 * Initialize a pre-allocated dvbpsi_atsc_mgt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_version, uint8_t i_protocol, bool b_current_next)
{
    assert(p_mgt);

    p_mgt->i_table_id = i_table_id;
    p_mgt->i_extension = i_extension;

    p_mgt->i_version = i_version;
    p_mgt->b_current_next = b_current_next;
    p_mgt->i_protocol = i_protocol;
    p_mgt->p_first_table = NULL;
    p_mgt->p_first_descriptor = NULL;
}

dvbpsi_atsc_mgt_t *dvbpsi_atsc_NewMGT(uint8_t i_table_id, uint16_t i_extension,
                                      uint8_t i_version, uint8_t i_protocol, bool b_current_next)
{
    dvbpsi_atsc_mgt_t* p_mgt;
    p_mgt = (dvbpsi_atsc_mgt_t*)calloc(1, sizeof(dvbpsi_atsc_mgt_t));
    if (p_mgt != NULL)
        dvbpsi_atsc_InitMGT(p_mgt, i_table_id, i_extension, i_version, i_protocol, b_current_next);
    return p_mgt;
}

/*****************************************************************************
 * dvbpsi_atsc_EmptyMGT
 *****************************************************************************
 * Clean a dvbpsi_atsc_mgt_t structure.
 *****************************************************************************/
void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t* p_mgt)
{
  dvbpsi_atsc_mgt_table_t* p_table = p_mgt->p_first_table;

  while(p_table != NULL)
  {
    dvbpsi_atsc_mgt_table_t* p_tmp = p_table->p_next;
    dvbpsi_DeleteDescriptors(p_table->p_first_descriptor);
    free(p_table);
    p_table = p_tmp;
  }
  dvbpsi_DeleteDescriptors(p_mgt->p_first_descriptor);
  p_mgt->p_first_table = NULL;
  p_mgt->p_first_descriptor = NULL;
}

void dvbpsi_atsc_DeleteMGT(dvbpsi_atsc_mgt_t *p_mgt)
{
    if (p_mgt)
        dvbpsi_atsc_EmptyMGT(p_mgt);
    free(p_mgt);
    p_mgt = NULL;
}

/*****************************************************************************
 * dvbpsi_atsc_MGTAddDescriptor
 *****************************************************************************
 * Add a descriptor to the MGT table.
 *****************************************************************************/
static dvbpsi_descriptor_t *dvbpsi_atsc_MGTAddDescriptor(
                                               dvbpsi_atsc_mgt_t *p_mgt,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
    dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
    if (p_descriptor == NULL)
        return NULL;

    p_mgt->p_first_descriptor = dvbpsi_AddDescriptor(p_mgt->p_first_descriptor,
                                                     p_descriptor);
    assert(p_mgt->p_first_descriptor);
    if (p_mgt->p_first_descriptor == NULL)
        return NULL;

    return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_atsc_MGTAddTable
 *****************************************************************************
 * Add a Table description at the end of the MGT.
 *****************************************************************************/
static dvbpsi_atsc_mgt_table_t *dvbpsi_atsc_MGTAddTable(dvbpsi_atsc_mgt_t* p_mgt,
						 uint16_t i_table_type,
						 uint16_t i_table_type_pid,
						 uint8_t  i_table_type_version,
						 uint32_t i_number_bytes)
{
  dvbpsi_atsc_mgt_table_t * p_table
                = (dvbpsi_atsc_mgt_table_t*)malloc(sizeof(dvbpsi_atsc_mgt_table_t));
  if(p_table)
  {
    p_table->i_table_type = i_table_type;
    p_table->i_table_type_pid = i_table_type_pid;
    p_table->i_table_type_version = i_table_type_version;
    p_table->i_number_bytes = i_number_bytes;

    p_table->p_first_descriptor = NULL;
    p_table->p_next = NULL;

    if(p_mgt->p_first_table== NULL)
    {
      p_mgt->p_first_table = p_table;
    }
    else
    {
      dvbpsi_atsc_mgt_table_t * p_last_table = p_mgt->p_first_table;
      while(p_last_table->p_next != NULL)
        p_last_table = p_last_table->p_next;
      p_last_table->p_next = p_table;
    }
  }

  return p_table;
}

/*****************************************************************************
 * dvbpsi_MGTTableAddDescriptor
 *****************************************************************************
 * Add a descriptor in the MGT table description.
 *****************************************************************************/
static dvbpsi_descriptor_t *dvbpsi_atsc_MGTTableAddDescriptor(
                                               dvbpsi_atsc_mgt_table_t *p_table,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
{
  dvbpsi_descriptor_t * p_descriptor
                        = dvbpsi_NewDescriptor(i_tag, i_length, p_data);
  if(p_descriptor)
  {
    if(p_table->p_first_descriptor == NULL)
    {
      p_table->p_first_descriptor = p_descriptor;
    }
    else
    {
      dvbpsi_descriptor_t * p_last_descriptor = p_table->p_first_descriptor;
      while(p_last_descriptor->p_next != NULL)
        p_last_descriptor = p_last_descriptor->p_next;
      p_last_descriptor->p_next = p_descriptor;
    }
  }

  return p_descriptor;
}

/*****************************************************************************
 * dvbpsi_ReInitMGT                                                          *
 *****************************************************************************/
static void dvbpsi_ReInitMGT(dvbpsi_atsc_mgt_decoder_t *p_decoder, const bool b_force)
{
    assert(p_decoder);

    dvbpsi_decoder_reset(DVBPSI_DECODER(p_decoder), b_force);

    /* Force redecoding */
    if (b_force)
    {
        /* Free structures */
        if (p_decoder->p_building_mgt)
            dvbpsi_atsc_DeleteMGT(p_decoder->p_building_mgt);
    }
    p_decoder->p_building_mgt = NULL;
}

static bool dvbpsi_CheckMGT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_mgt_decoder_t *p_decoder,
                            dvbpsi_psi_section_t *p_section)
{
    bool b_reinit = false;

    assert(p_dvbpsi);
    assert(p_decoder);

    if (p_decoder->p_building_mgt->i_table_id_ext != p_section->i_extension)
    {
        /* transport_stream_id */
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder",
                     "'transport_stream_id' differs"
                     " whereas no TS discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->p_building_mgt->i_version != p_section->i_version)
    {
        /* version_number */
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder",
                     "'version_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }
    else if (p_decoder->i_last_section_number != p_section->i_last_number)
    {
        /* last_section_number */
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder",
                     "'last_section_number' differs"
                     " whereas no discontinuity has occured");
        b_reinit = true;
    }

    return b_reinit;
}

static bool dvbpsi_AddSectionMGT(dvbpsi_t *p_dvbpsi, dvbpsi_atsc_mgt_decoder_t *p_decoder,
                                 dvbpsi_psi_section_t* p_section)
{
    assert(p_dvbpsi);
    assert(p_decoder);
    assert(p_section);

    /* Initialize the structures if it's the first section received */
    if (!p_decoder->p_building_mgt)
    {
        p_decoder->p_building_mgt = dvbpsi_atsc_NewMGT(p_section->i_table_id,
                                                       p_section->i_extension,
                                                       p_section->i_version,
                                                       p_section->p_payload_start[0],
                                                       p_section->b_current_next);
        if (!p_decoder->p_building_mgt)
            return false;

        p_decoder->i_last_section_number = p_section->i_last_number;
    }

    /* Add to linked list of sections */
    if (dvbpsi_decoder_psi_section_add(DVBPSI_DECODER(p_decoder), p_section))
        dvbpsi_debug(p_dvbpsi, "ATSC MGT decoder", "overwrite section number %d",
                     p_section->i_number);
    return true;
}

/*****************************************************************************
 * dvbpsi_atsc_GatherMGTSections
 *****************************************************************************
 * Callback for the subtable demultiplexor.
 *****************************************************************************/
static void dvbpsi_atsc_GatherMGTSections(dvbpsi_t * p_dvbpsi,
                              dvbpsi_decoder_t *p_decoder,
                              dvbpsi_psi_section_t * p_section)
{
    assert(p_dvbpsi);
    assert(p_dvbpsi->p_decoder);

    if (!dvbpsi_CheckPSISection(p_dvbpsi, p_section, 0xC7, "ATSC MGT decoder"))
    {
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* We have a valid MGT section */
    dvbpsi_demux_t *p_demux = (dvbpsi_demux_t *) p_dvbpsi->p_decoder;
    dvbpsi_atsc_mgt_decoder_t * p_mgt_decoder = (dvbpsi_atsc_mgt_decoder_t*)p_decoder;
    if (!p_mgt_decoder)
    {
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder", "No decoder specified");
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* TS discontinuity check */
    if (p_demux->b_discontinuity)
    {
        dvbpsi_ReInitMGT(p_mgt_decoder, true);
        p_mgt_decoder->b_discontinuity = false;
        p_demux->b_discontinuity = false;
    }
    else
    {
        /* Perform a few sanity checks */
        if (p_mgt_decoder->p_building_mgt)
        {
            if (dvbpsi_CheckMGT(p_dvbpsi, p_mgt_decoder, p_section))
                dvbpsi_ReInitMGT(p_mgt_decoder, true);
        }
        else
        {
            if (   (p_mgt_decoder->b_current_valid)
                && (p_mgt_decoder->current_mgt.i_version == p_section->i_version)
                && (p_mgt_decoder->current_mgt.b_current_next ==
                                               p_section->b_current_next))
            {
                /* Don't decode since this version is already decoded */
                dvbpsi_debug(p_dvbpsi, "ATSC MGT decoder",
                             "ignoring already decoded section %d",
                             p_section->i_number);
                dvbpsi_DeletePSISections(p_section);
                return;
            }
#if 0 /* Do we need to signal b_current_next here? or later */
            if ((p_mgt_decoder->b_current_valid) &&
                (p_mgt_decoder->current_mgt.i_version == p_section->i_version))
            {
                /* Signal a new MGT if the previous one wasn't active */
                if ((!p_mgt_decoder->current_mgt.b_current_next) &&
                     (p_section->b_current_next))
                {
                    dvbpsi_atsc_mgt_t * p_mgt = (dvbpsi_atsc_mgt_t*)malloc(sizeof(dvbpsi_atsc_mgt_t));
                    if (p_mgt)
                    {
                        p_mgt_decoder->current_mgt.b_current_next = true;
                        *p_mgt = p_mgt_decoder->current_mgt;
                        p_mgt_decoder->pf_mgt_callback(p_mgt_decoder->p_cb_data, p_mgt);
                    }
                    else
                        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder", "Could not signal new ATSC MGT.");
                }
            }
            dvbpsi_DeletePSISections(p_section);
            return;
#endif
        }
    }

    /* Add section to MGT */
    if (!dvbpsi_AddSectionMGT(p_dvbpsi, p_mgt_decoder, p_section))
    {
        dvbpsi_error(p_dvbpsi, "ATSC MGT decoder", "failed decoding section %d",
                     p_section->i_number);
        dvbpsi_DeletePSISections(p_section);
        return;
    }

    /* Check if we have all the sections */
    if (dvbpsi_decoder_psi_sections_completed(DVBPSI_DECODER(p_mgt_decoder)))
    {
        assert(p_mgt_decoder->pf_mgt_callback);

        /* Save the current information */
        p_mgt_decoder->current_mgt = *p_mgt_decoder->p_building_mgt;
        p_mgt_decoder->b_current_valid = true;
        /* Decode the sections */
        dvbpsi_atsc_DecodeMGTSections(p_mgt_decoder->p_building_mgt,
                                      p_mgt_decoder->p_sections);
        /* signal the new MGT */
        p_mgt_decoder->pf_mgt_callback(p_mgt_decoder->p_cb_data,
                                       p_mgt_decoder->p_building_mgt);
        /* Delete sections and Reinitialize the structures */
        dvbpsi_ReInitMGT(p_mgt_decoder, false);
        assert(p_mgt_decoder->p_sections == NULL);
    }
}

/*****************************************************************************
 * dvbpsi_DecodeMGTSection
 *****************************************************************************
 * MGT decoder.
 *****************************************************************************/
static void dvbpsi_atsc_DecodeMGTSections(dvbpsi_atsc_mgt_t* p_mgt,
                                          dvbpsi_psi_section_t* p_section)
{
  uint8_t *p_byte, *p_end;

  while(p_section)
  {
    uint16_t i_tables_defined = (p_section->p_payload_start[1] << 8) |
                                (p_section->p_payload_start[2]);
    uint16_t i_tables_count = 0;
    uint16_t i_length = 0;

    for(p_byte = p_section->p_payload_start + 3;
        ((p_byte + 6) < p_section->p_payload_end) && (i_tables_count < i_tables_defined);
        i_tables_count ++)
    {
	dvbpsi_atsc_mgt_table_t* p_table;
	uint16_t i_table_type         = ((uint16_t)(p_byte[0]) << 8) |
	                                ((uint16_t)(p_byte[1]));
	uint16_t i_table_type_pid     = ((uint16_t)(p_byte[2] & 0x1f) << 8) |
	                                ((uint16_t)(p_byte[3] & 0xff));
	uint8_t  i_table_type_version = ((uint8_t) (p_byte[4] & 0x1f));
	uint32_t i_number_bytes       = ((uint32_t)(p_byte[5] << 24)) |
                                        ((uint32_t)(p_byte[6] << 16)) |
                                        ((uint32_t)(p_byte[7] <<  8)) |
                                        ((uint32_t)(p_byte[8]));
        i_length = ((uint16_t)(p_byte[9] & 0xf) <<8) | p_byte[10];

        p_table = dvbpsi_atsc_MGTAddTable(p_mgt,
					  i_table_type,
					  i_table_type_pid,
					  i_table_type_version,
					  i_number_bytes);

        /* Table descriptors */
        p_byte += 11;
        p_end = p_byte + i_length;
        if( p_end > p_section->p_payload_end ) break;

        while(p_byte + 2 <= p_end)
        {
            uint8_t i_tag = p_byte[0];
            uint8_t i_len = p_byte[1];
            if(i_len + 2 <= p_end - p_byte)
              dvbpsi_atsc_MGTTableAddDescriptor(p_table, i_tag, i_len, p_byte + 2);
            p_byte += 2 + i_len;
        }
    }

    /* Table descriptors */
    i_length = ((uint16_t)(p_byte[0] & 0xf) <<8) | p_byte[1];
    p_byte += 2;
    p_end = p_byte + i_length;

    while(p_byte + 2 <= p_end)
    {
        uint8_t i_tag = p_byte[0];
        uint8_t i_len = p_byte[1];
        if(i_length + 2 <= p_end - p_byte)
          dvbpsi_atsc_MGTAddDescriptor(p_mgt, i_tag, i_len, p_byte + 2);
        p_byte += 2 + i_len;
    }
    p_section = p_section->p_next;
  }
}
