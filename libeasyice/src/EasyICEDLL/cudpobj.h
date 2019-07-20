/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef CUDPOBJ_H
#define CUDPOBJ_H

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>


using namespace std;



typedef enum _UDP_OBJ_TYPE_T
{
	OBJ_UDP_CLIENT,
	OBJ_UDP_SERVER
}UDP_OBJ_TYPE_T;


typedef struct _UDP_OBJ_PARAM_T
{
	UDP_OBJ_TYPE_T emObjType;
	string ipAddr; // client 或 server ip地址，当为client并且是组播地址时，支持 ip@mulIP方式
	int port;	//client 或 server port
	string localAddr; //当ipAddr为组播地址时，本字段表示使用的本级ip地址;
}UDP_OBJ_PARAM_T;


typedef struct _UDP_OBJ_ATTRIBUTE_T
{
	int socket;
	struct sockaddr_in sock_addr_in;
	int addr_len;
}UDP_OBJ_ATTRIBUTE_T;



class CUdpObj
{
public:
	CUdpObj();
	~CUdpObj();
    
	void SetParam(const UDP_OBJ_PARAM_T& param);
	int CreateObj();
	const UDP_OBJ_ATTRIBUTE_T* GetObjAttr();
	const UDP_OBJ_PARAM_T* GetObjParam();
    
private:
	int CreateServerObj();
	int CreateClientObj();
private:
	UDP_OBJ_PARAM_T m_stParam;
	UDP_OBJ_ATTRIBUTE_T m_stAttr;
};

#endif // CUDPOBJ_H
