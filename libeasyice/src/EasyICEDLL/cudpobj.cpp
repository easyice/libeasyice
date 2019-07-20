/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/




#include "cudpobj.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
 #include <unistd.h>



bool IsIgmpV3(const std::string& addr)
{
	if (addr.find('@') == string::npos)
	{
		return false;
	}
	
	return true;
}

bool Ismcast(const std::string& addr)
{
	if (IsIgmpV3(addr))
	{
		return true;
	}
	
	std::string firstbyte = addr.substr(0,addr.find_first_of("."));
	int nFirstByte = atoi(firstbyte.c_str());
	if (nFirstByte >= 224 && nFirstByte <= 239)
		return true;
	return false;
}

bool Isbroadcast(const std::string& addr)
{
	std::string lastbyte = addr.substr(addr.find_last_of(".")+1,addr.size());
	int nLastByte = atoi(lastbyte.c_str());
	if (nLastByte == 255)
		return true;
	return false;
}



string GetIgmpSourceAddr(const std::string& addr)
{
	if (!IsIgmpV3(addr))
	{
		return "";
	}
	
	return addr.substr(0,addr.find('@'));
}

string GetIgmpMultiAddr(const std::string& addr)
{
	if (!IsIgmpV3(addr))
	{
		return "";
	}
	
	return addr.substr(addr.find('@')+1,addr.size());
}

CUdpObj::CUdpObj()
{
	m_stAttr.socket = -1;
}

CUdpObj::~CUdpObj()
{
	if (m_stAttr.socket != -1)
	{
		close(m_stAttr.socket);
	}
}

void CUdpObj::SetParam(const UDP_OBJ_PARAM_T& param)
{
	m_stParam = param;
	
	if (!m_stParam.ipAddr.empty())
	{
		if (m_stParam.ipAddr[0] == '@')
		{
			m_stParam.ipAddr = m_stParam.ipAddr.substr(1,m_stParam.ipAddr.size());
		}
	}
}

int CUdpObj::CreateObj()
{
	if (m_stParam.emObjType == OBJ_UDP_CLIENT)
	{
		return CreateClientObj();
	}
	else if (m_stParam.emObjType == OBJ_UDP_SERVER)
	{
		return CreateServerObj();
	}
	return 0;
}

const UDP_OBJ_ATTRIBUTE_T* CUdpObj::GetObjAttr()
{
	return &m_stAttr;
}

const UDP_OBJ_PARAM_T* CUdpObj::GetObjParam()
{
	return &m_stParam;
}


int CUdpObj::CreateClientObj()
{
	bool bbroadcast = Isbroadcast(m_stParam.ipAddr);
	bool bmcast = Ismcast(m_stParam.ipAddr);
	bool bIgmpV3 = IsIgmpV3(m_stParam.ipAddr);
	string source_addr;
	string multi_addr;
	
	if (bbroadcast)
	{
	    cout << "接收网络类型->广播" << endl;
	}
	else if (bmcast)
	{
		if (bIgmpV3)
		{
			source_addr = GetIgmpSourceAddr(m_stParam.ipAddr);
			multi_addr = GetIgmpMultiAddr(m_stParam.ipAddr);
			cout << "接收网络类型->组播 IGMP_V3"<< endl;
			cout << "source_addr=" << source_addr << endl;
			cout << "multi_addr=" << multi_addr << endl;
		}
		else
		{
			cout << "接收网络类型->组播 IGMP_V2" << endl;
		}
	}
	else
	{
		cout << "接收网络类型->单播" << endl;
	}
	
	// create sock
	if((m_stAttr.socket = socket(AF_INET,SOCK_DGRAM, 0)) == -1)
	{
		cout << "RecvData socket Init failed." << endl;
		return -1;
	}
	
	cout << "localAddr=" << m_stParam.localAddr << endl;
	//join mcast
	if (bmcast)
	{
	    cout << "source_addr=" << source_addr << endl;
	    cout << "multi_addr=" << multi_addr << endl;
		if (bIgmpV3)
		{
			struct ip_mreq_source mreq;
			mreq.imr_sourceaddr.s_addr = inet_addr(source_addr.c_str());
			mreq.imr_multiaddr.s_addr = inet_addr(multi_addr.c_str());
			
			if (m_stParam.localAddr.empty())
			{
				mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				mreq.imr_interface.s_addr =  inet_addr(m_stParam.localAddr.c_str());
			}
			
			if (setsockopt(m_stAttr.socket, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
			{
				string errstr = strerror(errno);
				cout << "setscoketopt error.errno=" << errno << ",msg=" << errstr << endl;
				close(m_stAttr.socket);
				m_stAttr.socket = -1;
				return -1;
			}
		}
		else
		{
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(m_stParam.ipAddr.c_str());
			
			if (m_stParam.localAddr.empty())
			{
				mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			}
			else
			{
				mreq.imr_interface.s_addr =  inet_addr(m_stParam.localAddr.c_str());
			}
			
 
			if (setsockopt(m_stAttr.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
			{
				string errstr = strerror(errno);
				cout << "setscoketopt error.errno=" << errno << ",msg=" << errstr << endl;
				close(m_stAttr.socket);
				m_stAttr.socket = -1;
				return -1;
			}
		}
	}
	
	memset(&m_stAttr.sock_addr_in, 0, sizeof(m_stAttr.sock_addr_in));
	m_stAttr.sock_addr_in.sin_family = AF_INET;
	m_stAttr.sock_addr_in.sin_port = htons(m_stParam.port);
	
	
	//fill sin_addr.s_addr
	if (bIgmpV3)
	{
		m_stAttr.sock_addr_in.sin_addr.s_addr = inet_addr(multi_addr.c_str());
	}
	else
	{
	    m_stAttr.sock_addr_in.sin_addr.s_addr = inet_addr(m_stParam.ipAddr.c_str());
	}
	
	m_stAttr.addr_len = sizeof(m_stAttr.sock_addr_in);
	int socketbuffersize = 1024*1024*18;
	if (setsockopt(m_stAttr.socket,SOL_SOCKET,SO_RCVBUF,(char *)&socketbuffersize,sizeof(socketbuffersize)) < 0)
	{
		string errstr = strerror(errno);
		cout << "setscoketopt error.errno=" << errno << ",msg=" << errstr << endl;
		close(m_stAttr.socket);
		m_stAttr.socket = -1;
		return -1;
	}
	
	int yes = 1;
	if (setsockopt(m_stAttr.socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{   
		string errstr = strerror(errno);
		cout << "setscoketopt error.errno=" << errno << ",msg=" << errstr << endl;
		close(m_stAttr.socket);
		m_stAttr.socket = -1;
		return -1;
	}
	
	
	if( (bind(m_stAttr.socket,(struct sockaddr *)&m_stAttr.sock_addr_in,m_stAttr.addr_len)) == -1)
	{
		cout << "Udp Bind is error.recvip=" << m_stParam.ipAddr << ",recvport=" << m_stParam.port << endl;
		close(m_stAttr.socket);
		m_stAttr.socket = -1;
		return -1;
	}
	
	//cout << "socket init ok!" << endl;
	return 0;
}

int CUdpObj::CreateServerObj()
{
	bool bbroadcast = Isbroadcast(m_stParam.ipAddr);
	bool bmcast = Ismcast(m_stParam.ipAddr);
	int ret = 0;
	

	if((m_stAttr.socket = socket(AF_INET,SOCK_DGRAM, 0)) == -1)
	{
		cout << "socket Init failed." << endl;
		return -1;
	}
	memset(&m_stAttr.sock_addr_in, 0, sizeof(m_stAttr.sock_addr_in));
	
	m_stAttr.sock_addr_in.sin_family = AF_INET;
	m_stAttr.sock_addr_in.sin_addr.s_addr = inet_addr(m_stParam.ipAddr.c_str());
	m_stAttr.sock_addr_in.sin_port = htons(m_stParam.port);
	m_stAttr.addr_len = sizeof(m_stAttr.sock_addr_in);
	
	if (bbroadcast)
	{
		int nbroad=1;
		ret = setsockopt(m_stAttr.socket, SOL_SOCKET, SO_BROADCAST, &nbroad, sizeof(nbroad));
		if (ret < 0)
		{
			printf("setsockopt error:%s\n",strerror(errno));
			return -1;
		}
	}
	else if (bmcast)
	{
		struct in_addr addr;
		inet_aton(m_stParam.localAddr.c_str(), &addr);
		ret = setsockopt(m_stAttr.socket,IPPROTO_IP,IP_MULTICAST_IF,(char*)&addr,sizeof(addr));
		if (ret < 0)
		{
			printf("setsockopt error:%s\n",strerror(errno));
			return -1;
		}

		int ttl=10;
		ret = setsockopt(m_stAttr.socket,IPPROTO_IP,IP_MULTICAST_TTL,(char*)&ttl,sizeof(ttl));
		if (ret < 0)
		{
			printf("setsockopt error:%s\n",strerror(errno));
			return -1;
		}

	}
	
	
	return 0;
}

