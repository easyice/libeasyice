/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "H264Dec.h"
#include "H264DecodeCore.h"


CH264Dec::CH264Dec(void)
{
	m_pDecoder = new CH264DecodeCore();
}

CH264Dec::~CH264Dec(void)
{
	delete m_pDecoder;
}


STRING_TREE* CH264Dec::ParseTsPacket(BYTE *pPacket, int nLen)
{
	return m_pDecoder->ParseTsPacket(pPacket,nLen);
}

PARSED_FRAME_INFO CH264Dec::ParseTsContinue(BYTE* pPacket,int nLen)
{
	return m_pDecoder->ParseTsContinue(pPacket,nLen);
}

void CH264Dec::Reset()
{
	return m_pDecoder->Reset();
}
