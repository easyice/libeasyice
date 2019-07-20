/*
Copyright (C) 2013-2014  Michael Ira Krufky

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

dr_81.c

Decode AC-3 Audio Descriptor.

*/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../dvbpsi.h"
#include "../dvbpsi_private.h"
#include "../descriptor.h"

#include "dr_81.h"

/*****************************************************************************
 * dvbpsi_DecodeAc3AudioDr
 *****************************************************************************/
dvbpsi_ac3_audio_dr_t *dvbpsi_DecodeAc3AudioDr(dvbpsi_descriptor_t *p_descriptor)
{
    dvbpsi_ac3_audio_dr_t *p_decoded;
    uint8_t * buf = p_descriptor->p_data;

    /* Check the tag */
    if (!dvbpsi_CanDecodeAsDescriptor(p_descriptor, 0x81))
        return NULL;

    /* Don't decode twice */
    if (dvbpsi_IsDescriptorDecoded(p_descriptor))
        return p_descriptor->p_decoded;

    /* Check length */
    if (p_descriptor->i_length < 3)
        return NULL;

    p_decoded = (dvbpsi_ac3_audio_dr_t*)calloc(1, sizeof(dvbpsi_ac3_audio_dr_t));
    if (!p_decoded)
        return NULL;

    p_descriptor->p_decoded = (void*)p_decoded;

    p_decoded->i_sample_rate_code = 0x07 & (buf[0] >> 5);
    p_decoded->i_bsid             = 0x1f & buf[0];
    p_decoded->i_bit_rate_code    = 0x3f & (buf[1] >> 2);
    p_decoded->i_surround_mode    = 0x03 & buf[1];
    p_decoded->i_bsmod            = 0x07 & (buf[2] >> 5);
    p_decoded->i_num_channels     = 0x0f & (buf[2] >> 1);
    p_decoded->b_full_svc         = 0x01 & buf[2];
    buf += 3;
    if (buf == p_descriptor->p_data + p_descriptor->i_length)
        return p_decoded;

    p_decoded->i_lang_code = buf[0];
    buf++;

    if (buf == p_descriptor->p_data + p_descriptor->i_length)
        return p_decoded;

    if (!p_decoded->i_num_channels) {
        p_decoded->i_lang_code2 = buf[0];
        buf++;
    }

    if (buf == p_descriptor->p_data + p_descriptor->i_length)
        return p_decoded;

    if (p_decoded->i_bsmod < 2) {
        p_decoded->i_mainid       = 0x07 & (buf[0] >> 5);
        p_decoded->i_priority     = 0x03 & (buf[0] >> 3);
    } else
        p_decoded->i_asvcflags = buf[0];
    buf++;

    if (buf == p_descriptor->p_data + p_descriptor->i_length)
        return p_decoded;

    p_decoded->i_textlen   = 0x7f & (buf[0] >> 1);
    p_decoded->b_text_code = 0X01 & buf[0];
    buf++;

    memset(p_decoded->text, 0, sizeof(p_decoded->text));
    memcpy(p_decoded->text, buf, p_decoded->i_textlen);
    buf += p_decoded->i_textlen;

    if (buf == p_descriptor->p_data + p_descriptor->i_length)
        return p_decoded;

    p_decoded->b_language_flag   = 0x01 & (buf[0] >> 7);
    p_decoded->b_language_flag_2 = 0x01 & (buf[0] >> 6);
    buf++;

    if (p_decoded->b_language_flag) {
        memcpy(p_decoded->language, buf, 3);
        buf += 3;
    }
    if (p_decoded->b_language_flag_2) {
        memcpy(p_decoded->language_2, buf, 3);
        buf += 3;
    }
    return p_decoded;
}
