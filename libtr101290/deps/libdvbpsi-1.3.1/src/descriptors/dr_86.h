/*
Copyright (C) 2013-2014  Michael Ira Krufky

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

dr_86.h

Decode Caption Service Descriptor.

*/

/*!
 * \file dr_86.h
 * \author Michael Ira Krufky
 * \brief Decode Caption Service Descriptor.
 */

#ifndef _DR_86_H
#define _DR_86_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_caption_service_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_caption_service_s
 * \brief Caption Service
 *
 * This structure is used to store a decoded Caption Service.
 */
/*!
 * \typedef struct dvbpsi_caption_service_s dvbpsi_caption_service_t
 * \brief dvbpsi_caption_service_t type definition.
 */
typedef struct dvbpsi_caption_service_s
{
    char     i_iso_639_code[3]; /*!< ISO 639 language code */
    int      b_digital_cc;      /*!< Digital CC  flag */
    int      b_line21_field;    /*!< Line 21 */
    uint16_t i_caption_service_number; /*!< Caption Service Number */
    int      b_easy_reader;     /*!< Easy reader flag */
    int      b_wide_aspect_ratio; /*!< Wide aspect ratio flag */
} dvbpsi_caption_service_t;

/*****************************************************************************
 * dvbpsi_caption_service_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_caption_service_dr_s
 * \brief Caption Service Descriptor
 *
 * This structure is used to store a decoded Caption Service descriptor.
 */
/*!
 * \typedef struct dvbpsi_caption_service_dr_s dvbpsi_caption_service_dr_t
 * \brief dvbpsi_caption_service_dr_t type definition.
 */
typedef struct dvbpsi_caption_service_dr_s
{
    uint8_t i_number_of_services; /*!< Number of Captions services */
    dvbpsi_caption_service_t services[0x1f]; /*!< Caption services array */
} dvbpsi_caption_service_dr_t;

/*****************************************************************************
 * dvbpsi_DecodeCaptionServiceDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_caption_service_dr_t dvbpsi_DecodeCaptionServiceDr(dvbpsi_descriptor_t *p_descriptor)
 * \brief Decode a Caption Service descriptor (tag 0x86)
 * \param p_descriptor Raw descriptor to decode.
 * \return NULL if the descriptor could not be decoded or a pointer to a
 *         dvbpsi_caption_service_dr_t structure.
 */
dvbpsi_caption_service_dr_t *dvbpsi_DecodeCaptionServiceDr(dvbpsi_descriptor_t *p_descriptor);

#ifdef __cplusplus
}
#endif

#endif

