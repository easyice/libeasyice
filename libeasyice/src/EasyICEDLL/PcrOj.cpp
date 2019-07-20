/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "PcrOj.h"



CPcrOj::CPcrOj(void)
{
	m_llSysClockStart = 0;

	m_llPcrDuration = 0;
	m_llPcrPrev = -1;
}

CPcrOj::~CPcrOj(void)
{
}

long long CPcrOj::RecvPcr(long long pcr,long long sys_clock)
{
	if (m_llPcrPrev < 0)
	{
		m_llSysClockStart = sys_clock;
		m_llPcrPrev = pcr;
		return 0;
	}

	// pcr ±äÐ¡
	if (pcr < m_llPcrPrev)
	{
		m_llPcrDuration = m_llPcrDuration + ((long long)0x1FFFFFFFF*300 + 299 - m_llPcrPrev) + pcr;
	}
	else
	{
		m_llPcrDuration = m_llPcrDuration + (pcr - m_llPcrPrev);
	}

	m_llPcrPrev = pcr;


	long long sys_duration_usec = sys_clock - m_llSysClockStart;
	long long pcr_duration_usec = m_llPcrDuration / 27;

	m_llSysClockStart = sys_clock;
	m_llPcrDuration = 0;
	
	return sys_duration_usec - pcr_duration_usec;
}
