/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "Dvb.h"

//关闭将int类型转换成bool的警告信息
#pragma warning (disable:4800)

#define MPEG_FRAME_TYPE_I	1
#define MPEG_FRAME_TYPE_P	2
#define MPEG_FRAME_TYPE_B	3


class CTsPacket
{
public:
	CTsPacket(void);
	~CTsPacket(void);

	 bool SetPacket(BYTE *pPacket);

//设置信息
	 void Set_PID(PID pid);
	 bool Set_PCR(PCR pcr);
	 void Set_continuity_counter(BYTE Counter);
	 void RemoveAfField();
//基本信息
	 bool Get_transport_error_indicator();
	 bool Get_payload_unit_start_indicator();
	 bool Get_transport_priority();
	 WORD Get_PID();
	 BYTE Get_transport_scrambling_control();
	 BYTE Get_adaptation_field_control();
	 BYTE Get_continuity_counter();

//填充字段信息
	 BYTE * Get_adaptation_field(BYTE &Size);
	 BYTE Get_adaptation_field_length(BYTE *pAF_Data);
	 bool Get_discontinuity_indicator(BYTE *pAF_Data);
	 bool Get_random_access_indicator(BYTE *pAF_Data);
	 bool Get_elementary_stream_priority_indicator(BYTE *pAF_Data);
	 bool Get_PCR_flag(BYTE *pAF_Data);
	 bool Get_PCR_flag();
	 bool Get_OPCR_flag(BYTE *pAF_Data);
	 bool Get_splicing_point_flag(BYTE *pAF_Data);
	 bool Get_transport_private_data_flag(BYTE *pAF_Data);
	 bool Get_adaptation_field_extension_flag(BYTE *pAF_Data);
	 __int64 Get_program_clock_reference_base(BYTE *pAF_Data);
	 WORD Get_program_clock_reference_extension(BYTE *pAF_Data);
	 PCR  Get_PCR(BYTE *pAF_Data);
	 PCR  Get_PCR();

	 __int64 Get_original_program_clock_reference_base(BYTE *pAF_Data);
	 WORD Get_original_program_clock_reference_extension(BYTE *pAF_Data);
	 PCR  Get_OPCR(BYTE *pAF_Data);
	 BYTE Get_splice_countdown(BYTE *pAF_Data);
	 BYTE Get_transport_private_data_length(BYTE *pAF_Data);
	 BYTE *Get_private_data_byte(BYTE *pAF_Data,int &size);
	 BYTE Get_adaptation_field_extension_length(BYTE *pAF_Data);
	 bool Get_ltw_flag(BYTE *pAF_Data);
	 bool Get_piecewise_rate_flag(BYTE *pAF_Data);
	 bool Get_seamless_splice_flag(BYTE *pAF_Data);
	 bool Get_ltw_valid_flag(BYTE *pAF_Data);
	 WORD Get_ltw_offset(BYTE *pAF_Data);
	 DWORD Get_piecewise_rate(BYTE *pAF_Data);
	 __int64 Get_DTS_next_AU(BYTE *pAF_Data);
	 BYTE *Get_stuffing_byte(BYTE *pAF_Data,BYTE &Size);

//获取PES GOP信息
	 bool Get_PES_stream_id(BYTE &stream_id);
	 bool Get_PES_GOP_INFO(DWORD &TimeCode/*时码信息*/);
	 bool Get_PES_PIC_INFO(BYTE &PictureType/*时码信息*/);
	 BYTE Get_PTS_DTS_flag();
//获取视频信息
	 bool Get_VIDEO_TYPE(BYTE &VideoType/*类型1-PAL,2-NTSC*/);
	 bool Get_PTS(LONGLONG &pts);
	 bool Get_DTS(LONGLONG &dts);
//获取有效负载
	 BYTE * Get_Data(BYTE &Size);
//Section information
	 bool Get_SectionInfo(WORD& section_length, BYTE& section_number,BYTE& last_section_number);

	 bool H264_Get_slice_type(BYTE &SliceType/*10:IDR */);
	 bool Mpeg_Get_Sequenec_head_code();
	 bool Mpeg_Get_group_start_code();
	 bool H264_Get_sps();
	 bool H264_Get_pps();

	 int Get_ES_pos();
//获取缓冲
	BYTE *m_pPacket;
};

/*	======================================================
*	私有段结构解析器类
*/

class CPrivate_Section
{
public:
	CPrivate_Section();
	~CPrivate_Section();
	
	bool SetPacket(CTsPacket &Packet);
	
//通用私有段
	BYTE Get_table_id();
	bool Get_Section_sytax_indicator();
	WORD Get_Section_length();
//标准私有段语法定义
	WORD Get_table_id_extension();
	BYTE Get_version_number();
	bool Get_current_next_indicator();
	BYTE Get_section_number();
	BYTE Get_last_section_number();

//私有分段数据
	BYTE * Get_private_data_byte(WORD &Size);
//CRC数据
	DWORD Get_CRC_32();
	
private:
    BYTE *m_pSection;
};

/**
* 封装描述子信息的类
*/

class CDescriptor
{
public:
	CDescriptor(){};
	~CDescriptor(){};
	void SetDescriptor(BYTE *Descriptor);
	BYTE Get_descriptor_tag();
	BYTE Get_descriptor_length();
private:
	BYTE *m_pDescriptor;
};
