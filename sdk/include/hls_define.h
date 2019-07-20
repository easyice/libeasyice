/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef HLS_DEFINE_H
#define HLS_DEFINE_H





typedef enum _HLS_REPORT_TYPE_T
{
	//SegmentQuality
	EM_HRT_SQ,
	EM_HRT_SQ_STARTPAT,
	EM_HRT_SQ_VIDEOBOUNDARY,
	EM_HRT_SQ_AUDIOBOUNDARY,
	EM_HRT_SQ_CC,
	EM_HRT_SQ_TC,
	EM_HRT_SQ_ATLEASTONEIFRAME,
	EM_HRT_SQ_STARTIFRAME,


	//Diagnosis
	EM_HRT_DIAGNOSIS,
	EM_HRT_DIAGNOSIS_SPTS,
	EM_HRT_DIAGNOSIS_AUDIOTYPE,
	EM_HRT_DIAGNOSIS_VIDEOTYPE,
	//EM_HRT_DIAGNOSIS_CODECSTYPE,
	EM_HRT_DIAGNOSIS_HASNULLPKT,
	EM_HRT_DIAGNOSIS_HASBFRAME,


	//Protocol
	EM_HRT_PROTOCOL,
	EM_HRT_PROTOCOL_HLS,
	EM_HRT_PROTOCOL_HLS_UNDERFLOW,
	EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD,
	EM_HRT_PROTOCOL_HLS_WAITTING_DOWNLOAD_COUNT,
	EM_HRT_PROTOCOL_HLS_DOWNLOAD_ERRORS,
	EM_HRT_PROTOCOL_HLS_M3U8,

	EM_HRT_PROTOCOL_HTTP,
	EM_HRT_PROTOCOL_HTTP_DOWNLOADING,
	EM_HRT_PROTOCOL_HTTP_DOWNLOAD_HISTORY,
	EM_HRT_PROTOCOL_HTTP_HEAD_TS,
	EM_HRT_PROTOCOL_HTTP_HEAD_M3U8,

	//System message
	EM_HRT_SYS,
	EM_HRT_SYS_ANALYSISING,
	EM_HRT_SYS_WAITTING_FIRST_SEGMENT,//only for live
	EM_HRT_SYS_FINISH,

	//other
	EM_HRT_SEGMENT_MEDIAINFO,
	EM_HRT_SEGMENT_PIDLIST,


	EM_HRT_LAST

}HLS_REPORT_TYPE_T;

typedef struct _HLS_REPORT_PARAM_T
{
	HLS_REPORT_TYPE_T emType;
	void *pParam;
	void* pApp;
	long long llOccTime;//second
}HLS_REPORT_PARAM_T;

typedef void (*PF_HLS_REPORT_CB)(HLS_REPORT_PARAM_T param);




typedef struct _HLS_RECORD_INIT_PARAM_T
{
        //_HLS_RECORD_INIT_PARAM_T()
        //{
        //        realtime_mode = false;
		//		continue_mode = false;
        //}
        //string url;
        char* savepath;
        char*  savefile;
		char*  outname;
        bool realtime_mode;
		bool continue_mode;
}HLS_RECORD_INIT_PARAM_T;




#define LIB_HLS_ANALYSIS_CALL_PASSWD 0x1F
//////////////////////////////////////////////////////////////////////////

#endif

