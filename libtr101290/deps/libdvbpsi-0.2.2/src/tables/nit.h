/*****************************************************************************
 * nit.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johann Hanne
 *          heavily based on pmt.c which was written by
 *          Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 * \file <nit.h>
 * \author Johann Hanne
 * \brief Application interface for the NIT decoder and the NIT generator.
 *
 * Application interface for the NIT decoder and the NIT generator.
 * New decoded NIT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_NIT_H_
#define _DVBPSI_NIT_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_nit_ts_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nit_ts_s
 * \brief NIT TS structure.
 *
 * This structure is used to store a decoded TS description.
 * (ETSI EN 300 468 section 5.2.1).
 */
/*!
 * \typedef struct dvbpsi_nit_ts_s dvbpsi_nit_ts_t
 * \brief dvbpsi_nit_ts_t type definition.
 */
typedef struct dvbpsi_nit_ts_s
{
  uint16_t                      i_ts_id;                /*!< transport stream id */
  uint16_t                      i_orig_network_id;      /*!< original network id */

  dvbpsi_descriptor_t *         p_first_descriptor;     /*!< descriptor list */

  struct dvbpsi_nit_ts_s *      p_next;                 /*!< next element of
                                                             the list */

} dvbpsi_nit_ts_t;


/*****************************************************************************
 * dvbpsi_nit_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_nit_s
 * \brief NIT structure.
 *
 * This structure is used to store a decoded NIT.
 * (ETSI EN 300 468 section 5.2.1).
 */
/*!
 * \typedef struct dvbpsi_nit_s dvbpsi_nit_t
 * \brief dvbpsi_nit_t type definition.
 */
typedef struct dvbpsi_nit_s
{
  uint16_t                  i_network_id;       /*!< network_id */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

  dvbpsi_nit_ts_t *         p_first_ts;         /*!< TS list */

} dvbpsi_nit_t;


/*****************************************************************************
 * dvbpsi_nit_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_nit_callback)(void* p_cb_data,
                                         dvbpsi_nit_t* p_new_nit)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_nit_callback)(void* p_cb_data, dvbpsi_nit_t* p_new_nit);


/*****************************************************************************
 * dvbpsi_AttachNIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachNIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                             uint16_t i_extension, dvbpsi_nit_callback pf_callback,
                             void* p_cb_data)
 * \brief Creation and initialization of a NIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \param pf_callback function to call back on new NIT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachNIT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
                     uint16_t i_extension, dvbpsi_nit_callback pf_callback,
                     void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachNIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachNIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                             uint16_t i_extension)
 * \brief Destroy a NIT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4E, 0x4F, or 0x50-0x6F.
 * \param i_extension Table ID extension, here service ID.
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachNIT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                      uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitNIT/dvbpsi_NewNIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitNIT(dvbpsi_nit_t* p_nit, uint16_t i_network_id,
                           uint8_t i_version, int b_current_next)
 * \brief Initialize a user-allocated dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \param i_network_id network id
 * \param i_version NIT version
 * \param b_current_next current next indicator
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitNIT(dvbpsi_nit_t* p_nit, uint16_t i_network_id,
                    uint8_t i_version, int b_current_next);

/*!
 * \def dvbpsi_NewNIT(p_nit, i_network_id,
                      i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \param i_network_id network id
 * \param i_version NIT version
 * \param b_current_next current next indicator
 * \param i_pcr_pid PCR_PID
 * \return nothing.
 */
#define dvbpsi_NewNIT(p_nit, i_network_id,                              \
                      i_version, b_current_next)                        \
do {                                                                    \
  p_nit = (dvbpsi_nit_t*)malloc(sizeof(dvbpsi_nit_t));                  \
  if(p_nit != NULL)                                                     \
    dvbpsi_InitNIT(p_nit, i_network_id, i_version, b_current_next);     \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptyNIT/dvbpsi_DeleteNIT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptyNIT(dvbpsi_nit_t* p_nit)
 * \brief Clean a dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptyNIT(dvbpsi_nit_t* p_nit);

/*!
 * \def dvbpsi_DeleteNIT(p_nit)
 * \brief Clean and free a dvbpsi_nit_t structure.
 * \param p_nit pointer to the NIT structure
 * \return nothing.
 */
#define dvbpsi_DeleteNIT(p_nit)                                         \
do {                                                                    \
  dvbpsi_EmptyNIT(p_nit);                                               \
  free(p_nit);                                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_NITAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_NITAddDescriptor(dvbpsi_nit_t* p_nit,
                                                    uint8_t i_tag,
                                                    uint8_t i_length,
                                                    uint8_t* p_data)
 * \brief Add a descriptor in the NIT.
 * \param p_nit pointer to the NIT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_NITAddDescriptor(dvbpsi_nit_t* p_nit,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_NITAddTS
 *****************************************************************************/
/*!
 * \fn dvbpsi_nit_ts_t* dvbpsi_NITAddTS(dvbpsi_nit_t* p_nit,
                                        uint8_t i_type, uint16_t i_pid)
 * \brief Add an TS in the NIT.
 * \param p_nit pointer to the NIT structure
 * \param i_type type of TS
 * \param i_pid PID of the TS
 * \return a pointer to the added TS.
 */
__attribute__((deprecated))
dvbpsi_nit_ts_t* dvbpsi_NITAddTS(dvbpsi_nit_t* p_nit,
                                 uint16_t i_ts_id, uint16_t i_orig_network_id);


/*****************************************************************************
 * dvbpsi_NITTSAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_NITTSAddDescriptor(dvbpsi_nit_ts_t* p_ts,
                                                      uint8_t i_tag,
                                                      uint8_t i_length,
                                                      uint8_t* p_data)
 * \brief Add a descriptor in the NIT TS.
 * \param p_ts pointer to the TS structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_NITTSAddDescriptor(dvbpsi_nit_ts_t* p_ts,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_GenNITSections
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_GenNITSections(dvbpsi_nit_t* p_nit,
                                                   uint8_t i_table_id)
 * \brief NIT generator
 * \param p_nit NIT structure
 * \param i_table_id table id, 0x40 = actual network / 0x41 = other network
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate NIT sections based on the dvbpsi_nit_t structure.
 */
__attribute__((deprecated))
dvbpsi_psi_section_t* dvbpsi_GenNITSections(dvbpsi_nit_t* p_nit,
                                            uint8_t i_table_id);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of nit.h"
#endif

