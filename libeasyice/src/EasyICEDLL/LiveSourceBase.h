/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <string>
#include "StreamFilterBase.h"
#include "cudpobj.h"

using namespace std;



class CLiveSourceBase
{
public:
	//max 188*7
	typedef void (* ON_RECVDATA_CB)(void* pApp,BYTE* pData,int nLen);

	
public:
	CLiveSourceBase();
	virtual ~CLiveSourceBase(void);

	void SetRecvDataCB(ON_RECVDATA_CB pRecvCB,void* pApp);

	void SetParam(UDP_OBJ_PARAM_T param);

	//可有可无的过滤器。目前只支持一个.本类负责自动销毁过滤器
	void SetFilter(CStreamFilterBase *pFilter);

	//停止接收
	void Stop(bool bForce = false);

	//启动接收，成功返回 0 ，否则返回非零值
	virtual int Run() = 0;
	
	//接收线程函数
	virtual void WorkFun() = 0;

protected:
	void RunThread();
	static void* WorkThread(void* lpParam);

protected:
	bool m_bStop;
	UDP_OBJ_PARAM_T m_stParam;

	
	BYTE* m_pBuffer;
	int m_nBufferLen;

	//回掉函数及参数
	ON_RECVDATA_CB m_pRecvDataCB;
	void* m_pApp;

	CStreamFilterBase* m_pFilter;

public:
    string m_strRemoteAddress;
    int m_nRemotePort;
private:

	pthread_t m_hThread;
	
	
};
