/*****************************************************************************
 * decode_bat.c: BAT decoder example
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
 * $Id: decode_bat.c 01 2010-04-01 17:55:18 zhuzlu $
 *
 * Authors: Zhu zhenglu <zhuzlu@gmail.com>
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
#include "../src/tables/bat.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/psi.h>
#include <dvbpsi/demux.h>
#include <dvbpsi/descriptor.h>
#include <dvbpsi/bat.h>
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
      printf("%.2x", p_descriptor->p_data[i]);
    printf("\"\n");
    p_descriptor = p_descriptor->p_next;
  }
};

/*****************************************************************************
 * Print_0xb1
 *****************************************************************************/
static void Print_DescTag_0xb1(uint16_t i_ts_id, dvbpsi_descriptor_t* p_descriptor)
{
    int i;
    uint8_t *pdata;
    unsigned int sub_bouquet_id;
    int num;
    unsigned int formater;
    pdata = p_descriptor->p_data;
    num=(p_descriptor->i_length-2)/9;
    sub_bouquet_id= (((unsigned int)pdata[0]&0xff)<<8)|pdata[1];
    if(sub_bouquet_id!=0xffff)
    {
        printf("sub_bouquet_id!=0xffff\n");
        return;
    }
    if(num*9!=p_descriptor->i_length-2)
    {
        printf("num of private_services error\n");
        return;
    }
    pdata+=2;

    printf("\nts_id: %d, service_num: %d, service_id list: \n",i_ts_id,num);
    formater=0;
    for(i = 0; i < num; i++)
    {
      uint16_t service_id=(((uint16_t)pdata[0]&0xff)<<8)|pdata[1];
      printf("%.4x ", service_id);
      formater++;
      if(0 == formater%16)
      {
          printf("\n");
      }
      pdata+=9;
    }
    printf("\r\n");

}

/*****************************************************************************
 * DumpDescriptors_verbose
 *****************************************************************************/
static void DumpDescriptors_verbose(uint16_t i_ts_id, dvbpsi_descriptor_t* p_descriptor)
{
  while(p_descriptor)
  {
    if(0xb1 == p_descriptor->i_tag)
    {
        Print_DescTag_0xb1(i_ts_id,p_descriptor);
    }
    p_descriptor = p_descriptor->p_next;
  }
};


/*****************************************************************************
 * DumpBAT_verbose
 *****************************************************************************/
static void DumpBAT_verbose(void* p_zero, dvbpsi_bat_t* p_bat)
{
    dvbpsi_bat_ts_t* p_ts = p_bat->p_first_ts;
    while(p_ts)
    {
      DumpDescriptors_verbose(p_ts->i_ts_id, p_ts->p_first_descriptor);
      p_ts = p_ts->p_next;
    }
}
/*****************************************************************************
 * DumpBAT
 *****************************************************************************/
static void DumpBAT(void* p_zero, dvbpsi_bat_t* p_bat)
{
  dvbpsi_bat_ts_t* p_ts = p_bat->p_first_ts;
  {
      printf(  "\n");
      printf(  "New active BAT(binary dumped)\n");
      printf(  "  bouquet_id : %d\n",
             p_bat->i_bouquet_id);
      printf(  "  version_number : %d\n",
             p_bat->i_version);
      printf(  "    | ts_id \n");
      while(p_ts)
      {
        printf("    | 0x%02x \n",
               p_ts->i_ts_id);
        DumpDescriptors("    |  ]", p_ts->p_first_descriptor);
        p_ts = p_ts->p_next;
      }
      printf(  "\n");
      printf(  "New active BAT(string dumped)\n");
      DumpBAT_verbose(p_zero,p_bat);
      printf("\n");
  }
  dvbpsi_DeleteBAT(p_bat);
}


/*****************************************************************************
 * NewSubtable
 *****************************************************************************/
static void NewSubtableBAT(void * p_zero, dvbpsi_handle h_dvbpsi,
                 uint8_t i_table_id, uint16_t i_extension)
{
  if(i_table_id == 0x4a)
  {
    dvbpsi_AttachBAT(h_dvbpsi, i_table_id, i_extension, DumpBAT, NULL);
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

  h_dvbpsi = dvbpsi_AttachDemux(NewSubtableBAT, NULL);

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
