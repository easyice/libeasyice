/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H


#include <string>
#include "hls_type.h"

#include "hls_define.h"
#include <curl/curl.h>

/**
 * m3u8与ts在不同线程下载，必须有本类的两个各自的实例。不能用同一个类指针
*/




typedef enum {
	CHC_OK = 0,
	CHC_FAILED_INIT,
	CHC_CANCEL, //用户中止
	CHC_TIMEOUT,
	CHC_SELECT_ERROR,
	CHC_FDSET_ERROR,
	CHC_OUT_OF_MEMORY,
	CHC_RESPONSE_ERROR,
	CHC_UNKNOW_ERROR,
	CHC_CURL_ERROR,

	CHC_LAST /* never use! */
} CHCcode;

static const char* errorString[ CHC_LAST ] = 
{
	"No error",
	"init faild",
	"user canceled",
	"timeout",
	"select error",
	"fdset error",
	"out of memory",
	"http response error",
	"unknow error",
	"curl error",
};

static const char* chc_strerror(CHCcode code)
{
	return errorString[code];
}

class CMsgMgr;
class CDlBuffer;
class CHttpClient
{
public:
	CHttpClient(CMsgMgr* pMsgMgr);
public:
	~CHttpClient(void);

	/**
	* @brief HTTP POST请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strPost 输入参数,使用如下格式para1=val12=val2&…
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/
	CURLcode Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);

	/**
	* @brief HTTP GET请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/
	CURLcode Get(const std::string & strUrl, std::string & strResponse);
	CURLcode Get(const std::string & strUrl, BYTE** pData,int& nLen);

	/**
	* @brief HTTPS POST请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strPost 输入参数,使用如下格式para1=val12=val2&…
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Post成功
	*/
	CURLcode Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);

	/**
	* @brief HTTPS GET请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Post成功
	*/
	CURLcode Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);
	CURLcode Gets(const std::string & strUrl, BYTE** pData,int& nLen, const char * pCaPath = NULL);


	CURLcode GetBin(const std::string & strUrl, BYTE** pData,int& nLen);
	CURLcode GetString(const std::string & strUrl, std::string & strResponse);
	void SetDebug(bool bDebug);

	void SetProxy(const string& strProxy);

	////////////////////////////////////异步接口///////////////////////////////////////////
	CHCcode Async_Get(const std::string & strUrl, BYTE** pData,int& nLen,const char * pCaPath = NULL,bool bEnableReportProgress = false);
	CHCcode Async_GetBin(const std::string & strUrl, BYTE** pData,int& nLen,HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T& info);
	CHCcode Async_GetString(const std::string & strUrl, std::string & strResponse);
	void Cancel();
private:
	static size_t OnRecvData(void *buffer, size_t size, size_t nmemb, void *stream);
	static size_t OnRecvString(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int download_progress(void *papp,   double dltotal,   double dlnow,   double ultotal,   double ulnow);
	static size_t getheaderfunc(void *ptr, size_t size, size_t nmemb, void *stream); 

private:
	void ResetInfo();
private:
	bool m_bDebug;
	CDlBuffer* m_pDlBuffer;
	bool m_bActiveStop;

	int m_nFileSize;
	string m_strUrl;
	string m_strHeader;
	string m_strServerIP;
	string m_strProxy;
	int m_nServerPort;
	double m_fTotalTime;//seconds
	double m_fSpeed;//bytes/second

    CMsgMgr* m_pMsgMgr;
public:
	string m_strEffective_url;
};


#endif

