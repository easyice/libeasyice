/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "stdafx.h"
#include "csysclock.h"
#include "tr101290_defs.h"
#include "global.h"

using namespace tr101290;

CSysClock::CSysClock()
{
	Reset();
}

CSysClock::~CSysClock()
{

}

void CSysClock::SetInitRate(double rate)
{
	m_fTransportRate = rate;
}

void CSysClock::Reset()
{
	m_pcrBefor = -1;
	m_fTransportRate = -1;
	//m_fTransportRate = 10*1024*1024/8;
	m_nPacketCountOfPcr = 0;
}

void CSysClock::AddPcrPacket(long long pcr)
{
	if (m_pcrBefor < 0)	
	{
		m_pcrBefor = pcr;
		m_nPacketCountOfPcr = 1;
		return;
	}

	long long pcr_it;//pcr ¼ä¸ô

	if (pcr < m_pcrBefor)
	{
		pcr_it = (PCR_MAX - m_pcrBefor) + pcr;
	}
	else
	{
		pcr_it = pcr - m_pcrBefor;
	}
	
	m_fTransportRate = ( (double)(m_nPacketCountOfPcr*188)*27000000 )/(double)(pcr_it);
	m_nPacketCountOfPcr = 1;
	m_pcrBefor = pcr;
}

void CSysClock::AddPayloadPacket()
{
	if (m_pcrBefor != -1)
	{
		m_nPacketCountOfPcr++;
	}
}

long long CSysClock::GetPcr()
{
	//return m_pcrBefor + ((long long)m_nPacketCountOfPcr*27000000*188)/m_fTransportRate;

	if (m_fTransportRate < 0)
	{
		return m_pcrBefor;
	}

	long long llPcr = m_pcrBefor + ((m_nPacketCountOfPcr*188)*27000000)/m_fTransportRate;
	return  llPcr %  (PCR_MAX+1);
}

long long CSysClock::GetPcrPrev()
{
	return m_pcrBefor;
}

