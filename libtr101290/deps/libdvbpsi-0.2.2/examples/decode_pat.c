/*****************************************************************************
 * decode_pat.c: PAT decoder example
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
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
#include "../src/tables/pat.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/psi.h>
#include <dvbpsi/pat.h>
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
 * DumpPAT
 *****************************************************************************/
static void DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat)
{
  dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
  printf(  "\n");
  printf(  "New PAT\n");
  printf(  "  transport_stream_id : %d\n", p_pat->i_ts_id);
  printf(  "  version_number      : %d\n", p_pat->i_version);
  printf(  "    | program_number @ [NIT|PMT]_PID\n");
  while(p_program)
  {
    printf("    | %14d @ 0x%x (%d)\n",
           p_program->i_number, p_program->i_pid, p_program->i_pid);
    p_program = p_program->p_next;
  }
  printf(  "  active              : %d\n", p_pat->b_current_next);
  dvbpsi_DeletePAT(p_pat);
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

  h_dvbpsi = dvbpsi_AttachPAT(DumpPAT, NULL);

  b_ok = ReadPacket(i_fd, data);

  while(b_ok)
  {
    uint16_t i_pid = ((uint16_t)(data[1] & 0x1f) << 8) + data[2];
    if(i_pid == 0x0)
      dvbpsi_PushPacket(h_dvbpsi, data);
    b_ok = ReadPacket(i_fd, data);
  }

  dvbpsi_DetachPAT(h_dvbpsi);

  return 0;
}

