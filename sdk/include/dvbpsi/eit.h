/*****************************************************************************
 * eit.h
 * Copyright (C) 2004-2011 VideoLAN
 * $Id: eit.h 88 2004-02-24 14:31:18Z sam $
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
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
  int                       b_free_ca;              /*!< Free CA mode flag */
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
  uint16_t                  i_service_id;       /*!< service_id */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */
  uint16_t                  i_ts_id;            /*!< transport stream id */
  uint16_t                  i_network_id;       /*!< original network id */
  uint8_t                   i_segment_last_section_number; /*!< segment last section number */
  uint8_t                   i_last_table_id;    /*!< last table id */

  dvbpsi_eit_event_t *      p_first_event;      /*!< event information list */

} dvbpsi_eit_t;


/*****************************************************************************
 * dvbpsi_eit_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_eit_callback)(void* p_cb_data,
                                         dvbpsi_eit_t* p_new_eit)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_eit_callback)(void* p_cb_data, dvbpsi_eit_t* p_new_eit);


/*****************************************************************************
 * dvbpsi_AttachEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_eit_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a EIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \param pf_callback function to call back on new EIT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachEIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_eit_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a EIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachEIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitEIT/dvbpsi_NewEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitEIT(dvbpsi_eit_t* p_eit, uint16_t i_service_id,
          uint8_t i_version, int b_current_next, uint16_t i_ts_id,
          uint16_t i_network_id, uint8_t i_segment_last_section_number,
          uint8_t i_last_table_id)
 * \brief Initialize a user-allocated dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \param i_service_id service ID
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_ts_id transport stream ID
 * \param i_network_id original network id
 * \param i_segment_last_section_number segment_last_section_number
 * \param i_last_table_id i_last_table_id
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitEIT(dvbpsi_eit_t* p_eit, uint16_t i_service_id, uint8_t i_version,
                    int b_current_next, uint16_t i_ts_id, uint16_t i_network_id,
                    uint8_t i_segment_last_section_number,
                    uint8_t i_last_table_id);

/*!
 * \def dvbpsi_NewEIT(p_eit, i_ts_id, i_version, b_current_next, i_network_id)
 * \brief Allocate and initialize a new dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \param i_ts_id transport stream ID
 * \param i_version EIT version
 * \param b_current_next current next indicator
 * \param i_network_id original network id
 * \return nothing.
 */
#define dvbpsi_NewEIT(p_eit, i_service_id, i_version, b_current_next, i_ts_id, i_network_id, i_segment_last_section_number, i_last_table_id) \
do {                                                                    \
  p_eit = (dvbpsi_eit_t*)malloc(sizeof(dvbpsi_eit_t));                  \
  if(p_eit != NULL)                                                     \
    dvbpsi_InitEIT(p_eit, i_service_id, i_version, b_current_next, i_ts_id, i_network_id, i_segment_last_section_number, i_last_table_id); \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptyEIT/dvbpsi_DeleteEIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptyEIT(dvbpsi_eit_t* p_eit)
 * \brief Clean a dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptyEIT(dvbpsi_eit_t* p_eit);

/*!
 * \def dvbpsi_DeleteEIT(p_eit)
 * \brief Clean and free a dvbpsi_eit_t structure.
 * \param p_eit pointer to the EIT structure
 * \return nothing.
 */
#define dvbpsi_DeleteEIT(p_eit)                                         \
do {                                                                    \
  dvbpsi_EmptyEIT(p_eit);                                               \
  free(p_eit);                                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_EITAddEvent
 *****************************************************************************/
/*!
 * \fn dvbpsi_eit_event_t* dvbpsi_EITAddEvent(dvbpsi_eit_t* p_eit,
                                              uint16_t i_event_id,
                                              uint64_t i_start_time,
                                              uint32_t i_duration,
                                              uint8_t i_running_status,
                                              int b_free_ca)
 * \brief Add a service description at the end of the EIT.
 * \param p_eit pointer to the EIT structure
 * \param i_event_id Event ID
 * \param i_start_time Start Time
 * \param i_duration Duration
 * \param i_running_status Running status
 * \param b_free_ca Free CA flag
 * \return a pointer to the added service description.
 */
dvbpsi_eit_event_t* dvbpsi_EITAddEvent(dvbpsi_eit_t* p_eit,
    uint16_t i_event_id, uint64_t i_start_time, uint32_t i_duration,
    uint8_t i_running_status, int b_free_ca);

/*****************************************************************************
 * dvbpsi_EITEventAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_EITEventAddDescriptor(
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
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_EITEventAddDescriptor(
                                               dvbpsi_eit_event_t* p_event,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of eit.h"
#endif

