/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CDEMUX_H
#define CDEMUX_H

#include "ccalcpcrn1.h"
#include "PsiCheck.h"
#include "tr101290_defs.h"
#include "TsPacket.h"
#include "config.h"
#include <map>
#include <vector>



using namespace std;


class CTrCore;

///@brief Demux and parse
class CDemux
{
// the structs define move to CDemux, avoid to redefine in comondefs.h in libeasyice of struct PROGRAM_INFO
typedef struct _PMTINFO
{
	_PMTINFO()
	{
		pmt_pid = 0;
		pcr_pid = 0;
		parsed = false;
	}
	int pmt_pid;
	int pcr_pid;
	dvbpsi_handle handle;
	bool parsed;
}PMTINFO;

//单个ES流结构
typedef struct _ES_INFO
{
	_ES_INFO()
	{
		llPrevPts = -1;
		llPrevPts_occ = -1;
	}
	int pid;
	long long llPrevPts;
	long long llPrevPts_occ;
	int stream_type;
}ES_INFO;

//demux后的每个节目的信息
typedef struct _PROGRAM_INFO
{
	_PROGRAM_INFO()
	{
		nPcrPid = -1;
		nPmtPid = -1;
		pCalcPcrN1 = NULL;
	}
	int nPcrPid;
	int nPmtPid;
	CCalcPcrN1* pCalcPcrN1;
	std::vector<ES_INFO> vecPayloadPid;
}PROGRAM_INFO;


public:
	CDemux(CTrCore* pParent);
	~CDemux();

	void AddPacket(uint8_t* pPacket);

	//PAT，PMT是否解析完毕
	bool IsDemuxFinish();

private:
	void Demux(uint8_t* pPacket);
	void ProcessPacket(uint8_t* pPacket);
	void UpdateClock(uint8_t* pPacket);

	static void DumpPAT(void* p_zero, dvbpsi_pat_t* p_pat);
	static void DumpPMT(void* p_zero, dvbpsi_pmt_t* p_pmt);

private:
	bool IsPmtPid(int pid);
	long long CheckOccTime(int pid,long long llCurTime);

	//check es pid err and pts err,return true if the pid is an es pid,otherwide return false
	bool CheckEsPid(int pid,long long llCurTime,CTsPacket& tsPacket);

	//check pcr error
	void CheckPCR(int pid,CTsPacket& tsPacket);

	//check LV3_UNREFERENCED_PID
	void CheckUnreferPid(int pid,long long llCurTime);

	void InitCrcCk();
private:
	dvbpsi_handle m_h_dvbpsi_pat;

	CTrCore* m_pParent;

	//PAT，PMT是否解析完毕
	bool m_bDemuxFinish;

	//选用第一个遇到的PCR作为系统时钟
	int m_nUsedPcrPid;

	//CCalcPcrN1 m_calcPcrN1;
	CPsiCheck *m_pPsiCk;

	long long m_llFirstPcr;
private:
	//解复用后信息
	vector<PROGRAM_INFO> m_vecDemuxInfoBuf;

	//program_num pmtinfo
    std::map<int,PMTINFO> m_mapPmtmInfo;

	//下标为PID，内容为时间
	long long* m_pOldOccurTime;

	//for LV3_UNREFERENCED_PID

	//map<pid,time>
    std::map<int,long long> m_mapUnReferPid;
	//下标为PID，内容为是否已知，已知为true
	bool *m_pKnownPid;

};





#endif


