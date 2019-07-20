/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "PsiCheck.h"
#include "global.h"
#include "TrCore.h"
#include "csysclock.h"
#include "CBit.h"
#include <stdlib.h>
#include <string.h>

using namespace tr101290;


//static void DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat)
//{
//	dvbpsi_DeletePAT(p_pat);
//}
//
//static void DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt)
//{
//	dvbpsi_DeletePMT(p_pmt);
//}
//
//static void DumpCAT(void* p_cb_data, dvbpsi_cat_t* p_new_cat)
//{
//	dvbpsi_DeleteCAT(p_new_cat);
//}
//
//static void DumpNIT(void* p_cb_data, dvbpsi_nit_t* p_new_nit)
//{
//	dvbpsi_DeleteNIT(p_new_nit);
//}



CPsiCheck::CPsiCheck(CTrCore* pParent)
{
	m_pParent = pParent;

	m_pAllCkHds = new dvbpsi_handle[8192];
	m_pOldOccurTime = new long long[256];
	
	for (int i = 0; i < 8192; i++)
	{
		m_pAllCkHds[i] = NULL;

		if (i < 256)
		{
			m_pOldOccurTime[i] = -1;
		}
	}

	InitAllCkHds();

	m_bHaveEit_P = false;
	m_bHaveEit_F = false;
	m_bWaitFirstEitSection = true;

}

CPsiCheck::~CPsiCheck()
{
	UnInitAllCkHds();
	delete [] m_pAllCkHds;
	delete [] m_pOldOccurTime;
}

void CPsiCheck::UnInitAllCkHds()
{
	//dvbpsi_DetachPAT(m_pAllCkHds[0]);
	//m_pAllCkHds[0] = NULL;

	//dvbpsi_DetachCAT(m_pAllCkHds[1]);
	//m_pAllCkHds[1] = NULL;


	//pmt
	for (int i = 0; i < 8192; i++)
	{
		if (m_pAllCkHds[i] != NULL)
		{
			FreeHandle(m_pAllCkHds[i]);
		}
	}
}

void CPsiCheck::InitAllCkHds()
{
	m_pAllCkHds[0] = NewHandle();		//PAT
	m_pAllCkHds[1] = NewHandle();		//CAT
	m_pAllCkHds[0x10] = NewHandle();	//NIT
	m_pAllCkHds[0x11] = NewHandle();	//SDT BAT ST
	m_pAllCkHds[0x12] = NewHandle();	//EIT ST
	m_pAllCkHds[0x14] = NewHandle();	//TOT TDT ST
	
}

void CPsiCheck::AddPmtPid(int pid,int program_num)
{
	//这里可能覆盖其他PID
	m_pAllCkHds[pid] = NewHandle();
}

void CPsiCheck::AddPacket(uint8_t* pPacket,bool bEs,int pid)
{
	//if (pid == 0x12)
	//{
	//	m_bEitPFPacket = true;
	//}

	if (!bEs && m_pAllCkHds[pid] != NULL)
	{
		//check psi
		PushPacket(m_pAllCkHds[pid],pPacket,pid);
	}

	//check eit PF error
	//CheckEitPF(pid);

}

void CPsiCheck::CheckPrevEitPF(int pid)
{

	if (!(m_bHaveEit_P && m_bHaveEit_F))
	{
		m_pParent->Report(3,LV3_PF_ERROR,pid,-1,-1);
	}


	m_bHaveEit_P = false;
	m_bHaveEit_F = false;
}

void CPsiCheck::UpdatePF(int section_number)
{
	if (section_number == 0)
	{
		m_bHaveEit_P = true;
	}
	else if (section_number == 1)
	{
		m_bHaveEit_F = true;
	}
}

void CPsiCheck::OnCrcError(uint8_t table_id,int pid)
{
	ERROR_NAME_T emName;

	switch(table_id)
	{
	case 0x00:
		emName = LV2_CRC_ERROR_PAT;
		break;
	case 0x01:
		emName = LV2_CRC_ERROR_CAT;
		break;
	case 0x02:
		emName = LV2_CRC_ERROR_PMT;
		break;
	case 0x40:	//NIT actual
	case 0x41:	//NIT other
		emName = LV2_CRC_ERROR_NIT;
		break;
	case 0x42:	//SDT actual
	case 0x46:	//SDT other
		emName = LV2_CRC_ERROR_SDT;
		break;
	case 0x4A:
		emName = LV2_CRC_ERROR_BAT;
		break;
	case 0x73:
		emName = LV2_CRC_ERROR_TOT;
		break;
	case 0x4E:	//EIT PF actual
	case 0x4F:	//EIT PF other
		emName = LV2_CRC_ERROR_EIT;
		break;
	default:
		emName = LV2_CRC_ERROR_OTHER;
		break;
	}

	if (emName != LV2_CRC_ERROR_OTHER)
	{
		m_pParent->Report(2,emName,pid,-1,-1);
	}
}

void CPsiCheck::OnRecvNewSection(dvbpsi_psi_section_t * p_section,int pid)
{
	long long llCurTime = m_pParent->m_pSysClock->GetPcr();

	//check timeout
	if (llCurTime != -1 && m_pOldOccurTime[p_section->i_table_id] != -1)
	{
		ERROR_NAME_T emName;
		long long interval = diff_pcr(llCurTime, m_pOldOccurTime[p_section->i_table_id]) /*/ 27000*/;

		//2015-2-11 去掉对i_number的判断，似乎没用,而且会导致i_number大于等于2时没有处理，进而导致程序崩溃
		//if (p_section->i_number == 1)	//EIT_PF_F
		//{
		//	switch(p_section->i_table_id)
		//	{
		//	case 0x4E:
		//		emName = LV3_PSI_INTERVAL_EIT_PF_ACT;
		//		break;
		//	case 0x4F:
		//		emName = LV3_PSI_INTERVAL_EIT_PF_OTHER;
		//		break;
		//	default:
		//		emName = LV3_PSI_INTERVAL_OTHER;
		//		break;
		//	}
		//}
		//else if (p_section->i_number == 0)
		//{
			switch(p_section->i_table_id)
			{
			case 0x00:
				emName = LV3_PSI_INTERVAL_PAT;
				break;
			case 0x01:
				emName = LV3_PSI_INTERVAL_CAT;
				break;
			case 0x02:
				emName = LV3_PSI_INTERVAL_PMT;
				break;
			case 0x03:
				emName = LV3_PSI_INTERVAL_TSDT;
				break;
			case 0x40:
				emName = LV3_PSI_INTERVAL_NIT_ACT;
				break;
			case 0x41:
				emName = LV3_PSI_INTERVAL_NIT_OTHER;
				break;
			case 0x42:
				emName = LV3_PSI_INTERVAL_SDT_ACT;
				break;
			case 0x46:
				emName = LV3_PSI_INTERVAL_SDT_OTHER;
				break;
			case 0x4A:
				emName = LV3_PSI_INTERVAL_BAT;
				break;
			case 0x4E:
				emName = LV3_PSI_INTERVAL_EIT_PF_ACT;
				break;
			case 0x4F:
				emName = LV3_PSI_INTERVAL_EIT_PF_OTHER;
				break;
			case 0x70:
				emName = LV3_PSI_INTERVAL_TDT;
				break;
			case 0x71:
				emName = LV3_PSI_INTERVAL_RST;
				break;
			case 0x72:
				emName = LV3_PSI_INTERVAL_ST;
				break;
			case 0x73:
				emName = LV3_PSI_INTERVAL_TOT;
				break;
			case 0x7E:
				emName = LV3_PSI_INTERVAL_DIT;
				break;
			case 0x7F:
				emName = LV3_PSI_INTERVAL_SIT;
				break;

			default:
				emName = LV3_PSI_INTERVAL_OTHER;
				break;
			}
			
//		} //!else if

		if (p_section->i_table_id >= 0x50 && p_section->i_table_id <= 0x5F)
			emName = LV3_PSI_INTERVAL_EIT_SCHEDULE_ACT;
		else if(p_section->i_table_id >= 0x60 && p_section->i_table_id <= 0x6F)
			emName = LV3_PSI_INTERVAL_EIT_SCHEDULE_OTHER;

		m_pParent->Report(3,emName,pid,interval,-1);
	}
	
	
	//check tid error
	if (pid == 0x10) //nit tid error
	{
		if (p_section->i_table_id != 0x40 && p_section->i_table_id != 0x41 &&
			 p_section->i_table_id != 0x72)
		{
			m_pParent->Report(3,LV3_NIT_ERROR_TID,pid,-1,-1);
		}
	}	
	else if (pid == 0x11) //sdt tid error
	{
		if (p_section->i_table_id != 0x42 && p_section->i_table_id != 0x46 &&
			p_section->i_table_id != 0x4A && p_section->i_table_id != 0x72)
		{
			m_pParent->Report(3,LV3_SDT_ERROR_TID,pid,-1,-1);
		}
	}
	else if (pid == 0x12) //eit tid error
	{
		if (p_section->i_table_id >= 0x4E && p_section->i_table_id <= 0x6F)
		{
		}
		else if (p_section->i_table_id == 0x72)
		{
		}
		else
		{
			m_pParent->Report(3,LV3_EIT_ERROR_TID,pid,-1,-1);
		}
	}
	else if (pid == 0x13) //rst tid error
	{
		if (p_section->i_table_id != 0x71 && p_section->i_table_id != 0x72)
		{
			m_pParent->Report(3,LV3_RST_ERROR_TID,pid,-1,-1);
		}
	}
	else if (pid == 0x14) //tdt tid error
	{
		if (p_section->i_table_id != 0x70 && p_section->i_table_id != 0x72 &&
			p_section->i_table_id != 0x73)
		{
			m_pParent->Report(3,LV3_TDT_ERROR_TID,pid,-1,-1);
		}
	}

	//update PF
	if (p_section->i_table_id == 0x4E || p_section->i_table_id == 0x4F)
	{
		eit_section_t cur_eit_section;
		cur_eit_section.i_service_id	 = CBit::getBits (p_section->p_data, 0, 24, 16);
		cur_eit_section.i_version 		 = CBit::getBits (p_section->p_data, 0, 42,  5);
		cur_eit_section.i_ts_id			 = CBit::getBits (p_section->p_data, 0, 64, 16);
		cur_eit_section.i_network_id	 = CBit::getBits (p_section->p_data, 0, 80, 16);
		cur_eit_section.i_number		 = p_section->i_number;
		cur_eit_section.i_table_id		 = p_section->i_table_id;
		if (m_bWaitFirstEitSection)
		{
			m_bWaitFirstEitSection = false;
		}
		else
		{
			if (cur_eit_section.i_service_id	== m_PrevEitSection.i_service_id &&
				cur_eit_section.i_version		== m_PrevEitSection.i_version &&
				cur_eit_section.i_ts_id			== m_PrevEitSection.i_ts_id &&
				cur_eit_section.i_network_id	== m_PrevEitSection.i_network_id &&
				cur_eit_section.i_table_id		== m_PrevEitSection.i_table_id)
			{
			}
			else
			{
				//on new subtab,check prev pf
				CheckPrevEitPF(pid);
			}

		}
		UpdatePF(p_section->i_number );
		m_PrevEitSection = cur_eit_section;

	}


	m_pOldOccurTime[p_section->i_table_id] = llCurTime;
}


/*****************************************************************************
 * dvbpsi_AttachPAT
 *****************************************************************************
 * Initialize a PAT decoder and return a handle on it.
 *****************************************************************************/
dvbpsi_handle CPsiCheck::NewHandle(/*dvbpsi_pat_callback pf_callback,
                               void* p_cb_data*/)
{
  dvbpsi_handle h_dvbpsi = (dvbpsi_decoder_t*)malloc(sizeof(dvbpsi_decoder_t));
  //dvbpsi_pat_decoder_t* p_pat_decoder;

  if(h_dvbpsi == NULL)
    return NULL;

 /* p_pat_decoder = (dvbpsi_pat_decoder_t*)malloc(sizeof(dvbpsi_pat_decoder_t));

  if(p_pat_decoder == NULL)
  {
    free(h_dvbpsi);
    return NULL;
  }*/

  /* PSI decoder configuration */
  //h_dvbpsi->pf_callback = &dvbpsi_GatherPATSections;
  //h_dvbpsi->p_private_decoder = p_pat_decoder;
  h_dvbpsi->i_section_max_size = 1024;
  /* PSI decoder initial state */
  h_dvbpsi->i_continuity_counter = 31;
  h_dvbpsi->b_discontinuity = 1;
  h_dvbpsi->p_current_section = NULL;

  /* PAT decoder information */
  //p_pat_decoder->pf_callback = pf_callback;
  //p_pat_decoder->p_cb_data = p_cb_data;
  /* PAT decoder initial state */
  /*p_pat_decoder->b_current_valid = 0;
  p_pat_decoder->p_building_pat = NULL;
  for(unsigned int i = 0; i <= 255; i++)
    p_pat_decoder->ap_sections[i] = NULL;*/

  return h_dvbpsi;
}


/*****************************************************************************
 * dvbpsi_DetachPAT
 *****************************************************************************
 * Close a PAT decoder. The handle isn't valid any more.
 *****************************************************************************/
void CPsiCheck::FreeHandle(dvbpsi_handle h_dvbpsi)
{
  //dvbpsi_pat_decoder_t* p_pat_decoder
  //                      = (dvbpsi_pat_decoder_t*)h_dvbpsi->p_private_decoder;

  //free(p_pat_decoder->p_building_pat);

 /* for(unsigned int i = 0; i <= 255; i++)
  {
    if(p_pat_decoder->ap_sections[i])
      free(p_pat_decoder->ap_sections[i]);
  }*/

  //free(h_dvbpsi->p_private_decoder);
  if(h_dvbpsi->p_current_section)
    dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
  free(h_dvbpsi);
}


/*****************************************************************************
 * dvbpsi_PushPacket
 *****************************************************************************
 * Injection of a TS packet into a PSI decoder.
 *****************************************************************************/
void CPsiCheck::PushPacket(dvbpsi_handle h_dvbpsi, uint8_t* p_data,int pid)
{
  uint8_t i_expected_counter;           /* Expected continuity counter */
  dvbpsi_psi_section_t* p_section;      /* Current section */
  uint8_t* p_payload_pos;               /* Where in the TS packet */
  uint8_t* p_new_pos = NULL;            /* Beginning of the new section,
                                           updated to NULL when the new
                                           section is handled */
  int i_available;                      /* Byte count available in the
                                           packet */

  /* TS start code */
  if(p_data[0] != 0x47)
  {
    //DVBPSI_ERROR("PSI decoder", "not a TS packet");
    return;
  }

  /* Continuity check */
  i_expected_counter = (h_dvbpsi->i_continuity_counter + 1) & 0xf;
  h_dvbpsi->i_continuity_counter = p_data[3] & 0xf;

  if(i_expected_counter == ((h_dvbpsi->i_continuity_counter + 1) & 0xf)
      && !h_dvbpsi->b_discontinuity)
  {
   /* DVBPSI_ERROR_ARG("PSI decoder",
                     "TS duplicate (received %d, expected %d) for PID %d",
                     h_dvbpsi->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);*/
    return;
  }

  if(i_expected_counter != h_dvbpsi->i_continuity_counter)
  {
    /*DVBPSI_ERROR_ARG("PSI decoder",
                     "TS discontinuity (received %d, expected %d) for PID %d",
                     h_dvbpsi->i_continuity_counter, i_expected_counter,
                     ((uint16_t)(p_data[1] & 0x1f) << 8) | p_data[2]);*/
    h_dvbpsi->b_discontinuity = 1;
    if(h_dvbpsi->p_current_section)
    {
      dvbpsi_DeletePSISections(h_dvbpsi->p_current_section);
      h_dvbpsi->p_current_section = NULL;
    }
  }

  /* Return if no payload in the TS packet */
  if(!(p_data[3] & 0x10))
  {
    return;
  }

  /* Skip the adaptation_field if present */
  if(p_data[3] & 0x20)
    p_payload_pos = p_data + 5 + p_data[4];
  else
    p_payload_pos = p_data + 4;

  /* Unit start -> skip the pointer_field and a new section begins */
  if(p_data[1] & 0x40)
  {
    p_new_pos = p_payload_pos + *p_payload_pos + 1;
    p_payload_pos += 1;
  }

  p_section = h_dvbpsi->p_current_section;

  /* If the psi decoder needs a begginning of section and a new section
     begins in the packet then initialize the dvbpsi_psi_section_t structure */
  if(p_section == NULL)
  {
    if(p_new_pos)
    {
      /* Allocation of the structure */
      h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
      /* Update the position in the packet */
      p_payload_pos = p_new_pos;
      /* New section is being handled */
      p_new_pos = NULL;
      /* Just need the header to know how long is the section */
      h_dvbpsi->i_need = 3;
      h_dvbpsi->b_complete_header = 0;
    }
    else
    {
      /* No new section => return */
      return;
    }
  }

  /* Remaining bytes in the payload */
  i_available = 188 + p_data - p_payload_pos;

  while(i_available > 0)
  {
    if(i_available >= h_dvbpsi->i_need)
    {
      /* There are enough bytes in this packet to complete the
         header/section */
      memcpy(p_section->p_payload_end, p_payload_pos, h_dvbpsi->i_need);
      p_payload_pos += h_dvbpsi->i_need;
      p_section->p_payload_end += h_dvbpsi->i_need;
      i_available -= h_dvbpsi->i_need;

      if(!h_dvbpsi->b_complete_header)
      {
        /* Header is complete */
        h_dvbpsi->b_complete_header = 1;
        /* Compute p_section->i_length and update h_dvbpsi->i_need */
        h_dvbpsi->i_need = p_section->i_length
                         =   ((uint16_t)(p_section->p_data[1] & 0xf)) << 8
                           | p_section->p_data[2];
        /* Check that the section isn't too long */
        if(h_dvbpsi->i_need > h_dvbpsi->i_section_max_size - 3)
        {
//          DVBPSI_ERROR("PSI decoder", "PSI section too long");
          dvbpsi_DeletePSISections(p_section);
          h_dvbpsi->p_current_section = NULL;
          /* If there is a new section not being handled then go forward
             in the packet */
          if(p_new_pos)
          {
            h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
            p_payload_pos = p_new_pos;
            p_new_pos = NULL;
            h_dvbpsi->i_need = 3;
            h_dvbpsi->b_complete_header = 0;
            i_available = 188 + p_data - p_payload_pos;
          }
          else
          {
            i_available = 0;
          }
        }
      }
      else
      {
        /* PSI section is complete */
        p_section->b_syntax_indicator = p_section->p_data[1] & 0x80;
        p_section->b_private_indicator = p_section->p_data[1] & 0x40;
        /* Update the end of the payload if CRC_32 is present */
        if(p_section->b_syntax_indicator)
          p_section->p_payload_end -= 4;


        if(p_section->p_data[0] != 0x72 && dvbpsi_ValidPSISection(p_section))
        {
          /* PSI section is valid */
          p_section->i_table_id = p_section->p_data[0];
          if(p_section->b_syntax_indicator)
          {
            p_section->i_extension =   (p_section->p_data[3] << 8)
                                     | p_section->p_data[4];
            p_section->i_version = (p_section->p_data[5] & 0x3e) >> 1;
            p_section->b_current_next = p_section->p_data[5] & 0x1;
            p_section->i_number = p_section->p_data[6];
            p_section->i_last_number = p_section->p_data[7];
            p_section->p_payload_start = p_section->p_data + 8;
          }
          else
          {
            p_section->i_extension = 0;
            p_section->i_version = 0;
            p_section->b_current_next = 1;
            p_section->i_number = 0;
            p_section->i_last_number = 0;
            p_section->p_payload_start = p_section->p_data + 3;
          }

		  OnRecvNewSection(p_section,pid);
          //h_dvbpsi->pf_callback(h_dvbpsi, p_section);
		  dvbpsi_DeletePSISections(p_section);
          h_dvbpsi->p_current_section = NULL;
        }
        else
        {
          //report error here
		  OnCrcError(p_section->p_data[0],pid);

		   /* PSI section isn't valid => trash it */
          dvbpsi_DeletePSISections(p_section);
          h_dvbpsi->p_current_section = NULL;
        }

        /* A TS packet may contain any number of sections, only the first
         * new one is flagged by the pointer_field. If the next payload
         * byte isn't 0xff then a new section starts. */
        if(p_new_pos == NULL && i_available && *p_payload_pos != 0xff)
          p_new_pos = p_payload_pos;

        /* If there is a new section not being handled then go forward
           in the packet */
        if(p_new_pos)
        {
          h_dvbpsi->p_current_section
                        = p_section
                        = dvbpsi_NewPSISection(h_dvbpsi->i_section_max_size);
          p_payload_pos = p_new_pos;
          p_new_pos = NULL;
          h_dvbpsi->i_need = 3;
          h_dvbpsi->b_complete_header = 0;
          i_available = 188 + p_data - p_payload_pos;
        }
        else
        {
          i_available = 0;
        }
      }
    }
    else
    {
      /* There aren't enough bytes in this packet to complete the
         header/section */
      memcpy(p_section->p_payload_end, p_payload_pos, i_available);
      p_section->p_payload_end += i_available;
      h_dvbpsi->i_need -= i_available;
      i_available = 0;
    }
  }
}

