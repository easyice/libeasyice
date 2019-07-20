/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "CircularBuffer.h"

#define TS_PACKET_SIZE 188


CCircularBuffer::CCircularBuffer(void)
{
	m_start = 1;
    m_end = 0;
	m_item_data = NULL;
	m_pItemInfo = NULL;
	m_item_size = 0;
}

CCircularBuffer::~CCircularBuffer(void)
{
	delete [] m_item_data;
	delete [] m_pItemInfo;
}

void CCircularBuffer::Init(int circ_buf_size,int TS_in_packet)
{
	m_size = circ_buf_size;
	m_TS_in_item = TS_in_packet;

	m_item_size = TS_in_packet*TS_PACKET_SIZE;
	m_item_data = new BYTE[circ_buf_size*m_item_size];
	m_pItemInfo = new ITEMINFO_T[circ_buf_size];
}

/*
 * Is the buffer empty?
 */
inline bool CCircularBuffer::Empty()
{
  return (m_start == (m_end + 1) % m_size);
}

/*
 * Is the buffer full?
 */
inline bool CCircularBuffer::Full()
{
  return ((m_end + 2) % m_size == m_start);
}

bool CCircularBuffer::WriteItem(BYTE* pData,int nLen,long long llTime)
{
	if (Full())
	{
		return false;
	}

	int end = (m_end+1) % m_size;

	memcpy(m_item_data+m_item_size*end,pData,nLen);
	m_pItemInfo[end].item_size = nLen;
	m_pItemInfo[end].time = llTime;

	m_end = end;

	return true;

}

BYTE* CCircularBuffer::ReadItem(ITEMINFO_T& info)
{
	if (Empty())
	{
		info.item_size = 0;
		return NULL;
	}

	info.item_size = m_pItemInfo[m_start].item_size;
	info.time = m_pItemInfo[m_start].time;

	BYTE* p = m_item_data + m_item_size*m_start;

	m_start = (m_start +1) % m_size;
	return p;
}
