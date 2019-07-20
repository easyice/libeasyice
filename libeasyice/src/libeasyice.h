/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef LIB_EASYICE_H
#define LIB_EASYICE_H

/**
 *@brief 本头文件为纯 C 封装的接口，结构体序列化为 json 返回，调用方需要反序列化。
 文件分析：
 1.easyice_process函数会阻塞运行，直到分析完毕为止,随后调用方可以easyice_cleanup释放。

 UDP 直播分析
 1.easyice_process函数会异步运行，调用方想要停止分析时，调用easyice_cleanup会终止分析并释放资源
 * */




#include <stdio.h>
#include "sdkdefs.h"

extern void easyice_global_init();
extern void easyice_global_cleanup();

extern EASYICE* easyice_init();
extern void easyice_cleanup(EASYICE* handle);
//extern int easyice_setopt( EASYICE* handle,EASYICEopt option, ...);
extern int easyice_setopt( EASYICE* handle,EASYICEopt option, void* val);

extern int easyice_getinfo(EASYICE* handle,EASYICEinfo info,void* val);
extern EASYICEcode easyice_process(EASYICE* handle);







#endif


