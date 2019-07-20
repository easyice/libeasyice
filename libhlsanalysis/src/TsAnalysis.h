/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <iostream>

#include "libtr101290.h"

using namespace std;

class CMsgMgr;
class Clibtr101290;
class CMpegDec;
class CH264Dec;
class CTsAnalysis
{
	typedef struct _SEGMENT_MI_T
	{
		_SEGMENT_MI_T()
		{
			pmt_pid = -1;
			video_pid = -1;
			audio_pid = -1;
			is_avc = true;
		}
		int pmt_pid;
		int video_pid;
		int audio_pid;
		bool is_avc;
	}SEGMENT_MI_T;

public:
	CTsAnalysis(CMsgMgr* pMsgMgr);
	~CTsAnalysis(void);
public:
	void OnRecvSegment(BYTE* pData,int nLen,const string& url);

private:
	bool DumpMediaInfo(BYTE* pData,int nLen);
	int TryToSync(BYTE* pData,int nLength,int& nSycByte);

	bool IsStartWithPatPmt(BYTE* pData,int nLen,const string& url);
	void CheckAudioBoundary(BYTE* pData,int nLen,const string& url);
	void CheckVideoBoundary(BYTE* pData,int nLen,const string& url);
	void CheckIFrame(BYTE* pData,int nLen,const string& url);

	static void OnTrReport(REPORT_PARAM_T param);
private:
	bool m_bInited;

	//是否已报告过，诊断信息，整个分析过程只报告一次
	bool m_bReportedNullPkt;

	//每个分片只报告一次
	bool m_bReportedCCError;
	bool m_bReportedTCError;

	SEGMENT_MI_T m_stSegMi;
	
	//解复用器
	CH264Dec* m_pAvcParser;

	CMpegDec *m_pMpegdec;

	string m_strUrl;

    Clibtr101290* m_pTrcore;
    CMsgMgr* m_pMsgMgr;
};
