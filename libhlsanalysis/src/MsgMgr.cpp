/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "MsgMgr.h"
#include <time.h>
#include "global.h"



// 类的静态成员变量要在类体外进行定义
//CMsgMgr* CMsgMgr::m_pStatic = NULL;


CMsgMgr::CMsgMgr(void) :m_hEvent("CMsgMgr")
{
	m_pApp = NULL;
	m_pfHlsReport = NULL;
	m_bActiveStop = false;

	m_bWorkThreadValid = false;
	pthread_mutex_init(&m_muxMsg,NULL);
	m_hEvent.Reset();
}

CMsgMgr::~CMsgMgr(void)
{
	pthread_mutex_destroy(&m_muxMsg);
}

void CMsgMgr::Init()
{
	m_bWorkThreadValid = (pthread_create(&m_hThread,NULL,WorkThread,this) == 0);
}

//CMsgMgr* CMsgMgr::GetInstancePtr()
//{
//	if (NULL == m_pStatic)
//	{
//		m_pStatic = new CMsgMgr();
//	}
//
//	return m_pStatic;
//}

void CMsgMgr::Destroy()
{
	//销毁线程
	Clear();

	m_bActiveStop = true;
	m_hEvent.Set();	
	//if (m_hThread != NULL)
	//{
	//	WaitForSingleObject(m_hThread,500);
	//	CloseHandle(m_hThread);
	//	m_hThread = NULL;
	//}

	if (m_bWorkThreadValid)
	{
		hls_analysis::Ei_MsgWaitForSingleObjects(&m_hThread,500);
	}
	
}

void CMsgMgr::SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp)
{
	m_pApp = pApp;
	m_pfHlsReport = pCB;
}



void* CMsgMgr::WorkThread(void*lpParam)
{
	CMsgMgr* lpthis = (CMsgMgr*)lpParam;

	lpthis->WorkFun();
	return 0;
}

void CMsgMgr::WorkFun()
{
	const int time_out_ms = 10;
	while (1)
	{
		if (m_bActiveStop)
		{
			return;
		}

		pthread_mutex_lock(&m_muxMsg);
		bool bEmpty = m_dqMsg.empty();
		pthread_mutex_unlock(&m_muxMsg);

		if (bEmpty)
		{
			m_hEvent.Wait(time_out_ms);
			continue;
		}

		pthread_mutex_lock(&m_muxMsg);
		HA_PARAM_T param = m_dqMsg.front();
		m_dqMsg.pop_front();
		pthread_mutex_unlock(&m_muxMsg);

		HlsSendMessage(param);
	}
}

void CMsgMgr::Clear()
{
	pthread_mutex_lock(&m_muxMsg);
	m_dqMsg.clear();
	pthread_mutex_unlock(&m_muxMsg);
}

void CMsgMgr::HlsPostMessageComm(HA_PARAM_T& param)
{
	pthread_mutex_lock(&m_muxMsg);
	param.llOccTime = time( NULL );
	m_dqMsg.push_back(param);
	pthread_mutex_unlock(&m_muxMsg);

	m_hEvent.Set();
}


void CMsgMgr::HlsSendMessage(const HA_PARAM_T& param)
{
	HLS_REPORT_PARAM_T hls_param;
	hls_param.emType = param.emType;
	hls_param.pApp = m_pApp;
	hls_param.llOccTime = param.llOccTime;

	switch(param.emType)
	{
	case EM_HRT_SQ_STARTPAT:
	case EM_HRT_SQ_VIDEOBOUNDARY:
	case EM_HRT_SQ_AUDIOBOUNDARY:
	case EM_HRT_SQ_CC:
	case EM_HRT_SQ_TC:
	case EM_HRT_SQ_ATLEASTONEIFRAME:
	case EM_HRT_SQ_STARTIFRAME:
		hls_param.pParam = (void*)param.stData.strVal.c_str();
		break;
	case EM_HRT_SYS_ANALYSISING:
	case EM_HRT_SYS_WAITTING_FIRST_SEGMENT:
	case EM_HRT_SYS_FINISH:
		hls_param.pParam = NULL;
		break;
	case EM_HRT_DIAGNOSIS:
	case EM_HRT_DIAGNOSIS_SPTS:
	case EM_HRT_DIAGNOSIS_AUDIOTYPE:
	case EM_HRT_DIAGNOSIS_VIDEOTYPE:
	case EM_HRT_DIAGNOSIS_HASNULLPKT:
	case EM_HRT_DIAGNOSIS_HASBFRAME:
		//hls_param.pParam = (void*)&param.stData.dia_msg;
		hls_param.pParam = (void*)param.stData.dia_msg.to_json().toStyledString().c_str();
		break;
    case EM_HRT_SEGMENT_PIDLIST:
	case EM_HRT_SEGMENT_MEDIAINFO:
		hls_param.pParam = (void*)param.stData.strVal.c_str();
		break;
	case EM_HRT_PROTOCOL_HLS_UNDERFLOW:
		hls_param.pParam = (void*)&param.stData.fVal;
		break;
	case EM_HRT_PROTOCOL_HTTP_DOWNLOADING:
		//hls_param.pParam = (void*)&param.stData.dling;
		hls_param.pParam = (void*)param.stData.dling.to_json().toStyledString().c_str();
		break;
	case EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD_COUNT:
		hls_param.pParam = (void*)&param.stData.llVal;
		break;
	case EM_HRT_PROTOCOL_HTTP_DOWNLOAD_HISTORY:
		//hls_param.pParam = (void*)&param.stData.dlHistory;
		hls_param.pParam = (void*)param.stData.dlHistory.to_json().toStyledString().c_str();
		break;
	case EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD:
	case EM_HRT_PROTOCOL_HLS_DOWNLOAD_ERRORS:
	case EM_HRT_PROTOCOL_HLS_M3U8:
	case EM_HRT_PROTOCOL_HTTP_HEAD_TS:
	case EM_HRT_PROTOCOL_HTTP_HEAD_M3U8:
	case EM_HRT_LAST:
		hls_param.pParam = (void*)param.stData.strVal.c_str();
		break;
	default:
		hls_param.pParam = NULL;
		break;
	}
	
	if (m_pfHlsReport != NULL)
	{
		m_pfHlsReport(hls_param);
	}
}

void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType)
{
	HA_PARAM_T param;
	param.emType = emType;
	HlsPostMessageComm(param);
}

//void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,int n)
//{
//	HA_PARAM_T param;
//	param.emType = emType;
//	param.stData.nVal = n;
//	HlsPostMessageComm(param);
//}

void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,long long llVal)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.llVal = llVal;
	HlsPostMessageComm(param);
}

void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,double fVal)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.fVal = fVal;
	HlsPostMessageComm(param);
}


void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const string& strVal)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.strVal = strVal;
	HlsPostMessageComm(param);
}

//void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const wstring& strVal)
//{
//	HA_PARAM_T param;
//	param.emType = emType;
//	param.stData.wstrVal = strVal;
//	HlsPostMessageComm(param);
//}

void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const HA_PARAM_DIAGNOSIS_MSG_T& diaMsg)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.dia_msg = diaMsg;
	HlsPostMessageComm(param);
}

void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const HLS_REPORT_PARAM_DOWNLOADING_T& dling)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.dling = dling;
	HlsPostMessageComm(param);
}


void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T& dlHistory)
{
	HA_PARAM_T param;
	param.emType = emType;
	param.stData.dlHistory = dlHistory;
	HlsPostMessageComm(param);
}


void CMsgMgr::HlsPostMessage(HLS_REPORT_TYPE_T emType,const string& level,const string& layer,const string& description)
{
	HA_PARAM_DIAGNOSIS_MSG_T diaMsg;
	diaMsg.level = level;
	diaMsg.layer = layer;
	diaMsg.description = description;


	HA_PARAM_T param;
	param.emType = emType;
	param.stData.dia_msg = diaMsg;
	HlsPostMessageComm(param);
}


