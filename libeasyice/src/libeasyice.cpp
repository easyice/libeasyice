/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "libeasyice.h"
#include "EasyICEDLL/FileAnalysis.h"
#include "EasyICEDLL/EiLog.h"
#include "EasyICEDLL/LiveAnalysis.h"
#include "HlsAnalysis.h"

static char* log_buffer = NULL;
static char* VERSION = "1.0.3";

static void DemoLogCb(int level,const char* module,const char* sInfo,void *pApp)
{
   // CLogView* lpthis = (CLogView*)pApp;
    int len= 0;

    time_t timep;
    time(&timep);
    struct tm* p = localtime(&timep);
    len = sprintf(log_buffer,"%d/%02d/%02d %02d:%02d:%02d ",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

    switch(level)
    {
        case LV_DEBUG:
            snprintf(log_buffer+len,LOG_BUFFER_SIZE-len,"[%s] DEBUG:  %s\r\n",module,sInfo);
            break;
        case LV_INFO:
            snprintf(log_buffer+len,LOG_BUFFER_SIZE-len,"[%s] INFO:  %s\r\n",module,sInfo);
            break;
        case LV_WARNING:
            snprintf(log_buffer+len,LOG_BUFFER_SIZE-len,"[%s] WARNING:  %s\r\n",module,sInfo);
            break;
        case LV_ERROR:
            snprintf(log_buffer+len,LOG_BUFFER_SIZE-len,"[%s] ERROR:  %s\r\n",module,sInfo);
            break;
        default:
            snprintf(log_buffer+len,LOG_BUFFER_SIZE-len,"[%s]: %s\r\n",module,sInfo);
            break;
    }

    cerr <<  log_buffer << endl;
   // lpthis->m_EditLogger.AddText(log_buffer);
}

void easyice_global_init()
{
    log_buffer = new char[LOG_BUFFER_SIZE];
    memset(log_buffer,0,LOG_BUFFER_SIZE);
    easyice_global_setlogfuncallback(DemoLogCb,NULL);

    ei_log(LV_DEBUG,"libeasyice","api called: easyice_global_init,version %s",VERSION);
}

void easyice_global_cleanup()
{
    ei_log(LV_DEBUG,"libeasyice","api called: easyice_global_cleanup");
    CEiLog::GetInstancePtr()->Destroy();
    tables::CDescriptor::GetInstancePtr()->Destroy();
    delete [] log_buffer;
}

EASYICE* easyice_init()
{
    EASYICE* p = new EASYICE();
    memset(p,0,sizeof(EASYICE));
    strcpy(p->local_ip,"0.0.0.0");

    p->udplive_probe_buf_size = 3850240;
    p->udplive_cb_update_interval= 1000000; //1s
    p->udplive_calctsrate_interval_ms = 1000;//1s
    //memset(p->mrl,0,sizeof(p->mrl));
    //p->b_file_check_park = 0;
    //p->progress_cb_func = NULL;
    //p->progress_cb_data = NULL;
    ei_log(LV_DEBUG,"libeasyice","api called: easyice_init");
    return p;
}

void easyice_cleanup(EASYICE* handle)
{
    ei_log(LV_DEBUG,"libeasyice","api called: easyice_cleanup");
    if (handle->udplive_handle != NULL)
    {
        CLiveAnalysis* p = (CLiveAnalysis*) handle->udplive_handle;
        p->Stop();
        delete p;
    }

    if (handle->hls_handle != NULL)
    {
        CHlsAnalysis* p = (CHlsAnalysis*) handle->hls_handle;
        p->Stop();
        delete p;
        
    }
    delete handle;
}


/**
 *ref : /Users/zhangchao-so/Develop/git/curl/lib/setopt.c
 * **/

static int setopt(EASYICE* handle,EASYICEopt option,va_list param)
{
    //必须注释掉下面的语句，多余的 va_arg 会导致后面取不到
   // long arg = va_arg(param, long);
   // char* argptr = va_arg(param, char *);
   // void* fun_cb = va_arg(param, void *);
   //arg = va_arg(param, long); 
    switch (option)
    {
        case EASYICEOPT_MRL:
            strncpy(handle->mrl,va_arg(param, char *),sizeof(handle->mrl));
            break;
        case EASYICEOPT_PROGRESS_FUNCTION:
            handle->progress_cb_func = va_arg(param, void *);
            break;
        case EASYICEOPT_PROGRESS_DATA:
            handle->progress_cb_data = va_arg(param, void *);
            break;
        case EASYICEOPT_UDPLIVE_FUNCTION:
            handle->udplive_cb_func= va_arg(param, void *);
            break;
        case EASYICEOPT_UDPLIVE_DATA:
            handle->udplive_cb_data= va_arg(param, void *);
            break;
        case EASYICEOPT_UDPLIVE_PROBE_BUF_SIZE:
            handle->udplive_probe_buf_size = va_arg(param, int);
            break;
        case EASYICEOPT_UDPLIVE_CB_UPDATE_INTERVAL:
            handle->udplive_cb_update_interval= va_arg(param, int);
            break;
        case EASYICEOPT_UDPLIVE_CALCTSRATE_INTERVAL:
            handle->udplive_calctsrate_interval_ms = va_arg(param, int);
            break;
        case EASYICEOPT_UDPLIVE_START_RECORD:
            {
                CLiveAnalysis* p = (CLiveAnalysis*)handle->udplive_handle;
                char* recordfile = va_arg(param, char*);
                p->StartRecord(recordfile);
                break;
            }
        case EASYICEOPT_UDPLIVE_STOP_RECORD:
            {
                CLiveAnalysis* p = (CLiveAnalysis*)handle->udplive_handle;
                p->StopRecord();
                break;
            }
        case EASYICEOPT_UDPLIVE_LOCAL_IP:
            {
                strncpy(handle->local_ip,va_arg(param, char *),sizeof(handle->local_ip));
                break;
            }
        case EASYICEOPT_HLS_FUNCTION:
            handle->hls_cb_func= va_arg(param, void *);
            break;
        case EASYICEOPT_HLS_DATA:
            handle->hls_cb_data= va_arg(param, void *);
            break;
        default:
            break;
    }
    return 0;
}

static int easyice_setopt_inner(EASYICE* handle,EASYICEopt option, ...)
{
    va_list arg;

    if(handle == NULL)
        return -1;

    va_start(arg, option);

    int result = setopt(handle, option, arg);

    va_end(arg);
    return result;
}

int easyice_setopt( EASYICE* handle,EASYICEopt option, void* val)
{
    ei_log(LV_DEBUG,"libeasyice","api called: setopt");
    return easyice_setopt_inner(handle,option,val);
}

EASYICEcode easyice_process(EASYICE* handle)
{
    ei_log(LV_DEBUG,"libeasyice","api called: easyice_process");
    if (strncasecmp(handle->mrl,support_protocals[PROTOCAL_FILE].ptr,support_protocals[PROTOCAL_FILE].len) == 0)
    {
        memmove(handle->mrl,handle->mrl+support_protocals[PROTOCAL_FILE].len,strlen(handle->mrl)+1-support_protocals[PROTOCAL_FILE].len);
        FileAnalysis* p = new FileAnalysis();
        p->OpenMRL(handle);
        delete p;
    }
    else if (strncasecmp(handle->mrl,support_protocals[PROTOCAL_UDP].ptr,support_protocals[PROTOCAL_UDP].len) == 0)
    {
        CLiveAnalysis* p = new CLiveAnalysis();
        handle->udplive_handle = p;
        p->OpenMRL(handle);
    }
    else if (strncasecmp(handle->mrl,support_protocals[PROTOCAL_RTP].ptr,support_protocals[PROTOCAL_RTP].len) == 0)
    {
    }
    else if (strncasecmp(handle->mrl,support_protocals[PROTOCAL_HTTP].ptr,support_protocals[PROTOCAL_HTTP].len) == 0 ||
               strncasecmp(handle->mrl,support_protocals[PROTOCAL_HTTPS].ptr,support_protocals[PROTOCAL_HTTPS].len) == 0)
    {
        CHlsAnalysis* p = new CHlsAnalysis();
        p->SetReportCB((PF_HLS_REPORT_CB)handle->hls_cb_func,handle->hls_cb_data);
        handle->hls_handle = p;
        p->OpenMRL(handle);
    }
    return EASYICECODE_OK;
}

int easyice_getinfo(EASYICE* handle,EASYICEinfo info,void* val)
{
    ei_log(LV_DEBUG,"libeasyice","api called: easyice_getinfo");
    switch(info)
    {
        case EASYICEINFO_HLS_BUFFERDURATION:
            {
                CHlsAnalysis* p = (CHlsAnalysis*) handle->hls_handle;
                p->GetBufferDuration(handle->bufferduration.duration,handle->bufferduration.cur_time);
		HlsBufferDuration** phbd = (HlsBufferDuration**)val;
                *phbd = &(handle->bufferduration);
                break;
            }
        default:
            break;
    }
    return EASYICECODE_OK;
}






