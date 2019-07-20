/*****************************************************************************
 * gen_crc.c: CRC_32 table generator
 *----------------------------------------------------------------------------
 * Copyright (c)2001-2010 VideoLAN
 * $Id: gen_crc.c,v 1.2 2002/06/02 23:04:08 bozo Exp $
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
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif


/*****************************************************************************
 * main
 *****************************************************************************/
int main(void)
{
  uint32_t table[256];
  uint32_t i, j, k;

  for(i = 0; i < 256; i++)
  {
    k = 0;
    for (j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1)
      k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);
    table[i] = k;
  }

  printf("static uint32_t table[256] =\n{\n");
  for(i = 0; i < 64; i++)
  {
    printf("  0x%08x, 0x%08x, 0x%08x, 0x%08x",
           table[i << 2 | 0],
           table[i << 2 | 1],
           table[i << 2 | 2],
           table[i << 2 | 3]);
    if(i < 63)
      printf(",\n");
    else
      printf("\n");
  }
  printf("};\n");

  return 0;
}

