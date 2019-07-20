/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "CDescriptorImpl.h"
#include "ztypes.h"

using namespace std;



static	char* m_pTcharBuffer;
static	char*  m_pCharBuffer;
static	int m_nBufferSize;


CDescriptorImpl::CDescriptorImpl(void)
{
	for(int i = 0; i < 256; i++)
	{
		m_pDecodefunc[i] = DumpDescriptorDefault;
	}

	m_nBufferSize = 256;
	m_pTcharBuffer = new char[m_nBufferSize];
	m_pCharBuffer = new char[m_nBufferSize];

	m_pDecodefunc[0x02] = DumpVideoStreamDescriptor;
	m_pDecodefunc[0x03] = DumpAudioStreamDescriptor;
	m_pDecodefunc[0x04] = DumpHierarchyDescriptor;
	m_pDecodefunc[0x05] = DumpRegistrationDescriptor;
	m_pDecodefunc[0x06] = DumpDataStreamAlignmentDescriptor;
	m_pDecodefunc[0x07] = DumpTargetBackgroundGridDescriptor;
	m_pDecodefunc[0x08] = DumpVideoWindowDescriptor;
	m_pDecodefunc[0x09] = DumpCADescriptor;
	m_pDecodefunc[0x0A] = DumpISO_639LanguageDescriptor;
	m_pDecodefunc[0x0B] = DumpSystemClockDescriptor;
	m_pDecodefunc[0x0C] = DumpMultiplexBufferUtilizationDescriptor;
	m_pDecodefunc[0x0D] = DumpCopyrightDescriptor;
	m_pDecodefunc[0x0E] = DumpMaxBitrateDescriptor;
	m_pDecodefunc[0x0F] = DumpPrivateDataIndicatorDescriptor;


	m_pDecodefunc[0x42] = DumpStuffingDescriptor;
	m_pDecodefunc[0x43] = DumpSatelliteDeliverySystemDescriptor;
	m_pDecodefunc[0x44] = DumpCableDeliverySystemDescriptor;
	m_pDecodefunc[0x45] = DumpVBITeletextDescriptor;
	m_pDecodefunc[0x47] = DumpBouquetNameDescriptor;
	m_pDecodefunc[0x48] = DumpServiceDescriptor;
	m_pDecodefunc[0x4D] = DumpShortEventDescriptor;
	m_pDecodefunc[0x4E] = DumpExtendedEventDescriptor;

	m_pDecodefunc[0x52] = DumpStreamIdentifierDescriptor;
	m_pDecodefunc[0x55] = DumpParentalRatingDescriptor;
	m_pDecodefunc[0x56] = DumpTeletextDescriptor;
	m_pDecodefunc[0x58] = DumpLocalTimeOffsetDescriptor;
	m_pDecodefunc[0x59] = DumpSubtitleDescriptor;
	m_pDecodefunc[0x5A] = DumpTerrestrialDeliverySystemDescriptor;

	m_pDecodefunc[0x69] = DumpPDCDescriptor;

	m_pDecodefunc[0x8A] = DumpCUEIDescriptor;


}

CDescriptorImpl::~CDescriptorImpl(void)
{
	delete [] m_pTcharBuffer;
	delete [] m_pCharBuffer;
}


dvbpsi_descriptor_t* CDescriptorImpl::dvbpsi_AddDescriptor(dvbpsi_descriptor_t ** p_first_descriptor_in,
												 uint8_t i_tag, uint8_t i_length,
												 uint8_t* p_data)
{
	dvbpsi_descriptor_t* p_descriptor
		= dvbpsi_NewDescriptor(i_tag, i_length, p_data);

	//dvbpsi_descriptor_t* p_first_descriptor = *p_first_descriptor_in;

	if(p_descriptor)
	{
		if(*p_first_descriptor_in == NULL)
		{
			*p_first_descriptor_in = p_descriptor;
		}
		else
		{
			dvbpsi_descriptor_t* p_last_descriptor = *p_first_descriptor_in;
			while(p_last_descriptor->p_next != NULL)
				p_last_descriptor = p_last_descriptor->p_next;
			p_last_descriptor->p_next = p_descriptor;
		}
	}

	return p_descriptor;
}



void CDescriptorImpl::DumpDescriptors(DECODED_DESCRIPTOR_N& descriptor_n, dvbpsi_descriptor_t* p_descriptor)
{

	while(p_descriptor)
	{
		if (p_descriptor->i_tag < TAG_MAX && m_pDecodefunc[p_descriptor->i_tag] != NULL)
		{
			m_pDecodefunc[p_descriptor->i_tag](descriptor_n,p_descriptor);
		}

		p_descriptor = p_descriptor->p_next;
	}
}


void CDescriptorImpl::DecodeDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,BYTE* pData,int nLen)
{
	dvbpsi_descriptor_t *     p_first_descriptor = NULL;

	uint8_t* p_byte = pData;
	uint8_t* p_end = p_byte + nLen;
	while(p_byte + 2 <= p_end)
	{
		uint8_t i_tag = p_byte[0];
		uint8_t i_length = p_byte[1];
		if(i_length + 2 <= p_end - p_byte)
			dvbpsi_AddDescriptor(&p_first_descriptor, i_tag, i_length, p_byte + 2);
		p_byte += 2 + i_length;
	}

	DumpDescriptors(descriptor_n,p_first_descriptor);

	dvbpsi_DeleteDescriptors(p_first_descriptor);

}

inline void CDescriptorImpl::Item_push_back_uint(STRING_TREE_LEAF& leaf,char* name,uint64_t val)
{
	memset(m_pTcharBuffer,0,m_nBufferSize);
	snprintf(m_pTcharBuffer,m_nBufferSize,("0x%02X"),val);
	leaf.push_back(name,m_pTcharBuffer);
}

inline void CDescriptorImpl::Item_push_back_int64(STRING_TREE_LEAF& leaf,char* name,long long val)
{
	memset(m_pTcharBuffer,0,m_nBufferSize);
	snprintf(m_pTcharBuffer,m_nBufferSize,("0x%02X"),val);
	leaf.push_back(name,m_pTcharBuffer);
}

inline void CDescriptorImpl::Item_push_back_str(STRING_TREE_LEAF& leaf,char* name,uint8_t* data,uint32_t len)
{
	memset(m_pCharBuffer,0,m_nBufferSize);

	if (len > 255) len = 255;
	memcpy(m_pCharBuffer,data,len);
	leaf.push_back(name,m_pCharBuffer);
}

inline void CDescriptorImpl::Item_push_back_hex(STRING_TREE_LEAF& leaf,char* name,uint8_t* data,uint32_t len)
{
	string str;
	
	for (uint32_t i = 0; i < len; i++)
	{
		if (i != 0)
		{
			str += (",");
		}
		memset(m_pTcharBuffer,0,m_nBufferSize);
		snprintf(m_pTcharBuffer,m_nBufferSize,("0x%02X"),data[i]);
		str += m_pTcharBuffer;
	}
	leaf.push_back(name,str);
}

void CDescriptorImpl::DumpDescriptorDefault(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);
	Item_push_back_hex(leaf,("descriptor_data"),p_descriptor->p_data,p_descriptor->i_length);



	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	//Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	//Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);
	//Item_push_back_hex(leaf,("descriptor_data"),p_descriptor->p_data,p_descriptor->i_length);


	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);

}


void CDescriptorImpl::DumpVideoStreamDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_vstream_dr_t* p = dvbpsi_DecodeVStreamDr(p_descriptor);
	if (p != NULL)
	{
		DECODED_DESCRIPTOR dr_null;
		STRING_TREE_LEAF leaf_null;

		dr_null.str_tree.push_back(leaf_null);
		descriptor_n.push_back(dr_null);

		DECODED_DESCRIPTOR& dr = descriptor_n.back();
		STRING_TREE_LEAF& leaf = dr.str_tree.back();

		//DECODED_DESCRIPTOR dr;
		//STRING_TREE_LEAF leaf;

		Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
		Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

		Item_push_back_int64(leaf,("multiple_frame_rate"),p->b_multiple_frame_rate);
		Item_push_back_uint(leaf,("frame_rate_code"),p->i_frame_rate_code);
		Item_push_back_int64(leaf,("mpeg2"),p->b_mpeg2);
		Item_push_back_int64(leaf,("constrained_parameter"),p->b_constrained_parameter);
		Item_push_back_int64(leaf,("still_picture"),p->b_still_picture);
		Item_push_back_uint(leaf,("profile_level_indication"),p->i_profile_level_indication);
		Item_push_back_uint(leaf,("chroma_format"),p->i_chroma_format);
		Item_push_back_int64(leaf,("frame_rate_extension"),p->b_frame_rate_extension);
		
		dr.dr_name = ("video_stream_descriptor");
		//dr.str_tree.push_back(leaf);
		//descriptor_n.push_back(dr);
	}
}

void CDescriptorImpl::DumpAudioStreamDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_astream_dr_t* p = dvbpsi_DecodeAStreamDr(p_descriptor);
	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();
	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_int64(leaf,("free_format"),p->b_free_format);
	Item_push_back_uint(leaf,("id"),p->i_id);
	Item_push_back_uint(leaf,("layer"),p->i_layer);

	
	dr.dr_name = ("audio_stream_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpHierarchyDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_hierarchy_dr_t* p = dvbpsi_DecodeHierarchyDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("h_type"),p->i_h_type);
	Item_push_back_uint(leaf,("h_layer_index"),p->i_h_layer_index);
	Item_push_back_uint(leaf,("h_embedded_layer"),p->i_h_embedded_layer);
	Item_push_back_uint(leaf,("h_priority"),p->i_h_priority);

	
	dr.dr_name = ("hierarchy_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpRegistrationDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_registration_dr_t* p = dvbpsi_DecodeRegistrationDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("format_identifier"),p->i_format_identifier);
	Item_push_back_uint(leaf,("additional_length"),p->i_additional_length);
	Item_push_back_hex(leaf,("additional_info"),p->i_additional_info,p->i_additional_length);
	
	dr.dr_name = ("registration_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpDataStreamAlignmentDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_ds_alignment_dr_t* p = dvbpsi_DecodeDSAlignmentDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("alignment_type"),p->i_alignment_type);
	
	dr.dr_name = ("data_stream_alignment_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpTargetBackgroundGridDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_target_bg_grid_dr_t* p = dvbpsi_DecodeTargetBgGridDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("horizontal_size"),p->i_horizontal_size);
	Item_push_back_uint(leaf,("vertical_size"),p->i_vertical_size);
	Item_push_back_uint(leaf,("pel_aspect_ratio"),p->i_pel_aspect_ratio);
	
	dr.dr_name = ("target_background_grid_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpVideoWindowDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_vwindow_dr_t* p = dvbpsi_DecodeVWindowDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("horizontal_offset"),p->i_horizontal_offset);
	Item_push_back_uint(leaf,("vertical_offset"),p->i_vertical_offset);
	Item_push_back_uint(leaf,("window_priority"),p->i_window_priority);
	
	dr.dr_name = ("video_window_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpCADescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_ca_dr_t* p = dvbpsi_DecodeCADr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("ca_system_id"),p->i_ca_system_id);
	Item_push_back_uint(leaf,("ca_pid"),p->i_ca_pid);
	Item_push_back_uint(leaf,("private_length"),p->i_private_length);
	Item_push_back_hex(leaf,("private_data"),p->i_private_data,p->i_private_length);
	
	dr.dr_name = ("ca_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpISO_639LanguageDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_iso639_dr_t* p = dvbpsi_DecodeISO639Dr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("code_count"),p->i_code_count);
	for (uint8_t i = 0 ; i < p->i_code_count; i++)
	{
		Item_push_back_hex(leaf,("iso_639_code"),p->code[i].iso_639_code,sizeof(p->code[i].iso_639_code));
		Item_push_back_uint(leaf,("audio_type"),p->code[i].i_audio_type);
	}
	
	dr.dr_name = ("iso_639language_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpSystemClockDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_system_clock_dr_t* p = dvbpsi_DecodeSystemClockDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_int64(leaf,("external_clock_ref"),p->b_external_clock_ref);
	Item_push_back_uint(leaf,("clock_accuracy_integer"),p->i_clock_accuracy_integer);
	Item_push_back_uint(leaf,("clock_accuracy_exponent"),p->i_clock_accuracy_exponent);
	
	dr.dr_name = ("system_clock_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpMultiplexBufferUtilizationDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_mx_buff_utilization_dr_t* p = dvbpsi_DecodeMxBuffUtilizationDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_int64(leaf,("mdv_valid"),p->b_mdv_valid);
	Item_push_back_uint(leaf,("mx_delay_variation"),p->i_mx_delay_variation);
	Item_push_back_uint(leaf,("mx_strategy"),p->i_mx_strategy);
	
	dr.dr_name = ("multiplex_buffer_utilization_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);

}

void CDescriptorImpl::DumpCopyrightDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_copyright_dr_t* p = dvbpsi_DecodeCopyrightDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("copyright_identifier"),p->i_copyright_identifier);
	Item_push_back_uint(leaf,("additional_length"),p->i_additional_length);
	//Item_push_back_hex(leaf,("i_additional_info_hex"),p->i_additional_info,p->i_additional_length);
	Item_push_back_str(leaf,("i_additional_info_str"),p->i_additional_info,p->i_additional_length);
	
	dr.dr_name = ("copyright_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}


void CDescriptorImpl::DumpMaxBitrateDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_max_bitrate_dr_t* p = dvbpsi_DecodeMaxBitrateDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("max_bitrate"),p->i_max_bitrate);
	
	dr.dr_name = ("max_bitrate_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpPrivateDataIndicatorDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_private_data_dr_t* p = dvbpsi_DecodePrivateDataDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("private_data"),p->i_private_data);
	
	dr.dr_name = ("private_data_indicator_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpStuffingDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_stuffing_dr_t* p = dvbpsi_DecodeStuffingDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("stuffing_length"),p->i_stuffing_length);
	Item_push_back_hex(leaf,("stuffing_byte"),p->i_stuffing_byte,p->i_stuffing_length);
	
	dr.dr_name = ("stuffing_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpSatelliteDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_sat_deliv_sys_dr_t* p = dvbpsi_DecodeSatDelivSysDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("frequency"),p->i_frequency);
	Item_push_back_uint(leaf,("orbital_position"),p->i_orbital_position);
	Item_push_back_uint(leaf,("west_east_flag"),p->i_west_east_flag);
	Item_push_back_uint(leaf,("polarization"),p->i_polarization);
	Item_push_back_uint(leaf,("roll_off"),p->i_roll_off);
	Item_push_back_uint(leaf,("modulation_system"),p->i_modulation_system);
	Item_push_back_uint(leaf,("modulation_type"),p->i_modulation_type);
	Item_push_back_uint(leaf,("symbol_rate"),p->i_symbol_rate);
	Item_push_back_uint(leaf,("fec_inner"),p->i_fec_inner);
	
	dr.dr_name = ("satellite_delivery_system_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpCableDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_cable_deliv_sys_dr_t* p = dvbpsi_DecodeCableDelivSysDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("frequency"),p->i_frequency);
	Item_push_back_uint(leaf,("modulation"),p->i_modulation);
	Item_push_back_uint(leaf,("symbol_rate"),p->i_symbol_rate);
	Item_push_back_uint(leaf,("fec_inner"),p->i_fec_inner);
	Item_push_back_uint(leaf,("fec_outer"),p->i_fec_outer);
	
	dr.dr_name = ("cabledeliverysystem_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpVBITeletextDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_vbi_dr_t* p = dvbpsi_DecodeVBIDataDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("services_number"),p->i_services_number);
	for (int i = 0; i < p->i_services_number; i++)
	{
		Item_push_back_uint(leaf,("data_service_id"),p->p_services[i].i_data_service_id);
		Item_push_back_uint(leaf,("lines"),p->p_services[i].i_lines);

		for (int j = 0; j < p->p_services[i].i_lines; j++)
		{
			Item_push_back_uint(leaf,("parity"),p->p_services[i].p_lines[j].i_parity);
			Item_push_back_uint(leaf,("line_offset"),p->p_services[i].p_lines[j].i_line_offset);
		}
	}
	
	dr.dr_name = ("vbi_teletext_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpBouquetNameDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_bouquet_name_dr_t* p = dvbpsi_DecodeBouquetNameDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("name_length"),p->i_name_length);
	Item_push_back_str(leaf,("char"),p->i_char,p->i_name_length);
	
	dr.dr_name = ("bouquet_name_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpServiceDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_service_dr_t* p_service_descriptor = dvbpsi_DecodeServiceDr(p_descriptor);
	if (p_service_descriptor != NULL)
	{
		DECODED_DESCRIPTOR dr_null;
		STRING_TREE_LEAF leaf_null;

		dr_null.str_tree.push_back(leaf_null);
		descriptor_n.push_back(dr_null);

		DECODED_DESCRIPTOR& dr = descriptor_n.back();
		STRING_TREE_LEAF& leaf = dr.str_tree.back();

		//DECODED_DESCRIPTOR dr;
		//STRING_TREE_LEAF leaf;

		Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
		Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);
		Item_push_back_uint(leaf,("service_type"),p_service_descriptor->i_service_type);
		Item_push_back_uint(leaf,("service_provider_length"),p_service_descriptor->i_service_provider_name_length);
		Item_push_back_str(leaf,("service_provider"),p_service_descriptor->i_service_provider_name,p_service_descriptor->i_service_provider_name_length);
		Item_push_back_uint(leaf,("service_name_length"),p_service_descriptor->i_service_name_length);
		Item_push_back_str(leaf,("service_name"),p_service_descriptor->i_service_name,p_service_descriptor->i_service_name_length);

		dr.dr_name = ("service_descriptor");
		//dr.str_tree.push_back(leaf);
		//descriptor_n.push_back(dr);
	}
}


void CDescriptorImpl::DumpShortEventDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_short_event_dr_t* p = dvbpsi_DecodeShortEventDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_hex(leaf,("iso_639_code"),p->i_iso_639_code,sizeof(p->i_iso_639_code));
	Item_push_back_int64(leaf,("event_name_length"),p->i_event_name_length);
	Item_push_back_str(leaf,("event_name"),p->i_event_name,p->i_event_name_length);
	Item_push_back_int64(leaf,("text_length"),p->i_text_length);
	Item_push_back_str(leaf,("text"),p->i_text,p->i_text_length);
	
	dr.dr_name = ("short_event_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpExtendedEventDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_extended_event_dr_t* p = dvbpsi_DecodeExtendedEventDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("descriptor_number"),p->i_descriptor_number);
	Item_push_back_uint(leaf,("last_descriptor_number"),p->i_last_descriptor_number);
	Item_push_back_hex(leaf,("iso_639_code"),p->i_iso_639_code,sizeof(p->i_iso_639_code));
	Item_push_back_int64(leaf,("entry_count"),p->i_entry_count);
	
	for (int i = 0; i < p->i_entry_count;i++)
	{
		Item_push_back_uint(leaf,("item_description_length"),p->i_item_description_length[i]);
		Item_push_back_str(leaf,("item_description"),p->i_item_description[i],p->i_item_description_length[i]);
		Item_push_back_uint(leaf,("item_length"),p->i_item_length[i]);
		Item_push_back_str(leaf,("item"),p->i_item[i],p->i_item_length[i]);
	}

	Item_push_back_int64(leaf,("text_length"),p->i_text_length);
	Item_push_back_str(leaf,("text"),p->i_text,p->i_text_length);

	dr.dr_name = ("extended_event_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpStreamIdentifierDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_stream_identifier_dr_t* p = dvbpsi_DecodeStreamIdentifierDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("component_tag"),p->i_component_tag);
	
	dr.dr_name = ("stream_identifier_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpParentalRatingDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_parental_rating_dr_t* p = dvbpsi_DecodeParentalRatingDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("ratings_number"),p->i_ratings_number);
	for (int i = 0; i < p->i_ratings_number; i++)
	{
		Item_push_back_uint(leaf,("country_code"),p->p_parental_rating[i].i_country_code);
		Item_push_back_uint(leaf,("rating"),p->p_parental_rating[i].i_rating);
	}
	
	dr.dr_name = ("parental_rating_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpTeletextDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_teletext_dr_t* p = dvbpsi_DecodeTeletextDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("pages_number"),p->i_pages_number);
	for (int i = 0; i < p->i_pages_number; i++)
	{
		Item_push_back_hex(leaf,("iso6392_language_code"),p->p_pages[i].i_iso6392_language_code,sizeof(p->p_pages[i].i_iso6392_language_code));
		Item_push_back_uint(leaf,("teletext_type"),p->p_pages[i].i_teletext_type);
		Item_push_back_uint(leaf,("teletext_magazine_number"),p->p_pages[i].i_teletext_magazine_number);
		Item_push_back_uint(leaf,("teletext_page_number"),p->p_pages[i].i_teletext_page_number);
	}
	
	dr.dr_name = ("teletext_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpLocalTimeOffsetDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_local_time_offset_dr_t* p = dvbpsi_DecodeLocalTimeOffsetDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("local_time_offsets_number"),p->i_local_time_offsets_number);
	for (int i = 0; i < p->i_local_time_offsets_number; i++)
	{
		Item_push_back_hex(leaf,("country_code"),p->p_local_time_offset[i].i_country_code,sizeof(p->p_local_time_offset[i].i_country_code));
		Item_push_back_uint(leaf,("country_region_id"),p->p_local_time_offset[i].i_country_region_id);
		Item_push_back_uint(leaf,("local_time_offset_polarity"),p->p_local_time_offset[i].i_local_time_offset_polarity);
		Item_push_back_uint(leaf,("local_time_offset"),p->p_local_time_offset[i].i_local_time_offset);
		Item_push_back_uint(leaf,("time_of_change"),p->p_local_time_offset[i].i_time_of_change);
		Item_push_back_uint(leaf,("next_time_offset"),p->p_local_time_offset[i].i_next_time_offset);
	}
	
	dr.dr_name = ("localtime_offset_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpSubtitleDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_subtitling_dr_t* p = dvbpsi_DecodeSubtitlingDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("subtitles_number"),p->i_subtitles_number);
	for (int i = 0; i < p->i_subtitles_number;i++)
	{
		Item_push_back_hex(leaf,("iso6392_language_code"),p->p_subtitle[i].i_iso6392_language_code,sizeof(p->p_subtitle[i].i_iso6392_language_code));
		Item_push_back_uint(leaf,("subtitling_type"),p->p_subtitle[i].i_subtitling_type);
		Item_push_back_uint(leaf,("composition_page_id"),p->p_subtitle[i].i_composition_page_id);
		Item_push_back_uint(leaf,("ancillary_page_id"),p->p_subtitle[i].i_ancillary_page_id);
	}
	
	dr.dr_name = ("subtitle_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpTerrestrialDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_terr_deliv_sys_dr_t* p = dvbpsi_DecodeTerrDelivSysDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("centre_frequency"),p->i_centre_frequency);
	Item_push_back_uint(leaf,("bandwidth"),p->i_bandwidth);
	Item_push_back_uint(leaf,("priority"),p->i_priority);
	Item_push_back_uint(leaf,("time_slice_indicator"),p->i_time_slice_indicator);
	Item_push_back_uint(leaf,("mpe_fec_indicator"),p->i_mpe_fec_indicator);
	Item_push_back_uint(leaf,("constellation"),p->i_constellation);
	Item_push_back_uint(leaf,("hierarchy_information"),p->i_hierarchy_information);
	Item_push_back_uint(leaf,("code_rate_hp_stream"),p->i_code_rate_hp_stream);
	Item_push_back_uint(leaf,("code_rate_lp_stream"),p->i_code_rate_lp_stream);
	Item_push_back_uint(leaf,("guard_interval"),p->i_guard_interval);
	Item_push_back_uint(leaf,("transmission_mode"),p->i_transmission_mode);
	Item_push_back_uint(leaf,("other_frequency_flag"),p->i_other_frequency_flag);

	
	dr.dr_name = ("terrestrial_delivery_system_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpPDCDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_PDC_dr_t* p = dvbpsi_DecodePDCDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_hex(leaf,("PDC"),p->i_PDC,sizeof(p->i_PDC));
	
	dr.dr_name = ("pdc_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}

void CDescriptorImpl::DumpCUEIDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor)
{
	dvbpsi_cuei_dr_t* p = dvbpsi_DecodeCUEIDr(p_descriptor);

	if (p == NULL)
	{
		return;
	}

	DECODED_DESCRIPTOR dr_null;
	STRING_TREE_LEAF leaf_null;

	dr_null.str_tree.push_back(leaf_null);
	descriptor_n.push_back(dr_null);

	DECODED_DESCRIPTOR& dr = descriptor_n.back();
	STRING_TREE_LEAF& leaf = dr.str_tree.back();

	//DECODED_DESCRIPTOR dr;
	//STRING_TREE_LEAF leaf;

	Item_push_back_uint(leaf,("descriptor_tag"),p_descriptor->i_tag);
	Item_push_back_uint(leaf,("descriptor_length"),p_descriptor->i_length);

	Item_push_back_uint(leaf,("cue_stream_type"),p->i_cue_stream_type);
	
	dr.dr_name = ("cuei_descriptor");
	//dr.str_tree.push_back(leaf);
	//descriptor_n.push_back(dr);
}






