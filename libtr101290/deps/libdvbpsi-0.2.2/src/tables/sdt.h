/*****************************************************************************
 * sdt.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
 * \file <sdt.h>
 * \author Johan Bilien <jobi@via.ecp.fr>
 * \brief Application interface for the SDT decoder and the SDT generator.
 *
 * Application interface for the SDT decoder and the SDT generator.
 * New decoded SDT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_SDT_H_
#define _DVBPSI_SDT_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_sdt_service_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_sdt_service_s
 * \brief SDT service description structure.
 *
 * This structure is used to store a decoded SDT service description.
 * (ETSI EN 300 468 V1.4.1 section 5.2.3).
 */
/*!
 * \typedef struct dvbpsi_sdt_service_s dvbpsi_sdt_service_t
 * \brief dvbpsi_sdt_service_t type definition.
 */
typedef struct dvbpsi_sdt_service_s
{
  uint16_t                  i_service_id;           /*!< service_id */
  int                       b_eit_schedule;         /*!< EIT schedule flag */
  int                       b_eit_present;          /*!< EIT present/following
                                                         flag */
  uint8_t                   i_running_status;       /*!< Running status */
  int                       b_free_ca;              /*!< Free CA mode flag */
  uint16_t                  i_descriptors_length;   /*!< Descriptors loop
                                                         length */
  dvbpsi_descriptor_t *     p_first_descriptor;     /*!< First of the following
                                                         DVB descriptors */


  struct dvbpsi_sdt_service_s * p_next;             /*!< next element of
                                                             the list */

} dvbpsi_sdt_service_t;


/*****************************************************************************
 * dvbpsi_sdt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_sdt_s
 * \brief SDT structure.
 *
 * This structure is used to store a decoded SDT.
 * (ETSI EN 300 468 V1.4.1 section 5.2.3).
 */
/*!
 * \typedef struct dvbpsi_sdt_s dvbpsi_sdt_t
 * \brief dvbpsi_sdt_t type definition.
 */
typedef struct dvbpsi_sdt_s
{
  uint16_t                  i_ts_id;            /*!< transport_stream_id */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */
  uint16_t                  i_network_id;       /*!< original network id */

  dvbpsi_sdt_service_t *    p_first_service;    /*!< service description
                                                     list */

} dvbpsi_sdt_t;


/*****************************************************************************
 * dvbpsi_sdt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_sdt_callback)(void* p_cb_data,
                                         dvbpsi_sdt_t* p_new_sdt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_sdt_callback)(void* p_cb_data, dvbpsi_sdt_t* p_new_sdt);


/*****************************************************************************
 * dvbpsi_AttachSDT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachSDT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sdt_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a SDT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x42 or 0x46.
 * \param i_extension Table ID extension, here TS ID.
 * \param pf_callback function to call back on new SDT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachSDT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sdt_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachSDT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachSDT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a SDT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x42 or 0x46.
 * \param i_extension Table ID extension, here TS ID.
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachSDT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitSDT/dvbpsi_NewSDT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitSDT(dvbpsi_sdt_t* p_sdt, uint16_t i_ts_id,
          uint8_t i_version, int b_current_next, uint16_t i_network_id)
 * \brief Initialize a user-allocated dvbpsi_sdt_t structure.
 * \param p_sdt pointer to the SDT structure
 * \param i_ts_id transport stream ID
 * \param i_version SDT version
 * \param b_current_next current next indicator
 * \param i_network_id original network id
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitSDT(dvbpsi_sdt_t *p_sdt, uint16_t i_ts_id, uint8_t i_version,
                    int b_current_next, uint16_t i_network_id);

/*!
 * \def dvbpsi_NewSDT(p_sdt, i_ts_id, i_version, b_current_next, i_network_id)
 * \brief Allocate and initialize a new dvbpsi_sdt_t structure.
 * \param p_sdt pointer to the SDT structure
 * \param i_ts_id transport stream ID
 * \param i_version SDT version
 * \param b_current_next current next indicator
 * \param i_network_id original network id
 * \return nothing.
 */
#define dvbpsi_NewSDT(p_sdt, i_ts_id, i_version, b_current_next,i_network_id) \
do {                                                                    \
  p_sdt = (dvbpsi_sdt_t*)malloc(sizeof(dvbpsi_sdt_t));                  \
  if(p_sdt != NULL)                                                     \
    dvbpsi_InitSDT(p_sdt, i_ts_id, i_version, b_current_next, i_network_id); \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptySDT/dvbpsi_DeleteSDT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptySDT(dvbpsi_sdt_t* p_sdt)
 * \brief Clean a dvbpsi_sdt_t structure.
 * \param p_sdt pointer to the SDT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptySDT(dvbpsi_sdt_t *p_sdt);

/*!
 * \def dvbpsi_DeleteSDT(p_sdt)
 * \brief Clean and free a dvbpsi_sdt_t structure.
 * \param p_sdt pointer to the SDT structure
 * \return nothing.
 */
#define dvbpsi_DeleteSDT(p_sdt)                                         \
do {                                                                    \
  dvbpsi_EmptySDT(p_sdt);                                               \
  free(p_sdt);                                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_SDTAddService
 *****************************************************************************/
/*!
 * \fn dvbpsi_sdt_service_t* dvbpsi_SDTAddService(dvbpsi_sdt_t* p_sdt,
                                                  uint16_t i_service_id,
                                                  int b_eit_schedule,
                                                  int b_eit_present,
                                                  uint8_t i_running_status,
                                                  int b_free_ca)
 * \brief Add a service description at the end of the SDT.
 * \param p_sdt pointer to the SDT structure
 * \param i_service_id Service ID
 * \param b_eit_schedule EIT Schedule flag
 * \param b_eit_present EIT Present/Following flag
 * \param i_running_status Running status
 * \param b_free_ca Free CA flag
 * \return a pointer to the added service description.
 */
__attribute__((deprecated))
dvbpsi_sdt_service_t *dvbpsi_SDTAddService(dvbpsi_sdt_t* p_sdt,
    uint16_t i_service_id, int b_eit_schedule, int b_eit_present,
    uint8_t i_running_status,int b_free_ca);


/*****************************************************************************
 * dvbpsi_SDTServiceAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_SDTServiceAddDescriptor(
                                               dvbpsi_sdt_service_t *p_service,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
 * \brief Add a descriptor in the SDT service.
 * \param p_service pointer to the service structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
__attribute__((deprecated))
dvbpsi_descriptor_t *dvbpsi_SDTServiceAddDescriptor(
                                               dvbpsi_sdt_service_t *p_service,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data);


/*****************************************************************************
 * dvbpsi_GenSDTSections
 *****************************************************************************
 * Generate SDT sections based on the dvbpsi_sdt_t structure.
 *****************************************************************************/
dvbpsi_psi_section_t *dvbpsi_GenSDTSections(dvbpsi_sdt_t * p_sdt);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of sdt.h"
#endif

