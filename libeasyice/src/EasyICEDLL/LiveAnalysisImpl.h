/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "sdkdefs.h"
#include "ztypes.h"
#include <stdio.h>
#include "zevent.h"
#include "commondefs.h"


class CMpegDec;
class CLiveSourceBase;
class CCircularBuffer;
class CLivePcrProc;
class CUdpSend;
class Clibtr101290;
class CEiMediaInfo;
class CTrView;

class CLiveAnalysisImpl
{
public:
	CLiveAnalysisImpl(void);
	~CLiveAnalysisImpl(void);
	int OpenMRL(EASYICE* handle);

	//释放资源，失败返回false
	bool Stop(bool bForce = false); 

	//获取节目摘要信息
	ALL_PROGRAM_BRIEF* GetAllProgramBrief();

	//实时分析时，获取码率
	LST_RATE_INFO_T* LockGetRate();
	void UnlockRate();

	//实时分析时，获取PCR信息
	LST_PCR_INFO_T* LockGetPcrInfo(int pcr_pid);
	void UnlockPcrInfo(int pcr_pid);


		//开始录制码流
	int StartRecord(const char* strFileName);

	//停止录制码流
	void StopRecord();

private:
	static void OnRecvData(void* pApp,BYTE* pData,int nLen);
	static void* MediaInfoThread(void* lpParam);

	//处理线程函数
	static void* WorkThread(void* lpParam);
	void WorkFun();
	void ProcessItem(BYTE* pItem,int nSize,long long llTime);

	//录制线程函数
	static void* RecordThread(void* lpParam);

    void LiveCallBackPidList();
    void LiveCallBackPsi();
    void LiveCallBackPcr();
    void LiveCallBackRate();
    void LiveCallBackProgramInfoBrief();
    void LiveCallBackTr101290();
private:
	CLiveSourceBase* m_pSource;
	CCircularBuffer* m_pCircularBuffer;

	//处理线程
	pthread_t m_hThread;
	bool m_bStop;
	int m_nTsLength;

	//媒体信息检测
	BYTE* m_pMediaInfoBuffer;
	int m_nMediaInfoBufferLen;
	bool m_bMediaInfoChecked;
	pthread_t m_hMiThread;

	//是否已初始化完毕，指是否已解析完PAT与PMT，得到了所有节目的信息
	bool m_bInited;

	//指示何时收到第一次数据
	long long m_llFirstByteRecvTime;

	//最后一次调用回调的时间
	long long m_llCbUpdateTime;

	CMpegDec *m_mpegdec;
	CLivePcrProc *m_pLiveProc;

	//发送到本地以供vlc播放
	//CUdpSend* m_pUdpSend;

	//指示是否已收到第一个字节的数据
	bool m_bRecvedFirstByte;

	FILE* m_fpRecordFile;
	pthread_mutex_t m_mutexRecordFp; //对录制文件指针的锁,保证文件指针有效性

	//录制线程
	pthread_t m_hRecordThread;

	//录制缓冲
	CCircularBuffer *m_pRecordBuffer;
	pthread_mutex_t m_mutexRecordBuf; //对录制缓冲指针的锁，保证缓冲指针的有效性,用写操作与delete操作。不管读操作。因为有其他机制保证
	bool m_bStartRecord;

    Clibtr101290* m_pTrcore;
    EASYICE* m_pHandle;
    string m_sMrl;
    ZEvent m_event;

    CEiMediaInfo* m_pEiMediaInfo;
    CTrView* m_pTrView;

    bool m_bWorkThreadValid;
    bool m_bMiThreadValid;
    bool m_bRecordThreadValid;
};
