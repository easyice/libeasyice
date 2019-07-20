/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "TsPacket.h"
#include "zbitop.h"


#define GET_TS_PID(pBuf) (WORD)((((*(pBuf)) & 0x1f)<<8) | (*(pBuf+1)))
#define GETBITAT(object,condition)(object>>condition&1)

inline int te(BYTE src)
{
	bool bStart = false;
	int ZeroBitCount = 0;
	for (int i = 7; i >=0; i--)
	{
		if ( GETBITAT(src,i) == 1)
		{
			if (bStart)
			{
				//取之后的ZeroBitCount位
				BYTE result = 1;
				if (i - ZeroBitCount < 0) return -1;
				for (int j = i -1; j > i -1 - ZeroBitCount; j--)
				{
					result <<= 1;
					result += GETBITAT(src,j);
				}
				return result - 1;
			}
			else
				bStart = true;
		}
		else
		{
			if (bStart)
				ZeroBitCount++;
		}
	}
	return -1;
}

CTsPacket::CTsPacket(void)
:m_pPacket(NULL)
{
}


CTsPacket::~CTsPacket(void)
{
}

void CTsPacket::RemoveAfField()
{
	if((*(m_pPacket +3)) & 0x20)
	{
		if(*(m_pPacket +4) >0)
		{
			*(m_pPacket +5) =0;
			memset(m_pPacket +6,0xff,(*(m_pPacket +4))-1);
		}
	}
}


bool CTsPacket::SetPacket(BYTE *pPacket)
{
	if((*pPacket) != 0x47)
		return false;

	m_pPacket =pPacket;
	return true;
}

bool CTsPacket::Get_transport_error_indicator()
{
	return (*(m_pPacket +1))&0x80;
}

bool CTsPacket::Get_payload_unit_start_indicator()
{
	return (*(m_pPacket +1))&0x40;
}

bool CTsPacket::Get_transport_priority()
{
	return (*(m_pPacket +1))&0x20;
}


WORD CTsPacket::Get_PID()
{
 	return (((*(m_pPacket +1))&0x1f)<<8)|(*(m_pPacket +2));
}

BYTE CTsPacket::Get_transport_scrambling_control()
{
	return (*(m_pPacket +3))>>6;
}

BYTE CTsPacket::Get_adaptation_field_control()
{
	return (*(m_pPacket +3) & 0x30)>>4;
}
BYTE CTsPacket::Get_continuity_counter()
{
	return (*(m_pPacket +3) & 0x0f);
}

void CTsPacket::Set_continuity_counter(BYTE Counter)
{
	(*(m_pPacket + 3)) = ((*(m_pPacket + 3)) & 0xf0 ) | (Counter & 0x0f);
}


BYTE * CTsPacket::Get_adaptation_field(BYTE &Size)
{
	BYTE adaptation_field_control = Get_adaptation_field_control();
	if(adaptation_field_control & 0x02)
	{
		if(*(m_pPacket +4)==0)
			return NULL;
		Size = (*(m_pPacket +4)) +1;//包含了长度字段
		return m_pPacket +4;
	}
	return NULL;
}

BYTE CTsPacket::Get_adaptation_field_length(BYTE *pAF_Data)
{
	return *pAF_Data;
}

bool CTsPacket::Get_discontinuity_indicator(BYTE *pAF_Data)
{		
	return (*(pAF_Data+1))>>7;
}

bool CTsPacket::Get_random_access_indicator(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>6) & 0x01;

}

bool CTsPacket::Get_elementary_stream_priority_indicator(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>5) & 0x01;
}

bool CTsPacket::Get_PCR_flag(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>4) & 0x01;
}

bool CTsPacket::Get_PCR_flag()
{
	BYTE *af;
	BYTE size;
	if((af = Get_adaptation_field(size)) == NULL)
		return false;
	else
		return ((*(af+1))>>4) & 0x01;
}


bool CTsPacket::Get_OPCR_flag(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>3) & 0x01;
}

bool CTsPacket::Get_splicing_point_flag(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>2) & 0x01;
}


bool CTsPacket::Get_transport_private_data_flag(BYTE *pAF_Data)
{
	return ((*(pAF_Data+1))>>1) & 0x01;
}


bool CTsPacket::Get_adaptation_field_extension_flag(BYTE *pAF_Data)
{
	return (*(pAF_Data+1)) & 0x01;
}

__int64 CTsPacket::Get_program_clock_reference_base(BYTE *pAF_Data)
{
	__int64 pcr_base;

	pcr_base = *(pAF_Data +2);
	pcr_base = (pcr_base <<8) | (*(pAF_Data +3));
	pcr_base = (pcr_base <<8) | (*(pAF_Data +4));
	pcr_base = (pcr_base <<8) | (*(pAF_Data +5));
	pcr_base = (pcr_base <<1) | ((*(pAF_Data +6))>>7);

	return pcr_base;
}


WORD CTsPacket::Get_program_clock_reference_extension(BYTE *pAF_Data)
{
	WORD pcr_ext;

	pcr_ext = (*(pAF_Data +6)) & 0x01;
	pcr_ext = (pcr_ext <<8) | (*(pAF_Data +7));

	return pcr_ext;	
}


PCR CTsPacket::Get_PCR(BYTE *pAF_Data)
{
	if((*pAF_Data)==0)
		return INVALID_PCR;

	if(*(pAF_Data +1) & 0x10)
	{
		return Get_program_clock_reference_base(pAF_Data)*300 + Get_program_clock_reference_extension(pAF_Data);
	}
	else
		return INVALID_PCR;
}

PCR CTsPacket::Get_PCR()
{
	BYTE size;
	BYTE *af;
	if((af = Get_adaptation_field(size)) != NULL)
	{
		if(size != 0)
			return Get_PCR(af);
	}
	return INVALID_PCR;
}

//未实现
__int64 CTsPacket::Get_original_program_clock_reference_base(BYTE *pAF_Data)
{
	return NULL;
}


WORD CTsPacket::Get_original_program_clock_reference_extension(BYTE *pAF_Data)
{
	return NULL;
	
}


PCR CTsPacket::Get_OPCR(BYTE *pAF_Data)
{
	return NULL;	
}


BYTE CTsPacket::Get_splice_countdown(BYTE *pAF_Data)
{
	return NULL;

}


BYTE CTsPacket::Get_transport_private_data_length(BYTE *pAF_Data)
{
	return NULL;
}


BYTE * CTsPacket::Get_private_data_byte(BYTE *pAF_Data,int &size)
{
	return NULL;

}


BYTE CTsPacket::Get_adaptation_field_extension_length(BYTE *pAF_Data)
{
	return NULL;

}


bool CTsPacket::Get_ltw_flag(BYTE *pAF_Data)
{
	return NULL;

}


bool CTsPacket::Get_piecewise_rate_flag(BYTE *pAF_Data)
{
	return NULL;

}


bool CTsPacket::Get_seamless_splice_flag(BYTE *pAF_Data)
{
	return NULL;

}


bool CTsPacket::Get_ltw_valid_flag(BYTE *pAF_Data)
{
	return NULL;

}


WORD CTsPacket::Get_ltw_offset(BYTE *pAF_Data)
{
	return NULL;

}


DWORD CTsPacket::Get_piecewise_rate(BYTE *pAF_Data)
{
	return NULL;

}


__int64 CTsPacket::Get_DTS_next_AU(BYTE *pAF_Data)
{
	return NULL;
}


BYTE * CTsPacket::Get_stuffing_byte(BYTE *pAF_Data,BYTE &Size)
{
	return NULL;

}


BYTE * CTsPacket::Get_Data(BYTE &Size)
{
	BYTE afc = Get_adaptation_field_control();
	if(afc==0x01)
	{
		Size = TS_PACKET_LENGTH_STANDARD - 4;
		return m_pPacket + 4;
	}
	else if(afc == 0x03)
	{
		BYTE af_length;
		Get_adaptation_field(af_length);
		Size = TS_PACKET_LENGTH_STANDARD - 4 - af_length;
		return m_pPacket +4 +af_length;
	}
	else
	{
		return NULL;
	}
}



void CTsPacket::Set_PID(PID pid)
{
	(*(m_pPacket +1)) = (pid>>8) | ((*(m_pPacket +1)) & 0xe0);
	(*(m_pPacket +2)) = (BYTE)pid ;
}

bool CTsPacket::Set_PCR(PCR pcr)
{
	if(Get_PCR_flag())
	{
		__int64 base;
		WORD ext;
		
		base = pcr/300;
		ext = (WORD)(pcr %300);


		static PCR localPcr=pcr;
		if(pcr<localPcr)
		{
			int jj=1;
		}
		BYTE size;
		BYTE *af = Get_adaptation_field(size);
		if(af != NULL)
		{
			*(af+2) = (BYTE)((base>>25) & 0xff);
			*(af+3) = (BYTE)((base>>17) & 0xff);
			*(af+4) = (BYTE)((base>>9) & 0xff);
			*(af+5) = (BYTE)((base>>1) & 0xff);
			*(af+6) = (BYTE)(((base & 0x01)<<7) | 0x7e | (ext>>8));
			*(af+7) = (BYTE)(ext & 0xff );
		}
	}
	return false;
}
bool CTsPacket::Get_PES_stream_id(BYTE &stream_id)
{
	if(!Get_payload_unit_start_indicator())
	{
		return false;
	}

	BYTE size;
	BYTE *p_data;
	if((p_data = Get_Data(size)) == NULL)
	{
		return false;
	}

	if(size <4)
	{
		return false;
	}
	if((p_data[0] == 0) &&
			(p_data[1] == 0) &&
			(p_data[2] == 1))
	{
		stream_id = p_data[3];
		return true;
	}
	return false;
}
bool	CTsPacket::Get_PES_GOP_INFO(DWORD &TimeCode/*时码信息*/)
{
	BYTE stream_id;
	if(!Get_PES_stream_id(stream_id))
	{
		return false;
	}
	//判断是否视频流
	if((stream_id & 0xF0) != 0xE0)
	{
		return false;
	}

	BYTE size;
	BYTE *p_data;
	if((p_data = Get_Data(size)) == NULL)
	{
		return false;
	}
	if(size <8)
	{
		return false;
	}

	int i;
	for(i=0;i<size -8;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 1) &&
			(p_data[i+3] == 0xB8))
		{
			//发现GOP，获取时码
			//		TIMECODE(25)																									CLOSEGOP(1)		BROKEN(1)
			//		1            5(H)    6(M)				1(marker)      6(S)                    6(F)
			//      1        11111     11   1111		1                 111      111        11111		1
			//		BYTE1                     BYTE2		                               BYTE3			        BYTE4
			
			TimeCode = (p_data[i+4]>>2) & 0x1F;//H
			TimeCode = TimeCode <<8;

			TimeCode |= (((p_data[i+4] &0x03) <<4) | (p_data[i+5]>>4))  ;//M
			TimeCode = TimeCode <<8;

			TimeCode |=(((p_data[i+5]&0x07)<<3) | (p_data[i+6]>>5)); //S
			TimeCode = TimeCode <<8;
			
			TimeCode |= (((p_data[i+6]&0x1F)<<1) |  (p_data[i+7]>>7)); //F
			return true;
		}
		
	}

	return false;

}
bool	CTsPacket::Get_PES_PIC_INFO(BYTE &PictureType/*类型1-I,2-P,3-B*/)
{
	/*
		BYTE stream_id;
	if(!Get_PES_stream_id(stream_id))
	{
		return false;
	}
	//判断是否视频流
	if((stream_id & 0xF0) != 0xE0)
	{
		return false;
	}

	BYTE size;
	BYTE *p_data;
	if((p_data = Get_Data(size)) == NULL)
	{
		return false;
	}
	if(size <8)
	{
		return false;
	}
	*/
	
	int i;
	BYTE size;
	BYTE *p_data=m_pPacket;
	size=188-5;
	for(i=4;i<size;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 1) &&
			(p_data[i+3] == 0x00))
		{
			PictureType = (p_data[i+5]>>3)&0x07;
			return true;
		}
	}
	return false;
}


bool CTsPacket::Get_SectionInfo(WORD& section_length, BYTE& section_number,BYTE& last_section_number)
{
	section_length = (*(m_pPacket+6)) & 0x0F;
	section_length = (section_length << 8) + *(m_pPacket + 7);
	section_number = *(m_pPacket + 11);
	last_section_number = *(m_pPacket + 12);
	return true;
}

BYTE CTsPacket::Get_PTS_DTS_flag()
{
	if(!Get_payload_unit_start_indicator())
	{
		return false;
	}
	BYTE size;
	BYTE flag;
	BYTE *p_data;
	if((p_data = Get_Data(size)) == NULL)
	{
		return false;
	}

	if(size <4)
	{
		return false;
	}
	if((p_data[0] == 0) &&
		   (p_data[1] == 0) &&
		   (p_data[2] == 1))
	{
		flag = p_data[7]; //from pts_dts_flags to pes_exteasion_flag
		//return flag & 0xC0;
		return flag >> 6;
	}
	return 0;
}

bool CTsPacket::Get_PTS(LONGLONG &pts)
{
    bool retval = false;
    //首先检测PES_FLAG
    if(!Get_payload_unit_start_indicator())
    {
        return false;
    }
	BYTE* pData = m_pPacket;
	BYTE adaptation_field_control = Get_adaptation_field_control();
	if (adaptation_field_control == 2 || adaptation_field_control == 3)	//含调整字段
	{
		int adaptation_field_length = *(m_pPacket + 4);
		pData = pData + adaptation_field_length + 1; //包含长度字段
	}
    
    //检测PES包头
    if((*(pData + 4) == 0x00) 
          && (*(pData + 5) == 0x00) 
          && (*(pData + 6) == 0x01))
    {
        //处理PES包，检测PTS标志
        if(ZBit::PointTest(pData + 11,0))
        {
            //获取PTS数据
            ZBIT_VAR_FROM_POINT(pts,pData + 13,-4,3,-1,15,-1,15);
            retval = true;
        }
    }
    return retval;
}

bool CTsPacket::Get_DTS(LONGLONG &dts)
{
    bool retval = false;
    //首先检测PES_FLAG
    if(!Get_payload_unit_start_indicator())
    {
        return false;
    }
    
	BYTE* pData = m_pPacket;
	BYTE adaptation_field_control = Get_adaptation_field_control();
	if (adaptation_field_control == 2 || adaptation_field_control == 3)	//含调整字段
	{
		int adaptation_field_length = *(m_pPacket + 4);
		pData = pData + adaptation_field_length + 1; //包含长度字段
	}

    //检测PES包头
    if((*(pData + 4) == 0x00) 
          && (*(pData + 5) == 0x00) 
          && (*(pData + 6) == 0x01))
    {
        //处理PES包，检测PTS标志
        if(ZBit::PointTest(pData + 11,1))
        {
            //获取DTS数据
            ZBIT_VAR_FROM_POINT(dts,pData + 13 + 5 ,-4,3,-1,15,-1,15);
            retval = true;
        }
    }
    return retval;
}


bool CTsPacket::Mpeg_Get_Sequenec_head_code()
{
	int i;
	BYTE size;
	BYTE *p_data=m_pPacket;
	size=188-3;
	for(i=4;i<size;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 1) &&
			(p_data[i+3] == 0xB3))
		{
			return true;
		}
	}
	return false;
}

bool CTsPacket::Mpeg_Get_group_start_code()
{
	int i;
	BYTE size;
	BYTE *p_data=m_pPacket;
	size=188-3;
	for(i=4;i<size;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 1) &&
			(p_data[i+3] == 0xB8))
		{
			return true;
		}
	}
	return false;
}

bool CTsPacket::H264_Get_sps()
{
	BYTE nal_unit_type = 0;
	BYTE *p_data=m_pPacket;
	BYTE size=188-4;
	for(int i=0;i<size;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 0) &&
			(p_data[i+3] == 1))
		{
			nal_unit_type = (p_data[i+3+1])&0x1F;
			if (nal_unit_type == 7)
			{
				return true;
			}
		}
	}
	return false;
}

bool CTsPacket::H264_Get_pps()
{
	BYTE nal_unit_type = 0;
	BYTE *p_data=m_pPacket;
	BYTE size=188-4;
	for(int i=0;i<size;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 0) &&
			(p_data[i+3] == 1))
		{
			nal_unit_type = (p_data[i+3+1])&0x1F;
			if (nal_unit_type == 8)
			{
				return true;
			}
		}
	}
	return false;
}

int CTsPacket::Get_ES_pos()
{
	int start_pos = 4;
	BYTE stream_id;
	BYTE adaptation_field_control = Get_adaptation_field_control();
	if (adaptation_field_control == 2 || adaptation_field_control == 3)	//含调整字段
	{
		int adaptation_field_length = *(m_pPacket + 4);
		start_pos = start_pos + adaptation_field_length + 1; //包含长度字段
	}
	if (Get_PES_stream_id(stream_id))
	{
		start_pos = start_pos + 3 + 1 + 2 +2 +1 + m_pPacket[start_pos+8];//packet_start_code_prefix + stream_id + PES_packet_length
	}
	return start_pos;
}


/*	======================================================
*	私有段结构解析器类
*   CPrivate_Section
*/


CPrivate_Section::CPrivate_Section()
:m_pSection(NULL)
{

}

CPrivate_Section::~CPrivate_Section()
{

}

bool CPrivate_Section::SetPacket(CTsPacket &Packet)
{
	BYTE size;
	BYTE *pData;//Ts包中携带的数据
	pData = Packet.Get_Data(size);
	
	if(pData == NULL)
		return false;

	if((*pData) >= size)
	{
		//指针非法 
		return false;
	}
	//(*pData)为私有段指针
	m_pSection = pData +1 + *pData;
	return true;
}


//通用私有段
BYTE CPrivate_Section::Get_table_id()
{
	return *m_pSection;
}

bool CPrivate_Section::Get_Section_sytax_indicator()
{
	return (*(m_pSection +1)) & 0x80;
}


WORD CPrivate_Section::Get_Section_length()
{
	WORD length;
	length = ((*(m_pSection +1)) & 0x03 );
	length = (length << 8 ) | (*(m_pSection +2));

	return length;
}


//标准私有段语法定义
WORD CPrivate_Section::Get_table_id_extension()
{
	WORD ext;
	ext = (*(m_pSection +3));
	ext = (ext<<8) | (*(m_pSection +4));
	return ext;
}


BYTE CPrivate_Section::Get_version_number()
{
	return ((*(m_pSection +5)) & 0x3e)>>1;
}


bool CPrivate_Section::Get_current_next_indicator()
{
	return ((*(m_pSection +5)) & 0x1);
}


BYTE CPrivate_Section::Get_section_number()
{
	return (*(m_pSection +6));
}


BYTE CPrivate_Section::Get_last_section_number()
{
	return (*(m_pSection +7));
}

//私有分段数据
BYTE *CPrivate_Section::Get_private_data_byte(WORD &Size)
{
	if(Get_Section_sytax_indicator())
	{
		//标准数据
		Size = Get_Section_length() - 9 ;
		return m_pSection + 8;
	}
	else
	{
		//非标数据
		Size = Get_Section_length();
		return m_pSection + 3;
	}
}

DWORD CPrivate_Section::Get_CRC_32()
{
	DWORD crc;
	crc = *(m_pSection + 3 + Get_Section_length() -4);
	crc = (crc << 8) | (*(m_pSection + 3 + Get_Section_length() -3));
	crc = (crc << 8) | (*(m_pSection + 3 + Get_Section_length() -2));
	crc = (crc << 8) | (*(m_pSection + 3 + Get_Section_length() -1));

	return crc;
}

bool CTsPacket::Get_VIDEO_TYPE(BYTE &VideoType/*类型1-PAL,2-NTSC*/)
{
	int i;
	BYTE size;
	BYTE frame_rate_code = 0;
	BYTE *p_data=m_pPacket;
	size=180;
	for(i=0;i<size -8;i++)
	{
		if((p_data[i] == 0) &&
			(p_data[i+1] == 0) &&
			(p_data[i+2] == 1) &&
			(p_data[i+3] == 0xb3))
		{
			frame_rate_code = (p_data[i+3+3+1])&0x0F;
			if (frame_rate_code == 0x03 || frame_rate_code == 0x06)
				VideoType = 1;
			else
				VideoType = 2;
			return true;
		}
	}
	return false;
}


