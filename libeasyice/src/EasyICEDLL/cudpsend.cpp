/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "cudpsend.h"



CUdpSend::CUdpSend(void)
{
	m_sock = -1;

}

CUdpSend::~CUdpSend(void)
{
	if (m_sock != -1) 
	{
		closesocket(m_sock);
	}
}

int CUdpSend::InitSend(const string& sendip,int sendport)
{
	int err;


	if((m_sock = socket(AF_INET,SOCK_DGRAM, 0)) == -1)
	{
		err = GetLastError();
		cout << "SendData socket Init failed." << errno<< endl;
		return -1;
	}

	m_s_addr.sin_family = AF_INET;
	m_s_addr.sin_port = htons(sendport);
	m_s_addr.sin_addr.s_addr = inet_addr(sendip.c_str());


	return 0;
}


int CUdpSend::SendData(BYTE *buf, int buf_size)
{
	int send_len;
	send_len = sendto(m_sock, (char*)buf, buf_size, 0, (struct sockaddr *)&m_s_addr, sizeof(m_s_addr));

	if (send_len != buf_size) 
	{
		int err = GetLastError();
		cout << "Send data failed." << errno<< endl;
		return -1;
	}

	return 0;
}

