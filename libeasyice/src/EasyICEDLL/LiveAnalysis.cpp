/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "LiveAnalysis.h"
#include "LiveAnalysisImpl.h"


CLiveAnalysis::CLiveAnalysis(void)
{
	m_pLiveAnalysisImpl = new CLiveAnalysisImpl();
}

CLiveAnalysis::~CLiveAnalysis(void)
{
	delete m_pLiveAnalysisImpl;
}


int CLiveAnalysis::OpenMRL(EASYICE* handle)
{
	return m_pLiveAnalysisImpl->OpenMRL(handle);
}


bool CLiveAnalysis::Stop(bool bForce)
{
	return m_pLiveAnalysisImpl->Stop(bForce);
}

ALL_PROGRAM_BRIEF* CLiveAnalysis::GetAllProgramBrief()
{
	return m_pLiveAnalysisImpl->GetAllProgramBrief();
}


LST_RATE_INFO_T* CLiveAnalysis::LockGetRate()
{
	return m_pLiveAnalysisImpl->LockGetRate();
}

void CLiveAnalysis::UnlockRate()
{
	return m_pLiveAnalysisImpl->UnlockRate();
}

LST_PCR_INFO_T* CLiveAnalysis::LockGetPcrInfo(int pcr_pid)
{
	return m_pLiveAnalysisImpl->LockGetPcrInfo(pcr_pid);
}

void CLiveAnalysis::UnlockPcrInfo(int pcr_pid)
{
	return m_pLiveAnalysisImpl->UnlockPcrInfo(pcr_pid);
}


//void CLiveAnalysis::SetCalcTsRateIntervalTime(int nTime)
//{
//	return m_pLiveAnalysisImpl->SetCalcTsRateIntervalTime(nTime);
//}

int CLiveAnalysis::StartRecord(const char* strFileName)
{
	return m_pLiveAnalysisImpl->StartRecord(strFileName);
}


void CLiveAnalysis::StopRecord()
{
	m_pLiveAnalysisImpl->StopRecord();
}

