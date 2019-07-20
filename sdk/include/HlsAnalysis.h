/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 HLSANALYSIS_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// HLSANALYSIS_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifndef HLS_ANALYSIS_H
#define HLS_ANALYSIS_H


#include "sdkdefs.h"
#include "hls_define.h"










class CHlsAnalysisImpl;

// 此类是从 HlsAnalysis.dll 导出的 每个任务对应一个单独的类对象
class CHlsAnalysis {
public:
	CHlsAnalysis(void);
	~CHlsAnalysis();


    //成功返回 0 ，否则返回非零值
    int OpenMRL(EASYICE* handle);

	//释放资源，失败返回false
	bool Stop(bool bForce = false); 


	//获取当前缓冲内容可播放的时长
	bool GetBufferDuration(double& duration,long long& llCurTime);


	void SetProxy(const char* strProxy);
	/***********************for record****************************/

	//开始录制码流,成功返回0，失败返回非零
	int StartRecord(const char* strFileName);

	//停止录制码流
	void StopRecord();

	void SetRecord(const HLS_RECORD_INIT_PARAM_T& rc_param);

	void SetReportCB(PF_HLS_REPORT_CB pCB,void* pApp);
private:
	CHlsAnalysisImpl* m_pHlsAnalysisImpl;
};







//只为使用EasyiceDLL中 的PID统计代码
//extern void LibHlsAnalysisSetMsgWnd(HWND hwnd);





extern int nHlsAnalysis;













#endif

