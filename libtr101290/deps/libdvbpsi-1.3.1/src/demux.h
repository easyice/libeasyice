/*****************************************************************************
 * demux.h
 *
 * Copyright (C) 2001-2011 VideoLAN
 * $Id$
 *
 * Authors: Johan Bilien <jobi@via.ecp.fr>
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
 * \typedef void(* dvbpsi_demux_new_cb_t) (dvbpsi_t *p_dvbpsi,
                                           uint8_t  i_table_id,
                                           uint16_t i_extension,
                                           void *   p_cb_data);
 * \brief Callback used in case of a new subtable detected.
 */
typedef void (*dvbpsi_demux_new_cb_t) (dvbpsi_t *p_dvbpsi,  /*!< pointer to dvbpsi handle */
                                       uint8_t  i_table_id, /*!< table id to attach */
                                       uint16_t i_extension,/*!< table extention to attach */
                                       void *   p_cb_data); /*!< pointer to callback data */

/*!
 * \typedef void(*dvbpsi_demux_gather_cb_t)(dvbpsi_t *p_dvbpsi,
                                            void *p_cb_data,
                                            dvbpsi_psi_section_t *p_section);
 * \brief Callback used for gathering psi sections on behalf of subtable decoders.
 */
typedef void (*dvbpsi_demux_gather_cb_t) (dvbpsi_t *p_dvbpsi, /*!< pointer to dvbpsi handle */
                                          dvbpsi_decoder_t *p_decoder, /*!< pointer to decoder */
                                          dvbpsi_psi_section_t *p_section); /*!< pointer to psi section */

/*!
 * \typedef void (*dvbpsi_demux_detach_cb_t) (dvbpsi_t *p_dvbpsi,
                                              uint8_t i_table_id,
                                              uint16_t i_extension);
 * \brief Callback used for detaching subtable decoder from demuxer
 */
typedef void (*dvbpsi_demux_detach_cb_t) (dvbpsi_t *p_dvbpsi,    /*!< pointer to dvbpsi handle */
                                          uint8_t i_table_id,    /*!< table id to detach */
                                          uint16_t i_extension); /*!< table extention to detach */

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
typedef struct dvbpsi_demux_subdec_s
{
  uint32_t                      i_id;      /*!< subtable id */

  dvbpsi_demux_gather_cb_t      pf_gather; /*!< gather subdec callback */
  dvbpsi_decoder_t             *p_decoder; /*!< private decoder for this subdec */

  dvbpsi_demux_detach_cb_t      pf_detach; /*!< detach subdec callback */

  struct dvbpsi_demux_subdec_s *p_next;    /*!< next subdec */
} dvbpsi_demux_subdec_t;


/*****************************************************************************
 * dvbpsi_demux_s
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
typedef struct dvbpsi_demux_s dvbpsi_demux_t;

struct dvbpsi_demux_s
{
    DVBPSI_DECODER_COMMON

    dvbpsi_demux_subdec_t *   p_first_subdec;     /*!< First subtable decoder */

    /* New subtable callback */
    dvbpsi_demux_new_cb_t     pf_new_callback;    /*!< New subtable callback */
    void *                    p_new_cb_data;      /*!< Data provided to the
                                                     previous callback */
};

/*****************************************************************************
 * dvbpsi_AttachDemux
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_AttachDemux(dvbpsi_t *p_dvbpsi, dvbpsi_demux_new_cb_t pf_new_cb, void * p_new_cb_data)
 * \brief Attaches a new demux structure on dvbpsi_t* handle.
 * \param p_dvbpsi pointer to dvbpsi_t handle
 * \param pf_new_cb A callcack called when a new type of subtable is found.
 * \param p_new_cb_data Data given to the previous callback.
 * \return true on success, false on failure
 */
bool dvbpsi_AttachDemux(dvbpsi_t *            p_dvbpsi,
                        dvbpsi_demux_new_cb_t pf_new_cb,
                        void *                p_new_cb_data);

/*****************************************************************************
 * dvbpsi_DetachDemux
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachDemux(dvbpsi_t *p_dvbpsi)
 * \brief Destroys a demux structure.
 * \param p_dvbpsi The handle of the demux to be destroyed.
 * \return nothing
 */
void dvbpsi_DetachDemux(dvbpsi_t *p_dvbpsi);

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
dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t * p_demux,
                                              uint8_t          i_table_id,
                                              uint16_t         i_extension);

/*****************************************************************************
 * dvbpsi_Demux
 *****************************************************************************/
/*!
 * \fn void dvbpsi_Demux(dvbpsi_t *p_dvbpsi,
                         dvbpsi_psi_section_t * p_section)
 * \brief Sends the PSI sections to the right subtable decoder according to their table ID and extension.
 * \param p_dvbpsi PSI decoder handle.
 * \param p_section PSI section.
 */
void dvbpsi_Demux(dvbpsi_t *p_dvbpsi, dvbpsi_psi_section_t *p_section);

/*****************************************************************************
 * dvbpsi_NewDemuxSubDecoder
 *****************************************************************************/
/*!
 * \fn dvbpsi_demux_subdec_t *dvbpsi_NewDemuxSubDecoder(const uint8_t i_table_id,
                                                 const uint16_t i_extension,
                                                 dvbpsi_demux_detach_cb_t pf_detach,
                                                 dvbpsi_demux_gather_cb_t pf_gather,
                                                 dvbpsi_decoder_t *p_decoder)
 * \brief Allocates a new demux sub table decoder and initializes it.
 * \param i_table_id table id to create subtable decoder for
 * \param i_extension table extension to create subtable decoder for
 * \param pf_detach pointer to detach function for subtable decoder.
 * \param pf_gather pointer to gather function for subtable decoder.
 * \param p_decoder pointer to private decoder.
 * \return pointer to demux subtable decoder.
 */
dvbpsi_demux_subdec_t *dvbpsi_NewDemuxSubDecoder(const uint8_t i_table_id,
                                                 const uint16_t i_extension,
                                                 dvbpsi_demux_detach_cb_t pf_detach,
                                                 dvbpsi_demux_gather_cb_t pf_gather,
                                                 dvbpsi_decoder_t *p_decoder);

/*****************************************************************************
 * dvbpsi_DeleteDemuxSubDecoder
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DeleteDemuxSubDecoder(dvbpsi_demux_subdec_t *p_subdec)
 * \brief Releases memory allocated with @see dvbpsi_NewDemuxSubDecoder. It will
 * also release p_cb_data pointer.
 * \param p_subdec pointer to demux subtable decoder.
 * \return nothing.
 */
void dvbpsi_DeleteDemuxSubDecoder(dvbpsi_demux_subdec_t *p_subdec);

/*****************************************************************************
 * dvbpsi_AttachDemuxSubDecoder
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec)
 * \brief Attach a subtable decoder to the given demux handle.
 * \param p_demux pointer to dvbpsi_demux_t
 * \param p_subdec pointer to dvbpsi_demux_subdec_t
 * \return nothing
 */
void dvbpsi_AttachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec);

/*****************************************************************************
 * dvbpsi_DetachDemuxSubDecoder
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec)
 * \brief Detach a subtable decoder from the given demux pointer.
 * \param p_demux pointer to dvbpsi_demux_t
 * \param p_subdec pointer to dvbpsi_demux_subdec_t
 * \return nothing
 */
void dvbpsi_DetachDemuxSubDecoder(dvbpsi_demux_t *p_demux, dvbpsi_demux_subdec_t *p_subdec);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of demux.h"
#endif
