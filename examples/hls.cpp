/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "libeasyice.h"
#include <unistd.h>
#include "hls_define.h"
#include <iostream>
#include "string_res.h"
#include <vector>

using namespace std;

#define MAX_ITEM_CNT 50

typedef enum _LIST_ITEM_T
{
    LI_HLS_SQ_PATPMT = 0,
    LI_HLS_SQ_KEYFRAME_START,
    LI_HLS_SQ_COMPLETE_VF,
    LI_HLS_SQ_COMPLETE_AF,
    LI_HLS_SQ_CC,
    LI_HLS_SQ_TC,
    LI_HLS_SQ_ONE_KEYFRAME
}LIST_ITEM_T;







int* m_pErrCnt;
vector<string>  m_vecOccTime;
vector<string> m_vecResStr;

//切片质量回调信息,回调信息意味着发现了某个检测项不合标准
//(char*)param->pParam 事件对应 的切片 URL
void OnReportHlsQuality(const char* strtime,const HLS_REPORT_PARAM_T* param)
{
    switch (param->emType)
    {
        case EM_HRT_SQ_STARTPAT: //以 PAT，PMT 开始
            m_pErrCnt[LI_HLS_SQ_PATPMT]++;
            m_vecOccTime[LI_HLS_SQ_PATPMT] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_PATPMT].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_PATPMT,(char*)param->pParam);
            break;
        case EM_HRT_SQ_VIDEOBOUNDARY: //完整的视频帧
            m_pErrCnt[LI_HLS_SQ_COMPLETE_VF]++;
            m_vecOccTime[LI_HLS_SQ_COMPLETE_VF] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_COMPLETE_VF].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_COMPLETE_VF,(char*)param->pParam);
            break;
        case EM_HRT_SQ_AUDIOBOUNDARY: //完整的音频帧
            m_pErrCnt[LI_HLS_SQ_COMPLETE_AF]++;
            m_vecOccTime[LI_HLS_SQ_COMPLETE_AF] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_COMPLETE_AF].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_COMPLETE_AF,(char*)param->pParam);
            break;
        case EM_HRT_SQ_CC: //TS 包连续性
            m_pErrCnt[LI_HLS_SQ_CC]++;
            m_vecOccTime[LI_HLS_SQ_CC] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_CC].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_CC,(char*)param->pParam);
            break;
        case EM_HRT_SQ_TC: //时间戳连续性
            m_pErrCnt[LI_HLS_SQ_TC]++;
            m_vecOccTime[LI_HLS_SQ_TC] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_TC].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_TC,(char*)param->pParam);
            break;
        case EM_HRT_SQ_ATLEASTONEIFRAME: //至少1个 I 帧
            m_pErrCnt[LI_HLS_SQ_ONE_KEYFRAME]++;
            m_vecOccTime[LI_HLS_SQ_ONE_KEYFRAME] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_ONE_KEYFRAME].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_ONE_KEYFRAME,(char*)param->pParam);
            break;
        case EM_HRT_SQ_STARTIFRAME: //以关键帧开始
            m_pErrCnt[LI_HLS_SQ_KEYFRAME_START]++;
            m_vecOccTime[LI_HLS_SQ_KEYFRAME_START] = strtime;
            printf("%s,%s\n",m_vecResStr[LI_HLS_SQ_KEYFRAME_START].c_str(),(char*)param->pParam);
            //OnAddRelevantSegments(LI_HLS_SQ_KEYFRAME_START,(char*)param->pParam);
            break;
        default:
            break;
    }

}

//协议分析，HLS 部分
void OnReportProtocolHls(const HLS_REPORT_PARAM_T* param)
{

    switch(param->emType)
    {
        case EM_HRT_SYS_FINISH:
            printf("analysisi finish.\n");
            //((CHlsBufferView*)m_pView)->OnFinish();
            break;
        case EM_HRT_PROTOCOL_HLS_UNDERFLOW: //产生下溢
            break;
        case EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD_COUNT: //待下载 TS 数量
            {
                printf("wating downlaod count=%d\n",*(long long*)(param->pParam));
            }
            break;
        case EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD: //待下载的 TS
            {
                char* str = (char*) param->pParam;
                printf("waiting downlaod:%s\n",str);
            }
            break;
        case EM_HRT_PROTOCOL_HLS_DOWNLOAD_ERRORS: //TS 下载失败
            {
                char* str = (char*) param->pParam;
                printf("downlaod error:%s\n",str);
            }
            break;
        case EM_HRT_PROTOCOL_HLS_M3U8: //m3u8 内容
            {            
                //string out_str;
                //utils_ntorn((char*) param->pParam,out_str);

                printf("m3u8 content:%s\n",(char*) param->pParam);
            }
            break;
        default:
            break;
    }
}



//协议分析，HTTP 部分
void OnReportProtocolHttp(const HLS_REPORT_PARAM_T* param)
{

    switch(param->emType)
    {
        case EM_HRT_PROTOCOL_HTTP_DOWNLOADING: //正在下载
            printf("donwloading:%s\n",(char*)param->pParam);
            break;
        case EM_HRT_PROTOCOL_HTTP_DOWNLOAD_HISTORY: //下载历史
            printf("donwload  history:%s\n",(char*)param->pParam);
            break;
        case EM_HRT_PROTOCOL_HTTP_HEAD_TS: //下载 TS 请求 Response 信息
            {
                printf("EM_HRT_PROTOCOL_HTTP_HEAD_TS:%s\n",(char*) param->pParam);
                //string out_str;
                //utils_ntorn((char*) param->pParam,out_str);
                break;
            }
        case EM_HRT_PROTOCOL_HTTP_HEAD_M3U8: //下载 m3u8 请求 Response 信息
            {
                printf("EM_HRT_PROTOCOL_HTTP_HEAD_M3U8:%s\n",(char*) param->pParam);
                //string out_str;
                //utils_ntorn((char*) param->pParam,out_str);

                break;
            }
        default:
            break;
    }
}

//回调总入口
void demo_callback(HLS_REPORT_PARAM_T param)
{
    //事件产生的绝对时间
    char *strtime = new char[2048];
    time_t ltime= (time_t)param.llOccTime;
    struct tm* p = localtime(&ltime);
    sprintf(strtime,("%d/%02d/%02d %02d:%02d:%02d "),1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

    if (param.emType == EM_HRT_SEGMENT_MEDIAINFO)
    {
        printf("============mediainfo============%s\n",(char*)param.pParam);
        return;	
    }
    if (param.emType == EM_HRT_SEGMENT_PIDLIST)
    {
        printf("============pidlist============%s\n",(char*)param.pParam);
        return;	
    }
    //诊断信息
    if (param.emType > EM_HRT_DIAGNOSIS && param.emType < EM_HRT_PROTOCOL)
    {
        printf("%s,%s\n",strtime,(char*)param.pParam);
        //m_pHlsDiagnosisFrame->OnReport(param);
        return;
    }

    //HLS 质量
    if (param.emType > EM_HRT_SQ && param.emType < EM_HRT_DIAGNOSIS)
    {
        OnReportHlsQuality(strtime,&param);
        //m_pHlsQualityFrame->OnReport(param);
        return;
    }

    ///协议分析-HLS
    if (param.emType > EM_HRT_PROTOCOL && param.emType < EM_HRT_PROTOCOL_HTTP)
    {
        OnReportProtocolHls(&param);
        //m_pHlsProtocolHlsFrame->OnReport(param);
        return;
    }

    //协议分析-HTTP
    if (param.emType > EM_HRT_PROTOCOL_HTTP && param.emType < EM_HRT_SYS)
    {
        OnReportProtocolHttp(&param);
        //m_pHlsProtocolHttpFrame->OnReport(param);
        return;
    }

    switch(param.emType)
    {
        case EM_HRT_SYS_ANALYSISING:
            printf("analysing...\n");
            //AfxGetMainWnd()->SendMessage(MESSAGE_TASK_STATUS,0,STATUS_ANALYSING);
            break;
        case EM_HRT_SYS_WAITTING_FIRST_SEGMENT:
            printf("watting first segment...\n");
            //AfxGetMainWnd()->SendMessage(MESSAGE_TASK_STATUS,0,STATUS_WAITTING_FIRST_SEGMENT);
            break;
        case EM_HRT_SYS_FINISH:
            printf("analysis finish.\n");

            //AfxGetMainWnd()->SendMessage(MESSAGE_TASK_STATUS,0,STATUS_FINISH);
            OnReportProtocolHls(&param);
            //m_pHlsProtocolHlsFrame->OnReport(param);
        default:
            break;
    }



    delete [] strtime;
}

int main(int argc,char** argv)
{
    m_pErrCnt = new int[MAX_ITEM_CNT];
    m_vecOccTime.resize(MAX_ITEM_CNT);
    m_vecResStr.resize(MAX_ITEM_CNT);

    m_vecResStr[LI_HLS_SQ_PATPMT] = IDS_HLS_SQ_PATPMT;
    m_vecResStr[LI_HLS_SQ_KEYFRAME_START] = IDS_HLS_SQ_KEYFRAME_START;
    m_vecResStr[LI_HLS_SQ_COMPLETE_VF] = IDS_HLS_SQ_COMPLETE_VF;
    m_vecResStr[LI_HLS_SQ_COMPLETE_AF] = IDS_HLS_SQ_COMPLETE_AF;
    m_vecResStr[LI_HLS_SQ_CC] = IDS_HLS_SQ_CC;
    m_vecResStr[LI_HLS_SQ_TC] = IDS_HLS_SQ_TC;
    m_vecResStr[LI_HLS_SQ_ONE_KEYFRAME] = IDS_HLS_SQ_ONE_KEYFRAME;

    const char* pdata = "pdata";
    easyice_global_init();

    EASYICE* handle  = easyice_init();
    easyice_setopt(handle,EASYICEOPT_MRL, (void*)"https://devstreaming-cdn.apple.com/videos/streaming/examples/img_bipbop_adv_example_ts/v2/prog_index.m3u8");
    easyice_setopt(handle,EASYICEOPT_HLS_FUNCTION, (void*)demo_callback);
    easyice_setopt(handle,EASYICEOPT_HLS_DATA, (void*)"pdata");

    
    //执行后立即返回，后台线程开始启动分析任务
    EASYICEcode ret = easyice_process(handle);
    if (ret != EASYICECODE_OK)
    {
    	printf("process error\n");
        _exit(-1);
    }

//    easyice_setopt(handle,EASYICEOPT_UDPLIVE_START_RECORD, (void*)"/tmp/r.ts");

    //stop analysis after 10s
    sleep(10);

    //定时调用下面的函数，获取缓冲分析中的相关信息
    HlsBufferDuration *hbd;
    easyice_getinfo(handle,EASYICEINFO_HLS_BUFFERDURATION,(void*)&hbd);
    printf("HlsBufferDuration:%f,%lld\n",hbd->duration,hbd->cur_time);
    
  //  easyice_setopt(handle,EASYICEOPT_UDPLIVE_STOP_RECORD,NULL);
    
    easyice_cleanup(handle);
    easyice_global_cleanup();
    printf("success\n");
    return 0;
}
