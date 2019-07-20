/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef CDOWNLOADER_H
#define CDOWNLOADER_H

#include "hls_define.h"
#include "hls_type.h"
#include "EiLog.h"
#include <deque>
#include <list>
#include "zevent.h"

class CMsgMgr;
class CAsyncSaveFile;
class CBufferAnalysis;
class CTsAnalysis;
class CHttpClient;
class CDownloader
{
public:
        CDownloader(CMsgMgr* pMsgMgr);
        ~CDownloader();
        void Run(const string& url);
        void Stop();
		bool GetBufferDuration(double& duration,long long& llCurTime);

		void SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param);
		void SetProxy(const char* strProxy);
private:
        //m3u8 reload thread
        static void* ReloadThread(void* pParam);
        void ReloadFun();
        void MainLoop();
		void ReportWaittingDownload();
        
        /**
         *@param [in/out] sys
         * @param [in/out] hls
         */
        bool SelectStream(stream_sys_t &sys,hls_stream_t& hls,const string& m3u8_content);
        
        //判断新增segment，加入待下载列表
        void OnNewM3u8(const list<segment_t>& lst);

		string ReplaceM3u8(const string& m3u8_content);
private:

        CHttpClient* m_pHttpClientM3u8;
		CHttpClient* m_pHttpClientTs;
		CTsAnalysis* m_pTsAnalysis;
		CBufferAnalysis* m_pBufferAnalysis;
		CAsyncSaveFile* m_pAsyncSaveFile;

        
        pthread_t m_thread;
        pthread_mutex_t m_mutex;
        bool m_bThreadValid;
        deque<segment_t> m_dqTsUrl;//待下载TS
        
        stream_sys_t m_sys;
        hls_stream_t m_hls;
        ZEvent m_eventOnTsAdd;
        
        list<segment_t> m_lstOldSegments;
        string m_strOldM3u8Content;
        
        bool m_bStop;//error or activestop
		bool m_bFinish;

		bool m_bFirstSegmentRecved;
        
        int m_duration;
        
		bool m_bRecordEnable;
		HLS_RECORD_INIT_PARAM_T m_stRecordParam;

        CMsgMgr* m_pMsgMgr;
};

#endif // CDOWNLOADER_H
