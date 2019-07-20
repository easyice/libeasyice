/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "MpegDec.h"
#include "DetectStreamType.h"
#include "EiLog.h"
#include "DemuxTs.h"



bool g_bExitWork;



CMpegDec::CMpegDec(void)
{
	m_pDemuxTs = new CDemuxTs();
	m_pDemuxTs->SetOutputBuffer(&m_allProgramInfo);
    m_pProgressApp = NULL;
    m_pfProgressCb = NULL;
}

CMpegDec::~CMpegDec(void)
{
	ALL_PROGRAM_INFO::iterator itInfo = m_allProgramInfo.begin();
	for (; itInfo != m_allProgramInfo.end(); itInfo++)
	{
		delete itInfo->second;
	}
	delete m_pDemuxTs;
}

void CMpegDec::SetProgressFunCb(easyice_progress_callback pFun,void *pApp)
{
    m_pfProgressCb = pFun;
    m_pProgressApp = pApp;
}

void CMpegDec::Init(int nTsLength,long long nPacket_Total)
{
	m_nTsLength = nTsLength;

	m_Packet_Total = nPacket_Total;

	m_Packet_Counter = 0;

	m_TableAnalyzer.Init();
	m_TableAnalyzer.SetTsPacketLength(m_nTsLength);

	InitPacketType();
	m_bHavePAT = true;


	m_nVideoPID = -1;
	m_nAudioPID = -1;
	m_nPcrPID = -1;
	
	m_bDemuxAnaEnable = true;
	m_bTableAnaEnable = true;
	m_bPidAnaEnable = true;

	for(int i = 0; i < 1000; i++)
		m_pPidInfo[i].pid = -1;


	
}


void CMpegDec::LiveInit(int nTsLength)
{

	m_nTsLength = nTsLength;

	m_Packet_Total = -1;
	//m_llPacketListPktTotal = -1;
	m_Packet_Counter = 0;

	m_TableAnalyzer.Init();
	m_TableAnalyzer.SetTsPacketLength(m_nTsLength);

	InitPacketType();
	m_bHavePAT = true;
	m_mapPidList.clear();

	m_nVideoPID = -1;
	m_nAudioPID = -1;
	m_nPcrPID = -1;
	
	m_bDemuxAnaEnable = true;
	m_bTableAnaEnable = true;
	m_bPidAnaEnable = true;

	for(int i = 0; i < 1000; i++)
		m_pPidInfo[i].pid = -1;

	//m_nFrameNum = -1;
	//m_pktLst = NULL;
}

int CMpegDec::ProbeMediaInfo(BYTE * pData,int length)
{
    if (m_TableAnalyzer.ParseFindPAT(pData,(int)length) == -1)	//没有找到PAT，按单路模式解析
    {
        if (!ParsePidNoPsi(pData,length,m_nVideoPID,m_nAudioPID,m_nPcrPID) )//能检测到视频和pcr就可以
        {
            ei_log(LV_WARNING,"libeasyice","not found psi,program analysis is disabled");
            m_bDemuxAnaEnable = false;
            m_bTableAnaEnable = false;
        }
        m_bHavePAT = false;

        if (m_bDemuxAnaEnable)
        {
            //无表方式检测视频流类型
            CDetectStreamType dt;
            dt.Init(pData,(int)length,m_nVideoPID);
            m_nVideoStream_type = dt.GetVideoType();
            m_pDemuxTs->SetupDemux(m_nVideoPID,m_nVideoStream_type,m_nTsLength);
        }
    }
    else
    {
        //m_pDemuxTs->SetupDemux(pData,(int)length,m_nTsLength);

        //parse psi
        CAnalyzeTable TableAnalyzer;
        TableAnalyzer.Init();
        TableAnalyzer.SetTsPacketLength(m_nTsLength);
        TableAnalyzer.ParseFindPAT(pData,length);

        CTsPacket tsPacket;
        for (int i = 0; i < length; i+= m_nTsLength)
        {
            tsPacket.SetPacket(&pData[i]);
            int pid = tsPacket.Get_PID();
            if (pid == 0)
            {
                TableAnalyzer.PushBackTsPacket(&tsPacket);
                continue;
            }
            vector<int>::iterator it_find = find( TableAnalyzer.m_vecPmtPidList.begin(),TableAnalyzer.m_vecPmtPidList.end(),pid ); 
            if (it_find != TableAnalyzer.m_vecPmtPidList.end() )
            {
                TableAnalyzer.PushBackTsPacket(&tsPacket);
            }
        }
        FillPacketType(TableAnalyzer.GetTables());
        m_pDemuxTs->SetupDemux(TableAnalyzer.GetTables(),m_nTsLength);
    }

    return 0;
}

int CMpegDec::ProcessBuffer(BYTE * pData,int length)
{

	DWORD inPos = 0;

	while (inPos < length)
	{
		if (g_bExitWork)		//是否强制结束处理
		{
			break;
		}

		ProcessPacket(&pData[inPos]);
		
		//--------------------------
		//计算进度
		inPos += m_nTsLength;
		m_Packet_Counter++;
		int pct = (int)(m_Packet_Counter*PROGRESS_RANGE / m_Packet_Total );
		if (m_nPct != pct)
		{
			m_nPct = pct;
            if (m_pfProgressCb != NULL) m_pfProgressCb(pct,m_pProgressApp);
			//SendMessage(g_msgWnd,MESSAGE_GUI_PROGRESS, (WPARAM)&m_nPct,0);
		}
		
	}
	
	return 0;
}

int CMpegDec::ProcessPacket(BYTE * pData)
{
	CTsPacket tsPacket;
	PARSED_FRAME_INFO frame_rst;

	if ( !tsPacket.SetPacket(pData) )
	{
		return -1;
	}

	//-----------处理部分-------
	//解析PSI/SI
	if (m_bTableAnaEnable)
		m_TableAnalyzer.PushBackTsPacket(&tsPacket);

	//统计PID信息
	if (m_bPidAnaEnable)
		CalcPidList(tsPacket.Get_PID());

	//解复用，并分析每路节目的信息
	if (m_bDemuxAnaEnable)
		frame_rst = m_pDemuxTs->AddTsPacket(&tsPacket);



	return 0;
}

bool CMpegDec::ParsePidNoPsi(BYTE * pData, __int64 length,int& nVideoPID,int& nAudioPID,int& nPcrPID)
{
	nVideoPID = -1;
	nAudioPID = -1;
	nPcrPID = -1;

	DWORD inPos = 0;
	CTsPacket tsPacket;
	while (inPos < length)
	{
		if ( !tsPacket.SetPacket(&pData[inPos]))
		{
			//TS packet error
			inPos += m_nTsLength;
			continue;
		}
		WORD pid = tsPacket.Get_PID();
		if (pid == 0x1FFF)
		{
			inPos += m_nTsLength;
			continue;
		}
		if (tsPacket.Get_PCR_flag())
		{
			nPcrPID = pid;
		}
		BYTE streamID;
		bool bRet=tsPacket.Get_PES_stream_id(streamID);
		if(bRet&&streamID>=0xe0&&streamID<=0xef)
		{
			nVideoPID = pid;
		}
		else if(bRet&&streamID>=0xc0&&streamID<=0xdf)
		{
			nAudioPID = pid;
		}
		
		inPos += m_nTsLength;

	}

	if (nVideoPID >0 && nPcrPID > 0)
	{
		return true;
	}
	return false;
}


void CMpegDec::FillPacketType(TABLES* tables)
{

	if (tables->vecTabPAT.empty())
	{
		return;
	}

	//add pmts
	vector<PAT>::iterator it = tables->vecTabPAT[0].begin();
	for (; it != tables->vecTabPAT[0].end(); it++)
	{
		vector<PAT_LIST>::iterator it_list = it->vec_pat_list.begin();
		for (; it_list != it->vec_pat_list.end(); it_list++)
		{
			if (it_list->program_number == 0)
			{
				m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(it_list->network_pmt_PID,PACKET_NIT_ST));
			}
			else
			{
				m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(it_list->network_pmt_PID,PACKET_PMT));
			}
		}//!for it_list
	}

	//other streams (audio video pcr)
	map<int,STU_SECTION_PMT>::iterator it_pmts = tables->mapTabPMT.begin();
	for (; it_pmts != tables->mapTabPMT.end(); it_pmts++)
	{
		vector<PID_TYPE> prog_pids;

		int section_count = (int)it_pmts->second.size();
		for (int i = 0; i < section_count; i++)
		{

			PMT& pmt = (it_pmts->second)[i];
			vector<PMT_LIST2>::iterator it_list = pmt.vec_pmt_list2.begin();
			for (; it_list != pmt.vec_pmt_list2.end(); it_list++)
			{
				PACKET_TYPE packet_type;
				const STREAMTYPE* p_streamtype = GetStreamInfoByStreamType(it_list->stream_type);
				if (p_streamtype == NULL)
				{
					packet_type = PACKET_UNKNOWN;
				}
				else
				{
					packet_type = p_streamtype->packet_type;
				}

				
				m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(it_list->elementary_PID,packet_type));

				PID_TYPE pid_type;
				pid_type.pid = it_list->elementary_PID;
				pid_type.type = packet_type;

				prog_pids.push_back(pid_type);
			}//!for it_list

			//add pcr.如果pcr在视频中，此包认为是取视频类型，这样后添加pcrpid，会插入失败，不覆盖原视频类型。
			m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(pmt.pcr_pid,PACKET_PCR));

			PID_TYPE pid_type;
			pid_type.pid = pmt.pcr_pid;
			pid_type.type = PACKET_PCR;

			prog_pids.push_back(pid_type);

		}//for i

		m_allProgramBrief.insert(ALL_PROGRAM_BRIEF::value_type(it_pmts->first,prog_pids));
	}
}



void CMpegDec::CalcPidList(int pid)
{

	//PART 4
	PIDINFO* p = m_pPidInfo;
	while (p->pid != -1)
	{
		if (p->pid == pid)
		{
			p->count++;
			return;
		}
		p++;
		if (p - m_pPidInfo > 1000)
		{
			return;
		}
	}
	p->pid = pid;
	p->count = 1;
}


bool CMpegDec::GetPacketHeadInfo(BYTE* pData,int nLength,MSG_FLAG_LIST* headInfo,char* strDescriptor,int descriptorLen)
{
	CTsPacket tsPacket;
	if ( !tsPacket.SetPacket(pData))
		return false;
	headInfo->ts_head.sync_byte = 0x47;
	headInfo->ts_head.transport_error_indicator = tsPacket.Get_transport_error_indicator();
	headInfo->ts_head.payload_unit_start_indicator = tsPacket.Get_payload_unit_start_indicator();
	headInfo->ts_head.transport_priority = tsPacket.Get_transport_priority();
	headInfo->ts_head.PID = tsPacket.Get_PID();
	headInfo->ts_head.transport_scrambling_control = tsPacket.Get_transport_scrambling_control();
	headInfo->ts_head.adaptation_field_control = tsPacket.Get_adaptation_field_control();
	headInfo->ts_head.continuity_counter = tsPacket.Get_continuity_counter();

	if (headInfo->ts_head.adaptation_field_control == 2 || headInfo->ts_head.adaptation_field_control == 3)
	{
		BYTE *pAF_Data = &pData[4];
		headInfo->adaptation_field.b_exist = 0x1;
		headInfo->adaptation_field.adaptation_field_length = pData[4];
		if (headInfo->adaptation_field.adaptation_field_length > 0)
		{
			headInfo->adaptation_field.discontinuity_indicator = tsPacket.Get_discontinuity_indicator(pAF_Data);
			headInfo->adaptation_field.random_access_indicator = tsPacket.Get_random_access_indicator(pAF_Data);
			headInfo->adaptation_field.elementary_stream_priority_indicator = tsPacket.Get_elementary_stream_priority_indicator(pAF_Data);
			headInfo->adaptation_field.PCR_flag = tsPacket.Get_PCR_flag(pAF_Data);
			headInfo->adaptation_field.OPCR_flag = tsPacket.Get_OPCR_flag(pAF_Data);
			headInfo->adaptation_field.splicing_point_flag = tsPacket.Get_splicing_point_flag(pAF_Data);
			headInfo->adaptation_field.transport_private_data_flag = tsPacket.Get_transport_private_data_flag(pAF_Data);
			headInfo->adaptation_field.adaptation_field_extension_flag = tsPacket.Get_adaptation_field_extension_flag(pAF_Data);
			if (headInfo->adaptation_field.PCR_flag)
			{
				snprintf(strDescriptor,descriptorLen-100,("PCR:%I64u\r\n"),tsPacket.Get_PCR());
			}
		}
	}
	
	BYTE stream_id = 0;
	if (tsPacket.Get_PES_stream_id(stream_id))
	{
		BYTE size = 0;
		BYTE *pPES_Data = tsPacket.Get_Data(size);
		if (size < 4)
		{
			return true;
		}
		headInfo->pes_head.b_exist = 0x1;
		headInfo->pes_head.stream_id = stream_id;
		headInfo->pes_head.PES_packet_length = ((*(pPES_Data + 4))<<8)|(*(pPES_Data + 5));
		headInfo->pes_head.PES_scrambling_control = ((*(pPES_Data + 6))>>4) & 0x03;
		headInfo->pes_head.PES_priority = ((*(pPES_Data + 6))>>3) & 0x01;
		headInfo->pes_head.data_alignment_indicator = ((*(pPES_Data + 6))>>2) & 0x01;
		headInfo->pes_head.copyright = ((*(pPES_Data + 6))>>1) & 0x01;
		headInfo->pes_head.orginal_or_copy = (*(pPES_Data + 6)) & 0x01;
		headInfo->pes_head.PTS_DTS_flags = (*(pPES_Data + 7)) >> 6;
		headInfo->pes_head.ESCR_flag = ((*(pPES_Data + 7))>>5) & 0x01;
		headInfo->pes_head.ES_rate_flag = ((*(pPES_Data + 7))>>4) & 0x01;
		headInfo->pes_head.DSM_trick_mode_flag = ((*(pPES_Data + 7))>>3) & 0x01;
		headInfo->pes_head.additional_copy_info_flag = ((*(pPES_Data + 7))>>2) & 0x01;
		headInfo->pes_head.PES_CRC_flag = ((*(pPES_Data + 7))>>1) & 0x01;
		headInfo->pes_head.PES_extension_flag = (*(pPES_Data + 7)) & 0x01;
		headInfo->pes_head.PES_header_data_length = (*(pPES_Data + 8));
		if (headInfo->pes_head.PTS_DTS_flags == 0x2)
		{
			char buf[50] = ("");
			LONGLONG pts;
			tsPacket.Get_PTS(pts);
			snprintf(buf,sizeof(buf)/sizeof(char),("PTS:%I64u\r\n"),pts);
			strncat(strDescriptor,buf,descriptorLen);
		}
		else if (headInfo->pes_head.PTS_DTS_flags == 0x3)
		{
			char buf[50] = ("");
			char buf1[50] = ("");
			LONGLONG pts,dts;
			tsPacket.Get_PTS(pts);
			tsPacket.Get_DTS(dts);
			snprintf(buf,sizeof(buf)/sizeof(char),("PTS:%I64u\r\n"),pts);
			snprintf(buf1,sizeof(buf1)/sizeof(char),("DTS:%I64u\r\n"),dts);
			strncat(strDescriptor,buf,descriptorLen);
			strncat(strDescriptor,buf1,descriptorLen);
		}
	}
	return true;
}



void CMpegDec::InitPacketType()
{
	m_mapPacketType.clear();
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x0,PACKET_PAT));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1,PACKET_CAT));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x2,PACKET_TSDT));

	for (int i = 0x3; i <= 0xF; i++)
	{
		m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(i,PACKET_RESERVED));	//预留
	}

	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x10,PACKET_NIT_ST));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x11,PACKET_SDT_BAT_ST));

	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x12,PACKET_EIT_ST));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x13,PACKET_RST_ST));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x14,PACKET_TDT_TOT_ST));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x15,PACKET_NETSYNC));

	for (int i = 0x16; i <= 0x1B; i++)
	{
		m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(i,PACKET_RESERVED));	//预留
	}

	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1C,PACKET_SIGN));	//带内信令
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1D,PACKET_SURVEY));	//测量
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1E,PACKET_DIT));
	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1F,PACKET_SIT));

	m_mapPacketType.insert(map<int,PACKET_TYPE>::value_type(0x1FFF,PACKET_NULL));

}


void CMpegDec::UpdatePidListResult()
{
	//计算总包数
	__int64 nAllPacketCount = 0;
	PIDINFO* p = m_pPidInfo;
	while (p->pid != -1)
	{
		nAllPacketCount += p->count;
		
		if (m_mapPidList.find(p->pid) == m_mapPidList.end())
		{
			MSG_PID_LIST msg_pid_list;
			msg_pid_list.PID = p->pid;
			msg_pid_list.total = p->count;
			msg_pid_list.type = m_mapPacketType[p->pid];
			m_mapPidList.insert(map<int,MSG_PID_LIST>::value_type(p->pid,msg_pid_list));
		}
		else
		{
			m_mapPidList[p->pid].total = p->count;
			m_mapPidList[p->pid].type = m_mapPacketType[p->pid];
		}

		p++;
	}

	//SendMessage(g_msgWnd,MESSAGE_DATA_PID_LIST_BEFOR,0,0);
	map<int,MSG_PID_LIST>::iterator it = m_mapPidList.begin();
	for (; it != m_mapPidList.end(); it++)
	{
		//计算百分比
		it->second.percent = (float) (it->second.total * 1.0 / nAllPacketCount);
		//SendMessage(g_msgWnd,MESSAGE_DATA_PID_LIST,0,(LPARAM)&(it->second));
	}
	//SendMessage(g_msgWnd,MESSAGE_DATA_PID_LIST_END,0,0);
}



std::vector<PID_TYPE>* CMpegDec::GetProgPidNoPsi()
{
	m_vecPids.clear();
	PID_TYPE pt;

	
	if (m_nPcrPID < 0)
		return &m_vecPids;

	if (m_nVideoPID < 0 && m_nAudioPID < 0)
		return &m_vecPids;
	
	pt.pid = m_nPcrPID;
	pt.type = PACKET_PCR;
	m_vecPids.push_back(pt);

	
	if (m_nVideoPID > 0)
	{
		pt.pid = m_nVideoPID;

		if (m_nVideoStream_type ==0x2)
			pt.type = PACKET_MPEG2_VIDEO;
		else if (m_nVideoStream_type == 0x1B)
			pt.type = PACKET_VIDEO_H264;

		m_vecPids.push_back(pt);
	}

	if (m_nAudioPID > 0)
	{
		pt.pid = m_nAudioPID;
		pt.type = PACKET_MPEG1_AUDIO;
		m_vecPids.push_back(pt);
	}
	
	return &m_vecPids;
}

void CMpegDec::FillPacketType()
{
	FillPacketType(m_TableAnalyzer.GetTables());
}

void CMpegDec::LiveProcessPacket(BYTE* pPacket)
{
	CTsPacket tsPacket;
	if ( !tsPacket.SetPacket(pPacket) )
	{
		return;
	}

	m_TableAnalyzer.PushBackTsPacket2(pPacket);
	CalcPidList(tsPacket.Get_PID());
}

void CMpegDec::Finish()
{
    UpdatePidListResult();
}

