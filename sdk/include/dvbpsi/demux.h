/*****************************************************************************
 * demux.h
 *
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
 * \file <demux.h>
 * \author Johan Bilien <jobi@via.ecp.fr>
 * \brief Subtable demutiplexor.
 *
 * Subtable demultiplexor structure
 */

#ifndef _DVBPSI_DEMUX_H_
#define _DVBPSI_DEMUX_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * dvbpsi_demux_new_cb_t
 *****************************************************************************/
/*!
 * \typedef void(* dvbpsi_demux_new_cb_t) (void *   p_cb_data,
                                           dvbpsi_handle h_dvbpsi,
                                           uint8_t  i_table_id,
                                           uint16_t i_extension);
 * \brief Callback used in case of a new subtable detected.
 */
typedef void (*dvbpsi_demux_new_cb_t) (void *   p_cb_data,
                                       dvbpsi_handle h_dvbpsi,
                                       uint8_t  i_table_id,
                                       uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_demux_subdec_cb_t
 *****************************************************************************/
/*!
 * \typedef void (*dvbpsi_demux_subdec_cb_t)
                    (dvbpsi_decoder_t*     p_psi_decoder,
                     void*                 p_private_decoder,
                     dvbpsi_psi_section_t* p_section);
 * \brief Subtable specific decoder.
 */
typedef void (*dvbpsi_demux_subdec_cb_t)
                     (dvbpsi_decoder_t*     p_psi_decoder,
                      void*                 p_private_decoder,
                      dvbpsi_psi_section_t* p_section);

/*****************************************************************************
 * dvbpsi_demux_subdec_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_demux_subdec_s
 * \brief Subtable decoder structure
 *
 * This structure contains the data specific to the decoding of one
 * subtable.
 */
/*!
 * \typedef struct dvbpsi_demux_subdec_s dvbpsi_demux_subdec_t
 * \brief dvbpsi_demux_subdec_t type definition.
 */
struct dvbpsi_demux_s;
typedef struct dvbpsi_demux_subdec_s
{
  uint32_t                        i_id;
  dvbpsi_demux_subdec_cb_t        pf_callback;
  void *                          p_cb_data;
  struct dvbpsi_demux_subdec_s *  p_next;

  void (*pf_detach)(struct dvbpsi_demux_s *, uint8_t, uint16_t);

} dvbpsi_demux_subdec_t;

/*****************************************************************************
 * dvbpsi_demux_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_demux_s
 * \brief subtable demultiplexor structure
 *
 * This structure contains the subtables demultiplexor data, such as the
 * decoders and new subtable callback.
 */
/*!
 * \typedef struct dvbpsi_demux_s dvbpsi_demux_t
 * \brief dvbpsi_demux_t type definition.
 */
typedef struct dvbpsi_demux_s
{
  dvbpsi_handle             p_decoder;          /*!< Parent PSI Decoder */
  dvbpsi_demux_subdec_t *   p_first_subdec;     /*!< First subtable decoder */
  /* New subtable callback */
  dvbpsi_demux_new_cb_t     pf_new_callback;    /*!< New subtable callback */
  void *                    p_new_cb_data;      /*!< Data provided to the
                                                     previous callback */

} dvbpsi_demux_t;


/*****************************************************************************
 * dvbpsi_AttachDemux
 *****************************************************************************/
/*!
 * \fn dvbpsi_handle_t dvbpsi_NewPSISection(dvbpsi_demux_new_cb_t pf_new_cb, void * p_new_cb_data)
 * \brief Creates a new demux structure.
 * \param pf_new_cb A callcack called when a new type of subtable is found.
 * \param p_new_cb_data Data given to the previous callback.
 * \return a handle to the new demux structure.
 */
__attribute__((deprecated))
dvbpsi_handle dvbpsi_AttachDemux(dvbpsi_demux_new_cb_t pf_new_cb,
                                 void *                p_new_cb_data);

/*****************************************************************************
 * dvbpsi_DetachDemux
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachDemux(dvbpsi_handle h_dvbpsi)
 * \brief Destroys a demux structure.
 * \param h_dvbpsi The handle of the demux to be destroyed.
 */
__attribute__((deprecated))
void dvbpsi_DetachDemux(dvbpsi_handle h_dvbpsi);

/*****************************************************************************
 * dvbpsi_demuxGetSubDec
 *****************************************************************************/
/*!
 * \fn dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t *, uint8_t, uint16_t)
 * \brief Looks for a subtable decoder, given the subtable ID.
 * \param p_demux Pointer to the demux structure.
 * \param i_table_id Table ID of the wanted subtable.
 * \param i_extension Table ID extension of the wanted subtable.
 * \return a pointer to the found subdecoder, or NULL.
 *
 */
__attribute__((deprecated))
dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t * p_demux,
                                              uint8_t          i_table_id,
                                              uint16_t         i_extension);
/*****************************************************************************
 * dvbpsi_Demux
 *****************************************************************************/
/*!
 * \fn void dvbpsi_Demux(dvbpsi_handle h_dvbpsi,
                         dvbpsi_psi_section_t * p_section)
 * \brief Sends the PSI sections to the right subtable decoder according to their table ID and extension.
 * \param h_dvbpsi PSI decoder handle.
 * \param p_section PSI section.
 */
__attribute__((deprecated))
void dvbpsi_Demux(dvbpsi_handle h_dvbpsi,
                  dvbpsi_psi_section_t * p_section);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of demux.h"
#endif

