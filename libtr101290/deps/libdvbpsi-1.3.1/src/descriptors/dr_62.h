/*
Copyright (C) 2006  Adam Charrett

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

dr_62.h

Decode Frequency List Descriptor.

*/

/*!
 * \file dr_62.h
 * \author Adam Charrett
 * \brief Decode Frequency List Descriptor.
 */

#ifndef _DR_62_H
#define _DR_62_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_frequency_list_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_frequency_list_dr_s
 * \brief Frequency List Descriptor
 *
 * This structure is used to store a decoded Frequency List descriptor.
 */
/*!
 * \typedef struct dvbpsi_frequency_list_dr_s dvbpsi_frequency_list_dr_t
 * \brief dvbpsi_frequency_list_dr_t type definition.
 */
typedef struct dvbpsi_frequency_list_dr_s
{
    uint8_t i_coding_type;             /*!< Coding type, 1 = Satellite, 2 = Cable, 3 = Terrestrial */
    uint8_t i_number_of_frequencies;   /*!< Number of center frequencies present */
    uint32_t p_center_frequencies[63]; /*!< Center frequency as defined by a delivery_system_descriptor */
}dvbpsi_frequency_list_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeFrequencyListDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_frequency_list_dr_t *dvbpsi_DecodeFrequencyListDr(
 *        dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Frequency List descriptor (tag 0x62)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_frequency_list_dr_t structure.
 */
dvbpsi_frequency_list_dr_t *dvbpsi_DecodeFrequencyListDr(dvbpsi_descriptor_t *p_descriptor);

/*****************************************************************************
 * dvbpsi_Bcd8ToUint32
 *****************************************************************************/
/*!
 * \fn uint32_t dvbpsi_Bcd8ToUint32(uint32_t bcd)
 * \brief Decode an 8 character BCD encoded number into a uint32_t.
 * \param bcd BCD value to convert.
 * \return Decoded value as a uint32_t.
 */
uint32_t dvbpsi_Bcd8ToUint32(uint32_t bcd);

#ifdef __cplusplus
};
#endif

#endif
