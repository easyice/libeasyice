/*****************************************************************************
 * rst.h
 * Copyright (c) 2012 VideoLAN
 * $Id$
 *
 * Authors: Corno Roberto <corno.roberto@gmail.com>
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
 * \file <rst.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the RST decoder and the RST generator.
 *
 * Application interface for the RST decoder and the RST generator.
 * New decoded RST tables are sent by callback to the application.
 */

#ifndef _DVBPSI_RST_H_
#define _DVBPSI_RST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_rst_event_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_rst_event_s
 * \brief RST service description structure.
 *
 * This structure is used to store a decoded RST event description.
 * (ETSI EN 300 468 V1.5.1 section 5.2.7).
 */
/*!
 * \typedef struct dvbpsi_rst_service_s dvbpsi_rst_event_t
 * \brief dvbpsi_rst_event_t type definition.
 */
typedef struct dvbpsi_rst_event_s
{
  uint16_t                  i_ts_id;                /*!< transport stream id */
  uint16_t                  i_orig_network_id;      /*!< original network id */
  uint16_t                  i_service_id;           /*!< service id */
  uint16_t                  i_event_id;             /*!< event id */
  uint8_t                   i_running_status;       /*!< Running status */

  struct dvbpsi_rst_event_s * p_next;               /*!< next element of
                                                             the list */

} dvbpsi_rst_event_t;

/*****************************************************************************
 * dvbpsi_rst_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_rst_s
 * \brief RST structure.
 *
 * This structure is used to store a decoded RST service description.
 * (ETSI EN 300 468 V1.5.1 section 5.2.7).
 */
/*!
 * \typedef struct dvbpsi_rst_s dvbpsi_rst_t
 * \brief dvbpsi_rst_t type definition.
 */
typedef struct dvbpsi_rst_s
{
  dvbpsi_rst_event_t *      p_first_event;      /*!< event information list */
} dvbpsi_rst_t;


/*****************************************************************************
 * dvbpsi_rst_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_rst_callback)(void* p_cb_data,
                                         dvbpsi_rst_t* p_new_rst)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_rst_callback)(void* p_cb_data, dvbpsi_rst_t* p_new_rst);

/*****************************************************************************
 * dvbpsi_rst_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_rst_attach(dvbpsi_t *p_dvbpsi,
                            dvbpsi_rst_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a RST decoder. It will be attached to p_dvbpsi
 * \param p_dvbpsi is a pointer to dvbpsi_t which holds a pointer to the decoder
 * \param pf_callback function to call back on new RST
 * \param p_cb_data private data given in argument to the callback
 * \return true on success, false on failure
 */
bool dvbpsi_rst_attach(dvbpsi_t *p_dvbpsi, dvbpsi_rst_callback pf_callback,
                      void* p_cb_data);

/*****************************************************************************
 * dvbpsi_rst_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_rst_detach(dvbpsi_t *p_dvbpsi)
 * \brief Destroy a RST decoder.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_dvbpsi handle holds the decoder pointer
 * \return nothing.
 *
 * The handle isn't valid any more.
 */
void dvbpsi_rst_detach(dvbpsi_t *p_dvbpsi);

/*****************************************************************************
 * dvbpsi_rst_init/dvbpsi_rst_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_rst_init(dvbpsi_rst_t* p_rst)
 * \brief Initialize a user-allocated dvbpsi_cat_t structure.
 * \param p_rst pointer to the RST structure
 * \return nothing.
 */
void dvbpsi_rst_init(dvbpsi_rst_t* p_rst);

/*!
 * \fn dvbpsi_rst_t *dvbpsi_rst_new(void)
 * \brief Allocate and initialize a new dvbpsi_rst_t structure.
 * \return p_rst pointer to the RST structure
 */
dvbpsi_rst_t *dvbpsi_rst_new(void);

/*****************************************************************************
 * dvbpsi_rst_empty/dvbpsi_rst_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_rst_empty(dvbpsi_rst_t* p_rst)
 * \brief Clean a dvbpsi_rst_t structure.
 * \param p_rst pointer to the RST structure
 * \return nothing.
 */
void dvbpsi_rst_empty(dvbpsi_rst_t* p_rst);

/*!
 * \fn void dvbpsi_rst_delete(dvbpsi_rst_t *p_rst)
 * \brief Clean and free a dvbpsi_rst_t structure.
 * \param p_rst pointer to the RST structure
 * \return nothing.
 */
void dvbpsi_rst_delete(dvbpsi_rst_t *p_rst);

/*****************************************************************************
 * dvbpsi_rst_event_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_rst_event_t* dvbpsi_rst_event_add(dvbpsi_rst_t* p_rst,
 *                                            uint16_t i_ts_id,
 *                                            uint16_t i_orig_network_id,
 *                                            uint16_t i_service_id,
 *                                            uint16_t i_event_id,
 *                                            uint8_t i_running_status)
 * \brief Add an event in the RST.
 * \param p_rst pointer to the RST structure
 * \param i_ts_id event's transport stream id
 * \param i_orig_network_id event's original network id
 * \param i_service_id event's service id
 * \param i_event_id event's id
 * \param i_running_status event's running status
 * \return a pointer to the added event.
 */
dvbpsi_rst_event_t* dvbpsi_rst_event_add(dvbpsi_rst_t* p_rst,
                                            uint16_t i_ts_id,
                                            uint16_t i_orig_network_id,
                                            uint16_t i_service_id,
                                            uint16_t i_event_id,
                                            uint8_t i_running_status);

/*****************************************************************************
 * dvbpsi_rst_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_rst_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_rst_t* p_rst)
 * \brief RST generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_rst RST structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate RST sections based on the dvbpsi_rst_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_rst_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_rst_t* p_rst);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of rst.h"
#endif
