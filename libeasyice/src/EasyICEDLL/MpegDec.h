/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <stdio.h>
#include <map>
#include "TsPacket.h"
#include "tables/CAnalyzeTable.h"
#include "../sdkdefs.h"

using namespace std;
using namespace tables;

typedef struct _PIDINFO
{
	_PIDINFO()
	{
		pid = -1;
	}
	int pid;
	__int64 count;
}PIDINFO;


class CDemuxTs;
class CMpegDec
{
public:
	CMpegDec(void);
public:
	~CMpegDec(void);
public:
    

    void SetProgressFunCb(easyice_progress_callback pFun,void *pApp);

    // must do probe befor processbuffer
    int ProbeMediaInfo(BYTE * pData,int length);
	int ProcessBuffer(BYTE * pData, int length);


	//必须保证能够读到数据
	//MSG_PACKET_LIST* ProcessBufferForPacketNextView(BYTE * pData, __int64 length,__int64 offset);
	void Init(int nTsLength,long long nPacket_Total);

	void LiveInit(int nTsLength);


	void LiveProcessPacket(BYTE* pPacket);

	//只统计PID
	void LiveProcessPacket2(BYTE* pPacket);

    //callit on the end
    void Finish();    

	bool GetPacketHeadInfo(BYTE* pData,int nLength,MSG_FLAG_LIST* headInfo,char* strDescriptor,int descriptorLen);	//

	MSG_PACKET_LIST* GetPacketListBuf(long long &nCount);

	//获取解析的节目信息
	ALL_PROGRAM_INFO* GetAllProgramInfo() {return &m_allProgramInfo;}

	//获取节目摘要信息
	ALL_PROGRAM_BRIEF* GetAllProgramBrief() { return &m_allProgramBrief; }

	std::vector<PID_TYPE>* GetProgPidNoPsi();

	//立刻取当前解析到的PAT，PMT作为节目信息，为何要外界来通知呢，是因考虑到PMT不全的时候，需要外界控制超时时间
	void FillPacketType();

    //更新计算一下 PID 列表
	void UpdatePidListResult();

private:


	int ProcessPacket(BYTE * pData);

	//pid类型中，加入动态解析出的pmtpid以及从pmt解析出的其他流pid
	//m_allProgramBrief也在这里填充了
	void FillPacketType(TABLES* tables);

private:


	int m_nTsLength;	//188 or 204
	long long m_Packet_Counter; //处理过了多少个包,用于计算进度
	long long m_Packet_Total;	//总共需要处理多少个包，用于计算进度

    easyice_progress_callback m_pfProgressCb;
    void* m_pProgressApp;

	int m_nPct; //当前进度指示
	int m_nVideoPID;//只当psi表明只有1路节目或没有psi信息时使用
	int m_nAudioPID;//只当psi表明只有1路节目或没有psi信息时使用
	int m_nPcrPID;//只为没有psi信息时，播发器使用

	//视频流类型
	int m_nVideoStream_type;



	//没有找到PAT将在对包类型判断时采用包解析模式
	bool m_bHavePAT;

	//只在 GetProgPidNoPsi 接口使用
	std::vector<PID_TYPE> m_vecPids;

public:
	//解复用器
	CDemuxTs* m_pDemuxTs;
	
    //表解析器
	CAnalyzeTable m_TableAnalyzer;

	
	//存储PID列表,这里是最终结果
	std::map<int,MSG_PID_LIST> m_mapPidList;

/**
	* 存储区域
*/
private:
	//用于保存pid类型.pid,type
	std::map<int,PACKET_TYPE> m_mapPacketType;


	//由于效率的关系没有用map直接存储，下面的方式最快。支持到1000个pid
	PIDINFO m_pPidInfo[1000];
	

	//所有节目的信息
	ALL_PROGRAM_INFO m_allProgramInfo;

	//节目概要信息，包含每路节目的pid，以及pid类型描述
	ALL_PROGRAM_BRIEF m_allProgramBrief;

	//解析模块启用情况
	bool m_bDemuxAnaEnable;
	bool m_bTableAnaEnable;
	bool m_bPidAnaEnable;



/**
	* 私有函数
*/
private:
	void InitPacketType();

	//以下是工作者线程需要调用的函数
	
	//统计pid信息
	void CalcPidList(int pid);

	//在没有PAT、PMT信息的情况下，检测包类型
	void GetPESType(CTsPacket *tsPacket,FRAME_TYPE& FrameType);

	//没有psi信息时使用
	bool ParsePidNoPsi(BYTE * pData, __int64 length,int& nVideoPID,int& nAudioPID,int& nPcrPID);

	void CheckFrameType(CTsPacket *tsPacket,FRAME_TYPE& FrameType,int VideoStreamType);

};
