/*****************************************************************************
 * bat.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: bat.h  $
 *
 * Authors: Zhu zhenglu <zhuzlu@gmail.com>
 *          heavily based on nit.h which was written by
 *          Johann Hanne
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
 *
 *****************************************************************************/

/*!
 * \file bat.h
 * \author Zhu zhenglu <zhuzlu@gmail.com>
 * \brief Application interface for the BAT decoder.
 *
 * Application interface for the BAT decoder. New
 * decoded BAT are sent by callback to the application. If a table
 * wasn't active (b_current_next == false) and the next is the same but active
 * (b_current_next == true) then the service description list is empty and should
 * be caught from the previous structure.
 * This is a simulation to sdt.h
 */

#ifndef _DVBPSI_BAT_H_
#define _DVBPSI_BAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_bat_ts_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_bat_ts_s
 * \brief BAT transport stream description structure.
 *
 * This structure is used to store a decoded BAT service description.
 * (ETSI EN 300 468 V1.5.1 section 5.2.2).
 */
/*!
 * \typedef struct dvbpsi_bat_ts_s dvbpsi_bat_ts_t
 * \brief dvbpsi_bat_ts_t type definition.
 */
typedef struct dvbpsi_bat_ts_s
{
    uint16_t                i_ts_id;            /*!< transport stream id */
    uint16_t                i_orig_network_id;  /*!< original network id */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< descriptor list */

    struct dvbpsi_bat_ts_s *p_next;             /*!< next element of
                                                             the list */

} dvbpsi_bat_ts_t;


/*****************************************************************************
 * dvbpsi_bat_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_bat_s
 * \brief BAT structure.
 *
 * This structure is used to store a decoded BAT.
 * (ETSI EN 300 468 V1.5.1 section 5.2.2).
 */
/*!
 * \typedef struct dvbpsi_bat_s dvbpsi_bat_t
 * \brief dvbpsi_bat_t type definition.
 */
typedef struct dvbpsi_bat_s
{
    uint8_t                 i_table_id;         /*!< table id */
    uint16_t                i_extension;        /*!< subtable id (here bouquet id) */

    uint8_t                 i_version;          /*!< version_number */
    bool                    b_current_next;     /*!< current_next_indicator */

    dvbpsi_descriptor_t *   p_first_descriptor; /*!< descriptor list */

    dvbpsi_bat_ts_t *       p_first_ts;         /*!< transport stream
                                                     description list */

} dvbpsi_bat_t;

/*****************************************************************************
 * dvbpsi_bat_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_bat_callback)(void* p_cb_data,
                                         dvbpsi_bat_t* p_new_bat)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_bat_callback)(void* p_cb_data, dvbpsi_bat_t* p_new_bat);

/*****************************************************************************
 * dvbpsi_bat_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_bat_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_bat_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a BAT decoder. It will be attached to p_dvbpsi.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \param pf_callback function to call back on new BAT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true on success, false on failure
 */
bool dvbpsi_bat_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                       dvbpsi_bat_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_bat_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_bat_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a BAT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \return nothing.
 */
void dvbpsi_bat_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_bat_init/dvbpsi_bat_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_bat_init(dvbpsi_bat_t *p_bat, uint8_t i_table_id, uint16_t i_extension,
                            uint8_t i_version, bool b_current_next)
 * \brief Initialize a user-allocated dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \param i_version BAT version
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_bat_init(dvbpsi_bat_t *p_bat, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next);

/*!
 * \fn dvbpsi_bat_t *dvbpsi_bat_new(uint8_t i_table_id, uint16_t i_extension,
 *                                  uint8_t i_version, bool b_current_next)
 * \brief Allocate and initialize a new dvbpsi_bat_t structure.
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \param i_version BAT version
 * \param b_current_next current next indicator
 * \return p_bat pointer to the BAT structure
 */
dvbpsi_bat_t *dvbpsi_bat_new(uint8_t i_table_id, uint16_t i_extension,
                             uint8_t i_version, bool b_current_next);

/*****************************************************************************
 * dvbpsi_bat_empty/dvbpsi_bat_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_bat_empty(dvbpsi_bat_t* p_bat)
 * \brief Clean a dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \return nothing.
 */
void dvbpsi_bat_empty(dvbpsi_bat_t *p_bat);

/*!
 * \fn dvbpsi_bat_delete(dvbpsi_bat_t *p_bat)
 * \brief Clean and free a dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \return nothing.
 */
void dvbpsi_bat_delete(dvbpsi_bat_t *p_bat);

/*****************************************************************************
 * dvbpsi_bat_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_bat_bouquet_descriptor_add(dvbpsi_bat_t* p_bat,
                                                              uint8_t i_tag,
                                                              uint8_t i_length,
                                                              uint8_t* p_data)
 * \brief Add a descriptor in the BAT.
 * \param p_bat pointer to the BAT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_bat_bouquet_descriptor_add(dvbpsi_bat_t* p_bat,
                                                       uint8_t i_tag, uint8_t i_length,
                                                       uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_bat_ts_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_bat_ts_add(dvbpsi_bat_t* p_bat,
                                              uint16_t i_ts_id,
                                              uint16_t i_orig_network_id)
 * \brief Add a TS description at the end of the BAT.
 * \param p_bat pointer to the BAT structure
 * \param i_ts_id descriptor's transport stream id
 * \param i_orig_network_id descriptor's original network id
 * \return a pointer to the added descriptor.
 */

dvbpsi_bat_ts_t *dvbpsi_bat_ts_add(dvbpsi_bat_t* p_bat,
                                 uint16_t i_ts_id, uint16_t i_orig_network_id);

/*****************************************************************************
 * dvbpsi_bat_ts_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_bat_ts_descriptor_add(dvbpsi_bat_ts_t *p_bat,
                                                         uint8_t i_tag,
                                                         uint8_t i_length,
                                                         uint8_t *p_data)
 * \brief Add a descriptor in the BAT TS descriptors.
 * \param p_bat pointer to the BAT structure
 * \param i_tag descriptor number
 * \param i_length length of descriptor in bytes
 * \param p_data pointer to descriptor data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t *dvbpsi_bat_ts_descriptor_add(dvbpsi_bat_ts_t *p_bat,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

/*****************************************************************************
 * dvbpsi_bat_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_bat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_bat_t* p_bat)
 * \brief BAT generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_bat BAT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate BAT sections based on the dvbpsi_bat_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_bat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_bat_t * p_bat);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of bat.h"
#endif
