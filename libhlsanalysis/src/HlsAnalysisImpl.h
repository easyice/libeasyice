/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "hls_define.h"
#include <iostream>
#include <pthread.h>
#include "sdkdefs.h"

using namespace std;

class CMsgMgr;
class CDownloader;
class CHlsAnalysisImpl
{
public:
	CHlsAnalysisImpl(void);
	~CHlsAnalysisImpl(void);

	int OpenMRL(EASYICE* handle);


	bool Stop(bool bForce = false); 

	bool GetBufferDuration(double& duration,long long& llCurTime);

	void SetProxy(const char* strProxy);

	int StartRecord(const char* strFileName);


	void StopRecord();

	void SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param);
    void SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp);
private:
	//处理线程函数
	static void* WorkThread(void* lpParam);
	void WorkFun();
	

private:
	//处理线程
	pthread_t  m_hThread;
    bool m_bWorkThreadValid;

	CDownloader* m_pDownLoader;
	string m_strUrl;
    CMsgMgr* m_pMsgMgr;

};
