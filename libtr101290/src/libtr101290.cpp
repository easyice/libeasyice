/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// libtr101290.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "libtr101290.h"
#include "global.h"
#include "TrCore.h"
#include <iostream>

using namespace tr101290;


#define SDK_RELEASE





// 这是导出变量的一个示例
 int nlibtr101290=0;

// 这是导出函数的一个示例。
 int fnlibtr101290(void)
{
	return 42;
}


// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 libtr101290.h
Clibtr101290::Clibtr101290()
{
	m_pTrCore = new CTrCore();

#ifdef SDK_RELEASE
	nlibtr101290 = CALL_PASSWD;
#endif
}

Clibtr101290::~Clibtr101290()
{
	delete m_pTrCore;

}

void Clibtr101290::SetReportCB(pfReportCB pCB,void* pApp)
{
	m_pTrCore->SetReportCB(pCB,pApp);
}

void Clibtr101290::SetStartOffset(long long llOffset)
{
	if (nlibtr101290 != CALL_PASSWD)
	{
		llOffset += 188*204*1000;
	}
	m_pTrCore->SetStartOffset(llOffset);
}

void Clibtr101290::SetTsLen(int nLen)
{
	m_pTrCore->SetTsLen(nLen);
}

void Clibtr101290::SetEnable(bool *p)
{
	m_pTrCore->SetEnable(p);
}
	
void Clibtr101290::AddPacket(BYTE* pPacket)
{
	m_pTrCore->AddPacket(pPacket);
}

bool Clibtr101290::IsDemuxFinish()
{
	return m_pTrCore->IsDemuxFinish();
}
