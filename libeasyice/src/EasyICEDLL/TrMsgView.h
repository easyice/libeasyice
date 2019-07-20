/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <vector>
#include <iostream>
#include <deque>
 #include "libtr101290.h"
#include "json/json.h"


using namespace std;

class CTrMsgView  
{
    public:
typedef struct  _MSG_LIST_T
{
	REPORT_PARAM_T param;
	int nErrType;
}MSG_LIST_T;

public:
	CTrMsgView();           // 动态创建所使用的受保护的构造函数
	virtual ~CTrMsgView();


public:
	void AddMsg(const REPORT_PARAM_T& param,int nErrType);
    const std::deque<CTrMsgView::MSG_LIST_T>& GetMsgList();
    Json::Value MsgListToJson();
	void ClearMsg();
    string GetMsgStringByReportParam(const REPORT_PARAM_T& param);
	const string& GetErrorTypeString(int nType);
public:
	std::deque<MSG_LIST_T> m_dqLst;


	std::vector<string> m_vecErrTypeString;
	//HBRUSH m_hbr;
private:


	void InitErrorTypeString();

private:
	int m_nLastColumn; //最后一列
	int m_nMaxItemPerPage;
    char* m_pStrBuf;
    int m_pStrBufLen;
};


