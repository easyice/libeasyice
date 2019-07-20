/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#include "StdAfx.h"
#include "H264DecodeCore.h"
#include "TsPacket.h"

extern "C"
{
	#include <vlc.h>
}

CH264DecodeCore::CH264DecodeCore(void)
{
	/*init_subset_sps_list(m_sps,MAXSPS);*/
	memset( m_sps,0,MAXSPS*sizeof(m_sps[0]) );
	memset( m_pps, 0, MAXPPS*sizeof(m_pps[0]) );

	for (int i = 0; i < MAXSPS; i++)
	{
		m_sps[i].seq_parameter_set_id = INT_MAX;
	}

	for (int i = 0; i < MAXPPS; i++)
	{
		m_pps[i].seq_parameter_set_id = INT_MAX;
	}

	m_oldSlice =  (OldSliceParams *) calloc(1, sizeof(OldSliceParams));
	init_old_slice(m_oldSlice);

	m_pBufferUnpk = new BYTE[204];
	memset(m_pBufferUnpk,0,204);

	m_currSlice = malloc_slice(NULL,NULL);

	m_currsps = AllocSPS();
	m_currpps = AllocPPS();

	m_pDecodeBuf = new BYTE[188*2];
	memset(m_pDecodeBuf,0,188*2);
	m_nDecodeBufPos = 0;
}

CH264DecodeCore::~CH264DecodeCore(void)
{
	ClearParset();
	free(m_oldSlice);

	delete [] m_pBufferUnpk;
	free_slice(m_currSlice);

	FreeSPS(m_currsps);
	FreePPS(m_currpps);

	delete [] m_pDecodeBuf;
}

void CH264DecodeCore::Reset()
{
	m_nDecodeBufPos = 0;
}

inline bool CH264DecodeCore::NextStartCode()
{
	int size = m_nDecodeBufPos - 3;
	int i=m_nByteOffset;
	for(;i<size;i++)
	{
		if((m_pDecodeBuf[i] == 0) &&
			(m_pDecodeBuf[i+1] == 0) &&
			(m_pDecodeBuf[i+2] == 1))
		{
			m_nByteOffset = i+2+1;
			return true;
		}
	}

	m_nByteOffset = i;
	return false;
}



inline seq_parameter_set_rbsp_t *CH264DecodeCore::GetActiveSps(pic_parameter_set_rbsp_t *active_pps)
{

	if (active_pps == NULL)
	{
		return NULL;
	}

	return GetActiveSps(active_pps->seq_parameter_set_id);
}

inline seq_parameter_set_rbsp_t *CH264DecodeCore::GetActiveSps(int seq_parameter_set_id)
{
	if (seq_parameter_set_id < 0 || seq_parameter_set_id >= MAXSPS)
	{
		return NULL;
	}

	seq_parameter_set_rbsp_t *active_sps = &m_sps[seq_parameter_set_id];
	if (active_sps->seq_parameter_set_id == INT_MAX)
	{
		return NULL;
	}

	return active_sps;
}

inline pic_parameter_set_rbsp_t *CH264DecodeCore::GetActivePps(const Slice* currSlice)
{
	if (currSlice->pic_parameter_set_id < 0 || currSlice->pic_parameter_set_id >= MAXPPS)
	{
		return NULL;
	}

	pic_parameter_set_rbsp_t *active_pps = &m_pps[currSlice->pic_parameter_set_id];
	if (active_pps->seq_parameter_set_id == INT_MAX)
	{
		return NULL;
	}

	return active_pps;
}

PARSED_FRAME_INFO CH264DecodeCore::ParseTsContinue(BYTE* pPacket,int nLen)
{
	PARSED_FRAME_INFO parsed_frame_info;
	PARSED_FRAME_INFO needed_parsed_frame_info;//符合new pic 条件的 slice
	bool recved_frame_info = false;
//	m_nPacketLen = nLen;

	//EBSP to RBSP
	memset(m_pBufferUnpk,0,204);

	//parse
	CTsPacket tsPacket;
	tsPacket.SetPacket(pPacket);

	int start_pos = tsPacket.Get_ES_pos();
	int paload_len = 188-start_pos;
	if (start_pos <= 187)
	{
		memcpy(m_pDecodeBuf+m_nDecodeBufPos,pPacket+start_pos,paload_len);
		m_nDecodeBufPos += paload_len;
	}

	//m_pPacket = pPacket;
	m_nByteOffset = 0;
	//m_nPacketLen = nLen;


	while(NextStartCode())
	{
		BYTE nal_unit_type = m_pDecodeBuf[m_nByteOffset] & 0x1F;
		m_nByteOffset++;


		//EBSP to RBSP
		memcpy(m_pBufferUnpk,m_pDecodeBuf+m_nByteOffset,m_nDecodeBufPos-m_nByteOffset);
		int rbsplen = EBSPtoRBSP(m_pBufferUnpk,m_nDecodeBufPos-m_nByteOffset,1);

		if (rbsplen < 0)
		{
			rbsplen = m_nDecodeBufPos-m_nByteOffset;//不好的处理方式，需要改进。。
		}

		//不很准确的防止溢出
		if (rbsplen < 3)
		{
			goto no_exit;
		}

		switch(nal_unit_type)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 19:
			{
				parsed_frame_info.bNewSlice = true;


				bool idr_flag = nal_unit_type == 5 ? true:false;
				Slice *slice = slice_header(m_pBufferUnpk,rbsplen,idr_flag,true);
				if (slice == NULL)
				{
					break;
				}

				if (nal_unit_type == 5)
				{
					parsed_frame_info.FrameType = FRAME_IDR;
				}
				else
				{
					 if (slice->slice_type == I_SLICE || slice->slice_type == SI_SLICE)
						parsed_frame_info.FrameType = FRAME_I;
					 else if(slice->slice_type == P_SLICE || slice->slice_type == SP_SLICE)
						 parsed_frame_info.FrameType = FRAME_P;
					 else if (slice->slice_type == B_SLICE)
						 parsed_frame_info.FrameType = FRAME_B;
				}

				//is_new_picture
				pic_parameter_set_rbsp_t *active_pps = GetActivePps(slice);
				seq_parameter_set_rbsp_t *active_sps = NULL;
				if (active_pps != NULL)
				{
					active_sps = GetActiveSps(active_pps);
				}

				if (m_oldSlice->pps_id == INT_MAX)	//第一个slice,如果是I slice并且含PES头，就认为是新帧的开始
				{
					BYTE stream_id;
					if (slice->slice_type == I_SLICE || slice->slice_type == SI_SLICE)
					{
						if (tsPacket.Get_PES_stream_id(stream_id))
						{
							parsed_frame_info.bNewPicture = true;
						}
					}
				}

				//第一个帧标志与PES头不在同一个TS包时会判断不到，屏蔽了第一个判断解决此问题
				if (/*(m_oldSlice->pps_id != INT_MAX) && */(active_sps != NULL))
				{
					parsed_frame_info.bNewPicture = is_new_picture_brief(active_sps,active_pps,slice,m_oldSlice);
				}

				if (active_sps != NULL)
				{
					CopySliceInfo(slice,m_oldSlice,active_sps);
				}

				parsed_frame_info.structure = slice->structure;

				if (!recved_frame_info && parsed_frame_info.bNewPicture)
				{
					needed_parsed_frame_info = parsed_frame_info;
					recved_frame_info = true;
				}
				break;
			}
		case 6:
			{
				sei_rbsp(m_pBufferUnpk,rbsplen);
				break;
			}
		case 7:
			{
				seq_parameter_set_rbsp_t *sps = sps_rbsp(m_pBufferUnpk,rbsplen);
				break;
			}
		case 8:
			{
				pic_parameter_set_rbsp_t *pps = pps_rbsp(m_pBufferUnpk,rbsplen);
				break;
			}
		default:
			break;

		}
	}

no_exit:
	memmove(m_pDecodeBuf,m_pDecodeBuf+m_nByteOffset,m_nDecodeBufPos-m_nByteOffset);
	m_nDecodeBufPos = m_nDecodeBufPos-m_nByteOffset;

	return needed_parsed_frame_info;
}


STRING_TREE* CH264DecodeCore::ParseTsPacket(BYTE *pPacket, int nLen)
{
//	ClearParset();
////	m_nPacketLen = nLen;
//
//	//EBSP to RBSP
//	BYTE *pBufferUnpk = new BYTE[m_nPacketLen];
//	memset(pBufferUnpk,0,m_nPacketLen);
//
//	//parse
//	CTsPacket tsPacket;
//	tsPacket.SetPacket(pPacket);
//
//	int start_pos = tsPacket.Get_ES_pos();
//
////	m_pPacket = pPacket;
//	m_nByteOffset = start_pos;
//
//
//	while(NextStartCode())
//	{
//		NextStartCode();
//		BYTE nal_unit_type; //= m_pPacket[m_nByteOffset] & 0x1F;
//		m_nByteOffset++;
//
//		//EBSP to RBSP
////		memcpy(pBufferUnpk,m_pPacket+m_nByteOffset,m_nPacketLen-m_nByteOffset);
//		int rbsplen = EBSPtoRBSP(pBufferUnpk,m_nPacketLen-m_nByteOffset,0);
//		switch(nal_unit_type)
//		{
//		case 1:
//		case 2:
//		case 3:
//		case 4:
//		case 5:
//		case 19:
//			{
//				Slice *slice = slice_header(pBufferUnpk,rbsplen,1,1);
//				slice_result(slice);
//				free_slice(slice);
//				break;
//			}
//		case 6:
//			{
//				sei_rbsp(pBufferUnpk,rbsplen);
//				break;
//			}
//		case 7:
//			{
//				seq_parameter_set_rbsp_t *sps = sps_rbsp(pBufferUnpk,rbsplen);
//				sps_result(sps);
//				FreeSPS(sps);
//				break;
//			}
//		case 8:
//			{
//				pic_parameter_set_rbsp_t *pps = pps_rbsp(pBufferUnpk,rbsplen);
//				pps_result(pps);
//				FreePPS(pps);
//				break;
//			}
//		default:
//			break;
//
//		}
//	}
//
//	delete [] pBufferUnpk;
	return &m_vecVideoParset;
}

void CH264DecodeCore::Init_Slice()
{
	//memset(m_currSlice,0,sizeof(Slice));
	m_currSlice->pic_parameter_set_id = -1;
}

void CH264DecodeCore::Init_SPS()
{
	memset(m_currsps,0,sizeof (seq_parameter_set_rbsp_t));
}

void CH264DecodeCore::Init_PPS()
{
	memset(m_currpps,0,sizeof (pic_parameter_set_rbsp_t));
	m_currpps->slice_group_id = NULL;
}

Slice * CH264DecodeCore::slice_header(BYTE* pRBSP,int nLen,bool bIdrFlag,bool bBrief)
{
	//return NULL;
	if (nLen <= 0)
	{
		return NULL;
	}

	//Slice *currSlice = malloc_slice(NULL,NULL);
	Slice *currSlice = m_currSlice;
	Init_Slice();


	Bitstream currStream;
	currStream.frame_bitoffset = 0;
	currStream.streamBuffer = pRBSP;
	currStream.bitstream_length = nLen;

	currSlice->idr_flag = bIdrFlag;
	FirstPartOfSliceHeader(currSlice,&currStream);

	pic_parameter_set_rbsp_t *active_pps = GetActivePps(currSlice);
	seq_parameter_set_rbsp_t *active_sps = GetActiveSps(active_pps);

	if (active_pps != NULL && active_sps != NULL)
	{
		if (bBrief)
		{
			RestOfSliceHeader_brief(currSlice,active_sps,active_pps,&currStream);
		}
		else
		{
			RestOfSliceHeader(currSlice,active_sps,active_pps,&currStream);
		}
	}

	return currSlice;

}

int CH264DecodeCore::is_new_picture_brief(seq_parameter_set_rbsp_t *active_sps,pic_parameter_set_rbsp_t *active_pps, Slice *currSlice, OldSliceParams *p_old_slice)
{
  //VideoParameters *p_Vid = currSlice->p_Vid;

  int result=0;

//  result |= (NULL==dec_picture);

  result |= (p_old_slice->pps_id != currSlice->pic_parameter_set_id);

  result |= (p_old_slice->frame_num != currSlice->frame_num);

  result |= (p_old_slice->field_pic_flag != currSlice->field_pic_flag);

  if(currSlice->field_pic_flag && p_old_slice->field_pic_flag)
  {
    result |= (p_old_slice->bottom_field_flag != currSlice->bottom_field_flag);
  }

 // result |= (p_old_slice->nal_ref_idc != currSlice->nal_reference_idc) && ((p_old_slice->nal_ref_idc == 0) || (currSlice->nal_reference_idc == 0));
  result |= (p_old_slice->idr_flag    != currSlice->idr_flag);

  if (currSlice->idr_flag && p_old_slice->idr_flag)
  {
    result |= (p_old_slice->idr_pic_id != currSlice->idr_pic_id);
  }

  if (active_sps->pic_order_cnt_type == 0)
  {
    result |= (p_old_slice->pic_oder_cnt_lsb          != currSlice->pic_order_cnt_lsb);
    if( active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
    {
      result |= (p_old_slice->delta_pic_oder_cnt_bottom != currSlice->delta_pic_order_cnt_bottom);
    }
  }

  if (active_sps->pic_order_cnt_type == 1)
  {
    if (!active_sps->delta_pic_order_always_zero_flag)
    {
      result |= (p_old_slice->delta_pic_order_cnt[0] != currSlice->delta_pic_order_cnt[0]);
      if( active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
      {
        result |= (p_old_slice->delta_pic_order_cnt[1] != currSlice->delta_pic_order_cnt[1]);
      }
    }
  }
//
//#if (MVC_EXTENSION_ENABLE)
//  result |= (currSlice->view_id != p_old_slice->view_id);
//  result |= (currSlice->inter_view_flag != p_old_slice->inter_view_flag);
//  result |= (currSlice->anchor_pic_flag != p_old_slice->anchor_pic_flag);
//#endif

  return result;
}


int CH264DecodeCore::RestOfSliceHeader_brief(Slice *currSlice,seq_parameter_set_rbsp_t *active_sps,pic_parameter_set_rbsp_t *active_pps,Bitstream *currStream)
{
  int intra_profile_deblocking = 0;
//  int val, len;

  currSlice->frame_num = u_v (active_sps->log2_max_frame_num_minus4 + 4, "SH: frame_num", currStream);


  if (active_sps->frame_mbs_only_flag)
  {
	currSlice->structure = FRAME;
    currSlice->field_pic_flag=0;
  }
  else
  {
    // field_pic_flag   u(1)
    currSlice->field_pic_flag = u_1("SH: field_pic_flag", currStream);
    if (currSlice->field_pic_flag)
    {
      // bottom_field_flag  u(1)
      currSlice->bottom_field_flag = (byte) u_1("SH: bottom_field_flag", currStream);
      currSlice->structure = currSlice->bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
    }
    else
    {
      currSlice->structure = FRAME;
      currSlice->bottom_field_flag = FALSE;
    }
  }

  //currSlice->structure = (PictureStructure) p_Vid->structure;

  currSlice->mb_aff_frame_flag = (active_sps->mb_adaptive_frame_field_flag && (currSlice->field_pic_flag==0));


  if (currSlice->idr_flag)
  {
    currSlice->idr_pic_id = ue_v("SH: idr_pic_id", currStream);
  }

  if (active_sps->pic_order_cnt_type == 0)
  {
    currSlice->pic_order_cnt_lsb = u_v(active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4, "SH: pic_order_cnt_lsb", currStream);
    if( active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1 &&  !currSlice->field_pic_flag )
      currSlice->delta_pic_order_cnt_bottom = se_v("SH: delta_pic_order_cnt_bottom", currStream);
    else
      currSlice->delta_pic_order_cnt_bottom = 0;
  }

  if( active_sps->pic_order_cnt_type == 1)
  {
    if ( !active_sps->delta_pic_order_always_zero_flag )
    {
      currSlice->delta_pic_order_cnt[ 0 ] = se_v("SH: delta_pic_order_cnt[0]", currStream);
      if( active_pps->bottom_field_pic_order_in_frame_present_flag  ==  1  &&  !currSlice->field_pic_flag )
        currSlice->delta_pic_order_cnt[ 1 ] = se_v("SH: delta_pic_order_cnt[1]", currStream);
      else
        currSlice->delta_pic_order_cnt[ 1 ] = 0;  // set to zero if not in stream
    }
    else
    {
      currSlice->delta_pic_order_cnt[ 0 ] = 0;
      currSlice->delta_pic_order_cnt[ 1 ] = 0;
    }
  }

  return 0;
}

seq_parameter_set_rbsp_t * CH264DecodeCore::sps_rbsp(BYTE* pRBSP,int nLen)
{
	/*return NULL;*/
	if (nLen <= 0)
	{
		return NULL;
	}

	seq_parameter_set_rbsp_t *sps = m_currsps;//AllocSPS();
	Init_SPS();

	Bitstream currStream;
	currStream.frame_bitoffset = 0;
	currStream.streamBuffer = pRBSP;
	currStream.bitstream_length = nLen;
	if (InterpretSPS(sps,&currStream) <= 0)
	{
		return NULL;
	}

	MakeSPSavailable(m_sps,sps->seq_parameter_set_id, sps);


	return sps;
}



pic_parameter_set_rbsp_t * CH264DecodeCore::pps_rbsp(BYTE* pRBSP,int nLen)
{
	/*return NULL;*/
	if (nLen <= 0)
	{
		return NULL;
	}

	//parse sps_id
	Bitstream tmpStream;
	tmpStream.frame_bitoffset = 0;
	tmpStream.streamBuffer = pRBSP;
	tmpStream.bitstream_length = nLen;
	int pic_parameter_set_id                  = ue_v ("PPS: pic_parameter_set_id"                   , &tmpStream);
	int seq_parameter_set_id                  = ue_v ("PPS: seq_parameter_set_id"                   , &tmpStream);

	seq_parameter_set_rbsp_t* active_sps = GetActiveSps(seq_parameter_set_id);
	if (active_sps == NULL)
	{
		return NULL;
	}


	pic_parameter_set_rbsp_t *pps = m_currpps;//AllocPPS();
	Init_PPS();

	Bitstream currStream;
	currStream.frame_bitoffset = 0;
	currStream.streamBuffer = pRBSP;
	currStream.bitstream_length = nLen;

	InterpretPPS(pps,active_sps,&currStream);
	MakePPSavailable(m_pps, pps->pic_parameter_set_id,pps);

	return pps;

}

void CH264DecodeCore::sei_rbsp(BYTE* pRBSP,int nLen)
{

}

void CH264DecodeCore::ClearParset()
{
	STRING_TREE::iterator it = m_vecVideoParset.begin();
	for (; it != m_vecVideoParset.end(); ++it)
	{
		ClearSubSet(&(*it));
	}
	m_vecVideoParset.clear();
}

void CH264DecodeCore::ClearSubSet(STRING_TREE_LEAF* root)
{
	if (root->subset == NULL)
		return;
	else
	{
		ClearSubSet(root->subset);
		delete root->subset;
	}
}

STRING_TREE_LEAF* CH264DecodeCore::AddRootItem(const string& element,const string& value,bool exist,STRING_TREE_LEAF* pParent)
{
	STRING_TREE_ITEM item;
	item.element = element;
	item.value = value;
	item.exist = exist;
	if (pParent == NULL)
	{
		STRING_TREE_LEAF itemset;
		itemset.lstItem.push_back(item);
		itemset.subset = new STRING_TREE_LEAF();
		m_vecVideoParset.push_back(itemset);
		return itemset.subset;
	}
	else
	{
		pParent->lstItem.push_back(item);
		pParent->subset = new STRING_TREE_LEAF();
		return pParent->subset;
	}
	return NULL;
}

inline void CH264DecodeCore::InsertItem(const string& element,const string& value,bool exist,STRING_TREE_LEAF* pParent)
{
	assert(pParent != NULL);
	STRING_TREE_ITEM item;
	item.element = element;
	item.value = value;
	item.exist = exist;
	pParent->lstItem.push_back(item);
}

inline string CH264DecodeCore::lltowstr(long long value)
{
        return "";
        //modify here
	//wchar_t buffer[65];
	//_ui64tow_s(value,buffer,65,10);
	//return buffer;
}


void CH264DecodeCore::slice_result(const Slice* currSlice)
{
	STRING_TREE_LEAF* slice_root = AddRootItem("slice_header()","",true,NULL);
	InsertItem("first_mb_in_slice",lltowstr(currSlice->start_mb_nr),true,slice_root);
}

void CH264DecodeCore::sps_result(const seq_parameter_set_rbsp_t* sps)
{
	STRING_TREE_LEAF itemset;

	m_vecVideoParset.push_back(itemset);
}

void CH264DecodeCore::pps_result(const pic_parameter_set_rbsp_t* pps)
{
	STRING_TREE_LEAF itemset;

	m_vecVideoParset.push_back(itemset);
}

