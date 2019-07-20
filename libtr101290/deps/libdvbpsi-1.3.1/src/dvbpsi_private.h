/*****************************************************************************
 * dvbpsi_private.h: main private header
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2012 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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

#ifndef _DVBPSI_DVBPSI_PRIVATE_H_
#define _DVBPSI_DVBPSI_PRIVATE_H_

#include <stdarg.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*****************************************************************************
 * Error management
 *
 * libdvbpsi messages have the following format:
 * "libdvbpsi [error | warning | debug] (<component>): <msg>"
 *****************************************************************************/

#ifdef HAVE_VARIADIC_MACROS
void dvbpsi_message(dvbpsi_t *dvbpsi, const int level, const char *fmt, ...);

#  define dvbpsi_error(hnd, src, str, x...)                             \
        dvbpsi_message(hnd, DVBPSI_MSG_ERROR, "libdvbpsi error (%s): " str, src, ##x)
#  define dvbpsi_warning(hnd, src, str, x...)                                \
        dvbpsi_message(hnd, DVBPSI_MSG_WARN, "libdvbpsi warning (%s): " str, src, ##x)
#  define dvbpsi_debug(hnd, src, str, x...)                                  \
        dvbpsi_message(hnd, DVBPSI_MSG_DEBUG, "libdvbpsi debug (%s): " str, src, ##x)
#else
void dvbpsi_error(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...);
void dvbpsi_warning(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...);
void dvbpsi_debug(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...);
#endif

#else
#error "Multiple inclusions of dvbpsi_private.h"
#endif

