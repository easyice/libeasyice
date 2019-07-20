/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "commondefs.h"
#include "sdkdefs.h"



class CLiveAnalysisImpl;
class CLiveAnalysis
{
public:
	CLiveAnalysis(void);
	~CLiveAnalysis(void);

	//成功返回 0 ，否则返回非零值
	int OpenMRL(EASYICE* handle);

	//释放资源，失败返回false
	bool Stop(bool bForce = false); 

	//获取节目摘要信息
	ALL_PROGRAM_BRIEF* GetAllProgramBrief();

	//实时分析时，获取码率,获取后调用方应清掉缓冲
	LST_RATE_INFO_T* LockGetRate();
	void UnlockRate();

	//实时分析时，获取PCR信息，获取后调用方应清掉缓冲
	LST_PCR_INFO_T* LockGetPcrInfo(int pcr_pid);
	void UnlockPcrInfo(int pcr_pid);

	//设置计算码率时间间隔(ms)
//	void SetCalcTsRateIntervalTime(int nTime);

	//开始录制码流,成功返回0，失败返回非零
	int StartRecord(const char* strFileName);

	//停止录制码流
	void StopRecord();
private:
	CLiveAnalysisImpl* m_pLiveAnalysisImpl;
};
