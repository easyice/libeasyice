/*****************************************************************************
 * bat.h
 * Copyright (C) 2001-2011 VideoLAN
 * $Id: bat.h  $
 *
 * Authors: Zhu zhenglu <zhuzlu@gmail.com>
 *          heavily based on nit.h which was written by
 *          Johann Hanne
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
 *
 *****************************************************************************/

/*!
 * \file bat.h
 * \author Zhu zhenglu <zhuzlu@gmail.com>
 * \brief Application interface for the BAT decoder.
 *
 * Application interface for the BAT decoder. New
 * decoded BAT are sent by callback to the application. If a table
 * wasn't active (b_current_next == 0) and the next is the same but active
 * (b_current_next == 1) then the service description list is empty and should
 * be caught from the previous structure.
 * This is a simulation to sdt.h
 */

#ifndef _DVBPSI_BAT_H_
#define _DVBPSI_BAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_bat_ts_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_bat_ts_s
 * \brief BAT transport stream description structure.
 *
 * This structure is used to store a decoded BAT service description.
 * (ETSI EN 300 468 V1.5.1 section 5.2.2).
 */
/*!
 * \typedef struct dvbpsi_bat_ts_s dvbpsi_bat_ts_t
 * \brief dvbpsi_bat_ts_t type definition.
 */
typedef struct dvbpsi_bat_ts_s
{
  uint16_t                      i_ts_id;                /*!< transport stream id */
  uint16_t                      i_orig_network_id;      /*!< original network id */

  dvbpsi_descriptor_t *         p_first_descriptor;     /*!< descriptor list */


  struct dvbpsi_bat_ts_s * p_next;             /*!< next element of
                                                             the list */

} dvbpsi_bat_ts_t;


/*****************************************************************************
 * dvbpsi_bat_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_bat_s
 * \brief BAT structure.
 *
 * This structure is used to store a decoded BAT.
 * (ETSI EN 300 468 V1.5.1 section 5.2.2).
 */
/*!
 * \typedef struct dvbpsi_bat_s dvbpsi_bat_t
 * \brief dvbpsi_bat_t type definition.
 */
typedef struct dvbpsi_bat_s
{
  uint16_t                  i_bouquet_id;            /*!< bouquet_id */
  uint8_t                   i_version;          /*!< version_number */
  int                       b_current_next;     /*!< current_next_indicator */

  dvbpsi_descriptor_t *     p_first_descriptor; /*!< descriptor list */

  dvbpsi_bat_ts_t *         p_first_ts;    /*!< transport stream description
                                                     list */

} dvbpsi_bat_t;


/*****************************************************************************
 * dvbpsi_bat_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_bat_callback)(void* p_cb_data,
                                         dvbpsi_bat_t* p_new_bat)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_bat_callback)(void* p_cb_data, dvbpsi_bat_t* p_new_bat);


/*****************************************************************************
 * dvbpsi_AttachBAT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachBAT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_bat_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a BAT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \param pf_callback function to call back on new BAT.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachBAT(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_bat_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachBAT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachBAT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a BAT decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0x4a.
 * \param i_extension Table ID extension, here bouquet ID.
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachBAT(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitBAT/dvbpsi_NewBAT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitBAT(dvbpsi_bat_t *p_bat, uint16_t i_bouquet_id, uint8_t i_version,
                    int b_current_next)
 * \brief Initialize a user-allocated dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \param i_bouquet_id bouquet ID
 * \param i_version BAT version
 * \param b_current_next current next indicator
 * \param i_network_id original network id
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitBAT(dvbpsi_bat_t *p_bat, uint16_t i_bouquet_id, uint8_t i_version,
                    int b_current_next);

/*!
 * \def dvbpsi_NewBAT(p_bat, i_bouquet_id, i_version, b_current_next)
 * \brief Allocate and initialize a new dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \param i_bouquet_id bouquet ID
 * \param i_version BAT version
 * \param b_current_next current next indicator
 * \param i_network_id original network id
 * \return nothing.
 */
#define dvbpsi_NewBAT(p_bat, i_bouquet_id, i_version, b_current_next) \
do {                                                                    \
  p_bat = (dvbpsi_bat_t*)malloc(sizeof(dvbpsi_bat_t));                  \
  if(p_bat != NULL)                                                     \
    dvbpsi_InitBAT(p_bat, i_bouquet_id, i_version, b_current_next); \
} while(0);


/*****************************************************************************
 * dvbpsi_EmptyBAT/dvbpsi_DeleteBAT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptyBAT(dvbpsi_bat_t* p_bat)
 * \brief Clean a dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptyBAT(dvbpsi_bat_t *p_bat);

/*!
 * \def dvbpsi_DeleteBAT(p_bat)
 * \brief Clean and free a dvbpsi_bat_t structure.
 * \param p_bat pointer to the BAT structure
 * \return nothing.
 */
#define dvbpsi_DeleteBAT(p_bat)                                         \
do {                                                                    \
  dvbpsi_EmptyBAT(p_bat);                                               \
  free(p_bat);                                                          \
} while(0);

/*****************************************************************************
 * dvbpsi_GenBATSections
 *****************************************************************************
/*!
 * \fn dvbpsi_psi_section_t* dvbpsi_GenBATSections(dvbpsi_bat_t* p_bat)
 * \brief BAT generator
 * \param p_bat BAT structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate BAT sections based on the dvbpsi_bat_t structure.
 *****************************************************************************/
__attribute__((deprecated))
dvbpsi_psi_section_t *dvbpsi_GenBATSections(dvbpsi_bat_t * p_bat);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of bat.h"
#endif

