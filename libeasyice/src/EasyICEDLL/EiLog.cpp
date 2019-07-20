/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "EiLog.h"
#include <iostream>
#include "../sdkdefs.h"

#include <stdio.h>

using namespace std;

// 类的静态成员变量要在类体外进行定义
CEiLog* CEiLog::m_pStatic = NULL;



static void logfun(int level,const char* module,const char* sInfo,void *pApp)
{
	switch(level)
	{
		case LV_DEBUG:
			printf("debug: %s\n",sInfo);
			break;
		case LV_WARNING:
			printf("warning: %s\n",sInfo);
			break;
		case LV_ERROR:
			printf("error: %s\n",sInfo);
			break;
		default:
			printf("%s\n",sInfo);
			break;
	}
}

//需要对外提供log接口时将此全局变量设置为别的函数指针
static pfLogFun g_varpfLogFun = logfun;
static void *g_pApp = NULL;

#define EI_LOG(level,module,sInfo) \
    if (g_varpfLogFun!=NULL) \
{\
	g_varpfLogFun(level,module,sInfo,g_pApp);\
}


CEiLog::CEiLog(void)
{
	INIT_MUTEX(&m_muxLog);
	m_pLogBuffer = new char[LOG_BUFFER_SIZE];
}

CEiLog::~CEiLog(void)
{
	DESTROY_MUTEX(&m_muxLog);
	delete [] m_pLogBuffer;
}

CEiLog* CEiLog::GetInstancePtr()
{
	if (NULL == m_pStatic)
	{
		m_pStatic = new CEiLog();
	}

	return m_pStatic;
}

void CEiLog::Destroy()
{
	delete m_pStatic;
	m_pStatic = NULL;
}
void CEiLog::ei_vlog(int level,const char *module,  const char *fmt, va_list vl)
{
    THREAD_MUTEX_LOCK(&m_muxLog);
    //char line[1024]; 
    //line[0]=0;
    //vsnprintf(line + strlen(line), sizeof(line) - strlen(line), fmt, vl);

	vsnprintf(m_pLogBuffer, LOG_BUFFER_SIZE, fmt, vl);

	EI_LOG(level,module,m_pLogBuffer);
   // sanitize(line);
    //colored_fputs(av_clip(level>>3, 0, 6), line);
    THREAD_MUTEX_UNLOCK(&m_muxLog);
}
//
//void CMuxLog::Log(int level, const char *fmt, ...)
//{
//    va_list vl;
//    va_start(vl, fmt);
//    
//    mux_vlog(level, fmt, vl);
//    va_end(vl);
//}


void ei_log(int level,const char *module,const char *fmt, ...)
{
	 va_list vl;
    va_start(vl, fmt);
    
    CEiLog::GetInstancePtr()->ei_vlog(level,module, fmt, vl);
    va_end(vl);
}

void easyice_global_setlogfuncallback(pfLogFun pFun,void *pApp)
{
	g_varpfLogFun = pFun;
	g_pApp = pApp;
}

