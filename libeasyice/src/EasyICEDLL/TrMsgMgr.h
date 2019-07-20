/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <list>
#include "libtr101290.h"
#include "mylist.h"


#ifndef TRMSGMGR_H
#define TRMSGMGR_H


//最大存储消息数,按每秒100条的速度，1000000可支持3小时的事件。3小时后，旧数据被冲掉，其中包含任何的告警事件。
#define MAX_MSG_SAVE_CNT 1000000

namespace tr101290_mgr
{
typedef enum _ERROR_TYPE_T
{
	TR_LV1,
	TR_LV1_SYNC_LOST,
	TR_LV1_SYNC_BYTE_ERR,
	TR_LV1_PAT_ERR,
	TR_LV1_CC_ERR,
	TR_LV1_PMT_ERR,
	TR_LV1_PID_ERR,
	TR_LV2,
	TR_LV2_TS_ERR,
	TR_LV2_CRC_ERR,
	TR_LV2_PCR_REPET_ERR,
	TR_LV2_PCR_DISCON_ERR,
	TR_LV2_PCR_AC_ERR,
	TR_LV2_PTS_ERR,
	TR_LV2_CAT_ERR,
	TR_LV3,
	TR_LV3_NIT_ACT_ERR,
	TR_LV3_NIT_OTHER_ERR,
	TR_LV3_SI_REPET_ERR,
	TR_LV3_UNREFER_PID_ERR,
	TR_LV3_SDT_ACT_ERR,
	TR_LV3_SDT_OTHER_ERR,
	TR_LV3_EIT_ACT_ERR,
	TR_LV3_EIT_OTHER_ERR,
	TR_LV3_EIT_PF_ERR,
	TR_LV3_RST_ERR,
	TR_LV3_TDT_ERR
}ERROR_TYPE_T;


typedef struct _TR_MSG_T
{
	_TR_MSG_T()
	{
		nSiRepetitionCount = -1;
		nErrCount = -1;
	}
	ERROR_TYPE_T emErrType;
	int nErrCount;	// -1 表示此告警不需要显示（disabled或门限范围之外的）
	int nSiRepetitionCount; // -1 表示没有产生

}TR_MSG_T;


//PSI/SI 超时门限 （单位：ms）
typedef struct _PSI_TIMEOUT_VALUE_T
{
	//DVB STD default
	_PSI_TIMEOUT_VALUE_T()
	{
		pat		= 500;
		cat		= 500;
		pmt		= 500;
		tsdt		= 10000;
		nit_act	= 10000;
		nit_other	= 10000;
		sdt_act	= 2000;
		sdt_other	= 10000;
		bat		= 10000;
		rst		= -1;
		tdt		= 30000;
		tot		= 30000;
		dit		= -1;
		sit		= -1;
		eit_act_pf	= 2000;

		eit_other_pf	= 10000;
		eit_in8day		= 10000;
		eit_out8day	= 30000;
	}
	int pat;
	int cat;
	int pmt;
	int tsdt;
	int nit_act;
	int nit_other;
	int sdt_act;
	int sdt_other;
	int bat;
	int rst;
	int tdt;
	int tot;
	int dit;
	int sit;
	int eit_act_pf;
	int eit_other_pf;
	int eit_in8day;
	int eit_out8day;

}PSI_TIMEOUT_VALUE_T;
}

using namespace  tr101290_mgr;

class CTrMsgMgr
{

    public:
public:
	CTrMsgMgr(void);
	~CTrMsgMgr(void);

	//添加一个事件
	TR_MSG_T* AddMsg(const REPORT_PARAM_T& msg);

	//回放事件
	TR_MSG_T* PlayBackMsg(const REPORT_PARAM_T& msg);

	//设置超时门限
	void SetPsiTimeOut(PSI_TIMEOUT_VALUE_T ptv);

	//设置超时门限为 DVB 标准 (默认)
	void SetPsiTimeOutDVB();

	void ClearError();
	

private:
	TR_MSG_T* ProcessReport(const REPORT_PARAM_T& msg,bool bRepaly = false);

	//判断是否开启
	bool CheckEnabled(const REPORT_PARAM_T& msg);

	//判断是否超过阀值
	bool CheckFilter(const REPORT_PARAM_T& msg);

public:

	//存储所有事件
	MyList<REPORT_PARAM_T> m_vecMsg;
private:

	TR_MSG_T* m_pTrMsg;

	//门限
	PSI_TIMEOUT_VALUE_T m_PsiTimeOut;

public:
	//统计计数
	int m_nErrCnt_SyncLost;
	int m_nErrCnt_SyncByte;

	int m_nErrCnt_pat;
	int m_nErrCnt_pat_occ;
	int m_nErrCnt_pat_tid;
	int m_nErrCnt_pat_scf;

	int m_nErrCnt_cc;

	int m_nErrCnt_pmt;
	int m_nErrCnt_pmt_occ;
	int m_nErrCnt_pmt_scf;

	int m_nErrCnt_pid;
	int m_nErrCnt_ts;

	int m_nErrCnt_crc;
	int m_nErrCnt_crc_pat;
	int m_nErrCnt_crc_cat;
	int m_nErrCnt_crc_pmt;
	int m_nErrCnt_crc_nit;
	int m_nErrCnt_crc_sdt;
	int m_nErrCnt_crc_bat;
	int m_nErrCnt_crc_tot;
	int m_nErrCnt_crc_eit;

	int m_nErrCnt_pcr_repetition;
	int m_nErrCnt_pcr_discontinuity;
	int m_nErrCnt_pcr_ac;
	int m_nErrCnt_pts;

	int m_nErrCnt_cat;
	int m_nErrCnt_cat_tid;

	int m_nErrCnt_nit_act;
	int m_nErrCnt_nit_act_tid;
	int m_nErrCnt_nit_act_timeout;
	int m_nErrCnt_nit_act_lower25ms;

	int m_nErrCnt_nit_other;
	int m_nErrCnt_nit_other_timeout;
	int m_nErrCnt_nit_other_lower25ms;


	int m_nErrCnt_si_repetition;
	int m_nErrCnt_si_repetition_lower25ms;

	int m_nErrCnt_unrefer_pid;

	int m_nErrCnt_sdt_act;
	int m_nErrCnt_sdt_act_tid;
	int m_nErrCnt_sdt_act_timeout;
	int m_nErrCnt_sdt_act_lower25ms;

	int m_nErrCnt_sdt_other;
	int m_nErrCnt_sdt_other_timeout;
	int m_nErrCnt_sdt_other_lower25ms;

	int m_nErrCnt_eit_pf_act;
	int m_nErrCnt_eit_pf_act_tid;
	int m_nErrCnt_eit_pf_act_timeout;
	int m_nErrCnt_eit_pf_act_lower25ms;

	int m_nErrCnt_eit_pf_other;
	int m_nErrCnt_eit_pf_other_timeout;
	int m_nErrCnt_eit_pf_other_lower25ms;

	int m_nErrCnt_eit_pf;

	int m_nErrCnt_rst;
	int m_nErrCnt_rst_tid;
	int m_nErrCnt_rst_lower25ms;

	int m_nErrCnt_tdt;
	int m_nErrCnt_tdt_tid;
	int m_nErrCnt_tdt_timeout;
	int m_nErrCnt_tdt_lower25ms;

	int m_nErrCnt_bat_timeout;
	int m_nErrCnt_bat_lower25ms;

	int m_nErrCnt_eit_schedule_act_timeout;
	int m_nErrCnt_eit_schedule_act_lower25ms;

	int m_nErrCnt_eit_schedule_other_timeout;
	int m_nErrCnt_eit_schedule_other_lower25ms;

	int m_nErrCnt_tot_timeout;
	int m_nErrCnt_tot_lower25ms;
};



#endif


