/*
Copyright (C) 2006  Adam Charrett
Copyright (C) 2011-2012  Michael Krufky

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

mgt.h

Decode PSIP Master Guide Table.

*/

/*!
 * \file atsc_mgt.h
 * \author Adam Charrett and Michael Krufky
 * \brief Decode PSIP Master Guide Table (ATSC MGT).
 */

#ifndef _ATSC_MGT_H
#define _ATSC_MGT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_mgt_table_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_mgt_table_s
 * \brief MGT table structure.
 *
 * This structure is used to store a decoded MGT table information.
 */
/*!
 * \typedef struct dvbpsi_atsc_mgt_table_s dvbpsi_atsc_mgt_table_t
 * \brief dvbpsi_atsc_mgt_table_t type definition.
 */
typedef struct dvbpsi_atsc_mgt_table_s
{
    uint16_t                    i_table_type;       /*!< type of table */
    uint16_t                    i_table_type_pid;   /*!< PID of table */
    uint8_t                     i_table_type_version; /*!< version of table */
    uint32_t                    i_number_bytes;     /*!< bytes used for table */

    dvbpsi_descriptor_t        *p_first_descriptor; /*!< First descriptor. */

    struct dvbpsi_atsc_mgt_table_s *p_next;         /*!< next element of the list */
} dvbpsi_atsc_mgt_table_t;

/*****************************************************************************
 * dvbpsi_atsc_mgt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_mgt_s
 * \brief MGT structure.
 *
 * This structure is used to store a decoded MGT.
 */
/*!
 * \typedef struct dvbpsi_atsc_mgt_s dvbpsi_atsc_mgt_t
 * \brief dvbpsi_atsc_mgt_t type definition.
 */
typedef struct dvbpsi_atsc_mgt_s
{
    uint8_t                 i_table_id;         /*!< Table id */
    uint16_t                i_extension;        /*!< Subtable id */

    uint8_t                 i_version;          /*!< version_number */
    bool                    b_current_next;     /*!< current_next_indicator */
    uint16_t                i_table_id_ext;     /*!< 0x0000 */
    uint8_t                 i_protocol;         /*!< PSIP Protocol version */

    dvbpsi_atsc_mgt_table_t   *p_first_table;   /*!< First table information structure. */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_mgt_t;

/*****************************************************************************
 * dvbpsi_mgt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_mgt_callback)(void* p_cb_data,
                                         dvbpsi_atsc_mgt_t* p_new_mgt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_mgt_callback)(void* p_cb_data, dvbpsi_atsc_mgt_t* p_new_mgt);

/*****************************************************************************
 * dvbpsi_atsc_AttachMGT
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_atsc_AttachMGT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data)
 *
 * \brief Creation and initialization of a MGT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \param pf_callback function to call back on new MGT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true if everything went ok, false otherwise
 */
bool dvbpsi_atsc_AttachMGT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                           dvbpsi_atsc_mgt_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_DetachMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachMGT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
 *
 * \brief Destroy a MGT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \return nothing.
 */
void dvbpsi_atsc_DetachMGT(dvbpsi_t * p_dvbpsi, uint8_t i_table_id, uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_atsc_InitMGT/dvbpsi_atsc_NewMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt, uint8_t i_table_id, uint16_t i_extension,
                                uint8_t i_version, uint8_t i_protocol, bool b_current_next);
 * \brief Initialize a user-allocated dvbpsi_atsc_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \param i_version MGT version
 * \param i_protocol PSIP Protocol version.
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_atsc_InitMGT(dvbpsi_atsc_mgt_t* p_mgt, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_version, uint8_t i_protocol, bool b_current_next);

/*!
 * \fn dvbpsi_atsc_mgt_t *dvbpsi_atsc_NewMGT(uint8_t i_table_id, uint16_t i_extension,
                        uint8_t i_version, uint8_t i_protocol, bool b_current_next);
 * \brief Allocate and initialize a new dvbpsi_mgt_t structure.
 * \param i_table_id Table ID, 0xC7.
 * \param i_extension Table ID extension, here 0x0000.
 * \param i_version MGT version
 * \param i_protocol PSIP Protocol version.
 * \param b_current_next current next indicator
 * \return p_mgt pointer to the MGT structure, or NULL on failure
 */
dvbpsi_atsc_mgt_t *dvbpsi_atsc_NewMGT(uint8_t i_table_id, uint16_t i_extension,
                        uint8_t i_version, uint8_t i_protocol, bool b_current_next);

/*****************************************************************************
 * dvbpsi_atsc_EmptyMGT/dvbpsi_atsc_DeleteMGT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t* p_mgt)
 * \brief Clean a dvbpsi_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptyMGT(dvbpsi_atsc_mgt_t *p_mgt);

/*!
 * \fn void dvbpsi_atsc_DeleteMGT(dvbpsi_atsc_mgt_t *p_mgt);
 * \brief Clean and free a dvbpsi_mgt_t structure.
 * \param p_mgt pointer to the MGT structure
 * \return nothing.
 */
void dvbpsi_atsc_DeleteMGT(dvbpsi_atsc_mgt_t *p_mgt);

#ifdef __cplusplus
};
#endif

#endif
