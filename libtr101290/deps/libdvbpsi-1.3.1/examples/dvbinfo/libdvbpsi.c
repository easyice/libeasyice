/*****************************************************************************
 * dvbpsi.c: DVB PSI Information
 *****************************************************************************
 * Copyright (C) 2010-2012 M2X BV
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

#if defined(HAVE_INTTYPES_H)
#   include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#if defined(HAVE_SYS_TIME_H)
#   include <sys/time.h>
#endif

#include <assert.h>

/* The libdvbpsi distribution defines DVBPSI_DIST */
#ifdef DVBPSI_DIST
#   include "../../src/dvbpsi.h"
#   include "../../src/demux.h"
#   include "../../src/psi.h"
#   include "../../src/descriptor.h"
#   include "../../src/tables/pat.h"
#   include "../../src/tables/pmt.h"
#   include "../../src/tables/cat.h"
#   include "../../src/tables/bat.h"
#   include "../../src/tables/eit.h"
#   include "../../src/tables/nit.h"
#   include "../../src/tables/sdt.h"
#   include "../../src/tables/tot.h"
#   include "../../src/tables/rst.h"
#   include "../../src/descriptors/dr.h"
/*  ATSC PSI Tables */
#   include "../../src/tables/atsc_eit.h"
#   include "../../src/tables/atsc_ett.h"
#   include "../../src/tables/atsc_mgt.h"
#   include "../../src/tables/atsc_stt.h"
#   include "../../src/tables/atsc_vct.h"
#else
#   include <dvbpsi/dvbpsi.h>
#   include <dvbpsi/demux.h>
#   include <dvbpsi/psi.h>
#   include <dvbpsi/descriptor.h>
#   include <dvbpsi/pat.h>
#   include <dvbpsi/pmt.h>
#   include <dvbpsi/cat.h>
#   include <dvbpsi/bat.h>
#   include <dvbpsi/eit.h>
#   include <dvbpsi/nit.h>
#   include <dvbpsi/sdt.h>
#   include <dvbpsi/tot.h>
#   include <dvbpsi/rst.h>
#   include <dvbpsi/dr.h>
/*  ATSC PSI Tables */
#   include <dvbpsi/atsc_eit.h>
#   include <dvbpsi/atsc_ett.h>
#   include <dvbpsi/atsc_mgt.h>
#   include <dvbpsi/atsc_stt.h>
#   include <dvbpsi/atsc_vct.h>
#endif

#include "libdvbpsi.h"

/* DVB CUEI Descriptors */
/* SIS support (SCTE 35 2004) */
#ifdef _DVBPSI_DR_8A_H_
#   define TS_USE_DVB_CUEI 1
#   define TS_USE_SCTE_SIS 1
#   ifdef DVBPSI_DIST
#       include "../../src/tables/sis.h"
#   else
#       include <dvbpsi/sis.h>
#   endif
#endif

/*****************************************************************************
 * Data structures
 *****************************************************************************/
typedef struct ts_pid_s
{
    /* TS header fields */
    int         i_pid;
    int         i_cc;   /* countinuity counter */

    bool        b_seen;

    /* flags */
    bool        b_transport_error_indicator;
    bool        b_payload_unit_start_indicator;
    bool        b_transport_priority;
    uint8_t     i_transport_scrambling_control;

    /* adaptation field: indicators and flags */
    bool        b_adaptation_field;
    bool        b_discontinuity_indicator;
    bool        b_random_access_indicator;
    bool        b_elementary_stream_priority_indicator;
    bool        b_transport_private_data;
    bool        b_splicing_point;
    int8_t      i_splice_countdown;
    uint8_t     i_splice_type;
    uint8_t     i_transport_private_data_length;

    bool        b_opcr;
    bool        b_pcr;  /* this PID is the PCR_PID */
    mtime_t     i_pcr;  /* last know PCR value */

    /* adaptation field extension */
    bool        b_adaptation_field_extension;
    uint32_t    i_adaptation_field_extension_length;
    bool        b_ltw;
    bool        b_ltw_valid;
    uint16_t    i_ltw_offset;
    bool        b_piecewise_rate;
    uint32_t    i_piecewise_rate;
    bool        b_seamless_splice;

    /* statistics */
    uint64_t    i_packets;    /* number of packets for this pid */
    mtime_t     i_first_pcr;  /* first pcr seen for this pid */
    mtime_t     i_prev_pcr;   /* previous pcr seen for this pid */
    mtime_t     i_last_pcr;   /* last pcr seen for this pid */
    mtime_t     i_prev_received; /* capture time of previous packet for this pid */
    mtime_t     i_received;   /* last capture time for packet of this pid */
} ts_pid_t;

typedef struct
{
    dvbpsi_t    *handle;

    int         i_pat_version;
    int         i_ts_id;

    ts_pid_t    *pid;
} ts_pat_t;

typedef struct ts_pmt_s ts_pmt_t;
struct ts_pmt_s
{
    dvbpsi_t    *handle;

    int         i_number; /* i_number = 0 is actually a NIT */
    int         i_pmt_version;
    ts_pid_t    *pid_pmt;
    ts_pid_t    *pid_pcr;

    ts_pmt_t    *p_next;
};

typedef struct ts_cat_s
{
    dvbpsi_t    *handle;

    uint8_t     i_version;

    /* */
    ts_pid_t    *pid;
} ts_cat_t;

#ifdef TS_USE_SCTE_SIS
typedef struct ts_sis_s
{
    uint8_t       i_protocol_version;

    /* */
} ts_sis_t;
#endif

typedef struct ts_sdt_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_sdt_t;

typedef struct ts_rst_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_rst_t;

typedef struct ts_eit_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_eit_t;

typedef struct ts_tdt_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_tdt_t;

typedef struct ts_atsc_s
{
    dvbpsi_t    *handle;
    ts_pid_t    *pid;
} ts_atsc_t;

typedef struct ts_atsc_eit_s ts_atsc_eit_t;
struct ts_atsc_eit_s
{
    dvbpsi_t    *handle;

    int         i_table_pid;
    int         i_mgt_version;
    ts_pid_t    *pid;

    ts_atsc_eit_t *p_next;
};

struct ts_stream_t
{
    /* Program Association Table */
    ts_pat_t    pat;

    /* Program Map Table */
    int         i_pmt;
    ts_pmt_t    *pmt;

    /* Conditional Access Table */
    ts_cat_t    cat;

#ifdef TS_USE_SCTE_SIS
    /* Splice Information Section */
    ts_sis_t    sis;
#endif

    ts_rst_t    rst;

    /* Subbtables */
    ts_sdt_t    sdt;
    ts_eit_t    eit;
    ts_tdt_t    tdt;

    /* Atsc tables */
    ts_atsc_t   atsc;
    ts_atsc_eit_t *atsc_eit;
    int         i_atsc_eit;

    /* pid */
    ts_pid_t    pid[8192];

    enum dvbpsi_msg_level level;

    /* statistics */
    uint64_t    i_packets;
    uint64_t    i_null_packets;
    uint64_t    i_lost_bytes;

    /* logging */
    ts_stream_log_cb pf_log;
    void *cb_data;
};

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/

static void handle_PMT(void* p_data, dvbpsi_pmt_t* p_pmt);
static void handle_CAT(void* p_data, dvbpsi_cat_t* p_cat);
static void handle_SDT(void* p_data, dvbpsi_sdt_t* p_sdt);
static void handle_TOT(void* p_data, dvbpsi_tot_t* p_tot);
static void handle_EIT(void* p_data, dvbpsi_eit_t* p_eit);
static void handle_NIT(void* p_data, dvbpsi_nit_t* p_nit);
static void handle_BAT(void* p_data, dvbpsi_bat_t* p_bat);
static void handle_RST(void* p_data, dvbpsi_rst_t* p_rst);
#ifdef TS_USE_SCTE_SIS
static void handle_SIS(void* p_data, dvbpsi_sis_t* p_sis);
#endif
static void handle_atsc_VCT(void* p_data, dvbpsi_atsc_vct_t *p_vct);
static void handle_atsc_MGT(void *p_data, dvbpsi_atsc_mgt_t *p_mgt);
static void handle_atsc_EIT(void *p_data, dvbpsi_atsc_eit_t *p_eit);
static void handle_atsc_ETT(void* p_data, dvbpsi_atsc_ett_t *p_ett);
static void handle_atsc_STT(void* p_data, dvbpsi_atsc_stt_t *p_stt);
static const char *AACProfileToString(dvbpsi_aac_profile_and_level_t profile);

/*****************************************************************************
 * mdate: current time in milliseconds
 *****************************************************************************/
mtime_t mdate(void)
{
#if defined(HAVE_SYS_TIME_H)
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0)
    {
        fprintf(stderr, "gettimeofday() error: %s\n", strerror(errno));
	/* coverity [+kill} */
        abort();
    }

    return (tv.tv_sec * (mtime_t)1000) + (tv.tv_usec / (mtime_t)1000);
#else
    return -1;
#endif
}

/*****************************************************************************
 * libdvbpsi message callback functions
 *****************************************************************************/
static void dvbpsi_message(dvbpsi_t *p_dvbpsi, const dvbpsi_msg_level_t level, const char* msg)
{
    /* See dvbinfo.h for the definition of these log levels.*/
    int code = 0;
    const char *psz_level;

    switch(level)
    {
        case DVBPSI_MSG_ERROR: code = 0; psz_level = "Error: "; break;
        case DVBPSI_MSG_WARN:  code = 1; psz_level = "Warning: "; break;
        case DVBPSI_MSG_DEBUG: code = 3; psz_level = "Debug: "; break;
        default: /* do nothing */
            return;
    }

    ts_stream_t *stream = (ts_stream_t *)p_dvbpsi->p_sys;
    if (stream && stream->pf_log)
    {
        stream->pf_log(stream->cb_data, code, msg);
    }
    else
    {
        char *reply = NULL;
        if (asprintf(&reply,"%s%s\n", psz_level, msg) > 0)
        {
            fprintf(stderr, "%s", reply);
            free(reply);
        }
    }
}

/*****************************************************************************
 * Dump TS packet as hex
 *****************************************************************************/
static void ts_hexdump(FILE *fd, const uint8_t * const data, const uint32_t length)
{
    uint32_t i;
    fprintf(fd, "\t");
    for (i=0; i < length; i++)
    {
        if ((i%8) == 0) fprintf(fd, " ");
        if ((i%16) == 0) fprintf(fd, "\n\t %.4x: ", i);
            fprintf(fd, "%.2x ", data[i]);
    }
}

static void ts_header_dump(FILE *fd, ts_pid_t *ts)
{
    fprintf(fd, "\n\tPID 0x%x seen %s\n",
           ts->i_pid, ts->b_seen ? "yes" : "no");
    fprintf(fd, "\tContinuity counter: %d\n", ts->i_cc);
    fprintf(fd, "\tTransport Error indicator: %s\n",
           ts->b_transport_error_indicator ? "yes" : "no");
    fprintf(fd, "\tPayload unit start indicator: %s\n",
           ts->b_payload_unit_start_indicator ? "yes" : "no");
    fprintf(fd, "\tScrambling control: %s\n",
           (ts->i_transport_scrambling_control != 0x0) ? "yes" : "no");
    if (ts->i_transport_scrambling_control > 0x0)
        fprintf(fd, "\tScrambling control word: 0x%x\n", ts->i_transport_scrambling_control);
    fprintf(fd, "\tAdaptation field control: %s\n",
           ts->b_adaptation_field ? "yes" : "no");
    if (ts->b_adaptation_field)
    {
        fprintf(fd, "\tDiscontinuity indicator: %s\n",
           ts->b_discontinuity_indicator ? "yes" : "no");
        fprintf(fd, "\tRandom access indicator: %s\n",
           ts->b_random_access_indicator ? "yes" : "no");
        fprintf(fd, "\tElementary stream priority indicator: %s\n",
           ts->b_elementary_stream_priority_indicator ? "yes" : "no");
        fprintf(fd, "\tTransport private data: %s\n",
           ts->b_transport_private_data ? "yes" : "no");
        if (ts->b_transport_private_data )
            fprintf(fd, "\tTransport private data length: %d\n",
                ts->i_transport_private_data_length);
        fprintf(fd, "\tSplicing point: %s\n",
           ts->b_splicing_point ? "yes" : "no");
        if (ts->b_splicing_point)
            fprintf(fd, "\tSplice countdown: %d (0x%x)\n",
                ts->i_splice_countdown, ts->i_splice_countdown);

        fprintf(fd, "\tOriginal PCR: %s\n", ts->b_opcr ? "yes" : "no");
        fprintf(fd, "\tPCR PID: %s\n", ts->b_pcr ? "yes" : "no");
        if (ts->b_pcr)
            fprintf(fd, "\tPCR: %"PRId64"\n", ts->i_pcr);

        /* adaptation field extension */
        if (ts->b_adaptation_field_extension &&
            ts->i_adaptation_field_extension_length > 0)
        {
            fprintf(fd, "\tadaptation field extension, length: %d\n",
               ts->i_adaptation_field_extension_length);
            fprintf(fd, "\tlegal time window (ltw): %s\n", ts->b_ltw ? "yes" : "no");
            fprintf(fd, "\tltw valid: %s\n", ts->b_ltw_valid ? "yes" : "no");
            if (ts->b_ltw)
                fprintf(fd, "\tlegal time window offset: %d\n", ts->i_ltw_offset);
            fprintf(fd, "\tpiecewise rate: %s\n", ts->b_piecewise_rate ? "yes" : "no");
            if (ts->b_piecewise_rate)
                fprintf(fd, "\tpiecewise rate: %d\n", ts->i_piecewise_rate);
            fprintf(fd, "\tseamless splice: %s\n",
               ts->b_seamless_splice ? "yes" : "no");
            if (ts->b_seamless_splice)
            {
                /* FIXME: this is only one of the definitions of splice_type
                 * tables. They depend on profiles, but the exact link with the
                 * rest of the TS headers is not clear to me at this point.
                 */
                const char *descr;
                switch(ts->i_splice_type)
                {
                case 0x00:
                    descr = "splice_decoding_delay = 120 ms; max_splice_rate = 15.0 × 106 bit/s";
                    break;
                case 0x01:
                    descr = "splice_decoding_delay = 150 ms; max_splice_rate = 12.0 × 106 bit/s";
                    break;
                case 0x02:
                    descr = "splice_decoding_delay = 225 ms; max_splice_rate = 8.0 × 106 bit/s";
                    break;
                case 0x03:
                    descr = "splice_decoding_delay = 250 ms; max_splice_rate = 7.2 × 106 bit/s";
                    break;
                default:
                    /* 0100-1011  Reserved */
                    /* 1100-1111  User-defined */
                    descr = "Reserved/User-defined";
                    break;
                }
                fprintf(fd, "\tsplice type 0x%x (%s)\n", ts->i_splice_type, descr);
            }
        }
    }
}

static void ts_dump_packet_details(FILE *fd, ts_stream_t *stream, const uint8_t *data, const uint16_t i_pid)
{
    fprintf(fd, "\n\t---------------------------------------------------------\n");
    fprintf(fd, "\tTS Packet number %"PRId64", ES number %"PRId64", pid %d (0x%x)\n",
       stream->i_packets, stream->pid[i_pid].i_packets, i_pid, i_pid);
#if defined(HAVE_SYS_TIME_H)
    fprintf(fd, "\tReceived time: %"PRId64" ms\n", stream->pid[i_pid].i_received);
#endif
    ts_header_dump(fd, &stream->pid[i_pid]);
    ts_hexdump(fd, data, 188);
    fprintf(fd, "\n\t---------------------------------------------------------\n");
}

/*****************************************************************************
 * Summary: Bandwidth, Packet, Table
 *****************************************************************************/
static void summary(FILE *fd, ts_stream_t *stream)
{
    uint64_t i_packets = 0;
    mtime_t i_first_pcr = 0, i_last_pcr = 0;
    mtime_t start = 0, end = 0;

    fprintf(fd, "\n---------------------------------------------------------\n");
    fprintf(fd, "\nSummary: Bandwidth\n");

    /* Find PCR PID and get pcr timestamps */
    for (int i_pid = 0; i_pid < 8192; i_pid++)
    {
        if (stream->pid[i_pid].b_pcr)
        {
            start = stream->pid[i_pid].i_first_pcr;
            end = stream->pid[i_pid].i_last_pcr;
            if (stream->pid[i_pid].b_discontinuity_indicator)
            {
                fprintf(fd, "PCR discontinuity was signalled for PID: %4d (0x%4x)\n",
                       i_pid, i_pid);
            }
        }
    }

    for (int i_pid = 0; i_pid < 8192; i_pid++)
    {
        if (stream->pid[i_pid].b_seen)
        {
            fprintf(fd, "Found PID: %4d (0x%4x), DRM: %s,", i_pid, i_pid,
                   (stream->pid[i_pid].i_transport_scrambling_control != 0x00) ? "yes" : " no" );

            double bitrate = 0;
            if ((end - start) > 0)
            {
                bitrate = (double) (stream->pid[i_pid].i_packets * 188 * 8) /
                                    ((double)(end - start)/1000.0);
            }
            fprintf(fd, " bitrate %0.4f kbit/s,", bitrate);
            fprintf(fd, " seen %"PRId64" packets",
                   stream->pid[i_pid].i_packets);
            fprintf(fd, "\n");

            i_packets += stream->pid[i_pid].i_packets;
            if (i_first_pcr == 0)
                i_first_pcr = start;
            else
                i_first_pcr = (i_first_pcr < start) ? i_first_pcr : start;
            i_last_pcr = (i_last_pcr > end) ? i_last_pcr : end;
        }
    }
    double total_bitrate = (double)(((i_packets*188) + stream->i_lost_bytes) * 8)/((double)(i_last_pcr - i_first_pcr)/1000.0);
    fprintf(fd, "\nTotal bitrate %0.4f kbits/s\n", total_bitrate);

    fprintf(fd, "Number of packets: %"PRId64", stuffing %"PRId64" packets, lost %"PRId64" bytes\n",
            i_packets, stream->i_null_packets, stream->i_lost_bytes);
    fprintf(fd, "PCR first: %"PRId64", last: %"PRId64", duration: %"PRId64"\n",
            i_first_pcr, i_last_pcr, (mtime_t)(i_last_pcr - i_first_pcr));
    fprintf(fd, "\n---------------------------------------------------------\n");
}

static void summary_table(FILE *fd, ts_stream_t *stream)
{
    fprintf(fd, "\n---------------------------------------------------------\n");
    fprintf(fd, "\nSummary: Table\n");

    fprintf(fd, "\nTable: PAT\n");
    if (stream->pat.handle)
        ts_header_dump(fd, stream->pat.pid);
    fprintf(fd, "\nTable: PMT\n");
    ts_pmt_t *p_pmt = stream->pmt;
    while (p_pmt)
    {
        if (p_pmt->handle)
            ts_header_dump(fd, p_pmt->pid_pmt);
        p_pmt = p_pmt->p_next;
    }
    fprintf(fd, "\nTable: CAT\n");
    if (stream->cat.handle)
        ts_header_dump(fd, stream->cat.pid );
    fprintf(fd, "\nTable: SDT\n");
    if (stream->sdt.handle)
        ts_header_dump(fd, stream->sdt.pid);
    fprintf(fd, "\nTable: EIT\n");
    if (stream->eit.handle)
        ts_header_dump(fd, stream->eit.pid);
    fprintf(fd, "\nTable: TDT\n");
    if (stream->tdt.handle)
        ts_header_dump(fd, stream->tdt.pid);

    fprintf(fd, "\n---------------------------------------------------------\n");
}

static void summary_packet(FILE *fd, ts_stream_t *stream)
{
    fprintf(fd, "\n---------------------------------------------------------\n");
    fprintf(fd, "\nSummary: Packet\n");

    /* Find PCR PID and get pcr timestamps */
    for (int i_pid = 0; i_pid < 8192; i_pid++)
    {
        ts_pid_t *ts = &stream->pid[i_pid];
        ts_header_dump(fd, ts);
    }

    fprintf(fd, "\n---------------------------------------------------------\n");
}

/*****************************************************************************
 * handle_subtable
 *****************************************************************************/
static void handle_subtable(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                            void *p_data)
{
    switch (i_table_id)
    {
#if 0 /* already handled */
        case 0x00: // PAT
        case 0x01: // CAT
        case 0x02: // program_map_section
        case 0x03: // transport_stream_description_section
        case 0x04: /* ISO_IEC_14496_scene_description_section */
        case 0x05: /* ISO_IEC_14496_object_descriptor_section */
        case 0x06: /* Metadata_section */
        case 0x07: /* IPMP_Control_Information_section (defined in ISO/IEC 13818-11) */
        /* 0x08-0x3F: ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved */
#endif
        case 0x40: // NIT network_information_section - actual_network
        case 0x41: // NIT network_information_section - other_network
            if (!dvbpsi_nit_attach(p_dvbpsi, i_table_id, i_extension, handle_NIT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach NIT subdecoder\n");
            break;
        case 0x42:
            if (!dvbpsi_sdt_attach(p_dvbpsi, i_table_id, i_extension, handle_SDT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach SDT subdecoder\n");
            break;
#if 0
        //0x43 to 0x45 reserved for future use
        case 0x46: //      service_description_section - other_transport_stream
        //0x47 to 0x49 reserved for future use
#endif
        case 0x4A: // BAT bouquet_association_section
            if (!dvbpsi_bat_attach(p_dvbpsi, i_table_id, i_extension, handle_BAT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach BAT subdecoder\n");
            break;
        //0x4B to 0x4D reserved for future use
        case 0x4E: // EIT event_information_section - actual_transport_stream, present/following
        case 0x4F: // EIT event_information_section - other_transport_stream, present/following
        //0x50 to 0x5F event_information_section - actual_transport_stream, schedule
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        //0x60 to 0x6F event_information_section - other_transport_stream, schedule
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
            if (!dvbpsi_eit_attach(p_dvbpsi, i_table_id, i_extension, handle_EIT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach EIT subdecoder\n");
            break;
        case 0x70: /* TDT */
        case 0x73: /* TOT only */
            if (!dvbpsi_tot_attach(p_dvbpsi, i_table_id, i_extension, handle_TOT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach TOT subdecoder\n");
            break;
#if 0
        case 0x71: // RST running_status_section
        case 0x72: // ST  stuffing_section
        case 0x74: // application information section (TS 102 812 [15])
        case 0x75: // container section (TS 102 323 [13])
        case 0x76: // related content section (TS 102 323 [13])
        case 0x77: // content identifier section (TS 102 323 [13])
        case 0x78: // MPE-FEC section (EN 301 192 [4])
        case 0x79: // resolution notification section (TS 102 323 [13])
#endif
        /* Handle ATSC PSI tables */
        case 0xC7: /* ATSC MGT */
            if (!dvbpsi_atsc_AttachMGT(p_dvbpsi, i_table_id, i_extension, handle_atsc_MGT, p_data))
                fprintf(stderr, "dvbinfo: Failed to attach ATSC MGT subdecoder\n");
            break;
        case 0xC8:
        case 0xC9: /* ATSC VCT */
            if (!dvbpsi_atsc_AttachVCT(p_dvbpsi, i_table_id, i_extension, handle_atsc_VCT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach ATSC VCT subdecoder\n");
            break;
        case 0xCB: /* ATSC EIT */
            if (!dvbpsi_atsc_AttachEIT(p_dvbpsi, i_table_id, i_extension, handle_atsc_EIT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach ATSC EIT subdecoder\n");
            break;
        case 0xCC: /* ATSC ETT */
            if (!dvbpsi_atsc_AttachETT(p_dvbpsi, i_table_id, i_extension, handle_atsc_ETT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach ATSC ETT subdecoder\n");
            break;
        case 0xCD: /* ATSC STT */
            if (!dvbpsi_atsc_AttachSTT(p_dvbpsi, i_table_id, i_extension, handle_atsc_STT, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach ATSC STT subdecoder\n");
            break;
#ifdef TS_USE_SCTE_SIS
        case 0xFC:
            if (!dvbpsi_sis_attach(p_dvbpsi, i_table_id, i_extension, handle_SIS, p_data))
                    fprintf(stderr, "dvbinfo: Failed to attach SIS subdecoder\n");
            break;
#endif
    }
}

/*****************************************************************************
 * handle_PAT
 *****************************************************************************/
static void handle_PAT(void* p_data, dvbpsi_pat_t* p_pat)
{
    dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
    ts_stream_t* p_stream = (ts_stream_t*) p_data;

    p_stream->pat.i_pat_version = p_pat->i_version;
    p_stream->pat.i_ts_id = p_pat->i_ts_id;

    printf("\n");
    printf("  PAT: Program Association Table\n");
    printf("\tTransport stream id : %d\n", p_pat->i_ts_id);
    printf("\tVersion number : %d\n", p_pat->i_version);
    printf("\tCurrent next   : %s\n", p_pat->b_current_next ? "yes" : "no");
    if (p_stream->pat.pid->i_prev_received > 0)
        printf("\tLast received  : %"PRId64" ms ago\n",
               (mtime_t)(p_stream->pat.pid->i_received - p_stream->pat.pid->i_prev_received));
    printf("\t\t| program_number @ [NIT|PMT]_PID\n");
    while (p_program)
    {
        /* Attach new PMT decoder */
        ts_pmt_t *p_pmt = calloc(1, sizeof(ts_pmt_t));
        if (p_pmt)
        {
            /* PMT */
            p_pmt->handle = dvbpsi_new(&dvbpsi_message, p_stream->level);
            if (p_pmt->handle == NULL)
            {
                fprintf(stderr, "dvbinfo: Failed attach new PMT decoder\n");
                free(p_pmt);
                break;
            }

            p_pmt->i_number = p_program->i_number;
            p_pmt->pid_pmt = &p_stream->pid[p_program->i_pid];
            p_pmt->pid_pmt->i_pid = p_program->i_pid;
            p_pmt->p_next = NULL;

            if (!dvbpsi_pmt_attach(p_pmt->handle, p_program->i_number, handle_PMT, p_stream))
            {
                 fprintf(stderr, "dvbinfo: Failed to attach new pmt decoder\n");
                 dvbpsi_delete(p_pmt->handle);
                 free(p_pmt);
                 break;
            }

            /* insert at start of list */
            p_pmt->p_next = p_stream->pmt;
            p_stream->pmt = p_pmt;
            p_stream->i_pmt++;
            assert(p_stream->pmt);
        }
        else
            fprintf(stderr, "dvbinfo: Failed create new PMT decoder\n");

        printf("\t\t| %14d @ pid: 0x%x (%d)\n",
                p_program->i_number, p_program->i_pid, p_program->i_pid);
        p_program = p_program->p_next;
    }
    printf("\tActive         : %s\n", p_pat->b_current_next ? "yes" : "no");
    dvbpsi_pat_delete(p_pat);
}

/*****************************************************************************
 * GetTypeName of PMT stream_type
 *****************************************************************************/
static char const* GetTypeName(uint8_t type)
{
    switch (type)
    {
    case 0x00: return "ITU-T | ISO/IEC Reserved";
    case 0x01: return "ISO/IEC 11172-2 Video";
    case 0x02: return "ITU-T Rec H.262 | ISO/IEC 13818-2 Video stream descriptor or ISO/IEC 11172-2 constrained parameter video stream";
    case 0x03: return "ISO/IEC 11172-3 Audio stream descriptor";
    case 0x04: return "ISO/IEC 13818-3 Audio MPEG Audio layer 1/2";
    case 0x05: return "ITU-T Rec H.222.0 | ISO/IEC 13818-1 Private Section: Registration descriptor";
    case 0x06: return "ITU-T Rec H.222.0 | ISO/IEC 13818-1 Private PES data packets";
    case 0x07: return "ISO/IEC 13522 MHEG";
    case 0x08: return "ITU-T Rec H.222.0 | ISO/IEC 13818-1 Annex A DSM CC";
    case 0x09: return "ITU-T Rec H222.1";
    case 0x0a: return "ISO/IEC 13818-6 type A";
    case 0x0b: return "ISO/IEC 13818-6 type B";
    case 0x0c: return "ISO/IEC 13818-6 type C";
    case 0x0d: return "ISO/IEC 13818-6 type D";
    case 0x0e: return "ITU-T Rec H.222.0 | ISO/IEC 13818-1 auxillary";

    case 0x0f: return "ISO/IEC 13818-7 MPEG2 Audio with ADTS transport syntax";
    case 0x10: return "ISO/IEC 14496-2 Visual";
    case 0x11: return "ISO/IEC 14496-3 MPEG4 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3";
    case 0x12: return "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in PES packets";
    case 0x13: return "ISO/IEC 14496-1 SL-packetized stream or FlexMux stream carried in ISO/IEC 14496_sections";
    case 0x14: return "ISO/IEC 13818-6 Synchronized Download Protocol";
    case 0x15: return "Metadata carried in PES packets";
    case 0x16: return "Metadata carried in metadata_sections";
    case 0x17: return "Metadata carried in ISO/IEC 13818-6 Data Carousel";
    case 0x18: return "Metadata carried in ISO/IEC 13818-6 Object Carousel";
    case 0x19: return "Metadata carried in ISO/IEC 13818-6 Synchronized Download Protocol";
    case 0x1A: return "IPMP stream (defined in ISO/IEC 13818-11, MPEG-2 IPMP)";
    case 0x1B: return "AVC video stream as defined in ITU-T Rec. H.264 | ISO/IEC 14496-10 Video";
    case 0x7F: return "IPMP stream";
    default:
        if ((type >= 0x1C) && (type <= 0x7E))
            return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved";
        else if (type >= 0x80) /* 0x80 - 0xFF */
            return "User Private";
        else
            return "Unknown";
        break;
    };
}

/*****************************************************************************
 * GetDescriptorName:
 *****************************************************************************/
static char const* GetDescriptorName(uint8_t tag)
{
    switch (tag)
    {
    case 0x00:
    case 0x01: return "Reserved";
    case 0x02: return "Video stream descriptor";
    case 0x03: return "Audio stream descriptor";
    case 0x04: return "Hierarchy descriptor";
    case 0x05: return "Registration descriptor";
    case 0x06: return "Data stream alignment descriptor";
    case 0x07: return "Target background grid descriptor";
    case 0x08: return "Video window descriptor";
    case 0x09: return "CA descriptor";
    case 0x0a: return "ISO 639 language descriptor";
    case 0x0b: return "System clock descriptor";
    case 0x0c: return "Multiplex buffer utilization descriptor";
    case 0x0d: return "Copyright descriptor";
    case 0x0e: return "Maximum bitrate descriptor";
    case 0x0f: return "Private data indicator descriptor";
    case 0x10: return "Smoothing buffer descriptor";
    case 0x11: return "STD descriptor";
    case 0x12: return "IBP descriptor";
    // case 0x13..0x1a: return "Defined in ISO/IEC 13818-6";
    case 0x1b: return "MPEG-4 video descriptor";
    case 0x1c: return "MPEG-4 audio descriptor";
    case 0x1d: return "IOD descriptor";
    case 0x1e: return "SL descriptor";
    case 0x1f: return "FMC descriptor";
    case 0x20: return "External ES ID descriptor";
    case 0x21: return "Mux Code descriptor";
    case 0x22: return "Fmx Buffer Size descriptor";
    case 0x23: return "Multiplex buffer descriptor";
    case 0x24: return "Content labeling descriptor";
    case 0x25: return "Metadata pointer descriptor";
    case 0x26: return "Metadata descriptor";
    case 0x27: return "Metadata STD descriptor";
    case 0x28: return "AVC video descriptor";
    case 0x29: return "IPMP descriptor (defined in ISO/IEC 13818-11; break; MPEG-2 IPMP)";
    case 0x2a: return "AVC timing and HRD descriptor";
    case 0x2b: return "MPEG-2 AAC audio descriptor";
    case 0x2c: return "FlexMuxTiming descriptor";
    // case 0x2d..0x3f: return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved";
    case 0x40: return "User Private | Network Name";
    case 0x41: return "User Private | Service List";
    case 0x42: return "User Private | Stuffing";
    case 0x43: return "Satellite Delivery System descriptor";
    case 0x44: return "Cable Delivery System descriptor";
    case 0x45: /*69*/ return "VBI Data descriptor";
    case 0x46: /*70*/ return "VBI Teletext descriptor";
    case 0x47: return "User Private | Bouquet Name";
    case 0x48: return "User Private | Service";
    case 0x49: return "User Private | Country Availability";
    case 0x4a: return "User Private | Linkage";
    case 0x4b: return "User Private | NVOD Reference";
    case 0x4c: return "User Private | Timeshifted Service";
    case 0x4d: return "User Private | ShortEvent";
    case 0x4e: return "User Private | Extended Event";
    case 0x4f: return "User Private | Timeshifted Event";
    case 0x50: return "User Private | Component";
    case 0x51: return "User Private | Mosaic";
    case 0x52: /*82*/ return "User Private | Stream Component Identifier";
    case 0x53: return "User Private | CA Identifier";
    case 0x54: return "User Private | Content";
    case 0x55: return "User Private | Parental Rating";
    case 0x56: /*86*/ return "EBU Teletext";
    case 0x57: return "Telephone";
    case 0x58: return "Local Time Offset";
    case 0x59: return "Subtitling";
    case 0x5A: return "Terrestrial Delivery System";
    case 0x5B: return "Multilingual Network Name";
    case 0x5C: return "Multilingual Bouquet Name";
    case 0x5D: return "Multilingual Service Name";
    case 0x5E: return "Multilingual Component";
    case 0x5F: return "Private Data Specifier";
    case 0x60: return "Service Move";
    case 0x61: return "Short Smoothing Buffer";
    case 0x62: return "Frequency List";
    case 0x63: return "Partial Transport Stream";
    case 0x64: return "Data Broadcast";
    case 0x65: return "CA System descriptor";
    case 0x66: return "Data Broadcast Identifier";
    case 0x67: return "Transport Stream Identifier";
    case 0x68: return "DSNG descriptor";
    case 0x69: return "PDC descriptor";
    case 0x6A: return "AC3 audio descriptor";
    case 0x6B: return "Ancilliary Data descriptor";
    case 0x6C: return "Cell List";
    case 0x6D: return "Cell Frequency Link";
    case 0x6E: return "Announcement Support";
    case 0x6F: return "Application Signalling";
    case 0x70: return "Adaptation Field";
    case 0x71: return "Service Identifier";
    case 0x72: return "Service Availability";
    case 0x73: return "Default Authority";
    case 0x74: return "Related Content";
    case 0x75: return "TVA ID";
    case 0x76: return "Content Identifier";
    case 0x77: return "Time Slice FEC Identifier";
    case 0x78: return "ECM Repeater Rate";
    case 0x79: return "DVB S2 Satellite Delivery System descriptor";
    case 0x7A: return "Enhanced AC3 audio descriptor";
    case 0x7B: return "DTS audio descriptor";
    case 0x7C: return "AAC audio descriptor";
    // case 0x7d..0xff: return "User Private";
    case 0x8a: return "CUE Identifier descriptor";
    default:
        if (tag >= 0x13 && tag <= 0x1a) {
            /*19-26*/ return "Defined in ISO/IEC 13818-6";
        } else if (tag >= 0x2d && tag <= 0x3f) {
            /*45-63*/ return "ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved";
        } else if ((tag >= 0x40 && tag <= 0x51) ||
                   (tag >= 0x53 && tag <= 0x55)) {
            /*64-81 and 83-84*/ return "User Private";
        }
        else
            /*87-255*/ return "User Private";
        break;
    };
}

/*****************************************************************************
 * DumpMaxBitrateDescriptor
 *****************************************************************************/
static void DumpMaxBitrateDescriptor(const void *p_descriptor)
{
    const dvbpsi_max_bitrate_dr_t* bitrate_descriptor = p_descriptor;
    printf("Bitrate: %d\n", bitrate_descriptor->i_max_bitrate);
}

/*****************************************************************************
 * DumpSmoothingBufferDescriptor
 *****************************************************************************/
static void DumpSmoothingBufferDescriptor(const void *p_descriptor)
{
    const dvbpsi_smoothing_buffer_dr_t *smoothing_descriptor = p_descriptor;
    printf("Leak rate: %d \n", smoothing_descriptor->i_sb_leak_rate);
    printf("\t\tSize: %d \n", smoothing_descriptor->i_sb_size);
}

/*****************************************************************************
 * DumpSTDDescriptor
 *****************************************************************************/
static void DumpSTDDescriptor(const void *p_descriptor)
{
    const dvbpsi_std_dr_t* std_descriptor = p_descriptor;
    printf("Leak valid flag: %d\n", std_descriptor->b_leak_valid_flag);
}

/*****************************************************************************
 * DumpIBPDescriptor
 *****************************************************************************/
static void DumpIBPDescriptor(const void *p_descriptor)
{
    const dvbpsi_ibp_dr_t *ibp_descriptor = p_descriptor;
    printf("Closed GOP flag: %d \n", ibp_descriptor->b_closed_gop_flag);
    printf("\t\tIdentical GOP flag: %d \n", ibp_descriptor->b_identical_gop_flag);
    printf("\t\tMax GOP length: %" PRIu16 " \n", ibp_descriptor->i_max_gop_length);
}

static const char* MPEG4VideoProfileToString(dvbpsi_mpeg4_visual_profile_and_level_t profile)
{
    switch(profile)
    {
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_L1: return "Simple Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_L2: return "Simple Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_L3: return "Simple Profile/Level 3";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_SCALABLE_L1: return "Simple Scalable Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_SCALABLE_L2: return "Simple Scalable Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_CORE_L1: return "Core Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_CORE_L2: return "Core Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_MAIN_L2: return "Main Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_MAIN_L3: return "Main Profile/Level 3";
        case DVBPSI_MPEG4V_PROFILE_MAIN_L4: return "Main Profile/Level 4";
        case DVBPSI_MPEG4V_PROFILE_N_BIT_L2: return "N-bit Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_SCALABLE_TEXTURE_L1: return "Scalable Texture Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_FACE_ANIMATION_L1: return "Simple Face Animation Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_FACE_ANIMATION_L2: return "Simple Face Animation Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_FBA_L1: return "Simple FBA Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_SIMPLE_FBA_L2: return "Simple FBA Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_BASIC_ANIMATED_TEXTURE_L1: return "Basic Animated Texture Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_BASIC_ANIMATED_TEXTURE_L2: return "Basic Animated Texture Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_HYBRID_L1: return "Hybrid Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_HYBRID_L2: return "Hybrid Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L1: return "Advanced Real Time Simple Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L2: return "Advanced Real Time Simple Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L3: return "Advanced Real Time Simple Profile/Level 3";
        case DVBPSI_MPEG4V_PROFILE_ADV_REAL_TIME_SIMPLE_L4: return "Advanced Real Time Simple Profile/Level 4";
        case DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L1: return "Core Scalable Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L2: return "Core Scalable Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_CORE_SCALABLE_L3: return "Core Scalable Profile/Level 3";
        case DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L1: return "Advanced Coding Efficiency Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L2: return "Advanced Coding Efficiency Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L3: return "Advanced Coding Efficiency Profile/Level 3";
        case DVBPSI_MPEG4V_PROFILE_ADV_CODING_EFF_L4: return "Advanced Coding Efficiency Profile/Level 4";
        case DVBPSI_MPEG4V_PROFILE_ADV_CORE_L1: return "Advanced Core Profile/Level 1";
        case DVBPSI_MPEG4V_PROFILE_ADV_CORE_L2: return "Advanced Core Profile/Level 2";
        case DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L1: return "Advanced Scalable Texture/Level 1";
        case DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L2: return "Advanced Scalable Texture/Level 2";
        case DVBPSI_MPEG4V_PROFILE_ADV_SCALABLE_TEXTURE_L3: return "Advanced Scalable Texture/Level 3";

        case DVBPSI_MPEG4V_PROFILE_LAST:
        default:
            return "Reserved";
    }
}

static void DumpMPEG4VideoDescriptor(const void *p_descriptor)
{
    const dvbpsi_mpeg4_video_dr_t *mpeg4_descriptor = p_descriptor;
    printf("MPEG-4 Video Profile and Level : %s (0x%02x) \n",
        MPEG4VideoProfileToString(mpeg4_descriptor->i_mpeg4_visual_profile_and_level),
        mpeg4_descriptor->i_mpeg4_visual_profile_and_level);
}

static void DumpMPEG4AudioDescriptor(const void *p_descriptor)
{
    const dvbpsi_mpeg4_audio_dr_t *mpeg4_descriptor = p_descriptor;
    printf("MPEG-4 Audio Profile and Level : %s (0x%02x) \n",
        AACProfileToString(mpeg4_descriptor->i_mpeg4_audio_profile_and_level),
        mpeg4_descriptor->i_mpeg4_audio_profile_and_level);
}

/*****************************************************************************
 * DumpSystemClockDescriptor
 *****************************************************************************/
static void DumpSystemClockDescriptor(const void *p_descriptor)
{
    const dvbpsi_system_clock_dr_t* p_clock_descriptor = p_descriptor;
    printf("External clock: %s, Accuracy: %E\n",
    p_clock_descriptor->b_external_clock_ref ? "Yes" : "No",
    p_clock_descriptor->i_clock_accuracy_integer *
    pow(10.0, -(double)p_clock_descriptor->i_clock_accuracy_exponent));
}

/*****************************************************************************
 * DumpStreamIdentifierDescriptor
 *****************************************************************************/
static void DumpStreamIdentifierDescriptor(const void *p_descriptor)
{
    const dvbpsi_stream_identifier_dr_t* p_si_descriptor = p_descriptor;
    printf("Component tag: %d\n", p_si_descriptor->i_component_tag);
}

/*****************************************************************************
 * DumpCAIdentifierDescriptor
 *****************************************************************************/
static void DumpCAIdentifierDescriptor(const void *p_descriptor)
{
    const dvbpsi_ca_identifier_dr_t *p_ca_descriptor = p_descriptor;
    printf("CA system id\n");
    for(int i = 0; i < p_ca_descriptor->i_number; i++ )
        printf("\t%d: %d\n", i, p_ca_descriptor->p_system[i].i_ca_system_id);
}

/*****************************************************************************
 * DumpContentDescriptor
 *****************************************************************************/
typedef struct {
    const int i_category;
    const char *p_category;
} dr_content_category_t;

static const char *GetContentSubCategory( const int i_type )
{
    dr_content_category_t content_subcategory[] = {
        /* Movie/Drama */
        { DVBPSI_CONTENT_MOVIE_GENERAL, "General" },
        { DVBPSI_CONTENT_MOVIE_DETECTIVE, "Detective" },
        { DVBPSI_CONTENT_MOVIE_ADVENTURE, "Adventure" },
        { DVBPSI_CONTENT_MOVIE_SF, "Science Fiction" },
        { DVBPSI_CONTENT_MOVIE_COMEDY, "Comedy" },
        { DVBPSI_CONTENT_MOVIE_SOAP, "Soap" },
        { DVBPSI_CONTENT_MOVIE_ROMANCE, "Romance" },
        { DVBPSI_CONTENT_MOVIE_CLASSICAL, "Classical" },
        { DVBPSI_CONTENT_MOVIE_ADULT, "Adult" },
        { DVBPSI_CONTENT_MOVIE_USERDEFINED, "User defined" },
        /* News/Current affairs */
        { DVBPSI_CONTENT_NEWS_GENERAL, "General" },
        { DVBPSI_CONTENT_NEWS_WEATHER, "Weather" },
        { DVBPSI_CONTENT_NEWS_MAGAZINE, "Magazine" },
        { DVBPSI_CONTENT_NEWS_DOCUMENTARY, "Documentary" },
        { DVBPSI_CONTENT_NEWS_DISCUSSION, "Discussion" },
        { DVBPSI_CONTENT_NEWS_USERDEFINED, "User Defined" },
        /* Show/Game show */
        { DVBPSI_CONTENT_SHOW_GENERAL, "General" },
        { DVBPSI_CONTENT_SHOW_QUIZ, "Quiz" },
        { DVBPSI_CONTENT_SHOW_VARIETY, "Variety" },
        { DVBPSI_CONTENT_SHOW_TALK, "Talk" },
        { DVBPSI_CONTENT_SHOW_USERDEFINED, "User Defined" },
        /* Sports */
        { DVBPSI_CONTENT_SPORTS_GENERAL, "General" },
        { DVBPSI_CONTENT_SPORTS_EVENTS, "Events" },
        { DVBPSI_CONTENT_SPORTS_MAGAZINE, "Magazine" },
        { DVBPSI_CONTENT_SPORTS_FOOTBALL, "Football" },
        { DVBPSI_CONTENT_SPORTS_TENNIS, "Tennis" },
        { DVBPSI_CONTENT_SPORTS_TEAM, "Team" },
        { DVBPSI_CONTENT_SPORTS_ATHLETICS, "Athletics" },
        { DVBPSI_CONTENT_SPORTS_MOTOR, "Motor" },
        { DVBPSI_CONTENT_SPORTS_WATER, "Water" },
        { DVBPSI_CONTENT_SPORTS_WINTER, "Winter" },
        { DVBPSI_CONTENT_SPORTS_EQUESTRIAN, "Equestrian" },
        { DVBPSI_CONTENT_SPORTS_MARTIAL, "Martial" },
        { DVBPSI_CONTENT_SPORTS_USERDEFINED, "User Defined" },
        /* Children's/Youth */
        { DVBPSI_CONTENT_CHILDREN_GENERAL, "General" },
        { DVBPSI_CONTENT_CHILDREN_PRESCHOOL, "Preschool" },
        { DVBPSI_CONTENT_CHILDREN_06TO14ENT, "06 to 14" },
        { DVBPSI_CONTENT_CHILDREN_10TO16ENT, "10 to 16" },
        { DVBPSI_CONTENT_CHILDREN_EDUCATIONAL, "Educational" },
        { DVBPSI_CONTENT_CHILDREN_CARTOONS, "Cartoons" },
        { DVBPSI_CONTENT_CHILDREN_USERDEFINED, "User Defined" },
        /* Music/Ballet/Dance */
        { DVBPSI_CONTENT_MUSIC_GENERAL, "General" },
        { DVBPSI_CONTENT_MUSIC_POPROCK, "Poprock" },
        { DVBPSI_CONTENT_MUSIC_CLASSICAL, "Classical" },
        { DVBPSI_CONTENT_MUSIC_FOLK, "Folk" },
        { DVBPSI_CONTENT_MUSIC_JAZZ, "Jazz" },
        { DVBPSI_CONTENT_MUSIC_OPERA, "Opera" },
        { DVBPSI_CONTENT_MUSIC_BALLET, "Ballet" },
        { DVBPSI_CONTENT_MUSIC_USERDEFINED, "User Defined" },
        /* Arts/Culture */
        { DVBPSI_CONTENT_CULTURE_GENERAL, "General" },
        { DVBPSI_CONTENT_CULTURE_PERFORMANCE, "Performance" },
        { DVBPSI_CONTENT_CULTURE_FINEARTS, "Fine Arts" },
        { DVBPSI_CONTENT_CULTURE_RELIGION, "Religion" },
        { DVBPSI_CONTENT_CULTURE_TRADITIONAL, "Traditional" },
        { DVBPSI_CONTENT_CULTURE_LITERATURE, "Literature" },
        { DVBPSI_CONTENT_CULTURE_CINEMA, "Cinema" },
        { DVBPSI_CONTENT_CULTURE_EXPERIMENTAL, "Experimental" },
        { DVBPSI_CONTENT_CULTURE_PRESS, "Press" },
        { DVBPSI_CONTENT_CULTURE_NEWMEDIA, "New Media" },
        { DVBPSI_CONTENT_CULTURE_MAGAZINE, "Magazine" },
        { DVBPSI_CONTENT_CULTURE_FASHION, "Fashion" },
        { DVBPSI_CONTENT_CULTURE_USERDEFINED, "User Defined" },
        /* Socal/Political/Economics */
        { DVBPSI_CONTENT_SOCIAL_GENERAL, "General" },
        { DVBPSI_CONTENT_SOCIAL_MAGAZINE, "Magazine" },
        { DVBPSI_CONTENT_SOCIAL_ADVISORY, "Advisory" },
        { DVBPSI_CONTENT_SOCIAL_PEOPLE, "People" },
        { DVBPSI_CONTENT_SOCIAL_USERDEFINED, "User Defined" },
        /* Eduction/Science/Factual */
        { DVBPSI_CONTENT_EDUCATION_GENERAL, "General" },
        { DVBPSI_CONTENT_EDUCATION_NATURE, "Nature" },
        { DVBPSI_CONTENT_EDUCATION_TECHNOLOGY, "Technology" },
        { DVBPSI_CONTENT_EDUCATION_MEDICINE, "Medicine" },
        { DVBPSI_CONTENT_EDUCATION_FOREIGN, "Foreign" },
        { DVBPSI_CONTENT_EDUCATION_SOCIAL, "Social" },
        { DVBPSI_CONTENT_EDUCATION_FURTHER, "Further" },
        { DVBPSI_CONTENT_EDUCATION_LANGUAGE, "Language" },
        { DVBPSI_CONTENT_EDUCATION_USERDEFINED, "User Defined" },
        /* Leisure/Hobbies */
        { DVBPSI_CONTENT_LEISURE_GENERAL, "General" },
        { DVBPSI_CONTENT_LEISURE_TRAVEL, "Travel" },
        { DVBPSI_CONTENT_LEISURE_HANDICRAFT, "Handicraft" },
        { DVBPSI_CONTENT_LEISURE_MOTORING, "Motoring" },
        { DVBPSI_CONTENT_LEISURE_FITNESS, "Fitness" },
        { DVBPSI_CONTENT_LEISURE_COOKING, "Cooking" },
        { DVBPSI_CONTENT_LEISURE_SHOPPING, "Shopping" },
        { DVBPSI_CONTENT_LEISURE_GARDENING, "Gardening" },
        { DVBPSI_CONTENT_LEISURE_USERDEFINED, "User Defined" },
        /* Special characteristics */
        { DVBPSI_CONTENT_SPECIAL_ORIGINALLANGUAGE, "Original Language" },
        { DVBPSI_CONTENT_SPECIAL_BLACKANDWHITE, "Black and White " },
        { DVBPSI_CONTENT_SPECIAL_UNPUBLISHED, "Unpublished" },
        { DVBPSI_CONTENT_SPECIAL_LIVE, "Live" },
        { DVBPSI_CONTENT_SPECIAL_PLANOSTEREOSCOPIC, "Planostereoscopic" },
        { DVBPSI_CONTENT_SPECIAL_USERDEFINED, "User Defined" },
        { DVBPSI_CONTENT_SPECIAL_USERDEFINED1, "User Defined 1" },
        { DVBPSI_CONTENT_SPECIAL_USERDEFINED2, "User Defined 2" },
        { DVBPSI_CONTENT_SPECIAL_USERDEFINED3, "User Defined 3" },
        { DVBPSI_CONTENT_SPECIAL_USERDEFINED4, "User Defined 4" }
    };

    for (unsigned int i = 0; i < ARRAY_SIZE(content_subcategory); i++)
    {
        if (i_type == content_subcategory[i].i_category)
            return content_subcategory[i].p_category;
    }
    return "Unknown";
}

static void DumpContentDescriptor(const void *p_descriptor)
{
    const dvbpsi_content_dr_t *p_content_descriptor = p_descriptor;
    dr_content_category_t content_category[] = {
        { DVBPSI_CONTENT_CAT_UNDEFINED, "Undefined" },
        { DVBPSI_CONTENT_CAT_MOVIE, "Movie" },
        { DVBPSI_CONTENT_CAT_NEWS, "News" },
        { DVBPSI_CONTENT_CAT_SHOW, "Show" },
        { DVBPSI_CONTENT_CAT_SPORTS, "Sports" },
        { DVBPSI_CONTENT_CAT_CHILDREN, "Children" },
        { DVBPSI_CONTENT_CAT_MUSIC, "Music" },
        { DVBPSI_CONTENT_CAT_CULTURE, "Culture" },
        { DVBPSI_CONTENT_CAT_SOCIAL, "Social" },
        { DVBPSI_CONTENT_CAT_EDUCATION, "Education" },
        { DVBPSI_CONTENT_CAT_LEISURE, "Leisur" },
        { DVBPSI_CONTENT_CAT_SPECIAL, "Special" },
        { 0, NULL }, /* 0xc unknown */
        { 0, NULL }, /* 0xd unknown */
        { 0, NULL }, /* 0xe unknown */
        { DVBPSI_CONTENT_CAT_USERDEFINED, "User defined" },
        { 0, NULL }
    };

    printf("Content\n");
    for(int i = 0; i < p_content_descriptor->i_contents_number; i++)
    {
        int i_type = p_content_descriptor->p_content[i].i_type;
        int i_category = DVBPSI_GetContentCategoryFromType(i_type);

        printf("\t\tcategory: %s\n", content_category[i_category].p_category);
        printf("\t\tsub category: %s\n", GetContentSubCategory(i_type));
        printf("\t\tuser byte: 0x%x\n", p_content_descriptor->p_content[i].i_user_byte);
    }
}

/*****************************************************************************
 * DumpSubtitleDescriptor
 *****************************************************************************/
static void DumpSubtitleDescriptor(const void *p_descriptor)
{
    int a;
    const dvbpsi_subtitling_dr_t* p_subtitle_descriptor = p_descriptor;

    printf("%d subtitles,\n", p_subtitle_descriptor->i_subtitles_number);
    for (a = 0; a < p_subtitle_descriptor->i_subtitles_number; ++a)
    {
        printf("\t\t   | %d - lang: %c%c%c, type: %d, cpid: %d, apid: %d\n", a,
            p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[0],
            p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[1],
            p_subtitle_descriptor->p_subtitle[a].i_iso6392_language_code[2],
            p_subtitle_descriptor->p_subtitle[a].i_subtitling_type,
            p_subtitle_descriptor->p_subtitle[a].i_composition_page_id,
            p_subtitle_descriptor->p_subtitle[a].i_ancillary_page_id);
    }
}

static const char *AACProfileToString(dvbpsi_aac_profile_and_level_t profile)
{
    switch(profile)
    {
    /* 0x00-0x0E Reserved */
    case DVBPSI_AAC_PROFILE_NOT_DEFINED:
        return "No audio profile and level defined for the associated MPEG-4 "
            "audio stream";

    case DVBPSI_AAC_PROFILE_MAIN_LEVEL_1: return "Main profile, level 1";
    case DVBPSI_AAC_PROFILE_MAIN_LEVEL_2: return "Main profile, level 2";
    case DVBPSI_AAC_PROFILE_MAIN_LEVEL_3: return "Main profile, level 3";
    case DVBPSI_AAC_PROFILE_MAIN_LEVEL_4: return "Main profile, level 4";
    /* 0x14-0x17 Reserved */
    case DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_1: return "Scalable Profile, level 1";
    case DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_2: return "Scalable Profile, level 2";
    case DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_3: return "Scalable Profile, level 3";
    case DVBPSI_AAC_PROFILE_SCALABLE_LEVEL_4: return "Scalable Profile, level 4";
    /* 0x1C-0x1F Reserved */
    case DVBPSI_AAC_PROFILE_SPEECH_LEVEL_1: return "Speech profile, level 1";
    case DVBPSI_AAC_PROFILE_SPEECH_LEVEL_2: return "Speech profile, level 2";
    /* 0x22-0x27 Reserved */
    case DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_1: return "Synthesis profile, level 1";
    case DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_2: return "Synthesis profile, level 2";
    case DVBPSI_AAC_PROFILE_SYNTHESIS_LEVEL_3: return "Synthesis profile, level 3";
    /* 0x2B-0x2F Reserved */
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_1: return "High quality audio profile, level 1";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_2: return "High quality audio profile, level 2";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_3: return "High quality audio profile, level 3";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_4: return "High quality audio profile, level 4";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_5: return "High quality audio profile, level 5";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_6: return "High quality audio profile, level 6";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_7: return "High quality audio profile, level 7";
    case DVBPSI_AAC_PROFILE_HQ_LEVEL_8: return "High quality audio profile, level 8";

    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_1: return "Low delay audio profile, level 1";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_2: return "Low delay audio profile, level 2";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_3: return "Low delay audio profile, level 3";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_4: return "Low delay audio profile, level 4";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_5: return "Low delay audio profile, level 5";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_6: return "Low delay audio profile, level 6";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_7: return "Low delay audio profile, level 7";
    case DVBPSI_AAC_PROFILE_LOW_DELAY_LEVEL_8: return "Low delay audio profile, level 8";

    case DVBPSI_AAC_PROFILE_NATURAL_LEVEL_1: return "Natural audio profile, level 1";
    case DVBPSI_AAC_PROFILE_NATURAL_LEVEL_2: return "Natural audio profile, level 2";
    case DVBPSI_AAC_PROFILE_NATURAL_LEVEL_3: return "Natural audio profile, level 3";
    case DVBPSI_AAC_PROFILE_NATURAL_LEVEL_4: return "Natural audio profile, level 4";
    /* 0x44-0x47 Reserved */
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_1: return "Mobile audio internetworking profile, level 1";
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_2: return "Mobile audio internetworking profile, level 2";
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_3: return "Mobile audio internetworking profile, level 3";
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_4: return "Mobile audio internetworking profile, level 4";
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_5: return "Mobile audio internetworking profile, level 5";
    case DVBPSI_AAC_PROFILE_MOBILE_LEVEL_6: return "Mobile audio internetworking profile, level 6";
    /* 0x4E-0x4F Reserved */
    case DVBPSI_AAC_PROFILE_LEVEL_1: return "AAC profile, level 1";
    case DVBPSI_AAC_PROFILE_LEVEL_2: return "AAC profile, level 2";
    case DVBPSI_AAC_PROFILE_LEVEL_4: return "AAC profile, level 4";
    case DVBPSI_AAC_PROFILE_LEVEL_5: return "AAC profile, level 5";
    /* 0x54-0x57 RESERVED */
    case DVBPSI_HE_AAC_PROFILE_LEVEL_2: return "High efficiency AAC profile, level 2";
    case DVBPSI_HE_AAC_PROFILE_LEVEL_3: return "High efficiency AAC profile, level 3";
    case DVBPSI_HE_AAC_PROFILE_LEVEL_4: return "High efficiency AAC profile, level 4";
    case DVBPSI_HE_AAC_PROFILE_LEVEL_5: return "High efficiency AAC profile, level 5";
    /* 0x5C-0x5F RESERVED */
    case DVBPSI_HE_AAC_V2_PROFILE_LEVEL_2: return "High efficiency AAC v2 profile, level 2";
    case DVBPSI_HE_AAC_V2_PROFILE_LEVEL_3: return "High efficiency AAC v2 profile, level 3";
    case DVBPSI_HE_AAC_V2_PROFILE_LEVEL_4: return "High efficiency AAC v2 profile, level 4";
    case DVBPSI_HE_AAC_V2_PROFILE_LEVEL_5: return "High efficiency AAC v2 profile, level 5";
    /* 0x64-0xFE RESERVED */
    case DVBPSI_AAC_PROFILE_NOT_SPECIFIED:
        return "Audio profile and level not specified by the "
            "MPEG-4_audio_profile_and_level field in this descriptor";
    /* RESERVED */
    case DVBPSI_AAC_PROFILE_RESERVED:
    default: return "reserved";
    }
}

/*****************************************************************************
 * DumpAACDescriptor
 *****************************************************************************/
static void DumpAACDescriptor(const void *p_descriptor)
{
    const dvbpsi_aac_dr_t *p_aac_descriptor = p_descriptor;
    printf("AAC audio descriptor\n");
    printf("\tprofile and level: %s (0x%02x)\n",
        AACProfileToString(p_aac_descriptor->i_profile_and_level),
        p_aac_descriptor->i_profile_and_level);

    if (p_aac_descriptor->b_type)
    {
        printf("\ttype: ");
        switch(p_aac_descriptor->i_type)
        {
        case DVBPSI_AAC_RESERVED0:
            printf("reserved\n");
            break;
        case DVBPSI_HE_AAC_MONO:
            printf("HE-AAC audio, single mono channel\n");
            break;
        case DVBPSI_AAC_RESERVED1:
            printf("reserved\n");
            break;
        case DVBPSI_HE_AAC_STEREO:
            printf("HE-AAC audio, stereo\n");
            break;
        case DVBPSI_AAC_RESERVED2:
            printf("reserved\n");
            break;
        case DVBPSI_HE_AAC_SURROUND:
            printf("HE-AAC audio, surround sound\n");
            break;

        case DVBPSI_HE_AAC_IMPAIRED:
            printf("HE-AAC audio description for the visually impaired\n");
            break;
        case DVBPSI_HE_AAC_HEARING:
            printf("HE-AAC audio for the hard of hearing\n");
            break;
        case DVBPSI_HE_AAC_MIXED:
            printf("HE-AAC receiver-mixed supplementary audio as per annex E of TS 101 154\n");
            break;
        case DVBPSI_HE_AAC_V2_STEREO:
            printf("HE-AAC v2 audio, stereo\n");
            break;
        case DVBPSI_HE_AAC_V2_IMPAIRED:
            printf("HE-AAC v2 audio description for the visually impaired\n");
            break;
        case DVBPSI_HE_AAC_V2_HEARING:
            printf("HE-AAC v2 audio for the hard of hearing\n");
            break;
        case DVBPSI_HE_AAC_V2_MIXED:
            printf("HE-AAC v2 receiver-mixed supplementary audio as per annex E of TS 101 154\n");
            break;
        case DVBPSI_HE_AAC_MIXED_IMPAIRED:
            printf("HE-AAC receiver mix audio description for the visually impaired\n");
            break;
        case DVBPSI_HE_AAC_BROADCAST_MIXED_IMPAIRED:
            printf("HE-AAC broadcaster mix audio description for the visually impaired\n");
            break;
        case DVBPSI_HE_AAC_V2_MIXED_IMPAIRED:
            printf("HE-AAC v2 receiver mix audio description for the visually impaired\n");
            break;
        case DVBPSI_HE_AAC_V2_BROADCAST_MIXED_IMPAIRED:
            printf("HE-AAC v2 broadcaster mix audio description for the visually impaired\n");
            break;

        default:
            printf("reserved\n");
            break;
        }
    }
    printf("\tadditional info bytes: %d\n", p_aac_descriptor->i_additional_info_length);
}

/*****************************************************************************
 * DumpTimeShiftedServiceDescriptor
 *****************************************************************************/
static void DumpTimeShiftedServiceDescriptor(const void *p_descriptor)
{
    const dvbpsi_tshifted_service_dr_t *p_ts_service = p_descriptor;
    printf("Time Shifted Service\n");
    printf("\treference service id:%d", p_ts_service->i_ref_service_id);
}

/*****************************************************************************
 * DumpTimeShiftedEventDescriptor
 *****************************************************************************/
static void DumpTimeShiftedEventDescriptor(const void *p_descriptor)
{
    const dvbpsi_tshifted_ev_dr_t *p_ts_event = p_descriptor;
    printf("Time Shifted Event");
    printf("\treference service id:%d", p_ts_event->i_ref_service_id);
    printf("\treference event id:%d", p_ts_event->i_ref_event_id);
}

/*****************************************************************************
 * DumpCUEIdentifierDescriptor
 *****************************************************************************/
#ifdef TS_USE_DVB_CUEI
static void DumpCUEIDescriptor(const void *p_descriptor)
{
    const char *cuei_stream_type;
    const dvbpsi_cuei_dr_t* p_cuei_descriptor = p_descriptor;

    assert(p_cuei_descriptor);

    switch(p_cuei_descriptor->i_cue_stream_type)
    {
        case 0x00:
            cuei_stream_type = "splice_insert, splice_null, splice_schedule";
            break;
        case 0x01: cuei_stream_type = "All Commands"; break;
        case 0x02: cuei_stream_type = "Segmentation"; break;
        case 0x03: cuei_stream_type = "Tiered Splicing"; break;
        case 0x04: cuei_stream_type = "Tiered Segmentation"; break;
        default:
            if ((p_cuei_descriptor->i_cue_stream_type >= 0x05) &&
                (p_cuei_descriptor->i_cue_stream_type <= 0x7f))
                cuei_stream_type = "Reserved";
            else if ((p_cuei_descriptor->i_cue_stream_type >= 0x80))
                cuei_stream_type = "User defined"; /* 0x80 - 0xFF */
            return;
    }
    printf("CUE Identifier stream type: (%0xd) %s\n",
           p_cuei_descriptor->i_cue_stream_type, cuei_stream_type );
}
#endif

#ifdef TS_USE_SCTE_SIS
/*****************************************************************************
 * DumpSISSegmentationDescriptor: SCTE 35 2004
 *****************************************************************************/
static void DumpSISSegmentationDescriptor(dvbpsi_descriptor_t* p_descriptor)
{
    /* FIXME: decode segmentation descriptor */
    printf("\"");
    for (int i = 4; i < p_descriptor->i_length; i++)
        printf("%c", p_descriptor->p_data[i]);
    printf("\" (%s)\n", "segmentation descriptor");
}

/*****************************************************************************
 * DumpSISDescriptors: SCTE 35 2004
 *****************************************************************************/
static void DumpSISDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
    while (p_descriptor)
    {
        assert(p_descriptor->i_length >= 4);
        uint32_t i_identifier = ((uint32_t)p_descriptor->p_data[0] << 24) |
                                ((uint32_t)p_descriptor->p_data[1] << 16) |
                                ((uint32_t)p_descriptor->p_data[2] << 8)  |
                                ((uint32_t)p_descriptor->p_data[3]);

        printf("%s 0x%02x : ", str, p_descriptor->i_tag);
        if (i_identifier == 0x43554549)
            printf("CUEI");
        else
            printf("unknown");

        switch (p_descriptor->i_tag)
        {
            case 0x00: /* avail_descriptor */
            {
                assert(p_descriptor->i_length >= 8);

                uint32_t id = ((uint32_t)p_descriptor->p_data[4] << 24) |
                              ((uint32_t)p_descriptor->p_data[5] << 16) |
                              ((uint32_t)p_descriptor->p_data[6] << 8)  |
                              ((uint32_t)p_descriptor->p_data[7]);

                printf("\"0x%x\" (%s)\n", id, "avail descriptor");
                break;
            }
            case 0x01: /* DTMF_descriptor */
            {
                assert(p_descriptor->i_length >= 6);
                double i_preroll = p_descriptor->p_data[4] * 0.1;
                uint8_t i_dtmf_count = (p_descriptor->p_data[5] & 0xE0);
                printf("\"");
                for (int i = 0; i < i_dtmf_count; i++)
                     printf("%c", p_descriptor->p_data[6 + i]);
                printf("\" preroll %.2f sec. (%s)\n", i_preroll, "DTMF descriptor");
                break;
            }
            case 0x02: /* segmentation_descriptor */
                DumpSISSegmentationDescriptor(p_descriptor);
                break;
            /* 0x03 - 0xFF : Reserved for future SCTE splice_descriptors */
            default:
                printf("\"");
                for (int i = 4; i < p_descriptor->i_length; i++)
                     printf("%c", p_descriptor->p_data[i]);
                printf("\" (%s)\n", GetDescriptorName(p_descriptor->i_tag));
                break;
        }
        p_descriptor = p_descriptor->p_next;
    }
}

static void handle_SIS(void* p_data, dvbpsi_sis_t* p_sis)
{
    ts_stream_t* p_stream = (ts_stream_t*) p_data;

    p_stream->sis.i_protocol_version = p_sis->i_protocol_version;

    printf("\n" );
    printf("  SIS: Splice Info Section\n" );
    printf("\tProtocol version : %d\n", p_sis->i_protocol_version );
    printf("\tEncrypted        : %s\n", p_sis->b_encrypted_packet ? "yes" : "no");
    printf("\tEncryption algorithm  : %d\n", p_sis->i_encryption_algorithm);
    printf("\tPTS adjustment   : %"PRId64"\n", p_sis->i_pts_adjustment);
    printf("\tCA Control word index : %d\n", p_sis->cw_index);
    printf("\tSplice command length : %d\n", p_sis->i_splice_command_length);

    printf("\tSplice command : ");
    switch(p_sis->i_splice_command_type)
    {
        default:
        case 0x00:
            printf("splice_null");
            break;
        case 0x04:
            printf("splice_schedule");
            break;
        case 0x05:
            printf("splice_insert");
            break;
        case 0x06:
            printf("time_signal");
            break;
        case 0x07:
            printf("bandwidth_reservation");
            break;
    }
    printf("\n");
    DumpSISDescriptors("\t   ]", p_sis->p_first_descriptor);
    dvbpsi_sis_delete(p_sis);
}
#endif

/*****************************************************************************
 * DumpDescriptor
 *****************************************************************************/
static void DumpDescriptor(dvbpsi_descriptor_t *p_descriptor)
{
    const void *p_decoded = NULL;
    void (*dump_dr_fn)(const void*) = NULL;

    switch (p_descriptor->i_tag)
    {
        case 0x06: /* data_stream_alignment_descriptor */
            /* ISO/IEC 11172-2 video, ITU-T Rec. H.262 | ISO/IEC 13818-2 video,
               or ISO/IEC 14496-2 visual streams */
        case 0x28:
            printf("\"");
            for(int i = 0; i < p_descriptor->i_length; i++)
            {
                switch(p_descriptor->p_data[i])
                {
                case 0x00: printf("0"); break;
                case 0x01: printf("1"); break;
                case 0x02: printf("2"); break;
                case 0x03: printf("3"); break;
                /* unknown or reserved values  */
                default: printf("?"); break;
                }
            }
            printf("\" (%s)\n", GetDescriptorName(p_descriptor->i_tag));
            return;
        case 0x6a:
            printf("\"a52\" (%s)\n", GetDescriptorName(p_descriptor->i_tag));
            return;
        case 0x7c:
            p_decoded = dvbpsi_DecodeAACDr(p_descriptor);
            dump_dr_fn = DumpAACDescriptor;
            break;
        case 0x08:
            p_decoded = dvbpsi_DecodeSystemClockDr(p_descriptor);
            dump_dr_fn = DumpSystemClockDescriptor;
            break;
#ifdef TS_USE_DVB_CUEI
        case 0x8a:
            p_decoded = dvbpsi_DecodeCUEIDr(p_descriptor);
            dump_dr_fn = DumpCUEIDescriptor;
            break;
#endif
        case 0x0e:
            p_decoded = dvbpsi_DecodeMaxBitrateDr(p_descriptor);
            dump_dr_fn = DumpMaxBitrateDescriptor;
            break;
        case 0x10:
            p_decoded = dvbpsi_DecodeSmoothingBufferDr(p_descriptor);
            dump_dr_fn = DumpSmoothingBufferDescriptor;
            break;
        case 0x11:
            p_decoded = dvbpsi_DecodeSTDDr(p_descriptor);
            dump_dr_fn = DumpSTDDescriptor;
            break;
        case 0x12:
            p_decoded = dvbpsi_DecodeIBPDr(p_descriptor);
            dump_dr_fn = DumpIBPDescriptor;
            break;
        case 0x1b:
            p_decoded = dvbpsi_DecodeMPEG4VideoDr(p_descriptor);
            dump_dr_fn = DumpMPEG4VideoDescriptor;
            break;
        case 0x1c:
            p_decoded = dvbpsi_DecodeMPEG4AudioDr(p_descriptor);
            dump_dr_fn = DumpMPEG4AudioDescriptor;
            break;
        case 0x4c:
            p_decoded = dvbpsi_DecodeTimeShiftedServiceDr(p_descriptor);
            dump_dr_fn = DumpTimeShiftedServiceDescriptor;
            break;
        case 0x4f:
            p_decoded = dvbpsi_DecodeTimeShiftedEventDr(p_descriptor);
            dump_dr_fn = DumpTimeShiftedEventDescriptor;
            break;
        case 0x52:
            p_decoded = dvbpsi_DecodeStreamIdentifierDr(p_descriptor);
            dump_dr_fn = DumpStreamIdentifierDescriptor;
            break;
        case 0x53:
            p_decoded = dvbpsi_DecodeCAIdentifierDr(p_descriptor);
            dump_dr_fn = DumpCAIdentifierDescriptor;
            break;
        case 0x54:
            p_decoded = dvbpsi_DecodeContentDr(p_descriptor);
            dump_dr_fn = DumpContentDescriptor;
            break;
        case 0x59:
            p_decoded = dvbpsi_DecodeSubtitlingDr(p_descriptor);
            dump_dr_fn = DumpSubtitleDescriptor;
            break;
    }

    if(dump_dr_fn && p_decoded)
    {
        /* call the dump function if we could decode the descriptor. */
        dump_dr_fn(p_decoded);
    }
    else
    {
        /* otherwise just dump the raw data. */
        printf("\"");
        for (int i = 0; i < p_descriptor->i_length; i++)
             printf("%c", p_descriptor->p_data[i]);
        printf("\" (%s)\n", GetDescriptorName(p_descriptor->i_tag));
    }
}

/*****************************************************************************
 * DumpDescriptors
 *****************************************************************************/
static void DumpDescriptors(const char* str, dvbpsi_descriptor_t* p_descriptor)
{
    while (p_descriptor)
    {
        printf("%s 0x%02x : ", str, p_descriptor->i_tag);
        DumpDescriptor(p_descriptor);
        p_descriptor = p_descriptor->p_next;
    }
}

/*****************************************************************************
 * handle_SDT
 *****************************************************************************/
static void handle_SDT(void* p_data, dvbpsi_sdt_t* p_sdt)
{
    dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;

    printf("\n");
    printf("  SDT: Session Descriptor Table\n");
    printf("\tVersion number : %d\n", p_sdt->i_version);
    printf("\tTransport stream id : %d\n", p_sdt->i_extension);
    printf("\tNetwork id     : %d\n", p_sdt->i_network_id);
    while (p_service)
    {
        printf("\t  | Service id   : 0x%02x \n", p_service->i_service_id);
        printf("\t  | EIT schedule : %s\n", p_service->b_eit_schedule ? "yes" : "no");
        printf("\t  | EIT present  : %s\n", p_service->b_eit_present ? "yes" : "no");
        printf("\t  | Running      : %d ", p_service->i_running_status);
        switch (p_service->i_running_status)
        {
            case 0x00: printf("(undefined)\n"); break;
            case 0x01: printf("(not running)\n"); break;
            case 0x02: printf("(starts in a few seconds (e.g. for video recording))\n"); break;
            case 0x03: printf("(pausing)\n"); break;
            case 0x04: printf("(running)\n"); break;
            case 0x05: printf("(service off-air)\n"); break;
            default: printf("(reserved for future use)\n"); break;
        }
        printf("\t  | Free CA      : %s\n", p_service->b_free_ca ? "yes" : "no");
        printf("\t  | Descriptor loop length: %d\n", p_service->i_descriptors_length);
        DumpDescriptors("\t  |  ]", p_service->p_first_descriptor);
        p_service = p_service->p_next;
    }
    dvbpsi_sdt_delete(p_sdt);
}

static void DumpEITEventDescriptors(dvbpsi_eit_event_t *p_eit_event)
{
    dvbpsi_eit_event_t *p_event = p_eit_event;

    while (p_event)
    {
        printf("\t  | Event id: %d\n", p_event->i_event_id);
        if( p_event->b_nvod )
            printf("\t  | Start time: Unscheduled Near Video On Demand (NVOD) event\n");
        else
            printf("\t  | Start time: %"PRId64"\n", p_event->i_start_time);
        printf("\t  | Duration: %d\n", p_event->i_duration);
        printf("\t  | Running status: %d\n", p_event->i_running_status);
        printf("\t  | Free CA mode: %s\n", p_event->b_free_ca ? "yes" : "no");
        printf("\t  | Descriptor loop length: %d bytes\n", p_event->i_descriptors_length);
        DumpDescriptors("\t  |  ]", p_event->p_first_descriptor);

        p_event = p_event->p_next;
    }
}

static void handle_EIT(void* p_data, dvbpsi_eit_t* p_eit)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  EIT: Event Information Table\n");
    printf("\tVersion number : %d\n", p_eit->i_version);
    printf("\tService id     : %d\n", p_eit->i_extension);
    printf("\tCurrent next   : %s\n", p_eit->b_current_next ? "yes" : "no");
    printf("\tTransport stream id : %d\n", p_eit->i_ts_id);
    printf("\tOriginal network id : %d\n", p_eit->i_network_id);
    printf("\tSegment last section number : %d\n", p_eit->i_segment_last_section_number);
    printf("\tLast Table id  : %d\n", p_eit->i_last_table_id);

    DumpEITEventDescriptors(p_eit->p_first_event);

    dvbpsi_eit_delete(p_eit);
}

static void handle_TOT(void* p_data, dvbpsi_tot_t* p_tot)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    uint8_t table_id = (p_tot->p_first_descriptor != NULL) ? 0x73 : 0x70;
    if (table_id == 0x70) /* TDT */
        printf("  TDT: Time and Date Table\n");
    else if (table_id == 0x73) /* TOT */
        printf("  TOT: Time Offset Table\n");

    printf("\tVersion number : %d\n", p_tot->i_version);
    printf("\tCurrent next   : %s\n", p_tot->b_current_next ? "yes" : "no");
    printf("\tUTC time       : %"PRId64"\n", p_tot->i_utc_time);

    DumpDescriptors("\t  |  ]", p_tot->p_first_descriptor);
    dvbpsi_tot_delete(p_tot);
}

static const char *GetATSCTableType(const int i_type)
{
    switch (i_type)
    {
    case 0x0000: return "Terrestrial VCT with current_next_indicator=’1’";
    case 0x0001: return "Terrestrial VCT with current_next_indicator=’0’";
    case 0x0002: return "Cable VCT with current_next_indicator=’1’";
    case 0x0003: return "Cable VCT with current_next_indicator=’0’";
    case 0x0004: return "Channel ETT";
    case 0x0005: return "DCCSCT";
    default:
        if (i_type >= 0x0006 && i_type <= 0x00FF)
           return "Reserved for future ATSC use";
        if (i_type >= 0x0100 && i_type <= 0x017F)
            return "EIT-0 to EIT-127";
        if (i_type >= 0x0180 && i_type <= 0x01FF)
            return "[Reserved for future ATSC use]";
        if (i_type >= 0x0200 && i_type <= 0x027F)
            return "Event ETT-0 to event ETT-127";
        if (i_type >= 0x0280 && i_type <= 0x0300)
            return "Reserved for future ATSC use";
        if (i_type >= 0x0301 && i_type <= 0x03FF)
            return "RRT with rating_region 1-255";
        if (i_type >= 0x0400 && i_type <= 0x0FFF)
            return "User private";
        if (i_type >= 0x1000 && i_type <= 0x13FF)
            return "Reserved for future ATSC use";
        if (i_type >= 0x1400 && i_type <= 0x14FF)
            return "DCCT with dcc_id 0x00 – 0xFF";
        if (i_type >= 0x1500 && i_type <= 0xFFFF)
            return "Reserved for future ATSC use";
        break;
    }
    return "unknown";
}

static void handle_atsc_MGT(void *p_data, dvbpsi_atsc_mgt_t *p_mgt)
{
    ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  ATSC MGT: Master Guide Table\n");

    printf("\tVersion number : %d\n", p_mgt->i_version);
    printf("\tCurrent next   : %s\n", p_mgt->b_current_next ? "yes" : "no");
    printf("\tTable ID extension: %d\n", p_mgt->i_table_id_ext);
    printf("\tProtocol version: %d\n", p_mgt->i_protocol); /* PSIP protocol version */

    dvbpsi_atsc_mgt_table_t   *p_table = p_mgt->p_first_table;
    while (p_table)
    {
        /* Attach new ATSC EIT decoder */
        ts_atsc_eit_t *p = calloc(1, sizeof(ts_atsc_eit_t));
        if (p)
        {
            /* PMT */
            p->handle = dvbpsi_new(&dvbpsi_message, p_stream->level);
            if (p->handle == NULL)
            {
                fprintf(stderr, "dvbinfo: Failed attach new ATSC EIT decoder\n");
                free(p);
                break;
            }

            p->i_table_pid = p_table->i_table_type_pid;
            p->pid = &p_stream->pid[p_table->i_table_type_pid];
            p->pid->i_pid = p_table->i_table_type_pid;
            p->p_next = NULL;

            if (!dvbpsi_AttachDemux(p->handle, handle_subtable, p_stream))
            {
                 fprintf(stderr, "dvbinfo: Failed to attach new ATSC EIT decoder\n");
                 dvbpsi_delete(p->handle);
                 free(p);
                 break;
            }

            /* insert at start of list */
            p->p_next = p_stream->atsc_eit;
            p_stream->atsc_eit = p;
            p_stream->i_atsc_eit++;
            assert(p_stream->atsc_eit);
        }
        else
            fprintf(stderr, "dvbinfo: Failed create new ATSC EIT decoder\n");

        printf("\n\t Table %d\n", p_stream->i_atsc_eit);
        printf("\t | PID : 0x%x (%d)\n", p_table->i_table_type_pid, p_table->i_table_type_pid);
        printf("\t | Type: %s\n", GetATSCTableType(p_table->i_table_type));
        printf("\t | Version: %d\n", p_table->i_table_type_version);
        printf("\t | Size: %d bytes\n", p_table->i_number_bytes);

        DumpDescriptors("\t  |  ]", p_table->p_first_descriptor);

        p_table = p_table->p_next;
    }

    DumpDescriptors("\t  |  ]", p_mgt->p_first_descriptor);
    dvbpsi_atsc_DeleteMGT(p_mgt);
}

static const char *GetAtscVCTModulationModes(const uint8_t i_mode)
{
    switch (i_mode)
    {
    case 0x00: return "Reserved";
    case 0x01: return "Analog — The virtual channel is modulated using standard analog methods for analog television.";
    case 0x02: return "SCTE_mode_1 — The virtual channel has a symbol rate of 5.057 Msps, transmitted in accordance with ANSI/SCTE 07 [21] (Mode 1). Typically, mode 1 will be used for 64-QAM.";
    case 0x03: return "SCTE_mode_2 — The virtual channel has a symbol rate of 5.361 Msps, transmitted in accordance with ANSI/SCTE 07 [21] (Mode 2). Typically, mode 2 will be used for 256-QAM.";
    case 0x04: return "ATSC (8 VSB) — The virtual channel uses the 8-VSB modulation method conforming to A/53 Part 2 [2].";
    case 0x05: return "ATSC (16 VSB) — The virtual channel uses the 16-VSB modulation method conforming to A/53 Part 2 [2].";
    default:
        if (i_mode >= 0x06 && i_mode <= 0x7F)
            return "Reserved for future use by ATSC";
        if (i_mode >= 0x80 && i_mode <= 0xFF)
            return "User Private";
        break;
    }
    return "unknown";
}

static const char *GetAtscETMLocations(const uint8_t i_etm_location)
{
    switch (i_etm_location)
    {
    case 0x0: return "No ETM";
    case 0x1: return "ETM located in the PTC carrying this PSIP";
    case 0x2: return "ETM located in the PTC specified by the channel_TSID";
    case 0x3: return "Reserved for future ATSC use";
    }
    return "unknown";
}

static void DumpAtscVCTChannels(dvbpsi_atsc_vct_channel_t *p_vct_channels)
{
    dvbpsi_atsc_vct_channel_t *p_channel = p_vct_channels;

    while (p_channel)
    {
        printf("\n");
        printf("\t  | Short name  : %s\n", p_channel->i_short_name);
        printf("\t  | Major number: %d\n", p_channel->i_major_number);
        printf("\t  | Minor number: %d\n", p_channel->i_minor_number);
        printf("\t  | Modulation  : %s\n", GetAtscVCTModulationModes(p_channel->i_modulation));
        printf("\t  | Carrier     : %d\n", p_channel->i_carrier_freq);
        printf("\t  | Transport id: %d\n", p_channel->i_channel_tsid);
        printf("\t  | Program number: %d\n", p_channel->i_program_number);
        printf("\t  | ETM location: %s\n", GetAtscETMLocations(p_channel->i_etm_location));
        printf("\t  | Scrambled   : %s\n", p_channel->b_access_controlled ? "yes" : "no");
        printf("\t  | Path Select : %s\n", p_channel->b_path_select ? "yes" : "no");
        printf("\t  | Out of band : %s\n", p_channel->b_out_of_band ? "yes" : "no");
        printf("\t  | Hidden      : %s\n", p_channel->b_hidden ? "yes" : "no");
        printf("\t  | Hide guide  : %s\n", p_channel->b_hide_guide ? "yes" : "no");
        printf("\t  | Service type: %d\n", p_channel->i_service_type);
        printf("\t  | Source id   : %d\n", p_channel->i_source_id);

        DumpDescriptors("\t  |  ]", p_channel->p_first_descriptor);
        p_channel = p_channel->p_next;
    }
}

static void handle_atsc_VCT(void* p_data, dvbpsi_atsc_vct_t *p_vct)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  ATSC VCT: Virtual Channel Table\n");

    printf("\tVersion number : %d\n", p_vct->i_version);
    printf("\tCurrent next   : %s\n", p_vct->b_current_next ? "yes" : "no");
    printf("\tProtocol version: %d\n", p_vct->i_protocol); /* PSIP protocol version */
    printf("\tType : %s Virtual Channel Table\n", (p_vct->b_cable_vct) ? "Cable" : "Terrestrial" );

    DumpAtscVCTChannels(p_vct->p_first_channel);
    DumpDescriptors("\t  |  ]", p_vct->p_first_descriptor);
    dvbpsi_atsc_DeleteVCT(p_vct);
}

static void DumpATSCEITEventDescriptors(dvbpsi_atsc_eit_event_t *p_atsc_eit_event)
{
    dvbpsi_atsc_eit_event_t *p_event = p_atsc_eit_event;

    while (p_event)
    {
        printf("\t  | Event id: %d\n", p_event->i_event_id);
        printf("\t  | Start time: %u\n", p_event->i_start_time);
        printf("\t  | ETM location: %s\n", GetAtscETMLocations(p_event->i_etm_location));
        printf("\t  | Duration: %d seconds\n", p_event->i_length_seconds);
        printf("\t  | Title length: %d bytes\n", p_event->i_title_length);
        printf("\t  | Title: %s\n", p_event->i_title);
        DumpDescriptors("\t  |  ]", p_event->p_first_descriptor);

        p_event = p_event->p_next;
    }
}

static void handle_atsc_EIT(void* p_data, dvbpsi_atsc_eit_t* p_eit)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  ATSC EIT: Event Information Table\n");

    printf("\tVersion number : %d\n", p_eit->i_version);
    printf("\tCurrent next   : %s\n", p_eit->b_current_next ? "yes" : "no");
    printf("\tProtocol version: %d\n", p_eit->i_protocol);
    printf("\tSource id      : %d\n", p_eit->i_source_id);

    printf("\tEIT events\n");
    DumpATSCEITEventDescriptors(p_eit->p_first_event);
    DumpDescriptors("\t  |  ]", p_eit->p_first_descriptor);
    dvbpsi_atsc_DeleteEIT(p_eit);

}

static void handle_atsc_ETT(void* p_data, dvbpsi_atsc_ett_t* p_ett)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  ATSC ETT: Extended Text Table\n");

    printf("\tVersion number : %d\n", p_ett->i_version);
    printf("\tCurrent next   : %s\n", p_ett->b_current_next ? "yes" : "no");
    printf("\tProtocol version: %d\n", p_ett->i_protocol);

    printf("\tETM specific\n");
    printf("\tIdentifier     : %d\n", p_ett->i_etm_id);
    printf("\tLength         : %d\n", p_ett->i_etm_length);
    printf("\tRaw Data       : '%s'\n", p_ett->p_etm_data);

    DumpDescriptors("\t  |  ]", p_ett->p_first_descriptor);
    dvbpsi_atsc_DeleteETT(p_ett);
}

static void handle_atsc_STT(void* p_data, dvbpsi_atsc_stt_t *p_stt)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  ATSC STT: System Time Table\n");

    printf("\tVersion number : %d\n", p_stt->i_version);
    printf("\tCurrent next   : %s\n", p_stt->b_current_next ? "yes" : "no");

    printf("\tSystem time (GPS): %d seconds\n", p_stt->i_system_time);
    printf("\tGPS-UTC Offset   : %d seconds\n", p_stt->i_gps_utc_offset);

    /* decode daylight savings */
    bool b_status = (p_stt->i_daylight_savings & 0x01);
    uint8_t i_day_of_month = ((p_stt->i_daylight_savings & 0x00F8) >> 3);
    uint8_t i_hour = (p_stt->i_daylight_savings >> 8);

    printf("\tDaylight savings : %s\n", b_status ? "on" : "off" );
    printf("\t\tDay of month: %d\n", i_day_of_month);
    printf("\t\tHour of day : %d\n", i_hour);

    DumpDescriptors("\t  |  ]", p_stt->p_first_descriptor);
    dvbpsi_atsc_DeleteSTT(p_stt);
}

static void DumpRSTEvents(const char* str, dvbpsi_rst_event_t* p_event)
{
    while (p_event)
    {
        printf("%s transport stream id: %d\n", str, p_event->i_ts_id);
        printf("%s original network id: %d\n", str, p_event->i_orig_network_id);
        printf("%s service id: %d\n", str, p_event->i_service_id);
        printf("%s event id: %d\n", str, p_event->i_event_id);
        printf("%s running status id: %d ", str, p_event->i_running_status);
        switch (p_event->i_running_status)
        {
            case 0x00: printf("(undefined)\n"); break;
            case 0x01: printf("(not running)\n"); break;
            case 0x02: printf("(starts in a few seconds (e.g. for video recording))\n"); break;
            case 0x03: printf("(pausing)\n"); break;
            case 0x04: printf("(running)\n"); break;
            case 0x05: printf("(service off-air)\n"); break;
            default: printf("(reserved for future use)\n"); break;
        }

        p_event = p_event->p_next;
    };
}

static void handle_RST(void* p_data, dvbpsi_rst_t* p_rst)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  RST: Running Status Table\n");
    DumpRSTEvents("\t  |  ]", p_rst->p_first_event);
    dvbpsi_rst_delete(p_rst);
}

/*****************************************************************************
 * handle_NIT
 *****************************************************************************/
static void DumpTSDescriptorsNIT(dvbpsi_nit_ts_t *p_nit_ts)
{
  dvbpsi_nit_ts_t *p_ts = p_nit_ts;

  while (p_ts)
  {
      printf("\t  | transport id: %d\n", p_ts->i_ts_id);
      printf("\t  | original network id: %d\n", p_ts->i_orig_network_id);
      DumpDescriptors("\t  |  ]", p_nit_ts->p_first_descriptor);
      p_ts = p_ts->p_next;
  }
}

static void handle_NIT(void* p_data, dvbpsi_nit_t* p_nit)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  NIT: Network Information Table\n");
    printf("\tVersion number : %d\n", p_nit->i_version);
    printf("\tNetwork id     : %d\n", p_nit->i_network_id);
    printf("\tCurrent next   : %s\n", p_nit->b_current_next ? "yes" : "no");
    DumpDescriptors("\t  |  ]", p_nit->p_first_descriptor);
    DumpTSDescriptorsNIT(p_nit->p_first_ts);
    dvbpsi_nit_delete(p_nit);
}

/*****************************************************************************
 * handle_BAT
 *****************************************************************************/
static void DumpTSDescriptorsBAT(dvbpsi_bat_ts_t *p_bat_ts)
{
  dvbpsi_bat_ts_t *p_ts = p_bat_ts;

  while (p_ts)
  {
      printf("\t  | transport id: %d\n", p_ts->i_ts_id);
      printf("\t  | original network id: %d\n", p_ts->i_orig_network_id);
      DumpDescriptors("\t  |  ]", p_bat_ts->p_first_descriptor);
      p_ts = p_ts->p_next;
  }
}

static void handle_BAT(void* p_data, dvbpsi_bat_t* p_bat)
{
    //ts_stream_t* p_stream = (ts_stream_t*) p_data;

    printf("\n");
    printf("  BAT: Bouquet Association Table\n");
    printf("\tVersion number : %d\n", p_bat->i_version);
    printf("\tBouquet id     : %d\n", p_bat->i_extension);
    printf("\tCurrent next   : %s\n", p_bat->b_current_next ? "yes" : "no");
    DumpDescriptors("\t  |  ]", p_bat->p_first_descriptor);
    DumpTSDescriptorsBAT(p_bat->p_first_ts);
    dvbpsi_bat_delete(p_bat);
}

/*****************************************************************************
 * handle_PMT
 *****************************************************************************/
static void handle_PMT(void* p_data, dvbpsi_pmt_t* p_pmt)
{
    dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
    ts_stream_t* p_stream = (ts_stream_t*) p_data;

    /* Find signalled PMT */
    ts_pmt_t *p = p_stream->pmt;
    while (p)
    {
        if (p->i_number == p_pmt->i_program_number)
            break;
        p = p->p_next;
    }
    assert(p);

    p->i_pmt_version = p_pmt->i_version;
    p->pid_pcr = &p_stream->pid[p_pmt->i_pcr_pid];
    p_stream->pid[p_pmt->i_pcr_pid].b_pcr = true;

    printf("\n");
    printf("  PMT: Program Map Table\n");
    printf("\tProgram number : %d\n", p_pmt->i_program_number);
    printf("\tVersion number : %d\n", p_pmt->i_version);
    printf("\tPCR_PID        : 0x%x (%d)\n", p_pmt->i_pcr_pid, p_pmt->i_pcr_pid);
    printf("\tCurrent next   : %s\n", p_pmt->b_current_next ? "yes" : "no");
    DumpDescriptors("\t   ]", p_pmt->p_first_descriptor);
    printf("\t| type @ elementary_PID : Description\n");
    while(p_es)
    {
        printf("\t| 0x%02x @ pid 0x%x (%d): %s\n",
                 p_es->i_type, p_es->i_pid, p_es->i_pid,
                 GetTypeName(p_es->i_type) );
        DumpDescriptors("\t|  ]", p_es->p_first_descriptor);
        p_es = p_es->p_next;
    }
    dvbpsi_pmt_delete(p_pmt);
}

/*****************************************************************************
 * handle_CAT
 *****************************************************************************/
static void handle_CAT(void *p_data, dvbpsi_cat_t *p_cat)
{
    ts_stream_t* p_stream = (ts_stream_t*) p_data;

    p_stream->cat.i_version = p_cat->i_version;

    printf("\n" );
    printf("  CAT: Conditional Access Table\n" );
    printf("\tVersion number : %d\n", p_cat->i_version );
    printf("\tCurrent next   : %s\n", p_cat->b_current_next ? "yes" : "no");
    DumpDescriptors("\t   ]", p_cat->p_first_descriptor);
    printf("\n");
    dvbpsi_cat_delete(p_cat);
}

/*****************************************************************************
 * Public API
 *****************************************************************************/
ts_stream_t *libdvbpsi_init(int debug, ts_stream_log_cb pf_log, void *cb_data)
{
    ts_stream_t *stream = (ts_stream_t *)calloc(1, sizeof(ts_stream_t));
    if (stream == NULL)
        return NULL;

    if (pf_log)
    {
        stream->pf_log = pf_log;
        stream->cb_data = cb_data;
    }

    /* print PSI tables debug anyway, unless no debug is wanted at all */
    switch (debug)
    {
        case 0: stream->level = DVBPSI_MSG_NONE; break;
        case 1: stream->level = DVBPSI_MSG_ERROR; break;
        case 2: stream->level = DVBPSI_MSG_WARN; break;
        case 3: stream->level = DVBPSI_MSG_DEBUG; break;
    }

    /* PAT */
    stream->pat.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->pat.handle == NULL)
        goto error;
    if (!dvbpsi_pat_attach(stream->pat.handle, handle_PAT, stream))
    {
        dvbpsi_delete(stream->pat.handle);
        stream->pat.handle = NULL;
        goto error;
    }
    /* CAT */
    stream->cat.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->cat.handle == NULL)
        goto error;
    if (!dvbpsi_cat_attach(stream->cat.handle, handle_CAT, stream))
    {
        dvbpsi_delete(stream->cat.handle);
        stream->cat.handle = NULL;
        goto error;
    }
    /* SDT demuxer */
    stream->sdt.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->sdt.handle == NULL)
        goto error;
    if (!dvbpsi_AttachDemux(stream->sdt.handle, handle_subtable, stream))
    {
        dvbpsi_delete(stream->sdt.handle);
        stream->sdt.handle = NULL;
        goto error;
    }
    /* RST */
    stream->rst.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->rst.handle == NULL)
        goto error;
    if (!dvbpsi_rst_attach(stream->rst.handle, handle_RST, stream))
    {
        dvbpsi_delete(stream->rst.handle);
        stream->rst.handle = NULL;
        goto error;
    }
    /* EIT demuxer */
    stream->eit.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->eit.handle == NULL)
        goto error;
    if (!dvbpsi_AttachDemux(stream->eit.handle, handle_subtable, stream))
    {
        dvbpsi_delete(stream->eit.handle);
        stream->eit.handle = NULL;
        goto error;
    }
    /* TDT demuxer */
    stream->tdt.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->tdt.handle == NULL)
        goto error;
    if (!dvbpsi_AttachDemux(stream->tdt.handle, handle_subtable, stream))
    {
        dvbpsi_delete(stream->tdt.handle);
        stream->tdt.handle = NULL;
        goto error;
    }

    /* ATSC MGT demuxer */
    stream->atsc.handle = dvbpsi_new(&dvbpsi_message, stream->level);
    if (stream->atsc.handle == NULL)
        goto error;
    if (!dvbpsi_AttachDemux(stream->atsc.handle, handle_subtable, stream))
    {
        dvbpsi_delete(stream->atsc.handle);
        stream->atsc.handle = NULL;
        goto error;
    }

    /* */
    stream->pat.pid = &stream->pid[0x00];
    stream->cat.pid = &stream->pid[0x01];
#if 0
    stream->tsdt.pid = &stream->pid[0x02];
    stream->ipmp.pid = &stream->pid[0x03];
#endif
    stream->sdt.pid = &stream->pid[0x11];
    stream->eit.pid = &stream->pid[0x12];
    stream->rst.pid = &stream->pid[0x13];
    stream->tdt.pid = &stream->pid[0x14];
    stream->atsc.pid = &stream->pid[0x1FFB];
    return stream;

error:
    if (dvbpsi_decoder_present(stream->pat.handle))
        dvbpsi_pat_detach(stream->pat.handle);
    if (dvbpsi_decoder_present(stream->cat.handle))
        dvbpsi_cat_detach(stream->cat.handle);
    if (dvbpsi_decoder_present(stream->sdt.handle))
        dvbpsi_DetachDemux(stream->sdt.handle);
    if (dvbpsi_decoder_present(stream->eit.handle))
        dvbpsi_DetachDemux(stream->eit.handle);
    if (dvbpsi_decoder_present(stream->rst.handle))
        dvbpsi_rst_detach(stream->rst.handle);
    if (dvbpsi_decoder_present(stream->tdt.handle))
        dvbpsi_DetachDemux(stream->tdt.handle);
    if (dvbpsi_decoder_present(stream->atsc.handle))
        dvbpsi_DetachDemux(stream->atsc.handle);

    if (stream->pat.handle)
        dvbpsi_delete(stream->pat.handle);
    if (stream->cat.handle)
        dvbpsi_delete(stream->cat.handle);
    if (stream->sdt.handle)
        dvbpsi_delete(stream->sdt.handle);
    if (stream->rst.handle)
        dvbpsi_delete(stream->rst.handle);
    if (stream->eit.handle)
        dvbpsi_delete(stream->eit.handle);
    if (stream->tdt.handle)
        dvbpsi_delete(stream->tdt.handle);
    if (stream->atsc.handle)
        dvbpsi_delete(stream->atsc.handle);

    free(stream);

    return NULL;
}

void libdvbpsi_exit(ts_stream_t *stream)
{
   summary(stdout, stream);

   if (dvbpsi_decoder_present(stream->pat.handle))
       dvbpsi_pat_detach(stream->pat.handle);

   ts_pmt_t *p_pmt = stream->pmt;
   ts_pmt_t *p_prev = NULL;
   while (p_pmt)
   {
       dvbpsi_t *handle = p_pmt->handle;
       if (dvbpsi_decoder_present(handle))
       {
            dvbpsi_pmt_detach(handle);
            dvbpsi_delete(p_pmt->handle);
       }
       stream->i_pmt--;
       p_prev = p_pmt;
       p_pmt = p_pmt->p_next;
       if (p_prev)
           p_prev->p_next = NULL;
       free(p_prev);
   }

   ts_atsc_eit_t *p_atsc_eit = stream->atsc_eit;
   ts_atsc_eit_t *p_atsc_prev = NULL;
   while (p_atsc_eit)
   {
       dvbpsi_t *handle = p_atsc_eit->handle;
       if (dvbpsi_decoder_present(handle))
       {
            dvbpsi_DetachDemux(handle);
            dvbpsi_delete(p_atsc_eit->handle);
       }
       stream->i_atsc_eit--;
       p_atsc_prev = p_atsc_eit;
       p_atsc_eit = p_atsc_eit->p_next;
       if (p_atsc_prev)
           p_atsc_prev->p_next = NULL;
       free(p_atsc_prev);
   }

   if (dvbpsi_decoder_present(stream->cat.handle))
       dvbpsi_cat_detach(stream->cat.handle);
   if (dvbpsi_decoder_present(stream->sdt.handle))
       dvbpsi_DetachDemux(stream->sdt.handle);
   if (dvbpsi_decoder_present(stream->rst.handle))
       dvbpsi_rst_detach(stream->rst.handle);
   if (dvbpsi_decoder_present(stream->eit.handle))
       dvbpsi_DetachDemux(stream->eit.handle);
   if (dvbpsi_decoder_present(stream->tdt.handle))
       dvbpsi_DetachDemux(stream->tdt.handle);
   if (dvbpsi_decoder_present(stream->atsc.handle))
        dvbpsi_DetachDemux(stream->atsc.handle);

   if (stream->pat.handle)
       dvbpsi_delete(stream->pat.handle);
   if (stream->cat.handle)
       dvbpsi_delete(stream->cat.handle);
   if (stream->sdt.handle)
       dvbpsi_delete(stream->sdt.handle);
   if (stream->rst.handle)
       dvbpsi_delete(stream->rst.handle);
   if (stream->eit.handle)
       dvbpsi_delete(stream->eit.handle);
   if (stream->tdt.handle)
       dvbpsi_delete(stream->tdt.handle);
   if (stream->atsc.handle)
       dvbpsi_delete(stream->atsc.handle);

   free(stream);
   stream = NULL;
}

static ssize_t check_sync_word(uint8_t *buf, ssize_t length)
{
    ssize_t i_lost = 0;

    for (i_lost = 0; i_lost < length; i_lost++)
    {
        if (buf[i_lost] == 0x47)
            break;
    }

    if ((length - i_lost) < 188)
        i_lost = length;

    return i_lost;
}

bool libdvbpsi_process(ts_stream_t *stream, uint8_t *buf, ssize_t length, mtime_t date)
{
    mtime_t  i_prev_pcr = 0;  /* 33 bits */
    int      i_old_cc = -1;

    for (ssize_t i = 0; i < length; i += 188)
    {
        /* check sync */
        ssize_t i_lost = check_sync_word(buf+i, length - i);
        if (i_lost > 0)
        {
            stream->i_lost_bytes += i_lost;
            i += i_lost;
            stream->pf_log(stream->cb_data, 0,
                           "dvbinfo: %"PRId64": lost %"PRId64" bytes out of %"PRId64" in buffer\n",
                           date, (int64_t) i_lost, (int64_t)length);
            if (i >= length)
                return true;
        }

        assert(buf[i] == 0x47);

        /* parse packet */
        uint8_t  *p_tmp = &buf[i];
        uint16_t i_pid = ((uint16_t)(p_tmp[1] & 0x1f) << 8) + p_tmp[2];
        int      i_cc = (p_tmp[3] & 0x0f);
        bool     b_discontinuity_seen = false;

        /* keep track nr of packets for this ES */
        stream->pid[i_pid].i_packets++;
        stream->i_packets++;

        /* received times */
        stream->pid[i_pid].i_prev_received = stream->pid[i_pid].i_received;
        stream->pid[i_pid].i_received = date;

        if (stream->level < DVBPSI_MSG_DEBUG)
            stream->pf_log(stream->cb_data, 3,
                           "dvbinfo: %"PRId64" packet %"PRId64" pid %u (0x%x) cc %d\n",
                           date, stream->i_packets, i_pid, i_pid, i_cc);

        if (i_pid == 0x0) /* PAT */
            dvbpsi_packet_push(stream->pat.handle, p_tmp);
        else if (i_pid == 0x01) /* CAT */
            dvbpsi_packet_push(stream->cat.handle, p_tmp);
        else if (i_pid == 0x02) /* Transport Stream Description Table */
            dvbpsi_packet_push(stream->tdt.handle, p_tmp);
#if 0
        else if (i_pid == 0x03) /* IPMP Control Information Table */
            dvbpsi_packet_push(stream->ipmp.handle, p_tmp);
#endif
        else if (i_pid == 0x11) /* SDT/BAT/NIT */
            dvbpsi_packet_push(stream->sdt.handle, p_tmp);
        else if (i_pid == 0x12) /* EIT */
            dvbpsi_packet_push(stream->eit.handle, p_tmp);
        else if (i_pid == 0x13) /* RST */
            dvbpsi_packet_push(stream->rst.handle, p_tmp);
        else if (i_pid == 0x14) /* TDT/TOT */
            dvbpsi_packet_push(stream->tdt.handle, p_tmp);
        else if (i_pid == 0x1FFB) /* ATSC tables */
            dvbpsi_packet_push(stream->atsc.handle, p_tmp);
        else
        {
            ts_pmt_t *p = stream->pmt;
            while(p)
            {
                if (p->pid_pmt->i_pid == i_pid)
                    dvbpsi_packet_push(p->handle, p_tmp);
                p = p->p_next;
            }

            ts_atsc_eit_t *p_atsc_eit = stream->atsc_eit;
            while (p_atsc_eit)
            {
                if (p_atsc_eit->pid->i_pid == i_pid)
                    dvbpsi_packet_push(p_atsc_eit->handle, p_tmp);
                p_atsc_eit = p_atsc_eit->p_next;
            }
        }

        /* Remember PID */
        if (!stream->pid[i_pid].b_seen)
        {
            stream->pid[i_pid].i_pid = i_pid;
            stream->pid[i_pid].b_seen = true;
            i_old_cc = i_cc;
            stream->pid[i_pid].i_cc = i_cc;
        }
        else
        {
            /* Check continuity counter */
            int i_diff = 0;

            i_diff = i_cc - (stream->pid[i_pid].i_cc+1)%16;
            b_discontinuity_seen = (i_diff != 0);

            /* Update CC */
            i_old_cc = stream->pid[i_pid].i_cc;
            stream->pid[i_pid].i_cc = i_cc;
        }

        if (i_pid == 0x1FFF)
        {
            stream->i_null_packets++;
            /* NULL packet - skip it */
            goto dump_packet;
        }

        /* */
        stream->pid[i_pid].b_transport_error_indicator = ((p_tmp[1] & 0x80) == 0x80);
        stream->pid[i_pid].b_payload_unit_start_indicator = ((p_tmp[1] & 0x40) == 0x40);
        stream->pid[i_pid].b_transport_priority = ((p_tmp[1] & 0x20) == 0x20);
        stream->pid[i_pid].i_transport_scrambling_control = ((p_tmp[3] & 0xC0) >> 6);
        stream->pid[i_pid].b_adaptation_field = (p_tmp[3] & 0x20);

        /* Handle discontinuities if they occurred,
         * according to ISO/IEC 13818-1: DIS pages 20-22 */
        if (stream->pid[i_pid].b_adaptation_field && (p_tmp[4] > 0))
        {
            bool b_pcr  = (p_tmp[5]&0x10) == 0x10;  /* PCR flag */
            bool b_opcr = (p_tmp[5]&0x08) == 0x08;  /* OPCR flag */

            stream->pid[i_pid].b_discontinuity_indicator = (p_tmp[5]&0x80) == 0x80;
            stream->pid[i_pid].b_random_access_indicator = (p_tmp[5]&0x40) == 0x40;
            stream->pid[i_pid].b_elementary_stream_priority_indicator = (p_tmp[5]&0x20) == 0x20;
            stream->pid[i_pid].b_splicing_point = (p_tmp[5]&0x04) == 0x04;
            stream->pid[i_pid].b_transport_private_data = (p_tmp[5]&0x02) == 0x02;
            stream->pid[i_pid].b_adaptation_field_extension = (p_tmp[5]&0x01) == 0x01;

            uint32_t i_ext = 5;

            if (b_pcr) i_ext += 6;

            /* PCR */
            if (b_pcr && (p_tmp[4] >= 7))
            {
                mtime_t i_pcr;  /* 33 bits */

                i_pcr = (( (mtime_t)p_tmp[6] << 25 ) |
                         ( (mtime_t)p_tmp[7] << 17 ) |
                         ( (mtime_t)p_tmp[8] << 9 ) |
                         ( (mtime_t)p_tmp[9] << 1 ) |
                         ( (mtime_t)(p_tmp[10]&0x80) >> 7 ));
                i_pcr = i_pcr * 100 / 9;
                i_prev_pcr = stream->pid[i_pid].i_pcr;
                stream->pid[i_pid].i_pcr = i_pcr;

                if (stream->pid[i_pid].i_first_pcr == 0)
                    stream->pid[i_pid].i_first_pcr = i_pcr;
                if (i_pcr < stream->pid[i_pid].i_last_pcr)
                {
                    if (b_discontinuity_seen)
                        stream->pf_log(stream->cb_data, 2,
                                       "dvbinfo: Warning wrapping PCR on discontinuity\n");
                    else
                        stream->pf_log(stream->cb_data, 2,
                                       "dvbinfo: Warning wrapping PCR\n");
                }
                stream->pid[i_pid].i_prev_pcr = i_prev_pcr;
                stream->pid[i_pid].i_last_pcr = i_pcr;

                if (stream->pid[i_pid].b_discontinuity_indicator)
                {
                    /* cc discontinuity is expected */
                    stream->pf_log(stream->cb_data, 2,
                                   "dvbinfo: Server signalled the continuity counter discontinuity\n");

                    /* Discontinuity has been handled */
                    b_discontinuity_seen = false;
                }
            }

            if (b_opcr) i_ext += 6;

            if (stream->pid[i_pid].b_splicing_point)
            {
                i_ext++;
                /* calculate tcimsbf */
                stream->pid[i_pid].i_splice_countdown = ((p_tmp[i_ext] & 0x80) == 0x80) ?
                                        -1 * (p_tmp[i_ext] & 0x7f) : (p_tmp[i_ext] & 0x7f);
            }

            if (stream->pid[i_pid].b_transport_private_data)
            {
                i_ext++;
                stream->pid[i_pid].i_transport_private_data_length = p_tmp[i_ext];
                i_ext += stream->pid[i_pid].i_transport_private_data_length;
            }

            if (stream->pid[i_pid].b_adaptation_field_extension)
            {
                /* i_ext is start of adaptation_extension field */
                i_ext++;
                uint8_t *p_ext = &p_tmp[i_ext];
                uint32_t i_seamless_splice = i_ext;

                stream->pid[i_pid].i_adaptation_field_extension_length = p_ext[0];

                if (stream->pid[i_pid].i_adaptation_field_extension_length > 0)
                {
                    stream->pid[i_pid].b_ltw = (p_ext[1]&0x80) == 0x80;
                    stream->pid[i_pid].b_piecewise_rate = (p_ext[1]&0x40) == 0x40;
                    stream->pid[i_pid].b_seamless_splice = (p_ext[1]&0x20) == 0x20;

                    if (stream->pid[i_pid].b_ltw)
                    {
                        stream->pid[i_pid].b_ltw_valid = ((p_ext[2]&0x80) == 0x80);
                        stream->pid[i_pid].i_ltw_offset = ((uint16_t)p_ext[2]&0x7F);
                        i_seamless_splice += 2;
                    }

                    if (stream->pid[i_pid].b_piecewise_rate)
                    {
                        stream->pid[i_pid].i_piecewise_rate =
                          (((uint32_t)p_ext[i_seamless_splice] & 0x3F) << 16) |
                          (((uint32_t)p_ext[i_seamless_splice + 1]) << 8) |
                           ((uint32_t)p_ext[i_seamless_splice + 2]);
                        i_seamless_splice += 3;
                    }

                    if (stream->pid[i_pid].b_seamless_splice)
                    {
                        stream->pid[i_pid].i_splice_type =
                            (p_tmp[i_seamless_splice]&0xF0);
                    }
                }
            } /* end of adaptation_extension_field */
        }

        if (b_discontinuity_seen)
        {
            stream->pf_log(stream->cb_data, 2,
                           "dvbinfo: Continuity counter discontinuity (pid %u 0x%x found %d expected %d)\n",
                           i_pid, i_pid, stream->pid[i_pid].i_cc, i_old_cc+1);

            /* Discontinuity has been handled */
            b_discontinuity_seen = false;
        }

dump_packet:
        if (stream->level >= DVBPSI_MSG_DEBUG)
        {
            ts_dump_packet_details(stdout, stream, &buf[i], i_pid);
        }
    }

    return true;
}

void libdvbpsi_summary(FILE *fd, ts_stream_t *stream, const int summary_mode)
{
    switch(summary_mode)
    {
        case SUM_TABLE:
            summary_table(fd, stream);
            break;
        case SUM_PACKET:
            summary_packet(fd, stream);
            break;
#if 0
        case SUM_WIRE:
            summary_wire(fd, stream);
            break;
#endif
        case SUM_BANDWIDTH:
        default:
            summary(fd, stream);
            break;
    }
}
