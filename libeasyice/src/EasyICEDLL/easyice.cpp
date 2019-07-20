/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "easyice.h"

#include "MpegDec.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "CheckMediaInfo.h"


using namespace std;

const int READ_BUF_SIZE = 3850240;

extern bool g_bEnableDebug;
extern int g_gop_size_threshold;
extern bool g_check_part;



static int TryToReSync(BYTE* pData,int nLength,int& nSycByte)
{
	nSycByte = 0;
	int maxLength = nLength - 204*3;
	if (maxLength < 204*3)
		return -1;

	for (int i = 0; i < maxLength; i++)
	{
		if(pData[i]==0x47&&pData[i+188]==0x47&&pData[i+188*2]==0x47&&pData[i+188*3]==0x47&&pData[i+188*4]==0x47)
		{	
			nSycByte = i;
			return 188;
		}
		else if(pData[i]==0x47&&pData[i+204]==0x47&&pData[i+204*2]==0x47&&pData[i+204*3]==0x47&&pData[i+204*4]==0x47)
		{
			nSycByte = i;
			return 204;
		}
	}
	return -1;
}


int GetTsLength(const char* PathName,int& nSyncByte)
{
	nSyncByte = 0;
	int ret = 0;
	FILE *fp = NULL;

	int bufferLength = 204*100;
	BYTE *pBuffer = new BYTE[bufferLength];
	memset(pBuffer,0,bufferLength);
	fp = fopen(PathName,"rb");
	if (fp == NULL)
	{
		delete [] pBuffer;
		return -1;
	}

	size_t numread = fread(pBuffer,sizeof(BYTE),bufferLength,fp);
	//if (numread != sizeof(pBuffer))
	//{
	//	fclose(fp);
	//	delete [] pBuffer;
	//	return ERROR_FILE_NOT_DATA;
	//}
	if(pBuffer[0]==0x47&&pBuffer[188]==0x47&&pBuffer[188*2]==0x47&&pBuffer[188*3]==0x47&&pBuffer[188*4]==0x47)
			ret=188;
	else if(pBuffer[0]==0x47&&pBuffer[204]==0x47&&pBuffer[204*2]==0x47&&pBuffer[204*3]==0x47&&pBuffer[204*4]==0x47)
			ret=204;
	else
	{
		//尝试重新同步
			ret = TryToReSync(pBuffer,bufferLength,nSyncByte);
	}
	delete [] pBuffer;
	fclose(fp);
	return ret;

}

static bool is_begin_with(const string& input,const string& key)
{
        if (input.find(key) ==string::npos)
        {
                return false;
        }
        return input.find(key) == 0;
}



static string format_ei_info( CMpegDec* pDec)
{
	if (g_bEnableDebug)
	{
		cerr << "g_gop_size_threshold="<<g_gop_size_threshold<<endl;
	}

	ALL_PROGRAM_INFO* allprograminfo = pDec->GetAllProgramInfo();
	PROGRAM_INFO* pi = allprograminfo->begin()->second;

	if (allprograminfo->empty())
	{
		cerr << "allprograminfo is empty!" << endl;
		return "";
	}


	bool b_all_idr = true;
	long long gop_size_total = 0;
	int max_gop_size = 0;
	//const int gop_size_threshold = 5;//gop size门限，全部小于此值，将b_gop_size_alarm设置为true
	bool b_gop_size_alarm = true;

	std::vector<GOP_LIST>::iterator it = pi->gopList.begin();
	for(; it != pi->gopList.end();++it)
	{
		if (!is_begin_with(it->gop,"IDR"))
		{
			b_all_idr = false;
		}
		if (it->gop_size > max_gop_size)
		{
			max_gop_size = it->gop_size;
		}
		if (it->gop_size > g_gop_size_threshold)
		{
			b_gop_size_alarm = false;
		}

		if (g_bEnableDebug)
		{
			cerr <<it->gop<<endl;
		}
		gop_size_total +=  it->gop_size ;
	}
	if (!pi->gopList.empty())
	{
		gop_size_total /= pi->gopList.size();
	}
	
	stringstream ss;
	ss << "<IsAllIdr>" ;
	if (b_all_idr) 
	{
		ss << "true";
	}
	else
	{
		ss << "false";
	}
	ss <<"</IsAllIdr>\n";
	ss << "<GopSizeAvg>" << gop_size_total << "</GopSizeAvg>\n";
	ss << "<MaxGopSize>" << max_gop_size << "</MaxGopSize>\n";
	ss << "<IsGopSizeAllLtThreshold>" ;
	if (b_gop_size_alarm) 
	{
		ss << "true";
	}
	else
	{
		ss << "false";
	}

	ss<< "</IsGopSizeAllLtThreshold>\n";


	return ss.str();
}



int ei_mediainfo(const char* pathname)
{
	int nSyncByte = 0;
	int nTsLength = GetTsLength(pathname,nSyncByte);
	if (nTsLength <= 0)
	{
		cerr << "GetTsLength error:" <<  pathname << endl;
		return -1;
	}




	FILE* fp = fopen(pathname,"rb");
	if (fp == NULL)
	{
		cerr << "open file faild:" << pathname << endl;
		return -1;
	}

	if (nSyncByte != 0)
	{
		fseek(fp,nSyncByte,SEEK_SET);
	}

	//easyice checking
	BYTE* buf = new BYTE[READ_BUF_SIZE];
	CMpegDec dec;
	dec.Init(188,1);
	long long total_size = 0;
	while(1)
	{
		int size = fread(buf,1,READ_BUF_SIZE,fp);
		total_size += size;
		if (size <= 0 || 
            (g_check_part && total_size > 50000000))//100M
		{
			break;
		}
		dec.ProcessBuffer(buf,size);
	}
	//cerr << "total_size:" << total_size << endl;

	string ei_mi = format_ei_info(&dec);

	//mediainfo check
	string mi_mi = CheckMediaInfo(pathname);

    struct stat filestat;
	if (lstat(pathname, &filestat) == 0) {
		struct tm *p;
		p=gmtime(&filestat.st_mtime);
		char s[100];
		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);  

		cout << "<FileCreatetime>" << s << "</FileCreatetime>\r\n";
	}
    cout << mi_mi << ei_mi;



	//release
	fclose(fp);
	delete [] buf;
	
    return 0;
}
