/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef CM3U8PARSE_H
#define CM3U8PARSE_H

#include <iostream>
#include "hls_define.h"
#include "hls_type.h"
#include <list>

using namespace std;

class CM3u8Parse
{
public:
        CM3u8Parse();
        ~CM3u8Parse();
        
        /**
         * @param [in/out] sys
         */
        //bool Parse(stream_sys_t& sys,hls_stream_t* init_hls,const string& m3u8_content);

        bool isHTTPLiveStreaming(const string& m3u8_content);
        bool isMeta(const string& m3u8_content);
        
        bool ParseMeta(stream_sys_t& sys,const string& m3u8_content);
        bool ParseStream(stream_sys_t& sys,hls_stream_t* init_hls,const string& m3u8_content);
private:
        
        
        int parse_AddSegment(hls_stream_t *hls, const int duration, const char *uri);
        
public:

        
public:
        //meta情况下解析结果保存在此处
        list<hls_stream_t> m_lstStreams;
        
        //非meta情况下解析结果保存在　?        //hls_stream_t* m_pHls;
        list<segment_t> m_lstSegments;
};

#endif // CM3U8PARSE_H
