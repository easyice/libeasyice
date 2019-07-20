/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "tr101290_defs.h"



namespace tr101290
{


static const long long PCR_MAX = (long long)0x1FFFFFFFF*300 + 299;


//extern long long g_llOffset;
//extern void* g_pApp;
//extern pfReportCB  g_pfReportCB;

//extern CSysClock* g_pSysClock;


//extern  void Report(int level,ERROR_NAME_T errName,long long llOffset,int pid,long long llVal,double fVal);
//extern  void Report(int level,ERROR_NAME_T errName,int pid,long long llVal,double fVal);

extern long long diff_pcr(long long curPcr,long long prevPcr);




static const char* SDK_CODE="defs_default_streamtype=0x02"; //北京正奇联讯

//static const char* SDK_CODE="defs_default_streamtype=0x1b"; //北京正奇联讯



};




#endif


