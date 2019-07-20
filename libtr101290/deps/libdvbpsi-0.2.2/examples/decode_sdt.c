/*****************************************************************************
 * decode_sdt.c: SDT decoder example
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: decode_sdt.c,v 1.1 2002/12/11 13:04:56 jobi Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *          Johan Bilien <jobi@via.ecp.fr>
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
#include <unistd.h>
#include <fcntl.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

/* The libdvbpsi distribution defines DVBPSI_DIST */
#ifdef DVBPSI_DIST
#include "../src/dvbpsi.h"
#include "../src/psi.h"
#include "../src/demux.h"
#include "../src/descriptor.h"
#include "../src/tables/sdt.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/psi.h>
#include <dvbpsi/demux.h>
#include <dvbpsi/descriptor.h>
#include <dvbpsi/sdt.h>
#endif


/*****************************************************************************
 * ReadPacket
 *****************************************************************************/
static int ReadPacket(int i_fd, uint8_t* p_dst)
{
  int i = 187;
  int i_rc = 1;

  p_dst[0] = 0;

  while((p_dst[0] != 0x47) && (i_rc > 0))
  {
    i_rc = read(i_fd, p_dst, 1);
  }

  while((i != 0) && (i_rc > 0))
  {
    i_rc = read(i_fd, p_dst + 188 - i, i);
    if(i_rc >= 0)
      i -= i_rc;
  }

  return (i == 0) ? 1 : 0;
}


/*****************************************************************************
 * DumpDescriptors
 *****************************************************************************/
static void DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
  while(p_descriptor)
  {
    int i;
    printf("%s 0x%02x : \"", str, p_descriptor->i_tag);
    for(i = 0; i < p_descriptor->i_length; i++)
      printf("%c", p_descriptor->p_data[i]);
    printf("\"\n");
    p_descriptor = p_descriptor->p_next;
  }
};


/*****************************************************************************
 * DumpSDT
 *****************************************************************************/
static void DumpSDT(void* p_zero, dvbpsi_sdt_t* p_sdt)
{
  dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;
  printf(  "\n");
  printf(  "New active SDT\n");
  printf(  "  ts_id : %d\n",
         p_sdt->i_ts_id);
  printf(  "  version_number : %d\n",
         p_sdt->i_version);
  printf(  "  network_id        : %d\n",
         p_sdt->i_network_id);
  printf(  "    | service_id \n");
  while(p_service)
  {
    printf("    | 0x%02x \n",
           p_service->i_service_id);
    DumpDescriptors("    |  ]", p_service->p_first_descriptor);
    p_service = p_service->p_next;
  }
  dvbpsi_DeleteSDT(p_sdt);
}

/*****************************************************************************
 * NewSubtable
 *****************************************************************************/
static void NewSubtable(void * p_zero, dvbpsi_handle h_dvbpsi,
                 uint8_t i_table_id, uint16_t i_extension)
{
  if(i_table_id == 0x42)
  {
    dvbpsi_AttachSDT(h_dvbpsi, i_table_id, i_extension, DumpSDT, NULL);
  }
}

/*****************************************************************************
 * main
 *****************************************************************************/
int main(int i_argc, char* pa_argv[])
{
  int i_fd;
  uint8_t data[188];
  dvbpsi_handle h_dvbpsi;
  int b_ok;

  if(i_argc != 2)
    return 1;

  i_fd = open(pa_argv[1], 0);

  h_dvbpsi = dvbpsi_AttachDemux(NewSubtable, NULL);

  b_ok = ReadPacket(i_fd, data);

  while(b_ok)
  {
    uint16_t i_pid = ((uint16_t)(data[1] & 0x1f) << 8) + data[2];
    if(i_pid == 0x11)
      dvbpsi_PushPacket(h_dvbpsi, data);
    b_ok = ReadPacket(i_fd, data);
  }

  dvbpsi_DetachDemux(h_dvbpsi);

  return 0;
}

