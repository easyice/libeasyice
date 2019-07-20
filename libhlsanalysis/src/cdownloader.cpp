/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "cdownloader.h"
#include "HttpClient.h"
#include <string.h>
#include "cm3u8parse.h"
#include <stdio.h>
#include "MsgMgr.h"
#include "commondefs.h"
#include "global.h"
#include "TsAnalysis.h"
#include "BufferAnalysis.h"
#include "AsyncSaveFile.h"
#include "creadline.h"
#include "utils.h"
#include <unistd.h>

extern "C"
{
#include <curl/curl.h>
}

extern int nHlsAnalysis;

CDownloader::CDownloader(CMsgMgr* pMsgMgr) : m_eventOnTsAdd("CDownloader")
{
    m_pMsgMgr = pMsgMgr;
	m_pHttpClientM3u8 = new CHttpClient(m_pMsgMgr);
	m_pHttpClientTs = new CHttpClient(m_pMsgMgr);
	m_pTsAnalysis = new CTsAnalysis(m_pMsgMgr);
	m_pBufferAnalysis = new CBufferAnalysis();
	m_pAsyncSaveFile = new CAsyncSaveFile();

	pthread_mutex_init(&m_mutex,NULL);
	m_bStop = false;
	m_bFinish = false;
	m_bRecordEnable = false;
	m_bFirstSegmentRecved = false;
	m_duration = 0;
	m_eventOnTsAdd.Reset();
    m_bThreadValid = false;
}

CDownloader::~CDownloader()
{
	delete m_pHttpClientM3u8;
	delete m_pHttpClientTs;
	delete m_pTsAnalysis;
	delete m_pBufferAnalysis;
	delete m_pAsyncSaveFile;


	pthread_mutex_destroy(&m_mutex);
}

void CDownloader::Stop()
{
	m_bStop = true;
	m_pHttpClientM3u8->Cancel();
	m_pHttpClientTs->Cancel();

	if (m_bRecordEnable)
	{
		m_pAsyncSaveFile->Stop();
	}
	
	//for wake up mainloop
    m_eventOnTsAdd.Set();
}

void CDownloader::SetProxy(const char* strProxy)
{
	m_pHttpClientM3u8->SetProxy(strProxy);
	m_pHttpClientTs->SetProxy(strProxy);
}

void CDownloader::Run(const string& url)
{
	
	CHCcode ret  ;
	string m3u8_content;
	string log_str;
	string str_tmp;

	if ((ret = m_pHttpClientM3u8->Async_GetString(url,m3u8_content)) != CHC_OK)
	{
		ei_log(LV_ERROR,"hls","download m3u8 file error! error=%s",chc_strerror(ret));
		goto exit;
	}

	
	utils_ntorn(m3u8_content,log_str);
	str_tmp = "m3u8 content:\r\n";
	str_tmp += log_str;
	ei_log(LV_DEBUG,"hls",str_tmp.c_str());
	

	m_sys.m3u8 = url;
	
	if(m_pHttpClientM3u8->m_strEffective_url != url)
	{
		ei_log(LV_DEBUG,"hls","effective_url=%s",m_pHttpClientM3u8->m_strEffective_url.c_str());
		m_sys.m3u8 = m_pHttpClientM3u8->m_strEffective_url;
	}


	if (!SelectStream(m_sys,m_hls,m3u8_content))
	{
		m_pMsgMgr->HlsPostMessage(EM_HRT_SYS_FINISH);
		ei_log(LV_ERROR,"hls","SelectStream error");
		return;
	}

	

	m_duration = m_hls.duration;


	ei_log(LV_DEBUG,"hls","%s Playlist HLS protocol version: %d,m_duration:%d",m_sys.b_live ? "Live": "VOD", m_hls.version,m_duration);

	if (m_bRecordEnable)
	{
		m_pAsyncSaveFile->m_pSaveFile->m_segment_count = m_hls.segment_count;
        m_pAsyncSaveFile->m_pSaveFile->m_m3u8name = m_hls.url.substr( m_hls.url.rfind("/")+1);
		m_pAsyncSaveFile->m_pSaveFile->m_m3u8name = hls_analysis::removeparam(m_pAsyncSaveFile->m_pSaveFile->m_m3u8name);
        if (m_pAsyncSaveFile->m_pSaveFile->m_m3u8name.empty())
        {
			ei_log(LV_ERROR,"hls","m3u8 name removeparam faild");
            return;
        }

		m_pAsyncSaveFile->Start(m_stRecordParam);
	}

	if (m_sys.b_live)
	{
		m_pMsgMgr->HlsPostMessage(EM_HRT_SYS_WAITTING_FIRST_SEGMENT);

		//为缓冲分析模块预先加载3个分片
		//没有使用反向迭代器，发现rbegin返回的的不是最后一个元素的迭代器，而是最后一个元素的下一个（不存在的）

		int need_pre_buffer = 2;//此处设置为2，加实际下载的一个，共缓存3个
		if (m_lstOldSegments.size() < need_pre_buffer)
		{
			need_pre_buffer = m_lstOldSegments.size();
		}
		int need_jump = m_lstOldSegments.size() - need_pre_buffer;
		list< segment_t >::iterator it = m_lstOldSegments.begin();
		for(int i = 0; i < need_jump; i++)
		{
			++it;
		}
		for (; it != m_lstOldSegments.end();++it)
		{
			m_pBufferAnalysis->AddPreBuffer(it->duration);
		}

        m_bThreadValid =(pthread_create(&m_thread,NULL,ReloadThread,this) == 0);
	}
	else
	{
		m_pMsgMgr->HlsPostMessage(EM_HRT_SYS_ANALYSISING);

		if (m_bRecordEnable)
		{
			m_pAsyncSaveFile->OnRecvM3u8(ReplaceM3u8(m_strOldM3u8Content));
		}

		list< segment_t >::const_iterator it = m_lstOldSegments.begin();
		for (;it != m_lstOldSegments.end();++it)
		{
			m_dqTsUrl.push_back(*it);
		}
        m_eventOnTsAdd.Set();

		//report m3u8 content
		m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HLS_M3U8,m_strOldM3u8Content);
	}

	MainLoop();

	if (m_bThreadValid)
	{
        pthread_join(m_thread,NULL);
	}

exit:
	ei_log(LV_DEBUG,"hls","analysis stoped");
	m_pMsgMgr->HlsPostMessage(EM_HRT_SYS_FINISH);
}

bool CDownloader::SelectStream(stream_sys_t& sys, hls_stream_t& hls,const string& m3u8_content)
{
	CM3u8Parse parse;
	if (!parse.isHTTPLiveStreaming(m3u8_content))
	{
		ei_log(LV_ERROR,"hls","not m3u8 file!");
		return false;
	}

	if (parse.isMeta(m3u8_content))
	{
		sys.b_meta = true;
		if (!parse.ParseMeta(sys,m3u8_content))
		{
			ei_log(LV_ERROR,"hls","meta parse error!");
			return false;
		}
		hls = parse.m_lstStreams.front();

		ei_log(LV_DEBUG,"hls", "meta list:");
		list<hls_stream_t>::iterator it = parse.m_lstStreams.begin();
		for (; it != parse.m_lstStreams.end(); ++it)
		{
			ei_log(LV_DEBUG,"hls", "%d--%s",it->id,it->url.c_str());
		}

		ei_log(LV_DEBUG,"hls", "selected stream:%s", hls.url.c_str());
		sys.m3u8 = hls.url;

		//download meta and parse
		string new_m3u8;
		if (m_pHttpClientM3u8->Async_GetString(sys.m3u8,new_m3u8) != CHC_OK)
		{
			ei_log(LV_ERROR,"hls","download m3u8 index error!");
			return false;
		}
		
		if(m_pHttpClientM3u8->m_strEffective_url != sys.m3u8)
		{
			ei_log(LV_DEBUG,"hls","effective_url=%s",m_pHttpClientM3u8->m_strEffective_url.c_str());
			sys.m3u8 = m_pHttpClientM3u8->m_strEffective_url;
		}
		return SelectStream(sys,hls,new_m3u8);
	}
	else
	{
		if (!parse.ParseStream(sys,&hls,m3u8_content))
		{
			ei_log(LV_ERROR,"hls","m3u8 parse error!");
			return false;
		}
		m_lstOldSegments = parse.m_lstSegments;
		m_strOldM3u8Content = m3u8_content;
	}
	return true;
}

void* CDownloader::ReloadThread(void* pParam)
{
	CDownloader* lpthis = (CDownloader*) pParam;
	lpthis->ReloadFun();

	return 0;
}

void CDownloader::ReloadFun()
{
	while(!m_bStop)
	{
		//ei_log(LV_DEBUG,"hls","reloading m3u8...");
		string m3u8_content;
		CHCcode ret = m_pHttpClientM3u8->Async_GetString(m_hls.url,m3u8_content);
		if (ret != CHC_OK)
		{
			ei_log(LV_ERROR,"hls","download m3u8 index error!");
			usleep(10000);
			continue;
			//if (ret ==  CHC_TIMEOUT)
			//{
			//	continue;
			//}
		}

		//report m3u8 content
		m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HLS_M3U8,m3u8_content);

		if (m3u8_content == m_strOldM3u8Content)
		{
			//wait
			int sleep_time = m_duration/2*1000;
			if (sleep_time >10000)
			{
				sleep_time = 3000;
			}
			struct timeval mp_start = hls_analysis::tvnow();
			while(1)
			{
				if (m_bStop)
				{
					return;
				}
				if (hls_analysis::tvdiff(hls_analysis::tvnow(), mp_start) > sleep_time)
				{
					break;
				}
				usleep(30000);
			}

			continue;
		}
		
		if (nHlsAnalysis != LIB_HLS_ANALYSIS_CALL_PASSWD)
		{
			m3u8_content = m_strOldM3u8Content;
		}

		

		CM3u8Parse parse;
		if (!parse.ParseStream(m_sys,&m_hls,m3u8_content))
		{
			ei_log(LV_ERROR,"hls","m3u8 parse error!");

			continue;
		}

		OnNewM3u8(parse.m_lstSegments);

		//等ts下载完再更新m3u8
		//while(1)
		//{
		//	Sleep(50);
		//	pthread_mutex_lock(&m_mutex);
		//	bool bempty = m_dqTsUrl.empty();
		//	pthread_mutex_unlock(&m_mutex);
		//	if (bempty)
		//	{
		//		break;
		//	}
		//}
		//

		if (m_bRecordEnable)
		{
			m_pAsyncSaveFile->OnRecvM3u8(ReplaceM3u8(m3u8_content));
		}

		m_strOldM3u8Content = m3u8_content;
		m_lstOldSegments = parse.m_lstSegments;


	}
	m_bFinish = true;
}

void CDownloader::OnNewM3u8(const list< segment_t >& lst)
{
	list< segment_t >::const_iterator it = lst.begin();
	for (;it != lst.end();++it)
	{
		bool bfind = false;
		list<segment_t>::iterator it_old = m_lstOldSegments.begin();
		for(; it_old != m_lstOldSegments.end();++it_old)
		{
			if (it->url == it_old->url)
			{
				bfind = true;
				break;
			}
		}
		if (!bfind)
		{
			pthread_mutex_lock(&m_mutex);
			m_dqTsUrl.push_back(*it);
			pthread_mutex_unlock(&m_mutex);
            m_eventOnTsAdd.Set();
		}
	}


}

void CDownloader::MainLoop()
{
	while(1)
	{
		if (m_bStop)
		{
			break;
		}
		if (m_bFinish)
		{
			break;
		}
		pthread_mutex_lock(&m_mutex);
		bool bempty = m_dqTsUrl.empty();
		pthread_mutex_unlock(&m_mutex);

		if (bempty)
		{
			if (m_sys.b_live)
			{
                m_eventOnTsAdd.Wait(100);
				continue;
			}
			else
			{
				ReportWaittingDownload();
				return;
			}
		}

		if (m_sys.b_live && !m_bFirstSegmentRecved)
		{
			m_bFirstSegmentRecved = true;
			m_pMsgMgr->HlsPostMessage(EM_HRT_SYS_ANALYSISING);
		}

		ReportWaittingDownload();

		pthread_mutex_lock(&m_mutex);
		string url = m_dqTsUrl.begin()->url;
		double duration = m_dqTsUrl.begin()->duration;
		pthread_mutex_unlock(&m_mutex);

		if (m_bRecordEnable)
		{
			if (!m_sys.b_live && m_stRecordParam.continue_mode && m_pAsyncSaveFile->m_pSaveFile->IsExist(url))
            {
                    pthread_mutex_lock(&m_mutex);
                    m_dqTsUrl.pop_front();
                    pthread_mutex_unlock(&m_mutex);
                    continue;
            }

		}

		BYTE* p;
		int nLen;

		HLS_REPORT_PARAM_DOWNLOAD_HISTORY_T dl_history;
		dl_history.duration = duration;
		//url="https://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/fileSequence44.ts";
		CHCcode ret = m_pHttpClientTs->Async_GetBin(url,&p,nLen,dl_history);
		if ( ret != CHC_OK)
		{
			ei_log(LV_ERROR,"hls","download ts error! ret=%s",chc_strerror(ret));
			if (ret ==  CHC_TIMEOUT)
			{
				ei_log(LV_WARNING,"hls","retrying.. ret=%s",chc_strerror(ret));
				continue;
			}
			else
			{
				//report this downlaod errors
				m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HLS_DOWNLOAD_ERRORS,hls_analysis::get_uri(url));

				//report downlaod history
				dl_history.status = false;
				m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_DOWNLOAD_HISTORY,dl_history);

				//其他类型的失败忽略此分片
				pthread_mutex_lock(&m_mutex);
				m_dqTsUrl.pop_front();
				pthread_mutex_unlock(&m_mutex);
				continue;
			}
		}
		pthread_mutex_lock(&m_mutex);
		m_dqTsUrl.pop_front();
		pthread_mutex_unlock(&m_mutex);

		if (nHlsAnalysis != LIB_HLS_ANALYSIS_CALL_PASSWD)
		{
			pthread_mutex_lock(&m_mutex);
			m_dqTsUrl.clear();
			pthread_mutex_unlock(&m_mutex);
			duration = 0;
			nLen /= 2;
		}

		//report downlaod history
		m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HTTP_DOWNLOAD_HISTORY,dl_history);

		
		//ei_log(LV_DEBUG,"hls","ts download ok:size=%d,url=%s,analysising",nLen,url.c_str());
		m_pTsAnalysis->OnRecvSegment(p,nLen,url);
		m_pBufferAnalysis->AddSegment(duration);
		//ei_log(LV_DEBUG,"hls","ts download ok:size=%d,url=%s,analysis finish",nLen,url.c_str());

		if (m_bRecordEnable)
		{
			m_pAsyncSaveFile->OnRecvTs(url,p,nLen);
		}

		pthread_mutex_lock(&m_mutex);
		int remain = m_dqTsUrl.size();
		pthread_mutex_unlock(&m_mutex);
		//ei_log(LV_DEBUG,"hls","remain:%d",remain);
	}
}

bool CDownloader::GetBufferDuration(double& duration,long long& llCurTime)
{
	return m_pBufferAnalysis->GetBufferDuration(duration,llCurTime);
}

void CDownloader::SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param)
{
	m_bRecordEnable = true;
	m_stRecordParam = rc_param;
}

void CDownloader::ReportWaittingDownload()
{
	string report;
	

	pthread_mutex_lock(&m_mutex);
	long long llsize = m_dqTsUrl.size();
	deque<segment_t>::iterator it = m_dqTsUrl.begin();
	for (; it != m_dqTsUrl.end(); ++it)
	{
		if (it->url.rfind("/") != string::npos)
		{
			report += it->url.substr(it->url.rfind("/")+1);
		}
		else
		{
			report += it->url;
		}
		report += "\r\n";
	}

	pthread_mutex_unlock(&m_mutex);

	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD_COUNT,llsize);
	m_pMsgMgr->HlsPostMessage(EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD,report);
}

string CDownloader::ReplaceM3u8(const string& m3u8_content)
{
        string rst;
        CReadLine readline;
        readline.SetString(m3u8_content);
        do 
        {
                /* Next line */
                string line = readline.ReadLine();
                
                if (line.empty())
                {
                        break;
                }
                
                line += "\n";
                if (line[0] == '#')
                {
                        rst.append(line);
                        continue;
                }
                else
                {
                        if (line.rfind("/") != string::npos)
                        {
                                string tsname = line.substr(line.rfind("/")+1);
								tsname = hls_analysis::removeparam(tsname);
                                if (tsname.empty())
                                {
                                        cout << "tsname removeparam in ReplaceM3u8 faild"<<endl;
                                        continue;
                                }
                                if (tsname[tsname.length()-1] != '\n')
                                {
                                        tsname += "\n";
                                }
                                rst.append(tsname);
                        }
                        else
                        {
                                rst.append(line);
                        }
                }
                
        }while(1);
        
        return rst;
}



