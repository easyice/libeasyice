/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CDESCRIPTORIMPL_H
#define CDESCRIPTORIMPL_H

#include "tablesdefs.h"



typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int  uint32_t;
typedef unsigned long long uint64_t;



extern "C"
{
	#include <dvbpsi/dvbpsi.h>
	#include <dvbpsi/psi.h>
	#include <dvbpsi/pat.h>
	#include <dvbpsi/descriptor.h>
	#include <dvbpsi/demux.h>
	#include <dvbpsi/pmt.h>
	#include <dvbpsi/dr.h>
}





#define TAG_MAX 256


class CDescriptorImpl
{
	typedef void (*DECODE_DR_FUN)(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
public:
	
	~CDescriptorImpl(void);
	CDescriptorImpl(void);

	void DecodeDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,BYTE* pData,int nLen);
private:
	dvbpsi_descriptor_t* dvbpsi_AddDescriptor(dvbpsi_descriptor_t ** p_first_descriptor_in,
												 uint8_t i_tag, uint8_t i_length,
												 uint8_t* p_data);
	void DumpDescriptors(DECODED_DESCRIPTOR_N& descriptor_n, dvbpsi_descriptor_t* p_descriptor);

	static inline void Item_push_back_uint(STRING_TREE_LEAF& leaf,char* name,uint64_t val);
	static inline void Item_push_back_int64(STRING_TREE_LEAF& leaf,char* name,long long val);
	static inline void Item_push_back_str(STRING_TREE_LEAF& leaf,char* name,uint8_t* data,uint32_t len);
	static inline void Item_push_back_hex(STRING_TREE_LEAF& leaf,char* name,uint8_t* data,uint32_t len);

	//default
	static void DumpDescriptorDefault(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);

	//0x02-0x0F
	static void DumpVideoStreamDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpAudioStreamDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpHierarchyDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpRegistrationDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpDataStreamAlignmentDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpTargetBackgroundGridDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpVideoWindowDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpCADescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpISO_639LanguageDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpSystemClockDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpMultiplexBufferUtilizationDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpCopyrightDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpMaxBitrateDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpPrivateDataIndicatorDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);

	//0x42-0x4E
	static void DumpStuffingDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpSatelliteDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpCableDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpVBITeletextDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpBouquetNameDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpServiceDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpShortEventDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpExtendedEventDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);

	//0x52-
	static void DumpStreamIdentifierDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpParentalRatingDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpTeletextDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpLocalTimeOffsetDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpSubtitleDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpTerrestrialDeliverySystemDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpPDCDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);
	static void DumpCUEIDescriptor(DECODED_DESCRIPTOR_N& descriptor_n,dvbpsi_descriptor_t* p_descriptor);


	
	
private:
	DECODE_DR_FUN m_pDecodefunc[TAG_MAX];



};






#endif

