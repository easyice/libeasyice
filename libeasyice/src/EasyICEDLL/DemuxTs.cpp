/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "DemuxTs.h"


static int TypeOfStreamType(int stream_type)
{
	if (stream_type == 0x01 || stream_type == 0x02 || stream_type == 0x1B || stream_type == 0x24)	//video
	{
		return 1;
	}
	else if (stream_type == 0x03 || stream_type == 0x04 || stream_type == 0x0F 
		||stream_type == 0x80 || stream_type == 0x81 || stream_type == 0x82
		||stream_type == 0x83 || stream_type == 0x84 || stream_type == 0x85
		||stream_type == 0x86 || stream_type == 0x8A|| stream_type == 0xA1 || stream_type == 0xA2) //audio
	{
		return 2;
	}

	return 0;
}


CDemuxTs::CDemuxTs(void)
{
	m_allProgramInfo = NULL;
	m_bSingleMode = false;
	m_llPacketID = 0;
}

CDemuxTs::~CDemuxTs(void)
{
	map<int,CProgramParser*>::iterator it = m_mapProgParser.begin();
	for (; it != m_mapProgParser.end(); it++)
	{
		delete it->second;
	}

}

int CDemuxTs::SetupDemux(BYTE* pData, int nLength,int nTsLen)
{
	CAnalyzeTable tableAnalyzer;
	tableAnalyzer.SetTsPacketLength(nTsLen);
	tableAnalyzer.ParseFindPAT(pData,nLength);
	for (int i = 0 ; i < nLength; i+= nTsLen)
	{
		CTsPacket tsPacket;
		if (tsPacket.SetPacket(pData+i))
		{
			tableAnalyzer.PushBackTsPacket(&tsPacket);
		}
	}

	TABLES* tables = tableAnalyzer.GetTables();
	map<int,STU_SECTION_PMT>::iterator it = tables->mapTabPMT.begin();
	for ( ; it != tables->mapTabPMT.end(); it++ )
	{
		PROGRAM_PIDS info;
		PID_STREAM_TYPE pid_type;

		//pat and pmt
		pid_type.pid = 0; //pat
		pid_type.stream_type = 0;
		info.pids.push_back(pid_type);

		vector<PAT_LIST>::iterator it_pat_list = (tables->vecTabPAT)[0][0].vec_pat_list.begin();
		for (; it_pat_list != (tables->vecTabPAT)[0][0].vec_pat_list.end();it_pat_list++)
		{
			if (it_pat_list->program_number == it->first)
			{
				pid_type.pid = it_pat_list->network_pmt_PID; //pmt
				pid_type.stream_type = 0;
				info.pids.push_back(pid_type);
				break;
			}
		}

		PMT& pmt = (it->second)[0];

		pid_type.pid = pmt.pcr_pid;
		pid_type.stream_type = 0;
		info.pids.push_back(pid_type);

		vector<PMT_LIST2>::iterator it_list = pmt.vec_pmt_list2.begin();
		for ( ; it_list != pmt.vec_pmt_list2.end(); it_list++ )
		{
			pid_type.pid = it_list->elementary_PID;
			pid_type.stream_type = it_list->stream_type;
			info.pids.push_back(pid_type);
		}

		m_mapProgPids.insert(map<int,PROGRAM_PIDS>::value_type(it->first,info));
		//目前先当做PMT表只有一个section 处理
		//STU_SECTION_PMT::iterator itpmt = it->second.begin();
		//for( ; itpmt != it->second.end(); itpmt++ )
		//{
		//	
		//}
	}

	
	//根据节目数量初始化存储器，解析器
	map<int,PROGRAM_PIDS>::iterator itpid = m_mapProgPids.begin();
	for( ; itpid != m_mapProgPids.end(); itpid++)
	{
		m_allProgramInfo->insert(ALL_PROGRAM_INFO::value_type(itpid->first,new PROGRAM_INFO));

		m_mapProgParser.insert(map<int,CProgramParser*>::value_type(itpid->first,new CProgramParser(nTsLen)) );

		m_mapProgParser[itpid->first]->SetOutputBuffer( (*m_allProgramInfo)[itpid->first] );

		//设置音视频流
		for (int i = 0; i < (int)itpid->second.pids.size(); i++)
		{
			if (TypeOfStreamType(itpid->second.pids[i].stream_type) == 1)
			{
				m_mapProgParser[itpid->first]->SetVideoStreamInfo(itpid->second.pids[i]);	
			}
			else if (TypeOfStreamType(itpid->second.pids[i].stream_type) == 2)
			{
				m_mapProgParser[itpid->first]->SetAudioPid(itpid->second.pids[i].pid);
			}
			//if (itpid->second.pids[i].stream_type == 0x2 ||	//MPEGV
			//	itpid->second.pids[i].stream_type == 0x1B || //H.264
			//	itpid->second.pids[i].stream_type == 0x24) //HEVC
			//{
			//	m_mapProgParser[itpid->first]->SetVideoStreamInfo(itpid->second.pids[i]);	
			//	break;
			//}
		}
		
	}
	
	return 1;
}

int CDemuxTs::SetupDemux(TABLES* tables,int nTsLen)
{
	map<int,STU_SECTION_PMT>::iterator it = tables->mapTabPMT.begin();
	for ( ; it != tables->mapTabPMT.end(); it++ )
	{
		PROGRAM_PIDS info;
		PID_STREAM_TYPE pid_type;

		//pat and pmt
		pid_type.pid = 0; //pat
		pid_type.stream_type = 0;
		info.pids.push_back(pid_type);

		vector<PAT_LIST>::iterator it_pat_list = (tables->vecTabPAT)[0][0].vec_pat_list.begin();
		for (; it_pat_list != (tables->vecTabPAT)[0][0].vec_pat_list.end();it_pat_list++)
		{
			if (it_pat_list->program_number == it->first)
			{
				pid_type.pid = it_pat_list->network_pmt_PID; //pmt
				pid_type.stream_type = 0;
				info.pids.push_back(pid_type);
				break;
			}
		}

		PMT& pmt = (it->second)[0];

		pid_type.pid = pmt.pcr_pid;
		pid_type.stream_type = 0;
		info.pids.push_back(pid_type);

		vector<PMT_LIST2>::iterator it_list = pmt.vec_pmt_list2.begin();
		for ( ; it_list != pmt.vec_pmt_list2.end(); it_list++ )
		{
			pid_type.pid = it_list->elementary_PID;
			pid_type.stream_type = it_list->stream_type;
			info.pids.push_back(pid_type);
		}

		m_mapProgPids.insert(map<int,PROGRAM_PIDS>::value_type(it->first,info));
		//目前先当做PMT表只有一个section 处理
		//STU_SECTION_PMT::iterator itpmt = it->second.begin();
		//for( ; itpmt != it->second.end(); itpmt++ )
		//{
		//	
		//}
	}

	
	//根据节目数量初始化存储器，解析器
	map<int,PROGRAM_PIDS>::iterator itpid = m_mapProgPids.begin();
	for( ; itpid != m_mapProgPids.end(); itpid++)
	{
		m_allProgramInfo->insert(ALL_PROGRAM_INFO::value_type(itpid->first,new PROGRAM_INFO));

		m_mapProgParser.insert(map<int,CProgramParser*>::value_type(itpid->first,new CProgramParser(nTsLen)) );

		m_mapProgParser[itpid->first]->SetOutputBuffer( (*m_allProgramInfo)[itpid->first] );

		//设置音视频流
		for (int i = 0; i < (int)itpid->second.pids.size(); i++)
		{
			if (TypeOfStreamType(itpid->second.pids[i].stream_type) == 1)
			{
				m_mapProgParser[itpid->first]->SetVideoStreamInfo(itpid->second.pids[i]);
			}
			else if (TypeOfStreamType(itpid->second.pids[i].stream_type) == 2)
			{
				m_mapProgParser[itpid->first]->SetAudioPid(itpid->second.pids[i].pid);
			}
			//if (itpid->second.pids[i].stream_type == 0x2 ||	//MPEGV
			//	itpid->second.pids[i].stream_type == 0x1B || //H.264
			//	itpid->second.pids[i].stream_type == 0x24) //HEVC	
			//{
			//	m_mapProgParser[itpid->first]->SetVideoStreamInfo(itpid->second.pids[i]);	
			//	break;
			//}
		}
		
	}
	
	return 1;
}

void CDemuxTs::SetupDemux(int VideoPID,int VideoStreamType,int nTsLen)
{
	m_allProgramInfo->insert(ALL_PROGRAM_INFO::value_type(SINGLE_MODE_PROG_NUM,new PROGRAM_INFO));
	m_mapProgParser.insert(map<int,CProgramParser*>::value_type(SINGLE_MODE_PROG_NUM,new CProgramParser(nTsLen)) );
	m_mapProgParser[SINGLE_MODE_PROG_NUM]->SetOutputBuffer( (*m_allProgramInfo)[SINGLE_MODE_PROG_NUM] );

	PID_STREAM_TYPE pid_type;
	pid_type.pid = VideoPID;
	pid_type.stream_type = VideoStreamType;
	m_mapProgParser[SINGLE_MODE_PROG_NUM]->SetVideoStreamInfo(pid_type);	

	m_bSingleMode = true;
}

void CDemuxTs::SetOutputBuffer(ALL_PROGRAM_INFO* p)
{
	m_allProgramInfo = p;
}

PARSED_FRAME_INFO CDemuxTs::AddTsPacket(CTsPacket* tsPacket)
{
	PARSED_FRAME_INFO rst;
	if (m_bSingleMode)
	{
		rst = m_mapProgParser[SINGLE_MODE_PROG_NUM]->PushBackTsPacket(tsPacket,m_llPacketID);
	}
	else
	{
		int pid = tsPacket->Get_PID();
		map<int,PROGRAM_PIDS>::iterator it = m_mapProgPids.begin();
		for (; it != m_mapProgPids.end(); it++)
		{
			PROGRAM_PIDS& pids = it->second;
			bool bFinded = false;
			for (int i = 0; i < (int)pids.pids.size(); i++)
			{
				if(pid == pids.pids[i].pid)
				{
					rst = m_mapProgParser[it->first]->PushBackTsPacket(tsPacket,m_llPacketID);
					bFinded = true;
					break;
				}
			}
			if (!bFinded)	//2012/7/1  其他节目数据
			{
				m_mapProgParser[it->first]->AddOtherPacket();
			}

		}
	}

	m_llPacketID++;

	return rst;
}


int CDemuxTs::DecodePacket(BYTE *pPacket, int nLen)
{
	CTsPacket tsPacket;
	if (!tsPacket.SetPacket(pPacket))
	{
		return 0;
	}

	if (m_bSingleMode)
	{
		m_mapProgParser[SINGLE_MODE_PROG_NUM]->DecodePacket(&tsPacket);
	}
	else
	{
		int pid = tsPacket.Get_PID();
		map<int,PROGRAM_PIDS>::iterator it = m_mapProgPids.begin();
		for (; it != m_mapProgPids.end(); it++)
		{
			PROGRAM_PIDS& pids = it->second;
			for (int i = 0; i < (int)pids.pids.size(); i++)
			{
				if(pid == pids.pids[i].pid)
				{
					m_mapProgParser[it->first]->DecodePacket(&tsPacket);
					break;
				}
			}
		}
	}
	return 0;
}

