/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef STRING_RESOURCE_H
#define STRING_RESOURCE_H




static const char* IDS_TR_LV1="-----第 1 级-----";
static const char* IDS_TR_LV1_SYNC_LOST="同步丢失错误";
static const char* IDS_TR_LV1_SYNC_BYTE_ERR="同步字节错误";
static const char* IDS_TR_LV1_PAT_ERR="PAT 错误";
static const char* IDS_TR_LV1_CC_ERR="连续计数错误";
static const char* IDS_TR_LV1_PMT_ERR="PMT 错误";
static const char* IDS_TR_LV1_PID_ERR="PID 错误";
static const char* IDS_TR_LV2="-----第 2 级-----";
static const char* IDS_TR_LV2_TS_ERR="传输错误";
static const char* IDS_TR_LV2_CRC_ERR="CRC 错误";
static const char* IDS_TR_LV2_PCR_REPET_ERR="PCR 间隔错误";
static const char* IDS_TR_LV2_PCR_DISCON_ERR="PCR 不连续错误";
static const char* IDS_TR_LV2_PCR_AC_ERR="PCR 精度错误";
static const char* IDS_TR_LV2_PTS_ERR="PTS 错误";
static const char* IDS_TR_LV2_CAT_ERR="CAT 错误";
static const char* IDS_TR_LV3="-----第 3 级-----";
static const char* IDS_TR_LV3_NIT_ACT_ERR="NIT_actual 错误";
static const char* IDS_TR_LV3_NIT_OTHER_ERR="NIT_other 错误";
static const char* IDS_TR_LV3_SI_REPET_ERR="SI 表间隔错误";
static const char* IDS_TR_LV3_UNREFER_PID_ERR="关联 PID 错误";
static const char* IDS_TR_LV3_SDT_ACT_ERR="SDT_actual 错误";
static const char* IDS_TR_LV3_SDT_OTHER_ERR="SDT_other 错误";
static const char* IDS_TR_LV3_EIT_ACT_ERR="EIT_actual 错误";
static const char* IDS_TR_LV3_EIT_OTHER_ERR="EIT_other 错误";
static const char* IDS_TR_LV3_EIT_PF_ERR="EIT_PF 错误";
static const char* IDS_TR_LV3_RST_ERR="RST 错误";
static const char* IDS_TR_LV3_TDT_ERR="TDT 错误";
static const char* IDS_TR_LV1_PAT_ERR_OCC="PAT 间隔 错误";
static const char* IDS_TR_LV1_PAT_ERR_TID="PAT table_id 错误";
static const char* IDS_TR_LV1_PAT_ERR_SCF="PAT 加扰指示 错误";
static const char* IDS_TR_LV1_PMT_ERR_OCC="PMT 间隔 错误";
static const char* IDS_TR_LV1_PMT_ERR_SCF="PMT 加扰指示 错误";
static const char* IDS_TR_LV2_CRC_ERR_PAT="PAT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_CAT="CAT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_PMT="PMT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_NIT="NIT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_SDT="SDT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_BAT="BAT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_TOT="TOT CRC 错误";
static const char* IDS_TR_LV2_CRC_ERR_EIT="EIT CRC 错误";
static const char* IDS_TR_LV2_CAT_ERR_TID="CAT table_id 错误";
static const char* IDS_TR_LV3_NIT_ACT_ERR_TID="NIT_act table_id 错误";
static const char* IDS_TR_LV3_NIT_ACT_ERR_INT="NIT_act 间隔 错误";
static const char* IDS_TR_LV3_SDT_ACT_ERR_TID="SDT_act table_id 错误";
static const char* IDS_TR_LV3_SDT_ACT_ERR_INT="SDT_act 间隔 错误";
static const char* IDS_TR_LV3_EIT_ACT_ERR_TID="EIT_act table_id 错误";
static const char* IDS_TR_LV3_EIT_ACT_ERR_INT="EIT_act 间隔 错误";
static const char* IDS_TR_LV3_RST_ERR_TID="RST table_id 错误";
static const char* IDS_TR_LV3_RST_ERR_INT="RST 间隔 错误";
static const char* IDS_TR_LV3_TDT_ERR_TID="TDT table_id 错误";
static const char* IDS_TR_LV3_TDT_ERR_INT="TDT 间隔 错误";
static const char* IDS_TR_LV3_INT_BAT="TDT 间隔 错误";
static const char* IDS_TR_LV3_INT_EIT_SCHEDULE_ACT="EIT_schedule_act 间隔 错误";
static const char* IDS_TR_LV3_INT_EIT_SCHEDULE_OTHER="EIT_schedule_other 间隔 错误";
static const char* IDS_TR_LV3_INT_TOT="TOT 间隔 错误";
static const char* IDS_TR_DESC_PAT_OCC="PAT 间隔时间大于 0.5 秒";
static const char* IDS_TR_DESC_PAT_TID="PAT table_id 不等于 0";
static const char* IDS_TR_DESC_PAT_SCF="PAT 加扰指示不等于 0";
static const char* IDS_TR_DESC_PMT_OCC="PMT 间隔时间大于 0.5 秒";
static const char* IDS_TR_DESC_PMT_SCF="PMT 加扰指示不等于 0";
static const char* IDS_TR_DESC_PID="PID 没有及时到来 %d";
static const char* IDS_TR_DESC_PCR_REPETITION="PCR 间隔超过40ms,PID = %d";
static const char* IDS_TR_DESC_PCR_AC="PCR 精度误差超过正负 500ns,PID = %d";
static const char* IDS_TR_DESC_PTS="PTS 间隔超过 700ms,PID = %d";
static const char* IDS_TR_DESC_INT="间隔：%lld,PID = %d";
static const char* IDS_TR_LV1_BRIEF_SYNC_LOST="同步丢失错误,共 %d 次.\r\n";
static const char* IDS_TR_LV1_BRIEF_SYNC_BYTE_ERR="同步字节不等于 0x47,共 %d 次.\r\n";
static const char* IDS_TR_LV1_BRIEF_PAT_ERR="PAT 错误,共 %d 次.\r\n\t其中:\r\nPAT 间隔大于 0.5 秒,有 %d 次.\r\nPID=0 但 table_id 不等于 0,有 %d 次. \r\nPAT 的 TS 包加扰指示不等于 0,有 %d 次.\r\n";
static const char* IDS_TR_LV1_BRIEF_CC_ERR="连续计数错误,共 %d 次.\r\n";
static const char* IDS_TR_LV1_BRIEF_PMT_ERR="PMT 错误,共 %d 次.\r\n\t其中:\r\nPMT 间隔时间大于 0.5 秒,有 %d 次.\r\nPMT 的 TS 包加扰指示不等于 0,有 %d 次.\r\n";
static const char* IDS_TR_LV1_BRIEF_PID_ERR="PMT 关联的 PID 没有及时到达,有 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_TS_ERR="TS 包的传输错误指示为 1,有 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_CRC_ERR="CRC 错误,共 %d 次.\r\n\t其中:\r\nCRC 错误出现在 PAT 表中,有 %d 次.\r\nCRC 错误出现在 CAT 表中,有 %d 次.\r\nCRC 错误出现在 PMT 表中,有 %d 次.\r\nCRC 错误出现在 NIT 表中,有 %d 次.\r\nCRC 错误出现在 SDT 表中,有 %d 次.\r\nCRC 错误出现在 BAT 表中,有 %d 次.\r\nCRC 错误出现在 TOT 表中,有 %d 次.\r\nCRC 错误出现在 EIT 表中,有 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_REPET_ERR="PCR 间隔超过 40ms,共 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_DISCON_ERR="未设置不连续标志的情况下前后 PCR 差值大于 100ms,共 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_AC_ERR="PCR 精度误差超过正负 500ns,共 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_PTS_ERR="PTS 间隔超过 700ms,共 %d 次.\r\n";
static const char* IDS_TR_LV2_BRIEF_CAT_ERR="CAT 错误,共 %d 次.\r\n\t其中:\r\nPID=1 但 table_id 不等于 1,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_NIT_ACT_ERR="NIT_actual 错误,共 %d 次.\r\n\t其中:\r\ntable_id 不等于 0x40 或 0x41 或 0x72,有 %d 次.\r\nNIT_act 间隔超时,有 %d 次.\r\nNIT_act 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_NIT_OTHER_ERR="NIT_other 错误,共 %d 次.\r\n\t其中:\r\nNIT_other 间隔超时,有 %d 次.\r\nNIT_other 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_SI_REPET_ERR="SI 表间隔错误,共 %d 次.\r\n\t其中:\r\nSI                  section 间隔短于 25ms,有 %d 次.\r\nNIT                 section 间隔超时,有 %d 次.\r\nBAT                 section 间隔超时,有 %d 次.\r\nSDT_act             section 间隔超时,有 %d 次.\r\nSDT_other           section 间隔超时,有 %d 次.\r\nEIT_PF_act          section 间隔超时,有 %d 次.\r\nEIT_PF_other        section 间隔超时,有 %d 次.\r\nEIT_SCHEDULE_act    section 间隔超时,有 %d 次.\r\nEIT_SCHEDULE_other  section 间隔超时,有 %d 次.\r\nTDT                 section 间隔超时,有 %d 次.\r\nTOT                 section 间隔超时,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_UNREFER_PID_ERR="有 PID 在 0.5 秒还未被 PMT 和 CAT 关联,有 %d 次.";
static const char* IDS_TR_LV3_BRIEF_SDT_ACT_ERR="SDT_actual 错误,共 %d 次.\r\n\t其中:\r\nSDT_act 间隔超时,有 %d 次.\r\nPID=0x11 但 table_id 不等于 0x42,0x46,0x4A 或 0x72,有 %d 次.\r\nSDT_act 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_SDT_OTHER_ERR="SDT_other 错误,共 %d 次.\r\n\t其中:\r\nSDT_other 间隔超时,有 %d 次.\r\nSDT_other 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_ACT_ERR="EIT_actual 错误,共 %d 次.\r\n\t其中:\r\nEIT_PF_act 间隔超时,有 %d 次.\r\ntable_id 错误,有 %d 次.\r\nEIT_PF_act 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_OTHER_ERR="EIT_other 错误,共 %d 次.\r\n\t其中:\r\nEIT_PF_other 间隔超时,有 %d 次.\r\nEIT_PF_other 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_PF_ERR="EIT_P 或 F 仅出现一个,有 %d 次.";
static const char* IDS_TR_LV3_BRIEF_RST_ERR="RST 错误,共 %d 次\r\n\t其中:\r\ntable_id 错误,有 %d 次.\r\nRST section 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_TR_LV3_BRIEF_TDT_ERR="TDT 错误,共 %d 次\r\n\t其中:\r\nTDT 间隔超时,有 %d 次.\r\ntable_id 错误,有 %d 次.\r\nRST section 间隔短于 25ms,有 %d 次.\r\n";
static const char* IDS_HLS_TP_QUALITY="传输流监测";
static const char* IDS_HLS_TP_SEGMENT_QUALITY="切片质量";
static const char* IDS_HLS_TP_DIAGNOSIS="诊断";
static const char* IDS_HLS_TP_PROTOCOL="协议分析";
static const char* IDS_HLS_TP_PROTOCOL_HLS="HLS";
static const char* IDS_HLS_TP_PROTOCOL_HTTP="HTTP";
static const char* IDS_HLS_SQ_PATPMT="以 PAT,PMT 开始";
static const char* IDS_HLS_SQ_PATPMT_BRIEF="每个切片都应当以PAT和PMT为起始. \r\n";
static const char* IDS_HLS_SQ_COMPLETE_VF="完整的视频帧";
static const char* IDS_HLS_SQ_COMPLETE_VF_BRIEF="如果存在视频，切片最好包含完整的视频帧。切片时最好选择在帧边界\r\n";
static const char* IDS_HLS_SQ_COMPLETE_AF="完整的音频帧";
static const char* IDS_HLS_SQ_COMPLETE_AF_BRIEF="切片最好包含完整的音频帧。切片时最好选择在帧边界\r\n";
static const char* IDS_HLS_SQ_CC="TS 包连续性";
static const char* IDS_HLS_SQ_CC_BRIEF="相邻切片之间的TS包不应存在连续计数错 - 除非上一个切片在播放列表中有 EXT-X-DISCONTINUITY 标签\r\n\r\n";
static const char* IDS_HLS_SQ_TC="时间戳连续性";
static const char* IDS_HLS_SQ_TC_BRIEF="相邻切片之间的时间戳应该是连续的 - 除非上一个切片在播放列表中有 EXT-X-DISCONTINUITY 标签\r\n\r\n";
static const char* IDS_HLS_SQ_ONE_KEYFRAME="至少一个I帧";
static const char* IDS_HLS_SQ_ONE_KEYFRAME_BRIEF="如果存在视频，切片应该至少存在 一个 关键帧\r\n";
static const char* IDS_HLS_TP_GLOBAL="全局";
static const char* IDS_HLS_SQ_KEYFRAME_START="以关键帧开始";
static const char* IDS_HLS_SQ_KEYFRAME_START_BRIEF="如果存在视频，切片中出现的第一个帧最好是关键帧";
static const char* IDS_HLS_DIA_MSG_SPTS="切片必须是单节目流";
static const char* IDS_HLS_DIA_MSG_VIDEOTYPE="视频编码类型不符合HLS协议规范";
static const char* IDS_HLS_DIA_MSG_AUDIOTYPE="音频编码类型不符合HLS协议规范";
static const char* IDS_HLS_DIA_MSG_HAS_NULLPKT="码流存在空包";
static const char* IDS_HLS_DIA_MSG_HAS_HASBFRAME="存在B帧";
static const char* IDS_HLS_BUFFER_UNDERFLOW="下溢 (Last 1000)";
static const char* IDS_HLS_WAITTING_QUEUE="待下载";
static const char* IDS_HLS_DOWNLOAD_ERRORS="下载失败";
static const char* IDS_HLS_M3U8="M3U8";
static const char* IDS_HLS_DOWNLOAD_HISTORY="下载历史 (Last 1000)";
static const char* IDS_HLS_HTTP_RESPONSE_HEAD_M3U8="M3U8 响应头";
static const char* IDS_HLS_HTTP_RESPONSE_HEAD_SEGMENT="分片响应头";
static const char* IDS_HLS_RELEVANT_SEGMENT="相关分片 (Top 100)";

#endif


