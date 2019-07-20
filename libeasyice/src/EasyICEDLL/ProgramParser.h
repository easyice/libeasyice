/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "TsPacket.h"
#include "H264Dec.h"
#include <iostream>
#include "jmdec.h"

using namespace std;

#define MAXSPS  32
#define MAXPPS  256


///@brief	解析单路节目。统计pts，dts，pcr等
class CProgramParser
{
public:
	CProgramParser(int nTsLength);
	~CProgramParser(void);
public:
	/**
	* 需要保证传入的TS包是本路节目的。
	*/
	PARSED_FRAME_INFO PushBackTsPacket(CTsPacket* tsPacket,long long packetID);

	//设置解析结果信息存储缓冲
	void SetOutputBuffer(PROGRAM_INFO* p) { m_pProgInfo = p; }

	//设置视频pid及其类型（目前支持解码的）
	void SetVideoStreamInfo(PID_STREAM_TYPE video_pid_type);

	//如果村子啊多个音频PID，仅支持1个
	void SetAudioPid(int pid);

	//解码一个TS包
	int DecodePacket(CTsPacket* tsPacket);

	//加入一个非本节目PID的包
	void AddOtherPacket();
private:
	//组合GOP
	void MPEG2_AssembleGop(BYTE bPic);

	void H264_AssembleGop(FRAME_TYPE frame_type);

	//计算码率
	inline void MakeRate(long long pcr);
private:
	//本节目的TS包计数器
	unsigned long long m_llTotalPacketCounter;

	//各种时间戳信息
	

	//当前pcr，pts，dts信息
	long long  m_pcr;
	long long  m_Vpts;
	long long  m_Apts;
	long long  m_dts;

	//最后一个pcr值
	//TIMESTAMP m_lastPcrtp;

	//组GOP的临时字符串
	string m_gopTmp;
	int m_nGopsizeTmp;
	int m_nGopBytesTmp;

	//存储解析出的节目信息
	PROGRAM_INFO* m_pProgInfo;


	//两PCR间包计数
	int m_nPacketCountOfPcr;

	//上一个PCR
	long long m_pcrBefor;

	//视频pid及其类型
	PID_STREAM_TYPE m_video_pid_type;

	int m_nAudioPid;

	//H264语法解析器
	CH264Dec m_avcParser;
	

	int m_nTsLength;
private:
	//13818-2 p22 传送速率(字节率)
	double m_fTransportRate;

private:

};
