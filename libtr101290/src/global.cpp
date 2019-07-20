/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "global.h"


using namespace tr101290;


//long long tr101290::g_llOffset = 0;
//void* tr101290::g_pApp = NULL;
//pfReportCB  tr101290::g_pfReportCB;


//用于获取当前时间
//CSysClock* tr101290::g_pSysClock = NULL;


//void tr101290::Report(int level,ERROR_NAME_T errName,long long llOffset,int pid,long long llVal,double fVal)
//{
//	REPORT_PARAM_T param;
//	param.level = level;
//	param.errName = errName;
//	param.llOffset = llOffset;
//
//	param.pApp = g_pApp;
//	param.pid = pid;
//	param.llVal = llVal;
//	param.fVal = fVal;
//
//	g_pfReportCB(param);
//}
//
//
//void tr101290::Report(int level,ERROR_NAME_T errName,int pid,long long llVal,double fVal)
//{
//	Report(level,errName,g_llOffset,pid,llVal,fVal);
//}

long long tr101290::diff_pcr(long long curPcr,long long prevPcr)
{
	if (curPcr > prevPcr)
	{
		return curPcr - prevPcr;
	}


	//异常变小
	if (PCR_MAX - prevPcr > 27000000)
	{
		return curPcr - prevPcr;
	}

	//正常变小
	return (PCR_MAX - prevPcr) + curPcr;
}