/*****************************************************************************
 * dr_54.h
 * Copyright (C) 2013 VideoLAN
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
 * \file <dr_54.h>
 * \author François Cartegnie <fcvlcdev@free.fr>
 * \brief Content descriptor parsing.
 *
 * Content descriptor parsing, according to ETSI EN 300 468
 * section 6.2.9.
 */

#ifndef _DVBPSI_DR_54_H_
#define _DVBPSI_DR_54_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \def DVBPSI_GetContentCategoryFromType
 * \brief Extract content category by type. The value 'type' includes the
 * content category (MSB) and content detail (LSB).
 */
#define DVBPSI_GetContentCategoryFromType(type) ((type) >> 4)

/*!
 * \def L1L2MERGE
 * \brief Merge content category and content detail in one byte. The
 * category is in the topmost 4-bits (MSB) and the content detail in
 * the lower 4-bits (LSB).
 */
#define L1L2MERGE(L1,L2) ( ( DVBPSI_CONTENT_CAT ## L1<<4) | (L2) )

/* Content category */
#define DVBPSI_CONTENT_CAT_UNDEFINED    0x0 /*!< Undefined content */
#define DVBPSI_CONTENT_CAT_MOVIE        0x1 /*!< Movie content */
#define DVBPSI_CONTENT_CAT_NEWS         0x2 /*!< News content */
#define DVBPSI_CONTENT_CAT_SHOW         0x3 /*!< TV Show content */
#define DVBPSI_CONTENT_CAT_SPORTS       0x4 /*!< Sports content */
#define DVBPSI_CONTENT_CAT_CHILDREN     0x5 /*!< Children content */
#define DVBPSI_CONTENT_CAT_MUSIC        0x6 /*!< Music content */
#define DVBPSI_CONTENT_CAT_CULTURE      0x7 /*!< Culture content */
#define DVBPSI_CONTENT_CAT_SOCIAL       0x8 /*!< Social content */
#define DVBPSI_CONTENT_CAT_EDUCATION    0x9 /*!< Educational content */
#define DVBPSI_CONTENT_CAT_LEISURE      0xa /*!< Leisure content */
#define DVBPSI_CONTENT_CAT_SPECIAL      0xb /*!< Special content */
#define DVBPSI_CONTENT_CAT_USERDEFINED  0xf /*!< User defined content */

/* Movie/Drama */
#define DVBPSI_CONTENT_MOVIE_GENERAL        L1L2MERGE( _MOVIE, 0x0 ) /*!< General Movie */
#define DVBPSI_CONTENT_MOVIE_DETECTIVE      L1L2MERGE( _MOVIE, 0x1 ) /*!< Detective Movie */
#define DVBPSI_CONTENT_MOVIE_ADVENTURE      L1L2MERGE( _MOVIE, 0x2 ) /*!< Adventure Movie */
#define DVBPSI_CONTENT_MOVIE_SF             L1L2MERGE( _MOVIE, 0x3 ) /*!< SciFi Movie */
#define DVBPSI_CONTENT_MOVIE_COMEDY         L1L2MERGE( _MOVIE, 0x4 ) /*!< Comedy Movie */
#define DVBPSI_CONTENT_MOVIE_SOAP           L1L2MERGE( _MOVIE, 0x5 ) /*!< Soap Movie */
#define DVBPSI_CONTENT_MOVIE_ROMANCE        L1L2MERGE( _MOVIE, 0x6 ) /*!< Romance Movie */
#define DVBPSI_CONTENT_MOVIE_CLASSICAL      L1L2MERGE( _MOVIE, 0x7 ) /*!< Classical Movie */
#define DVBPSI_CONTENT_MOVIE_ADULT          L1L2MERGE( _MOVIE, 0x8 ) /*!< Adult Movie */
#define DVBPSI_CONTENT_MOVIE_USERDEFINED    L1L2MERGE( _MOVIE, 0xf ) /*!< User defined Movie */
/* News/Current affairs */
#define DVBPSI_CONTENT_NEWS_GENERAL         L1L2MERGE( _NEWS, 0x0 ) /*!< General News */
#define DVBPSI_CONTENT_NEWS_WEATHER         L1L2MERGE( _NEWS, 0x1 ) /*!< Weather News */
#define DVBPSI_CONTENT_NEWS_MAGAZINE        L1L2MERGE( _NEWS, 0x2 ) /*!< Magazine News */
#define DVBPSI_CONTENT_NEWS_DOCUMENTARY     L1L2MERGE( _NEWS, 0x3 ) /*!< Documentary News */
#define DVBPSI_CONTENT_NEWS_DISCUSSION      L1L2MERGE( _NEWS, 0x4 ) /*!< Discussion News */
#define DVBPSI_CONTENT_NEWS_USERDEFINED     L1L2MERGE( _NEWS, 0xf ) /*!< User defined News */
/* Show/Game show */
#define DVBPSI_CONTENT_SHOW_GENERAL         L1L2MERGE( _SHOW, 0x0 ) /*!< General Show */
#define DVBPSI_CONTENT_SHOW_QUIZ            L1L2MERGE( _SHOW, 0x1 ) /*!< Quiz Show */
#define DVBPSI_CONTENT_SHOW_VARIETY         L1L2MERGE( _SHOW, 0x2 ) /*!< Variety Show */
#define DVBPSI_CONTENT_SHOW_TALK            L1L2MERGE( _SHOW, 0x3 ) /*!< Talk Show */
#define DVBPSI_CONTENT_SHOW_USERDEFINED     L1L2MERGE( _SHOW, 0xf ) /*!< User defined Show */
/* Sports */
#define DVBPSI_CONTENT_SPORTS_GENERAL       L1L2MERGE( _SPORTS, 0x0 ) /*!< General Sports */
#define DVBPSI_CONTENT_SPORTS_EVENTS        L1L2MERGE( _SPORTS, 0x1 ) /*!< Sports Events */
#define DVBPSI_CONTENT_SPORTS_MAGAZINE      L1L2MERGE( _SPORTS, 0x2 ) /*!< Sports Magazine */
#define DVBPSI_CONTENT_SPORTS_FOOTBALL      L1L2MERGE( _SPORTS, 0x3 ) /*!< Football Sports */
#define DVBPSI_CONTENT_SPORTS_TENNIS        L1L2MERGE( _SPORTS, 0x4 ) /*!< Tennis Sports */
#define DVBPSI_CONTENT_SPORTS_TEAM          L1L2MERGE( _SPORTS, 0x5 ) /*!< Team Sports */
#define DVBPSI_CONTENT_SPORTS_ATHLETICS     L1L2MERGE( _SPORTS, 0x6 ) /*!< Athletics Sports */
#define DVBPSI_CONTENT_SPORTS_MOTOR         L1L2MERGE( _SPORTS, 0x7 ) /*!< Motor Sports */
#define DVBPSI_CONTENT_SPORTS_WATER         L1L2MERGE( _SPORTS, 0x8 ) /*!< Water Sports */
#define DVBPSI_CONTENT_SPORTS_WINTER        L1L2MERGE( _SPORTS, 0x9 ) /*!< Winter Sports */
#define DVBPSI_CONTENT_SPORTS_EQUESTRIAN    L1L2MERGE( _SPORTS, 0xa ) /*!< Equestrian Sports */
#define DVBPSI_CONTENT_SPORTS_MARTIAL       L1L2MERGE( _SPORTS, 0xb ) /*!< Marital Sports */
#define DVBPSI_CONTENT_SPORTS_USERDEFINED   L1L2MERGE( _SPORTS, 0xf ) /*!< User defined Sports */
/* Children's/Youth */
#define DVBPSI_CONTENT_CHILDREN_GENERAL     L1L2MERGE( _CHILDREN, 0x0 ) /*!< General Children */
#define DVBPSI_CONTENT_CHILDREN_PRESCHOOL   L1L2MERGE( _CHILDREN, 0x1 ) /*!< Preschool Children */
#define DVBPSI_CONTENT_CHILDREN_06TO14ENT   L1L2MERGE( _CHILDREN, 0x2 ) /*!< 06 to 14 years old Children */
#define DVBPSI_CONTENT_CHILDREN_10TO16ENT   L1L2MERGE( _CHILDREN, 0x3 ) /*!< 10 to 16 years old Children */
#define DVBPSI_CONTENT_CHILDREN_EDUCATIONAL L1L2MERGE( _CHILDREN, 0x4 ) /*!< Educational Children */
#define DVBPSI_CONTENT_CHILDREN_CARTOONS    L1L2MERGE( _CHILDREN, 0x5 ) /*!< Cartoons Children */
#define DVBPSI_CONTENT_CHILDREN_USERDEFINED L1L2MERGE( _CHILDREN, 0xf ) /*!< User defined for Children */
/* Music/Ballet/Dance */
#define DVBPSI_CONTENT_MUSIC_GENERAL        L1L2MERGE( _MUSIC, 0x0 ) /*!< General Music */
#define DVBPSI_CONTENT_MUSIC_POPROCK        L1L2MERGE( _MUSIC, 0x1 ) /*!< Poprock Music */
#define DVBPSI_CONTENT_MUSIC_CLASSICAL      L1L2MERGE( _MUSIC, 0x2 ) /*!< Classical Music */
#define DVBPSI_CONTENT_MUSIC_FOLK           L1L2MERGE( _MUSIC, 0x3 ) /*!< Folk Music */
#define DVBPSI_CONTENT_MUSIC_JAZZ           L1L2MERGE( _MUSIC, 0x4 ) /*!< Jazz Music */
#define DVBPSI_CONTENT_MUSIC_OPERA          L1L2MERGE( _MUSIC, 0x5 ) /*!< Opera Music */
#define DVBPSI_CONTENT_MUSIC_BALLET         L1L2MERGE( _MUSIC, 0x6 ) /*!< Ballet Music */
#define DVBPSI_CONTENT_MUSIC_USERDEFINED    L1L2MERGE( _MUSIC, 0xf ) /*!< User defined Music */
/* Arts/Culture */
#define DVBPSI_CONTENT_CULTURE_GENERAL      L1L2MERGE( _CULTURE, 0x0 ) /*!< General Culture */
#define DVBPSI_CONTENT_CULTURE_PERFORMANCE  L1L2MERGE( _CULTURE, 0x1 ) /*!< Performance Culture */
#define DVBPSI_CONTENT_CULTURE_FINEARTS     L1L2MERGE( _CULTURE, 0x2 ) /*!< Fine Arts Culture */
#define DVBPSI_CONTENT_CULTURE_RELIGION     L1L2MERGE( _CULTURE, 0x3 ) /*!< Religion Culture */
#define DVBPSI_CONTENT_CULTURE_TRADITIONAL  L1L2MERGE( _CULTURE, 0x4 ) /*!< Traditional Culture */
#define DVBPSI_CONTENT_CULTURE_LITERATURE   L1L2MERGE( _CULTURE, 0x5 ) /*!< Literature Culture */
#define DVBPSI_CONTENT_CULTURE_CINEMA       L1L2MERGE( _CULTURE, 0x6 ) /*!< Cinema Culture */
#define DVBPSI_CONTENT_CULTURE_EXPERIMENTAL L1L2MERGE( _CULTURE, 0x7 ) /*!< Experimental Culture */
#define DVBPSI_CONTENT_CULTURE_PRESS        L1L2MERGE( _CULTURE, 0x8 ) /*!< Press Culture */
#define DVBPSI_CONTENT_CULTURE_NEWMEDIA     L1L2MERGE( _CULTURE, 0x9 ) /*!< New Media Culture */
#define DVBPSI_CONTENT_CULTURE_MAGAZINE     L1L2MERGE( _CULTURE, 0xa ) /*!< Multure magzine */
#define DVBPSI_CONTENT_CULTURE_FASHION      L1L2MERGE( _CULTURE, 0xb ) /*!< Fashion Culture */
#define DVBPSI_CONTENT_CULTURE_USERDEFINED  L1L2MERGE( _CULTURE, 0xf ) /*!< User defined Culture */
/* Socal/Political/Economics */
#define DVBPSI_CONTENT_SOCIAL_GENERAL       L1L2MERGE( _SOCIAL, 0x0 ) /*!< General Social */
#define DVBPSI_CONTENT_SOCIAL_MAGAZINE      L1L2MERGE( _SOCIAL, 0x1 ) /*!< Social Magazine*/
#define DVBPSI_CONTENT_SOCIAL_ADVISORY      L1L2MERGE( _SOCIAL, 0x2 ) /*!< Advisory Social */
#define DVBPSI_CONTENT_SOCIAL_PEOPLE        L1L2MERGE( _SOCIAL, 0x3 ) /*!< Social People */
#define DVBPSI_CONTENT_SOCIAL_USERDEFINED   L1L2MERGE( _SOCIAL, 0xf ) /*!< User defined Social */
/* Eduction/Science/Factual */
#define DVBPSI_CONTENT_EDUCATION_GENERAL    L1L2MERGE( _EDUCATION, 0x0 ) /*!< General Education */
#define DVBPSI_CONTENT_EDUCATION_NATURE     L1L2MERGE( _EDUCATION, 0x1 ) /*!< Nature Education */
#define DVBPSI_CONTENT_EDUCATION_TECHNOLOGY L1L2MERGE( _EDUCATION, 0x2 ) /*!< Technology Education */
#define DVBPSI_CONTENT_EDUCATION_MEDICINE   L1L2MERGE( _EDUCATION, 0x3 ) /*!< Medicine Education */
#define DVBPSI_CONTENT_EDUCATION_FOREIGN    L1L2MERGE( _EDUCATION, 0x4 ) /*!< Foreign Education */
#define DVBPSI_CONTENT_EDUCATION_SOCIAL     L1L2MERGE( _EDUCATION, 0x5 ) /*!< Social Education */
#define DVBPSI_CONTENT_EDUCATION_FURTHER    L1L2MERGE( _EDUCATION, 0x6 ) /*!< Futher Education */
#define DVBPSI_CONTENT_EDUCATION_LANGUAGE   L1L2MERGE( _EDUCATION, 0x7 ) /*!< Language Education */
#define DVBPSI_CONTENT_EDUCATION_USERDEFINED L1L2MERGE( _EDUCATION, 0xf ) /*!< User defined Education */
/* Leisure/Hobbies */
#define DVBPSI_CONTENT_LEISURE_GENERAL      L1L2MERGE( _LEISURE, 0x0 ) /*!< General Leisure */
#define DVBPSI_CONTENT_LEISURE_TRAVEL       L1L2MERGE( _LEISURE, 0x1 ) /*!< Travel Leisure */
#define DVBPSI_CONTENT_LEISURE_HANDICRAFT   L1L2MERGE( _LEISURE, 0x2 ) /*!< Handicraft Leisure */
#define DVBPSI_CONTENT_LEISURE_MOTORING     L1L2MERGE( _LEISURE, 0x3 ) /*!< Motoring Leisure */
#define DVBPSI_CONTENT_LEISURE_FITNESS      L1L2MERGE( _LEISURE, 0x4 ) /*!< Fitness Leisure */
#define DVBPSI_CONTENT_LEISURE_COOKING      L1L2MERGE( _LEISURE, 0x5 ) /*!< Cooking Leisure */
#define DVBPSI_CONTENT_LEISURE_SHOPPING     L1L2MERGE( _LEISURE, 0x6 ) /*!< Shopping Leisure */
#define DVBPSI_CONTENT_LEISURE_GARDENING    L1L2MERGE( _LEISURE, 0x7 ) /*!< Gardening Leisure */
#define DVBPSI_CONTENT_LEISURE_USERDEFINED  L1L2MERGE( _LEISURE, 0xf ) /*!< User defined Leisure */
/* Special characteristics */
#define DVBPSI_CONTENT_SPECIAL_ORIGINALLANGUAGE L1L2MERGE( _SPECIAL, 0x0 ) /*!< Original language Special */
#define DVBPSI_CONTENT_SPECIAL_BLACKANDWHITE    L1L2MERGE( _SPECIAL, 0x1 ) /*!< Black and White Special */
#define DVBPSI_CONTENT_SPECIAL_UNPUBLISHED      L1L2MERGE( _SPECIAL, 0x2 ) /*!< Unpublished Special */
#define DVBPSI_CONTENT_SPECIAL_LIVE             L1L2MERGE( _SPECIAL, 0x3 ) /*!< Live Special */
#define DVBPSI_CONTENT_SPECIAL_PLANOSTEREOSCOPIC L1L2MERGE( _SPECIAL, 0x4 ) /*!< Planostereoscopic Special */
#define DVBPSI_CONTENT_SPECIAL_USERDEFINED      L1L2MERGE( _SPECIAL, 0xb ) /*!< User defined Special */
#define DVBPSI_CONTENT_SPECIAL_USERDEFINED1     L1L2MERGE( _SPECIAL, 0xc ) /*!< User defined Special */
#define DVBPSI_CONTENT_SPECIAL_USERDEFINED2     L1L2MERGE( _SPECIAL, 0xd ) /*!< User defined Special */
#define DVBPSI_CONTENT_SPECIAL_USERDEFINED3     L1L2MERGE( _SPECIAL, 0xe ) /*!< User defined Special */
#define DVBPSI_CONTENT_SPECIAL_USERDEFINED4     L1L2MERGE( _SPECIAL, 0xf ) /*!< User defined Special */

/*****************************************************************************
 * dvbpsi_content_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_s
 * \brief  Content nibble structure.
 *
 * This structure is used since content_descriptor will contain several
 * content nibbles pairs.
 */
/*!
 * \typedef struct dvbpsi_content_s dvbpsi_content_t
 * \brief dvbpsi_content_t type definition.
 */
typedef struct dvbpsi_content_s
{
  uint8_t       i_type;            /*!< content nibble level 1+2 (4+4bits)*/
  uint8_t       i_user_byte;       /*!< defined by broadcaster */

} dvbpsi_content_t;

/*!
 * \def DVBPSI_CONTENT_DR_MAX
 * \brief Maximum number of dvbps_content_t entries present in @see dvbpsi_content_dr_t
 */
#define DVBPSI_CONTENT_DR_MAX 64

/*****************************************************************************
 * dvbpsi_content_dr_t
 *****************************************************************************/
/*!
 * \struct dvbpsi_content_dr_s
 * \brief "content" descriptor structure.
 *
 * This structure is used to store a decoded "content"
 * descriptor. (ETSI EN 300 468 section 6.2.9).
 */
/*!
 * \typedef struct dvbpsi_content_dr_s dvbpsi_content_dr_t
 * \brief dvbpsi_content_dr_t type definition.
 */
typedef struct dvbpsi_content_dr_s
{
  uint8_t          i_contents_number;                /*!< number of content */
  dvbpsi_content_t p_content[DVBPSI_CONTENT_DR_MAX]; /*!< parental rating table */

} dvbpsi_content_dr_t;


/*****************************************************************************
 * dvbpsi_DecodeContentDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_content_dr_t * dvbpsi_DecodeContentDr(
                                        dvbpsi_descriptor_t * p_descriptor)
 * \brief "content" descriptor decoder.
 * \param p_descriptor pointer to the descriptor structure
 * \return a pointer to a new "content" descriptor structure
 * which contains the decoded data.
 */
dvbpsi_content_dr_t* dvbpsi_DecodeContentDr(
                                        dvbpsi_descriptor_t * p_descriptor);


/*****************************************************************************
 * dvbpsi_GenContentDataDr
 *****************************************************************************/
/*!
 * \fn dvbpsi_descriptor_t * dvbpsi_GenContentDr(
                        dvbpsi_content_dr_t * p_decoded, bool b_duplicate)
 * \brief "content" descriptor generator.
 * \param p_decoded pointer to a decoded "content" descriptor
 * structure
 * \param b_duplicate if true then duplicate the p_decoded structure into
 * the descriptor
 * \return a pointer to a new descriptor structure which contains encoded data.
 */
dvbpsi_descriptor_t * dvbpsi_GenContentDr(
                                        dvbpsi_content_dr_t * p_decoded,
                                        bool b_duplicate);


#ifdef __cplusplus
};
#endif

#else
#error "Multiple inclusions of dr_54.h"
#endif
