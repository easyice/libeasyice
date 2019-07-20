/*****************************************************************************
 * connect.c: Routines for establishing a network connection.
 *----------------------------------------------------------------------------
 * Copyright (c) 2005-2010 M2X BV
 * $Id: connect.c 104 2005-03-21 13:38:56Z massiot $
 *
 * Authors: Jean-Paul Saman <jpsaman #_at_# m2x dot nl>
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#if defined(WIN32)
#   include <netinet/if_ether.h>
#endif
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include "connect.h"

int create_tcp_connection( const char *ipaddress, int port )
{
    struct sockaddr_in addr_ctl;
    int s_ctl = -1;
    int result = -1;

    if( !ipaddress ) return -1;

    s_ctl = socket( PF_INET, SOCK_STREAM, 0 );
    if (s_ctl < 0)
    {
        perror( "tcp socket error" );
        return -1;
    }

    addr_ctl.sin_family = AF_INET;
    addr_ctl.sin_addr.s_addr = inet_addr(ipaddress);
    addr_ctl.sin_port = htons(port);

    result = connect( s_ctl, (struct sockaddr*) &addr_ctl, sizeof(addr_ctl) );
    if( result < 0 )
    {
        perror( "tcp connect error" );
        close(s_ctl);
        return -1;
    }

    return s_ctl;
}

int close_connection( int socket_fd )
{
    int result = 0;

    result = shutdown( socket_fd, 2 );
    if( result < 0 )
        perror( "shutdown error" );
    return result;
}

int create_udp_connection( const char *ipaddress, int port )
{
    struct sockaddr_in addr_ctl;
    int s_ctl = -1;
    int result = -1;

    if( !ipaddress ) return -1;

    s_ctl = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s_ctl < 0)
    {
        perror( "udp socket error" );
        return -1;
    }

    addr_ctl.sin_family = AF_INET;
    addr_ctl.sin_addr.s_addr = inet_addr(ipaddress);
    addr_ctl.sin_port = htons(port);

    result = bind(s_ctl, (struct sockaddr*) &addr_ctl, sizeof(addr_ctl));
    if( result < 0 )
    {
        close(s_ctl);
        perror( "udp bind error" );
        return -1;
    }

    return s_ctl;
}
