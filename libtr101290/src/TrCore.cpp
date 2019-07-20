/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "TrCore.h"
#include "Demux.h"
#include "tspacket.h"
#include "global.h"
#include "csysclock.h"
#include <string.h>

using namespace tr101290;


extern int nlibtr101290;

#define BUFFER_SIZE (188*20)


CTrCore::CTrCore(void)
{
	m_pEnable = new bool[LV3_DATA_DELAY_ERROR+1];
	memset(m_pEnable,true,LV3_DATA_DELAY_ERROR+1);

	m_pBuffer = new BYTE[BUFFER_SIZE];
	memset(m_pBuffer,0,BUFFER_SIZE);

	m_pCC = new unsigned long long[8192];
	for (int i = 0; i < 8192; i++)
	{
		m_pCC[i] = -1;
	}
	
	m_nBufferPos = 0;
	m_bSynced = false;
	//m_llOffset = 0;
	//g_pllOffset = &m_llOffset;
	m_bPrevPktSync = true;

	m_pDemuxer = new CDemux(this);
	m_pSysClock = new CSysClock();
}

CTrCore::~CTrCore(void)
{
	delete [] m_pEnable;


	delete [] m_pBuffer;

	delete [] m_pCC;

	delete m_pDemuxer;

	delete m_pSysClock;

}



void CTrCore::SetStartOffset(long long llOffset)
{
	m_llOffset = llOffset;
}

void CTrCore::SetReportCB(pfReportCB pCB,void* pApp)
{
	m_pfReportCB = pCB;
	m_pApp = pApp;
}

void CTrCore::SetEnable(bool *p)
{
	memcpy(m_pEnable,p,LV3_DATA_DELAY_ERROR+1);
}




//void CTrCore::Report(int level,ERROR_NAME_T errName,int pid,long long llVal,double fVal)
//{
//	Report(level,errName,m_llOffset,pid,llVal,fVal);
//}
//
//void CTrCore::Report(int level,ERROR_NAME_T errName,long long llOffset,int pid,long long llVal,double fVal)
//{
//	REPORT_PARAM_T param;
//	param.level = level;
//	param.errName = errName;
//	param.llOffset = llOffset;
//
//	param.pApp = m_pApp;
//	param.pid = pid;
//	param.llVal = llVal;
//	param.fVal = fVal;
//
//	m_pReportCB(param);
//}

void CTrCore::SetTsLen(int nLen)
{
	m_nTslen = nLen;
}

int CTrCore::TryToReSync()
{
	int loop;
	if (m_nBufferPos < m_nTslen*5)
		return -1;
	else if (m_nBufferPos == m_nTslen*5)
		loop = 1;
	else
		loop = m_nBufferPos - m_nTslen*5;

	for (int i = 0; i < loop; i++)
	{
		if(m_pBuffer[i]==0x47&&m_pBuffer[i+m_nTslen]==0x47&&m_pBuffer[i+m_nTslen*2]==0x47
			&&m_pBuffer[i+m_nTslen*3]==0x47&&m_pBuffer[i+m_nTslen*4]==0x47)
		{
			if (i > 0)
			{
				memmove(m_pBuffer,m_pBuffer+i,m_nBufferPos-i);
				m_nBufferPos -= i;
				m_llOffset += i;
			}
			
			return 1;
		}

	}
	return -1;
}

void CTrCore::AddPacket(BYTE* pPacket)
{
	if (m_nBufferPos + m_nTslen >= BUFFER_SIZE)
	{
		memmove(m_pBuffer,m_pBuffer+m_nBufferPos-m_nTslen*5,m_nTslen*5);
		m_nBufferPos = m_nTslen*5;
	}

	memcpy(m_pBuffer+m_nBufferPos,pPacket,m_nTslen);
	m_nBufferPos += m_nTslen;


	if (!m_bSynced)
	{
		if (m_nBufferPos >= m_nTslen*5 && TryToReSync() > 0)
		{
			m_bSynced = true;

			int usedlen = 0;

			
			//for (int i= 0; i < m_nBufferPos; i+= m_nTslen)
			for (int i= 0; i < m_nBufferPos && i+m_nTslen <= m_nBufferPos; i+= m_nTslen)
			{
				if  (*(m_pBuffer+i) != 0x47)//防止剩余缓冲中5个ts包之后有同步字节错的，会死掉
				{
					break;
				}
				ProcessPacket(m_pBuffer+i);
				m_llOffset+=m_nTslen;
				usedlen += m_nTslen;
			}
			if (usedlen > m_nBufferPos)//很重要的处理，不判断可能会越界
			{
				usedlen -= m_nTslen;
			}
			m_nBufferPos -= usedlen;
			if (m_nBufferPos > 0)
			{
				memmove(m_pBuffer,m_pBuffer+usedlen,m_nBufferPos);
			}
			m_bPrevPktSync = true;
		}

		return;
	}

	
	//已同步，缓冲中的数据为一个或一个半TS包

	if (!m_bPrevPktSync && *(m_pBuffer+m_nTslen) != 0x47)		//上一个TS产生同步字节错误，当前TS也产生同步错误
	{
		m_bSynced = false;
		
		//sync lost
		Report(1,LV1_TS_SYNC_LOST,m_llOffset,-1,-1,-1);
		return;
	}
	else if (!m_bPrevPktSync)									//上一个TS产生同步字节错误，当前TS没产生同步错误
	{
		m_bPrevPktSync = true;
		m_llOffset+=m_nTslen;
		m_nBufferPos -= m_nTslen;
		if (m_nBufferPos > 0)
		{
			memmove(m_pBuffer,m_pBuffer+m_nTslen,m_nBufferPos);
		}
	}

	//执行处理
	if (*m_pBuffer != 0x47)
	{
		m_bPrevPktSync = false;

		//sync byte error
		Report(1,LV1_SYNC_BYTE_ERROR,m_llOffset,-1,-1,-1);
	}
	else
	{
		ProcessPacket(m_pBuffer);
		m_llOffset+=m_nTslen;
		m_nBufferPos -= m_nTslen;
		if (m_nBufferPos > 0)
		{
			memmove(m_pBuffer,m_pBuffer+m_nTslen,m_nBufferPos);
		}
	}

	
}

void CTrCore::ProcessPacket(BYTE* pPacket)
{
	CTsPacket tsPacket;
	tsPacket.SetPacket(pPacket);
	int pid = tsPacket.Get_PID();
	
	//check cc
	bool bcontinue = true;
	int cc = tsPacket.Get_continuity_counter();

	if (pid != 0x1FFF && m_pCC[pid] >= 0)
	{
		if ((m_pCC[pid]+1) % 0x10 != cc)	bcontinue = false;
	}
	if (pid != 0x1FFF  && m_pCC[pid] >= 0 && (tsPacket.Get_adaptation_field_control() & 0x1) == 0)
	{
		if ((m_pCC[pid]) % 0x10 == cc)	bcontinue = true;
		else	bcontinue = false;
	}
	BYTE afLen;
	BYTE* pAf = tsPacket.Get_adaptation_field(afLen);
	if (pAf != NULL)
	{
		if (tsPacket.Get_discontinuity_indicator(pAf)) bcontinue = true;
	}
	if (!bcontinue)
	{
		Report(1,LV1_CC_ERROR,m_llOffset,pid,-1,-1);
	}
	m_pCC[pid] = cc;


	//LV2_TRANSPORT_ERROR
	if (tsPacket.Get_transport_error_indicator())
	{
		Report(1,LV2_TRANSPORT_ERROR,m_llOffset,pid,-1,-1);
	}

	if (nlibtr101290 != CALL_PASSWD && (pid % 2 == 0))
	{
		if (SDK_CODE==NULL)
		{
		}
		return;
	}

	//other
	m_pDemuxer->AddPacket(pPacket);
}

void CTrCore::Report(int level,ERROR_NAME_T errName,long long llOffset,int pid,long long llVal,double fVal)
{
	REPORT_PARAM_T param;
	param.level = level;
	param.errName = errName;
	param.llOffset = llOffset;

	param.pApp = m_pApp;
	param.pid = pid;
	param.llVal = llVal;
	param.fVal = fVal;

	m_pfReportCB(param);
}


void CTrCore::Report(int level,ERROR_NAME_T errName,int pid,long long llVal,double fVal)
{
	Report(level,errName,m_llOffset,pid,llVal,fVal);
}

bool CTrCore::IsDemuxFinish()
{
	return m_pDemuxer->IsDemuxFinish();
}

