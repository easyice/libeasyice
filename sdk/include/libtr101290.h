/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LIBTR101290_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LIBTR101290_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifndef LIBTR101290_H
#define LIBTR101290_H

#include "tr101290_defs.h"



//#ifdef LIBTR101290_EXPORTS
//#define LIBTR101290_API __declspec(dllexport)
//#else
//#define LIBTR101290_API __declspec(dllimport)
//#endif


/*
 report 说明

 ------------------------------------------------
 LV1_PID_ERROR
 pid

 ------------------------------------------------
 LV2_PCR_REPETITION_ERROR
 llVal 的任意值都有意义，单位为27m系统时钟
 (int)fVal 代表discontinuity_indicator

 ------------------------------------------------
 LV2_PCR_ACCURACY_ERROR
 llVal 的任意值都有意义，单位为27m系统时钟

 ------------------------------------------------
 LV2_CRC_ERROR


 ------------------------------------------------
 LV3_PSI_INTERVAL_XXX
 llVal  有意义,单位 ms


 ------------------------------------------------
 LV3_NIT_ACTUAL_ERROR_TID
 pid

 ------------------------------------------------
 LV3_PF_ERROR
 pid

 ------------------------------------------------
 LV3_UNREFERENCED_PID
 pid


------------------------------------------------
LV1_PAT_ERROR_OCC
LV1_PMT_ERROR_OCC
llVal()








 ------------------------------------------------
 其他情况下，llVal与fVal 等于 -1 表示没有意义




*/


class CTrCore;


// 此类是从 libtr101290.dll 导出的

//每个文件对应一个新的类对象,关闭文件时delete，打开时新申请一个
class  Clibtr101290 {
public:
	Clibtr101290(void);
	~Clibtr101290(void);

	//PAT，PMT是否解析完毕
	bool IsDemuxFinish();

	void SetReportCB(pfReportCB pCB,void* pApp);

	void SetStartOffset(long long llOffset);

	void SetTsLen(int nLen);

	//the size must eq to LV3_DATA_DELAY_ERROR+1  默认全部开启
	void SetEnable(bool *p);

	//最后一个被调用的函数
	void AddPacket(BYTE* pPacket);
private:
	CTrCore* m_pTrCore;
};



extern  int nlibtr101290;




 int fnlibtr101290(void);



#endif

