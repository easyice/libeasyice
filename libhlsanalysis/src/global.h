/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "hls_define.h"
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

namespace hls_analysis
{

extern struct timeval tvnow(void);
extern long long tvdiff(struct timeval newer, struct timeval older);
extern string get_uri(const string& url);

extern void Ei_MsgWaitForSingleObjects(const pthread_t *pHandles,int dwMilliseconds);

//输出不带换行符。除非输出与输入相同 
extern string removeparam(const string& filename);

extern void CreateMulPath( const char *muldir );

};



#endif

