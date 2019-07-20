/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TR101290_DEFINE_H
#define TR101290_DEFINE_H
#include <stdint.h>






typedef unsigned char BYTE;
//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int  uint32_t;
//typedef unsigned long long uint64_t;


typedef enum _ERROR_NAME_T
{
	LV1_TS_SYNC_LOST,
	LV1_SYNC_BYTE_ERROR,
	LV1_PAT_ERROR_OCC,
	LV1_PAT_ERROR_TID,
	LV1_PAT_ERROR_SCF,
	LV1_CC_ERROR,
	LV1_PMT_ERROR_OCC,
	LV1_PMT_ERROR_SCF,
	LV1_PID_ERROR,

	LV2_TRANSPORT_ERROR,
	
	//CRC ERROR
	LV2_CRC_ERROR_PAT,
	LV2_CRC_ERROR_CAT,
	LV2_CRC_ERROR_PMT,
	LV2_CRC_ERROR_NIT,
	LV2_CRC_ERROR_SDT,
	LV2_CRC_ERROR_BAT,
	LV2_CRC_ERROR_TOT,
	LV2_CRC_ERROR_EIT,
	LV2_CRC_ERROR_OTHER,
	

	LV2_PCR_REPETITION_ERROR,
	LV2_PCR_ACCURACY_ERROR,
	LV2_PTS_ERROR,
	LV2_CAT_ERROR_TID,

	//PSI/SI 间隔
	LV3_PSI_INTERVAL_PAT,
	LV3_PSI_INTERVAL_PMT,
	LV3_PSI_INTERVAL_CAT,
	LV3_PSI_INTERVAL_TSDT,
	LV3_PSI_INTERVAL_NIT_ACT,
	LV3_PSI_INTERVAL_NIT_OTHER,
	LV3_PSI_INTERVAL_SDT_ACT,
	LV3_PSI_INTERVAL_SDT_OTHER,
	LV3_PSI_INTERVAL_BAT,
	LV3_PSI_INTERVAL_EIT_PF_ACT,
	LV3_PSI_INTERVAL_EIT_PF_OTHER,
	LV3_PSI_INTERVAL_EIT_SCHEDULE_ACT,
	LV3_PSI_INTERVAL_EIT_SCHEDULE_OTHER,
	LV3_PSI_INTERVAL_TDT,
	LV3_PSI_INTERVAL_RST,
	LV3_PSI_INTERVAL_ST,
	LV3_PSI_INTERVAL_TOT,
	LV3_PSI_INTERVAL_DIT,
	LV3_PSI_INTERVAL_SIT,
	LV3_PSI_INTERVAL_OTHER,


	LV3_NIT_ERROR_TID,
	LV3_SDT_ERROR_TID,
	LV3_EIT_ERROR_TID,
	LV3_RST_ERROR_TID,
	LV3_TDT_ERROR_TID,


	LV3_PF_ERROR,
	LV3_BUFFER_ERROR,
	LV3_UNREFERENCED_PID,

	

	
	
	
	LV3_EMPTY_BUFFER_ERROR,
	LV3_DATA_DELAY_ERROR

}ERROR_NAME_T;


typedef struct _REPORT_PARAM_T
{
	_REPORT_PARAM_T()
	{
		pid = -1;
		llVal = -1;
		fVal = -1;
	}

	//必填
	int level;
	ERROR_NAME_T errName;
	long long llOffset;
	void* pApp;

	//可选
	int pid;

	//可选1个
	long long llVal;		
	double fVal;
	
}REPORT_PARAM_T;

typedef void (*pfReportCB)(REPORT_PARAM_T param);

#define CALL_PASSWD 0x1b







#endif

