/*****************************************************************************
 * nit.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 *****************************************************************************/

/*!
 * \file <nit.h>
 * \author Johann Hanne
 * \brief Application interface for the NIT decoder and the NIT generator.
 *
 * Application interface for the NIT decoder and the NIT generator.
 * New decoded NIT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_NIT_H_
#define _DVBPSI_NIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_nit_ts_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nit_ts_s
 * \brief NIT TS structure.
 *
 * This structure is used to store a decoded TS description.
 * (ETSI EN 300 468 section 5.2.1).
 */
/*!
 * \typedef struct dvbpsi_nit_ts_s dvbpsi_nit_ts_t
 * \brief dvbpsi_nit_ts_t type definition.
 */
typedef struct dvbpsi_nit_ts_s
{
  uint16_t                      i_ts_id;                /*!< transport stream id */
  uint16_t                      i_orig_network_id;      /*!< original network id */

  dvbpsi_descriptor_t *         p_first_descriptor;     /*!< descriptor list */

  struct dvbpsi_nit_ts_s *      p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_nit_ts_t;

/*****************************************************************************
 * dvbpsi_nit_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nit_s
 * \brief NIT structure.
 *
 * This structure is used to store a decoded NIT.
 * (ETSI EN 300 468 section 5.2.1).
 */
/*!
 * \typedef struct dvbpsi_nit_s dvbpsi_nit_t
 * \brief dvbpsi_nit_t type definition.
 */
typedef struct dvbpsi_nit_s
{
    uint8_t              i_table_id;        /*!< table id */
    uint16_t             i_extension;       /*!< subtable id */

    uint16_t             i_network_id;       /*!< network_id */
    uint8_t              i_version;          /*!< version_number */
    bool                 b_current_next;     /*!< current_next_indicator */

    dvbpsi_descriptor_t *p_first_descriptor; /*!< descriptor list */

    dvbpsi_nit_ts_t *    p_first_ts;         /*!< TS list */

} dvbpsi_nit_t;

/*****************************************************************************
 * dvbpsi_nit_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_nit_callback)(void* p_cb_data,
                                         dvbpsi_nit_t* p_new_nit)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_nit_callback)(void* p_cb_data, dvbpsi_nit_t* p_new_nit);

/*****************************************************************************
 * dvbpsi_nit_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_nit_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                              dvbpsi_nit_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a NIT decoder. It is attached to p_dvbpsi.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x40 (actual) or 0x41 (other).
 * \param i_extension Table ID extension, here network ID.
 * \param pf_callback function to call back on new NIT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true on success, false on failure
 */
bool dvbpsi_nit_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                       dvbpsi_nit_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_nit_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_nit_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
                             uint16_t i_extension)
 * \brief Destroy a NIT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x40 (actual) or 0x41 (other).
 * \param i_extension Table ID extension, here network ID.
 * \return nothing.
 */
void dvbpsi_nit_detach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id,
                      uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_nit_init/dvbpsi_nit_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_nit_init(dvbpsi_nit_t* p_nit, uint8_t i_table_id, uint16_t i_extension,
                            uint16_t i_network_id, uint8_t i_version, bool b_current_next)
 * \brief Initialize a user-allocated dvbpsi_nit_t structure.
 * \param i_table_id Table ID, 0x40 (actual) or 0x41 (other).
 * \param i_extension Table ID extension, here network ID.
 * \param p_nit pointer to the NIT structure
 * \param i_network_id network id
 * \param i_version NIT version
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_nit_init(dvbpsi_nit_t* p_nit, uint8_t i_table_id, uint16_t i_extension,
                     uint16_t i_network_id, uint8_t i_version, bool b_current_next);

/*!
 * \fn dvbpsi_nit_t *dvbpsi_nit_new(uint8_t i_table_id, uint16_t i_extension,
 *                                  uint16_t i_network_id, uint8_t i_version,
 *                                  bool b_current_next);
 * \brief Allocate and initialize a new dvbpsi_nit_t structure.
 * \param i_table_id Table ID, 0x40 (actual) or 0x41 (other)
 * \param i_extension Table ID extension, here network ID.
 * \param i_network_id network id
 * \param i_version NIT version
 * \param b_current_next current next indicator
 * \return p_nit pointer to the NIT structure
 */
dvbpsi_nit_t *dvbpsi_nit_new(uint8_t i_table_id, uint16_t i_extension,
                             uint16_t i_network_id, uint8_t i_version,
                             bool b_current_next);

/*****************************************************************************
 * dvbpsi_nit_empty/dvbpsi_nit_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_nit_empty(dvbpsi_nit_t* p_nit)
 * \brief Clean a dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \return nothing.
 */
void dvbpsi_nit_empty(dvbpsi_nit_t* p_nit);

/*!
 * \fn dvbpsi_nit_delete(dvbpsi_nit_t *p_nit)
 * \brief Clean and free a dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \return nothing.
 */
void dvbpsi_nit_delete(dvbpsi_nit_t *p_nit);

/*****************************************************************************
 * dvbpsi_nit_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_nit_descriptor_add(dvbpsi_nit_t* p_nit,
                                                      uint8_t i_tag,
                                                      uint8_t i_length,
                                                      uint8_t* p_data)
 * \brief Add a descriptor in the NIT.
 * \param p_nit pointer to the NIT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_nit_descriptor_add(dvbpsi_nit_t *p_nit,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);

/*****************************************************************************
 * dvbpsi_nit_ts_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_nit_ts_t* dvbpsi_nit_ts_add(dvbpsi_nit_t* p_nit,
                                  uint16_t i_ts_id, uint16_t i_orig_network_id)
 * \brief Add an TS in the NIT.
 * \param p_nit pointer to the NIT structure
 * \param i_ts_id type of TS
 * \param i_orig_network_id PID of the TS
 * \return a pointer to the added TS.
 */
dvbpsi_nit_ts_t* dvbpsi_nit_ts_add(dvbpsi_nit_t* p_nit,
                                 uint16_t i_ts_id, uint16_t i_orig_network_id);

/*****************************************************************************
 * dvbpsi_nit_ts_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_nit_ts_descriptor_add(dvbpsi_nit_ts_t* p_ts,
                                                         uint8_t i_tag,
                                                         uint8_t i_length,
                                                         uint8_t* p_data)
 * \brief Add a descriptor in the NIT TS.
 * \param p_ts pointer to the TS structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_nit_ts_descriptor_add(dvbpsi_nit_ts_t* p_ts,
                                                  uint8_t i_tag, uint8_t i_length,
                                                  uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_nit_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_nit_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_nit_t* p_nit,
                                                   uint8_t i_table_id)
 * \brief NIT generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_nit NIT structure
 * \param i_table_id table id, 0x40 = actual network / 0x41 = other network
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate NIT sections based on the dvbpsi_nit_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_nit_sections_generate(dvbpsi_t* p_dvbpsi, dvbpsi_nit_t* p_nit,
                                            uint8_t i_table_id);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of nit.h"
#endif

