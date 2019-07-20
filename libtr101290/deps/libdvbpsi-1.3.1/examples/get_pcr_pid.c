/*****************************************************************************
 * get_pcr_pid.c: stdout the PID of the PCR of a given program
 *****************************************************************************
 * Copyright (C) 2009-2011 VideoLAN
 * $Id: pcread.c 15 2006-06-15 22:17:58Z cmassiot $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

/* The libdvbpsi distribution defines DVBPSI_DIST */
#ifdef DVBPSI_DIST
#include "../src/dvbpsi.h"
#include "../src/descriptor.h"
#include "../src/demux.h"
#include "../src/tables/pat.h"
#include "../src/tables/pmt.h"
#include "../src/psi.h"
#else
#include <dvbpsi/dvbpsi.h>
#include <dvbpsi/descriptor.h>
#include <dvbpsi/demux.h>
#include <dvbpsi/pat.h>
#include <dvbpsi/pmt.h>
#include <dvbpsi/psi.h>
#endif

/*****************************************************************************
 * Local declarations
 *****************************************************************************/
#define READ_ONCE 100
#define MAX_PROGRAMS 25
#define TS_SIZE 188

static int i_fd = -1;
static int i_ts_read = 0;
static dvbpsi_t *p_pat_dvbpsi_fd;
static int i_nb_programs = 0;
static uint16_t i_program = 0;
static uint16_t pi_pmt_pids[MAX_PROGRAMS];
static dvbpsi_t *p_pmt_dvbpsi_fds[MAX_PROGRAMS];

/*****************************************************************************
 * DVBPSI messaging callback
 *****************************************************************************/
static void message(dvbpsi_t *handle, const dvbpsi_msg_level_t level, const char* msg)
{
    switch(level)
    {
        case DVBPSI_MSG_ERROR: fprintf(stderr, "Error: "); break;
        case DVBPSI_MSG_WARN:  fprintf(stderr, "Warning: "); break;
        case DVBPSI_MSG_DEBUG: fprintf(stderr, "Debug: "); break;
        default: /* do nothing */
            return;
    }
    fprintf(stderr, "%s\n", msg);
}

/*****************************************************************************
 * PMTCallback
 *****************************************************************************/
static void PMTCallback( void *_unused, dvbpsi_pmt_t *p_pmt )
{
    if ( p_pmt->i_program_number != i_program )
    {
        dvbpsi_pmt_delete( p_pmt );
        return;
    }

    printf( "%u\n", p_pmt->i_pcr_pid );
    close( i_fd );
    exit(EXIT_SUCCESS);
}

/*****************************************************************************
 * PATCallback
 *****************************************************************************/
static void PATCallback( void *_unused, dvbpsi_pat_t *p_pat )
{
    dvbpsi_pat_program_t *p_program;

    if (i_nb_programs >= MAX_PROGRAMS)
    {
        fprintf(stderr, "Too many PMT programs\n");
        dvbpsi_pat_delete( p_pat );
        return;
    }

    for( p_program = p_pat->p_first_program; p_program != NULL && i_nb_programs < MAX_PROGRAMS;
         p_program = p_program->p_next )
    {
        if( p_program->i_number != 0
             && (!i_program || i_program == p_program->i_number) )
        {
            pi_pmt_pids[i_nb_programs] = p_program->i_pid;
            p_pmt_dvbpsi_fds[i_nb_programs] = dvbpsi_new(&message, DVBPSI_MSG_DEBUG);
            if (p_pmt_dvbpsi_fds[i_nb_programs])
            {
                if (dvbpsi_pmt_attach(p_pmt_dvbpsi_fds[i_nb_programs],
                                      p_program->i_number, PMTCallback, NULL))
                    i_nb_programs++;
            }
        }
    }

    dvbpsi_pat_delete( p_pat );
}

/*****************************************************************************
 * TSHandle: find and decode PSI
 *****************************************************************************/
static inline uint16_t ts_GetPID( const uint8_t *p_ts )
{
    return (((uint16_t)p_ts[1] & 0x1f) << 8) | p_ts[2];
}

static inline int ts_CheckSync( const uint8_t *p_ts )
{
    return p_ts[0] == 0x47;
}

static void TSHandle( uint8_t *p_ts )
{
    uint16_t i_pid = ts_GetPID( p_ts );
    int i;

    if ( !ts_CheckSync( p_ts ) )
    {
        fprintf( stderr, "lost TS synchro, go and fix your file "
#if defined(WIN32)
                 " (pos=%u)\n",  (uint32_t) i_ts_read * TS_SIZE
#else
                 " (pos=%ju)\n", (uintmax_t)i_ts_read * TS_SIZE
#endif
	);
        exit(EXIT_FAILURE);
    }

    if ( i_pid == 0 && !i_nb_programs )
    {
        dvbpsi_packet_push( p_pat_dvbpsi_fd, p_ts );
    }

    for ( i = 0; i < i_nb_programs; i++ )
    {
        if ( pi_pmt_pids[i] == i_pid )
            dvbpsi_packet_push( p_pmt_dvbpsi_fds[i], p_ts );
    }
}

/*****************************************************************************
 * Entry point
 *****************************************************************************/
int main( int i_argc, char **pp_argv )
{
    uint8_t *p_buffer;
    int result = EXIT_FAILURE;

    if ( i_argc < 2 || i_argc > 3 || !strcmp( pp_argv[1], "-" ) )
    {
        fprintf( stderr, "Usage: get_pcr_pid <input ts> [<program number>]\n" );
        exit(EXIT_FAILURE);
    }

    if ( (i_fd = open( pp_argv[1], O_RDONLY )) == -1 )
    {
        fprintf( stderr, "open(%s) failed (%s)\n", pp_argv[1],
                 strerror(errno) );
        exit(EXIT_FAILURE);
    }

    if ( i_argc == 3 )
        i_program = strtol( pp_argv[2], NULL, 0 );

    p_pat_dvbpsi_fd = dvbpsi_new(&message, DVBPSI_MSG_DEBUG);
    if (p_pat_dvbpsi_fd == NULL)
        goto out;

    if (!dvbpsi_pat_attach(p_pat_dvbpsi_fd, PATCallback, NULL ))
        goto out;

    p_buffer = malloc( TS_SIZE * READ_ONCE );
    if (p_buffer == NULL)
        goto out;

    for ( ; ; )
    {
        int i;
        ssize_t i_ret;

        if ( (i_ret = read( i_fd, p_buffer, TS_SIZE * READ_ONCE )) < 0 )
        {
            fprintf( stderr, "read error (%s)\n", strerror(errno) );
            break;
        }
        if ( i_ret == 0 )
        {
            fprintf( stderr, "end of file reached\n" );
            break;
        }

        for ( i = 0; i < i_ret / TS_SIZE; i++ )
        {
            TSHandle( p_buffer + TS_SIZE * i );
            i_ts_read++;
        }
    }
    free( p_buffer );

    for( int i = 0; i < MAX_PROGRAMS; i++)
    {
        if (p_pmt_dvbpsi_fds[i])
        {
            dvbpsi_pmt_detach(p_pmt_dvbpsi_fds[i]);
            dvbpsi_delete(p_pmt_dvbpsi_fds[i]);
        }
        p_pmt_dvbpsi_fds[i] = NULL;
    }
    result = EXIT_SUCCESS;

out:
    if (p_pat_dvbpsi_fd)
    {
      dvbpsi_pat_detach(p_pat_dvbpsi_fd);
      dvbpsi_delete(p_pat_dvbpsi_fd);
    }
    close( i_fd );

    return result;
}
