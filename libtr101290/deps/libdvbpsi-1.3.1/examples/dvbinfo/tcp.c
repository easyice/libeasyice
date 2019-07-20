/*****************************************************************************
 * tcp.c: network socket abstractions
 *****************************************************************************
 * Copyright (C) 2010-2013 M2X BV
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_INTTYPES_H)
#   include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <net/if.h>
#   if defined(WIN32)
#       include <netinet/if_ether.h>
#   endif
#   include <netdb.h>
#   include <netinet/ip.h>
#   include <netinet/udp.h>
#   include <arpa/inet.h>
#endif

#ifndef SOCK_CLOEXEC
#   include <fcntl.h>
#endif

#include "tcp.h"

#ifdef HAVE_SYS_SOCKET_H

#ifndef SOCK_CLOEXEC
#   include <stdbool.h>
static bool set_fdsocketclosexec(int s)
{
    int flags = fcntl(s, F_GETFD);
    if (flags != -1)
    {
        if (fcntl(s, F_SETFD, flags | FD_CLOEXEC) != -1)
        {
            return true;
        }
    }

    perror("tcp socket error");
    return false;
}
#endif

int tcp_close(int fd)
{
    int result = 0;

    result = shutdown(fd, SHUT_RDWR);
    if (result < 0)
        perror("tcp shutdown error");
    return result;
}

int tcp_open(const char *ipaddress, int port)
{
    int s_ctl = -1;
    int result = -1;

    if (!ipaddress) return -1;

    /* only support ipv4 */
    struct addrinfo hints, *addr;
    char *psz_service;

    if ((port > 65535) || (port < 0))
    {
        fprintf(stderr, "tcp error: invalid port %d specified\n", port);
        return -1;
    }
    if (asprintf(&psz_service, "%d", port) < 0)
        return -1;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET; /* use AF_INET6 for ipv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME | 0;

    result = getaddrinfo(ipaddress, psz_service, &hints, &addr);
    if (result < 0)
    {
        fprintf(stderr, "tcp address error: %s\n", gai_strerror(result));
        free(psz_service);
        return -1;
    }

    for (struct addrinfo *ptr = addr; ptr != NULL; ptr = ptr->ai_next )
    {
        int sflags = 0;
#ifdef SOCK_CLOEXEC
        sflags = SOCK_CLOEXEC;
#endif
        s_ctl = socket(ptr->ai_family, ptr->ai_socktype | sflags, ptr->ai_protocol);
        if (s_ctl < 0)
        {
            perror("tcp socket error");
            continue;
        }

#ifndef SOCK_CLOEXEC
        if (!set_fdsocketclosexec(s_ctl))
        {
            close(s_ctl);
            s_ctl = -1;
            continue;
        }
#endif

        if (setsockopt (s_ctl, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof (int)) < 0)
            perror("tcp setsockopt error");

        result = connect( s_ctl, ptr->ai_addr, ptr->ai_addrlen );
        if (result < 0)
        {
            close(s_ctl);
            s_ctl = -1;
            perror( "tcp connect error" );
            continue;
        }

        break; /* success */
    }

    freeaddrinfo(addr);
    free(psz_service);

    return s_ctl;
}

ssize_t tcp_read(int fd, void *buf, size_t count)
{
    ssize_t err;
again:
    err = recv(fd, buf, count, MSG_WAITALL);
    if (err < 0)
    {
        switch(errno)
        {
            case EINTR:
            case EAGAIN:
                goto again;
            case ECONNREFUSED:
                fprintf(stderr, "remote host refused connection\n");
                break;
            case ENOTCONN:
                fprintf(stderr, "connection not established\n");
                break;
            default:
                fprintf(stderr, "recv error: %s\n", strerror(errno));
                return -1;
        }
    }
    return err;
}
#endif
