/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/




#pragma once

#include <map>
#include <vector>
#include <list>
#include <iostream>
#include "ztypes.h"
#include "json/json.h"

//共享内存能容纳的数据大小，MSG_PACKET_LIST结构个数
#define PACKET_LIST_FILE_MAPPING_SIZE					1000*1000

//进度条范围
#define PROGRESS_RANGE									100

//PacketListView 最大条目个数
//#define MAXITEM_PACKET_LIST	1000

//文件打开失败
#define ERROR_FILE_OPEN_ERROR									-1

//不是合法的MPEG-TS 文件
#define ERROR_FILE_NOT_MPEG_TS									-2

//读出大小与期望值不一致。认为文件过小，没有足够的数据进行分析。
#define ERROR_FILE_NOT_DATA										-3

//定义消息-------------------------------------------------------------------------





//定义消息所用数据结构--------------------------------------------------------------


typedef enum _AVMEDIATYPE {
    AVMEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
    AVMEDIA_TYPE_NB,
}AVMEDIATYPE;




//包类型
typedef enum _PACKET_TYPE
{
    PACKET_MPEG1_VIDEO,
    PACKET_MPEG2_VIDEO,
    PACKET_MPEG1_AUDIO,
    PACKET_MPEG2_AUDIO_PART_3,
    PACKET_AUDIO_AAC,
    PACKET_VIDEO_H264,
    PACKET_VIDEO_HEVC,
    PACKET_VIDEO_CAVS,
    PACKET_AUDIO_PCM_BLURAY,
    PACKET_AUDIO_AC3,
    PACKET_AUDIO_DTS,
    PACKET_AUDIO_TRUEHD,
    PACKET_AUDIO_EAC3,
    PACKET_VIDEO_DIRAC,
    PACKET_VIDEO_VC1,
    PACKET_MPEG4_VIDEO,
    PACKET_MPEG4_AUDIO,
    PACKET_HDMV_PGS_SUBTITLE,

    PACKET_MPEG2_PES_PRIVATE,
    PACKET_PCR,
    PACKET_PAT,
    PACKET_PMT,
    PACKET_CAT,
    PACKET_TSDT,
    PACKET_NIT_ST,
    PACKET_SDT_BAT_ST,
    PACKET_EIT_ST,
    PACKET_RST_ST,
    PACKET_TDT_TOT_ST,
    PACKET_DIT,
    PACKET_SIT,
    PACKET_NETSYNC,	//网络同步
    PACKET_NULL,	//空包
    PACKET_RESERVED, //保留
    PACKET_SIGN,		//带内信令
    PACKET_SURVEY,	//测量
    PACKET_UNKNOWN	//未知
}PACKET_TYPE;

typedef struct {
    int stream_type;
    AVMEDIATYPE codec_type;
    PACKET_TYPE packet_type;
} STREAMTYPE;

static const STREAMTYPE AllStreamType[] = {
    { 0x01, AVMEDIA_TYPE_VIDEO, PACKET_MPEG1_VIDEO },
    { 0x02, AVMEDIA_TYPE_VIDEO, PACKET_MPEG2_VIDEO },
    { 0x03, AVMEDIA_TYPE_AUDIO,        PACKET_MPEG1_AUDIO },
    { 0x04, AVMEDIA_TYPE_AUDIO,        PACKET_MPEG2_AUDIO_PART_3 },
    { 0x06, AVMEDIA_TYPE_DATA,        PACKET_MPEG2_PES_PRIVATE },
    { 0x0f, AVMEDIA_TYPE_AUDIO,        PACKET_AUDIO_AAC },
    { 0x10, AVMEDIA_TYPE_VIDEO,      PACKET_MPEG4_VIDEO },
    /* Makito encoder sets stream type 0x11 for AAC,
     * so auto-detect LOAS/LATM instead of hardcoding it. */

    { 0x11, AVMEDIA_TYPE_AUDIO,   PACKET_MPEG4_AUDIO }, /* LATM syntax */

    { 0x1b, AVMEDIA_TYPE_VIDEO,       PACKET_VIDEO_H264 },
    { 0x24, AVMEDIA_TYPE_VIDEO,       PACKET_VIDEO_HEVC },
    { 0x42, AVMEDIA_TYPE_VIDEO,       PACKET_VIDEO_CAVS },
    { 0xd1, AVMEDIA_TYPE_VIDEO,      PACKET_VIDEO_DIRAC },
    { 0xea, AVMEDIA_TYPE_VIDEO,        PACKET_VIDEO_VC1 },


    //HDMV_types
    { 0x80, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_PCM_BLURAY },
    { 0x81, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_AC3 },
    { 0x82, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_DTS },
    { 0x83, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_TRUEHD },
    { 0x84, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_EAC3 },
    { 0x85, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_DTS }, /* DTS HD */
    { 0x86, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_DTS }, /* DTS HD MASTER*/
    { 0xa1, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_EAC3 }, /* E-AC3 Secondary Audio */
    { 0xa2, AVMEDIA_TYPE_AUDIO, PACKET_AUDIO_DTS },  /* DTS Express Secondary Audio */
    { 0x90, AVMEDIA_TYPE_SUBTITLE, PACKET_HDMV_PGS_SUBTITLE },


    //MISC_types
    { 0x81, AVMEDIA_TYPE_AUDIO,   PACKET_AUDIO_AC3 },
    { 0x8a, AVMEDIA_TYPE_AUDIO,   PACKET_AUDIO_DTS },

    { 0 },
};

static const STREAMTYPE* GetStreamInfoByStreamType(int stream_type)
{
    const STREAMTYPE *p = &AllStreamType[0];
    while(p->stream_type != 0)
    {
        if (p->stream_type == stream_type)
            return p;
        p++;
    }
    return NULL;
}

static AVMEDIATYPE GetMediaTypeByPacketType(PACKET_TYPE type)
{
    const STREAMTYPE *p = &AllStreamType[0];
    while(p->stream_type != 0)
    {
        if (p->packet_type == type)
            return p->codec_type;
        p++;
    }
    return AVMEDIA_TYPE_UNKNOWN;
}


typedef struct _PACKET_TYPE_DES
{
    PACKET_TYPE		type;
    const char*	 descriptor;     /*      描述*/
} PACKET_TYPE_DES;

static PACKET_TYPE_DES packet_type_des[] =
{
    { PACKET_MPEG1_VIDEO,			"MPEG1 video" },
    { PACKET_MPEG2_VIDEO,			"MPEG2 video" },
    { PACKET_MPEG1_AUDIO,			"MPEG1 audio" },
    { PACKET_MPEG2_AUDIO_PART_3,	"MPEG2 audio part 3" },
    { PACKET_AUDIO_AAC,				"AAC" },
    //{ PACKET_AUDIO_A52,				"A52" },
    { PACKET_AUDIO_AC3,				"AC3" },
    { PACKET_VIDEO_H264,			"H.264" },
    { PACKET_VIDEO_HEVC,			"HEVC" },
    { PACKET_VIDEO_CAVS,			"CAVS" },
    { PACKET_AUDIO_PCM_BLURAY,		"PCM_BLURAY" },
    { PACKET_AUDIO_DTS,				"DTS" },
    { PACKET_AUDIO_TRUEHD,			"TRUEHD" },
    { PACKET_AUDIO_EAC3,			"EAC3" },
    { PACKET_VIDEO_DIRAC,			"DIRAC" },
    { PACKET_VIDEO_VC1,				"VC1" },
    { PACKET_HDMV_PGS_SUBTITLE,		"HDMV_PGS_SUBTITLE" },

    { PACKET_MPEG4_VIDEO,			"MPEG4 video" },
    { PACKET_MPEG4_AUDIO,			"MPEG4 audio" },
    { PACKET_MPEG2_PES_PRIVATE,		"MPEG2 pes private" },
    { PACKET_PCR,					"PCR" },
    { PACKET_PAT,					"PAT" },
    { PACKET_PMT,					"PMT" },
    { PACKET_CAT,					"CAT" },
    { PACKET_TSDT,					"TSDT" },
    { PACKET_NIT_ST,				"NIT" },
    { PACKET_SDT_BAT_ST,			"SDT/BAT" },
    { PACKET_EIT_ST,				"EIT" },
    { PACKET_RST_ST,				"RST" },
    { PACKET_TDT_TOT_ST,			"TDT/TOT" },
    { PACKET_DIT,					"DIT" },
    { PACKET_SIT,					"SIT" },
    { PACKET_NETSYNC,				"NEWWORK SYNC" },
    { PACKET_NULL,					"NULL" },
    { PACKET_RESERVED,				"RESERVED" },
    { PACKET_SIGN,					"SIGN" },
    { PACKET_SURVEY,				"SURVEY" },
    { PACKET_UNKNOWN,				"UnKnown" },
    { PACKET_UNKNOWN,	 NULL }
};

static const char* Get_PacketTypeDes(PACKET_TYPE type)
{
    PACKET_TYPE_DES *p = packet_type_des;
    while(p->descriptor)
    {
        if (p->type == type)
            return p->descriptor;
        p++;
    }
    return "UnKnown";
}


typedef enum _FRAME_TYPE
{
    FRAME_I	= 1,
    FRAME_P,
    FRAME_B,
    FRAME_IDR,
    FRAME_SP,
    FRAME_SI,
    FRAME_UNKNOWN,
    FRAME_NULL
}FRAME_TYPE;

typedef enum
{
    STRUCTURE_FRAME,
    STRUCTURE_TOP_FIELD,
    STRUCTURE_BOTTOM_FIELD
} PICTURESTRUCTURE;           //!< New enum for field processing

typedef struct _PARSED_FRAME_INFO
{
    _PARSED_FRAME_INFO()
    {
        bNewPicture = false;
        bNewSlice = false;
        FrameType = FRAME_NULL;
    }

    bool bNewSlice;
    bool bNewPicture;	//新的帧或场
    FRAME_TYPE FrameType;
    int structure;                     //!< Identify picture structure type
}PARSED_FRAME_INFO;


/*
Flags:
1 payload
1 b_new_pic
2 frame,or top or button
4 cc

*/

#pragma pack(1)
typedef struct _MSG_PACKET_LIST
{
    _MSG_PACKET_LIST()
    {
        //memset(SectionID,0,sizeof(SectionID));
        //memset(PCR,0,sizeof(PCR));
        //memset(Other,0,sizeof(Other));
        PCR = -1;
        PID = -1;
        Flags = 0;
        PacketType = PACKET_UNKNOWN;
        FrameType = FRAME_NULL;
        FrameNum = -1;
    }
    //__int64           offset;

    BYTE            Flags;
    WORD            PID;
    int             FrameNum;
    /*TCHAR         SectionID[5];*/
    //TCHAR         PCR[15];
    
    PACKET_TYPE     PacketType;
    FRAME_TYPE      FrameType;
    long long       PCR;
//  BYTE            Continuity_Counter;
    //TCHAR         Other[50];
}MSG_PACKET_LIST;
#pragma pack()



static void set_flags_payload(BYTE &flags)
{
    flags = flags | 0x80;
}

static int get_flags_payload(BYTE flags)
{
    return (flags & 0x80) >> 7;
}

static void set_flags_newpic(BYTE &flags)
{
    flags = flags | 0x40;
}

static int get_flags_newpic(BYTE flags)
{
    return ((flags & 0x40) >> 6 & 0x1);
}

static void set_flags_structure(int val,BYTE &flags)
{
    val &= 0x3;
    val = val << 4;
    flags = flags & 0xCF;
    flags |= val;
}

static int get_flags_structure(BYTE flags)
{
    return ((flags >> 4) & 0x3);
}

static void set_flags_cc(int val,BYTE &flags)
{
    flags &= 0xF0;
    flags |= val;
}

static int get_flags_cc(BYTE flags)
{
    return (flags & 0xF);
}



//pid list message data
typedef struct _MSG_PID_LIST
{
    WORD			PID;
    long long			total;
    float			percent;
    PACKET_TYPE		type;

	Json::Value to_json() {
		Json::Value root;
		root["PID"] = PID;
		root["total"] = total;
		root["percent"] = percent;
		root["type"] = Get_PacketTypeDes(type);
		return root;
    }
}MSG_PID_LIST;

typedef enum
{
    PAGE_PREV,
    PAGE_NEXT,
    PAGE_HOME,
    PAGE_END,
    PAGE_INIT
}PAGE_DIRECTION;

/*
   PacketList滚动消息
lastItem: 当前item，视图中显示的顶部或底部itemId
direction:指示向上还是向下请求,true表示向下，false表示向上。
*/
typedef struct _MSG_PACKET_LIST_PAGE
{
    ULONGLONG		lastPacketID;
    PAGE_DIRECTION			enDirection;
}MSG_PACKET_LIST_PAGE;

/*
   单击PacketList一个条目时所用消息数据结构
   */
typedef struct _MSG_PACKET_LIST_SELECT
{
    _MSG_PACKET_LIST_SELECT():bReload(false){}
    int				nItem;
    bool			bReload;
}MSG_PACKET_LIST_SELECT;

/*
   Flag列表显示所用结构
   */

typedef struct _TS_HEAD
{
    BYTE			sync_byte;
    BYTE			transport_error_indicator;
    BYTE			payload_unit_start_indicator;
    BYTE			transport_priority;
    WORD			PID;
    BYTE			transport_scrambling_control;
    BYTE			adaptation_field_control;
    BYTE			continuity_counter;
}TS_HEAD;

typedef struct _ADAPTATION_FIELD
{
    BYTE			b_exist;
    BYTE			adaptation_field_length;
    BYTE			discontinuity_indicator;
    BYTE			random_access_indicator;
    BYTE			elementary_stream_priority_indicator;
    BYTE			PCR_flag;
    BYTE			OPCR_flag;
    BYTE			splicing_point_flag;
    BYTE			transport_private_data_flag;
    BYTE			adaptation_field_extension_flag;
}ADAPTATION_FIELD;

typedef struct _PES_HEAD
{
    BYTE			b_exist;
    BYTE			stream_id;
    WORD			PES_packet_length;
    BYTE			PES_scrambling_control;
    BYTE			PES_priority;
    BYTE			data_alignment_indicator;
    BYTE			copyright;
    BYTE			orginal_or_copy;
    BYTE			PTS_DTS_flags;
    BYTE			ESCR_flag;
    BYTE			ES_rate_flag;
    BYTE			DSM_trick_mode_flag;
    BYTE			additional_copy_info_flag;
    BYTE			PES_CRC_flag;
    BYTE			PES_extension_flag;
    BYTE			PES_header_data_length;
}PES_HEAD;

typedef struct _MSG_FLAG_LIST
{
    TS_HEAD				ts_head;
    ADAPTATION_FIELD	adaptation_field;
    PES_HEAD			pes_head;
}MSG_FLAG_LIST;

//pid ,stream_type
typedef struct _PID_STREAM_TYPE
{
    int pid;
    int stream_type;
}PID_STREAM_TYPE;

//节目信息
typedef struct _PROGRAM_PIDS
{
    _PROGRAM_PIDS()
    {
        memset(descriptor,0,sizeof(descriptor));
    }
    std::vector<PID_STREAM_TYPE>	pids;	// pid
    char	 descriptor[50];     /*      描述*/
}PROGRAM_PIDS;


typedef struct _TIMESTAMP
{
    long long pos;
    long long		timestamp;

	Json::Value to_json() {
		Json::Value root;
		root["pos"] = pos;
		root["timestamp"] = timestamp;
		return root;
	}
}TIMESTAMP;

//一路节目的各种时间戳信息
typedef struct _PROGRAM_TIMESTAMPS
{
    //视频PTS
    std::vector<TIMESTAMP> vecVpts;

    //音频PTS <pid,timestamps>
    std::vector<TIMESTAMP> vecApts;
    //std::map<int,std::vector<TIMESTAMP>> mapApts;

    //DTS
    std::vector<TIMESTAMP> vecDts;

    //PCR
    std::vector<TIMESTAMP> vecPcr;

    //PTS - PCR
    std::vector<TIMESTAMP> vecPtsSub;

    //DTS - PCR
    std::vector<TIMESTAMP> vecDtsSub;

    //PTS - PCR Audio
    std::vector<TIMESTAMP> vecAPtsSub;

	Json::Value to_json() {
		Json::Value root;

		Json::Value vptsArray;
		for (size_t i = 0; i < vecVpts.size(); i++) {
			vptsArray.append(vecVpts[i].to_json());
		}
		root["Vpts"] = vptsArray;

		Json::Value aptsArray;
		for (size_t i = 0; i < vecApts.size(); i++) {
			aptsArray.append(vecApts[i].to_json());
		}
		root["Apts"] = aptsArray;

		Json::Value dtsArray;
		for (size_t i = 0; i < vecDts.size(); i++) {
			dtsArray.append(vecDts[i].to_json());
		}
		root["Dts"] = dtsArray;

		Json::Value pcrArray;
		for (size_t i = 0; i < vecPcr.size(); i++) {
			pcrArray.append(vecPcr[i].to_json());
		}
		root["Pcr"] = pcrArray;

		Json::Value ptsSubArray;
		for (size_t i = 0; i< vecPtsSub.size(); i++){
			ptsSubArray.append(vecPtsSub[i].to_json());
		}
		root["PtsSub"] = ptsSubArray;

		Json::Value dtsSubArray;
		for (size_t i = 0; i < vecDtsSub.size(); i++) {
			dtsSubArray.append(vecDtsSub[i].to_json());
		}
		root["DtsSub"] = dtsSubArray;

		Json::Value aptsSubArray;
		for (size_t i = 0; i < vecAPtsSub.size(); i++) {
			aptsArray.append(vecAPtsSub[i].to_json());
		}
		root["APtsSub"] = aptsSubArray;

		return root;
	}
}PROGRAM_TIMESTAMPS;

//GOPs
typedef struct _GOP_LIST
{
    std::string gop;
    unsigned long long pos;
    int gop_size;
    int gop_bytes;

	Json::Value to_json() {
        Json::Value root;
		root["gop"] = gop;
		root["pos"] = pos;
		root["gop_size"] = gop_size;
		root["gop_bytes"] = gop_bytes;
	    return root;
	}
}GOP_LIST;

//Rates
typedef struct _RATE_LIST
{
    double rate;
    unsigned long long pos;

	Json::Value to_json() {
		Json::Value root;
		// convert element to string
		root["rate"] = rate;
		root["pos"] = pos;
		return root;
	}
}RATE_LIST;

//一路节目的信息
typedef struct _PROGRAM_INFO
{
    //时间信息
    PROGRAM_TIMESTAMPS tts;
    std::vector<GOP_LIST> gopList;
    std::vector<RATE_LIST> rateList;

	Json::Value to_json() {
		Json::Value root;
		root["tts"] = tts.to_json();

		Json::Value gopArray;
		for (size_t i = 0; i< gopList.size(); i++) {
			gopArray.append(gopList[i].to_json());
		}
		root["gop_list"] = gopArray;

		Json::Value rateArray;
		for (size_t i = 0; i < rateList.size(); i++) {
			rateArray.append(rateList[i].to_json());
		}
		root["rate_list"] = rateArray;

		return root;
	}
}PROGRAM_INFO;

//所有节目的信息。节目号，节目信息
typedef std::map<int,PROGRAM_INFO*> ALL_PROGRAM_INFO;

typedef struct _PID_TYPE
{
    int pid;
    PACKET_TYPE type;

    Json::Value to_json() {
        Json::Value root;
        root["pid"] = pid;
        root["type"] = Get_PacketTypeDes(type);
        return root;
    }
}PID_TYPE;

//节目概要信息，包含每路节目的pid，以及pid类型描述
typedef std::map<int,std::vector<PID_TYPE> > ALL_PROGRAM_BRIEF;

//任务状态
typedef enum _TASK_STATUS
{
    STATUS_CHECKING_MEDIAINFO,
    STATUS_PREPARE_DATA,

    //for live
    STATUS_WAITTING_DATA,
    STATUS_WAITTING_SYNC,
    STATUS_ANALYSING,

    //for hls
    STATUS_WAITTING_RESPONSE,
    STATUS_WAITTING_FIRST_SEGMENT,//only for live
    STATUS_FINISH
}TASK_STATUS;

typedef struct _STRING_TREE_ITEM
{
    _STRING_TREE_ITEM()
    {
        exist = false;
    }
    std::string element;
    std::string value;
    bool exist;

	Json::Value to_string() {
		Json::Value root;
		root["element"] = element;
		root["value"] = value;
		root["exist"] = exist;
		return root;
	}
}STRING_TREE_ITEM;

typedef struct _STRING_TREE_LEAF
{
    _STRING_TREE_LEAF()
    {
        subset = NULL;
    }
    void push_back(const std::string& element,const std::string& value)
    {
        STRING_TREE_ITEM item;
        item.element = element;
        item.value = value;
        item.exist = true;
        lstItem.push_back(item);
    }

	Json::Value to_json() {
		Json::Value root;

		Json::Value itemArray;
		std::list<STRING_TREE_ITEM>::iterator it = lstItem.begin();
		for (; it != lstItem.end(); it++) {
			itemArray.append(it->to_string());
		}
		root["lstItem"] = itemArray;


		if (subset != NULL) {
			root["subset"] = subset->to_json();
		}
		return root;
	}

    std::list<STRING_TREE_ITEM> lstItem;
    _STRING_TREE_LEAF* subset;

}STRING_TREE_LEAF;


typedef std::list<STRING_TREE_LEAF> STRING_TREE;

typedef struct _DECODED_DESCRIPTOR
{
    _DECODED_DESCRIPTOR()
    {
        dr_name = "descriptor";
    }
    std::string dr_name;
    STRING_TREE str_tree;

	Json::Value to_json() {
		Json::Value root;
		root["dr_name"] = dr_name;

		Json::Value treeArray;
		std::list<STRING_TREE_LEAF>::iterator it = str_tree.begin();
		for (; it != str_tree.end(); it++) {
			treeArray.append(it->to_json());
		}
		root["str_tree"] = treeArray;

		return root;
	}
}DECODED_DESCRIPTOR;
//typedef STRING_TREE DECODED_DESCRIPTOR;
typedef std::list<DECODED_DESCRIPTOR> DECODED_DESCRIPTOR_N;


//live source
typedef enum _LIVE_SOURCE_TYPE_T
{
    LIVE_SOURCE_UDP,
    LIVE_SOURCE_RTP
}LIVE_SOURCE_TYPE_T;



typedef struct _PCR_INFO_T
{
    long long llPcr_Oj;//ms
    long long llPcr_Ac;//us 微秒
    long long llPcr_interval;//ms
    long long llTime;//us

    Json::Value to_json() {
        Json::Value root;
        root["llPcr_Oj"] = llPcr_Oj;
        root["llPcr_Ac"] = llPcr_Ac;
        root["llPcr_interval"] = llPcr_interval;
        root["llTime"] = llTime;
        return root;
    }
}PCR_INFO_T;

typedef std::list<PCR_INFO_T> LST_PCR_INFO_T;


typedef struct _RATE_INFO_T
{
    double fRate;// bit/s
    long long llTime;
    Json::Value to_json() {
        Json::Value root;
        root["fRate"] = fRate;
        root["llTime"] = llTime;
        return root;
    }
}RATE_INFO_T;

typedef std::list<RATE_INFO_T> LST_RATE_INFO_T;










