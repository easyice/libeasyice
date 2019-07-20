/*****************************************************************************
 * tot.h
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
 * \file <tot.h>
 * \author Johann Hanne
 * \brief Application interface for the TDT/TOT decoder and the TDT/TOT generator.
 *
 * Application interface for the TDT (Time and Date Table)/TOT (Time Offset Table)
 * decoder and the TDT/TOT generator. New decoded TDT/TOT tables are sent by
 * callback to the application.
 */

#ifndef _DVBPSI_TOT_H_
#define _DVBPSI_TOT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_tot_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_tot_s
 * \brief TDT/TOT structure.
 *
 * This structure is used to store a decoded TDT/TOT.
 * (ETSI EN 300 468 section 5.2.5/5.2.6).
 */
/*!
 * \typedef struct dvbpsi_tot_s dvbpsi_tot_t
 * \brief dvbpsi_tot_t type definition.
 */
typedef struct dvbpsi_tot_s
{
    uint8_t                   i_table_id;         /*!< table id */
    uint16_t                  i_extension;        /*!< subtable id */

    /* Subtable specific */
    uint8_t                   i_version;          /*!< version_number */
    bool                      b_current_next;     /*!< current_next_indicator */

    uint64_t                  i_utc_time;         /*!< UTC_time */

    dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

} __attribute__((packed)) dvbpsi_tot_t;

/*****************************************************************************
 * dvbpsi_tot_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_tot_callback)(void* p_cb_data,
                                         dvbpsi_tot_t* p_new_tot)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_tot_callback)(void* p_cb_data, dvbpsi_tot_t* p_new_tot);

/*****************************************************************************
 * dvbpsi_tot_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_tot_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                            dvbpsi_tot_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a TDT/TOT decoder.
 * \param p_dvbpsi dvbpsi handle pointing to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \param pf_callback function to call back on new TDT/TOT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true on success, false on failure
 */
bool dvbpsi_tot_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                       dvbpsi_tot_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_tot_detach
 *****************************************************************************/
/*!
 * \fn int dvbpsi_tot_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
 * \brief Destroy a TDT/TOT decoder.
 * \param p_dvbpsi Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \return nothing.
 */
void dvbpsi_tot_detach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id,
                      uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_tot_init/dvbpsi_tot_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_tot_init(dvbpsi_tot_t* p_tot, uint8_t i_table_id, uint16_t i_extension,
                            uint8_t i_version, bool b_current_next, uint64_t i_utc_time);
 * \brief Initialize a user-allocated dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \param i_version SDT version
 * \param b_current_next current next indicator
 * \param i_utc_time the time in UTC
 * \return nothing.
 */
void dvbpsi_tot_init(dvbpsi_tot_t* p_tot, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next, uint64_t i_utc_time);

/*!
 * \fn dvbpsi_tot_t *dvbpsi_tot_new(uint8_t i_table_id, uint16_t i_extension,
                            uint8_t i_version, bool b_current_next, uint64_t i_utc_time);
 * \brief Allocate and initialize a new dvbpsi_tot_t structure.
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \param i_version SDT version
 * \param b_current_next current next indicator
 * \param i_utc_time the time in UTC
 * \return p_tot pointer to the TDT/TOT structure
 */
dvbpsi_tot_t *dvbpsi_tot_new(uint8_t i_table_id, uint16_t i_extension, uint8_t i_version,
                             bool b_current_next, uint64_t i_utc_time);

/*****************************************************************************
 * dvbpsi_tot_empty/dvbpsi_tot_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_tot_empty(dvbpsi_tot_t* p_tot)
 * \brief Clean a dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \return nothing.
 */
void dvbpsi_tot_empty(dvbpsi_tot_t* p_tot);

/*!
 * \fn dvbpsi_tot_delete(dvbpsi_tot_t* p_tot)
 * \brief Clean and free a dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \return nothing.
 */
void dvbpsi_tot_delete(dvbpsi_tot_t* p_tot);

/*****************************************************************************
 * dvbpsi_tot_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_tot_descriptor_add(dvbpsi_tot_t* p_tot,
                                                      uint8_t i_tag,
                                                      uint8_t i_length,
                                                      uint8_t* p_data)
 * \brief Add a descriptor in the TOT.
 * \param p_tot pointer to the TOT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_tot_descriptor_add(dvbpsi_tot_t* p_tot,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_tot_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_tot_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_tot_t* p_tot)
 * \brief TDT/TOT generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_tot TDT/TOT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate TDT/TOT sections based on the dvbpsi_tot_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_tot_sections_generate(dvbpsi_t* p_dvbpsi, dvbpsi_tot_t* p_tot);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of tot.h"
#endif
