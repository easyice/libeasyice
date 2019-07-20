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
  uint8_t                   i_table_id;         /*!< table id */
  uint16_t                  i_extension;        /*!< subtable id */

  uint16_t                  i_ts_id;            /*!< transport_stream_id */
  uint8_t                   i_version;          /*!< version_number */
  uint8_t                   i_protocol_version; /*!< Protocol version
                                                     shall be 0 */
  bool                      b_current_next;     /*!< current_next_indicator */

  /* encryption */
  bool                      b_encrypted_packet;     /*!< 1 when packet is
                                                         encrypted */
  uint8_t                   i_encryption_algorithm; /*!< Encryption algorithm
                                                         used */

  uint64_t                  i_pts_adjustment;       /*!< PTS offset */
  uint8_t                   cw_index;               /*!< CA control word */

  /* splice command */
  uint16_t                  i_splice_command_length;/*!< Length of splice command */
  uint8_t                   i_splice_command_type;  /*!< Splice command type */

  /* Splice Command:
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
  void                      *p_splice_command;      /*!< Pointer to splice command
                                                         structure */

  /* descriptors */
  uint16_t                  i_descriptors_length;   /*!< Descriptors loop
                                                         length */
  dvbpsi_descriptor_t       *p_first_descriptor;     /*!< First of the following
                                                          SIS descriptors */

  /* FIXME: alignment stuffing */
  uint32_t i_ecrc; /*!< CRC 32 of decrypted splice_info_section */

} __attribute__((packed)) dvbpsi_sis_t;

/*****************************************************************************
 * Splice Commands
 *****************************************************************************/
/*!
 * \brief The Splice Info Section (SIS) defines some Splice Commands, which
 * are described below:
 */
/*!
 * \typedef struct dvbpsi_sis_cmd_splice_null_s dvbpsi_sis_cmd_splice_null_t
 * \brief splice_null() splice command definition
 */
/*!
 * \struct dvbpsi_sis_cmd_splice_null_s
 * \brief splice_null() splice command definition
 */
typedef struct dvbpsi_sis_cmd_splice_null_s
{
    /* nothing */
} dvbpsi_sis_cmd_splice_null_t;

/*!
 * \typedef struct dvbpsi_sis_break_duration_s dvbpsi_sis_break_duration_t
 * \brief splice event definition
 */
/*!
 * \struct dvbpsi_sis_break_duration_s
 * \brief splice break duration
 */
typedef struct dvbpsi_sis_break_duration_s
{
    bool        b_auto_return;  /*!< when true it denotes that the duration
                                     shall be used by the splicing device to
                                     know when the return to the network feed
                                     (end of break) is to take place */
    uint64_t    i_duration;     /*!< indicates elapsed time in terms of ticks
                                     of the program’s 90 kHz clock indicates
                                     elapsed time in terms of ticks of the
                                     program’s 90 kHz clock */
} dvbpsi_sis_break_duration_t;

/*!
 * \typedef struct dvbpsi_sis_component_utc_splice_time_s dvbpsi_sis_component_utc_splice_time_t
 * \brief combined component tag and UTC splice time definition
 */
typedef struct dvbpsi_sis_component_utc_splice_time_s dvbpsi_sis_component_utc_splice_time_t;
/*!
 * \struct dvbpsi_sis_component_utc_splice_time_s
 * \brief combined component tag and UTC splice time definition
 */
struct dvbpsi_sis_component_utc_splice_time_s
{
    uint8_t     component_tag;      /*!< identifies the elementary PID stream containing
                                         the Splice Point specified by the value of
                                         splice_time() that follows. */
    uint32_t    i_utc_splice_time;  /*!< time of the signaled splice event as
                                         the number of seconds since 00 hours UTC,
                                         January 6th, 1980.
                                         Maybe converted to UTC without use of
                                         GPS_UTC_offset value from System Time table. */

    dvbpsi_sis_component_utc_splice_time_t *p_next; /*!< next component, utc splice time structure */
};

/*!
 * \typedef struct dvbpsi_sis_splice_event_s dvbpsi_sis_splice_event_t
 * \brief splice event definition
 */
/*!
 * \struct dvbpsi_sis_splice_event_s
 * \brief splice events structure, @see dvbpsi_sis_splice_event_t
 */
typedef struct dvbpsi_sis_splice_event_s dvbpsi_sis_splice_event_t;
struct dvbpsi_sis_splice_event_s
{
    uint32_t        i_splice_event_id;               /*!< splice event identifier */
    bool            b_splice_event_cancel_indicator; /*!< cancels splice event when true */

    /* if (!b_splice_event_cancel_indicator) */
    bool            b_out_of_network_indicator; /*!< signals an out of network feed event */
    bool            b_program_splice_flag;      /*!< signals a Program Splice Point */
    bool            b_duration_flag;            /*!< signals existing break_duration() field */
    /*      if (b_program_splice_flag) */
    uint32_t        i_utc_splice_time;          /*!< time of the signaled splice event as
                                                     the number of seconds since 00 hours UTC,
                                                     January 6th, 1980.
                                                     Maybe converted to UTC without use of
                                                     GPS_UTC_offset value from System Time table.*/
    /*      if (!b_program_splice_flag) */
    uint8_t         i_component_count;          /*!< number of stream PID in the following
                                                     loop. A component is equivalent to
                                                     elementary stream PIDs.*/
    dvbpsi_sis_component_utc_splice_time_t  *p_data;
                                                /*!< identifies the elementary PID stream containing
                                                     the Splice Point specified by the value of
                                                     splice_time() that follows. */
    /*      if (b_duration_flag) */
    dvbpsi_sis_break_duration_t *p_break_duration;     /*!< break duration is present when
                                                     b_duration_flag is set */
    /* */

    uint16_t        i_unique_program_id; /*!< provide a unique identification for
                                              a viewing event */
    uint8_t         i_avail_num;         /*!< identification for a specific
                                              avail within one unique_program_id. */
    uint8_t         i_avails_expected;   /*!< count of the expected number of individual
                                              avails within the current viewing event */
    /* end */

    dvbpsi_sis_splice_event_t *p_next;   /*!< next splice event structure */
};

/*!
 * \typedef struct dvbpsi_sis_cmd_splice_schedule_s dvbpsi_sis_cmd_splice_schedule_t
 * \brief splice_schedule() splice command definition
 */
/*!
 * \struct dvbpsi_sis_cmd_splice_schedule_s
 * \brief splice_schedule() splice command definition
 */
typedef struct dvbpsi_sis_cmd_splice_schedule_s
{
    uint8_t                     i_splice_count; /*!< Count of splice events */
    dvbpsi_sis_splice_event_t  *p_splice_event; /*!< List splice of events */
} dvbpsi_sis_cmd_splice_schedule_t;

/*!
 * \typedef struct dvbpsi_sis_splice_time_s dvbpsi_sis_splice_time_t
 * \brief splice_time() splice definition
 */
/*!
 * \struct dvbpsi_sis_splice_time_s
 * \brief splice_time() splice definition
 */
typedef struct dvbpsi_sis_splice_time_s dvbpsi_sis_splice_time_t;
struct dvbpsi_sis_splice_time_s
{
    bool        b_time_specified_flag; /*!< signals presence of PTS time field */
    /* if (b_time_specified_flag) */
    uint64_t    i_pts_time;        /*!< time in terms of ticks of the program’s 90 kHz
                                        clock. This field, when modified by pts_adjustment,
                                        represents the time of the intended splice point.*/
    /* else reserved */
    /* end */

    dvbpsi_sis_splice_time_t *p_next; /*!< next splice_time() entry */
};

/*!
 * \typedef struct dvbpsi_sis_component_splice_time_s dvbpsi_sis_component_splice_time_t
 * \brief component_tag, splice_time definition
 */
/*!
 * \struct dvbpsi_sis_component_splice_time_s
 * \brief component_tag, splice_time definition
 */
typedef struct dvbpsi_sis_component_splice_time_s dvbpsi_sis_component_splice_time_t;
struct dvbpsi_sis_component_splice_time_s
{
    uint8_t     i_component_tag;    /*!< identifies the elementary PID stream containing
                                         the Splice Point specified by the value of
                                         splice_time() that follows. */
    /* if (splice_immediate_flag) */
    dvbpsi_sis_splice_time_t *p_splice_time; /*!< splice time defintions */
    /* */

    dvbpsi_sis_component_splice_time_t *p_next; /*!< next in list */
};

/*!
 * \typedef struct dvbpsi_sis_cmd_splice_insert_s dvbpsi_sis_cmd_splice_insert_t
 * \brief splice_insert() splice command definition
 */
/*!
 * \struct dvbpsi_sis_cmd_splice_insert_s
 * \brief splice_insert() splice command definition
 */
typedef struct dvbpsi_sis_cmd_splice_insert_s
{
    uint32_t        i_splice_event_id;               /*!< splice event identifier */
    bool            b_splice_event_cancel_indicator; /*!< cancels splice event when true */

    /* if (!b_splice_event_cancel_indicator) */
    bool            b_out_of_network_indicator; /*!< signals an out of network feed event */
    bool            b_program_splice_flag;      /*!< signals a Program Splice Point */
    bool            b_duration_flag;            /*!< signals existing break_duration() field */
    bool            b_splice_immediate_flag;    /*!< signals immediate splice insertion */

    /*      if (b_program_splice_flag) && (!b_splice_immediate_flag) */
    dvbpsi_sis_splice_time_t *p_splice_time;    /*!< splice time */

    /*      if (!b_program_splice_flag) */
    uint8_t         i_component_count;           /*!< number of stream PID in the following loop.
                                                      A component is equivalent to elementary stream PIDs.*/
    dvbpsi_sis_component_splice_time_t  *p_data; /*!< identifies the elementary PID stream containing
                                                      the Splice Point specified by the value of
                                                      splice_time() that follows. */
    /*      if (b_duration_flag) */
    dvbpsi_sis_break_duration_t *p_break_duration; /*!< break duration is present when b_duration_flag is set */

    /* */
    uint16_t        i_unique_program_id;      /*!< provide a unique identification for a viewing event */
    uint8_t         i_avail_num;              /*!< identification for a specific avail within
                                                   one unique_program_id. */
    uint8_t         i_avails_expected;        /*!< count of the expected number of individual avails
                                                   within the current viewing event */
    /* end */
} dvbpsi_sis_cmd_splice_insert_t;

/*!
 * \typedef struct dvbpsi_sis_cmd_time_signal_s dvbpsi_sis_cmd_time_signal_t
 * \brief time_signal() splice command definition
 */
/*!
 * \struct dvbpsi_sis_cmd_time_signal_s
 * \brief time_signal() splice command definition
 */
typedef struct dvbpsi_sis_cmd_time_signal_s
{
    dvbpsi_sis_splice_time_t *p_splice_time;       /*!< splice time command */
} dvbpsi_sis_cmd_time_signal_t;

/*!
 * \typedef struct dvbpsi_sis_cmd_bandwidth_reservation_s dvbpsi_sis_cmd_bandwidth_reservation_t
 * \brief bandwidth_reservation() splice command definition
 */
/*!
 * \struct dvbpsi_sis_cmd_bandwidth_reservation_s
 * \brief bandwidth_reservation() splice command definition
 */
typedef struct dvbpsi_sis_cmd_bandwidth_reservation_s
{
    /* nothing */
} dvbpsi_sis_cmd_bandwidth_reservation_t;

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
 * dvbpsi_sis_attach
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_sis_attach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
          uint16_t i_extension, dvbpsi_sis_callback pf_callback,
                               void* p_cb_data)
 * \brief Creation and initialization of a SIS decoder. It is attached to p_dvbpsi.
 * \param p_dvbpsi pointer to dvbpsi to hold decoder/demuxer structure
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension.
 * \param pf_callback function to call back on new SIS.
 * \param p_cb_data private data given in argument to the callback.
 * \return true on success, false on failure
 */
bool dvbpsi_sis_attach(dvbpsi_t* p_dvbpsi, uint8_t i_table_id, uint16_t i_extension,
                      dvbpsi_sis_callback pf_callback, void* p_cb_data);

/*****************************************************************************
 * dvbpsi_sis_detach
 *****************************************************************************/
/*!
 * \fn void dvbpsi_sis_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id,
                             uint16_t i_extension)
 * \brief Destroy a SIS decoder.
 * \param p_dvbpsi pointer to dvbpsi to hold decoder/demuxer structure
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension, here TS ID.
 * \return nothing.
 */
void dvbpsi_sis_detach(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension);

/*****************************************************************************
 * dvbpsi_sis_init/dvbpsi_sis_new
 *****************************************************************************/
/*!
 * \fn void dvbpsi_sis_init(dvbpsi_sis_t *p_sis, uint8_t i_table_id, uint16_t i_extension,
                    uint8_t i_version, bool b_current_next, uint8_t i_protocol_version);
 * \brief Initialize a user-allocated dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension.
 * \param i_version SIS version
 * \param b_current_next current next indicator
 * \param i_protocol_version SIS protocol version (currently 0)
 * \return nothing.
 */
void dvbpsi_sis_init(dvbpsi_sis_t *p_sis, uint8_t i_table_id, uint16_t i_extension,
                     uint8_t i_version, bool b_current_next, uint8_t i_protocol_version);

/*!
 * \fn dvbpsi_sis_t* dvbpsi_sis_new(uint8_t i_table_id, uint16_t i_extension,
            uint8_t i_version, bool b_current_next, uint8_t i_protocol_version);
 * \brief Allocate and initialize a new dvbpsi_sis_t structure.
 * \param i_table_id Table ID, 0xFC.
 * \param i_extension Table ID extension.
 * \param i_version SIS version
 * \param b_current_next current next indicator
 * \param i_protocol_version SIS protocol version (currently 0)
 * \return p_sis pointer to the SIS structure
 */
dvbpsi_sis_t* dvbpsi_sis_new(uint8_t i_table_id, uint16_t i_extension, uint8_t i_version,
                             bool b_current_next, uint8_t i_protocol_version);

/*****************************************************************************
 * dvbpsi_sis_empty/dvbpsi_sis_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_sis_empty(dvbpsi_sis_t* p_sis)
 * \brief Clean a dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \return nothing.
 */
void dvbpsi_sis_empty(dvbpsi_sis_t *p_sis);

/*!
 * \fn void dvbpsi_sis_delete(dvbpsi_sis_t *p_sis)
 * \brief Clean and free a dvbpsi_sis_t structure.
 * \param p_sis pointer to the SIS structure
 * \return nothing.
 */
void dvbpsi_sis_delete(dvbpsi_sis_t *p_sis);

/*****************************************************************************
 * dvbpsi_sis_descriptor_add
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t *dvbpsi_sis_descriptor_add(dvbpsi_sis_t *p_sis,
                                               uint8_t i_tag, uint8_t i_length,
                                               uint8_t *p_data)
 * \brief Add a descriptor in the SIS service.
 * \param p_sis pointer to the SIS structure
 * \param i_tag descriptor's tag
 * \param i_length descriptor's length
 * \param p_data descriptor's data
 * \return a pointer to the added descriptor.
 */
dvbpsi_descriptor_t *dvbpsi_sis_descriptor_add(dvbpsi_sis_t *p_sis,
                                             uint8_t i_tag, uint8_t i_length,
                                             uint8_t *p_data);

/*****************************************************************************
 * dvbpsi_sis_sections_generate
 *****************************************************************************
 * Generate SIS sections based on the dvbpsi_sis_t structure.
 *****************************************************************************/
/*!
 * \fn dvbpsi_psi_section_t *dvbpsi_sis_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_sis_t * p_sis);
 * \brief SIS generator
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_sis SIS structure
 * \return a pointer to the list of generated PSI sections.
 *
 * Generate SIS sections based on the dvbpsi_sis_t structure.
 */
dvbpsi_psi_section_t *dvbpsi_sis_sections_generate(dvbpsi_t *p_dvbpsi, dvbpsi_sis_t * p_sis);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of sis.h"
#endif

