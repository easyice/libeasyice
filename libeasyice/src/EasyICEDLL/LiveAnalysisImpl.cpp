/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "LiveAnalysisImpl.h"
#include "LiveSourceFactory.h"
#include "LiveSourceBase.h"
#include "CircularBuffer.h"
#include "CheckMediaInfo.h"
#include "MpegDec.h"
#include "LivePcrProc.h"
//#include "cudpsend.h"
#include "EiLog.h"
#include "libtr101290.h"
#include <sys/time.h>
#include "json/json.h"
#include <algorithm>
#include "CheckMediaInfo.h"
#include "TrView.h"
#include <unistd.h>

using namespace std;



//只支持188的TS包
#define SPUPPRT_TS_PACKET_LEN		188

//等待解码所有PMT的超时时间(ns)
#define WAIT_FOR_ALL_PMT_TIMEOUT	3000000




CLiveAnalysisImpl::CLiveAnalysisImpl(void) : m_event("LiveAnalysis")
{
    m_pHandle = NULL;
	m_pSource = NULL;
	m_pCircularBuffer = new CCircularBuffer();
	m_pCircularBuffer->Init(8192*3,7);
	m_bStop = false;

	m_pMediaInfoBuffer = NULL;
	m_nMediaInfoBufferLen = 0;
	m_bMediaInfoChecked = false;
	m_nTsLength = SPUPPRT_TS_PACKET_LEN;

	m_pTrcore = new Clibtr101290();
	nlibtr101290 = CALL_PASSWD;
	m_bInited = false;
	m_llFirstByteRecvTime = -1;
	m_llCbUpdateTime = -1;
	m_bRecvedFirstByte = false;

	m_mpegdec = new CMpegDec();
	m_pLiveProc = new CLivePcrProc();
    m_pEiMediaInfo = new CEiMediaInfo();
	//m_pUdpSend = new CUdpSend();

	m_fpRecordFile = NULL;
	m_pRecordBuffer = NULL;
	pthread_mutex_init(&m_mutexRecordFp,NULL);
	pthread_mutex_init(&m_mutexRecordBuf,NULL);

	m_bStartRecord = false;
    m_event.Reset();
    m_pTrView = new CTrView();
    m_pTrcore->SetReportCB(CTrView::OnTrReport,m_pTrView);
    m_bWorkThreadValid = false;;
    m_bMiThreadValid = false;
    m_bRecordThreadValid = false;
}

CLiveAnalysisImpl::~CLiveAnalysisImpl(void)
{
	if (!m_bStop)
	{
		Stop();
	}
	
	delete m_pCircularBuffer;
	delete [] m_pMediaInfoBuffer;
	delete m_pTrcore;
	delete m_mpegdec;
	delete m_pLiveProc;
    delete m_pTrView;
    delete m_pEiMediaInfo;
	//delete m_pUdpSend;

	StopRecord();
	pthread_mutex_destroy(&m_mutexRecordFp);
	pthread_mutex_destroy(&m_mutexRecordBuf);
}


int CLiveAnalysisImpl::OpenMRL(EASYICE* handle)
{
    ei_log(LV_INFO,"libeasyice","open mrl:%s",handle->mrl);

	if (m_pSource != NULL)
	{
		//some error code
        ei_log(LV_ERROR,"libeasyice","m_pSource != NULL！");
		return -1;
	}

	CLiveSourceFactory lsf;
	m_pSource = lsf.CreateSource(handle->mrl,handle->local_ip);
	if (m_pSource == NULL)
	{
		//some error code
        ei_log(LV_ERROR,"libeasyice","CreateSource error！");
		return -1;
	}

	//转发到本地
	//m_pUdpSend->InitSend("127.0.0.1",7789);
	
	m_pSource->SetRecvDataCB(OnRecvData,this);
	int ret = m_pSource->Run();
	if (ret < 0)
	{
		delete m_pSource;
		m_pSource = NULL;
		return ret;
	}

    m_pMediaInfoBuffer = new BYTE[handle->udplive_probe_buf_size];

    m_pHandle = handle;
    m_sMrl = handle->mrl;

    m_pLiveProc->SetCalcTsRateIntervalTime(&handle->udplive_calctsrate_interval_ms);

    ei_log(LV_INFO,"libeasyice","Waiting data...");

	//创建线程
    m_bWorkThreadValid = (pthread_create(&m_hThread,NULL,WorkThread,this) == 0);
	return ret;
}

void CLiveAnalysisImpl::OnRecvData(void* pApp,BYTE* pData,int nLen)
{
	CLiveAnalysisImpl* pthis = (CLiveAnalysisImpl*) pApp;

	struct timeval tv_now;
	gettimeofday(&tv_now,NULL);

	long long time = (long long)tv_now.tv_sec * 1000000 + (long long)tv_now.tv_usec;
	if (!pthis->m_pCircularBuffer->WriteItem(pData,nLen,time))
    {
        ei_log(LV_WARNING,"libeasyice","CircularBuffer is full ,droping data");
    }
    pthis->m_event.Set();
}

bool CLiveAnalysisImpl::Stop(bool bForce)
{
	if (m_pSource)
	{
		m_pSource->Stop(bForce);
		delete m_pSource;
		m_pSource = NULL;
	}

	m_bStop = true;
    if (m_bWorkThreadValid)
    {
        pthread_join(m_hThread,NULL);
    }
    if (m_bMiThreadValid)
    {
        pthread_join(m_hMiThread,NULL);
    }
	return true;
}

void* CLiveAnalysisImpl::WorkThread(void* lpParam)
{
	CLiveAnalysisImpl* lpthis = (CLiveAnalysisImpl*)lpParam;
	lpthis->WorkFun();
	return 0;
}

void CLiveAnalysisImpl::WorkFun()
{
	BYTE* pItem = NULL;
	CCircularBuffer::ITEMINFO_T info;

	m_pTrcore->SetStartOffset(0);
	m_pTrcore->SetTsLen(SPUPPRT_TS_PACKET_LEN);
	m_mpegdec->LiveInit(SPUPPRT_TS_PACKET_LEN);
	m_pLiveProc->SetTsLength(SPUPPRT_TS_PACKET_LEN);

	while(1)
	{
		if (m_bStop)
		{
			break;
		}

		pItem = m_pCircularBuffer->ReadItem(info);
		if (pItem == NULL)
		{
            m_event.Wait(10);
			continue;
		}


		if (m_bStartRecord)
		{
			pthread_mutex_lock(&m_mutexRecordBuf);
			if (!m_pRecordBuffer->WriteItem(pItem,info.item_size,info.time))
            {
                ei_log(LV_WARNING,"libeasyice","recordbuffer is full droping data..");
            }
			pthread_mutex_unlock(&m_mutexRecordBuf);
		}


		if (!m_bRecvedFirstByte)
		{
            ei_log(LV_INFO,"libeasyice","waitting sync ");
			m_bRecvedFirstByte = true;
		}

		ProcessItem(pItem,info.item_size,info.time);

		//转发到本地
		//m_pUdpSend->SendData(pItem,info.item_size);

	}// !while(1)
}

void CLiveAnalysisImpl::ProcessItem(BYTE* pItem,int nSize,long long llTime)
{
	//不严谨的同步检测
	for (int i = 0; i < nSize; i+=m_nTsLength)
	{
		if (pItem[i] != 0x47)
		{
            ei_log(LV_WARNING,"libeasyice","skiping error bytes packet...");
			return;
		}
	}

	if (m_llFirstByteRecvTime < 0)
	{
		m_llFirstByteRecvTime = llTime;
		m_llCbUpdateTime = llTime;

		//设置状态栏
        ei_log(LV_INFO,"libeasyice","checking mediainfo...");
	}

	if (!m_bMediaInfoChecked && m_llFirstByteRecvTime > 0)
	{
		memcpy(m_pMediaInfoBuffer+m_nMediaInfoBufferLen,pItem,nSize);
		m_nMediaInfoBufferLen += nSize;

		bool b_time_out = llTime - m_llFirstByteRecvTime > WAIT_FOR_ALL_PMT_TIMEOUT ? true : false;
		if (m_nMediaInfoBufferLen > m_pHandle->udplive_probe_buf_size>>1 || b_time_out)
		{
            m_bMiThreadValid = (pthread_create(&m_hMiThread,NULL,MediaInfoThread,this) == 0);
			m_bMediaInfoChecked = true;

			//更新PSI.采样的表
            LiveCallBackPsi();

			//设置状态栏,接下来就是 “正在分析” 状态了
            ei_log(LV_INFO,"libeasyice","Analysising...");
		}
	}


	for (int i = 0; i < nSize; i+= m_nTsLength)
	{
		m_pTrcore->AddPacket(pItem+i);
		m_mpegdec->LiveProcessPacket(pItem+i);
	}

	//择机 回调检测结果
	if (llTime - m_llCbUpdateTime >= m_pHandle->udplive_cb_update_interval)
	{
		m_llCbUpdateTime = llTime;

        LiveCallBackPidList();
        LiveCallBackPcr();
        LiveCallBackRate();
        LiveCallBackTr101290();
	}

	if (!m_bInited)
	{
		bool b_time_out = llTime - m_llFirstByteRecvTime > WAIT_FOR_ALL_PMT_TIMEOUT ? true : false;
		if (m_pTrcore->IsDemuxFinish() || b_time_out)
		{
			m_mpegdec->FillPacketType();

			//get pcr pid
			ALL_PROGRAM_BRIEF* pBrif = m_mpegdec->GetAllProgramBrief();
			ALL_PROGRAM_BRIEF::iterator it = pBrif->begin();
			for (; it != pBrif->end(); it++)
			{
				vector<PID_TYPE>& pids = it->second;
				for (size_t i = 0; i < pids.size(); i++)
				{
					if (pids[i].type == PACKET_PCR)
					{
						m_pLiveProc->AddPcrPid(pids[i].pid);//重复添加的pid函数内会判断
						break;
					}
				}
			}

			// init ok
			m_bInited = true;

            LiveCallBackProgramInfoBrief();
			//发送解复用完毕消息
            ei_log(LV_INFO,"libeasyice","demux finish.");
		}
		return;
	}

	m_pLiveProc->ProcessBuffer(pItem,nSize,llTime);


}


void* CLiveAnalysisImpl::MediaInfoThread(void* lpParam)
{
	CLiveAnalysisImpl *pthis = (CLiveAnalysisImpl*)lpParam;
	
	MediaInfoDLL::String To_Display = pthis->m_pEiMediaInfo->CheckMediaInfo(pthis->m_pMediaInfoBuffer,pthis->m_nMediaInfoBufferLen,pthis->m_nTsLength);
    ((easyice_udplive_callback)pthis->m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_MEDIAFINO,To_Display.c_str(),pthis->m_pHandle->udplive_cb_data);

    //mktemp
    string tmpfile = pthis->m_sMrl;
    tmpfile.erase(std::remove(tmpfile.begin(), tmpfile.end(), ':'), tmpfile.end());
    tmpfile.erase(std::remove(tmpfile.begin(), tmpfile.end(), '@'), tmpfile.end());
    tmpfile.erase(std::remove(tmpfile.begin(), tmpfile.end(), '/'), tmpfile.end());
    tmpfile = "/tmp/" + tmpfile;
    FILE* fp = fopen(tmpfile.c_str(),"wb");
    if (fp != NULL)
    {
        fwrite(pthis->m_pMediaInfoBuffer,1,pthis->m_nMediaInfoBufferLen,fp); 
        fflush(fp);
        fclose(fp);
        //sleep(3);
        string ffprobe = pthis->m_pEiMediaInfo->ffprobe_all(tmpfile);
        //MediaInfoDLL::String To_Display = pthis->m_pEiMediaInfo->CheckMediaInfo(tmpfile);
        remove(tmpfile.c_str());
       
        ((easyice_udplive_callback)pthis->m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_FFPROBE,ffprobe.c_str(),pthis->m_pHandle->udplive_cb_data);
        //((easyice_udplive_callback)pthis->m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_MEDIAFINO,To_Display.c_str(),pthis->m_pHandle->udplive_cb_data);
    }
	return 0;
}


ALL_PROGRAM_BRIEF* CLiveAnalysisImpl::GetAllProgramBrief()
{
	return m_mpegdec->GetAllProgramBrief();
}



LST_RATE_INFO_T* CLiveAnalysisImpl::LockGetRate()
{
	return m_pLiveProc->LockGetRate();
}

void CLiveAnalysisImpl::UnlockRate()
{
	return m_pLiveProc->UnlockRate();
}


LST_PCR_INFO_T* CLiveAnalysisImpl::LockGetPcrInfo(int pcr_pid)
{
	return m_pLiveProc->LockGetPcrInfo(pcr_pid);
}

void CLiveAnalysisImpl::UnlockPcrInfo(int pcr_pid)
{
	return m_pLiveProc->UnlockPcrInfo(pcr_pid);
}


int CLiveAnalysisImpl::StartRecord(const char* strFileName)
{
	if (( m_fpRecordFile = fopen(strFileName,("wb")) ) == NULL)
	{
		return -1;
	}

	m_pRecordBuffer = new CCircularBuffer();
	m_pRecordBuffer->Init(8192,7);

    m_bRecordThreadValid = (pthread_create(&m_hRecordThread,NULL,RecordThread,this) == 0 );

	m_bStartRecord = true;
	return 0;
}


void CLiveAnalysisImpl::StopRecord()
{
	m_bStartRecord = false;

	//录制线程根据文件指针判断是否需要停止录制
	pthread_mutex_lock(&m_mutexRecordFp);
	if (m_fpRecordFile == NULL)
	{
		pthread_mutex_unlock(&m_mutexRecordFp);
		return;
	}
	fclose(m_fpRecordFile);
	m_fpRecordFile = NULL;
	pthread_mutex_unlock(&m_mutexRecordFp);

	if (m_bRecordThreadValid) 
    {
        pthread_join(m_hRecordThread,NULL);
    }

	pthread_mutex_lock(&m_mutexRecordBuf);
	delete m_pRecordBuffer;
	pthread_mutex_unlock(&m_mutexRecordBuf);
}

void* CLiveAnalysisImpl::RecordThread(void* lpParam)
{
	CLiveAnalysisImpl* lpthis = (CLiveAnalysisImpl*)lpParam;

	while (1)
	{
		pthread_mutex_lock(&lpthis->m_mutexRecordFp);
		if (lpthis->m_fpRecordFile == NULL)
		{
			pthread_mutex_unlock(&lpthis->m_mutexRecordFp);
			return 0;
		}
		pthread_mutex_unlock(&lpthis->m_mutexRecordFp);


		BYTE* pItem = NULL;
		CCircularBuffer::ITEMINFO_T info;
		pItem = lpthis->m_pRecordBuffer->ReadItem(info);
		if (pItem == NULL)
		{
			usleep(10000);
			continue;
		}

		pthread_mutex_lock(&lpthis->m_mutexRecordFp);
		if (lpthis->m_fpRecordFile == NULL)
		{
			pthread_mutex_unlock(&lpthis->m_mutexRecordFp);
			return 0;
		}
		fwrite(pItem,1,info.item_size,lpthis->m_fpRecordFile);
		pthread_mutex_unlock(&lpthis->m_mutexRecordFp);
	}
	
	return 0;
}


void CLiveAnalysisImpl::LiveCallBackPidList()
{
	m_mpegdec->UpdatePidListResult();

    //pid list
	Json::Value rootPid;
    std::map<int,MSG_PID_LIST>::iterator it_pids = m_mpegdec->m_mapPidList.begin();
    for (;it_pids != m_mpegdec->m_mapPidList.end();++it_pids)
    {
        int pid = it_pids->second.PID;
        int total = it_pids->second.total;
        float pct = it_pids->second.percent * 100;
        string type = Get_PacketTypeDes(it_pids->second.type);
		char key[32];
		sprintf(key,"%d", it_pids->first);
		rootPid[key] = it_pids->second.to_json();
    }
	string outPid = rootPid.toStyledString();  
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_PIDS,outPid.c_str(),m_pHandle->udplive_cb_data);

}

void CLiveAnalysisImpl::LiveCallBackPsi()
{
    Json::Value rootTable;
    TABLES* tables = m_mpegdec->m_TableAnalyzer.GetTables();
    rootTable = tables->to_json();
    string outTable = rootTable.toStyledString();
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_PSI,outTable.c_str(),m_pHandle->udplive_cb_data);
}

void CLiveAnalysisImpl::LiveCallBackPcr()
{
    ALL_PROGRAM_BRIEF* pBrif = GetAllProgramBrief();


    ALL_PROGRAM_BRIEF::iterator it = pBrif->begin();

    Json::Value root;
    for(;it != pBrif->end();++it)
    {
        vector<PID_TYPE>& pids = it->second;
        vector<PID_TYPE>::iterator it_pids = pids.begin();
        Json::Value pcrArray;
        for (; it_pids != pids.end(); ++it_pids)
        {
            if (it_pids->type == PACKET_PCR)
            {
                LST_PCR_INFO_T* pPcrInfo = LockGetPcrInfo(it_pids->pid);
                LST_PCR_INFO_T::iterator itPcr = pPcrInfo->begin();
                for (; itPcr != pPcrInfo->end(); ++itPcr)
                {
                    pcrArray.append(itPcr->to_json());
                }
                UnlockPcrInfo(it_pids->pid);
                break;
            }
        }
        char buf[32];
        sprintf(buf, "%d", it->first);
        root[buf] = pcrArray;
    }
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_PCR,root.toStyledString().c_str(),m_pHandle->udplive_cb_data);
}

void CLiveAnalysisImpl::LiveCallBackRate()
{
    LST_RATE_INFO_T* pRateInfo = LockGetRate();
    LST_RATE_INFO_T::iterator itRate = pRateInfo->begin();
    Json::Value root;
    Json::Value rates;
    for (; itRate != pRateInfo->end(); ++itRate)
    {
        rates.append(itRate->to_json());
    }
    UnlockRate();

    root["tsrate"] = rates;
    //TODO: 目前是数组形式，不是严禁的 json
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_RATE,root.toStyledString().c_str(),m_pHandle->udplive_cb_data);
}

void CLiveAnalysisImpl::LiveCallBackProgramInfoBrief()
{
    ALL_PROGRAM_BRIEF* pBrif = m_mpegdec->GetAllProgramBrief();
    std::map<int,std::vector<PID_TYPE> >::iterator it = pBrif->begin();
    Json::Value root;
    for (; it != pBrif->end(); it++) {
        char buf[32];
        sprintf(buf,"%d", it->first);
        Json::Value pidArray;
        for (size_t i = 0; i < it->second.size(); i++) {
            pidArray.append(it->second[i].to_json());
        }
        root[buf] = pidArray;
    }
    root["remote_address"] = m_pSource->m_strRemoteAddress;
    root["remote_port"] = m_pSource->m_nRemotePort;
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_PROGRAM_INFO_BRIEF,root.toStyledString().c_str(),m_pHandle->udplive_cb_data);
}

void CLiveAnalysisImpl::LiveCallBackTr101290()
{
    string json = m_pTrView->ReadResultToJson();
    ((easyice_udplive_callback)m_pHandle->udplive_cb_func)(UDPLIVE_CALLBACK_TR101290,json.c_str(),m_pHandle->udplive_cb_data);
}


