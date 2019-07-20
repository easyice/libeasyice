/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// TrView.cpp : 实现文件
//

#include "TrView.h"
#include "string_res.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "json/json.h"

const int ICON_NULL = 0;
const int ICON_OK = 1;
const int ICON_ERROR = 2;
const int ICON_DISABLE = 3;

#define MAX_ITEM_CNT 50

//实时模式下，视图更新间隔时间（秒）
#define UPDATE_INTERVAL 1000000


/*
1. disable 的项的日志，不应该抛上来
2. 实时分析情况下，当分析未完成时，当点击某项，此项之前的日志不应显示，只显示点击动作之后的
3. 离线分析，TR不应实时刷新
4. 离线分析 ，时间项  不应填充
5. 开启或禁用某项，应在开始分析之前设置
6. 分析完毕之后再开启或禁用，可进行过滤显示

存在三种开关：1.使能开关，2，过滤器（单个/全部），3，阀值重设
*/

// CTrView


CTrView::CTrView()
{
	m_pTrMsgMgr = new CTrMsgMgr();
    m_pMsgView = new CTrMsgView();

    m_pBriefBufLen = 8192;
    m_pBriefBuf = new char [m_pBriefBufLen];
    memset(m_pBriefBuf,0,m_pBriefBufLen);

	m_llLastUpdateTime = -1;

	m_pErrCnt = new int[MAX_ITEM_CNT];
	for (int i = 0; i < MAX_ITEM_CNT; i++)
	{
		m_pErrCnt[i] = 0;
	}
	InitResString();
}

CTrView::~CTrView()
{
	delete m_pTrMsgMgr;
    delete m_pMsgView;
	delete [] m_pErrCnt;
    delete [] m_pBriefBuf;
}

// CTrView 消息处理程序
//
//int CTrView::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	CListCtrl& theCtrl = GetListCtrl();
//	m_pListCtrl = &theCtrl;
//	int i = 0;
//	m_pListCtrl->InsertColumn(i,_T(""),LVCFMT_RIGHT,30);
//	m_pListCtrl->InsertColumn(++i,_T("Indicator"),NULL,180);
//	m_pListCtrl->InsertColumn(++i,_T("Errors"),NULL,60);
//	m_pListCtrl->InsertColumn(++i,_T("Time of Last Error"),NULL,150);
//	m_nLastColumn = m_pListCtrl->InsertColumn(++i,_T("Enabled"),NULL,55);
//	
//	
//	InitItem();
//
//	return 0;
//}
//

//
//switch (pItem->iSubItem)
//{
//    case 0:
//        if (itemid == TR_LV1 || itemid == TR_LV2 || itemid == TR_LV3)
//        {
//            //pItem->iImage = ICON_NULL;
//        }
//        else if (m_pErrCnt[itemid] > 0)
//        {
//            pItem->iImage = ICON_ERROR;
//        }
//        else
//        {
//            pItem->iImage = ICON_OK;
//        }
//        break;
//    case 1:
//        text = LOCALIZED_STRING(m_pResStr[itemid]);
//        break;
//    case 2:
//        text.Format(_T("%d"),m_pErrCnt[itemid]);
//        break;
//    case 3:
//        text = _T("-");
//        break;
//    case 4:
//        text = _T("Yes");
//        break;
//    default:
//        break;
//}
//
//if (pItem->iSubItem != 0)
//{
//    lstrcpyn(pItem->pszText, text, pItem->cchTextMax);
//}
//

string CTrView::ToJson()
{
    Json::Value root;
    Json::Value res;

    char key[32];
    //overview
    for (int i = 0; i < m_nLastColumn; i++)
    {
        string indicator = m_vecResStr[i];
        int errors = m_pErrCnt[i];

        Json::Value sub;
        sprintf(key, "%02d", i);
        sub["indicator"] = indicator;
        sub["errors"] = errors;
        sub["brief"] = GetErrorBriefByErrorType((tr101290_mgr::ERROR_TYPE_T)i);
        res[key] = sub;
    }
    root["overview"] = res;

    //msglist
    root["msglist"] = m_pMsgView->MsgListToJson();
    return root.toStyledString();
}

string CTrView::ReadResultToJson()
{
    string str = ToJson();
    m_pMsgView->ClearMsg();
    return str;
}

void CTrView::InitResString()
{
    m_vecResStr.resize(MAX_ITEM_CNT);
	m_vecResStr[0] = IDS_TR_LV1;
	m_vecResStr[1] = IDS_TR_LV1_SYNC_LOST;
	m_vecResStr[2] = IDS_TR_LV1_SYNC_BYTE_ERR;
	m_vecResStr[3] = IDS_TR_LV1_PAT_ERR;
	m_vecResStr[4] = IDS_TR_LV1_CC_ERR;
	m_vecResStr[5] = IDS_TR_LV1_PMT_ERR;
	m_vecResStr[6] = IDS_TR_LV1_PID_ERR;
	m_vecResStr[7] = IDS_TR_LV2;
	m_vecResStr[8] = IDS_TR_LV2_TS_ERR;
	m_vecResStr[9] = IDS_TR_LV2_CRC_ERR;
	m_vecResStr[10] = IDS_TR_LV2_PCR_REPET_ERR;
	m_vecResStr[11] = IDS_TR_LV2_PCR_DISCON_ERR;
	m_vecResStr[12] = IDS_TR_LV2_PCR_AC_ERR;
	m_vecResStr[13] = IDS_TR_LV2_PTS_ERR;
	m_vecResStr[14] = IDS_TR_LV2_CAT_ERR;
	m_vecResStr[15] = IDS_TR_LV3;
	m_vecResStr[16] = IDS_TR_LV3_NIT_ACT_ERR;
	m_vecResStr[17] = IDS_TR_LV3_NIT_OTHER_ERR;
	m_vecResStr[18] = IDS_TR_LV3_SI_REPET_ERR;
	m_vecResStr[19] = IDS_TR_LV3_UNREFER_PID_ERR;
	m_vecResStr[20] = IDS_TR_LV3_SDT_ACT_ERR;
	m_vecResStr[21] = IDS_TR_LV3_SDT_OTHER_ERR;
	m_vecResStr[22] = IDS_TR_LV3_EIT_ACT_ERR;
	m_vecResStr[23] = IDS_TR_LV3_EIT_OTHER_ERR;
	m_vecResStr[24] = IDS_TR_LV3_EIT_PF_ERR;
	m_vecResStr[25] = IDS_TR_LV3_RST_ERR;
	m_vecResStr[26] = IDS_TR_LV3_TDT_ERR;

    m_nLastColumn = 27;
}

void CTrView::OnTrReport(REPORT_PARAM_T param)
{
	CTrView* lpthis = (CTrView*)param.pApp;

	//TRACE(_T("level=%d,errName=%d,offset=%d\n"),param.level,param.errName,param.llOffset);

    TR_MSG_T* pTrMsg = lpthis->m_pTrMsgMgr->AddMsg(param);

	if (pTrMsg->nErrCount == -1)
	{
		return;
	}
		

	int nItem = pTrMsg->emErrType;
	lpthis->m_pErrCnt[nItem] = pTrMsg->nErrCount;
	
	
	if (pTrMsg->nSiRepetitionCount != -1)
	{
		lpthis->m_pErrCnt[TR_LV3_SI_REPET_ERR] = pTrMsg->nSiRepetitionCount;
	}

	//if (g_emInputType == INPUT_TYPE_LIVE_UDP)
    if (0)
	{
		struct timeval tv_now;
		gettimeofday(&tv_now,NULL);
		long long ll_now = (long long)tv_now.tv_sec*1000000 + tv_now.tv_usec;
		
		if (lpthis->m_llLastUpdateTime > 0 && ll_now - lpthis->m_llLastUpdateTime >= UPDATE_INTERVAL)
		{
			lpthis->m_llLastUpdateTime = ll_now;
		}

		if (lpthis->m_llLastUpdateTime < 0)
		{
			lpthis->m_llLastUpdateTime = ll_now;
		}
	}

	//更新MSG视图数据
	lpthis->m_pMsgView->AddMsg(param,pTrMsg->emErrType);


}


string CTrView::GetErrorBriefByErrorType(ERROR_TYPE_T emErrType)
{
    return ShowBrief(emErrType);
}
const std::deque<CTrMsgView::MSG_LIST_T>& CTrView:: GetErrorMsgListAll(ERROR_TYPE_T emErrType)
{
    return m_pMsgView->GetMsgList();
}

const std::deque<CTrMsgView::MSG_LIST_T>& CTrView:: GetErrorMsgListByErrorType(ERROR_TYPE_T emErrType)
{
    int nItem = emErrType;
	m_pMsgView->ClearMsg();


	std::list<REPORT_PARAM_T>::iterator it = m_pTrMsgMgr->m_vecMsg.begin();
	for (; it != m_pTrMsgMgr->m_vecMsg.end(); ++it)
	{
		if (nItem == TR_LV1 || nItem == TR_LV2 || nItem == TR_LV3)
		{
			continue;
		}

		TR_MSG_T* pTrMsg = m_pTrMsgMgr->PlayBackMsg(*it);

		if (pTrMsg->nErrCount == -1)
		{
			continue;
		}

		if (pTrMsg->emErrType != nItem)	//匹配的类型直接保留，不匹配的要看是否SI_REPETION
		{
			if (nItem == TR_LV3_SI_REPET_ERR && pTrMsg->nSiRepetitionCount > 0)
			{
				//当选择SI repetion 时，产生 sirepetion的保留，其余过滤掉
			}
			else
			{
				continue;
			}
		}
		//if (pTrMsg->nSiRepetitionCount < 0 && pTrMsg->emErrType != nItem)
		//{
		//	continue;//产生nSiRepetitionCount的都显示,除此之外非匹配类型的过滤掉
		//}

		m_pMsgView->AddMsg(*it,pTrMsg->emErrType);
	}

	return m_pMsgView->GetMsgList();
}

string CTrView::ShowBrief(int nItem)
{
    memset(m_pBriefBuf,0,m_pBriefBufLen);
	switch(nItem)
	{
	//case 	TR_LV1:
	case TR_LV1_SYNC_LOST:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_SYNC_LOST),m_pErrCnt[nItem]);
		break;
	case TR_LV1_SYNC_BYTE_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_SYNC_BYTE_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV1_PAT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_PAT_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_pat_occ,m_pTrMsgMgr->m_nErrCnt_pat_tid,m_pTrMsgMgr->m_nErrCnt_pat_scf);
		break;
	case TR_LV1_CC_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_CC_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV1_PMT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_PMT_ERR),m_pErrCnt[nItem],m_pTrMsgMgr->m_nErrCnt_pmt_occ,m_pTrMsgMgr->m_nErrCnt_pmt_scf);
		break;
	case TR_LV1_PID_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV1_BRIEF_PID_ERR),m_pErrCnt[nItem]);
		break;
	//case 	TR_LV2:
	case TR_LV2_TS_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_TS_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV2_CRC_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_CRC_ERR),m_pErrCnt[nItem],m_pTrMsgMgr->m_nErrCnt_crc_pat,
			m_pTrMsgMgr->m_nErrCnt_crc_cat,
			m_pTrMsgMgr->m_nErrCnt_crc_pmt,
			m_pTrMsgMgr->m_nErrCnt_crc_nit,
			m_pTrMsgMgr->m_nErrCnt_crc_sdt,
			m_pTrMsgMgr->m_nErrCnt_crc_bat,
			m_pTrMsgMgr->m_nErrCnt_crc_tot,
			m_pTrMsgMgr->m_nErrCnt_crc_eit);
		break;
	case TR_LV2_PCR_REPET_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_PCR_REPET_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV2_PCR_DISCON_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_PCR_DISCON_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV2_PCR_AC_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_PCR_AC_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV2_PTS_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_PTS_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV2_CAT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV2_BRIEF_CAT_ERR),m_pErrCnt[nItem],m_pTrMsgMgr->m_nErrCnt_cat_tid);
		break;
	//case 	TR_LV3:
	case TR_LV3_NIT_ACT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_NIT_ACT_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_nit_act_tid,m_pTrMsgMgr->m_nErrCnt_nit_act_timeout,m_pTrMsgMgr->m_nErrCnt_nit_act_lower25ms);
		break;
	case TR_LV3_NIT_OTHER_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_NIT_OTHER_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_nit_other_timeout,m_pTrMsgMgr->m_nErrCnt_nit_other_lower25ms);
		break;
	case TR_LV3_SI_REPET_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_SI_REPET_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_si_repetition_lower25ms,
			m_pTrMsgMgr->m_nErrCnt_nit_act_timeout + m_pTrMsgMgr->m_nErrCnt_nit_other_timeout,
			m_pTrMsgMgr->m_nErrCnt_bat_timeout,
			m_pTrMsgMgr->m_nErrCnt_sdt_act_timeout,
			m_pTrMsgMgr->m_nErrCnt_sdt_other_timeout,
			m_pTrMsgMgr->m_nErrCnt_eit_pf_act_timeout,
			m_pTrMsgMgr->m_nErrCnt_eit_pf_other_timeout,
			m_pTrMsgMgr->m_nErrCnt_eit_schedule_act_timeout,
			m_pTrMsgMgr->m_nErrCnt_eit_schedule_other_timeout,
			m_pTrMsgMgr->m_nErrCnt_tdt_timeout,
			m_pTrMsgMgr->m_nErrCnt_tot_timeout);
		break;
	case TR_LV3_UNREFER_PID_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_UNREFER_PID_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV3_SDT_ACT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_SDT_ACT_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_sdt_act_timeout,m_pTrMsgMgr->m_nErrCnt_sdt_act_tid,m_pTrMsgMgr->m_nErrCnt_sdt_act_lower25ms);
		break;
	case TR_LV3_SDT_OTHER_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_SDT_OTHER_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_sdt_other_timeout,m_pTrMsgMgr->m_nErrCnt_sdt_other_lower25ms);
		break;
	case TR_LV3_EIT_ACT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_EIT_ACT_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_eit_pf_act_timeout,m_pTrMsgMgr->m_nErrCnt_eit_pf_act_tid,m_pTrMsgMgr->m_nErrCnt_eit_pf_act_lower25ms);
		break;
	case TR_LV3_EIT_OTHER_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_EIT_OTHER_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_eit_pf_other_timeout,m_pTrMsgMgr->m_nErrCnt_eit_pf_other_lower25ms);
		break;
	case TR_LV3_EIT_PF_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_EIT_PF_ERR),m_pErrCnt[nItem]);
		break;
	case TR_LV3_RST_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_RST_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_rst_tid,m_pTrMsgMgr->m_nErrCnt_rst_lower25ms);
		break;
	case TR_LV3_TDT_ERR:
		sprintf(m_pBriefBuf,(IDS_TR_LV3_BRIEF_TDT_ERR),m_pErrCnt[nItem],
			m_pTrMsgMgr->m_nErrCnt_tdt_timeout,m_pTrMsgMgr->m_nErrCnt_tdt_tid,m_pTrMsgMgr->m_nErrCnt_tdt_lower25ms);
		break;
	default:
		break;
	}
    
    return m_pBriefBuf;
}

void CTrView::ClearError()
{
	for (int i = 0; i < MAX_ITEM_CNT; i++)
	{
		m_pErrCnt[i] = 0;
	}
}



//void CTrView::OnMenuTrShowAll()
//{
//	( (CTrChildFrame *)m_pParent )->m_pMsgView->ClearMsg();
//
//
//	std::list<REPORT_PARAM_T>::iterator it = m_pTrMsgMgr->m_vecMsg.begin();
//	for (; it != m_pTrMsgMgr->m_vecMsg.end(); ++it)
//	{
//		TR_MSG_T* pTrMsg = m_pTrMsgMgr->PlayBackMsg(*it);
//
//		if (pTrMsg->nErrCount == -1)
//		{
//			continue;
//		}
//
//		( (CTrChildFrame *)m_pParent )->m_pMsgView->AddMsg(*it,pTrMsg->emErrType);
//	}
//
//	( (CTrChildFrame *)m_pParent )->m_pMsgView->ShowData();
//}
//


