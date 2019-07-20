/*****************************************************************************
 * libdvbpsi.h: DVB PSI Information
 *****************************************************************************
 * Copyright (C) 2010-2011 M2X BV
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
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
 *****************************************************************************/

#ifndef DVBINFO_DVBPSI_H_
#define DVBINFO_DVBPSI_H_

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Date and time */
typedef int64_t mtime_t;
mtime_t mdate(void);

/* Summary */
#define SUM_BANDWIDTH 0
#define SUM_TABLE     1
#define SUM_PACKET    2
#define SUM_WIRE      3

/* MPEG-TS PSI decoders */
typedef struct ts_stream_t ts_stream_t;
typedef void (* ts_stream_log_cb)(void *data, const int level, const char *msg, ...);

/* */
ts_stream_t *libdvbpsi_init(int debug, ts_stream_log_cb pf_log, void *cb_data);
bool libdvbpsi_process(ts_stream_t *stream, uint8_t *buf, ssize_t length, mtime_t date);
void libdvbpsi_summary(FILE *fd, ts_stream_t *stream, const int summary_mode);
void libdvbpsi_exit(ts_stream_t *stream);

#endif
