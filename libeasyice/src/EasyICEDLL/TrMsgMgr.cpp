/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "TrMsgMgr.h"
#include <stdlib.h>
//#include "EasyICE.h"

using namespace std;

//extern INPUT_TYPE_T g_emInputType;
const static int PCR_PERIOD_MS_25 = 27000*25;
const static int PCR_PERIOD_MS_100 = 27000*100;

CTrMsgMgr::CTrMsgMgr(void)
{
	SetPsiTimeOutDVB();
	ClearError();

	m_pTrMsg = new TR_MSG_T();
}

CTrMsgMgr::~CTrMsgMgr(void)
{
	delete m_pTrMsg;
}

void CTrMsgMgr::SetPsiTimeOut(PSI_TIMEOUT_VALUE_T ptv)
{
	m_PsiTimeOut = ptv;
}

void CTrMsgMgr::SetPsiTimeOutDVB()
{
	m_PsiTimeOut.pat		= 500;
	m_PsiTimeOut.cat		= 500;
	m_PsiTimeOut.pmt		= 500;
	m_PsiTimeOut.tsdt		= 10000;
	m_PsiTimeOut.nit_act	= 10000;
	m_PsiTimeOut.nit_other	= 10000;
	m_PsiTimeOut.sdt_act	= 2000;
	m_PsiTimeOut.sdt_other	= 10000;
	m_PsiTimeOut.bat		= 10000;
	m_PsiTimeOut.rst		= -1;
	m_PsiTimeOut.tdt		= 30000;
	m_PsiTimeOut.tot		= 30000;
	m_PsiTimeOut.dit		= -1;
	m_PsiTimeOut.sit		= -1;
	m_PsiTimeOut.eit_act_pf	= 2000;

	m_PsiTimeOut.eit_other_pf	= 10000;
	m_PsiTimeOut.eit_in8day		= 10000;
	m_PsiTimeOut.eit_out8day	= 30000;
}

void CTrMsgMgr::ClearError()
{
	m_nErrCnt_SyncLost = 0;
	 m_nErrCnt_SyncByte = 0;

	 m_nErrCnt_pat = 0;
	 m_nErrCnt_pat_occ = 0;
	 m_nErrCnt_pat_tid = 0;
	 m_nErrCnt_pat_scf = 0;

	 m_nErrCnt_cc = 0;

	 m_nErrCnt_pmt = 0;
	 m_nErrCnt_pmt_occ = 0;
	 m_nErrCnt_pmt_scf = 0;

	 m_nErrCnt_pid = 0;
	 m_nErrCnt_ts = 0;

	 m_nErrCnt_crc = 0;
	 m_nErrCnt_crc_pat = 0;
	 m_nErrCnt_crc_cat = 0;
	 m_nErrCnt_crc_pmt = 0;
	 m_nErrCnt_crc_nit = 0;
	 m_nErrCnt_crc_sdt = 0;
	 m_nErrCnt_crc_bat = 0;
	 m_nErrCnt_crc_tot = 0;
	 m_nErrCnt_crc_eit = 0;

	 m_nErrCnt_pcr_repetition = 0;
	 m_nErrCnt_pcr_discontinuity = 0;
	 m_nErrCnt_pcr_ac = 0;
	 m_nErrCnt_pts = 0;

	 m_nErrCnt_cat = 0;
	 m_nErrCnt_cat_tid = 0;

	 m_nErrCnt_nit_act = 0;
	 m_nErrCnt_nit_act_tid = 0;
	 m_nErrCnt_nit_act_timeout = 0;
	 m_nErrCnt_nit_act_lower25ms = 0;

	 m_nErrCnt_nit_other = 0;
	 m_nErrCnt_nit_other_timeout = 0;
	 m_nErrCnt_nit_other_lower25ms = 0;


	 m_nErrCnt_si_repetition = 0;
	 m_nErrCnt_si_repetition_lower25ms = 0;

	 m_nErrCnt_unrefer_pid = 0;

	 m_nErrCnt_sdt_act = 0;
	 m_nErrCnt_sdt_act_tid = 0;
	 m_nErrCnt_sdt_act_timeout = 0;
	 m_nErrCnt_sdt_act_lower25ms = 0;

	 m_nErrCnt_sdt_other = 0;
	 m_nErrCnt_sdt_other_timeout = 0;
	 m_nErrCnt_sdt_other_lower25ms = 0;

	 m_nErrCnt_eit_pf_act = 0;
	 m_nErrCnt_eit_pf_act_tid = 0;
	 m_nErrCnt_eit_pf_act_timeout = 0;
	 m_nErrCnt_eit_pf_act_lower25ms = 0;

	 m_nErrCnt_eit_pf_other = 0;
	 m_nErrCnt_eit_pf_other_timeout = 0;
	 m_nErrCnt_eit_pf_other_lower25ms = 0;

	 m_nErrCnt_eit_pf = 0;

	 m_nErrCnt_rst = 0;
	 m_nErrCnt_rst_tid = 0;
	 m_nErrCnt_rst_lower25ms = 0;

	 m_nErrCnt_tdt = 0;
	 m_nErrCnt_tdt_tid = 0;
	 m_nErrCnt_tdt_timeout = 0;
	 m_nErrCnt_tdt_lower25ms = 0;

	 m_nErrCnt_bat_timeout = 0;
	 m_nErrCnt_bat_lower25ms = 0;

	 m_nErrCnt_eit_schedule_act_timeout = 0;
	 m_nErrCnt_eit_schedule_act_lower25ms = 0;

	 m_nErrCnt_eit_schedule_other_timeout = 0;
	 m_nErrCnt_eit_schedule_other_lower25ms = 0;

	 m_nErrCnt_tot_timeout = 0;
	 m_nErrCnt_tot_lower25ms = 0;

	 m_vecMsg.clear();
}

TR_MSG_T* CTrMsgMgr::AddMsg(const REPORT_PARAM_T& msg)
{

	if (!CheckEnabled(msg))
	{
		m_pTrMsg->nErrCount = -1;
		m_pTrMsg->nSiRepetitionCount = -1;
		return m_pTrMsg;
	}
	
	if (m_vecMsg.size() > MAX_MSG_SAVE_CNT)
	{
		m_vecMsg.pop_front();
	}

	m_vecMsg.push_back(msg);

	//阀值过滤开关
	if (!CheckFilter(msg))
	{
		m_pTrMsg->nErrCount = -1;
		m_pTrMsg->nSiRepetitionCount = -1;
		return m_pTrMsg;
	}

	//分类信息及数值统计

	return ProcessReport(msg);
}

TR_MSG_T* CTrMsgMgr::PlayBackMsg(const REPORT_PARAM_T& msg)
{
	m_pTrMsg->nErrCount = -1;
	m_pTrMsg->nSiRepetitionCount = -1;

	//阀值过滤开关
	if (!CheckFilter(msg))
	{
		m_pTrMsg->nErrCount = -1;
		return m_pTrMsg;
	}

	//分类信息及数值统计

	return ProcessReport(msg,true);
}

bool CTrMsgMgr::CheckEnabled(const REPORT_PARAM_T& msg)
{
	return true;
}

bool CTrMsgMgr::CheckFilter(const REPORT_PARAM_T& msg)
{
	switch (msg.errName)
	{
	case LV1_PAT_ERROR_OCC:
		if (msg.llVal/27000 < 500)
		{
			return false;
		}
		break;
	case LV1_PMT_ERROR_OCC:
		if (msg.llVal/27000 < 500)
		{
			return false;
		}
		break;
	case LV2_PCR_REPETITION_ERROR:
		if (msg.llVal/27000 < 40)
		{
			return false;
		}
		break;
	case LV2_PCR_ACCURACY_ERROR:
		if (llabs(msg.llVal) < 0.5*27)
		{
			return false;
		}
		break;
	case LV2_PTS_ERROR:
		if (msg.llVal/27000 < 700)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_NIT_ACT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.nit_act)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_NIT_OTHER:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.nit_other)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_SDT_ACT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.sdt_act)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_SDT_OTHER:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.sdt_other)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_EIT_PF_ACT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.eit_act_pf)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_EIT_PF_OTHER:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.eit_other_pf)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_RST:
		if (msg.llVal/27000 >= 25)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_TDT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.tdt)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_BAT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.bat)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_ACT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.eit_in8day)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_OTHER:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.eit_out8day)
		{
			return false;
		}
		break;
	case LV3_PSI_INTERVAL_TOT:
		if (msg.llVal/27000 >= 25 && msg.llVal/27000 <= m_PsiTimeOut.tot)
		{
			return false;
		}
		break;


	default:
		break;
	}

	return true;
}

TR_MSG_T* CTrMsgMgr::ProcessReport(const REPORT_PARAM_T& msg,bool bRepaly)
{
	m_pTrMsg->nErrCount = -1;
	m_pTrMsg->nSiRepetitionCount = -1;

//	m_pTrMsg->strErrName = _T("");

	switch (msg.errName)
	{
	case LV1_TS_SYNC_LOST:
		m_pTrMsg->emErrType = TR_LV1_SYNC_LOST;
		if (!bRepaly) ++m_nErrCnt_SyncLost;
		m_pTrMsg->nErrCount = m_nErrCnt_SyncLost;
		break;
	case LV1_SYNC_BYTE_ERROR:
		m_pTrMsg->emErrType = TR_LV1_SYNC_BYTE_ERR;
		if (!bRepaly) ++m_nErrCnt_SyncByte;
		m_pTrMsg->nErrCount = m_nErrCnt_SyncByte;
		break;
	case LV1_PAT_ERROR_OCC:
		m_pTrMsg->emErrType = TR_LV1_PAT_ERR;
		if (!bRepaly) m_nErrCnt_pat_occ++;
		if (!bRepaly) ++m_nErrCnt_pat;
		m_pTrMsg->nErrCount = m_nErrCnt_pat;
		break;
	case LV1_PAT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV1_PAT_ERR;
		if (!bRepaly) m_nErrCnt_pat_tid++;
		if (!bRepaly) ++m_nErrCnt_pat;
		m_pTrMsg->nErrCount = m_nErrCnt_pat;
		break;
	case LV1_PAT_ERROR_SCF:
		m_pTrMsg->emErrType = TR_LV1_PAT_ERR;
		if (!bRepaly) m_nErrCnt_pat_scf++;
		if (!bRepaly) ++m_nErrCnt_pat;
		m_pTrMsg->nErrCount = m_nErrCnt_pat;
		break;
	case LV1_CC_ERROR:
		m_pTrMsg->emErrType = TR_LV1_CC_ERR;
		if (!bRepaly) ++m_nErrCnt_cc;
		m_pTrMsg->nErrCount = m_nErrCnt_cc;
		break;
	case LV1_PMT_ERROR_OCC:
		m_pTrMsg->emErrType = TR_LV1_PMT_ERR;
		if (!bRepaly) m_nErrCnt_pmt_occ++;
		if (!bRepaly) ++m_nErrCnt_pmt;
		m_pTrMsg->nErrCount = m_nErrCnt_pmt;
		break;
	case LV1_PMT_ERROR_SCF:
		m_pTrMsg->emErrType = TR_LV1_PMT_ERR;
		if (!bRepaly) m_nErrCnt_pmt_scf++;
		if (!bRepaly) ++m_nErrCnt_pmt;
		m_pTrMsg->nErrCount = m_nErrCnt_pmt;
		break;
	case LV1_PID_ERROR:
		m_pTrMsg->emErrType = TR_LV1_PID_ERR;
		if (!bRepaly) ++m_nErrCnt_pid;
		m_pTrMsg->nErrCount = m_nErrCnt_pid;
		break;

	//---------------lv2----------------------

	case LV2_TRANSPORT_ERROR:
		m_pTrMsg->emErrType = TR_LV2_TS_ERR;
		if (!bRepaly) ++m_nErrCnt_ts;
		m_pTrMsg->nErrCount = m_nErrCnt_ts;
		break;
	case LV2_CRC_ERROR_PAT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_pat++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_CAT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_cat++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_PMT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_pmt++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_NIT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_nit++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_SDT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_sdt++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_BAT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_bat++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_TOT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_tot++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_CRC_ERROR_EIT:
		m_pTrMsg->emErrType = TR_LV2_CRC_ERR;
		if (!bRepaly) m_nErrCnt_crc_eit++;
		if (!bRepaly) ++m_nErrCnt_crc;
		m_pTrMsg->nErrCount = m_nErrCnt_crc;
		break;
	case LV2_PCR_REPETITION_ERROR:
		if (msg.llVal/27000 > 100 && (msg.fVal != 1))
		{
			m_pTrMsg->emErrType = TR_LV2_PCR_DISCON_ERR;
			if (!bRepaly) ++m_nErrCnt_pcr_discontinuity;
			m_pTrMsg->nErrCount = m_nErrCnt_pcr_discontinuity;
		}
		else
		{
			m_pTrMsg->emErrType = TR_LV2_PCR_REPET_ERR;
			if (!bRepaly) ++m_nErrCnt_pcr_repetition;
			m_pTrMsg->nErrCount = m_nErrCnt_pcr_repetition;
		}
		
		break;
	case LV2_PCR_ACCURACY_ERROR:
		m_pTrMsg->emErrType = TR_LV2_PCR_AC_ERR;
		if (!bRepaly) ++m_nErrCnt_pcr_ac;
		m_pTrMsg->nErrCount = m_nErrCnt_pcr_ac;
		break;
	case LV2_PTS_ERROR:
		m_pTrMsg->emErrType = TR_LV2_PTS_ERR;
		if (!bRepaly) ++m_nErrCnt_pts;
		m_pTrMsg->nErrCount = m_nErrCnt_pts;
		break;
	case LV2_CAT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV2_CAT_ERR;
		if (!bRepaly) m_nErrCnt_cat_tid++;
		if (!bRepaly) ++m_nErrCnt_cat;
		m_pTrMsg->nErrCount = m_nErrCnt_cat;
		break;

	//---------------lv3----------------------

	case LV3_NIT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV3_NIT_ACT_ERR;
		if (!bRepaly) m_nErrCnt_nit_act_tid++;
		if (!bRepaly) ++m_nErrCnt_nit_act;
		m_pTrMsg->nErrCount = m_nErrCnt_nit_act;
		break;
	case LV3_PSI_INTERVAL_NIT_ACT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_nit_act_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_nit_act_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_NIT_ACT_ERR;
		if (!bRepaly) ++m_nErrCnt_nit_act;
		m_pTrMsg->nErrCount = m_nErrCnt_nit_act;
		break;
	case LV3_PSI_INTERVAL_NIT_OTHER:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_nit_other_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_nit_other_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_NIT_OTHER_ERR;
		if (!bRepaly) ++m_nErrCnt_nit_other;
		m_pTrMsg->nErrCount = m_nErrCnt_nit_other;
		break;
	case LV3_UNREFERENCED_PID:
		m_pTrMsg->emErrType = TR_LV3_UNREFER_PID_ERR;
		if (!bRepaly) ++m_nErrCnt_unrefer_pid;
		m_pTrMsg->nErrCount = m_nErrCnt_unrefer_pid;
		break;
	case LV3_SDT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV3_SDT_ACT_ERR;
		if (!bRepaly) m_nErrCnt_sdt_act_tid++;
		if (!bRepaly) ++m_nErrCnt_sdt_act;
		m_pTrMsg->nErrCount = m_nErrCnt_sdt_act;
		break;
	case LV3_PSI_INTERVAL_SDT_ACT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_sdt_act_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_sdt_act_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SDT_ACT_ERR;
		if (!bRepaly) ++m_nErrCnt_sdt_act;
		m_pTrMsg->nErrCount = m_nErrCnt_sdt_act;
		break;
	case LV3_PSI_INTERVAL_SDT_OTHER:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_sdt_other_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_sdt_other_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SDT_OTHER_ERR;
		if (!bRepaly) ++m_nErrCnt_sdt_other;
		m_pTrMsg->nErrCount = m_nErrCnt_sdt_other;
		break;
	case LV3_EIT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV3_EIT_ACT_ERR;
		if (!bRepaly) m_nErrCnt_eit_pf_act_tid++;
		if (!bRepaly) ++m_nErrCnt_eit_pf_act;
		m_pTrMsg->nErrCount = m_nErrCnt_eit_pf_act;
		break;
	case LV3_PSI_INTERVAL_EIT_PF_ACT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_eit_pf_act_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_eit_pf_act_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_EIT_ACT_ERR;
		if (!bRepaly) ++m_nErrCnt_eit_pf_act;
		m_pTrMsg->nErrCount = m_nErrCnt_eit_pf_act;
		break;
	case LV3_PSI_INTERVAL_EIT_PF_OTHER:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_eit_pf_other_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_eit_pf_other_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_EIT_OTHER_ERR;
		if (!bRepaly) ++m_nErrCnt_eit_pf_other;
		m_pTrMsg->nErrCount = m_nErrCnt_eit_pf_other;
		break;
	case LV3_PF_ERROR:
		m_pTrMsg->emErrType = TR_LV3_EIT_PF_ERR;
		if (!bRepaly) ++m_nErrCnt_eit_pf;
		m_pTrMsg->nErrCount = m_nErrCnt_eit_pf;
		break;
	case LV3_RST_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV3_RST_ERR;
		if (!bRepaly) m_nErrCnt_rst_tid++;
		if (!bRepaly) ++m_nErrCnt_rst;
		m_pTrMsg->nErrCount = m_nErrCnt_rst;
		break;
	case LV3_PSI_INTERVAL_RST:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_rst_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		m_pTrMsg->emErrType = TR_LV3_RST_ERR;
		if (!bRepaly) ++m_nErrCnt_rst;
		m_pTrMsg->nErrCount = m_nErrCnt_rst;
		break;
	case LV3_TDT_ERROR_TID:
		m_pTrMsg->emErrType = TR_LV3_TDT_ERR;
		if (!bRepaly) m_nErrCnt_tdt_tid++;
		if (!bRepaly) ++m_nErrCnt_tdt;
		m_pTrMsg->nErrCount = m_nErrCnt_tdt;
		break;
	case LV3_PSI_INTERVAL_TDT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_tdt_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_tdt_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_TDT_ERR;
		if (!bRepaly) ++m_nErrCnt_tdt;
		m_pTrMsg->nErrCount = m_nErrCnt_tdt;
		break;

// ----------剩余没处理的SI_repetition------------
	case LV3_PSI_INTERVAL_BAT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_bat_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_bat_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SI_REPET_ERR;
		m_pTrMsg->nErrCount = m_nErrCnt_si_repetition;
		break;
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_ACT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_eit_schedule_act_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_eit_schedule_act_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SI_REPET_ERR;
		m_pTrMsg->nErrCount = m_nErrCnt_si_repetition;
		break;
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_OTHER:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_eit_schedule_other_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_eit_schedule_other_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SI_REPET_ERR;
		m_pTrMsg->nErrCount = m_nErrCnt_si_repetition;
		break;
	case LV3_PSI_INTERVAL_TOT:
		if (msg.llVal / 27000 < 25)
		{
			if (!bRepaly) m_nErrCnt_tot_lower25ms++;
			if (!bRepaly) m_nErrCnt_si_repetition_lower25ms++;
		}
		else
		{
			if (!bRepaly) m_nErrCnt_tot_timeout++;
		}
		if (!bRepaly) ++m_nErrCnt_si_repetition;
		m_pTrMsg->nSiRepetitionCount = m_nErrCnt_si_repetition;
		m_pTrMsg->emErrType = TR_LV3_SI_REPET_ERR;
		m_pTrMsg->nErrCount = m_nErrCnt_si_repetition;
		break;


	default:
		m_pTrMsg->nErrCount = -1;
		break;
	}



	return m_pTrMsg;
}
