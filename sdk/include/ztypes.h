/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ZTYPES_H
#define ZTYPES_H

/**
 * @file ztypes.h
 * @brief 提供通用类型定义的头文件。
 */
#include <ctype.h>
#include <vector>
#include <typeinfo>
#include <string.h>
#include <stdlib.h>
using namespace std;

/**
 * @typedef UINT
 * @brief 无符号32位整形
 */
typedef unsigned int 			UINT ;

/**
 * @typedef LONGLONG
 * @brief 有符号64位整形
 */
typedef long long		 		LONGLONG;
/**
 * @typedef ULONGLONG
 * @brief 有符号64位整形
 */
typedef unsigned long long 		ULONGLONG;
/**
 * @typedef BYTE
 * @brief 无符号整形，8 位，字节类型
 */
typedef unsigned char			BYTE;
/**
 * @typedef WORD
 * @brief 无符号16位整形，16 位
 */
typedef unsigned short 			WORD;
/**
 * @typedef DWORD
 * @brief 无符号32位整形
 */
typedef unsigned int				DWORD;


/**
 * @typedef VEC_HEX_T
 * @brief 定义二进制类型
 */
typedef vector<unsigned char>    VEC_HEX_T;

/**
 * @typedef SOCKET_T
 * @brief 定义SOCKET类型
 */
typedef int		SOCKET_T;
/**
 * @typedef EPOLL_T
 * @brief 定义EPOLL 句柄类型
 */
typedef int  	EPOLL_T;
#define ZNULL ((void *)0)

#endif

