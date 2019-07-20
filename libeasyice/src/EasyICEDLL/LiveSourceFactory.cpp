/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "StdAfx.h"
#include "LiveSourceFactory.h"
#include "LiveSourceUdp.h"
#include "StreamFilterRtp.h"
#include <string>
#include "utils.h"


using namespace std;

CLiveSourceFactory::CLiveSourceFactory(void)
{
}

CLiveSourceFactory::~CLiveSourceFactory(void)
{
}

CLiveSourceBase* CLiveSourceFactory::CreateSource(const char* strMRL,const char* strLocalIp)
{
	CLiveSourceBase* pSource = NULL;

//	char* copy = strdup(strMRL);

	//to lower case
//	strlwr(copy);

	if (strncasecmp(strMRL,("udp"),3) == 0)
	{
		pSource = new CLiveSourceUdp();
	}
	else if (strncasecmp(strMRL,("rtp"),3) == 0)
	{
		pSource = new CLiveSourceUdp();
		pSource->SetFilter(new CStreamFilterRtp());
	}

	if (pSource != NULL)
	{
		UDP_OBJ_PARAM_T param;
		ParseParam(strMRL,param);
		param.localAddr = strLocalIp;
		pSource->SetParam(param);
	}

//	free(copy);
	return pSource;
}

void CLiveSourceFactory::ParseParam(const char* strMRL,UDP_OBJ_PARAM_T& param)
{

	string mrl = strMRL;
	string::size_type start = mrl.find(("//"));
	if (start == string::npos)
	{
		start = mrl.find(("\\\\"));
	}

	if (start == string::npos)
	{
		return;
	}

	string::size_type end = mrl.find((":"),start);
	if(end == string::npos)
	{
		return;
	}

    param.emObjType = OBJ_UDP_CLIENT;
	param.port = atoi( mrl.substr(end+1).c_str() );

    param.ipAddr= mrl.substr(start+2,end - start-2);
//	start = address.find(("@"));
//	if (start == string::npos)
//	{
//		param.recv_addr = address;
//		//param.source_addr = _T("0.0.0.0");
//	}
//	else if (start == 0)
//	{
//		param.recv_addr = address.substr(1);
//	}
//	else
//	{
//		param.source_addr = address.substr(0,start);
//		param.recv_addr = address.substr(start+1);
//	}
}
