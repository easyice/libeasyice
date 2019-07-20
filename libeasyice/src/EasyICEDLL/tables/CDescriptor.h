/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CDESCRIPTOR_H
#define CDESCRIPTOR_H

#include "commondefs.h"
#include <pthread.h>





class CDescriptorImpl;

namespace tables{


class CDescriptor
{
public:
	
	~CDescriptor(void);
	static CDescriptor* GetInstancePtr();

	static void Destroy();

	void DecodeDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,BYTE* pData,int nLen);
private:
	CDescriptor(void);

	// 静态成员变量,提供全局惟一的一个实例
	static CDescriptor* m_pStatic;

	CDescriptorImpl* m_pDescriptorImpl;

    pthread_mutex_t m_mutex;
};


}



#endif

