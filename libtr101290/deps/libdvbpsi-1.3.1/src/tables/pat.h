/*****************************************************************************
 * pat.h
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
 * \file <pat.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the PAT decoder and the PAT generator.
 *
 * Application interface for the PAT decoder and the PAT generator.
 * New decoded PAT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_PAT_H_
#define _DVBPSI_PAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_pat_program_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_pat_program_s
 * \brief PAT program structure.
 *
 * This structure is used to store a decoded PAT program.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 */
/*!
 * \typedef struct dvbpsi_pat_program_s dvbpsi_pat_program_t
 * \brief dvbpsi_pat_program_t type definition.
 */
typedef struct dvbpsi_pat_program_s
{
  uint16_t                      i_number;               /*!< program_number */
  uint16_t                      i_pid;                  /*!< PID of NIT/PMT */

  struct dvbpsi_pat_program_s * p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_pat_program_t;


/*****************************************************************************
 * dvbpsi_pat_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_pat_s
 * \brief PAT structure.
 *
 * This structure is used to store a decoded PAT.
 * (ISO/IEC 13818-1 section 2.4.4.3).
 */
/*!
 * \typedef struct dvbpsi_pat_s dvbpsi_pat_t
 * \brief dvbpsi_pat_t type definition.
 */
typedef struct dvbpsi_pat_s
{
  uint16_t                  i_ts_id;            /*!< transport_stream_id */
  uint8_t                   i_version;          /*!< version_number */
  bool                      b_current_next;     /*!< current_next_indicator */

  dvbpsi_pat_program_t *    p_first_program;    /*!< program list */

} dvbpsi_pat_t;


/*****************************************************************************
 * dvbpsi_pat_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_pat_callback)(void* p_cb_data,
                                         dvbpsi_pat_t* p_new_pat)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_pat_callback)(void* p_cb_data, dvbpsi_pat_t* p_new_pat);

/*****************************************************************************
 * dvbpsi_pat_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_pat_attach(dvbpsi_t *p_dvbpsi, dvbpsi_pat_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a PAT decoder. The decoder will be attached to 'p_dvbpsi' argument.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param pf_callback function to call back on new PAT
 * \param p_cb_data private data given in argument to the callback
 * \return true on success, false on failure
 */
bool dvbpsi_pat_attach(dvbpsi_t *p_dvbpsi, dvbpsi_pat_callback pf_callback,
                       void* p_cb_data);

/*****************************************************************************
 * dvbpsi_pat_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pat_detach(dvbpsi_t *p_dvbpsi)
 * \brief Destroy a PAT decoder.
 * \param p_dvbpsi pointer to dvbpsi_t handle
 * \return nothing.
 *
 * The handle isn't valid any more.
 */
void dvbpsi_pat_detach(dvbpsi_t *p_dvbpsi);

/*****************************************************************************
 * dvbpsi_pat_init/dvbpsi_pat_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pat_init(dvbpsi_pat_t* p_pat, uint16_t i_ts_id,
                           uint8_t i_version, bool b_current_next)
 * \brief Initialize a user-allocated dvbpsi_pat_t structure.
 * \param p_pat pointer to the PAT structure
 * \param i_ts_id transport stream ID
 * \param i_version PAT version
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_pat_init(dvbpsi_pat_t* p_pat, uint16_t i_ts_id, uint8_t i_version,
                     bool b_current_next);

/*!
 * \fn dvbpsi_pat_t *dvbpsi_pat_new(uint16_t i_ts_id, uint8_t i_version,
 *                                 bool b_current_next);
 * \brief Allocate and initialize a new dvbpsi_pat_t structure.
 * \param i_ts_id transport stream ID
 * \param i_version PAT version
 * \param b_current_next current next indicator
 * \return p_pat pointer to the PAT structure
 */
dvbpsi_pat_t *dvbpsi_pat_new(uint16_t i_ts_id, uint8_t i_version, bool b_current_next);

/*****************************************************************************
 * dvbpsi_pat_empty/dvbpsi_pat_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_pat_empty(dvbpsi_pat_t* p_pat)
 * \brief Clean a dvbpsi_pat_t structure.
 * \param p_pat pointer to the PAT structure
 * \return nothing.
 */
void dvbpsi_pat_empty(dvbpsi_pat_t* p_pat);

/*!
 * \fn void dvbpsi_pat_delete(dvbpsi_pat_t *p_pat)
 * \brief Clean and free a dvbpsi_pat_t structure.
 * \param p_pat pointer to the PAT structure
 * \return nothing.
 */
void dvbpsi_pat_delete(dvbpsi_pat_t *p_pat);

/*****************************************************************************
 * dvbpsi_pat_program_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_pat_program_t* dvbpsi_pat_program_add(dvbpsi_pat_t* p_pat,
                                                    uint16_t i_number,
                                                    uint16_t i_pid)
 * \brief Add a program at the end of the PAT.
 * \param p_pat pointer to the PAT structure
 * \param i_number program number
 * \param i_pid PID of the NIT/PMT
 * \return a pointer to the added program.
 */
dvbpsi_pat_program_t* dvbpsi_pat_program_add(dvbpsi_pat_t* p_pat,
                                             uint16_t i_number, uint16_t i_pid);

/*****************************************************************************
 * dvbpsi_pat_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_pat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_pat_t* p_pat,
                                                   int i_max_pps);
 * \brief PAT generator.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_pat pointer to the PAT structure
 * \param i_max_pps limitation of the number of program in each section
 * (max: 253).
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate PAT sections based on the dvbpsi_pat_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_pat_sections_generate(dvbpsi_t *p_dvbpsi,
                                            dvbpsi_pat_t* p_pat, int i_max_pps);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of pat.h"
#endif
