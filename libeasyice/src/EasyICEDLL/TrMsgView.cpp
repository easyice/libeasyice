/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TrMsgView.cpp : 实现文件
//

#include "TrMsgView.h"
#include "TrMsgMgr.h"
#include "string_res.h"
#include <stdio.h>

//MSG窗口最大显示条数
#define MAX_MSG_CNT 5000



// CTrMsgView


CTrMsgView::CTrMsgView()
{
    m_pStrBufLen = 8192;
    m_pStrBuf = new char[m_pStrBufLen];
	m_vecErrTypeString.resize(LV3_DATA_DELAY_ERROR+1);
	InitErrorTypeString();
}

CTrMsgView::~CTrMsgView()
{
    delete [] m_pStrBuf;
}

//
//int CTrMsgView::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CListView::OnCreate(lpCreateStruct) == -1)
//		return -1;
//
//	CListCtrl& theCtrl = GetListCtrl();
//	m_pListCtrl = &theCtrl;
//	//SetRedraw(false);
//
//	//SetExtendedStyle(LVS_EX_FULLROWSELECT);	
//	//SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |LVS_EX_SUBITEMIMAGES);	//When an item is selected, the item and all its subitems are highlighted
//	m_pListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
//	ModifyStyle(0, LVS_SINGLESEL);
//
//	int i = 0;
//	m_pListCtrl->InsertColumn(i,_T("Index"),NULL,45);
//	m_pListCtrl->InsertColumn(++i,_T("Offset"),NULL,100);
//	m_pListCtrl->InsertColumn(++i,_T("Type"),NULL,150);
//	m_pListCtrl->InsertColumn(++i,_T("Message"),NULL,300);
//	m_nLastColumn = m_pListCtrl->InsertColumn(++i,_T("Occur Time"),NULL,150);
//	return 0;
//}
//

//
//switch (pItem->iSubItem)
//{
//    case 0:
//        text.Format(_T("%d"),itemid);
//        break;
//    case 1:
//        text.Format(_T("0x%08X"),msgInfo->param.llOffset);
//        break;
//    case 2:
//        text = GetErrorTypeString(msgInfo->nErrType);
//        break;
//    case 3:
//        text = GetMsgString(msgInfo->param);
//        break;
//    case 4:
//        text = _T("-");
//        break;
//    default:
//        break;
//}
Json::Value CTrMsgView::MsgListToJson()
{
    int i = 0;
    Json::Value msglist;
    char key[32];
    std::deque<CTrMsgView::MSG_LIST_T>::iterator it = m_dqLst.begin();
    for (; it != m_dqLst.end(); ++it)
    {
        Json::Value sub;
        sub["index"] = i++;
        sub["offset"] = it->param.llOffset;
        sub["type"] = GetErrorTypeString(it->nErrType);
        sub["message"] = GetMsgStringByReportParam(it->param);
        sprintf(key, "%05d", i);
        msglist[key] = sub;
    }

    return msglist;
}

string CTrMsgView::GetMsgStringByReportParam(const REPORT_PARAM_T& param)
{
	string str;
	switch (param.errName)
	{
	case LV1_PAT_ERROR_OCC:
		str = (IDS_TR_DESC_PAT_OCC);
		break;
	case LV1_PAT_ERROR_TID:
		str = (IDS_TR_DESC_PAT_TID);
		break;
	case LV1_PAT_ERROR_SCF:
		str = (IDS_TR_DESC_PAT_SCF);
		break;
	case LV1_CC_ERROR:
        sprintf(m_pStrBuf,"PID = %d",param.pid);
        str = m_pStrBuf;
		break;
	case LV1_PMT_ERROR_OCC:
		str = (IDS_TR_DESC_PMT_OCC);
		break;
	case LV1_PMT_ERROR_SCF:
		str = (IDS_TR_DESC_PMT_SCF);
		break;
	case LV1_PID_ERROR:
		sprintf(m_pStrBuf,(IDS_TR_DESC_PID),param.pid);
        str = m_pStrBuf;
		break;
	case LV2_TRANSPORT_ERROR:
        sprintf(m_pStrBuf,"PID = %d",param.pid);
        str = m_pStrBuf;
		break;
	case LV2_CRC_ERROR_PAT:
	case LV2_CRC_ERROR_CAT:
	case LV2_CRC_ERROR_PMT:
	case LV2_CRC_ERROR_NIT:
	case LV2_CRC_ERROR_SDT:
	case LV2_CRC_ERROR_BAT:
	case LV2_CRC_ERROR_TOT:
	case LV2_CRC_ERROR_EIT:
        sprintf(m_pStrBuf,"PID = %d",param.pid);
        str = m_pStrBuf;
		break;
	case LV2_PCR_REPETITION_ERROR:
		sprintf(m_pStrBuf,(IDS_TR_DESC_PCR_REPETITION),param.pid);
        str = m_pStrBuf;
		break;
	case LV2_PCR_ACCURACY_ERROR:
		sprintf(m_pStrBuf,IDS_TR_DESC_PCR_AC,param.pid);
        str = m_pStrBuf;
		break;
	case LV2_PTS_ERROR:
		sprintf(m_pStrBuf,(IDS_TR_DESC_PTS),param.pid);
        str = m_pStrBuf;
		break;
	case LV3_PSI_INTERVAL_PAT:
	case LV3_PSI_INTERVAL_PMT:
	case LV3_PSI_INTERVAL_CAT:
	case LV3_PSI_INTERVAL_TSDT:
	case LV3_PSI_INTERVAL_NIT_ACT:
	case LV3_PSI_INTERVAL_NIT_OTHER:
	case LV3_PSI_INTERVAL_SDT_ACT:
	case LV3_PSI_INTERVAL_SDT_OTHER:
	case LV3_PSI_INTERVAL_BAT:
	case LV3_PSI_INTERVAL_EIT_PF_ACT:
	case LV3_PSI_INTERVAL_EIT_PF_OTHER:
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_ACT:
	case LV3_PSI_INTERVAL_EIT_SCHEDULE_OTHER:
	case LV3_PSI_INTERVAL_TDT:
	case LV3_PSI_INTERVAL_RST:
	case LV3_PSI_INTERVAL_ST:
	case LV3_PSI_INTERVAL_TOT:
	case LV3_PSI_INTERVAL_DIT:
	case LV3_PSI_INTERVAL_SIT:
		sprintf(m_pStrBuf,(IDS_TR_DESC_INT),param.llVal,param.pid);
        str = m_pStrBuf;
		break;

	case LV3_NIT_ERROR_TID:
	case LV3_SDT_ERROR_TID:
	case LV3_EIT_ERROR_TID:
	case LV3_RST_ERROR_TID:
	case LV3_TDT_ERROR_TID:
	case LV3_PF_ERROR:
        sprintf(m_pStrBuf,"PID = %d",param.pid);
        str = m_pStrBuf;
		break;



	default:
		str = ("");
		break;
	}

	return str;
}

void CTrMsgView::AddMsg(const REPORT_PARAM_T& param,int nErrType)
{
	if (m_dqLst.size() >= MAX_MSG_CNT)
	{
		m_dqLst.pop_front();
	}

	MSG_LIST_T lst;
	lst.param = param;
	lst.nErrType = nErrType;


	m_dqLst.push_back(lst);
}



const std::deque<CTrMsgView::MSG_LIST_T>& CTrMsgView::GetMsgList()
{
	return  m_dqLst;
}

void CTrMsgView::ClearMsg()
{
	m_dqLst.clear();
}




const string& CTrMsgView::GetErrorTypeString(int nType)
{
	return m_vecErrTypeString[nType];
}

void CTrMsgView::InitErrorTypeString()
{
	m_vecErrTypeString[TR_LV1_SYNC_LOST]=(IDS_TR_LV1_SYNC_LOST);
	m_vecErrTypeString[TR_LV1_SYNC_BYTE_ERR]=(IDS_TR_LV1_SYNC_BYTE_ERR);
	m_vecErrTypeString[TR_LV1_PAT_ERR]=(IDS_TR_LV1_PAT_ERR);
	//m_vecErrTypeString[TR_LV1_PAT_ERR]=(IDS_TR_LV1_PAT_ERR_OCC);
	//m_vecErrTypeString[TR_LV1_PAT_ERR]=(IDS_TR_LV1_PAT_ERR_TID);
	//m_vecErrTypeString[TR_LV1_PAT_ERR]=(IDS_TR_LV1_PAT_ERR_SCF);
	m_vecErrTypeString[TR_LV1_CC_ERR]=(IDS_TR_LV1_CC_ERR);
	m_vecErrTypeString[TR_LV1_PMT_ERR]=(IDS_TR_LV1_PMT_ERR);
	////m_vecErrTypeString[TR_LV1_PMT_ERR]=(IDS_TR_LV1_PMT_ERR_OCC);
	////m_vecErrTypeString[TR_LV1_PMT_ERR]=(IDS_TR_LV1_PMT_ERR_SCF);
	m_vecErrTypeString[TR_LV1_PID_ERR]=(IDS_TR_LV1_PID_ERR);
	m_vecErrTypeString[TR_LV2_TS_ERR]=(IDS_TR_LV2_TS_ERR);
	m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_CAT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_PMT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_NIT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_SDT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_BAT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_TOT);
	//m_vecErrTypeString[TR_LV2_CRC_ERR]=(IDS_TR_LV2_CRC_ERR_EIT);

	m_vecErrTypeString[TR_LV2_PCR_REPET_ERR]=(IDS_TR_LV2_PCR_REPET_ERR);
	m_vecErrTypeString[TR_LV2_PCR_DISCON_ERR]=(IDS_TR_LV2_PCR_DISCON_ERR);
	m_vecErrTypeString[TR_LV2_PCR_AC_ERR]=(IDS_TR_LV2_PCR_AC_ERR);
	m_vecErrTypeString[TR_LV2_PTS_ERR]=(IDS_TR_LV2_PTS_ERR);
	//m_vecErrTypeString[TR_LV2_CAT_ERR]=(IDS_TR_LV2_CAT_ERR_TID);
	m_vecErrTypeString[TR_LV2_CAT_ERR]=(IDS_TR_LV2_CAT_ERR);

	m_vecErrTypeString[TR_LV3_NIT_ACT_ERR]=(IDS_TR_LV3_NIT_ACT_ERR);
	//m_vecErrTypeString[TR_LV3_NIT_ACT_ERR]=(IDS_TR_LV3_NIT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_NIT_ACT_ERR]=(IDS_TR_LV3_NIT_ACT_ERR_INT);
	m_vecErrTypeString[TR_LV3_NIT_OTHER_ERR]=(IDS_TR_LV3_NIT_OTHER_ERR);
	m_vecErrTypeString[TR_LV3_UNREFER_PID_ERR]=(IDS_TR_LV3_UNREFER_PID_ERR);

	m_vecErrTypeString[TR_LV3_SDT_ACT_ERR]=(IDS_TR_LV3_SDT_ACT_ERR);
	//m_vecErrTypeString[TR_LV3_SDT_ACT_ERR]=(IDS_TR_LV3_SDT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_SDT_ACT_ERR]=(IDS_TR_LV3_SDT_ACT_ERR_INT);
	m_vecErrTypeString[TR_LV3_SDT_OTHER_ERR]=(IDS_TR_LV3_SDT_OTHER_ERR);

	m_vecErrTypeString[TR_LV3_EIT_ACT_ERR]=(IDS_TR_LV3_EIT_ACT_ERR);
	//m_vecErrTypeString[TR_LV3_EIT_ACT_ERR]=(IDS_TR_LV3_EIT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_EIT_ACT_ERR]=(IDS_TR_LV3_EIT_ACT_ERR_INT);
	m_vecErrTypeString[TR_LV3_EIT_OTHER_ERR]=(IDS_TR_LV3_EIT_OTHER_ERR);
	m_vecErrTypeString[TR_LV3_EIT_PF_ERR]=(IDS_TR_LV3_EIT_PF_ERR);

	m_vecErrTypeString[TR_LV3_RST_ERR]=(IDS_TR_LV3_RST_ERR);
	//m_vecErrTypeString[TR_LV3_RST_ERR]=(IDS_TR_LV3_RST_ERR_TID);
	//m_vecErrTypeString[TR_LV3_RST_ERR]=(IDS_TR_LV3_RST_ERR_INT);

	m_vecErrTypeString[TR_LV3_TDT_ERR]=(IDS_TR_LV3_TDT_ERR);
	//m_vecErrTypeString[TR_LV3_TDT_ERR]=(IDS_TR_LV3_TDT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_TDT_ERR]=(IDS_TR_LV3_TDT_ERR_INT);

	m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_SI_REPET_ERR);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_BAT);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_EIT_SCHEDULE_ACT);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_EIT_SCHEDULE_OTHER);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_TOT);

///另一种填充方式

	//m_vecErrTypeString[LV1_TS_SYNC_LOST]=(IDS_TR_LV1_SYNC_LOST);
	//m_vecErrTypeString[LV1_SYNC_BYTE_ERROR]=(IDS_TR_LV1_SYNC_BYTE_ERR);
	//m_vecErrTypeString[LV1_PAT_ERROR_OCC]=(IDS_TR_LV1_PAT_ERR_OCC);
	//m_vecErrTypeString[LV1_PAT_ERROR_TID]=(IDS_TR_LV1_PAT_ERR_TID);
	//m_vecErrTypeString[LV1_PAT_ERROR_SCF]=(IDS_TR_LV1_PAT_ERR_SCF);
	//m_vecErrTypeString[LV1_CC_ERROR]=(IDS_TR_LV1_CC_ERR);

	//m_vecErrTypeString[LV1_PMT_ERROR_OCC]=(IDS_TR_LV1_PMT_ERR_OCC);
	//m_vecErrTypeString[LV1_PMT_ERROR_SCF]=(IDS_TR_LV1_PMT_ERR_SCF);
	//m_vecErrTypeString[LV1_PID_ERROR]=(IDS_TR_LV1_PID_ERR);
	//m_vecErrTypeString[LV2_TRANSPORT_ERROR]=(IDS_TR_LV2_TS_ERR);

	//m_vecErrTypeString[LV2_CRC_ERROR_PAT]=(IDS_TR_LV2_CRC_ERR_PAT);
	//m_vecErrTypeString[LV2_CRC_ERROR_CAT]=(IDS_TR_LV2_CRC_ERR_CAT);
	//m_vecErrTypeString[LV2_CRC_ERROR_PMT]=(IDS_TR_LV2_CRC_ERR_PMT);
	//m_vecErrTypeString[LV2_CRC_ERROR_NIT]=(IDS_TR_LV2_CRC_ERR_NIT);
	//m_vecErrTypeString[LV2_CRC_ERROR_SDT]=(IDS_TR_LV2_CRC_ERR_SDT);
	//m_vecErrTypeString[LV2_CRC_ERROR_BAT]=(IDS_TR_LV2_CRC_ERR_BAT);
	//m_vecErrTypeString[LV2_CRC_ERROR_TOT]=(IDS_TR_LV2_CRC_ERR_TOT);
	//m_vecErrTypeString[LV2_CRC_ERROR_EIT]=(IDS_TR_LV2_CRC_ERR_EIT);

	////下面的还没改
	//m_vecErrTypeString[TR_LV2_PCR_REPET_ERR]=(IDS_TR_LV2_PCR_REPET_ERR);
	//m_vecErrTypeString[TR_LV2_PCR_DISCON_ERR]=(IDS_TR_LV2_PCR_DISCON_ERR);
	//m_vecErrTypeString[TR_LV2_PCR_AC_ERR]=(IDS_TR_LV2_PCR_AC_ERR);
	//m_vecErrTypeString[TR_LV2_PTS_ERR]=(IDS_TR_LV2_PTS_ERR);
	//m_vecErrTypeString[TR_LV2_CAT_ERR]=(IDS_TR_LV2_CAT_ERR_TID);

	//m_vecErrTypeString[TR_LV3_NIT_ACT_ERR]=(IDS_TR_LV3_NIT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_NIT_ACT_ERR]=(IDS_TR_LV3_NIT_ACT_ERR_INT);
	//m_vecErrTypeString[TR_LV3_NIT_OTHER_ERR]=(IDS_TR_LV3_NIT_OTHER_ERR);
	//m_vecErrTypeString[TR_LV3_UNREFER_PID_ERR]=(IDS_TR_LV3_UNREFER_PID_ERR);
	//m_vecErrTypeString[TR_LV3_SDT_ACT_ERR]=(IDS_TR_LV3_SDT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_SDT_ACT_ERR]=(IDS_TR_LV3_SDT_ACT_ERR_INT);
	//m_vecErrTypeString[TR_LV3_SDT_OTHER_ERR]=(IDS_TR_LV3_SDT_OTHER_ERR);
	//m_vecErrTypeString[TR_LV3_EIT_ACT_ERR]=(IDS_TR_LV3_EIT_ACT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_EIT_ACT_ERR]=(IDS_TR_LV3_EIT_ACT_ERR_INT);
	//m_vecErrTypeString[TR_LV3_EIT_OTHER_ERR]=(IDS_TR_LV3_EIT_OTHER_ERR);
	//m_vecErrTypeString[TR_LV3_EIT_PF_ERR]=(IDS_TR_LV3_EIT_PF_ERR);
	//m_vecErrTypeString[TR_LV3_RST_ERR]=(IDS_TR_LV3_RST_ERR_TID);
	//m_vecErrTypeString[TR_LV3_RST_ERR]=(IDS_TR_LV3_RST_ERR_INT);
	//m_vecErrTypeString[TR_LV3_TDT_ERR]=(IDS_TR_LV3_TDT_ERR_TID);
	//m_vecErrTypeString[TR_LV3_TDT_ERR]=(IDS_TR_LV3_TDT_ERR_INT);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_BAT);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_EIT_SCHEDULE_ACT);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_EIT_SCHEDULE_OTHER);
	//m_vecErrTypeString[TR_LV3_SI_REPET_ERR]=(IDS_TR_LV3_INT_TOT);
}

