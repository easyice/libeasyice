/*****************************************************************************
 * tot.h
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
 * \file <tot.h>
 * \author Johann Hanne
 * \brief Application interface for the TDT/TOT decoder and the TDT/TOT generator.
 *
 * Application interface for the TDT/TOT decoder and the TDT/TOT generator.
 * New decoded TDT/TOT tables are sent by callback to the application.
 */

#ifndef _DVBPSI_TOT_H_
#define _DVBPSI_TOT_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_tot_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_tot_s
 * \brief TDT/TOT structure.
 *
 * This structure is used to store a decoded TDT/TOT.
 * (ETSI EN 300 468 section 5.2.5/5.2.6).
 */
/*!
 * \typedef struct dvbpsi_tot_s dvbpsi_tot_t
 * \brief dvbpsi_tot_t type definition.
 */
typedef struct dvbpsi_tot_s
{
  uint64_t                  i_utc_time;         /*!< UTC_time */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

  uint32_t      i_crc;                          /*!< CRC_32 (TOT only) */

} dvbpsi_tot_t;


/*****************************************************************************
 * dvbpsi_tot_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_tot_callback)(void* p_cb_data,
                                         dvbpsi_tot_t* p_new_tot)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_tot_callback)(void* p_cb_data, dvbpsi_tot_t* p_new_tot);


/*****************************************************************************
 * dvbpsi_AttachTOT
 *****************************************************************************/
/*!
 * \fn int dvbpsi_AttachTOT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                            dvbpsi_tot_callback pf_callback, void* p_cb_data)
 * \brief Creation and initialization of a TDT/TOT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \param pf_callback function to call back on new TDT/TOT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachTOT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
                     uint16_t i_extension,
                     dvbpsi_tot_callback pf_callback, void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachTOT
 *****************************************************************************/
/*!
 * \fn int dvbpsi_DetachTOT(dvbpsi_demux_t * p_demux, uint8_t i_table_id)
 * \brief Destroy a TDT/TOT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, usually 0x70
 * \param i_extension Table ID extension, unused in the TDT/TOT
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachTOT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
                      uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitTOT/dvbpsi_NewTOT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitTOT(dvbpsi_tot_t* p_tot)
 * \brief Initialize a user-allocated dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \param i_utc_time the time in UTC
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitTOT(dvbpsi_tot_t* p_tot, uint64_t i_utc_time);

/*!
 * \def dvbpsi_NewTOT(p_tot)
 * \brief Allocate and initialize a new dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \param i_utc_time the time in UTC
 * \return nothing.
 */
#define dvbpsi_NewTOT(p_tot, i_utc_time)                                \
do {                                                                    \
  p_tot = (dvbpsi_tot_t*)malloc(sizeof(dvbpsi_tot_t));                  \
  if(p_tot != NULL)                                                     \
    dvbpsi_InitTOT(p_tot, i_utc_time);                                  \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptyTOT/dvbpsi_DeleteTOT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptyTOT(dvbpsi_tot_t* p_tot)
 * \brief Clean a dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptyTOT(dvbpsi_tot_t* p_tot);

/*!
 * \def dvbpsi_DeleteTOT(p_tot)
 * \brief Clean and free a dvbpsi_tot_t structure.
 * \param p_tot pointer to the TDT/TOT structure
 * \return nothing.
 */
#define dvbpsi_DeleteTOT(p_tot)                                         \
do {                                                                    \
  dvbpsi_EmptyTOT(p_tot);                                               \
  free(p_tot);                                                          \
} while(0);


/*****************************************************************************
 * dvbpsi_TOTAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t* dvbpsi_TOTAddDescriptor(dvbpsi_tot_t* p_tot,
                                                    uint8_t i_tag,
                                                    uint8_t i_length,
                                                    uint8_t* p_data)
 * \brief Add a descriptor in the TOT.
 * \param p_tot pointer to the TOT structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
__attribute__((deprecated))
dvbpsi_descriptor_t* dvbpsi_TOTAddDescriptor(dvbpsi_tot_t* p_tot,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t* p_data);


/*****************************************************************************
 * dvbpsi_GenTOTSections
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_GenTOTSections(dvbpsi_Ttot_t* p_tot)
 * \brief TDT/TOT generator
 * \param p_tot TDT/TOT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate TDT/TOT sections based on the dvbpsi_tot_t structure.
 */
__attribute__((deprecated))
dvbpsi_psi_section_t* dvbpsi_GenTOTSections(dvbpsi_tot_t* p_tot);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of tot.h"
#endif

