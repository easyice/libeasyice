/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#ifndef EILOG_H
#define EILOG_H

#include <stdarg.h>

/****************************************************************************************************************
 **************************************************** 跨平台的多线程相关定义**************************************
******************************************************************************************************************/
#ifdef _WIN32
#include <process.h>
	#define THREAD_MUTEX_T	CRITICAL_SECTION
	#define LPTHREAD_MUTEX_T	LPCRITICAL_SECTION

	#define INIT_MUTEX(LPTHREAD_MUTEX_T) \
	InitializeCriticalSection(LPTHREAD_MUTEX_T);

	#define DESTROY_MUTEX(LPTHREAD_MUTEX_T) \
	DeleteCriticalSection(LPTHREAD_MUTEX_T);

	#define THREAD_MUTEX_LOCK(LPTHREAD_MUTEX_T) \
	EnterCriticalSection(LPTHREAD_MUTEX_T);

	#define THREAD_MUTEX_UNLOCK(LPTHREAD_MUTEX_T) \
	LeaveCriticalSection(LPTHREAD_MUTEX_T);
#else
#include <pthread.h>
	#define THREAD_MUTEX_T	pthread_mutex_t
	#define LPTHREAD_MUTEX_T	pthread_mutex_t*

	#define INIT_MUTEX(LPTHREAD_MUTEX_T) \
	pthread_mutex_init(LPTHREAD_MUTEX_T,NULL);

	#define DESTROY_MUTEX(LPTHREAD_MUTEX_T) \
	pthread_mutex_destroy(LPTHREAD_MUTEX_T);

	#define THREAD_MUTEX_LOCK(LPTHREAD_MUTEX_T) \
	pthread_mutex_lock(LPTHREAD_MUTEX_T);

	#define THREAD_MUTEX_UNLOCK(LPTHREAD_MUTEX_T) \
	pthread_mutex_unlock(LPTHREAD_MUTEX_T);
#endif 




/**************************define logs *********************************************/

#define LV_DEBUG    0
#define LV_INFO     1
#define LV_WARNING  2
#define LV_ERROR    3


#define LOG_BUFFER_SIZE (1024*1024)

extern void ei_log(int level,const char *module, const char *fmt, ...);
typedef void (*pfLogFun)(int level,const char* module,const char* sInfo,void *pApp);

//static void LogCb(int level,const char* module,const char* sInfo,void *pApp);
extern void easyice_global_setlogfuncallback(pfLogFun pFun,void *pApp);





class CEiLog
{
public:
	
	~CEiLog(void);
	static CEiLog* GetInstancePtr();
	static void Destroy();

	//void Log(int level, const char *fmt, ...);
	void ei_vlog(int level,const char *module, const char *fmt, va_list vl);
private:
	
private:
	CEiLog(void);

	// 静态成员变量,提供全局惟一的一个实例
	static CEiLog* m_pStatic;

	THREAD_MUTEX_T m_muxLog;

	char* m_pLogBuffer;


};






#endif

