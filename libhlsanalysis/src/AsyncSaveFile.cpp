/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "commondefs.h"
#include "AsyncSaveFile.h"


CAsyncSaveFile::CAsyncSaveFile(void) : m_eventOnTsAdd("HlsSaveFile")
{
	m_pSaveFile = new CSaveFile();
    m_eventOnTsAdd.Reset();
	pthread_mutex_init(&m_mutex,NULL);
	m_bStop = false;
    m_bWorkThreadValid = false;
}

CAsyncSaveFile::~CAsyncSaveFile(void)
{
	delete m_pSaveFile;
	pthread_mutex_destroy(&m_mutex);
}

void CAsyncSaveFile::Start(const HLS_RECORD_INIT_PARAM_T& param)
{
	m_pSaveFile->SetParam(param);
	m_pSaveFile->Init();
    m_bWorkThreadValid = (pthread_create(&m_thread,NULL,WorkThread,this) == 0);
}

void CAsyncSaveFile::Stop()
{
	m_bStop = true;

	//for wake up WorkThread
    m_eventOnTsAdd.Set();

	if (m_bWorkThreadValid)
	{
        pthread_join(m_thread,NULL);
	}
}

void CAsyncSaveFile::OnRecvM3u8(const string& m3u8)
{
	m_pSaveFile->OnRecvM3u8(m3u8);
}

void CAsyncSaveFile::OnRecvTs(const string& url,const BYTE* pData,int nLen)
{
	RC_TS_BUF_T item;
	item.url = url;
	item.nLen = nLen;
	item.pData = new BYTE[nLen];
	memcpy(item.pData,pData,nLen);

	pthread_mutex_lock(&m_mutex);
	m_dqTsBuffer.push_back(item);
	pthread_mutex_unlock(&m_mutex);

    m_eventOnTsAdd.Set();
}


void* CAsyncSaveFile::WorkThread(void* pParam)
{
	CAsyncSaveFile* lpthis = (CAsyncSaveFile*) pParam;
	lpthis->WorkFun();

	return 0;
}

void CAsyncSaveFile::WorkFun()
{
	while(1)
	{
		if (m_bStop)
		{
			break;
		}

		pthread_mutex_lock(&m_mutex);
		bool bempty = m_dqTsBuffer.empty();
		pthread_mutex_unlock(&m_mutex);

		if (bempty)
		{
            m_eventOnTsAdd.Wait(100);
			continue;
		}

		
		pthread_mutex_lock(&m_mutex);
		deque<RC_TS_BUF_T>::iterator it = m_dqTsBuffer.begin();
		pthread_mutex_unlock(&m_mutex);

		m_pSaveFile->OnRecvTs(it->url,it->pData,it->nLen);
		delete [] it->pData;
		
		pthread_mutex_lock(&m_mutex);
		m_dqTsBuffer.pop_front();
		pthread_mutex_unlock(&m_mutex);


	}
}

