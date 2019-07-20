/*
Copyright (C) 2010  Adam Charrett

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

dr_66.h

Decode Data Broadcast id Descriptor.

*/

/*!
 * \file dr_66.h
 * \author Adam Charrett
 * \brief Decode Data Broadcast id Descriptor.
 */

#ifndef _DR_66_H
#define _DR_66_H

/*****************************************************************************
 * dvbpsi_data_broadcast_id_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_data_broadcast_id_dr_s
 * \brief Data Broadcast id Descriptor
 *
 * This structure is used to store a decoded Data Broadcast id descriptor.
 */
/*!
 * \typedef struct dvbpsi_data_broadcast_id_dr_s dvbpsi_data_broadcast_id_dr_t
 * \brief dvbpsi_data_broadcast_id_dr_t type definition.
 */
typedef struct dvbpsi_data_broadcast_id_dr_s
{
    uint16_t i_data_broadcast_id; /*!< broadcast identifier */
    uint8_t  i_id_selector_len;   /*!< length of selector data in bytes */
    uint8_t  *p_id_selector;      /*!< pointer to selector data. Memory is allocated
                                      right after sizeof struct, when freeing this
                                      struct the private data is freed at the same time. */
} dvbpsi_data_broadcast_id_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeDataBroadcastIdDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_data_broadcast_id_dr_t *dvbpsi_DecodeDataBroadcastIdDr(
 *        dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Data broadcast id descriptor (tag 0x66)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_data_broadcast_id_dr_t structure.
 */
dvbpsi_data_broadcast_id_dr_t *dvbpsi_DecodeDataBroadcastIdDr(dvbpsi_descriptor_t *p_descriptor);

#endif
