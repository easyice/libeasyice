/*****************************************************************************
 * dvbinfo.h: DVB PSI Information
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

#ifndef DVBINFO_H_
#define DVBINFO_H_

/* defined loglevels for use within dvbinfo are as follows,
 * they match to this order of syslog priority levels.
 * { LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG };
 */
#define DVBINFO_LOG_ERROR 0
#define DVBINFO_LOG_WARN  1
#define DVBINFO_LOG_INFO  2
#define DVBINFO_LOG_DEBUG 3

typedef struct params_s
{
    /* parameters */
    char *output;
    char *input;

    int  port;
    char *mcast_interface;

    bool b_udp;
    bool b_tcp;
    bool b_file;

    /* tuning options */
    size_t threshold; /* capture fifo threshold */

    /* */
    int  fd_in;
    int  fd_out;

    int  debug;
    bool b_verbose;
    bool b_monitor; /* run in daemon mode */

    /* statistics */
    bool b_summary; /* write summary */
    struct summary_s {
        int mode;       /* one of: i_summary_mode */
        int64_t period; /* summary period in ms */
        char *file;     /* summary file name    */
        FILE *fd;       /* summary file descriptor */
    } summary;

    /* read data from file of socket */
    ssize_t (*pf_read)(int fd, void *buf, size_t count);
    ssize_t (*pf_write)(int fd, const void *buf, size_t count);
} params_t;

#endif
