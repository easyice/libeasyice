/*****************************************************************************
 * dvbpsi.h
 * Copyright (C) 2001-2012 VideoLAN
 * $Id$
 *
 * Authors: Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
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
 * \file <dvbpsi.h>
 * \author Arnaud de Bossoreille de Ribou <bozo@via.ecp.fr>
 *         Jean-Paul Saman <jpsaman@videolan.org>
 * \brief Application interface for all DVB/PSI decoders.
 *
 * Application interface for all DVB/PSI decoders. The generic decoder
 * structure is public so that external decoders are allowed.
 */

#ifndef _DVBPSI_DVBPSI_H_
#define _DVBPSI_DVBPSI_H_

#define DVBPSI_VERSION      1.3.0              /*!< Human readible DVBPSI version*/
#define DVBPSI_VERSION_INT  ((1<<16)+(3<<8)+0) /*!< Machine readible DVBPSI version */

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * Helper for GCC version checks borrowed from VLC.
 *****************************************************************************/
/*!
 * \def DVBPSI_GCC_VERSION(maj,min)
 * \brief Helper macro for GCC version check
 */
#ifdef __GNUC__
# define DVBPSI_GCC_VERSION(maj,min) \
    ((__GNUC__ > (maj)) || (__GNUC__ == (maj) && __GNUC_MINOR__ >= (min)))
#else
# define DVBPSI_GCC_VERSION(maj,min) (0)
#endif

/*****************************************************************************
 * dvbpsi_t
 *****************************************************************************/
/*!
 * \typedef struct dvbpsi_s dvbpsi_t
 * \brief DVBPSI handle structure abstration.
 */
typedef struct dvbpsi_s dvbpsi_t;

/*!
 * \enum dvbpsi_msg_level
 * \brief DVBPSI message level enumeration type
 */
enum dvbpsi_msg_level
{
    DVBPSI_MSG_NONE  = -1, /*!< No messages */
    DVBPSI_MSG_ERROR =  0, /*!< Error messages only */
    DVBPSI_MSG_WARN  =  1, /*!< Error and Warning messages */
    DVBPSI_MSG_DEBUG =  2, /*!< Error, warning and debug messages */
};
/*!
 * \typedef enum dvbpsi_msg_level dvbpsi_msg_level_t
 * \brief DVBPSI message level enumeration type definition
 */
typedef enum dvbpsi_msg_level dvbpsi_msg_level_t;

/*****************************************************************************
 * dvbpsi_message_cb
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_message_cb)(dvbpsi_t *p_decoder,
 *                                     const dvbpse_msg_level_t level,
 *                                     const char* msg)
 * \brief Callback type definition.
 */
typedef void (* dvbpsi_message_cb)(dvbpsi_t *handle,
                                   const dvbpsi_msg_level_t level,
                                   const char* msg);

/*****************************************************************************
 * dvbpsi_decoder_t
 *****************************************************************************/
/*!
 * \typedef struct dvbpsi_decoder_s dvbpsi_decoder_t
 * \brief dvbpsi_decoder_t type definition.
 */
typedef struct dvbpsi_decoder_s dvbpsi_decoder_t;

/*!
 * \def DVBPSI_DECODER(x)
 * \brief Helper macro for casting a private decoder into a dvbpsi_decoder_t
 */
/* Clever cast borrowed from VLC */
#if DVBPSI_GCC_VERSION(4,0)
# ifndef __cplusplus
#  define DVBPSI_DECODER( x ) \
    __builtin_choose_expr( \
        __builtin_offsetof(__typeof__(*(x)), i_magic), \
        (void)0, \
        (dvbpsi_decoder_t *)(x))
# else
#  define DVBPSI_DECODER( x ) \
    ((dvbpsi_decoder_t *)(x) \
      + 0 * __builtin_offsetof(__typeof__(*(x)), i_magic))
# endif
#else
# define DVBPSI_DECODER(x) ((dvbpsi_decoder_t *)(x))
#endif

/*****************************************************************************
 * dvbpsi_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_s
 * \brief DVBPSI handle structure
 *
 * This structure provides a handle to libdvbpsi API and should be used instead
 * of dvbpsi_decoder_t.
 */
/*!
 * \typedef struct dvbpsi_s dvbpsi_t
 * \brief dvbpsi_t type definition.
 */
struct dvbpsi_s
{
    dvbpsi_decoder_t             *p_decoder;          /*!< private pointer to
                                                          specific decoder */
    /* Messages callback */
    dvbpsi_message_cb             pf_message;           /*!< Log message callback */
    enum dvbpsi_msg_level         i_msg_level;          /*!< Log level */

    /* private data pointer for use by caller, not by libdvbpsi itself ! */
    void                         *p_sys;                /*!< pointer to private data
                                                          from caller. Do not use
                                                          from inside libdvbpsi. It
                                                          will crash any application. */
};

/*****************************************************************************
 * dvbpsi_new
 *****************************************************************************/
/*!
 * \fn dvbpsi_t *dvbpsi_new(dvbpsi_message_cb callback, enum dvbpsi_msg_level level)
 * \brief Create a new dvbpsi_t handle to be used by PSI decoders or encoders
 * \param callback message callback handler, if NULL then no errors, warnings
 *        or debug messages will be sent to the caller application
 * \param level enum dvbpsi_msg_level for filtering logging messages
 * \return pointer to dvbpsi_t malloced data
 *
 * Creates a handle to use with PSI decoder and encoder API functions. The
 * handle must be freed with dvbpsi_delete().
 */
dvbpsi_t *dvbpsi_new(dvbpsi_message_cb callback, enum dvbpsi_msg_level level);

/*****************************************************************************
 * dvbpsi_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_delete(dvbpsi_t *p_dvbpsi)
 * \brief Deletes a dvbpsi_t handle created with dvbpsi_new
 * \param p_dvbpsi pointer to dvbpsi_t malloced data
 * \return nothing
 *
 * Delets a dvbpsi_t handle by calling free(handle). Make sure to detach any
 * decoder of encoder before deleting the dvbpsi handle.
 */
void dvbpsi_delete(dvbpsi_t *p_dvbpsi);

/*****************************************************************************
 * dvbpsi_packet_push
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_packet_push(dvbpsi_t *p_dvbpsi, uint8_t* p_data)
 * \brief Injection of a TS packet into a PSI decoder.
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \param p_data pointer to a 188 bytes playload of a TS packet
 * \return true when packet has been handled, false on error.
 *
 * Injection of a TS packet into a PSI decoder.
 */
bool dvbpsi_packet_push(dvbpsi_t *p_dvbpsi, uint8_t* p_data);

/*****************************************************************************
 * dvbpsi_psi_section_t
 *****************************************************************************/

/*!
 * \typedef struct dvbpsi_psi_section_s dvbpsi_psi_section_t
 * \brief dvbpsi_psi_section_t type definition.
 */
typedef struct dvbpsi_psi_section_s dvbpsi_psi_section_t;

/*****************************************************************************
 * dvbpsi_callback_gather_t
 *****************************************************************************/
/*!
 * \typedef void (* dvbpsi_callback_gather_t)(dvbpsi_t *p_dvbpsi,
                                              dvbpsi_psi_section_t* p_section)
 * \brief Callback used for gathering psi sections on behalf of PSI decoders.
 */
typedef void (* dvbpsi_callback_gather_t)(dvbpsi_t *p_dvbpsi,  /*!< pointer to dvbpsi handle */
                            dvbpsi_psi_section_t* p_section);  /*!< pointer to psi section */

/*****************************************************************************
 * DVBPSI_DECODER_COMMON
 *****************************************************************************/
/*!
 * \def DVBPSI_DECODER_COMMON
 * \brief Common members for all dvbpsi table decoders. These should be the first
 * members of a table decoder struct, such that they can be casted to a
 * dvbpsi_decoder_t type.
 */
#define DVBPSI_DECODER_COMMON                                                     \
/*!                                                                               \
 * \name DVBPSI_DECODER_COMMON                                                    \
 * these members are common for all dvbpsi_decoder_t                              \
 */                                                                               \
/**@{*/                                                                           \
    uint8_t  i_magic[3];           /*!< Reserved magic value */                   \
    bool     b_complete_header;    /*!< Flag for header completion */             \
    bool     b_discontinuity;      /*!< Discontinuity flag */                     \
    bool     b_current_valid;      /*!< Current valid indicator */                \
    uint8_t  i_continuity_counter; /*!< Continuity counter */                     \
    uint8_t  i_last_section_number;/*!< Last received section number */           \
    dvbpsi_psi_section_t *p_current_section; /*!< Current section */              \
    dvbpsi_psi_section_t *p_sections; /*!< List of received PSI sections */       \
    dvbpsi_callback_gather_t  pf_gather;/*!< PSI decoder's callback */            \
    int      i_section_max_size;   /*!< Max size of a section for this decoder */ \
    int      i_need;               /*!< Bytes needed */                           \
/**@}*/

/*****************************************************************************
 * struct dvbpsi_decoder_s
 *****************************************************************************/
/*!
 * \struct dvbpsi_decoder_s
 * \brief PSI decoder structure.
 *
 * This structure shouldn't be used but if you want to write an external
 * decoder.
 */
struct dvbpsi_decoder_s
{
    DVBPSI_DECODER_COMMON
};

/*****************************************************************************
 * dvbpsi_decoder_new
 *****************************************************************************/
/*!
 * \fn void *dvbpsi_decoder_new(dvbpsi_callback_gather_t pf_gather,
 *     const int i_section_max_size, const bool b_discontinuity, const size_t psi_size);
 * \brief Create a new (private) dvbpsi decoder.
 * \param pf_gather pointer to gather function for PSI decoder.
 * \param i_section_max_size Max size of a section for this decoder
 * \param b_discontinuity Discontinuity flag
 * \param psi_size size of new PSI struct, eg: sizeof(dvbpsi_pat_t)
 * \return pointer to memory of size 'psi_size' that can be casted into a dvbpsi_decoder_t.
 *
 * Creates a void pointer that points to a private dvbpsi decoder struct (eg: dvbpsi_pat_t).
 * The first elements in this newly created decoder should contain DVBPSI_DECODER_COMMON, which
 * corresponds to a dvbpsi_decoder_t.
 * Upon return the pointer should be casted to the correct type. Delete the pointer
 * with @see dvbpsi_decoder_delete() function.
 */
void *dvbpsi_decoder_new(dvbpsi_callback_gather_t pf_gather,
                         const int i_section_max_size,
                         const bool b_discontinuity,
                         const size_t psi_size);

/*****************************************************************************
 * dvbpsi_decoder_delete
 *****************************************************************************/
/*!
 * \fn void dvbpsi_decoder_delete(dvbpsi_decoder_t *p_decoder);
 * \brief Deletes decoder struct and frees its memory
 * \param p_decoder pointer to dvbpsi_decoder_t with decoder
 * \return nothing
 *
 * Delets a dvbpsi_t handle by calling free(handle). Make sure to detach any
 * decoder of encoder before deleting the dvbpsi handle.
 */
void dvbpsi_decoder_delete(dvbpsi_decoder_t *p_decoder);

/*****************************************************************************
 * dvbpsi_decoder_reset
 *****************************************************************************/
/*!
 * \fn void dvbpsi_decoder_reset(dvbpsi_decoder_t* p_decoder, const bool b_force);
 * \brief Resets a decoder internal state.
 * \param p_decoder pointer to dvbpsi_decoder_t with decoder
 * \param b_force  If 'b_force' is true then dvbpsi_decoder_t::b_current_valid
 * is set to false, invalidating the current section.
 * \return nothing
 */
void dvbpsi_decoder_reset(dvbpsi_decoder_t* p_decoder, const bool b_force);

/*****************************************************************************
 * dvbpsi_decoder_psi_sections_completed
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_decoder_psi_sections_completed(dvbpsi_decoder_t* p_decoder);
 * \brief Have all sections for this decoder been received?
 * \param p_decoder pointer to dvbpsi_decoder_t with decoder
 * \return true when all PSI sections have been received, false otherwise
 */
bool dvbpsi_decoder_psi_sections_completed(dvbpsi_decoder_t* p_decoder);

/*****************************************************************************
 * dvbpsi_decoder_psi_section_add
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_decoder_psi_section_add(dvbpsi_decoder_t *p_decoder, dvbpsi_psi_section_t *p_section);
 * \brief Add a section to the dvbpsi_decoder_t::p_sections list.
 * \param p_decoder pointer to dvbpsi_decoder_t with decoder
 * \param p_section PSI section to add to dvbpsi_decoder_t::p_sections list
 * \return true if it overwrites a earlier section, false otherwise
 */
bool dvbpsi_decoder_psi_section_add(dvbpsi_decoder_t *p_decoder, dvbpsi_psi_section_t *p_section);

/*****************************************************************************
 * dvbpsi_decoder_present
 *****************************************************************************/
/*!
 * \fn bool dvbpsi_decoder_present(dvbpsi_t *p_dvbpsi);
 * \brief Determines if a decoder has been attached to dvbpsi_t handle
 * \param p_dvbpsi handle to dvbpsi with attached decoder
 * \return true when decoder is attached, else it will return false.
 *
 * Determines if a decoder is attached to this dvbpsi_t handle. When the dvbpsi
 * handle is invalid the fuction will return false.
 */
bool dvbpsi_decoder_present(dvbpsi_t *p_dvbpsi);

#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dvbpsi.h"
#endif
