/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PSI_CHECK_H
#define PSI_CHECK_H



#include "config.h"


class CTrCore;

class CPsiCheck
{
	typedef struct _eit_section_t
	{
		uint8_t					  i_table_id;             /*!< table_id */
		uint16_t                  i_service_id;       /*!< service_id */
		uint8_t                   i_version;          /*!< version_number */
		uint8_t					  i_number;               /*!< section_number */
		uint16_t                  i_ts_id;            /*!< transport stream id */
		uint16_t                  i_network_id;       /*!< original network id */
	}eit_section_t;
public:
	CPsiCheck(CTrCore* pParent);
	~CPsiCheck();

	void AddPacket(uint8_t* pPacket,bool bEs,int pid);

	void AddPmtPid(int pid,int program_num);
private:
	dvbpsi_handle NewHandle();
	void FreeHandle(dvbpsi_handle h_dvbpsi);
	void PushPacket(dvbpsi_handle h_dvbpsi, uint8_t* p_data,int pid);

	void InitAllCkHds();
	void UnInitAllCkHds();

private:
	void OnCrcError(uint8_t table_id,int pid);
	void OnRecvNewSection(dvbpsi_psi_section_t * p_section,int pid);
	void CheckPrevEitPF(int pid);
	void UpdatePF(int section_number);
private:
	//下标为PID，内容为 handle
	dvbpsi_handle* m_pAllCkHds;

	//下标为table_id，内容为时间
	long long* m_pOldOccurTime;

	//当前包是否EIT PF 包
	//bool m_bEitPFPacket;

	bool m_bHaveEit_P;
	bool m_bHaveEit_F;
	//int m_nPrevEitSectionNumber;
	//tables::EIT m_PrevEitSection;
	eit_section_t m_PrevEitSection;
	bool m_bWaitFirstEitSection;

	CTrCore* m_pParent;
};



#endif

