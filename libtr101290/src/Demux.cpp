/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Demux.h"
#include "global.h"
#include "TrCore.h"
#include "csysclock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "EiLog.h"

using namespace tr101290;
using namespace std;

CDemux::CDemux(CTrCore* pParent)
{
	m_pParent = pParent;

	m_pPsiCk = new CPsiCheck(pParent);
	m_h_dvbpsi_pat = dvbpsi_AttachPAT(DumpPAT, this);
	m_bDemuxFinish = false;
	m_nUsedPcrPid = -1;
	m_pOldOccurTime = new long long[8192];
	m_pKnownPid = new bool[8192];
	
	for (int i = 0; i < 8192; i++)
	{
		m_pOldOccurTime[i] = -1;

		if (i <=0x1F)
			m_pKnownPid[i] = true;
		else
			m_pKnownPid[i] = false;
	}
	
	m_pKnownPid[0x1FFF] = true;
	

	m_llFirstPcr = -1;
}

CDemux::~CDemux()
{
	delete m_pPsiCk;

	delete [] m_pOldOccurTime;

	delete [] m_pKnownPid;

	vector<PROGRAM_INFO>::iterator it = m_vecDemuxInfoBuf.begin();
	for (;it != m_vecDemuxInfoBuf.end(); ++it)
	{
		delete it->pCalcPcrN1;
	}

	dvbpsi_DetachPAT(m_h_dvbpsi_pat);


	map<int,PMTINFO>::iterator itmap = m_mapPmtmInfo.begin();
	for (; itmap != m_mapPmtmInfo.end(); ++itmap)
	{
		dvbpsi_DetachPMT(itmap->second.handle);
	}
}



void CDemux::Demux(uint8_t* pPacket)
{


	uint16_t i_pid = ((uint16_t)(pPacket[1] & 0x1f) << 8) + pPacket[2];

	if (m_mapPmtmInfo.empty())
	{
		if(i_pid == 0x0)
		{
			dvbpsi_PushPacket(m_h_dvbpsi_pat, pPacket);
		}
		return;
	}
	else	// pmt
	{
		map<int,PMTINFO>::iterator it = m_mapPmtmInfo.begin();
		for (; it != m_mapPmtmInfo.end(); ++it)
		{
			if (it->second.pmt_pid == i_pid)
			{
				dvbpsi_PushPacket (it->second.handle,pPacket);
			}
		}
	}
	

	//判断是否解析完毕
	map<int,PMTINFO>::iterator it = m_mapPmtmInfo.begin();
	for (; it != m_mapPmtmInfo.end(); ++it)
	{
		if (!it->second.parsed)
		{
			//m_vecDemuxInfoBuf.clear();
			return;
		}
	}

	

	m_bDemuxFinish = true;

}

/*****************************************************************************
 * DumpPAT
 *****************************************************************************/
void CDemux::DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat)
{
	CDemux * lpthis = (CDemux*) p_zero;
	dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
	int count = 0;
	while(p_program)
	{
		count++;
		if (p_program->i_number != 0)	// 0 is nit
		{
			PMTINFO pmtinfo;
			pmtinfo.pmt_pid = p_program->i_pid;
			pmtinfo.handle = dvbpsi_AttachPMT(p_program->i_number, DumpPMT, p_zero);

			lpthis->m_mapPmtmInfo[p_program->i_number] = pmtinfo;

			lpthis->m_pPsiCk->AddPmtPid(p_program->i_pid,p_program->i_number);
			
		}
		//else if (m_pCrcCkHds[p_program->i_pid] != NULL), newHadle....

		lpthis->m_pKnownPid[p_program->i_pid] = true;
		lpthis->m_mapUnReferPid.erase(p_program->i_pid);

		p_program = p_program->p_next;
	}

	ei_log(LV_DEBUG,"libtr101290", "PAT decode finish,program count:%d",count);
	dvbpsi_DeletePAT(p_pat);

}

/*****************************************************************************
 * DumpPMT
 *****************************************************************************/
void CDemux::DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt)
{
	CDemux * lpthis = (CDemux*) p_zero;
	dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;

	ei_log(LV_DEBUG,"libtr101290","PMT decode info: program_number=0x%02x (%d) ",p_pmt->i_program_number,p_pmt->i_program_number);
	ei_log( LV_DEBUG,"libtr101290", "PCR_PID=0x%x (%d)",p_pmt->i_pcr_pid, p_pmt->i_pcr_pid);

	map<int,PMTINFO>::iterator it = lpthis->m_mapPmtmInfo.find(p_pmt->i_program_number);
	if (it == lpthis->m_mapPmtmInfo.end())
		return;

	lpthis->m_pKnownPid[p_pmt->i_pcr_pid] = true;
	lpthis->m_mapUnReferPid.erase(p_pmt->i_pcr_pid);

	PROGRAM_INFO prog_info;
	while(p_es != NULL)
	{
		ei_log(LV_DEBUG,"libtr101290","es_pid=0x%02x (%d)  stream_type=0x%x",p_es->i_pid, p_es->i_pid,p_es->i_type);

		ES_INFO es_info;
		es_info.pid = p_es->i_pid;
		es_info.stream_type = p_es->i_type;
		prog_info.vecPayloadPid.push_back(es_info);

		lpthis->m_pKnownPid[p_es->i_pid] = true;
		lpthis->m_mapUnReferPid.erase(p_es->i_pid);

		p_es = p_es->p_next;
	}


	it->second.pcr_pid = p_pmt->i_pcr_pid;
	prog_info.nPcrPid = p_pmt->i_pcr_pid;
	prog_info.nPmtPid = it->second.pmt_pid;
	prog_info.pCalcPcrN1 = new CCalcPcrN1();

	it->second.parsed = true;

	lpthis->m_vecDemuxInfoBuf.push_back(prog_info);
	dvbpsi_DeletePMT(p_pmt);
}

void CDemux::AddPacket(uint8_t* pPacket)
{
	//demux
	if (!m_bDemuxFinish)
	{
		Demux(pPacket);
	}

	UpdateClock(pPacket);

	ProcessPacket(pPacket);
}

void CDemux::UpdateClock(uint8_t* pPacket)
{
	CTsPacket tsPacket;
	tsPacket.SetPacket(pPacket);
	int pid = tsPacket.Get_PID();

	//find first eff pcr
	if (m_nUsedPcrPid < 0)
	{
		vector<PROGRAM_INFO>::iterator it = m_vecDemuxInfoBuf.begin();
		for (;it != m_vecDemuxInfoBuf.end(); ++it)
		{
			if (pid == it->nPcrPid && tsPacket.Get_PCR_flag())
			{
				m_nUsedPcrPid = pid;
			}
		}
	}

	if (m_nUsedPcrPid < 0)
	{
		return;
	}

	if (pid == m_nUsedPcrPid && tsPacket.Get_PCR_flag())
	{
		//检测PCR跳变
		long long calcPCr = m_pParent->m_pSysClock->GetPcr();
		long long curPcr = tsPacket.Get_PCR();
		if ( calcPCr >= 0 && llabs( diff_pcr(calcPCr,curPcr))  > 270000 )
		{
			m_pParent->m_pSysClock->Reset(); //10ms认为跳变
		}

		m_pParent->m_pSysClock->AddPcrPacket(curPcr);
		if (m_llFirstPcr < 0) 
		{
			m_llFirstPcr = curPcr;
		}
	}
	else
	{
		m_pParent->m_pSysClock->AddPayloadPacket();
	}
}

inline bool CDemux::IsPmtPid(int pid)
{
	vector<PROGRAM_INFO>::iterator it = m_vecDemuxInfoBuf.begin();
	for (;it != m_vecDemuxInfoBuf.end(); ++it)
	{
		if (pid == it->nPmtPid)
		{
			return true;
		}
	}
	return false;
}

inline long long CDemux::CheckOccTime(int pid,long long llCurTime)
{
	if (m_pOldOccurTime[pid] == -1 || llCurTime == -1)
	{
		return -1;
	}

	long long interval = diff_pcr(llCurTime, m_pOldOccurTime[pid]) /*/ 27000*/;

	return interval;
}

bool CDemux::CheckEsPid(int pid,long long llCurTime,CTsPacket& tsPacket)
{
	bool bEsPid = false;
	vector<PROGRAM_INFO>::iterator it = m_vecDemuxInfoBuf.begin();
	for (;it != m_vecDemuxInfoBuf.end(); ++it)
	{
		vector<ES_INFO>::iterator ites = it->vecPayloadPid.begin();
		for (; ites != it->vecPayloadPid.end(); ++ites)
		{
			if (m_pOldOccurTime[ites->pid] != -2)
			{
				//check timeout
				if (m_pOldOccurTime[ites->pid] != -1)	//pid dis aper
				{
					long long interval = diff_pcr(llCurTime, m_pOldOccurTime[ites->pid]) / 27000;
					if (interval > 5000)
					{
						m_pOldOccurTime[ites->pid] = -2;//for never report agein
						m_pParent->Report(1,LV1_PID_ERROR,ites->pid,-1,-1);
					}
				}
				else	//pid never occur
				{
					long long interval = diff_pcr(llCurTime, m_llFirstPcr ) / 27000;
					if (interval > 5000)
					{
						m_pOldOccurTime[ites->pid] = -2;//for never report agein
						m_pParent->Report(1,LV1_PID_ERROR,ites->pid,-1,-1);
					}
				}
			}

			if (ites->pid == pid)
			{
				bEsPid = true;

				//check pts
				long long pts;
				long long calcPCr = m_pParent->m_pSysClock->GetPcr();
				if (tsPacket.Get_PTS(pts) && ites->llPrevPts_occ >= 0)
				{
					m_pParent->Report(2,LV2_PTS_ERROR,pid,diff_pcr(calcPCr, ites->llPrevPts_occ),-1);
					//Report(2,LV2_PTS_ERROR,pid,pts-ites->llPrevPts,-1);

					ites->llPrevPts = pts;
					ites->llPrevPts_occ = calcPCr;
				}
			}
		} //!for ites
	} //!for it


	return bEsPid;
}

void CDemux::CheckPCR(int pid,CTsPacket& tsPacket)
{
	vector<PROGRAM_INFO>::iterator it = m_vecDemuxInfoBuf.begin();
	for (;it != m_vecDemuxInfoBuf.end(); ++it)
	{
		if (pid == it->nPcrPid && tsPacket.Get_PCR_flag())
		{
			long long pcr = tsPacket.Get_PCR();
			
			BYTE afLen;
			BYTE* pAf = tsPacket.Get_adaptation_field(afLen);
			int discontinuity_indicator = 0;
			if (pAf != NULL)
			{
				if (tsPacket.Get_discontinuity_indicator(pAf)) discontinuity_indicator = 1;
			}

			//check pcr it
			long long pcr_prev = it->pCalcPcrN1->GetPcrPrev();
			if (pcr_prev != -1)
			{
				m_pParent->Report(2,LV2_PCR_REPETITION_ERROR,pid, pcr - pcr_prev,discontinuity_indicator);
			}

			//check pcr ac
			long long pcr_calc = it->pCalcPcrN1->GetPcr();
			if (pcr_calc != -1)
			{
				m_pParent->Report(2,LV2_PCR_ACCURACY_ERROR,pid, pcr - pcr_calc,-1);
			}

			it->pCalcPcrN1->AddPcrPacket(pcr);
		}
		else
		{
			it->pCalcPcrN1->AddPayloadPacket();
		}
	}
}

void CDemux::CheckUnreferPid(int pid,long long llCurTime)
{
	long long interval = 0;

	//添加一种新的PID
	if (!m_pKnownPid[pid])
	{
		m_pKnownPid[pid] = true;
		m_mapUnReferPid[pid] = llCurTime;
		return;
	}

	//检测超时
	map<int,long long>::iterator it = m_mapUnReferPid.begin();
	for (;it != m_mapUnReferPid.end();++it)
	{
		if (it->second != -1)
		{
			interval = diff_pcr(llCurTime, it->second) / 27000;
			if (interval > 500)
			{
				//error here
				m_pParent->Report(3,LV3_UNREFERENCED_PID,pid,-1,-1);
				m_pKnownPid[pid] = true;
				m_mapUnReferPid.erase(it);
				return;
			}
		}
		else
		{
			it->second = llCurTime;
		}
	}
}


void CDemux::ProcessPacket(uint8_t* pPacket)
{
	CTsPacket tsPacket;
	tsPacket.SetPacket(pPacket);
	int pid = tsPacket.Get_PID();

	long long llCurTime = m_pParent->m_pSysClock->GetPcr();
	long long interval;

	bool bPsi = false;
	//pat err
	if (pid == 0)
	{
		//check occ
		if ((interval=CheckOccTime(0,llCurTime)) > 0)
		{
			m_pParent->Report(1,LV1_PAT_ERROR_OCC,0,interval,-1);
		}
		

		//check tid
		CPrivate_Section cs;
		if (cs.SetPacket(tsPacket))
		{
			if (cs.Get_table_id() != 0)
			{
				m_pParent->Report(1,LV1_PAT_ERROR_TID,0,-1,-1);
			}
		}

		//check scf
		if (tsPacket.Get_transport_scrambling_control() != 0)
		{
			m_pParent->Report(1,LV1_PAT_ERROR_SCF,0,-1,-1);
		}

		bPsi = true;
	}
	
	//pmt err
	if (IsPmtPid(pid))
	{
		//check occ
		if ((interval = CheckOccTime(pid,llCurTime)) > 0)
		{
			m_pParent->Report(1,LV1_PMT_ERROR_OCC,pid,interval,-1);
		}

		//check scf
		if (tsPacket.Get_transport_scrambling_control() != 0)
		{
			m_pParent->Report(1,LV1_PMT_ERROR_SCF,pid,-1,-1);
		}

		bPsi = true;
	}

	//cat err
	if (pid == 1)
	{
		//check tid
		CPrivate_Section cs;
		if (cs.SetPacket(tsPacket))
		{
			if (cs.Get_table_id() != 1)
			{
				m_pParent->Report(2,LV2_CAT_ERROR_TID,1,-1,-1);
			}
		}

		bPsi = true;
	}


	bool bEsPid = false;
	//pid err and pts err
	if (llCurTime > 0)
	{
		bEsPid = CheckEsPid(pid,llCurTime,tsPacket);
	}

	m_pPsiCk->AddPacket(pPacket,bEsPid,pid);
	

	

	//check pcr error
	CheckPCR(pid,tsPacket);

	if (!bPsi)
	{
		CheckUnreferPid(pid,llCurTime);
	}

	if (llCurTime < 0)
	{
		//当没有计算到时去码流中遇到的第一个PCR。
		//当收到第二个PCR包的时候，N1 PCR 已经计算成功了
		m_pOldOccurTime[pid] = m_llFirstPcr;
	}
	else
	{
		m_pOldOccurTime[pid] = llCurTime;
	}
}

bool CDemux::IsDemuxFinish()
{
	return m_bDemuxFinish;
}




