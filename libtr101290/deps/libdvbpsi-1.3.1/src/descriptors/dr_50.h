/*****************************************************************************
 * dr_50.h
 * Copyright (C) 2012 VideoLAN
 *
 * Authors: Roberto Corno <corno.roberto@gmail.com>
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
 * \file <dr_50.h>
 * \author Corno Roberto <corno.roberto@gmail.com>
 * \brief Application interface for the time shifted event
 * descriptor decoder and generator.
 *
 * Application interface for the Component descriptor
 * descriptor decoder and generator. This descriptor's definition can be found in
 * ETSI EN 300 468 section 6.2.8.
 */

#ifndef DR_50_H_
#define DR_50_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_component_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_component_dr_t
 * \brief "Component" descriptor structure.
 *
 * This structure is used to store a decoded "Component"
 * descriptor. (ETSI EN 300 468 section 6.2.8).
 */
/*!
 * \typedef struct dvbpsi_component_dr_s dvbpsi_component_dr_t
 * \brief dvbpsi_component_dr_t type definition.
 */
/*!
 * \struct dvbpsi_component_dr_s
 * \brief struct dvbpsi_component_dr_s @see dvbpsi_component_dr_t
 */
typedef struct dvbpsi_component_dr_t
{
  uint8_t       i_stream_content;         /*!< stream content */
  uint8_t       i_component_type;         /*!< component type */
  uint8_t       i_component_tag;          /*!< component tag */
  uint8_t       i_iso_639_code[3];        /*!< 3 letter ISO 639 language code */
  int           i_text_length;            /*!< text length */
  uint8_t      *i_text;                   /*!< text */

} dvbpsi_component_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeComponentDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_component_dr_t * dvbpsi_DecodeComponentDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "Component" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "Component" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_component_dr_t* dvbpsi_DecodeComponentDr(dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenComponentDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_GenComponentDr(dvbpsi_component_dr_t *p_decoded,
                                                  bool b_duplicate);
 * \brief "Component" descriptor generator.
 * \param p_decoded pointer to a decoded "Component" descriptor structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t *dvbpsi_GenComponentDr(dvbpsi_component_dr_t * p_decoded,
                                                  bool b_duplicate);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_50.h"
#endif /* DR_50_H_ */
