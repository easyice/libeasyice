/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "libtr101290.h"
#include <iostream>
#include <deque>
#include "TrMsgMgr.h"
#include "TrMsgView.h"

using namespace std;
using namespace tr101290_mgr;

class CTrMsgMgr;
class CTrMsgView;

// CTrView 视图

class CTrView 
{

public:
	CTrView();           // 动态创建所使用的受保护的构造函数
	~CTrView();
	static void OnTrReport(REPORT_PARAM_T param);

    string ToJson();
    //读完之后清空
    string ReadResultToJson();
    string GetErrorBriefByErrorType(ERROR_TYPE_T emErrType);
    const std::deque<CTrMsgView::MSG_LIST_T>& GetErrorMsgListByErrorType(ERROR_TYPE_T emErrType);
    const std::deque<CTrMsgView::MSG_LIST_T>& GetErrorMsgListAll(ERROR_TYPE_T emErrType);
private:
    void InitResString();
	string ShowBrief(int nItem);
	void ClearError();
private:

private:
	CTrMsgMgr* m_pTrMsgMgr;
    CTrMsgView*  m_pMsgView;

	int m_nLastColumn; //最后一列
    char* m_pBriefBuf;
    int m_pBriefBufLen;

    //错误列描述
	vector<string> m_vecResStr;

    //对外输出的错误技术记录在这个数组中
	int* m_pErrCnt;

	int m_nItem_SyncLost;
	int m_nItem_SyncByte;
	int m_nItem_pat;
	int m_nItem_cc;
	int m_nItem_pmt;
	int m_nItem_pid;

	int m_nItem_ts;
	int m_nItem_crc;
	int m_nItem_pcr_repetition;
	int m_nItem_pcr_discontinuity;
	int m_nItem_pcr_ac;
	int m_nItem_pts;
	int m_nItem_cat;

	int m_nItem_nit_act;
	int m_nItem_nit_other;
	int m_nItem_si_repetition;
	int m_nItem_unrefer_pid;
	int m_nItem_sdt_act;
	int m_nItem_sdt_other;
	int m_nItem_eit_pf_act;
	int m_nItem_eit_pf_other;
	int m_nItem_eit_pf;
	int m_nItem_rst;
	int m_nItem_tdt;

	//最后一次更新视图时间，仅在实时模式下使用
	long long m_llLastUpdateTime;
private:


};


