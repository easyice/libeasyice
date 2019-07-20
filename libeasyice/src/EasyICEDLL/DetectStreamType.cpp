/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "DetectStreamType.h"

CDetectStreamType::CDetectStreamType(void)
{
	m_pBuffer = NULL;
	m_nBufLen = 0;
	m_nVpid = -1;
}

CDetectStreamType::~CDetectStreamType(void)
{
}

void CDetectStreamType::Init(BYTE* pData, int nLen, int vpid)
{
	m_pBuffer = pData;
	m_nBufLen = nLen;
	m_nVpid = vpid;
}

void CDetectStreamType::Init(BYTE* pData, int nLen)
{
	m_pBuffer = pData;
	m_nBufLen = nLen;
	m_nVpid = GetVideoPid();
}

int CDetectStreamType::GetVideoPid()
{
	CTsPacket tsPacket;
	BYTE stream_id;
	for (int i = 0; i < m_nBufLen; i+= 188)
	{
		if (!tsPacket.SetPacket(m_pBuffer+i))
		{
			continue;
		}
		if (tsPacket.Get_PES_stream_id(stream_id))
		{
			if (stream_id >= 0xE0 && stream_id <= 0xEF)
				return tsPacket.Get_PID();
		}
	}
	return -1;
}

bool CDetectStreamType::Mpeg_Get_Sequenec_head_code(BYTE* pPacket,int nLen)
{
	int i;
	BYTE *p_data=pPacket;
	for(i=4;i<nLen-3;i++)
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

bool CDetectStreamType::Mpeg_Get_group_start_code(BYTE* pPacket,int nLen)
{
	int i;
	BYTE *p_data=pPacket;
	for(i=4;i<nLen-3;i++)
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

bool CDetectStreamType::IsMpegVideo()
{
	CTsPacket tsPacket;
	for (int i = 0; i < m_nBufLen; i+= 188)
	{
		if (!tsPacket.SetPacket(m_pBuffer+i))
		{
			continue;
		}
		if (tsPacket.Get_PID() == m_nVpid)
		{
			if (Mpeg_Get_Sequenec_head_code(m_pBuffer+i,188))
			{				
				for (int j = i; j < m_nBufLen; j += 188)
				{
					if (!tsPacket.SetPacket(m_pBuffer+j))
					{
						continue;
					}
					if (tsPacket.Get_PID() == m_nVpid)
					{
						if (Mpeg_Get_group_start_code(m_pBuffer+j,188))
							return true;
					}
				}
			}
		}
	}
	return false;
}

//get seq_parameter_set
bool CDetectStreamType::H264_Get_sps(BYTE* pPacket,int nLen)
{
	BYTE nal_unit_type = 0;
	BYTE *p_data=pPacket;
	BYTE size=nLen-4;
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

//get pic_parameter_set
bool CDetectStreamType::H264_Get_pps(BYTE* pPacket,int nLen)
{
	BYTE nal_unit_type = 0;
	BYTE *p_data=pPacket;
	BYTE size=nLen-4;
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

bool CDetectStreamType::IsH264Video()
{
	CTsPacket tsPacket;
	for (int i = 0; i < m_nBufLen; i+= 188)
	{
		if (!tsPacket.SetPacket(m_pBuffer+i))
		{
			continue;
		}
		if (tsPacket.Get_PID() == m_nVpid)
		{
			if (H264_Get_sps(m_pBuffer+i,188))
			{				
				for (int j = i; j < m_nBufLen; j += 188)
				{
					if (!tsPacket.SetPacket(m_pBuffer+j))
					{
						continue;
					}
					if (tsPacket.Get_PID() == m_nVpid)
					{
						if (H264_Get_pps(m_pBuffer+j,188))
							return true;
					}
				}
			}
		}
	}
	return false;
}

int CDetectStreamType::GetVideoType()
{
	if (m_pBuffer == NULL || m_nBufLen < 0 || m_nVpid < 0)
	{
		return -1;
	}

	if (IsMpegVideo())	
		return 2;
	else if (IsH264Video())
		return 0x1B;

	return -1;
}