/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "StreamFilterRtp.h"


#define RTP_VERSION 2



//return 0 if sucess
static int strip_rtp(const BYTE *data,  const int len,int *out_head_len)
{
	if (data == NULL)
		return -1;

	if (len < 12)
        return -1;

	if ((data[0] & 0xc0) != (RTP_VERSION << 6))
        return -1;

	int cc = data[0] & 0xF;

	int head_len = 12 + cc*4;

	if (len < head_len)
        return -1;

	*out_head_len = head_len;

	return 0;

}


CStreamFilterRtp::CStreamFilterRtp(void)
{
}

CStreamFilterRtp::~CStreamFilterRtp(void)
{
}

BYTE* CStreamFilterRtp::ProcessBuffer(BYTE* pData,int nLen,int& nRetLen)
{
	int head_len = 0;
	int ret = strip_rtp(pData,nLen,&head_len);
	if (ret != 0)
	{
		return NULL;
	}
	nRetLen = nLen - head_len;
	return pData+head_len;
}

