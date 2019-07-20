/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ZEVENT_H
#define ZEVENT_H

#include <pthread.h>

#include <string>
using namespace std;



/**
 * 多线程事件处理
 * 使用条件变量实现
 */
class ZEvent
{
    public:
        ZEvent(const string Name,bool bInitSet = false,bool bWaitAutoReset = true);
        ~ZEvent();
        
        void Set();
        void Reset();
        bool Wait(int Timeout_ms = 0);

    protected:
        


        pthread_mutex_t m_hMutex;
        pthread_cond_t m_hCond;
        bool m_bSeted;
        bool m_bWaitAutoReset;
        int m_Counter_Set;
        int m_Counter_Reset;
        int m_Counter_Wait;
        string m_Name;

};

#endif
