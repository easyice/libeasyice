/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "ztypes.h"

class CCircularBuffer
{
public:
typedef struct _ITEMINFO_T
{
		int item_size;  // and thus the size of said item's data array  字节数
		long long time; //usec
}ITEMINFO_T;

public:
	CCircularBuffer(void);
	~CCircularBuffer(void);
	void Init(int circ_buf_size,int TS_in_packet);
	bool WriteItem(BYTE* pData,int nLen,long long llTime);
	BYTE* ReadItem(ITEMINFO_T& info);
private:
	bool Empty();
	bool Full();
private:
	int      m_start;      // start of data "pointer"
	int      m_end;        // end of data "pointer" (you guessed)
	int      m_size;       // the actual length of the `item` array

	int      m_TS_in_item; // max number of TS packets in a circular buffer item 每元素最大TS包数量

	int		m_item_size;
	BYTE     *m_item_data;
	ITEMINFO_T *m_pItemInfo;

};
