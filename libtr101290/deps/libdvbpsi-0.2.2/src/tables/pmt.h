/*****************************************************************************
 * pmt.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
  int                       b_current_next;     /*!< current_next_indicator */

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
 * dvbpsi_AttachPMT
 *****************************************************************************/
/*!
 * \fn dvbpsi_handle dvbpsi_AttachPMT(uint16_t i_program_number,
                                      dvbpsi_pmt_callback pf_callback,
                                      void* p_cb_data)
 * \brief Creation and initialization of a PMT decoder.
 * \param i_program_number program number
 * \param pf_callback function to call back on new PMT
 * \param p_cb_data private data given in argument to the callback
 * \return a pointer to the decoder for future calls.
 */
__attribute__((deprecated))
dvbpsi_handle dvbpsi_AttachPMT(uint16_t i_program_number,
                               dvbpsi_pmt_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachPMT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachPMT(dvbpsi_handle h_dvbpsi)
 * \brief Destroy a PMT decoder.
 * \param h_dvbpsi handle to the decoder
 * \return nothing.
 *
 * The handle isn't valid any more.
 */
__attribute__((deprecated))
void dvbpsi_DetachPMT(dvbpsi_handle h_dvbpsi);


/*****************************************************************************
 * dvbpsi_InitPMT/dvbpsi_NewPMT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitPMT(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                           uint8_t i_version, int b_current_next,
                           uint16_t i_pcr_pid)
 * \brief Initialize a user-allocated dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitPMT(dvbpsi_pmt_t* p_pmt, uint16_t i_program_number,
                    uint8_t i_version, int b_current_next, uint16_t i_pcr_pid);

/*!
 * \def dvbpsi_NewPMT(p_pmt, i_program_number,
                      i_version, b_current_next, i_pcr_pid)
 * \brief Allocate and initialize a new dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \param i_program_number program number
 * \param i_version PMT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 */
#define dvbpsi_NewPMT(p_pmt, i_program_number,                          \
                      i_version, b_current_next, i_pcr_pid)             \
do {                                                                    \
  p_pmt = (dvbpsi_pmt_t*)malloc(sizeof(dvbpsi_pmt_t));                  \
  if(p_pmt != NULL)                                                     \
    dvbpsi_InitPMT(p_pmt, i_program_number, i_version, b_current_next,  \
                   i_pcr_pid);                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptyPMT/dvbpsi_DeletePMT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptyPMT(dvbpsi_pmt_t* p_pmt)
 * \brief Clean a dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptyPMT(dvbpsi_pmt_t* p_pmt);

/*!
 * \def dvbpsi_DeletePMT(p_pmt)
 * \brief Clean and free a dvbpsi_pmt_t structure.
 * \param p_pmt pointer to the PMT structure
 * \return nothing.
 */
#define dvbpsi_DeletePMT(p_pmt)                                         \
do {                                                                    \
  dvbpsi_EmptyPMT(p_pmt);                                               \
  free(p_pmt);                                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_PMTAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_PMTAddDescriptor(dvbpsi_pmt_t* p_pmt,
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
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_PMTAddDescriptor(dvbpsi_pmt_t* p_pmt,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_PMTAddES
 *****************************************************************************/
/*!
 * \fn dvbpsi_pmt_es_t* dvbpsi_PMTAddES(dvbpsi_pmt_t* p_pmt,
                                        uint8_t i_type, uint16_t i_pid)
 * \brief Add an ES in the PMT.
 * \param p_pmt pointer to the PMT structure
 * \param i_type type of ES
 * \param i_pid PID of the ES
 * \return a pointer to the added ES.
 */
__attribute__((deprecated))
dvbpsi_pmt_es_t* dvbpsi_PMTAddES(dvbpsi_pmt_t* p_pmt,
                                 uint8_t i_type, uint16_t i_pid);


/*****************************************************************************
 * dvbpsi_PMTESAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_PMTESAddDescriptor(dvbpsi_pmt_es_t* p_es,
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
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_PMTESAddDescriptor(dvbpsi_pmt_es_t* p_es,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_GenPMTSections
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_GenPMTSections(dvbpsi_pmt_t* p_pmt)
 * \brief PMT generator
 * \param p_pmt PMT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate PMT sections based on the dvbpsi_pmt_t structure.
 */
__attribute__((deprecated))
dvbpsi_psi_section_t* dvbpsi_GenPMTSections(dvbpsi_pmt_t* p_pmt);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of pmt.h"
#endif

