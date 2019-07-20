/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "stdafx.h"
#include "ccalcpcrn1.h"

CCalcPcrN1::CCalcPcrN1()
{
	m_pcrBefor = -1;
	m_fTransportRate = -1;
	m_nPacketCountOfPcr = 0;
}

CCalcPcrN1::~CCalcPcrN1()
{

}

void CCalcPcrN1::AddPcrPacket(long long pcr)
{
	if (m_pcrBefor < 0)	
	{
		m_pcrBefor = pcr;
		return;
	}
	
	m_fTransportRate = ( (double)(m_nPacketCountOfPcr*188)*27000000 )/(double)(pcr - m_pcrBefor);
	m_nPacketCountOfPcr = 1;
	m_pcrBefor = pcr;
}

void CCalcPcrN1::AddPayloadPacket()
{
	if (m_pcrBefor != -1)
	{
		m_nPacketCountOfPcr++;
	}
}

long long CCalcPcrN1::GetPcr()
{
	if (m_fTransportRate < 0)
		return -1;
	//return m_pcrBefor + ((long long)m_nPacketCountOfPcr*27000000*188)/m_fTransportRate;
	return m_pcrBefor + ((m_nPacketCountOfPcr*188)*27000000)/m_fTransportRate;
}

