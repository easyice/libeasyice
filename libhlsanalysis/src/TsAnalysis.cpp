/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "TsAnalysis.h"
#include "MsgMgr.h"
#include "json/json.h"
#include "commondefs.h"

#include "../../libeasyice/src/EasyICEDLL/tables/CAnalyzeTable.h"
#include "tspacket.h"
#include "../../libeasyice/src/EasyICEDLL/CheckMediaInfo.h"
#include "../../libeasyice/src/EasyICEDLL/MpegDec.h"

#include "../../libeasyice/src/H264DecDll/H264Dec.h"

#include <time.h>



//PID列表刷新间隔时间(ns)
#define PID_LIST_UPDATE_INTERVAL	1000000






CTsAnalysis::CTsAnalysis(CMsgMgr* pMsgMgr)
{
    m_pMsgMgr = pMsgMgr;
	m_bInited = false;
	m_bReportedNullPkt = false;

	m_pTrcore = new Clibtr101290();
	m_pTrcore->SetStartOffset(0);
	m_pTrcore->SetTsLen(188);
	m_pTrcore->SetReportCB(OnTrReport,this);
	m_pAvcParser = new CH264Dec();
	m_pMpegdec = new CMpegDec();


}

CTsAnalysis::~CTsAnalysis(void)
{
	delete m_pTrcore;
	delete m_pAvcParser;
	delete m_pMpegdec;
}

void CTsAnalysis::OnRecvSegment(BYTE* pData,int nLen,const string& url)
{
	m_bReportedCCError = false;
	m_bReportedTCError = false;

	m_strUrl = url;
	if (nLen <= 0)
	{
		ei_log(LV_ERROR,"hls","segment len error,len=%d,url=%s",nLen,url.c_str());
		return;
	}

	if (pData[0] !=0x47)
	{
		ei_log(LV_ERROR,"hls","segment data error,len=%d,url=%s",nLen,url.c_str());
		return;
	}
	if (!m_bInited)
	{
		int syncpos;
		int packet_len = TryToSync(pData,nLen,syncpos);
		if (packet_len != 188 || syncpos != 0)
		{
			ei_log(LV_ERROR,"hls","ts packet sync byte error,packet_len=%d,syncpos=%d,",packet_len,syncpos);
			return;
		}
		if (!DumpMediaInfo(pData,nLen))
		{
			ei_log(LV_ERROR,"hls","DumpMediaInfo error on segment,url=%s",url.c_str());
			return;
		}
	
		m_pMpegdec->LiveInit(188);
		for (int i = 0; i < nLen; i+=188)
		{
			m_pMpegdec->LiveProcessPacket(pData+i);
		}
		m_pMpegdec->FillPacketType();
		

		m_bInited = true;
	}

	//开始各项分析
	
	CTsPacket tspacket;
	for (int i = 0; i < nLen; i+=188)
	{
		tspacket.SetPacket(pData+i);
		int pid = tspacket.Get_PID();

		//is have null packet?
		if (pid == 0x1FFF && !m_bReportedNullPkt)
		{
			m_pMsgMgr->HlsPostMessage(EM_HRT_DIAGNOSIS_HASNULLPKT,("warning"),("ts"),(""));
			m_bReportedNullPkt = true;
		}

		m_pTrcore->AddPacket(pData+i);
		//m_pMpegdec->LiveProcessPacket2(pData+i);
		m_pMpegdec->LiveProcessPacket(pData+i);
	}

	if (!IsStartWithPatPmt(pData,nLen,url))
	{
		m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_STARTPAT,url);
	}

	CheckAudioBoundary(pData,nLen,url);
	CheckVideoBoundary(pData,nLen,url);
	CheckIFrame(pData,nLen,url);


	//更新PID列表
    //TODO: pidlist`
	m_pMpegdec->UpdatePidListResult();
    Json::Value rootPid;
    std::map<int,MSG_PID_LIST>::iterator it_pids = m_pMpegdec->m_mapPidList.begin();
    for (;it_pids != m_pMpegdec->m_mapPidList.end();++it_pids)
    {
        int pid = it_pids->second.PID;
        int total = it_pids->second.total;
        float pct = it_pids->second.percent * 100;
        string type = Get_PacketTypeDes(it_pids->second.type);
        char key[32];
        sprintf(key,"%d", it_pids->first);
        rootPid[key] = it_pids->second.to_json();
    }
    string outPid = rootPid.toStyledString();
	m_pMsgMgr->HlsPostMessage(EM_HRT_SEGMENT_PIDLIST,outPid);

}

bool CTsAnalysis::IsStartWithPatPmt(BYTE* pData,int nLen,const string& url)
{
	bool recved_pat = false;
	bool recved_pmt = false;
	CTsPacket tspacket;
	for (int i = 0; i < nLen; i+=188)
	{
		tspacket.SetPacket(pData+i);
		int pid = tspacket.Get_PID();

		if (pid == 0)
		{
			recved_pat = true;
		}

		if (pid == m_stSegMi.pmt_pid)
		{
			recved_pmt = true;
			if (!recved_pat)
			{
				ei_log(LV_WARNING,"hls","pmt is befor pat,url=%s",url.c_str());
			}
		}

		//is start with pat pmt?
		if (pid == m_stSegMi.video_pid || pid == m_stSegMi.audio_pid)
		{
			if (!recved_pat || !recved_pmt)
			{
				return false;
			}
		}
	}

	return true;
}

void CTsAnalysis::CheckAudioBoundary(BYTE* pData,int nLen,const string& url)
{
	CTsPacket tspacket;

	for (int i = 0; i < nLen; i+=188)
	{
		tspacket.SetPacket(pData+i);
		int pid = tspacket.Get_PID();

		if (pid != m_stSegMi.audio_pid )
		{
			continue;
		}

		if ((tspacket.Get_adaptation_field_control() & 1) == 0)
		{
			continue;
		}


		BYTE stream_id;
		if (!tspacket.Get_PES_stream_id(stream_id))
		{
			m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_AUDIOBOUNDARY,url);
		}
		return;
	}
}

void CTsAnalysis::CheckVideoBoundary(BYTE* pData,int nLen,const string& url)
{
	m_pAvcParser->Reset();

	CTsPacket tspacket;
	bool first_video_pkt = true;
	for (int i = 0; i < nLen; i+=188)
	{
		tspacket.SetPacket(pData+i);
		int pid = tspacket.Get_PID();

		if (pid != m_stSegMi.video_pid )
		{
			continue;
		}

		if ((tspacket.Get_adaptation_field_control() & 1) == 0)
		{
			continue;
		}

		//pes start
		if (first_video_pkt)
		{
			first_video_pkt = false;
			BYTE stream_id;
			if (!tspacket.Get_PES_stream_id(stream_id))
			{
				m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_VIDEOBOUNDARY,url);
				m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_STARTIFRAME,url);
				return;
			}
		}
		//判断帧类型（仅支持AVC）
		if (!m_stSegMi.is_avc)
		{
			return;
		}

		//先解析
		PARSED_FRAME_INFO parsed_frame_info = m_pAvcParser->ParseTsContinue(pData+i,188);
		
		//I帧开始必须有SPS，在遇到任何slice之前，没有遇到sps，认为不是I帧开始。
		if (tspacket.H264_Get_sps())
		{
			return;
		}

		
		
		if (parsed_frame_info.bNewPicture/* && parsed_frame_info.structure != STRUCTURE_BOTTOM_FIELD*/) 
		{
			if (parsed_frame_info.FrameType != FRAME_I && parsed_frame_info.FrameType != FRAME_IDR)
			{
				m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_STARTIFRAME,url);
			}
			return;
		}

	}
}

void CTsAnalysis::CheckIFrame(BYTE* pData,int nLen,const string& url)
{
	//判断帧类型（仅支持AVC）
	if (!m_stSegMi.is_avc)
	{
		return;
	}

	m_pAvcParser->Reset();
	CTsPacket tspacket;

	for (int i = 0; i < nLen; i+=188)
	{
		tspacket.SetPacket(pData+i);
		int pid = tspacket.Get_PID();

		if (pid != m_stSegMi.video_pid )
		{
			continue;
		}

		if ((tspacket.Get_adaptation_field_control() & 1) == 0)
		{
			continue;
		}

		//I帧开始必须有SPS，在遇到任何slice之前，没有遇到sps，认为不是I帧开始。
		if (tspacket.H264_Get_sps())
		{
			return;
		}

		PARSED_FRAME_INFO parsed_frame_info = m_pAvcParser->ParseTsContinue(pData+i,188);
		

		if (parsed_frame_info.bNewPicture/* && parsed_frame_info.structure != STRUCTURE_BOTTOM_FIELD*/) 
		{
			if (parsed_frame_info.FrameType == FRAME_I || parsed_frame_info.FrameType == FRAME_IDR)
			{
				return;
			}
		}
	}

	m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_ATLEASTONEIFRAME,url);
}

bool CTsAnalysis::DumpMediaInfo(BYTE* pData,int nLen)
{
	using namespace tables;


	//parse psi
	CAnalyzeTable TableAnalyzer;
	TableAnalyzer.Init();
	TableAnalyzer.SetTsPacketLength(188);
	if (TableAnalyzer.ParseFindPAT(pData,nLen) == -1)
	{
		return false;
	}

	
	CTsPacket tsPacket;
	for (int i = 0; i < nLen; i+= 188)
	{
		tsPacket.SetPacket(&pData[i]);
		TableAnalyzer.PushBackTsPacket(&tsPacket);

	}
	
	if (TableAnalyzer.GetTables()->vecTabPAT.empty())
	{
		return false;
	}
	if (TableAnalyzer.GetTables()->mapTabPMT.empty())
	{
		return false;
	}

	
	EI_MEDIAINFO_T emi;
    CEiMediaInfo EiMediaInfo;
	EiMediaInfo.CheckMediaInfo(pData,nLen,188,&emi);
	m_pMsgMgr->HlsPostMessage(EM_HRT_SEGMENT_MEDIAINFO,emi.mi);

	if (TableAnalyzer.GetTables()->mapTabPMT.size() > 1)
	{
		m_pMsgMgr->HlsPostMessage(EM_HRT_DIAGNOSIS_SPTS,("warning"),("ts"),(""));
	}

	// get pmt pid
	if (!TableAnalyzer.GetTables()->vecTabPAT[0].empty())
	{
		vector<PAT>::iterator it_pat = TableAnalyzer.GetTables()->vecTabPAT[0].begin();

		vector<PAT_LIST>::iterator it_list = it_pat->vec_pat_list.begin();
		for (; it_list != it_pat->vec_pat_list.end(); it_list++)
		{
			if (it_list->program_number != 0)//program_number = 0 is NIT 
			{
				m_stSegMi.pmt_pid = it_list->network_pmt_PID;
				break;//如果是多节目流，取第一个节目
			}
		}//!for it_list
	}
	

	//check codes type
	map<int,STU_SECTION_PMT>::iterator it_pmts = TableAnalyzer.GetTables()->mapTabPMT.begin();
	int section_count = (int)it_pmts->second.size();
	for (int i = 0; i < section_count; i++)
	{

		PMT& pmt = (it_pmts->second)[i];
		vector<PMT_LIST2>::iterator it_list = pmt.vec_pmt_list2.begin();
		for (; it_list != pmt.vec_pmt_list2.end(); it_list++)
		{
			vector<EI_VIDEOINFO_T>::iterator it_video = emi.vecVStream.begin();
			for (;it_video != emi.vecVStream.end(); ++it_video)
			{
				if (it_list->elementary_PID == it_video->id)
				{
					m_stSegMi.video_pid = it_video->id;
					if (it_video->format != ("AVC"))
					{
						string str = ("detected video type:");
						str += it_video->format;
						m_pMsgMgr->HlsPostMessage(EM_HRT_DIAGNOSIS_VIDEOTYPE,("warning"),("ts"),str.c_str());
						ei_log(LV_WARNING,"hls","非H264视频，某些功能将禁用");
						m_stSegMi.is_avc = false;
					}
				}
			}

			std::vector<EI_AUDIOINFO_T>::iterator it_audio = emi.vecAStream.begin();
			for (;it_audio != emi.vecAStream.end(); ++it_audio)
			{
				if (it_list->elementary_PID == it_audio->id)
				{
					m_stSegMi.audio_pid = it_audio->id;
					if (it_audio->format != ("AAC") && it_audio->format != ("MP3") && it_audio->format != ("AC-3"))
					{
						string str = ("detected audio type:");
						str += it_audio->format;
						m_pMsgMgr->HlsPostMessage(EM_HRT_DIAGNOSIS_AUDIOTYPE,("warning"),("ts"),str.c_str());
					}
				}
			}


		}//!for it_list
	}// !for section_count


	return true;
}

int CTsAnalysis::TryToSync(BYTE* pData,int nLength,int& nSycByte)
{
	nSycByte = 0;
	int maxLength = nLength - 204*3;
	if (maxLength < 204*3)
		return -1;

	for (int i = 0; i < maxLength; i++)
	{
		if(pData[i]==0x47&&pData[i+188]==0x47&&pData[i+188*2]==0x47&&pData[i+188*3]==0x47&&pData[i+188*4]==0x47)
		{	
			nSycByte = i;
			return 188;
		}
		else if(pData[i]==0x47&&pData[i+204]==0x47&&pData[i+204*2]==0x47&&pData[i+204*3]==0x47&&pData[i+204*4]==0x47)
		{
			nSycByte = i;
			return 204;
		}
	}
	return -1;
}

void CTsAnalysis::OnTrReport(REPORT_PARAM_T param)
{
	CTsAnalysis* lpthis = (CTsAnalysis*)param.pApp;
	switch (param.errName)
	{
	case LV1_CC_ERROR:
		if (!lpthis->m_bReportedCCError)
		{
			lpthis->m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_CC,lpthis->m_strUrl);
		}
		lpthis->m_bReportedCCError = true;
		break;
	case LV2_PCR_REPETITION_ERROR:
		if (param.llVal/27000 > 100 && (param.fVal != 1))
		{
			if (!lpthis->m_bReportedTCError)
			{
				lpthis->m_pMsgMgr->HlsPostMessage(EM_HRT_SQ_TC,lpthis->m_strUrl);
			}
			lpthis->m_bReportedTCError = true;
		}
	default:
		break;
	}
}
