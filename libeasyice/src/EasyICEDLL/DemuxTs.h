/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "TsPacket.h"
#include "ProgramParser.h"
#include "tables/CAnalyzeTable.h"
#include <map>

using namespace std;
using namespace tables;

//单路模式下的program_id值
#define SINGLE_MODE_PROG_NUM	-1

class CDemuxTs
{
public:
	CDemuxTs(void);
	~CDemuxTs(void);
public:
	//初始化解复用器，预分析一段缓冲来对ts解复用
	/**
		@brif 从给定缓冲中解析节目的pid信息.并保存到m_vecPrograms
		@return 成功返回1，失败返回0
	*/
	int SetupDemux(BYTE* pData, int nLength,int nTsLen);

	int SetupDemux(TABLES* tables,int nTsLen);

	//单路处理模式
	void SetupDemux(int VideoPID,int VideoStreamType,int nTsLen);

	//设置解析结果信息存储缓冲
	void SetOutputBuffer(ALL_PROGRAM_INFO* p);

	//对收到的ts包进行处理
	PARSED_FRAME_INFO AddTsPacket(CTsPacket* tsPacket);

	//解码一个TS包
	int DecodePacket(BYTE *pPacket, int nLen);
private:
	//多路节目pid信息 节目号，节目信息
	map<int,PROGRAM_PIDS> m_mapProgPids;

	//每路节目与其对应的处理器,节目号，处理器
	//节目号=-1 表示单路处理模式
	map<int,CProgramParser*> m_mapProgParser;

	//所有节目的信息
	ALL_PROGRAM_INFO *m_allProgramInfo;

	bool m_bSingleMode;

	//包ID，记录这是所分析内容的第几个包，解析264语法时会用到
	long long m_llPacketID;
};
