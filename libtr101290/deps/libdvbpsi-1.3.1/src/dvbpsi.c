/*****************************************************************************
 * dvbpsi.c: conversion from TS packets to PSI sections
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

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include <assert.h>

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"

/*****************************************************************************
 * dvbpsi_new
 *****************************************************************************/
dvbpsi_t *dvbpsi_new(dvbpsi_message_cb callback, enum dvbpsi_msg_level level)
{
    dvbpsi_t *p_dvbpsi = calloc(1, sizeof(dvbpsi_t));
    if (p_dvbpsi == NULL)
        return NULL;

    p_dvbpsi->p_decoder  = NULL;
    p_dvbpsi->pf_message = callback;
    p_dvbpsi->i_msg_level = level;
    return p_dvbpsi;
}

/*****************************************************************************
 * dvbpsi_delete
 *****************************************************************************/
void dvbpsi_delete(dvbpsi_t *p_dvbpsi)
{
    if (p_dvbpsi) {
        assert(p_dvbpsi->p_decoder == NULL);
        p_dvbpsi->pf_message = NULL;
    }
    free(p_dvbpsi);
}

/*****************************************************************************
 * dvbpsi_decoder_new
 *****************************************************************************/
#define DVBPSI_INVALID_CC (0xFF)
void *dvbpsi_decoder_new(dvbpsi_callback_gather_t pf_gather,
    const int i_section_max_size, const bool b_discontinuity, const size_t psi_size)
{
    assert(psi_size >= sizeof(dvbpsi_decoder_t));

    dvbpsi_decoder_t *p_decoder = (dvbpsi_decoder_t *) calloc(1, psi_size);
    if (p_decoder == NULL)
        return NULL;

    memcpy(&p_decoder->i_magic[0], "psi", 3);
    p_decoder->pf_gather = pf_gather;
    p_decoder->p_current_section = NULL;
    p_decoder->i_section_max_size = i_section_max_size;
    p_decoder->b_discontinuity = b_discontinuity;
    p_decoder->i_continuity_counter = DVBPSI_INVALID_CC;
    p_decoder->p_current_section = NULL;
    p_decoder->b_current_valid = false;

    p_decoder->i_last_section_number = 0;
    p_decoder->p_sections = NULL;
    p_decoder->b_complete_header = false;

    return p_decoder;
}

/*****************************************************************************
 * dvbpsi_decoder_reset
 *****************************************************************************/
void dvbpsi_decoder_reset(dvbpsi_decoder_t* p_decoder, const bool b_force)
{
    assert(p_decoder);

    /* Force redecoding */
    if (b_force)
        p_decoder->b_current_valid = false;

    /* Clear the section array */
    dvbpsi_DeletePSISections(p_decoder->p_sections);
    p_decoder->p_sections = NULL;
}

/*****************************************************************************
 * dvbpsi_decoder_psi_sections_completed
 *****************************************************************************/
bool dvbpsi_decoder_psi_sections_completed(dvbpsi_decoder_t* p_decoder)
{
    assert(p_decoder);

    bool b_complete = false;

    dvbpsi_psi_section_t *p = p_decoder->p_sections;
    unsigned int prev_nr = 0;
    while (p)
    {
        assert(prev_nr < 256);
        if (prev_nr != p->i_number)
            break;
        if (p_decoder->i_last_section_number == p->i_number)
            b_complete = true;
        p = p->p_next;
        prev_nr++;
    }

    return b_complete;
}

/*****************************************************************************
 * dvbpsi_decoder_psi_section_add
 *****************************************************************************/
bool dvbpsi_decoder_psi_section_add(dvbpsi_decoder_t *p_decoder, dvbpsi_psi_section_t *p_section)
{
    assert(p_decoder);
    assert(p_section);
    assert(p_section->p_next == NULL);

    /* Empty list */
    if (!p_decoder->p_sections)
    {
        p_decoder->p_sections = p_section;
        p_section->p_next = NULL;
        return false;
    }

    /* Insert in right place */
    dvbpsi_psi_section_t *p = p_decoder->p_sections;
    dvbpsi_psi_section_t *p_prev = NULL;
    bool b_overwrite = false;

    while (p)
    {
        if (p->i_number == p_section->i_number)
        {
            /* Replace */
            if (p_prev)
            {
                p_prev->p_next = p_section;
                p_section->p_next = p->p_next;
                p->p_next = NULL;
                dvbpsi_DeletePSISections(p);
                b_overwrite = true;
            }
            else
            {
                p_section->p_next = p->p_next;
                p->p_next = NULL;
                dvbpsi_DeletePSISections(p);
                p = p_section;
                p_decoder->p_sections = p;
                b_overwrite = true;
            }
            goto out;
        }
        else if (p->i_number > p_section->i_number)
        {
            /* Insert before p */
            if (p_prev)
            {
                p_prev->p_next = p_section;
                p_section->p_next = p;
            }
            else
            {
                p_section->p_next = p;
                p_decoder->p_sections = p_section;
            }
            goto out;
        }

        p_prev = p;
        p = p->p_next;
    }

    /* Add to end of list */
    if (p_prev->i_number < p_section->i_number)
    {
        p_prev->p_next = p_section;
        p_section->p_next = NULL;
    }

out:
    return b_overwrite;
}

/*****************************************************************************
 * dvbpsi_decoder_delete
 *****************************************************************************/
void dvbpsi_decoder_delete(dvbpsi_decoder_t *p_decoder)
{
    assert(p_decoder);

    if (p_decoder->p_sections)
    {
        dvbpsi_DeletePSISections(p_decoder->p_sections);
        p_decoder->p_sections = NULL;
    }

    dvbpsi_DeletePSISections(p_decoder->p_current_section);
    free(p_decoder);
}

/*****************************************************************************
 * dvbpsi_decoder_present
 *****************************************************************************/
bool dvbpsi_decoder_present(dvbpsi_t *p_dvbpsi)
{
    if (p_dvbpsi && p_dvbpsi->p_decoder)
        return true;
    else
        return false;
}

/*****************************************************************************
 * dvbpsi_packet_push
 *****************************************************************************
 * Injection of a TS packet into a PSI decoder.
 *****************************************************************************/
bool dvbpsi_packet_push(dvbpsi_t *p_dvbpsi, uint8_t* p_data)
{
    uint8_t i_expected_counter;           /* Expected continuity counter */
    dvbpsi_psi_section_t* p_section;      /* Current section */
    uint8_t* p_payload_pos;               /* Where in the TS packet */
    uint8_t* p_new_pos = NULL;            /* Beginning of the new section,
                                             updated to NULL when the new
                                             section is handled */
    int i_available;                      /* Byte count available in the
                                             packet */

    dvbpsi_decoder_t *p_decoder = p_dvbpsi->p_decoder;
    assert(p_decoder);

    /* TS start code */
    if (p_data[0] != 0x47)
    {
        dvbpsi_error(p_dvbpsi, "PSI decoder", "not a TS packet");
        return false;
    }

    /* Continuity check */
    bool b_first = (p_decoder->i_continuity_counter == DVBPSI_INVALID_CC);
    if (b_first)
        p_decoder->i_continuity_counter = p_data[3] & 0xf;
    else
    {
        i_expected_counter = (p_decoder->i_continuity_counter + 1) & 0xf;
        p_decoder->i_continuity_counter = p_data[3] & 0xf;

        if (i_expected_counter == ((p_decoder->i_continuity_counter + 1) & 0xf)
            && !p_decoder->b_discontinuity)
        {
            dvbpsi_error(p_dvbpsi, "PSI decoder",
                     "TS duplicate (received %d, expected %d) for PID %d",
                     p_decoder->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);
            return false;
        }

        if (i_expected_counter != p_decoder->i_continuity_counter)
        {
            dvbpsi_error(p_dvbpsi, "PSI decoder",
                     "TS discontinuity (received %d, expected %d) for PID %d",
                     p_decoder->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);
            p_decoder->b_discontinuity = true;
            if (p_decoder->p_current_section)
            {
                dvbpsi_DeletePSISections(p_decoder->p_current_section);
                p_decoder->p_current_section = NULL;
            }
        }
    }

    /* Return if no payload in the TS packet */
    if (!(p_data[3] & 0x10))
        return false;

    /* Skip the adaptation_field if present */
    if (p_data[3] & 0x20)
        p_payload_pos = p_data + 5 + p_data[4];
    else
        p_payload_pos = p_data + 4;

    /* Unit start -> skip the pointer_field and a new section begins */
    if (p_data[1] & 0x40)
    {
        p_new_pos = p_payload_pos + *p_payload_pos + 1;
        p_payload_pos += 1;
    }

    p_section = p_decoder->p_current_section;

    /* If the psi decoder needs a beginning of a section and a new section
       begins in the packet then initialize the dvbpsi_psi_section_t structure */
    if (p_section == NULL)
    {
        if (p_new_pos)
        {
            /* Allocation of the structure */
            p_decoder->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(p_decoder->i_section_max_size);
            if (!p_section)
                return false;
            /* Update the position in the packet */
            p_payload_pos = p_new_pos;
            /* New section is being handled */
            p_new_pos = NULL;
            /* Just need the header to know how long is the section */
            p_decoder->i_need = 3;
            p_decoder->b_complete_header = false;
        }
        else
        {
            /* No new section => return */
            return false;
        }
    }

    /* Remaining bytes in the payload */
    i_available = 188 + p_data - p_payload_pos;

    while (i_available > 0)
    {
        if (i_available >= p_decoder->i_need)
        {
            /* There are enough bytes in this packet to complete the
               header/section */
            memcpy(p_section->p_payload_end, p_payload_pos, p_decoder->i_need);
            p_payload_pos += p_decoder->i_need;
            p_section->p_payload_end += p_decoder->i_need;
            i_available -= p_decoder->i_need;

            if (!p_decoder->b_complete_header)
            {
                /* Header is complete */
                p_decoder->b_complete_header = true;
                /* Compute p_section->i_length and update p_decoder->i_need */
                p_decoder->i_need = p_section->i_length
                                  = ((uint16_t)(p_section->p_data[1] & 0xf)) << 8
                                       | p_section->p_data[2];
                /* Check that the section isn't too long */
                if (p_decoder->i_need > p_decoder->i_section_max_size - 3)
                {
                    dvbpsi_error(p_dvbpsi, "PSI decoder", "PSI section too long");
                    dvbpsi_DeletePSISections(p_section);
                    p_decoder->p_current_section = NULL;
                    /* If there is a new section not being handled then go forward
                       in the packet */
                    if (p_new_pos)
                    {
                        p_decoder->p_current_section
                                    = p_section
                                    = dvbpsi_NewPSISection(p_decoder->i_section_max_size);
                        if (!p_section)
                            return false;
                        p_payload_pos = p_new_pos;
                        p_new_pos = NULL;
                        p_decoder->i_need = 3;
                        p_decoder->b_complete_header = false;
                        i_available = 188 + p_data - p_payload_pos;
                    }
                    else
                    {
                        i_available = 0;
                    }
                }
            }
            else
            {
                bool b_valid_crc32 = false;
                bool has_crc32;

                /* PSI section is complete */
                p_section->i_table_id = p_section->p_data[0];
                p_section->b_syntax_indicator = p_section->p_data[1] & 0x80;
                p_section->b_private_indicator = p_section->p_data[1] & 0x40;

                /* Update the end of the payload if CRC_32 is present */
                has_crc32 = dvbpsi_has_CRC32(p_section);
                if (p_section->b_syntax_indicator || has_crc32)
                    p_section->p_payload_end -= 4;

                /* Check CRC32 if present */
                if (has_crc32)
                    b_valid_crc32 = dvbpsi_ValidPSISection(p_section);

                if (!has_crc32 || b_valid_crc32)
                {
                    /* PSI section is valid */
                    if (p_section->b_syntax_indicator)
                    {
                        p_section->i_extension =  (p_section->p_data[3] << 8)
                                                 | p_section->p_data[4];
                        p_section->i_version = (p_section->p_data[5] & 0x3e) >> 1;
                        p_section->b_current_next = p_section->p_data[5] & 0x1;
                        p_section->i_number = p_section->p_data[6];
                        p_section->i_last_number = p_section->p_data[7];
                        p_section->p_payload_start = p_section->p_data + 8;
                    }
                    else
                    {
                        p_section->i_extension = 0;
                        p_section->i_version = 0;
                        p_section->b_current_next = true;
                        p_section->i_number = 0;
                        p_section->i_last_number = 0;
                        p_section->p_payload_start = p_section->p_data + 3;
                    }
                    if (p_decoder->pf_gather)
                        p_decoder->pf_gather(p_dvbpsi, p_section);
                    p_decoder->p_current_section = NULL;
                }
                else
                {
                    if (has_crc32 && !dvbpsi_ValidPSISection(p_section))
                        dvbpsi_error(p_dvbpsi, "misc PSI", "Bad CRC_32 table 0x%x !!!",
                                               p_section->p_data[0]);
                    else
                        dvbpsi_error(p_dvbpsi, "misc PSI", "table 0x%x", p_section->p_data[0]);

                    /* PSI section isn't valid => trash it */
                    dvbpsi_DeletePSISections(p_section);
                    p_decoder->p_current_section = NULL;
                }

                /* A TS packet may contain any number of sections, only the first
                 * new one is flagged by the pointer_field. If the next payload
                 * byte isn't 0xff then a new section starts. */
                if (p_new_pos == NULL && i_available && *p_payload_pos != 0xff)
                    p_new_pos = p_payload_pos;

                /* If there is a new section not being handled then go forward
                   in the packet */
                if (p_new_pos)
                {
                    p_decoder->p_current_section
                              = p_section
                              = dvbpsi_NewPSISection(p_decoder->i_section_max_size);
                    if (!p_section)
                        return false;
                    p_payload_pos = p_new_pos;
                    p_new_pos = NULL;
                    p_decoder->i_need = 3;
                    p_decoder->b_complete_header = false;
                    i_available = 188 + p_data - p_payload_pos;
                }
                else
                {
                    i_available = 0;
                }
            }
        }
        else
        {
            /* There aren't enough bytes in this packet to complete the
               header/section */
            memcpy(p_section->p_payload_end, p_payload_pos, i_available);
            p_section->p_payload_end += i_available;
            p_decoder->i_need -= i_available;
            i_available = 0;
        }
    }
    return true;
}
#undef DVBPSI_INVALID_CC

/*****************************************************************************
 * Message error level:
 * -1 is disabled,
 *  0 is errors only
 *  1 is warning and errors
 *  2 is debug, warning and errors
 *****************************************************************************/
#if !defined(HAVE_ASPRINTF)
#   define DVBPSI_MSG_SIZE 1024
#endif

#define DVBPSI_MSG_FORMAT "libdvbpsi (%s): "

#ifdef HAVE_VARIADIC_MACROS
void dvbpsi_message(dvbpsi_t *dvbpsi, const dvbpsi_msg_level_t level, const char *fmt, ...)
{
    if ((dvbpsi->i_msg_level > DVBPSI_MSG_NONE) &&
        (level <= dvbpsi->i_msg_level))
    {
        va_list ap;
        va_start(ap, fmt);
        char *msg = NULL;
#if defined(HAVE_ASPRINTF)
        int err = vasprintf(&msg, fmt, ap);
#else
        msg = malloc(DVBPSI_MSG_SIZE);
        if (msg == NULL) {
            va_end(ap);
            return;
        }
        int err = vsnprintf(msg, DVBPSI_MSG_SIZE, DVBPSI_MSG_FORMAT fmt, ap);
#endif
        va_end(ap);
        if (err > 0) {
            if (dvbpsi->pf_message)
                dvbpsi->pf_message(dvbpsi, level, msg);
        }
        free(msg);
    }
}
#else

/* Common code for printing messages */
#   if defined(HAVE_ASPRINTF)
#       define DVBPSI_MSG_COMMON(level)                         \
    do {                                                        \
        va_list ap;                                             \
        va_start(ap, fmt);                                      \
        char *tmp = NULL;                                       \
        int err = vasprintf(&tmp, fmt, ap);                     \
        if (err < 0) {                                          \
            va_end(ap);                                         \
            return;                                             \
        }                                                       \
        char *msg = NULL;                                       \
        if (asprintf(&msg, DVBPSI_MSG_FORMAT "%s", src, tmp) < 0) { \
            va_end(ap);                                         \
            free(tmp);                                          \
            return;                                             \
        }                                                       \
        free(tmp);                                              \
        va_end(ap);                                             \
        if (err > 0) {                                          \
            if (dvbpsi->pf_message)                             \
                dvbpsi->pf_message(dvbpsi, level, msg);         \
        }                                                       \
        free(msg);                                              \
    } while(0);
#   else
#       define DVBPSI_MSG_COMMON(level)                         \
    do {                                                        \
        va_list ap;                                             \
        va_start(ap, fmt);                                      \
        char *tmp = malloc(DVBPSI_MSG_SIZE);                    \
        char *msg = malloc(DVBPSI_MSG_SIZE);                    \
        if ((tmp == NULL) || (msg == NULL)) {                   \
            va_end(ap);                                         \
            if (tmp) free(tmp);                                 \
            if (msg) free(msg);                                 \
            return;                                             \
        }                                                       \
        if (vsnprintf(tmp, DVBPSI_MSG_SIZE, fmt, ap) < 0) {     \
            va_end(ap);                                         \
            if (tmp) free(tmp);                                 \
            if (msg) free(msg);                                 \
            return;                                             \
        }                                                       \
        va_end(ap);                                             \
        int err = snprintf(msg, DVBPSI_MSG_SIZE, DVBPSI_MSG_FORMAT "%s", src, tmp); \
        if (err > 0) {                                          \
            if (dvbpsi->pf_message)                             \
                dvbpsi->pf_message(dvbpsi, level, msg);         \
        }                                                       \
        free(tmp);                                              \
        free(msg);                                              \
    } while(0);
#   endif

void dvbpsi_error(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...)
{
    if ((dvbpsi->i_msg_level > DVBPSI_MSG_NONE) &&
        (DVBPSI_MSG_ERROR <= dvbpsi->i_msg_level))
    {
        DVBPSI_MSG_COMMON(DVBPSI_MSG_ERROR)
    }
}

void dvbpsi_warning(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...)
{
    if ((dvbpsi->i_msg_level > DVBPSI_MSG_NONE) &&
        (DVBPSI_MSG_WARN <= dvbpsi->i_msg_level))
    {
        DVBPSI_MSG_COMMON(DVBPSI_MSG_WARN)
    }
}

void dvbpsi_debug(dvbpsi_t *dvbpsi, const char *src, const char *fmt, ...)
{
    if ((dvbpsi->i_msg_level > DVBPSI_MSG_NONE) &&
        (DVBPSI_MSG_DEBUG <= dvbpsi->i_msg_level))
    {
        DVBPSI_MSG_COMMON(DVBPSI_MSG_DEBUG)
    }
}
#endif
