/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "commondefs.h"
#include "CDescriptor.h"
#include "CDescriptorImpl.h"



// 类的静态成员变量要在类体外进行定义
tables::CDescriptor* tables::CDescriptor::m_pStatic = new CDescriptor();



tables::CDescriptor::CDescriptor(void)
{
	m_pDescriptorImpl = new CDescriptorImpl();
	pthread_mutex_init(&m_mutex,NULL);
}

tables::CDescriptor::~CDescriptor(void)
{
	delete m_pDescriptorImpl;
	pthread_mutex_destroy(&m_mutex);
}

tables::CDescriptor* tables::CDescriptor::GetInstancePtr()
{
	// if (NULL == m_pStatic)
	// {
	// 	m_pStatic = new CDescriptor();
		
	// }

	return m_pStatic;
}

void tables::CDescriptor::Destroy()
{
	delete m_pStatic;
	m_pStatic = NULL;
}


void tables::CDescriptor::DecodeDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,BYTE* pData,int nLen)
{
	pthread_mutex_lock(&m_mutex);
	m_pDescriptorImpl->DecodeDescriptor(descriptor_n,pData,nLen);
	pthread_mutex_unlock(&m_mutex);
}