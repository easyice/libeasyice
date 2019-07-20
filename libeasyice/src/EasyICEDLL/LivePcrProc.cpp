/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "LivePcrProc.h"
#include "tspacket.h"

#define MAX_PID 8192

//检测结果缓冲大小。超过后清空.100大约支持5秒钟的数据
#define MAX_BUFFER_SIZE 100

CLivePcrProc::CLivePcrProc(void)
{
	pthread_mutex_init(&m_mutexRate,NULL);
	m_nRecvedBytes = 0;
	m_llPrevSysClock = 0;
	m_nTsLength = 188;

	m_pCalcTsRateIt = NULL; //default to 1s
}

CLivePcrProc::~CLivePcrProc(void)
{
	map<int,PROGRAM_PCR_INFO_T*>::iterator it = m_mapProgramPcrInfo.begin();
	for (; it != m_mapProgramPcrInfo.end(); ++it)
	{
		delete it->second;
	}

	pthread_mutex_destroy(&m_mutexRate);
}

void CLivePcrProc::ProcessBuffer(BYTE* pData,int nLen,long long llTime)
{
	//ts rate
	m_nRecvedBytes += nLen;

	if (m_llPrevSysClock == 0)
	{
		m_llPrevSysClock = llTime;
	}
	else
	{
		long long sys_duration_usec = llTime - m_llPrevSysClock;
		sys_duration_usec /= 1000;
		if (sys_duration_usec >= *m_pCalcTsRateIt)
		{
			RATE_INFO_T rateInfo;
			rateInfo.llTime = llTime;
			rateInfo.fRate = (long long)m_nRecvedBytes*8*1000 / sys_duration_usec;

			//add to result buffer
			pthread_mutex_lock(&m_mutexRate);
			if (m_lstRate.size() > MAX_BUFFER_SIZE)
			{
				m_lstRate.clear();
			}
			m_lstRate.push_back(rateInfo);
			pthread_mutex_unlock(&m_mutexRate);

			m_nRecvedBytes = 0;
			m_llPrevSysClock = llTime;
		}

	}
	
	//pcr...
	for (int i = 0; i < nLen; i+= m_nTsLength)
	{
		CTsPacket tsPacket;
		if (!tsPacket.SetPacket(pData+i))
		{
			continue;
		}

		int pid = tsPacket.Get_PID();
		map<int,PROGRAM_PCR_INFO_T*>::iterator it = m_mapProgramPcrInfo.begin();
		for (; it != m_mapProgramPcrInfo.end(); ++it)
		{
			if (pid == it->first && tsPacket.Get_PCR_flag())
			{
				long long pcr = tsPacket.Get_PCR();

				//pcr oj
				PCR_INFO_T pi;
				pi.llPcr_Oj = it->second->pcrOj.RecvPcr(pcr,llTime) / 1000;

				//pcr ac
				long long pcr_calc = it->second->pcrAc.GetPcr();
				if (pcr_calc > 0)
					pi.llPcr_Ac = (pcr_calc - pcr) / 27;
				else
					pi.llPcr_Ac = -1;


				//pcr it
				if (it->second->llpcrPrev < 0)
				{
					pi.llPcr_interval = -1;
				}
				else
				{
					pi.llPcr_interval = (pcr - it->second->llpcrPrev) / 27000;
				}

				//recv time
				pi.llTime = llTime;


				//update
				it->second->llpcrPrev = pcr;
				it->second->pcrAc.AddPcrPacket(pcr);

				//add to buffer
				pthread_mutex_lock(&it->second->mutex);
				if (it->second->lstPcrInfo.size() > MAX_BUFFER_SIZE)
				{
					it->second->lstPcrInfo.clear();
				}
				it->second->lstPcrInfo.push_back(pi);
				pthread_mutex_unlock(&it->second->mutex);

			} // !if Get_PCR_flag
			else
			{
				it->second->pcrAc.AddPayloadPacket();
			}
		}// !for it

	}// !for i

	
}

void CLivePcrProc::SetTsLength(int nLen)
{
	m_nTsLength = nLen;
}

void CLivePcrProc::SetCalcTsRateIntervalTime(const int* pTime)
{
	m_pCalcTsRateIt = pTime;
}

void CLivePcrProc::AddPcrPid(int pid)
{
	if (pid >0x1FFF)
	{
		return;
	}

	PROGRAM_PCR_INFO_T* p = new PROGRAM_PCR_INFO_T();
	std::pair< std::map< int,PROGRAM_PCR_INFO_T* >::iterator,bool > ret;
	ret = m_mapProgramPcrInfo.insert(map<int,PROGRAM_PCR_INFO_T*>::value_type(pid,p) );
	if (!ret.second)
	{
		delete p;
	}
}

LST_RATE_INFO_T* CLivePcrProc::LockGetRate()
{
	pthread_mutex_lock(&m_mutexRate);
	return &m_lstRate;
}

void CLivePcrProc::UnlockRate()
{
	m_lstRate.clear();
	pthread_mutex_unlock(&m_mutexRate);
}

LST_PCR_INFO_T* CLivePcrProc::LockGetPcrInfo(int pcr_pid)
{
	map<int,PROGRAM_PCR_INFO_T*>::iterator it = m_mapProgramPcrInfo.find(pcr_pid);
	if (it == m_mapProgramPcrInfo.end())
	{
		return NULL;
	}

	pthread_mutex_lock(&it->second->mutex);
	return &it->second->lstPcrInfo;
}

void CLivePcrProc::UnlockPcrInfo(int pcr_pid)
{
	map<int,PROGRAM_PCR_INFO_T*>::iterator it = m_mapProgramPcrInfo.find(pcr_pid);
	if (it == m_mapProgramPcrInfo.end())
	{
		return;
	}

	it->second->lstPcrInfo.clear();
	pthread_mutex_unlock(&it->second->mutex);
}

//const map<int,CLivePcrProc::PROGRAM_PCR_INFO_T*>& CLivePcrProc::LockGetPcrInfoAll()
//{
//    pthread_mutex_lock(&it->second->mutex);
//    return m_mapProgramPcrInfo;
//}
//
//void CLivePcrProc::UnlockPcrInfoAll()
//{
//    //不可以直接清空 map
//	map<int,PROGRAM_PCR_INFO_T*>::iterator it = m_mapProgramPcrInfo.begin();
//	for (;it != m_mapProgramPcrInfo.end();++it)
//	{
//        it->second->lstPcrInfo.clear();
//	}
//
//    pthread_mutex_unlock(&it->second->mutex); 
//}
//


