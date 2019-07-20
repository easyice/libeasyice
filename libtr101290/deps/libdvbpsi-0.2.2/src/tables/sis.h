/*****************************************************************************
 * sis.h
 * Copyright (c) 2010-2011 VideoLAN
 * $Id$
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
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
 * \file <sis.h>
 * \author Jean-Paul Saman <jpsaman@videolan.org>
 * \brief Application interface for the SIS decoder and the SIS generator.
 *
 * Application interface for the SIS decoder and the SIS generator.
 * New decoded SIS tables are sent by callback to the application.
 */

#ifndef _DVBPSI_SIS_H_
#define _DVBPSI_SIS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_sis_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_sis_s
 * \brief SIS structure.
 *
 * This structure is used to store a decoded SIS service description.
 * (SCTE 35 2004 section 7.2).
 */
/*!
 * \typedef struct dvbpsi_sis_s dvbpsi_sis_t
 * \brief dvbpsi_sis_t type definition.
 */
typedef struct dvbpsi_sis_s
{
  /* section */
  uint8_t                   i_protocol_version;     /*!< Protocol version
                                                         shall be 0 */

  /* encryption */
  int                       b_encrypted_packet;     /*!< 1 when packet is
                                                         encrypted */
  uint8_t                   i_encryption_algorithm; /*!< Encryption algorithm
                                                         used */

  uint64_t                  i_pts_adjustment;       /*!< PTS offset */
  uint8_t                   cw_index;               /*!< CA control word */

  /* splice command */
  uint16_t                  i_splice_command_length;/*!< Length of splice command */
  uint8_t                   i_splice_command_type;  /*!< Splice command type */

  /* FIXME: splice_info_section comes here
   * splice_command_type     splice_info_section
   *    0x00                    splice_null()
   *    0x01                    reserved
   *    0x02                    reserved
   *    0x03                    reserved
   *    0x04                    splice_schedule()
   *    0x05                    splice_insert()
   *    0x06                    time_signal()
   *    0x07                    bandwidth_reservation()
   *    0x08 - 0xff             reserved
   */

  /* descriptors */
  uint16_t                  i_descriptors_length;   /*!< Descriptors loop
                                                         length */
  dvbpsi_descriptor_t *     p_first_descriptor;     /*!< First of the following
                                                         DVB descriptors */

  /* FIXME: alignment stuffing */

  uint32_t i_ecrc; /*!< CRC 32 of decrypted splice_info_section */

} dvbpsi_sis_t;

/*****************************************************************************
 * dvbpsi_sis_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_sis_callback)(void* p_cb_data,
                                         dvbpsi_sis_t* p_new_sis)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_sis_callback)(void* p_cb_data, dvbpsi_sis_t* p_new_sis);


/*****************************************************************************
 * dvbpsi_AttachSIS
 *****************************************************************************/
/*!
 * \fn void dvbpsi_AttachSIS(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sis_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a SIS decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension, here TS ID.
 * \param pf_callback function to call back on new SIS.
 * \param p_cb_data private data given in argument to the callback.
 * \return 0 if everything went ok.
 */
__attribute__((deprecated))
int dvbpsi_AttachSIS(dvbpsi_decoder_t * p_psi_decoder, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sis_callback pf_callback,
                               void* p_cb_data);


/*****************************************************************************
 * dvbpsi_DetachSIS
 *****************************************************************************/
/*!
 * \fn void dvbpsi_DetachSIS(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension)
 * \brief Destroy a SIS decoder.
 * \param p_demux Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension, here TS ID.
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_DetachSIS(dvbpsi_demux_t * p_demux, uint8_t i_table_id,
          uint16_t i_extension);


/*****************************************************************************
 * dvbpsi_InitSIS/dvbpsi_NewSIS
 *****************************************************************************/
/*!
 * \fn void dvbpsi_InitSIS(dvbpsi_sis_t* p_sis, uint16_t i_ts_id,
          uint8_t i_version, int b_current_next, uint16_t i_network_id)
 * \brief Initialize a user-allocated dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \param i_protocol_version SIS protocol version (currently 0)
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_InitSIS(dvbpsi_sis_t *p_sis, uint8_t i_protocol_version);

/*!
 * \def dvbpsi_NewSIS(p_sis, i_protocol_version)
 * \brief Allocate and initialize a new dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \param i_protocol_version SIS protocol version (currently 0)
 * \return nothing.
 */
#define dvbpsi_NewSIS(p_sis, i_protocol_version)                        \
do {                                                                    \
  p_sis = (dvbpsi_sis_t*)malloc(sizeof(dvbpsi_sis_t));                  \
  if(p_sis != NULL)                                                     \
    dvbpsi_InitSIS(p_sis, i_protocol_version);                          \
} while(0);

/*****************************************************************************
 * dvbpsi_EmptySIS/dvbpsi_DeleteSIS
 *****************************************************************************/
/*!
 * \fn void dvbpsi_EmptySIS(dvbpsi_sis_t* p_sis)
 * \brief Clean a dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \return nothing.
 */
__attribute__((deprecated))
void dvbpsi_EmptySIS(dvbpsi_sis_t *p_sis);

/*!
 * \def dvbpsi_DeleteSIS(p_sis)
 * \brief Clean and free a dvbpsi_sis_t structure.
 * \param p_sIt pointer to the SIS structure
 * \return nothing.
 */
#define dvbpsi_DeleteSIS(p_sis)                                         \
do {                                                                    \
  dvbpsi_EmptySIS(p_sis);                                               \
  free(p_sis);                                                          \
} while(0);

/*****************************************************************************
 * dvbpsi_SISAddDescriptor
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_SISAddDescriptor(dvbpsi_sis_t *p_sis,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
 * \brief Add a descriptor in the SIS service.
 * \param p_sis pointer to the SIS structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
__attribute__((deprecated))
dvbpsi_descriptor_t *dvbpsi_SISAddDescriptor( dvbpsi_sis_t *p_sis,
                                              uint8_t i_tag, uint8_t i_length,
                                              uint8_t *p_data);

/*****************************************************************************
 * dvbpsi_GenSISSections
 *****************************************************************************
 * Generate SIS sections based on the dvbpsi_sis_t structure.
 *****************************************************************************/
__attribute__((deprecated))
dvbpsi_psi_section_t *dvbpsi_GenSISSections(dvbpsi_sis_t * p_sis);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of sis.h"
#endif

