/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "../commondefs.h"

extern "C"
{
	#include "global.h"
	#include "header.h"
	#include "parset.h"
	#include "image.h"
}

using namespace std;
class CH264DecodeCore
{
public:
	CH264DecodeCore(void);
	~CH264DecodeCore(void);
	STRING_TREE* ParseTsPacket(BYTE* pPacket,int nLen);

	//只返回第一个符合new pic 条件的slice
	PARSED_FRAME_INFO ParseTsContinue(BYTE* pPacket,int nLen);

	void Reset();
private:
	bool NextStartCode();
	//bool NextNAL();
	Slice * slice_header(BYTE* pRBSP,int nLen,bool bIdrFlag,bool bBrief);
	seq_parameter_set_rbsp_t * sps_rbsp(BYTE* pRBSP,int nLen);
	pic_parameter_set_rbsp_t * pps_rbsp(BYTE* pRBSP,int nLen);
	void sei_rbsp(BYTE* pRBSP,int nLen);

	void ClearParset();
	void ClearSubSet(STRING_TREE_LEAF* root);

	int RestOfSliceHeader_brief(Slice *currSlice,seq_parameter_set_rbsp_t *active_sps,pic_parameter_set_rbsp_t *active_pps,Bitstream *currStream);
	int is_new_picture_brief(seq_parameter_set_rbsp_t *active_sps,pic_parameter_set_rbsp_t *active_pps, Slice *currSlice, OldSliceParams *p_old_slice);

	//解析结果转换为字符串
	void slice_result(const Slice* currSlice);
	void sps_result(const seq_parameter_set_rbsp_t* sps);
	void pps_result(const pic_parameter_set_rbsp_t* pps);

	seq_parameter_set_rbsp_t *GetActiveSps(pic_parameter_set_rbsp_t *active_pps);
	seq_parameter_set_rbsp_t *GetActiveSps(int seq_parameter_set_id);
	pic_parameter_set_rbsp_t *GetActivePps(const Slice* currSlice);

	//init
	void Init_Slice();
	void Init_SPS();
	void Init_PPS();


	/**
	* @brief 添加一个带子项的条目
	* @return return the subset point
	*/
	STRING_TREE_LEAF* AddRootItem(const string& element,const string& value,bool exist,STRING_TREE_LEAF* pParent);
	void InsertItem(const string& element,const string& value,bool exist,STRING_TREE_LEAF* pParent);

	string lltowstr(long long value);
private:
	//BYTE* m_pPacket;
	//int m_nPacketLen;
	int m_nByteOffset;

	BYTE* m_pBufferUnpk;
	Slice *m_currSlice;

	BYTE* m_pDecodeBuf;
	int m_nDecodeBufPos;

	pic_parameter_set_rbsp_t *m_currpps;
	seq_parameter_set_rbsp_t *m_currsps;

	seq_parameter_set_rbsp_t m_sps[MAXSPS];
	pic_parameter_set_rbsp_t m_pps[MAXPPS];


	OldSliceParams *m_oldSlice;
	STRING_TREE m_vecVideoParset;
};
