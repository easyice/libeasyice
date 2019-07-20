/*****************************************************************************
 * dvbpsi_private.h: main private header
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

#ifndef _DVBPSI_DVBPSI_PRIVATE_H_
#define _DVBPSI_DVBPSI_PRIVATE_H_

#include <stdarg.h>

extern uint32_t dvbpsi_crc32_table[];

/*****************************************************************************
 * Error management
 *****************************************************************************/
#define DVBPSI_ERROR(src, str)                                          \
        fprintf(stderr, "libdvbpsi error (" src "): " str "\n");
#ifdef HAVE_VARIADIC_MACROS
#  define DVBPSI_ERROR_ARG(src, str, x...)                              \
        fprintf(stderr, "libdvbpsi error (" src "): " str "\n", x);
#else
   __attribute__((deprecated))
   static inline void DVBPSI_ERROR_ARG( char *src, const char *str, ... )
   { va_list ap; va_start( ap, str );
     vfprintf(stderr, str, ap); fprintf(stderr,"\n"); va_end( ap ); }
#endif

#ifdef DEBUG
#  define DVBPSI_DEBUG(src, str)                                        \
          fprintf(stderr, "libdvbpsi debug (" src "): " str "\n");
#  ifdef HAVE_VARIADIC_MACROS
#     define DVBPSI_DEBUG_ARG(src, str, x...)                           \
          fprintf(stderr, "libdvbpsi debug (" src "): " str "\n", x);
#  else
      __attribute__((deprecated))
      static inline void DVBPSI_DEBUG_ARG( char *src, const char *str, ... )
      { va_list ap; va_start( ap, str );
        vfprintf(stderr, str, ap); fprintf(stderr,"\n"); va_end( ap ); }
#  endif
#else
#  define DVBPSI_DEBUG(src, str)
#  ifdef HAVE_VARIADIC_MACROS
#     define DVBPSI_DEBUG_ARG(src, str, x...)
#  else
      __attribute__((deprecated))
      static inline void DVBPSI_DEBUG_ARG( char *src, const char *str, ... ) {}
#  endif
#endif


#else
#error "Multiple inclusions of dvbpsi_private.h"
#endif

