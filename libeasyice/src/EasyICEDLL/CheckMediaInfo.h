/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CHECKMEDIAINFO_H
#define CHECKMEDIAINFO_H

#ifdef MEDIAINFO_LIBRARY
    #include "MediaInfo/MediaInfo.h" //Staticly-loaded library (.lib or .a or .so)
    #define MediaInfoNameSpace MediaInfoLib;
#else //MEDIAINFO_LIBRARY
    #include "MediaInfoDLL/MediaInfoDLL.h" //Dynamicly-loaded library (.dll or .so)
    #define MediaInfoNameSpace MediaInfoDLL;
#endif //MEDIAINFO_LIBRARY
#include <iostream>
#include <iomanip>


using namespace MediaInfoNameSpace;


//#include "..\MediaInfo\inc\MediaInfoDLL.h"
#include <ztypes.h>
#include <vector>
#include <string>

typedef struct _EI_AUDIOINFO_T
{
	int id;
	MediaInfoDLL::String format;
}EI_AUDIOINFO_T;

typedef struct _EI_VIDEOINFO_T
{
	int id;
	MediaInfoDLL::String format;
}EI_VIDEOINFO_T;

typedef struct _EI_MEDIAINFO_T
{
	MediaInfoDLL::String mi;
	std::vector<EI_VIDEOINFO_T> vecVStream;
	std::vector<EI_AUDIOINFO_T> vecAStream;
}EI_MEDIAINFO_T;


/*
MediaInfo库有奇怪的问题，当TaskCenter与 LiveAnalysis同时引用是，启动软件会崩溃，根本无法启动，
测试发现与定义 MediaInfo 对象有关，但将其作为全局变量时，谁引用都会报错。
只有一个人按普通调用方式没问题，要两个类都引用，有两种解决办法:
1.将实现代码放到同一个cpp中（同一个.h是不可以的），这是当前解决方案
2.一个在堆上定义，一个在栈上定义

*/

class CEiMediaInfo
{
    public:
    CEiMediaInfo();
    ~CEiMediaInfo();
    void CheckMediaInfo(BYTE* pData,int length,int nTsLength,EI_MEDIAINFO_T* pEmi);

    MediaInfoDLL::String CheckMediaInfo(BYTE* pData,int length,int nTsLength);

    MediaInfoDLL::String CheckMediaInfo(const string& PathName);
    string ffprobe_all(const string& PathName);
    
    private:
    char* m_pBuffer;
    int m_nBufferLen;
};
#endif

