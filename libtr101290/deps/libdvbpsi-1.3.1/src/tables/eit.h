/*****************************************************************************
 * eit.h
 * Copyright (C) 2004-2011 VideoLAN
 * $Id: eit.h 88 2004-02-24 14:31:18Z sam $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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
 * \file <eit.h>
 * \author Christophe Massiot <massiot@via.ecp.fr>
 * \brief Application interface for the EIT decoder and the EIT generator.
 *
 * Application interface for the EIT decoder and the EIT generator.
 * New decoded EIT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_EIT_H_
#define _DVBPSI_EIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_eit_event_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_eit_event_s
 * \brief EIT service description structure.
 *
 * This structure is used to store a decoded EIT event description.
 * (ETSI EN 300 468 V1.5.1 section 5.2.4).
 */
/*!
 * \typedef struct dvbpsi_eit_service_s dvbpsi_eit_event_t
 * \brief dvbpsi_eit_event_t type definition.
 */
typedef struct dvbpsi_eit_event_s
{
  uint16_t                  i_event_id;             /*!< event_id */
  uint64_t                  i_start_time;           /*!< start_time */
  uint32_t                  i_duration;             /*!< duration */
  uint8_t                   i_running_status;       /*!< Running status */
  bool                      b_free_ca;              /*!< Free CA mode flag */
  bool                      b_nvod;                 /*!< Unscheduled NVOD Event */
  uint16_t                  i_descriptors_length;   /*!< Descriptors loop
                                                         length */
  dvbpsi_descriptor_t *     p_first_descriptor;     /*!< First of the following
                                                         DVB descriptors */

  struct dvbpsi_eit_event_s * p_next;               /*!< next element of
                                                             the list */

} dvbpsi_eit_event_t;

/*****************************************************************************
 * dvbpsi_eit_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_eit_s
 * \brief EIT structure.
 *
 * This structure is used to store a decoded EIT.
 * (ETSI EN 300 468 V1.5.1 section 5.2.4).
 */
/*!
 * \typedef struct dvbpsi_eit_s dvbpsi_eit_t
 * \brief dvbpsi_eit_t type definition.
 */
typedef struct dvbpsi_eit_s
{
    uint8_t             i_table_id;         /*!< table id */
    uint16_t            i_extension;        /*!< subtable id, here service_id */

    uint8_t             i_version;          /*!< version_number */
    bool                b_current_next;     /*!< current_next_indicator */
    uint16_t            i_ts_id;            /*!< transport stream id */
    uint16_t            i_network_id;       /*!< original network id */
    uint8_t             i_segment_last_section_number; /*!< segment last section number */
    uint8_t             i_last_table_id;    /*!< last table id */

    dvbpsi_eit_event_t *p_first_event;      /*!< event information list */

} dvbpsi_eit_t;

/*****************************************************************************
 * dvbpsi_eit_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_eit_callback)(void* p_cb_data, dvbpsi_eit_t* p_new_eit)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_eit_callback)(void* p_cb_data, dvbpsi_eit_t* p_new_eit);

/*****************************************************************************
 * dvbpsi_AttachEIT
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_eit_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_eit_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a EIT decoder.
 * \param p_dvbpsi pointer to Subtable demultiplexor to which the EIT decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \param pf_callback function to call back on new EIT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true on success, false on failure
 */
bool dvbpsi_eit_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                       dvbpsi_eit_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_eit_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_eit_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a EIT decoder.
 * \param p_dvbpsi dvbpsi handle pointing to Subtable demultiplexor to which the
                   eit decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \return nothing.
 */
void dvbpsi_eit_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_eit_init/dvbpsi_eit_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_eit_init(dvbpsi_eit_t* p_eit, uint8_t i_table_id,
                     uint16_t i_extension, uint8_t i_version,
                     bool b_current_next, uint16_t i_ts_id, uint16_t i_network_id,
                     uint8_t i_segment_last_section_number,
                     uint8_t i_last_table_id);
 * \brief Initialize a user-allocated dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_ts_id transport stream ID
 * \param i_network_id original network id
 * \param i_segment_last_section_number segment_last_section_number
 * \param i_last_table_id i_last_table_id
 * \return nothing.
 */
void dvbpsi_eit_init(dvbpsi_eit_t* p_eit, uint8_t i_table_id,
                     uint16_t i_extension, uint8_t i_version,
                     bool b_current_next, uint16_t i_ts_id, uint16_t i_network_id,
                     uint8_t i_segment_last_section_number,
                     uint8_t i_last_table_id);

/*!
 * \fn dvbpsi_eit_t* dvbpsi_eit_new(uint8_t i_table_id, uint16_t i_extension,
            uint8_t i_version, bool b_current_next,
            uint16_t i_ts_id, uint16_t i_network_id, uint8_t i_segment_last_section_number,
            uint8_t i_last_table_id)
 * \brief Allocate and initialize a new dvbpsi_eit_t structure.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_ts_id transport stream ID
 * \param i_network_id original network id
 * \param i_segment_last_section_number segment_last_section_number
 * \param i_last_table_id i_last_table_id
 * \return p_eit pointer to the EIT structure
 */
dvbpsi_eit_t* dvbpsi_eit_new(uint8_t i_table_id, uint16_t i_extension,
                             uint8_t i_version, bool b_current_next,
                             uint16_t i_ts_id, uint16_t i_network_id,
                             uint8_t i_segment_last_section_number,
                             uint8_t i_last_table_id);

/*****************************************************************************
 * dvbpsi_eit_empty/dvbpsi_eit_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_eit_empty(dvbpsi_eit_t* p_eit)
 * \brief Clean a dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \return nothing.
 */
void dvbpsi_eit_empty(dvbpsi_eit_t* p_eit);

/*!
 * \fn void dvbpsi_eit_delete(dvbpsi_eit_t *p_eit)
 * \brief Clean and free a dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \return nothing.
 */
void dvbpsi_eit_delete(dvbpsi_eit_t* p_eit);

/*****************************************************************************
 * dvbpsi_eit_event_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_eit_event_t* dvbpsi_eit_event_add(dvbpsi_eit_t* p_eit,
                                              uint16_t i_event_id,
                                              uint64_t i_start_time,
                                              uint32_t i_duration,
                                              uint8_t i_running_status,
                                              bool b_free_ca,
                                              uint16_t i_event_descriptor_length)
 * \brief Add a service description at the end of the EIT.
 * \param p_eit pointer to the EIT structure
 * \param i_event_id Event ID
 * \param i_start_time Start Time
 * \param i_duration Duration
 * \param i_running_status Running status
 * \param b_free_ca Free CA flag
 * \param i_event_descriptor_length The descriptors loop length in bytes of
                                    all descriptors for this event.
 * \return a pointer to the added service description.
 */
dvbpsi_eit_event_t* dvbpsi_eit_event_add(dvbpsi_eit_t* p_eit,
    uint16_t i_event_id, uint64_t i_start_time, uint32_t i_duration,
    uint8_t i_running_status, bool b_free_ca,
    uint16_t i_event_descriptor_length);

/*****************************************************************************
 * dvbpsi_eit_nvod_event_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_eit_event_t* dvbpsi_eit_nvod_event_add(dvbpsi_eit_t* p_eit,
                                              uint16_t i_event_id,
                                              uint32_t i_duration,
                                              bool b_free_ca,
                                              uint16_t i_event_descriptor_length)
 * \brief Add a NVOD service description at the end of the EIT.
 * \param p_eit pointer to the EIT structure
 * \param i_event_id Event ID
 * \param i_duration Duration
 * \param b_free_ca Free CA flag
 * \param i_event_descriptor_length The descriptors loop length in bytes of
                                    all descriptors for this event.
 * \return a pointer to the added service description.
 */
dvbpsi_eit_event_t* dvbpsi_eit_nvod_event_add(dvbpsi_eit_t* p_eit,
    uint16_t i_event_id, uint32_t i_duration, bool b_free_ca,
    uint16_t i_event_descriptor_length);

/*****************************************************************************
 * dvbpsi_eit_event_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_eit_event_descriptor_add(
                                               dvbpsi_eit_event_t* p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data)
 * \brief Add a descriptor to the EIT event.
 * \param p_event pointer to the EIT event structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_eit_event_descriptor_add(
                                               dvbpsi_eit_event_t* p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_eit_sections_generate
 *****************************************************************************
 * Generate EIT sections based on the dvbpsi_eit_t structure.
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t *dvbpsi_eit_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_eit_t *p_eit,
 *                                                 uint8_t i_table_id);
 * \brief Generate a EIT section based on the information provided in p_eit.
 * \param p_dvbpsi pointer to Subtable demultiplexor to which the EIT decoder is attached.
 * \param p_eit pointer to EIT information to include in the PSI secion
 * \param i_table_id the EIT table id to use
 * \return a pointer to a new PSI section
 */
dvbpsi_psi_section_t *dvbpsi_eit_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_eit_t *p_eit,
                                            uint8_t i_table_id);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of eit.h"
#endif

