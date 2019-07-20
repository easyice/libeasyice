/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "HlsAnalysisImpl.h"
//#include "../EasyICEDLL/commonexport.h"
#include "MsgMgr.h"
#include "cdownloader.h"
//#include "../EasyICEDLL/tables/CAnalyzeTable.h"
#include "global.h"


CHlsAnalysisImpl::CHlsAnalysisImpl(void)
{
    m_pMsgMgr = new CMsgMgr();
    m_pMsgMgr->Init();

    m_bWorkThreadValid = false;
	m_pDownLoader = new CDownloader(m_pMsgMgr);
}

CHlsAnalysisImpl::~CHlsAnalysisImpl(void)
{

	Stop();

	delete m_pDownLoader;
	
    m_pMsgMgr->Destroy();
    delete m_pMsgMgr;
}

void CHlsAnalysisImpl::SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp)
{
    m_pMsgMgr->SetReportCB(pCB,pApp);
}

int CHlsAnalysisImpl::OpenMRL(EASYICE* handle)
{

	m_strUrl = handle->mrl;
    m_bWorkThreadValid = (pthread_create(&m_hThread,NULL,WorkThread,this) == 0);
	return 0;
}


bool CHlsAnalysisImpl::Stop(bool bForce)
{
	m_pDownLoader->Stop();

	if (m_bWorkThreadValid)
	{
        pthread_join(m_hThread,NULL);
	}

	m_pMsgMgr->Clear();
	//tables::CDescriptor::GetInstancePtr()->Destroy();
	return true;
}




int CHlsAnalysisImpl::StartRecord(const char* strFileName)
{
	return 0;
}


void CHlsAnalysisImpl::StopRecord()
{
}

void CHlsAnalysisImpl::SetProxy(const char* strProxy)
{
	m_pDownLoader->SetProxy(strProxy);
}

void CHlsAnalysisImpl::SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param)
{
	m_pDownLoader->SetRecord(rc_param);
}

void* CHlsAnalysisImpl::WorkThread(void* lpParam)
{
	CHlsAnalysisImpl* lpthis = (CHlsAnalysisImpl*)lpParam;
	lpthis->WorkFun();
	return 0;
}

void CHlsAnalysisImpl::WorkFun()
{
	m_pDownLoader->Run(m_strUrl);
}

bool CHlsAnalysisImpl::GetBufferDuration(double& duration,long long& llCurTime)
{
	return m_pDownLoader->GetBufferDuration(duration,llCurTime);
}

