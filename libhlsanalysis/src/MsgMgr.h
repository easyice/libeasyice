/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "hls_define.h"
#include "EiLog.h"
#include <deque>
#include <string>
#include <pthread.h>
#include "zevent.h"
#include "hls_type.h"

using namespace std;

class CMsgMgr
{
	

	typedef struct _HA_PARAM_DATA_T
	{
		//int nVal;
		long long llVal;
		double fVal;
		string strVal;
		//wstring wstrVal;
		HA_PARAM_DIAGNOSIS_MSG_T dia_msg;
		HLS_REPORT_PARAM_DOWNLOADING_T dling;
		HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T dlHistory;
	}HA_PARAM_DATA_T;


	//队列中的参数结构
	typedef struct _HA_PARAM_T
	{
		_HA_PARAM_T()
		{
			llOccTime = 0;
		}
		long long llOccTime;//second
		HLS_REPORT_TYPE_T emType;
		HA_PARAM_DATA_T stData;
	}HA_PARAM_T;


public:
	CMsgMgr(void);
	~CMsgMgr(void);

	//全局只初始化一次
	void Destroy();
	void Init();

	void SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp);

	void Clear();
	




	//将事件放入队列
	void HlsPostMessage(HLS_REPORT_TYPE_T emType);
	//void HlsPostMessage(HLS_REPORT_TYPE_T emType,int n);
	void HlsPostMessage(HLS_REPORT_TYPE_T emType,long long llVal);
	void HlsPostMessage(HLS_REPORT_TYPE_T emType,double fVal);
	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const string& strVal);
//	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const wstring& strVal);

	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const HLS_REPORT_PARAM_DOWNLOADING_T& dling);
	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T& dlHistory);

	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const HA_PARAM_DIAGNOSIS_MSG_T& diaMsg);
	void HlsPostMessage(HLS_REPORT_TYPE_T emType,const string& level,const string& layer,const string& description);


private:
	static void* WorkThread(void* lpParam);
	void WorkFun();

	//本模块对外事件回调，等待回调函数处理完
	void HlsSendMessage(const HA_PARAM_T& param);

	void HlsPostMessageComm( HA_PARAM_T& param);
private:


	pthread_mutex_t m_muxMsg;

	void* m_pApp;
	PF_HLS_REPORT_CB m_pfHlsReport;

	std::deque<HA_PARAM_T> m_dqMsg;

	pthread_t m_hThread;
	bool m_bWorkThreadValid;

	//表示队列中是否有事件
	ZEvent  m_hEvent;

	bool m_bActiveStop;
};
