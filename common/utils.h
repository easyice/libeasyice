/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//公用函数定义

#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>

#include <vector>
#include <iostream>
#include <stdio.h>

using namespace std;



extern char*strlwr(char *s);


static void splitstring(const string &SrcString,const string &SplitChars,vector <string >&VecOurString)
{
    string s;
    string::size_type last_pos = 0;
    string::size_type sz,i;
    sz = SrcString.size();
    VecOurString.clear();
    //匹配双引号，暂时不处理
//     bool b_match = false;
    for(i = 0;i<sz;i++)
    {
        if(SplitChars.find(SrcString.at(i)) != string::npos)
        {
            //发现分割符，并且字符串不为NULL
            if((i - last_pos) >0)
            {
                //排除分割符，获取字符串
                s = SrcString.substr(last_pos,i - last_pos);
                VecOurString.push_back(s);
            }
            last_pos = i + 1;
        }
    }

    //最后一段数据添加到分割队列
    if((i - last_pos) >0)
    {
        //排除分割符，获取字符串
        s = SrcString.substr(last_pos,i - last_pos);
        VecOurString.push_back(s);
    }

}

static void utils_ntorn(const string &SrcString,string &OurString)
{
	vector<string> out;
	splitstring(SrcString,"\n",out);
	vector<string>::iterator it = out.begin();
	for(; it != out.end();++it)
	{
		OurString += *it;
		OurString += "\r\n";
	}
}


static string byte2string(int bytes)
{
	char buf[256] = "";
	if (bytes < 1024)
	{
		sprintf(buf,"%dB",bytes);
	}
	else if (bytes >= 1024 && bytes < 1048576)
	{
		sprintf(buf,"%.2fK",(double)bytes/1024);
	}
	else if (bytes >= 1048576 && bytes < 1073741824LL)
	{
		sprintf(buf,"%.2fM",(double)bytes/1048576);
	}
	else
	{
		sprintf(buf,"%.2fG",(double)bytes/1073741824LL);
	}

	return buf;
}

static string bit2string(int bits)
{
	char buf[256] = "";
	if (bits < 1000)
	{
		sprintf(buf,"%d",bits);
	}
	else if (bits >= 1000 && bits < 1000000)
	{
		sprintf(buf,"%.2fKb",(double)bits/1000);
	}
	else if (bits >= 1000000 && bits < 1000000000LL)
	{
		sprintf(buf,"%.2fMb",(double)bits/1000000);
	}
	else
	{
		sprintf(buf,"%.2fGb",(double)bits/1000000000LL);
	}

	return buf;
}




//int TypeOfStreamType(int stream_type)
//{
//	if (stream_type == 0x01 || stream_type == 0x02 || stream_type == 0x1B || stream_type == 0x24)	//video
//	{
//		return 1;
//	}
//	else if (stream_type == 0x03 || stream_type == 0x04 || stream_type == 0x0F 
//		||stream_type == 0x80 || stream_type == 0x81 || stream_type == 0x82
//		||stream_type == 0x83 || stream_type == 0x84 || stream_type == 0x85
//		||stream_type == 0x86 || stream_type == 0x8A|| stream_type == 0xA1 || stream_type == 0xA2) //audio
//	{
//		return 2;
//	}
//
//	return 0;
//}

#endif
