/*
Copyright (C) 2012  Yonathan Yusim

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

dr_a1.h

Service Location Descriptor.

*/

/*!
 * \file dr_a1.h
 * \author Yonathan Yusim
 * \brief Decode Service Location Descriptor.
 */

#ifndef _DVBPSI_DR_A1_H_
#define _DVBPSI_DR_A1_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
 * dvbpsi_service_location_element_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_location_element_s
 * \brief "service location element" structure
 *
 * This structure is used to store a decoded server location element.
 */
/*!
 * \typedef struct dvbpsi_service_location_element_s dvbpsi_service_location_element_t
 * \brief dvbpsi_service_location_element_t typedefinition.
 */
typedef struct dvbpsi_service_location_element_s
{
    uint8_t i_stream_type;     /*!< service stream type */
    uint16_t i_elementary_pid; /*!< PID of elementary stream */
    char i_iso_639_code[3];    /*!< ISO 936 language code */
} dvbpsi_service_location_element_t;

/*****************************************************************************
 * dvbpsi_service_location_dr_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_service_location_dr_s
 * \brief "service" descriptor structure.
 *
 * This structure is used to store a decoded "service location"
 * descriptor.
 */
/*!
 * \typedef struct dvbpsi_service_location_dr_s dvbpsi_service_location_dr_t
 * \brief dvbpsi_service_location_dr_t type definition.
 */
typedef struct dvbpsi_service_location_dr_s
{
    uint16_t i_pcr_pid;		/*!< PCR_PID */
    uint8_t i_number_elements;	/*!< number of elements used for this service */

    dvbpsi_service_location_element_t elements[0xff]; /*!< service location elements array */

} dvbpsi_service_location_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeServiceLocationDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_service_location_dr_t * dvbpsi_DecodeServiceLocationDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "service location" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "service" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_service_location_dr_t
    *dvbpsi_DecodeServiceLocationDr (dvbpsi_descriptor_t * p_descriptor);

/*****************************************************************************
 * dvbpsi_GenServiceLocationDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenServiceLocationDr(
                        dvbpsi_service_location_dr_t * p_decoded, bool b_duplicate)
 * \brief "service" descriptor generator.
 * \param p_decoded pointer to a decoded "service" descriptor
 * structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t* dvbpsi_GenServiceLocationDr(
                                        dvbpsi_service_location_dr_t* p_decoded,
                                        bool b_duplicate);


#ifdef __cplusplus
}
#endif

#else
#error "Multiple inclusions of dr_a1.h"
#endif
