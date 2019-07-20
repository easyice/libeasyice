/*****************************************************************************
 * gen_pmt.c: PMT generator
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
#include <stdbool.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <sys/types.h>

/* the libdvbpsi distribution defines DVBPSI_DIST */
#ifdef DVBPSI_DIST
#include "../src/dvbpsi.h"
#include "../src/psi.h"
#include "../src/descriptor.h"
#include "../src/tables/pmt.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/psi.h>
#include <dvbpsi/descriptor.h>
#include <dvbpsi/pmt.h>
#endif


/*****************************************************************************
 * writePSI
 *****************************************************************************/
static void writePSI(uint8_t* p_packet, dvbpsi_psi_section_t* p_section)
{
  p_packet[0] = 0x47;

  while(p_section)
  {
    size_t i_bytes_written = 0;
    uint8_t* p_pos_in_ts;
    uint8_t* p_byte = p_section->p_data;
    uint8_t* p_end =   p_section->p_payload_end
                     + (p_section->b_syntax_indicator ? 4 : 0);

    p_packet[1] |= 0x40;
    p_packet[3] = (p_packet[3] & 0x0f) | 0x10;

    p_packet[4] = 0x00; /* pointer_field */
    p_pos_in_ts = p_packet + 5;

    while((p_pos_in_ts < p_packet + 188) && (p_byte < p_end))
      *(p_pos_in_ts++) = *(p_byte++);
    while(p_pos_in_ts < p_packet + 188)
      *(p_pos_in_ts++) = 0xff;
    i_bytes_written = fwrite(p_packet, 1, 188, stdout);
    if(i_bytes_written == 0)
    {
      fprintf(stderr,"eof detected ... aborting\n");
      return;
    }
    p_packet[3] = (p_packet[3] + 1) & 0x0f;

    while(p_byte < p_end)
    {
      p_packet[1] &= 0xbf;
      p_packet[3] = (p_packet[3] & 0x0f) | 0x10;

      p_pos_in_ts = p_packet + 4;

      while((p_pos_in_ts < p_packet + 188) && (p_byte < p_end))
        *(p_pos_in_ts++) = *(p_byte++);
      while(p_pos_in_ts < p_packet + 188)
        *(p_pos_in_ts++) = 0xff;
      i_bytes_written = fwrite(p_packet, 1, 188, stdout);
      if(i_bytes_written == 0)
      {
        fprintf(stderr,"eof detected ... aborting\n");
        return;
      }

      p_packet[3] = (p_packet[3] + 1) & 0x0f;
    }

    p_section = p_section->p_next;
  }
}

static void message(dvbpsi_t *handle, const dvbpsi_msg_level_t level, const char* msg)
{
    switch(level)
    {
        case DVBPSI_MSG_ERROR: fprintf(stderr, "Error: "); break;
        case DVBPSI_MSG_WARN:  fprintf(stderr, "Warning: "); break;
        case DVBPSI_MSG_DEBUG: fprintf(stderr, "Debug: "); break;
        default: /* do nothing */
            return;
    }
    fprintf(stderr, "%s\n", msg);
}

/*****************************************************************************
 * main
 *****************************************************************************/
int main(int i_argc, char* pa_argv[])
{
  uint8_t packet[188];
  uint8_t data[] = "abcdefghijklmnopqrstuvwxyz";
  dvbpsi_pmt_t pmt;
  dvbpsi_pmt_es_t* p_es;
  dvbpsi_psi_section_t* p_section1, * p_section2;
  dvbpsi_psi_section_t* p_section3, * p_section4;
  dvbpsi_psi_section_t* p_section5, * p_section6;

  dvbpsi_t *p_dvbpsi = dvbpsi_new(&message, DVBPSI_MSG_DEBUG);
  if (p_dvbpsi == NULL)
      return 1;

  /* PMT generation */
  dvbpsi_pmt_init(&pmt, 12, 0, 0, 42);
  dvbpsi_pmt_descriptor_add(&pmt, 12, 26, data);
  dvbpsi_pmt_descriptor_add(&pmt, 42, 12, data + 12);
  dvbpsi_pmt_descriptor_add(&pmt, 2, 1, data + 4);
  dvbpsi_pmt_descriptor_add(&pmt, 0, 4, data + 7);
  p_es = dvbpsi_pmt_es_add(&pmt, 12, 42);
  dvbpsi_pmt_es_descriptor_add(p_es, 12, 26, data);
  dvbpsi_pmt_es_descriptor_add(p_es, 42, 12, data + 12);
  dvbpsi_pmt_es_descriptor_add(p_es, 2, 1, data + 4);
  dvbpsi_pmt_es_descriptor_add(p_es, 0, 4, data + 7);

  p_section1 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);
  pmt.b_current_next = 1;
  p_section2 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);

  pmt.i_version = 1;

  pmt.b_current_next = 0;
  p_section3 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);
  pmt.b_current_next = 1;
  p_section4 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);

  pmt.i_version = 2;

  pmt.b_current_next = 0;
  p_section5 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);
  pmt.b_current_next = 1;
  p_section6 = dvbpsi_pmt_sections_generate(p_dvbpsi, &pmt);

  /* TS packets generation */
  packet[0] = 0x47;
  packet[1] = 0x02;
  packet[2] = 0x12;
  packet[3] = 0x00;

  writePSI(packet, p_section1);
  writePSI(packet, p_section2);
  writePSI(packet, p_section3);
  writePSI(packet, p_section4);
  writePSI(packet, p_section5);
  writePSI(packet, p_section6);


  dvbpsi_DeletePSISections(p_section1);
  dvbpsi_DeletePSISections(p_section2);
  dvbpsi_DeletePSISections(p_section3);
  dvbpsi_DeletePSISections(p_section4);
  dvbpsi_DeletePSISections(p_section5);
  dvbpsi_DeletePSISections(p_section6);

  dvbpsi_pmt_empty(&pmt);
  dvbpsi_delete(p_dvbpsi);
  return 0;
}

