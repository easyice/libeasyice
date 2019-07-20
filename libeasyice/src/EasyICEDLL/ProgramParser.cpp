/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "ProgramParser.h"



bool hm_hevc_check_rap(CTsPacket* tspacket)
{        
        BYTE nal_unit_type = 0;
        BYTE *p_data=tspacket->m_pPacket;
        BYTE size=188-4;

        int start_pos = 0;
        BYTE stream_id;
        BYTE adaptation_field_control = tspacket->Get_adaptation_field_control();
        if (adaptation_field_control == 2 || adaptation_field_control == 3)     //含调整字段
        {
                int adaptation_field_length = *(tspacket->m_pPacket + 4);
                start_pos = adaptation_field_length + 1; //包含长度字段
        }
        if (tspacket->Get_PES_stream_id(stream_id))
        {
                start_pos = start_pos + 3 + 1 + 2;//packet_start_code_prefix + stream_id + PES_packet_length
        }
        
        
        for(int i=start_pos;i<size;i++)
        {
                if((p_data[i] == 0) &&
                        (p_data[i+1] == 0) &&
                        (p_data[i+2] == 1) )
                {
                        nal_unit_type = ((p_data[i+2+1]) >> 1) & 0x3f;
                         //nal_unit_type = ((p_data[i+3+1])&0x7E) >> 1;
                        return nal_unit_type == 32 ? true : false;
                }
        }
        return false;
}


CProgramParser::CProgramParser(int nTsLength)
{
	m_llTotalPacketCounter = 0;
	m_pcr = -1;
	m_Vpts = -1;
	m_Apts = -1;
	m_dts = -1;
	m_pcrBefor = -1;
	m_nAudioPid = -1;

	m_pProgInfo = NULL;
	m_nPacketCountOfPcr = 0;
	m_fTransportRate = 0;
	m_nGopsizeTmp = 0;
	m_nGopBytesTmp = 0;

	m_nTsLength = nTsLength;
}

CProgramParser::~CProgramParser(void)
{
}

PARSED_FRAME_INFO CProgramParser::PushBackTsPacket(CTsPacket* tsPacket,long long packetID)
{
	PARSED_FRAME_INFO parsed_frame_info;
	m_nGopBytesTmp+=m_nTsLength;

	//m_llTotalPacketCounter++;
	//将绝对包数作为计数，不再使用每个节目各自的ts包计数
	m_llTotalPacketCounter = packetID;
	
	TIMESTAMP tp;
	tp.pos = m_llTotalPacketCounter;

	PROGRAM_TIMESTAMPS& pTts = m_pProgInfo->tts;
	bool bIFrame = false;
	BYTE stream_id;
	
	//check for gop
	int pid = tsPacket->Get_PID();
	if (pid == m_video_pid_type.pid)
	{
		if (m_video_pid_type.stream_type == 0x02)	//MPEGV
		{
			BYTE bPic = 0;
			if ( tsPacket->Get_PES_PIC_INFO(bPic) )
			{
				parsed_frame_info.bNewPicture = true;;
				parsed_frame_info.FrameType = (FRAME_TYPE)bPic;
				if (bPic == 1)
				{
					bIFrame = true;
				}
				MPEG2_AssembleGop(bPic);
			}

			if (tsPacket->Get_PES_stream_id(stream_id))
			{
				if (stream_id >= 0xE0 && stream_id <= 0xEF)
				{
					if (tsPacket->Mpeg_Get_Sequenec_head_code())
					{
						bIFrame = true;
					}
				}
			}
		}
		else if (m_video_pid_type.stream_type == 0x1B)	//H.264
		{
			parsed_frame_info = m_avcParser.ParseTsContinue(tsPacket->m_pPacket,188);
	
			//parse sps
			if (parsed_frame_info.bNewPicture && parsed_frame_info.structure != STRUCTURE_BOTTOM_FIELD)
			{
				H264_AssembleGop(parsed_frame_info.FrameType);
			}
			//else if (m_pProgInfo->gopList.empty()) //2017/6/28 增加else if，以支持第一个I帧前的半个GOP
			//{
			//	H264_AssembleGop(parsed_frame_info.FrameType);
			//}
			
			if (parsed_frame_info.bNewPicture && parsed_frame_info.structure != STRUCTURE_BOTTOM_FIELD)
			{
				if (parsed_frame_info.FrameType == FRAME_I || parsed_frame_info.FrameType == FRAME_IDR)
				{
					bIFrame = true;
				}
			}
			//带有sps的ts包认为是I帧起始。用于适应slice头在PES的下一个TS包中的情况
			if (tsPacket->Get_PES_stream_id(stream_id))
			{
				if (stream_id >= 0xE0 && stream_id <= 0xEF)
				{
					if (tsPacket->H264_Get_sps())
					{
						bIFrame = true;
					}
				}
			}
		}
		else if (m_video_pid_type.stream_type == 0x24)	//HEVC
		{
			if (tsPacket->Get_PES_stream_id(stream_id))
			{
				if (stream_id >= 0xE0 && stream_id <= 0xEF)
				{
					if (hm_hevc_check_rap(tsPacket))
					{
						bIFrame = true;
					}
				}
			}
		}
	}	//!Get_PID()

	/*Get times====================================================================*/

	if (tsPacket->Get_PCR_flag())
	{	
		tp.timestamp = tsPacket->Get_PCR();
		m_pcr = tp.timestamp;
		pTts.vecPcr.push_back(tp);

		MakeRate(m_pcr);
	}
	else if (m_pcrBefor != -1)
	{
		m_nPacketCountOfPcr++;
	}

	//PES head
	if ( tsPacket->Get_PES_stream_id(stream_id) )
	{
		//BYTE bPic = 0;
		//tsPacket->Get_PES_PIC_INFO(bPic);
		// 13818-2 p22
		long long curpcr = m_pcr;
		if (m_fTransportRate != 0)
		{
			long long curpcr = m_pcr + (m_nPacketCountOfPcr*188+188-11) * 27000000 / m_fTransportRate;
		}

		if (stream_id >= 0xE0 && stream_id <= 0xEF && bIFrame)	//video
		{
			BYTE flag = tsPacket->Get_PTS_DTS_flag();
			if (flag == 0x2)	//pts only
			{
				//pts值
				tsPacket->Get_PTS(tp.timestamp);
				tp.timestamp *= 300;
				m_Vpts = tp.timestamp;
				pTts.vecVpts.push_back(tp);

				//差值
				if (curpcr > 0)
				{
					tp.timestamp = m_Vpts/300 - curpcr/300;
					pTts.vecPtsSub.push_back(tp);
				}

				//最后一个pcr值
				//pTts.vecPcr.push_back(m_lastPcrtp);
			}
			else if (flag == 0x3)	//pts and dts
			{
				//pts dts 值
				tsPacket->Get_PTS(tp.timestamp);
				tp.timestamp *= 300;
				pTts.vecVpts.push_back(tp);
				m_Vpts = tp.timestamp;

				tsPacket->Get_DTS(tp.timestamp);
				tp.timestamp *= 300;
				m_dts = tp.timestamp;
				pTts.vecDts.push_back(tp);

				//差值
				if (curpcr > 0)
				{
					tp.timestamp = m_Vpts/300 - curpcr/300;
					pTts.vecPtsSub.push_back(tp);
					
					tp.timestamp = m_dts/300 - curpcr/300;
					pTts.vecDtsSub.push_back(tp);
				}
				//最后一个pcr值
				//pTts.vecPcr.push_back(m_lastPcrtp);
			}
		}
		else if (pid == m_nAudioPid && stream_id >= 0xC0 && stream_id <= 0xDF)	//audio
		{
			tsPacket->Get_PTS(tp.timestamp);
			tp.timestamp *= 300;
			m_Apts = tp.timestamp;
			pTts.vecApts.push_back(tp);
			//pTts.mapApts[pid].push_back(tp);

			tp.timestamp = m_Apts/300 - curpcr/300;
			pTts.vecAPtsSub.push_back(tp);
		}
	}

	return parsed_frame_info;
}

inline void CProgramParser::MPEG2_AssembleGop(BYTE bPic)
{
	switch(bPic)
	{
	case 0x01:	//I
		if ( !m_gopTmp.empty() )	//组合完成,新的开始
		{
			GOP_LIST gl;
			gl.gop = m_gopTmp;
			//gl.time = m_pcr;
			gl.pos = m_llTotalPacketCounter;
			gl.gop_size = m_nGopsizeTmp;
			gl.gop_bytes = m_nGopBytesTmp;
			m_pProgInfo->gopList.push_back(gl);
		}
		m_gopTmp = ("I");
		m_nGopsizeTmp = 1;
		m_nGopBytesTmp = m_nTsLength;
		break;

	case 0x02:	//P
		m_gopTmp += ("P");
		m_nGopsizeTmp++;
		break;

	case 0x03:	//B
		m_gopTmp += ("B");
		m_nGopsizeTmp++;
		break;
	}
}

inline void CProgramParser::H264_AssembleGop(FRAME_TYPE frame_type)
{


	switch(frame_type)
	{
	case FRAME_I:	//I
		if ( !m_gopTmp.empty() )	//组合完成,新的开始
		{
			GOP_LIST gl;
			gl.gop = m_gopTmp;
			//gl.time = m_pcr;
			gl.pos = m_llTotalPacketCounter;
			gl.gop_size = m_nGopsizeTmp;
			gl.gop_bytes = m_nGopBytesTmp;
			m_pProgInfo->gopList.push_back(gl);
		}
		m_gopTmp = ("I");
		m_nGopsizeTmp = 1;
		m_nGopBytesTmp = m_nTsLength;
		break;
	case FRAME_IDR:	//IDR
		if ( !m_gopTmp.empty() )	//组合完成,新的开始
		{
			GOP_LIST gl;
			gl.gop = m_gopTmp;
			//gl.time = m_pcr;
			gl.pos = m_llTotalPacketCounter;
			gl.gop_size = m_nGopsizeTmp;
			gl.gop_bytes = m_nGopBytesTmp;
			m_pProgInfo->gopList.push_back(gl);
		}
		m_gopTmp = ("IDR-");//IDR-
		m_nGopsizeTmp = 1;
		m_nGopBytesTmp = m_nTsLength;
		break;

	case FRAME_P:
		m_gopTmp += ("P");
		m_nGopsizeTmp++;
		break;

	case FRAME_B:
		m_gopTmp += ("B");
		m_nGopsizeTmp++;
		break;
	case FRAME_SP:
		m_gopTmp += ("-SP-");//-SP-
		m_nGopsizeTmp++;
		break;
	case FRAME_SI:
		m_gopTmp += ("-SI-");//-SI-
		m_nGopsizeTmp++;
		break;
	default:
		break;
	}
}

void CProgramParser::MakeRate(long long pcr)
{
	if (m_pcrBefor == -1)
	{
		m_pcrBefor = pcr;
		return;
	}

	RATE_LIST rl;

	//1048576 = 1024 * 1024
	//m_rate_perpcr = m_packetcountofpcrstep/(m_cur_pcr - m_first_pcr)*27000000*188*8/1024/1024;
	if (pcr - m_pcrBefor == 0)
		return;


	double Duration = (pcr - m_pcrBefor)/27;
	rl.rate = ((long long)m_nPacketCountOfPcr*188 + 188-11 + 11)*8*1000000/Duration / 1000000;
	
	//rl.rate = ((int)(rl.rate*100)) / 100.0;
	//long long nn = ((long long)m_nPacketCountOfPcr*188 + 188-11 + 11)*27000000*8;

//	rl.rate = ( (double)nn )/(double)((pcr - m_pcrBefor)*1048576);
	//rl.rate = ( (double)nn )/(double)((pcr - m_pcrBefor)*1000000);
	m_fTransportRate = ( (double)(m_nPacketCountOfPcr*188 + 188-11 + 11)*27000000 )/(double)(pcr - m_pcrBefor);
	rl.pos = m_llTotalPacketCounter;
	m_pProgInfo->rateList.push_back(rl);

	m_pcrBefor = pcr;
	m_nPacketCountOfPcr = 0;
}

void CProgramParser::SetVideoStreamInfo(PID_STREAM_TYPE video_pid_type)
{
	m_video_pid_type = video_pid_type;
}

int CProgramParser::DecodePacket(CTsPacket* tsPacket)
{
	if (tsPacket->Get_PID() == m_video_pid_type.pid)
	{
		if (m_video_pid_type.stream_type == 0x02)	//MPEGV
		{

		}
		else if (m_video_pid_type.stream_type == 0x1B)	//H.264
		{
			//m_avcParser.ParseTsPacket(tsPacket->m_pPacket,188);
		}
	}
	return 0;
}

void CProgramParser::AddOtherPacket()
{
	//m_llTotalPacketCounter++;
	m_nPacketCountOfPcr++;
}

void CProgramParser::SetAudioPid(int pid)
{
	m_nAudioPid = pid;
}

