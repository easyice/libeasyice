/*****************************************************************************
 * cat.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.h which was written by
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
 * \file <cat.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 * \brief Application interface for the CAT decoder and the CAT generator.
 *
 * Application interface for the CAT decoder and the CAT generator.
 * New decoded CAT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_CAT_H_
#define _DVBPSI_CAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_cat_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_cat_s
 * \brief CAT structure.
 *
 * This structure is used to store a decoded CAT.
 * (ISO/IEC 13818-1 section 2.4.4.6).
 */
/*!
 * \typedef struct dvbpsi_cat_s dvbpsi_cat_t
 * \brief dvbpsi_cat_t type definition.
 */
typedef struct dvbpsi_cat_s
{
  uint8_t                   i_version;          /*!< version_number */
  bool                      b_current_next;     /*!< current_next_indicator */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

} dvbpsi_cat_t;

/*****************************************************************************
 * dvbpsi_cat_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_cat_callback)(void* p_cb_data,
                                         dvbpsi_cat_t* p_new_cat)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_cat_callback)(void* p_cb_data, dvbpsi_cat_t* p_new_cat);

/*****************************************************************************
 * dvbpsi_cat_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_cat_attach(dvbpsi_t *p_dvbpsi,
                            dvbpsi_cat_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a CAT decoder. It will be attached to p_dvbpsi
 * \param p_dvbpsi is a pointer to dvbpsi_t which holds a pointer to the decoder
 * \param pf_callback function to call back on new CAT
 * \param p_cb_data private data given in argument to the callback
 * \return true on success, false on failure
 */
bool dvbpsi_cat_attach(dvbpsi_t *p_dvbpsi, dvbpsi_cat_callback pf_callback,
                      void* p_cb_data);

/*****************************************************************************
 * dvbpsi_cat_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_cat_detach(dvbpsi_t *p_dvbpsi)
 * \brief Destroy a CAT decoder.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_dvbpsi handle holds the decoder pointer
 * \return nothing.
 *
 * The handle isn't valid any more.
 */
void dvbpsi_cat_detach(dvbpsi_t *p_dvbpsi);

/*****************************************************************************
 * dvbpsi_cat_init/dvbpsi_cat_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_cat_init(dvbpsi_cat_t* p_cat,
                           uint8_t i_version, bool b_current_next)
 * \brief Initialize a user-allocated dvbpsi_cat_t structure.
 * \param p_cat pointer to the CAT structure
 * \param i_version CAT version
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_cat_init(dvbpsi_cat_t* p_cat,
                    uint8_t i_version, bool b_current_next);

/*!
 * \fn dvbpsi_cat_t *dvbpsi_cat_new(uint8_t i_version,
 *                                  bool b_current_next)
 * \brief Allocate and initialize a new dvbpsi_cat_t structure.
 * \param i_version CAT version
 * \param b_current_next current next indicator
 * \return p_cat pointer to the CAT structure
 */
dvbpsi_cat_t *dvbpsi_cat_new(uint8_t i_version, bool b_current_next);

/*****************************************************************************
 * dvbpsi_cat_empty/dvbpsi_cat_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_cat_empty(dvbpsi_cat_t* p_cat)
 * \brief Clean a dvbpsi_cat_t structure.
 * \param p_cat pointer to the CAT structure
 * \return nothing.
 */
void dvbpsi_cat_empty(dvbpsi_cat_t* p_cat);

/*!
 * \fn void dvbpsi_cat_delete(dvbpsi_cat_t *p_cat)
 * \brief Clean and free a dvbpsi_cat_t structure.
 * \param p_cat pointer to the CAT structure
 * \return nothing.
 */
void dvbpsi_cat_delete(dvbpsi_cat_t *p_cat);

/*****************************************************************************
 * dvbpsi_cat_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_cat_descriptor_add(dvbpsi_cat_t* p_cat,
                                                      uint8_t i_tag,
                                                      uint8_t i_length,
                                                      uint8_t* p_data)
 * \brief Add a descriptor in the CAT.
 * \param p_cat pointer to the CAT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t* dvbpsi_cat_descriptor_add(dvbpsi_cat_t* p_cat,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_cat_sections_generate
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_cat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_cat_t* p_cat)
 * \brief CAT generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_cat CAT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate CAT sections based on the dvbpsi_cat_t structure.
 */
dvbpsi_psi_section_t* dvbpsi_cat_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_cat_t* p_cat);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of cat.h"
#endif

