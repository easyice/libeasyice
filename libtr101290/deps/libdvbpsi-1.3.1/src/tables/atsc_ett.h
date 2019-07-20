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

ett.h

*/

/*!
 * \file atsc_ett.h
 * \author Adam Charrett
 * \brief Decode PSIP Extented Text Table (ATSC ETT).
 */

#ifndef _ATSC_ETT_H
#define _ATSC_ETT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_ett_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_ett_s
 * \brief ATSC ETT structure.
 *
 * The Extended Text Table (ETT) contains Extended Text Message (ETM) streams. They
 * provide detailed descriptions of virtual channels (channel ETM) and (event ETM).
 * An ETM consist of a multiple string data structure (see Section 6.10), and thus, it
 * may represent a description in several different languages (each string corresponding
 * to one language).
 */
/*!
 * \typedef struct dvbpsi_atsc_ett_s dvbpsi_atsc_ett_t
 * \brief dvbpsi_atsc_ett_t type definition.
 *
 * This structure is used to store a decoded ETT.
 * (ATSC document A/56-2009, section 6.6.)
 */
typedef struct dvbpsi_atsc_ett_s
{
    /* general PSI table */
    uint8_t                 i_table_id;     /*!< table id */
    uint16_t                i_extension;    /*!< subtable id:
                                                 ETT Table ID extension,
                                                 normally 0x0000 */

    uint8_t                 i_version;      /*!< version_number */
    bool                    b_current_next; /*!< current_next_indicator */
    uint8_t                 i_protocol;     /*!< PSIP Protocol version */

    /* ETT specific */
    uint32_t                i_etm_id;       /*!< ETM Identifier, made up of
                                                 source id and event id
                                                 (or 0 for channel ETT) */
    uint32_t                i_etm_length;   /*!< length of p_etm_data */
    uint8_t                 *p_etm_data;    /*!< ETM data organized as a
                                                 multiple string structure */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_ett_t;

/*****************************************************************************
 * dvbpsi_atsc_ett_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_ett_callback)(void* p_cb_data,
                                         dvbpsi_atsc_ett_t* p_new_ett)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_ett_callback)(void* p_cb_data, dvbpsi_atsc_ett_t* p_new_ett);

/*****************************************************************************
 * dvbpsi_atsc_AttachETT
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_atsc_AttachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a ETT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached
 * \param i_table_id Table ID, 0xCC.
 * \param i_extension Table ID extension, normally 0x0000.
 * \param pf_callback function to call back on new ETT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true if everything went ok, else it returns false.
 */
bool dvbpsi_atsc_AttachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_ett_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_atsc_DetachETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
 *
 * \brief Destroy a ETT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table ID extension, normally 0x0000.
 * \return nothing.
 */
void dvbpsi_atsc_DetachETT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_atsc_InitETT/dvbpsi_atsc_NewETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_version, uint8_t i_protocol,  uint32_t i_etm_id, bool b_current_next);
 * \brief Initialize a user-allocated dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \param i_table_id Table ID, 0xCC.
 * \param i_extension Table ID extension, normally 0x0000.
 * \param i_version version
 * \param i_protocol PSIP Protocol version.
 * \param i_etm_id ETM Identifier.
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_atsc_InitETT(dvbpsi_atsc_ett_t *p_ett, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_version, uint8_t i_protocol,
                         uint32_t i_etm_id, bool b_current_next);

/*!
 * \fn dvbpsi_atsc_ett_t *dvbpsi_atsc_NewETT(uint8_t i_table_id, uint16_t i_extension,
                          uint8_t i_version, uint8_t i_protocol,
                          uint32_t i_etm_id, bool b_current_next)
 * \brief Allocate and initialize a new dvbpsi_atsc_ett_t structure. Use ObjectRefDec to delete it.
 * \param i_table_id Table ID, 0xCC.
 * \param i_extension Table ID extension, normally 0x0000.
 * \param i_version version
 * \param i_protocol PSIP Protocol version.
 * \param i_etm_id ETM Identifier.
 * \param b_current_next current next indicator
 * \returns p_ett pointer to the ETT structure, NULL otherwise
 */
dvbpsi_atsc_ett_t *dvbpsi_atsc_NewETT(uint8_t i_table_id, uint16_t i_extension,
                                      uint8_t i_version, uint8_t i_protocol,
                                      uint32_t i_etm_id, bool b_current_next);

/*****************************************************************************
 * dvbpsi_atsc_EmptyETT/dvbpsi_atsc_DeleteETT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t* p_ett)
 * \brief Clean a dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyETT(dvbpsi_atsc_ett_t *p_ett);

/*!
 * \fn void dvbpsi_atsc_DeleteETT(dvbpsi_atsc_ett_t *p_ett);
 * \brief Clean and free a dvbpsi_atsc_ett_t structure.
 * \param p_ett pointer to the ETT structure
 * \return nothing.
 */
void dvbpsi_atsc_DeleteETT(dvbpsi_atsc_ett_t *p_ett);

#ifdef __cplusplus
};
#endif

#endif
