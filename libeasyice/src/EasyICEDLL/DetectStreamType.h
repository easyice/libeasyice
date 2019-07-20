/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "TsPacket.h"

class CDetectStreamType
{
public:
	CDetectStreamType(void);
	~CDetectStreamType(void);

	void Init(BYTE* pData, int nLen, int vpid);
	void Init(BYTE* pData, int nLen);
	int GetVideoType();

private:
	int GetVideoPid();
	bool Mpeg_Get_Sequenec_head_code(BYTE* pPacket,int nLen);
	bool Mpeg_Get_group_start_code(BYTE* pPacket,int nLen);
	bool H264_Get_sps(BYTE* pPacket,int nLen);
	bool H264_Get_pps(BYTE* pPacket,int nLen);
	bool IsMpegVideo();
	bool IsH264Video();
private:
	BYTE* m_pBuffer;
	int m_nBufLen;
	int m_nVpid;

};
