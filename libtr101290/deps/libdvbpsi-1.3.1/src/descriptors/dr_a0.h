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

dr_a0.h

Decode Extended Channel Name Descriptor.

*/

/*!
 * \file dr_a0.h
 * \author Michael Ira Krufky
 * \brief Decode Extended Channel Name Descriptor.
 */

#ifndef _DR_A0_H
#define _DR_A0_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_extended_channel_name_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_extended_channel_name_dr_s
 * \brief Extended Channel Name Descriptor
 *
 * This structure is used to store a decoded Extended Channel Name descriptor.
 */
/*!
 * \typedef struct dvbpsi_extended_channel_name_dr_s dvbpsi_extended_channel_name_dr_t
 * \brief dvbpsi_extended_channel_name_dr_t type definition.
 */
typedef struct dvbpsi_extended_channel_name_dr_s
{
    uint8_t    i_long_channel_name_length;  /*!< Length in bytes */
    uint8_t    i_long_channel_name[256];    /*!< multiple string structure format. */

}dvbpsi_extended_channel_name_dr_t;

/*****************************************************************************
 * dvbpsi_ExtendedChannelNameDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_extended_channel_name_dr_t dvbpsi_ExtendedChannelNameDr(dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Extended Channel Name descriptor (tag 0xA0)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_extended_channel_name_dr_t structure.
 */
dvbpsi_extended_channel_name_dr_t *dvbpsi_ExtendedChannelNameDr(dvbpsi_descriptor_t *p_descriptor);

#ifdef __cplusplus
}
#endif

#endif

