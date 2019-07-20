/*****************************************************************************
 * udp.c: network socket abstractions
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>

#ifdef HAVE_SYS_SOCKET_H
#   include <sys/socket.h>
#   include <netinet/in.h>
#   ifdef HAVE_NET_IF_H
#       include <net/if.h>
#   endif
#   if defined(WIN32)
#       include <netinet/if_ether.h>
#   endif
#   include <netdb.h>
#   include <netinet/ip.h>
#   include <netinet/udp.h>
#   include <arpa/inet.h>
#endif

#ifndef SOL_IP
#   define SOL_IP IPPROTO_IP
#endif
#ifndef SOL_IPV6
#   define SOL_IPV6 IPPROTO_IPV6
#endif
#ifndef IPPROTO_IPV6
# define IPPROTO_IPV6 41 /* IANA */
#endif

#ifndef SOCK_CLOEXEC
#   include <fcntl.h>
#endif

#include <assert.h>

#include "udp.h"

#ifdef HAVE_SYS_SOCKET_H
static bool is_multicast(const struct sockaddr_storage *saddr, socklen_t len)
{
    const struct sockaddr *addr = (const struct sockaddr *) saddr;

    switch(addr->sa_family)
    {
#if defined(IN_MULTICAST)
        case AF_INET:
        {
            const struct sockaddr_in *ip = (const struct sockaddr_in *)saddr;
            if ((size_t)len < sizeof (*ip))
                return false;
            return IN_MULTICAST(ntohl(ip->sin_addr.s_addr)) != 0;
        }
#endif
#if defined(IN6_IS_ADDR_MULTICAST)
        case AF_INET6:
        {
            const struct sockaddr_in6 *ip6 = (const struct sockaddr_in6 *)saddr;
            if ((size_t)len < sizeof (*ip6))
                return false;
            return IN6_IS_ADDR_MULTICAST(&ip6->sin6_addr) != 0;
        }
#endif
    }
    return false;
}

static bool mcast_connect(int s, const char *interface, const struct sockaddr_storage *saddr, socklen_t len)
{
    unsigned int ifindex = interface ? if_nametoindex(interface) : 0;
    const struct sockaddr *addr = (const struct sockaddr *) saddr;

#if defined(MCAST_JOIN_GROUP)
    /* Source Specific Multicast Join */
    struct group_req greq;
    memset(&greq, 0, sizeof(greq));

    if (ifindex == 0)
        return false;

    greq.gr_interface = ifindex;
    assert(len <= sizeof(greq.gr_group));
    memcpy(&greq.gr_group, addr, len);

    switch(addr->sa_family)
    {
        case AF_INET6:
        {
            const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)saddr;
            assert(len >= sizeof (struct sockaddr_in6));
            if (sin6->sin6_scope_id != 0)
                greq.gr_interface = sin6->sin6_scope_id;
            if (setsockopt(s, SOL_IPV6, MCAST_JOIN_GROUP, &greq, sizeof(greq)) == 0)
                return true;
            break;
        }
        case AF_INET:
            if (setsockopt(s, SOL_IP, MCAST_JOIN_GROUP, &greq, sizeof(greq)) == 0)
                return true;
            break;
        default:
            return false;
    }
#else
    switch(addr->sa_family)
    {
        case AF_INET6:
        {
            struct ipv6_mreq ipv6mr;
            const struct sockaddr_in6 *ip6 = (const struct sockaddr_in6 *)saddr;

            memset(&ipv6mr, 0, sizeof (ipv6mr));
            assert(len >= sizeof (struct sockaddr_in6));
            ipv6mr.ipv6mr_multiaddr = ip6->sin6_addr;
            ipv6mr.ipv6mr_interface = (ifindex > 0) ? ifindex : ip6->sin6_scope_id;
# ifdef IPV6_JOIN_GROUP
            if (setsockopt(s, SOL_IPV6, IPV6_JOIN_GROUP, &ipv6mr, sizeof (ipv6mr)) == 0)
# else
            if (setsockopt(s, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mr, sizeof (ipv6mr)) == 0)
# endif
                return true;
            break;
        }
# ifdef IP_ADD_MEMBERSHIP
        case AF_INET:
        {
            struct ip_mreq imr;

            memset(&imr, 0, sizeof (imr));
            assert(len >= sizeof (struct sockaddr_in));
            imr.imr_multiaddr = ((const struct sockaddr_in *)saddr)->sin_addr;
#if 0       /* TODO: Source Specific Multicast Join */
            if (ifaddr) /* Linux specific interface bound multicast address */
               imr.imr_address.s_addr = if_addr;
            if (ifindex > 0)
                imr.imr_index = ifindex;
#endif
            if (setsockopt(s, SOL_IP, IP_ADD_MEMBERSHIP, &imr, sizeof (imr)) == 0)
                return true;
            break;
        }
# endif
    }
#endif
    return false;
}

static bool is_ipv6(const char *ipaddress)
{
    /* NOTE: we assume the address does not include the port number */
    return (strchr(ipaddress, ':') != NULL);
}

#ifndef SOCK_CLOEXEC
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

    perror("udp socket error");
    return false;
}
#endif

int udp_close(int fd)
{
    int result = 0;

    result = close(fd);
    if (result < 0)
        perror("udp shutdown error");
    return result;
}

int udp_open(const char *interface, const char *ipaddress, int port)
{
    int s_ctl = -1;
    int result = -1;

    if (!ipaddress) return -1;

    /* only support ipv4 */
    struct addrinfo hints, *addr;
    char *psz_service;

    if ((port > 65535) || (port < 0))
    {
        fprintf(stderr, "udp error: invalid port %d specified\n", port);
        return -1;
    }
    if (asprintf(&psz_service, "%d", port) < 0)
        return -1;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = is_ipv6(ipaddress) ? AF_INET6: AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_CANONNAME | 0;

    result = getaddrinfo(ipaddress, psz_service, &hints, &addr);
    if (result < 0)
    {
        fprintf(stderr, "udp address error: %s\n", gai_strerror(result));
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
            perror("udp socket error");
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

        /* Increase the receive buffer size to 1/2MB (8Mb/s during 1/2s)
         * to avoid packet loss caused in case of scheduling hiccups */
        if (setsockopt (s_ctl, SOL_SOCKET, SO_RCVBUF,
                    (void *)&(int){ 0x80000 }, sizeof (int)) < 0)
            perror("udp setsockopt error");

        if (setsockopt (s_ctl, SOL_SOCKET, SO_SNDBUF,
                    (void *)&(int){ 0x80000 }, sizeof (int)) < 0)
            perror("udp setsockopt error");

        if (setsockopt (s_ctl, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof (int)) < 0)
            perror("udp setsockopt error");

        result = bind(s_ctl, ptr->ai_addr, ptr->ai_addrlen);
        if (result < 0)
        {
            close(s_ctl);
            s_ctl = -1;
            perror("udp bind error");
            continue;
        }

        const struct sockaddr_storage *saddr = (const struct sockaddr_storage *)&ptr->ai_addr;
        if (is_multicast(saddr, ptr->ai_addrlen) &&
            mcast_connect(s_ctl, NULL, saddr, ptr->ai_addrlen))
        {
            close(s_ctl);
            s_ctl = -1;
            perror("mcast connect error");
            continue;
        }

        break;
    }

    freeaddrinfo(addr);
    free(psz_service);

    return s_ctl;
}

ssize_t udp_read(int fd, void *buf, size_t count)
{
    ssize_t err;
again:
    err = recv(fd, buf, count, 0);
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
