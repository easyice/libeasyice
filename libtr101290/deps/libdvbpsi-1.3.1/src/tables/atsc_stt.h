/*
Copyright (C) 2006-2012  Adam Charrett

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

stt.h

*/

/*!
 * \file atsc_stt.h
 * \author Adam Charrett
 * \brief Decode PSIP System Time Table (ATSC STT).
 */

#ifndef _ATSC_STT_H
#define _ATSC_STT_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * dvbpsi_atsc_stt_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_atsc_stt_s
 * \brief STT structure.
 *
 * This structure is used to store a decoded STT.
 */
/*!
 * \typedef struct dvbpsi_atsc_stt_s dvbpsi_atsc_stt_t
 * \brief dvbpsi_atsc_stt_t type definition.
 */
typedef struct dvbpsi_atsc_stt_s
{
    uint8_t                 i_table_id;         /*!< Table id */
    uint16_t                i_extension;        /*!< Subtable id */

    uint8_t                 i_version;          /*!< PSIP Protocol version */
    bool                    b_current_next;     /*!< current_next_indicator */

    uint32_t                i_system_time;      /*!< GPS seconds since 1 January 1980 00:00:00 UTC. */
    uint8_t                 i_gps_utc_offset;   /*!< Seconds offset between GPS and UTC time. */
    uint16_t                i_daylight_savings; /*!< Daylight savings control bytes. */

    dvbpsi_descriptor_t    *p_first_descriptor; /*!< First descriptor. */
} dvbpsi_atsc_stt_t;

/*****************************************************************************
 * dvbpsi_atsc_stt_callback
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_atsc_stt_callback)(void* p_cb_data,
                                              dvbpsi_atsc_stt_t* p_new_stt)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_atsc_stt_callback)(void* p_cb_data, dvbpsi_atsc_stt_t* p_new_stt);

/*****************************************************************************
 * dvbpsi_atsc_AttachSTT
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_atsc_AttachSTT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
          dvbpsi_atsc_stt_callback pf_stt_callback, void* p_cb_data)
 * \brief Creation and initialization of a STT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table ID extension, here it should be 0.
 * \param pf_stt_callback function to call back on new STT.
 * \param p_cb_data private data given in argument to the callback.
 * \return true if everything went ok else false
 */
bool dvbpsi_atsc_AttachSTT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
          dvbpsi_atsc_stt_callback pf_stt_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_atsc_DetachSTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_DetachSTT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
 *
 * \brief Destroy a STT decoder.
 * \param p_dvbpsi dvbpsi handle to Subtable demultiplexor to which the decoder is attached.
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table extension, ignored as this should always be 0.
 *                    (Required to match prototype for demux)
 * \return nothing.
 */
void dvbpsi_atsc_DetachSTT(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_externsion);

/*****************************************************************************
 * dvbpsi_atsc_InitSTT/dvbpsi_atsc_NewSTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_InitSTT(dvbpsi_atsc_stt_t *p_stt, uint8_t i_table_id,
                                uint16_t i_extension, uint8_t i_protocol,
                                bool b_current_next)
 * \brief Initialize a user-allocated dvbpsi_atsc_stt_t structure.
 * \param p_stt pointer to the STT structure
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table extension, ignored as this should always be 0.
 * \param i_protocol PSIP Protocol version.
 * \param b_current_next current next indicator
 * \return nothing.
 */
void dvbpsi_atsc_InitSTT(dvbpsi_atsc_stt_t *p_stt, uint8_t i_table_id, uint16_t i_extension,
                         uint8_t i_protocol, bool b_current_next);

/*!
 * \fn dvbpsi_atsc_stt_t *dvbpsi_atsc_NewSTT(uint8_t i_table_id, uint16_t i_extension,
                                             uint8_t i_version, bool b_current_next)
 * \brief Allocate and initialize a new dvbpsi_atsc_stt_t structure. Use ObjectRefDec to delete it.
 * \param i_table_id Table ID, 0xCD.
 * \param i_extension Table extension, ignored as this should always be 0.
 * \param i_version PSIP Protocol version.
 * \param b_current_next current next indicator
 * \return p_stt pointer to the STT structure
 */
dvbpsi_atsc_stt_t *dvbpsi_atsc_NewSTT(uint8_t i_table_id, uint16_t i_extension,
                                      uint8_t i_version, bool b_current_next);

/*****************************************************************************
 * dvbpsi_atsc_EmptySTT
 *****************************************************************************/
/*!
 * \fn void dvbpsi_atsc_EmptySTT(dvbpsi_atsc_stt_t* p_stt)
 * \brief Clean a dvbpsi_atsc_stt_t structure.
 * \param p_stt pointer to the STT structure
 * \return nothing.
 */
void dvbpsi_atsc_EmptySTT(dvbpsi_atsc_stt_t *p_stt);

/*!
 * \fn dvbpsi_atsc_DeleteSTT(dvbpsi_atsc_stt_t *p_stt)
 * \brief Clean and free a dvbpsi_atsc_stt_t structure.
 * \param p_stt pointer to the STT structure
 * \return nothing.
 */
void dvbpsi_atsc_DeleteSTT(dvbpsi_atsc_stt_t *p_stt);

#ifdef __cplusplus
};
#endif

#endif
