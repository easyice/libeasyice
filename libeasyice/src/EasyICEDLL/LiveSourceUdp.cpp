/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "LiveSourceUdp.h"
#include <iostream>
#include "EiLog.h"
#include "cudpobj.h"
#include <errno.h>


CLiveSourceUdp::CLiveSourceUdp(void)
{
	m_pUdpObj = new CUdpObj();

}

CLiveSourceUdp::~CLiveSourceUdp(void)
{
	delete m_pUdpObj;
}

int CLiveSourceUdp::Run()
{
    m_pUdpObj->SetParam(m_stParam);

    if (m_pUdpObj->CreateObj() < 0)
    {
    	ei_log(LV_ERROR,"libeasyice","socket init faild!");
    	return -1;
    }
	RunThread();
	return 0;
}

void CLiveSourceUdp::WorkFun()
{
	fd_set rset;

    const UDP_OBJ_ATTRIBUTE_T* attr = m_pUdpObj->GetObjAttr();
    
	while(1)
	{
		if (m_bStop)
		{
			break;
		}


		FD_ZERO(&rset);
		FD_SET(attr->socket,&rset);
        timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
		int ret = select(attr->socket+1,&rset,NULL,NULL,&tv);
		if(ret == -1)
		{   
			ei_log(LV_ERROR,"easyicedll","udp recv error:%s",strerror(errno));
			return ;
		}
		else if(ret==0)
		{
			ei_log(LV_WARNING,"easyicedll","udp recv timeout");
			continue;
		}

		struct sockaddr_in c_addr;
		socklen_t addr_len = 0;
		if(FD_ISSET(attr->socket,&rset))
		{
			addr_len = sizeof(c_addr);
			int nLen = recvfrom(attr->socket,(char*)m_pBuffer,m_nBufferLen,0,(struct sockaddr *)&c_addr,&addr_len);
			if (nLen <= 0 /*|| (nLen%188 != 0)*/)
			{
				ei_log(LV_ERROR,"easyicedll","udp recv error:%d",errno);
				return;
			}

            if (m_strRemoteAddress.empty())
            {
		        m_strRemoteAddress = inet_ntoa(c_addr.sin_addr);
                m_nRemotePort = ntohs(c_addr.sin_port);
            }
			if (m_pFilter == NULL)
			{
				m_pRecvDataCB(m_pApp,m_pBuffer,nLen);
				
			}
			else //经过滤器处理
			{
				int n_tmp_len;
				BYTE* p_tmp_data = m_pFilter->ProcessBuffer(m_pBuffer,nLen,n_tmp_len);
				if (p_tmp_data != NULL)
				{
					m_pRecvDataCB(m_pApp,p_tmp_data,n_tmp_len);
				}
			}
			
		}
	}
}



