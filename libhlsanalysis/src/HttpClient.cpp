/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "HttpClient.h"
#include "DlBuffer.h"
//#include "../EasyICEDLL/commonexport.h"
#include "hls_define.h"
#include "MsgMgr.h"
#include "utils.h"
#include <unistd.h>





#define CONN_TIME_OUT 30


//多长时间没有收到数据做超时处理
#define MAX_TIME_OUT 30*1000



//curl_multi_fdset有问题，获取的maxfd为-1，官方要求重试，这里设置重试超时
#define ERROR_TIME_OUT 2*1000


static struct timeval tvnow(void)
{
	/*
	** time() returns the value of time in seconds since the Epoch.
	*/
	struct timeval now;
	now.tv_sec = (long)time(NULL);
	now.tv_usec = 0;
	return now;
}

static long long tvdiff(struct timeval newer, struct timeval older)
{
	return (newer.tv_sec-older.tv_sec)*1000+
		(newer.tv_usec-older.tv_usec)/1000;
}

CHttpClient::CHttpClient(CMsgMgr* pMsgMgr)
{
	m_bDebug = false;
	m_pDlBuffer = new CDlBuffer();
	m_bActiveStop = false;
	m_nFileSize = 0;
    m_pMsgMgr = pMsgMgr;
}

CHttpClient::~CHttpClient(void)
{
	delete m_pDlBuffer;
	
}

void CHttpClient::SetDebug(bool bDebug)
{
	m_bDebug = bDebug;
}

static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *)
{
	if(itype == CURLINFO_TEXT)
	{
		//printf("[TEXT]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if(itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}


CURLcode CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

CURLcode CHttpClient::Get(const std::string & strUrl, std::string & strResponse)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONN_TIME_OUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME_OUT);
	if (!m_strProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, m_strProxy.c_str());// 代理
	}

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		printf("curl_easy_perform:%d\n",res);
	}
	return res;
}

CURLcode CHttpClient::Get(const std::string & strUrl, BYTE** pData,int& nLen)
{
	m_pDlBuffer->ResetData();

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONN_TIME_OUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME_OUT);
	if (!m_strProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, m_strProxy.c_str());// 代理
	}

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (m_pDlBuffer->m_bError)
	{
		nLen = 0;
		ei_log(LV_ERROR,"hls","download buffer error");
		return res;
	}
	*pData = m_pDlBuffer->m_pBuffer;
	nLen = m_pDlBuffer->m_nDataLen;

	if (res != CURLE_OK)
	{
		printf("curl_easy_perform:%d\n",res);
	}

	return res;
}

CURLcode CHttpClient::Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		//缺省情况就是PEM，所以无需设置，另外支持DER
		//curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM");
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

CURLcode CHttpClient::Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath)
{
	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONN_TIME_OUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME_OUT);

	if (!m_strProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, m_strProxy.c_str());// 代理
	}

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		printf("curl_easy_perform:%d\n",res);
	}

	return res;
}

CURLcode CHttpClient::Gets(const std::string & strUrl, BYTE** pData,int& nLen, const char * pCaPath )
{
	m_pDlBuffer->ResetData();

	CURLcode res;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if(NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONN_TIME_OUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME_OUT);

	if (!m_strProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, m_strProxy.c_str());// 代理
	}

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (m_pDlBuffer->m_bError)
	{
		nLen = 0;
		ei_log(LV_ERROR,"hls","download buffer error");
		return res;
	}
	*pData = m_pDlBuffer->m_pBuffer;
	nLen = m_pDlBuffer->m_nDataLen;

	if (res != CURLE_OK)
	{
		printf("curl_easy_perform:%d\n",res);
	}
	return res;
}


size_t CHttpClient::OnRecvData(void *buffer, size_t size, size_t nmemb, void *stream)
{
	CHttpClient *lpthis = (CHttpClient*)stream;

	lpthis->m_pDlBuffer->Append((BYTE*)buffer,size*nmemb);
	return nmemb;
}

size_t CHttpClient::OnRecvString(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
	if( NULL == str || NULL == buffer )
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

CURLcode CHttpClient::GetBin(const std::string & strUrl, BYTE** pData,int& nLen)
{
	if (strncmp(strUrl.c_str(),"https",5) == 0)
	{
		return Gets(strUrl,pData,nLen);
	}
	return Get(strUrl,pData,nLen);
}

CURLcode CHttpClient::GetString(const std::string & strUrl, std::string & strResponse)
{
	if (strncmp(strUrl.c_str(),"https",5) == 0)
	{
		return Gets(strUrl,strResponse);
	}
	return Get(strUrl,strResponse);


}


CHCcode CHttpClient::Async_Get(const std::string & strUrl, BYTE** pData,int& nLen, const char * pCaPath,bool bEnableReportProgress)
{
	//m_bActiveStop = false;
	m_pDlBuffer->ResetData();
	bool b_https = strncmp(strUrl.c_str(),"https",5) == 0;
	
	
	CHCcode res = CHC_OK;
	CURLMcode mres;
	CURL* curl = curl_easy_init();
	CURLM *mcurl = curl_multi_init();
	int still_running;
	struct timeval mp_start = tvnow();;
	CURLMsg *msg; /* for picking up messages with the transfer status */
	int msgs_left; /* how many messages are left */
	int response_code = -1;

	if(NULL == curl || mcurl == NULL)
	{

		if (mcurl != NULL) curl_multi_cleanup(mcurl);
		if (NULL != curl) curl_easy_cleanup(curl);
		return CHC_FAILED_INIT;
	}


	if(m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnRecvData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	if (!m_strProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, m_strProxy.c_str());// 代理
	}

	if (bEnableReportProgress)
	{
		//curl的进度条声明
		curl_easy_setopt(curl,CURLOPT_NOPROGRESS, false);
		//回调进度条函数
		curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,download_progress);
		//设置进度条回调函数第一个参数
		curl_easy_setopt(curl,CURLOPT_PROGRESSDATA,this);

	}
	

	if (b_https)
	{
		if(NULL == pCaPath)
		{
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
			curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
		}
	}

	//设置http 头部处理函数  
	long long filesize;
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getheaderfunc);  
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

	//301
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,1);

	mres = curl_multi_add_handle(mcurl, curl);
	mres = curl_multi_perform(mcurl, &still_running);


    char* ip_tmp;
	long long port_tmp;
	double d_tmp;
	char *pbuff = NULL;
	while(still_running && !m_bActiveStop)
	{
		struct timeval timeout;
		int rc; /* select() return code */

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = -1;

		long curl_timeo = -1;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		/* set a suitable timeout to play around with */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;


		//mres = curl_multi_timeout(mcurl, &curl_timeo);
		//if(curl_timeo >= 0) {
		//  timeout.tv_sec = curl_timeo / 1000;
		//  if(timeout.tv_sec > 1)
		//	timeout.tv_sec = 1;
		//  else
		//	timeout.tv_usec = (curl_timeo % 1000) * 1000;
		//}

		/* get file descriptors from the transfers */
		mres = curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);

		if (maxfd < 0)
		{
			if (tvdiff(tvnow(), mp_start) > ERROR_TIME_OUT)
			{
				res = CHC_FDSET_ERROR;
				ei_log(LV_WARNING,"hls","curl_multi_fdset error");
				goto exit;
			}
			else
			{
				usleep(10000);
				curl_multi_perform(mcurl, &still_running);
				continue;
			}
		}

		/* In a real-world program you OF COURSE check the return code of the
		function calls.  On success, the value of maxfd is guaranteed to be
		greater or equal than -1.  We call select(maxfd + 1, ...), specially in
		case of (maxfd == -1), we call select(0, ...), which is basically equal
		to sleep. */

		rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

		if (rc > 0)
		{
			mp_start = tvnow();//如果有数据，重置计时器
		}
		if (tvdiff(tvnow(), mp_start) > MAX_TIME_OUT) 
		{
			ei_log(LV_WARNING,"hls","download timeout");
			res = CHC_TIMEOUT;
			goto exit;
		}

		switch(rc)
		{
		case -1:
			/* select error */
			res = CHC_SELECT_ERROR;
			ei_log(LV_WARNING,"hls","select error");
			goto exit;
		case 0: /* timeout */
		default: /* action */
			curl_multi_perform(mcurl, &still_running);
			break;
		}
	}

	msg = curl_multi_info_read(mcurl, &msgs_left);
	if (msg != NULL)
	{
		if(msg->msg == CURLMSG_DONE)
		{
			if (msg->data.result != CURLE_OK)
			{
				ei_log(LV_ERROR,"hls","curl error:%s",curl_easy_strerror(msg->data.result));
				return CHC_CURL_ERROR;
			}
		}
		else
		{
			ei_log(LV_ERROR,"hls","curl error,msg->msg != CURLMSG_DONE");
			return CHC_CURL_ERROR;
		}

		curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
	}
	else
	{
		ei_log(LV_ERROR,"hls","curl_multi_info_read return NULL");
		return CHC_CURL_ERROR;
	}

	// ask curl for IP address
    if (curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip_tmp) == CURLE_OK)
	{
		m_strServerIP = ip_tmp;
	}
	//port
	if (curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port_tmp) == CURLE_OK)
	{
		m_nServerPort = port_tmp;
	}
	//total time
	if (curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &d_tmp) == CURLE_OK)
	{
		m_fTotalTime = d_tmp;
	}
	//speed
	if (curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &d_tmp) == CURLE_OK)
	{
		m_fSpeed = d_tmp;
	}

	//获取最终请求的url地址
    if (curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL,&pbuff) == CURLE_OK)
	{
		m_strEffective_url = pbuff;
	}
    

exit:
	curl_multi_remove_handle(mcurl, curl);
	curl_multi_cleanup(mcurl);
	curl_easy_cleanup(curl);






	if (m_pDlBuffer->m_bError)
	{
		nLen = 0;
		ei_log(LV_ERROR,"hls","download buffer error");
		return CHC_OUT_OF_MEMORY;
	}

	if (res == CHC_OK && response_code == 200)
	{
		*pData = m_pDlBuffer->m_pBuffer;
		nLen = m_pDlBuffer->m_nDataLen;
	}
	else
	{
		//if (response_code != 200)
		//{
			if (response_code < 0)
			{
				//return CHC_UNKNOW_ERROR;
				return res;
			}
			ei_log(LV_ERROR,"hls","http response error,code=%d,url=%s",response_code,strUrl.c_str());
			return CHC_RESPONSE_ERROR;
		//}
	}
	


	


	if (m_bActiveStop)
	{
		return CHC_CANCEL;
	}
	return res;
}




CHCcode CHttpClient::Async_GetBin(const std::string & strUrl, BYTE** pData,int& nLen,HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T& info)
{
	ResetInfo();

	struct timeval tv_start;
	gettimeofday(&tv_start,NULL);

	m_strUrl = strUrl;

	//report downloading
	HLS_REPORT_PARAM_DOWNLOADING_T msg;
	msg.segment = strUrl.substr(strUrl.rfind("/")+1);
	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_DOWNLOADING,msg);

	//download
	CHCcode ret = Async_Get(strUrl,pData,nLen,NULL,true);
	
	//infos
	struct timeval tv_now;
	gettimeofday(&tv_now,NULL);
	long long used_ms = (long long)(tv_now.tv_sec-tv_start.tv_sec)*1000 + (tv_now.tv_usec-tv_start.tv_usec)/1000;
	//info.used_time = (double)used_ms / 1000;
	info.used_time = m_fTotalTime;//get by curl
	info.file_size = m_nFileSize;
	if (used_ms >0)
	{
		//info.download_speed = (double)m_nFileSize*8/used_ms*1000;
		info.download_speed = m_fSpeed*8;
	}
	info.segment = msg.segment;
	info.server_address = m_strServerIP;
	info.server_port = m_nServerPort;

	//report http header
	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_HEAD_TS,m_strHeader);

	//set downloading to null
	msg.segment = "";
	msg.progress = 0;
	msg.dlnow = 0;
	msg.dltotal = 0;
	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_DOWNLOADING,msg);

	return ret;

}

CHCcode CHttpClient::Async_GetString(const std::string & strUrl, std::string & strResponse)
{
	ResetInfo();

	CHCcode res;
	BYTE* pData;
	int nLen = 0;

	res = Async_Get(strUrl,&pData,nLen);

	if (res == CHC_OK && nLen > 0)
	{
		strResponse.resize(nLen+1,0);
		memcpy(&strResponse[0],pData,nLen);
	}

	//report http header
	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_HEAD_M3U8,m_strHeader);
	return res;
}

void CHttpClient::Cancel()
{
	m_bActiveStop = true;
}

int CHttpClient::download_progress(void *papp,   double dltotal,   double dlnow,   double ultotal,   double ulnow)
{
	if (dltotal <= 0)
	{
		return 0;
	}

	CHttpClient* lpthis = (CHttpClient*)papp;
	lpthis->m_nFileSize = dltotal;


	HLS_REPORT_PARAM_DOWNLOADING_T msg;
	msg.segment = lpthis->m_strUrl;
	msg.segment = msg.segment.substr(msg.segment.rfind("/")+1);
	msg.progress = 100.0 * (dlnow/dltotal);
	msg.dlnow = dlnow;
	msg.dltotal = dltotal;

	lpthis->m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_DOWNLOADING,msg);
	return 0;
}

size_t CHttpClient::getheaderfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	CHttpClient* lpthis = (CHttpClient*)stream;
	lpthis->m_strHeader += (char*)ptr;
	return size * nmemb;  
}

void CHttpClient::ResetInfo()
{
	m_strHeader.clear();
	m_strServerIP.clear();
	m_nFileSize = 0;
	m_strUrl.clear();
	m_nServerPort = 0;
	m_fTotalTime = 0;
	m_fSpeed = 0;
}

void CHttpClient::SetProxy(const string& strProxy)
{
	m_strProxy = strProxy;
}
