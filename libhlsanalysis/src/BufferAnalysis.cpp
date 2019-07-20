/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "BufferAnalysis.h"
#include <sys/time.h>
#include <stdio.h>

CBufferAnalysis::CBufferAnalysis(void)
{
	m_llSysClock = -1;
	m_fDataDuration = -1;
	m_fPreBuffered = 0;
}

CBufferAnalysis::~CBufferAnalysis(void)
{
}

void CBufferAnalysis::AddPreBuffer(double duration)
{
	m_fPreBuffered+=duration;
}

bool CBufferAnalysis::GetBufferDuration(double& duration,long long& llCurTime)
{
	if (m_llSysClock < 0 || m_fDataDuration < 0)
	{
		return false;
	}

	struct timeval tv_now;
	gettimeofday(&tv_now,NULL);

	llCurTime = (long long)tv_now.tv_sec * 1000000 + (long long)tv_now.tv_usec;

	duration = m_fDataDuration - (double)(llCurTime - m_llSysClock)/1000000;

	return true;
}

void CBufferAnalysis::Sync(double duration)
{
	struct timeval tv_now;
	gettimeofday(&tv_now,NULL);

	m_llSysClock = (long long)tv_now.tv_sec * 1000000 + (long long)tv_now.tv_usec;
	m_fDataDuration = duration;
}

void CBufferAnalysis::AddSegment(double duration)
{
	if (m_llSysClock < 0)
	{
		//重新同步系统时钟;
		Sync(duration+m_fPreBuffered);
		return;
	}

	double remain_duration;
	long long cur_time;
	GetBufferDuration(remain_duration,cur_time);
	if (remain_duration < 0)
	{
		Sync(duration);
	}
	else
	{
		m_fDataDuration += duration;
	}


}


