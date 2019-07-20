/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "hls_define.h"
#include <deque>
#include "EiLog.h"
#include "csavefile.h"
#include <pthread.h>
#include "zevent.h"

using namespace std;


class CAsyncSaveFile
{
	typedef struct _RC_TS_BUF_T
	{
		_RC_TS_BUF_T()
		{
			pData = NULL;
			nLen = 0;
		}
		string url;
		BYTE* pData;
		int nLen;
	}RC_TS_BUF_T;

public:
	CAsyncSaveFile(void);
	~CAsyncSaveFile(void);
	void Start(const HLS_RECORD_INIT_PARAM_T& param);
	void Stop();

public:
    void OnRecvM3u8(const string& m3u8);
    void OnRecvTs(const string& url,const BYTE* pData,int nLen);
    //bool IsExist(const string& url);

	
private:
	 static void* WorkThread(void* pParam);
     void WorkFun();
public:
	CSaveFile* m_pSaveFile;
private:
	
	pthread_t m_thread;
    bool m_bWorkThreadValid;
	ZEvent m_eventOnTsAdd;
	pthread_mutex_t m_mutex;

	deque<RC_TS_BUF_T> m_dqTsBuffer;

	bool m_bStop;
};
