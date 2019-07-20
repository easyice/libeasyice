/*****************************************************************************
 * pmt.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 * \file <pmt.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the PMT decoder and the PMT generator.
 *
 * Application interface for the PMT decoder and the PMT generator.
 * New decoded PMT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_PMT_H_
#define _DVBPSI_PMT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_pmt_es_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_pmt_es_s
 * \brief PMT ES structure.
 *
 * This structure is used to store a decoded ES description.
 * (ISO/IEC 13818-1 section 2.4.4.8).
 */
/*!
 * \typedef struct dvbpsi_pmt_es_s dvbpsi_pmt_es_t
 * \brief dvbpsi_pmt_es_t type definition.
 */
typedef struct dvbpsi_pmt_es_s
{
  uint8_t                       i_type;                 /*!< stream_type */
  uint16_t                      i_pid;                  /*!< elementary_PID */

  dvbpsi_descriptor_t *         p_first_descriptor;     /*!< descriptor list */

  struct dvbpsi_pmt_es_s *      p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_pmt_es_t;

/*****************************************************************************
 * dvbpsi_pmt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_pmt_s
 * \brief PMT structure.
 *
 * This structure is used to store a decoded PMT.
 * (ISO/IEC 13818-1 section 2.4.4.8).
 */
/*!
 * \typedef struct dvbpsi_pmt_s dvbpsi_pmt_t
 * \brief dvbpsi_pmt_t type definition.
 */
typedef struct dvbpsi_pmt_s
{
  uint16_t                  i_program_number;   /*!< program_number */
  uint8_t                   i_version;          /*!< version_number */
  bool                      b_current_next;     /*!< current_next_indicator */

  uint16_t                  i_pcr_pid;          /*!< PCR_PID */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

  dvbpsi_pmt_es_t *         p_first_es;         /*!< ES list */

} dvbpsi_pmt_t;

/*****************************************************************************
 * dvbpsi_pmt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_pmt_callback)(void* p_cb_data,
                                         dvbpsi_pmt_t* p_new_pmt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_pmt_callback)(void* p_cb_data, dvbpsi_pmt_t* p_new_pmt);

/*****************************************************************************
 * dvbpsi_pmt_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_pmt_attach(dvbpsi_t *p_dvbpsi,
                             uint16_t i_program_number,
                             dvbpsi_pmt_callback pf_callback,
                             void* p_cb_data)
 * \brief Creates and initialization of a PMT decoder and attaches it to dvbpsi_t
 *        handle
 * \param p_dvbpsi handle
 * \param i_program_number program number
 * \param pf_callback function to call back on new PMT
 * \param p_cb_data private data given in argument to the callback
 * \return true on success, false on failure
 */
bool dvbpsi_pmt_attach(dvbpsi_t *p_dvbpsi, uint16_t i_program_number,
                      dvbpsi_pmt_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_pmt_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pmt_detach(dvbpsi_t *p_dvbpsi)
 * \brief Destroy a PMT decoder.
 * \param p_dvbpsi handle
 * \return nothing.
 *
 * The handle isn't valid any more.
 */
void dvbpsi_pmt_detach(dvbpsi_t *p_dvbpsi);

/*****************************************************************************
 * dvbpsi_pmt_init/dvbpsi_pmt_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pmt_init(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                           uint8_t i_version, bool b_current_next,
                           uint16_t i_pcr_pid)
 * \brief Initialize a user-allocated dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 */
void dvbpsi_pmt_init(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, bool b_current_next, uint16_t i_pcr_pid);

/*!
 * \fn dvbpsi_pmt_t* dvbpsi_pmt_new(uint16_t i_program_number,
                           uint8_t i_version, bool b_current_next,
                           uint16_t i_pcr_pid)
 * \brief Allocate and initialize a new dvbpsi_pmt_t structure.
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return p_pmt pointer to the PMT structure
 */
dvbpsi_pmt_t* dvbpsi_pmt_new(uint16_t i_program_number, uint8_t i_version,
                            bool b_current_next, uint16_t i_pcr_pid);

/*****************************************************************************
 * dvbpsi_pmt_empty/dvbpsi_pmt_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pmt_empty(dvbpsi_pmt_t* p_pmt)
 * \brief Clean a dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \return nothing.
 */
void dvbpsi_pmt_empty(dvbpsi_pmt_t* p_pmt);

/*!
 * \fn void dvbpsi_pmt_delete(dvbpsi_pmt_t* p_pmt)
 * \brief Clean and free a dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \return nothing.
 */
void dvbpsi_pmt_delete(dvbpsi_pmt_t* p_pmt);

/*****************************************************************************
 * dvbpsi_pmt_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_pmt_descriptor_add(dvbpsi_pmt_t* p_pmt,
                                                    uint8_t i_tag,
                                                    uint8_t i_length,
                                                    uint8_t* p_data)
 * \brief Add a descriptor in the PMT.
 * \param p_pmt pointer to the PMT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_pmt_descriptor_add(dvbpsi_pmt_t* p_pmt,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_pmt_es_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_pmt_es_t* dvbpsi_pmt_es_add(dvbpsi_pmt_t* p_pmt,
                                        uint8_t i_type, uint16_t i_pid)
 * \brief Add an ES in the PMT.
 * \param p_pmt pointer to the PMT structure
 * \param i_type type of ES
 * \param i_pid PID of the ES
 * \return a pointer to the added ES.
 */
dvbpsi_pmt_es_t* dvbpsi_pmt_es_add(dvbpsi_pmt_t* p_pmt,
                                 uint8_t i_type, uint16_t i_pid);

/*****************************************************************************
 * dvbpsi_pmt_es_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_pmt_es_descriptor_add(dvbpsi_pmt_es_t* p_es,
                                                      uint8_t i_tag,
                                                      uint8_t i_length,
                                                      uint8_t* p_data)
 * \brief Add a descriptor in the PMT ES.
 * \param p_es pointer to the ES structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_pmt_es_descriptor_add(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_pmt_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_pmt_sections_generate(dvbpsi_t *p_dvbpsi,
                                                   dvbpsi_pmt_t* p_pmt)
 * \brief PMT generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_pmt PMT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate PMT sections based on the dvbpsi_pmt_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_pmt_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_pmt_t* p_pmt);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of pmt.h"
#endif
