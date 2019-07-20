/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef EASYICE_SDK_DEFS_H
#define EASYICE_SDK_DEFS_H

#include <stdio.h>

//进度回调
typedef void (*easyice_progress_callback)(int pct,void *pApp);



typedef enum _EASYICEinfo
{
    EASYICEINFO_HLS_BUFFERDURATION,
    EASYICEINFO_UNKNOWN
}EASYICEinfo;

typedef struct _HlsBufferDuration
{
    double duration; //当前缓冲剩余的可播放时长，单位：秒
    long long cur_time;//gettimeofday 返回的 结果，单位： usec 微秒
}HlsBufferDuration;


typedef struct _EASYICE
{
    char mrl[1024];
    char local_ip[32];//used for udplive analysis
    int ret;

    char b_file_check_park;

    void *udplive_handle;

    void *progress_cb_func;
    void *progress_cb_data;

    void *udplive_cb_func;
    void *udplive_cb_data;

    int udplive_probe_buf_size;//used for udplive analysis (bytes)
    int udplive_cb_update_interval;//used for udplive analysis (usec)
    int udplive_calctsrate_interval_ms;////used for udplive analysis

    void* hls_handle;
    void *hls_cb_func;
    void *hls_cb_data;
    HlsBufferDuration bufferduration;
}EASYICE;

typedef enum _EASYICEcode
{
    EASYICECODE_OK,
    EASYICECODE_ERROR
}EASYICEcode;

typedef enum _EASYICEopt
{
    EASYICEOPT_MRL,
    EASYICEOPT_PROGRESS_FUNCTION,
    EASYICEOPT_PROGRESS_DATA,
    EASYICEOPT_UDPLIVE_FUNCTION,
    EASYICEOPT_UDPLIVE_DATA,
    EASYICEOPT_UDPLIVE_PROBE_BUF_SIZE,
    EASYICEOPT_UDPLIVE_CB_UPDATE_INTERVAL,
    EASYICEOPT_UDPLIVE_CALCTSRATE_INTERVAL, //支持动态更新
    EASYICEOPT_UDPLIVE_START_RECORD, //支持动态更新
    EASYICEOPT_UDPLIVE_STOP_RECORD, //支持动态更新
    EASYICEOPT_UDPLIVE_LOCAL_IP, 
    EASYICEOPT_HLS_FUNCTION,
    EASYICEOPT_HLS_DATA,
    EASYICEOPT_UNKNOWN
}EASYICEopt;


struct protocal_t {
    const char    *ptr;
    int        len;
};

const struct protocal_t support_protocals[] = 
{
    {"file://",        7},
    {"udp://",         6},
    {"rtp://",         6},
    {"http://",        7},
    {"https://",       8},
    {NULL,         0}
};

enum
{
    PROTOCAL_FILE,
    PROTOCAL_UDP,
    PROTOCAL_RTP,
    PROTOCAL_HTTP,
    PROTOCAL_HTTPS,
    PROTOCAL_UNKNOWN
};


typedef enum _UDPLIVE_CALLBACK_TYPE
{
   UDPLIVE_CALLBACK_MEDIAFINO, //只调用一次
   UDPLIVE_CALLBACK_FFPROBE, //只调用一次
   UDPLIVE_CALLBACK_PIDS,
   UDPLIVE_CALLBACK_PSI,//只调用一次
   UDPLIVE_CALLBACK_TR101290,
   UDPLIVE_CALLBACK_PCR,
   UDPLIVE_CALLBACK_RATE,
   UDPLIVE_CALLBACK_PROGRAM_INFO_BRIEF,
   UDPLIVE_CALLBACK_UNKNOW
}UDPLIVE_CALLBACK_TYPE;

//UDP 直播分析信息回调
typedef void (*easyice_udplive_callback)(UDPLIVE_CALLBACK_TYPE type,const char* json,void *pApp);


#endif


