/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef CSAVEFILE_H
#define CSAVEFILE_H
#include "hls_type.h"
#include "hls_define.h"
#include <deque>
#include <list>

//两个线程分别写ts和m3u8,注意线程安全
class CSaveFile
{
public:
        CSaveFile();
        ~CSaveFile();
        void SetParam(const HLS_RECORD_INIT_PARAM_T& param);
        void OnRecvM3u8(const string& m3u8);
        void OnRecvTs(const string& url,const BYTE* pData,int nLen);
        bool IsExist(const string& url);
        bool Init();
public:
        int m_segment_count;
        string m_m3u8name;
private:
        //void OnCheckTsExpire();
private:
        HLS_RECORD_INIT_PARAM_T m_stParam;

        string m_m3u8pathname;
        string m_path;//保存文件的绝对路径
        list<string> m_lstTsHistory;
        
        bool m_bFirstSave;
};

#endif // CSAVEFILE_H
