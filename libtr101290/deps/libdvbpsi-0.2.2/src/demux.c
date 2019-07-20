/*****************************************************************************
 * demux.c: DVB subtables demux functions.
 *----------------------------------------------------------------------------
 * Copyright (C) 2001-2010 VideoLAN
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
 *----------------------------------------------------------------------------
 *
 *****************************************************************************/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "dvbpsi.h"
#include "dvbpsi_private.h"
#include "psi.h"
#include "demux.h"

/*****************************************************************************
 * dvbpsi_AttachDemux
 *****************************************************************************
 * Creation of the demux structure
 *****************************************************************************/
dvbpsi_handle dvbpsi_AttachDemux(dvbpsi_demux_new_cb_t pf_new_cb,
                                   void *                p_new_cb_data)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  dvbpsi_demux_t * p_demux;

  if(h_dvbpsi == NULL)
    return NULL;

  p_demux = (dvbpsi_demux_t*)malloc(sizeof(dvbpsi_demux_t));

  if(p_demux == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }

  /* PSI decoder configuration */
  h_dvbpsi->pf_callback = &dvbpsi_Demux;
  h_dvbpsi->p_private_decoder = p_demux;
  h_dvbpsi->i_section_max_size = 4096;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* Sutables demux configuration */
  p_demux->p_decoder = h_dvbpsi;
  p_demux->p_first_subdec = NULL;
  p_demux->pf_new_callback = pf_new_cb;
  p_demux->p_new_cb_data = p_new_cb_data;

  return h_dvbpsi;
}

/*****************************************************************************
 * dvbpsi_demuxGetSubDec
 *****************************************************************************
 * Finds a subtable decoder given the table id and extension
 *****************************************************************************/
dvbpsi_demux_subdec_t * dvbpsi_demuxGetSubDec(dvbpsi_demux_t * p_demux,
                                              uint8_t i_table_id,
                                              uint16_t i_extension)
{
  uint32_t i_id = (uint32_t)i_table_id << 16 |(uint32_t)i_extension;
  dvbpsi_demux_subdec_t * p_subdec = p_demux->p_first_subdec;

  while(p_subdec)
  {
    if(p_subdec->i_id == i_id)
      break;

    p_subdec = p_subdec->p_next;
  }

  return p_subdec;
}

/*****************************************************************************
 * dvbpsi_Demux
 *****************************************************************************
 * Sends a PSI section to the right subtable decoder
 *****************************************************************************/
void dvbpsi_Demux(dvbpsi_handle p_decoder, dvbpsi_psi_section_t * p_section)
{
  dvbpsi_demux_t * p_demux;
  dvbpsi_demux_subdec_t * p_subdec;

  p_demux = (dvbpsi_demux_t *)p_decoder->p_private_decoder;


  p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
      p_section->i_extension);

  if(p_subdec == NULL)
  {
    /* Tell the application we found a new subtable, so that it may attach a
     * subtable decoder */
    p_demux->pf_new_callback(p_demux->p_new_cb_data, p_decoder,
                 p_section->i_table_id,
                  p_section->i_extension);
    /* Check if a new subtable decoder is available */
    p_subdec = dvbpsi_demuxGetSubDec(p_demux, p_section->i_table_id,
      p_section->i_extension);
  }

  if(p_subdec)
  {
    p_subdec->pf_callback(p_demux->p_decoder, p_subdec->p_cb_data, p_section);
  }
  else
  {
    dvbpsi_DeletePSISections(p_section);
  }
}

/*****************************************************************************
 * dvbpsi_DetachDemux
 *****************************************************************************
 * Destroys a demux structure
 *****************************************************************************/
void dvbpsi_DetachDemux(dvbpsi_handle h_dvbpsi)
{
  dvbpsi_demux_t* p_demux
                  = (dvbpsi_demux_t*)h_dvbpsi->p_private_decoder;
  dvbpsi_demux_subdec_t* p_subdec
                  = p_demux->p_first_subdec;
  dvbpsi_demux_subdec_t* p_subdec_temp;

  while(p_subdec)
  {
    p_subdec_temp = p_subdec;
    p_subdec = p_subdec->p_next;
    if(p_subdec_temp->pf_detach)
        p_subdec_temp->pf_detach(p_demux, (p_subdec_temp->i_id >> 16) & 0xFFFF,
                                 p_subdec_temp->i_id & 0xFFFF);
    else free(p_subdec_temp);
  }

  free(p_demux);
  if(h_dvbpsi->p_current_section)
    dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
  free(h_dvbpsi);
}
