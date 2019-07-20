/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// HlsAnalysis.cpp : 定义 DLL 应用程序的入口点。
//

#include "HlsAnalysis.h"
//#include "commondefs.h"
#include "HlsAnalysisImpl.h"
#include "global.h"


//extern HWND g_msgWnd;


#define SDK_RELEASE

static bool b_RunErrorMode = false;
// 这是导出变量的一个示例
int nHlsAnalysis=0;



// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 HlsAnalysis.h
CHlsAnalysis::CHlsAnalysis()
{
#ifdef SDK_RELEASE
	nHlsAnalysis = LIB_HLS_ANALYSIS_CALL_PASSWD;
#endif
	m_pHlsAnalysisImpl = new CHlsAnalysisImpl();
}

CHlsAnalysis::~CHlsAnalysis()
{
	delete m_pHlsAnalysisImpl;
}

int CHlsAnalysis::OpenMRL(EASYICE* handle)
{
	return m_pHlsAnalysisImpl->OpenMRL(handle);
}


bool CHlsAnalysis::Stop(bool bForce)
{
	return m_pHlsAnalysisImpl->Stop(bForce);
}


int CHlsAnalysis::StartRecord(const char* strFileName)
{
	return m_pHlsAnalysisImpl->StartRecord(strFileName);
}


void CHlsAnalysis::StopRecord()
{
	return m_pHlsAnalysisImpl->StopRecord();
}

void CHlsAnalysis::SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param)
{
	return m_pHlsAnalysisImpl->SetRecord(rc_param);
}

bool CHlsAnalysis::GetBufferDuration(double& duration,long long& llCurTime)
{
	if (b_RunErrorMode)
	{
		return true;
	}
	return m_pHlsAnalysisImpl->GetBufferDuration(duration,llCurTime);
}


void CHlsAnalysis::SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp)
{
	if (b_RunErrorMode)
	{
		return;
	}
    return m_pHlsAnalysisImpl->SetReportCB(pCB,pApp);
}

void CHlsAnalysis::SetProxy(const char* strProxy)
{
	return m_pHlsAnalysisImpl->SetProxy(strProxy);
}

