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

dr_76.h

Decode Content Identifier Descriptor.

*/

/*!
 * \file dr_76.h
 * \author Adam Charrett
 * \brief Decode Content Identifier Descriptor
 */

#ifndef _DR_76_H
#define _DR_76_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \def CRID_TYPE_UNDEFINED
 * \brief Content Resource Identifier Descriptor for undefined content
 *
 * \def CRID_TYPE_CONTENT
 * \brief Content Resource Identifier Descriptor for type content
 *
 * \def CRID_TYPE_SERIES
 * \brief Content Resource Identifier Descriptor for type series content
 *
 * \def CRID_TYPE_RECOMMENDATION
 * \brief Content Resource Identifier Descriptor for type recommended content
 */
#define CRID_TYPE_UNDEFINED      0
#define CRID_TYPE_CONTENT        1
#define CRID_TYPE_SERIES         2
#define CRID_TYPE_RECOMMENDATION 3

/*!
 * \def CRID_LOCATION_DESCRIPTOR
 * \brief Content Resource Identifier Descriptor for location
 *
 * \def CRID_LOCATION_CIT
 * \brief Content Resource Identifier Descriptor for CIT(?)
 */
#define CRID_LOCATION_DESCRIPTOR 0
#define CRID_LOCATION_CIT        1

/*****************************************************************************
 * dvbpsi_lcn_entry_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_crid_entry_s
 * \brief CRID Entry
 *
 * This structure is used to store a decoded CRID entry.
 */
/*!
 * \typedef struct dvbpsi_crid_entry_s dvbpsi_crid_entry_t
 * \brief dvbpsi_crid_entry_t type definition.
 */
typedef struct dvbpsi_crid_entry_s
{
    uint8_t i_type;         /*!< content type */
    uint8_t i_location;     /*!< content location */
    union
    {
        uint8_t  path[253]; /*!< content path */
        uint16_t ref;       /*!< content reference */
    } value;                /*!< content specific value */
} dvbpsi_crid_entry_t;

/*!
 * \def DVBPSI_CRID_ENTRY_DR_MAX
 * \brief Maximum number of dvbpsi_crid_entry_t entries present in
 * @see dvbpsi_content_id_dr_t
 */
#define DVBPSI_CRID_ENTRY_DR_MAX 85

/*****************************************************************************
 * dvbpsi_content_id_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_id_dr_s
 * \brief Content Identifier Descriptor
 *
 * This structure is used to store a decoded Content Identifier descriptor.
 */
/*!
 * \typedef struct dvbpsi_content_id_dr_s dvbpsi_content_id_dr_t
 * \brief dvbpsi_content_id_dr_t type definition.
 */
typedef struct dvbpsi_content_id_dr_s
{
    uint8_t i_number_of_entries;      /*!< Number of CRID entries present. */
    dvbpsi_crid_entry_t p_entries[DVBPSI_CRID_ENTRY_DR_MAX];/*!< Array of CRID entries. */
} dvbpsi_content_id_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeLCNDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_content_id_dr_s dvbpsi_DecodeContentIdDr(dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Content Identifier descriptor (tag 0x76)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_content_id_dr_t structure.
 */
dvbpsi_content_id_dr_t *dvbpsi_DecodeContentIdDr(dvbpsi_descriptor_t *p_descriptor);

#ifdef __cplusplus
};
#endif

#endif


