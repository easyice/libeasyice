/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "LiveSourceBase.h"

#define BUFFER_SZIE (188*128)

CLiveSourceBase::CLiveSourceBase(void)
{
	m_bStop = false;
	m_pBuffer = new BYTE[BUFFER_SZIE];
	m_nBufferLen = BUFFER_SZIE;
	m_pFilter = NULL;
    m_nRemotePort = -1;
}

CLiveSourceBase::~CLiveSourceBase(void)
{
	delete [] m_pBuffer;
	delete m_pFilter;
}

void CLiveSourceBase::SetRecvDataCB(ON_RECVDATA_CB pRecvCB,void* pApp)
{
	m_pRecvDataCB = pRecvCB;
	m_pApp = pApp;
}

void CLiveSourceBase::SetParam(UDP_OBJ_PARAM_T param)
{
	m_stParam = param;
}

void CLiveSourceBase::RunThread()
{
	pthread_create(&m_hThread,NULL,WorkThread,this);
}

void* CLiveSourceBase::WorkThread(void* lpParam)
{
	CLiveSourceBase* pthis = (CLiveSourceBase*)lpParam;
	pthis->WorkFun();
	return 0;
}

void CLiveSourceBase::Stop(bool bForce)
{
	if (bForce)
	{
		//dwWait = 100;
	}

	m_bStop = true;
	pthread_join(m_hThread,NULL);
}

void CLiveSourceBase::SetFilter(CStreamFilterBase *pFilter)
{
	m_pFilter = pFilter;
}

