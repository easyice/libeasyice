/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef STRING_RESOURCE_H
#define STRING_RESOURCE_H
#include <iostream>


using namespace std;


static const char* IDS_TR_LV1="-----Priority 1-----";
static const char* IDS_TR_LV1_SYNC_LOST="TS_sync_loss";
static const char* IDS_TR_LV1_SYNC_BYTE_ERR="Sync_byte_error";
static const char* IDS_TR_LV1_PAT_ERR="PAT_error";
static const char* IDS_TR_LV1_CC_ERR="Continuity_count_error";
static const char* IDS_TR_LV1_PMT_ERR="PMT_error";
static const char* IDS_TR_LV1_PID_ERR="PID_error";
static const char* IDS_TR_LV2="-----Priority 2-----";
static const char* IDS_TR_LV2_TS_ERR="Transport_error";
static const char* IDS_TR_LV2_CRC_ERR="CRC_error";
static const char* IDS_TR_LV2_PCR_REPET_ERR="PCR_repetition_error";
static const char* IDS_TR_LV2_PCR_DISCON_ERR="PCR_discontinuity_indicator_error";
static const char* IDS_TR_LV2_PCR_AC_ERR="PCR_accuracy_error";
static const char* IDS_TR_LV2_PTS_ERR="PTS_error";
static const char* IDS_TR_LV2_CAT_ERR="CAT_error";
static const char* IDS_TR_LV3="-----Priority 3-----";
static const char* IDS_TR_LV3_NIT_ACT_ERR="NIT_actual_error";
static const char* IDS_TR_LV3_NIT_OTHER_ERR="NIT_other_error";
static const char* IDS_TR_LV3_SI_REPET_ERR="SI_repetition_error";
static const char* IDS_TR_LV3_UNREFER_PID_ERR="Unreferenced_PID";
static const char* IDS_TR_LV3_SDT_ACT_ERR="SDT_actual_error";
static const char* IDS_TR_LV3_SDT_OTHER_ERR="SDT_other_error";
static const char* IDS_TR_LV3_EIT_ACT_ERR="EIT_actual_error";
static const char* IDS_TR_LV3_EIT_OTHER_ERR="EIT_other_error";
static const char* IDS_TR_LV3_EIT_PF_ERR="EIT_PF_error";
static const char* IDS_TR_LV3_RST_ERR="RST_error";
static const char* IDS_TR_LV3_TDT_ERR="TDT_error";
static const char* IDS_TR_LV1_PAT_ERR_OCC="PAT Repetition Error";
static const char* IDS_TR_LV1_PAT_ERR_TID="PAT table_id Error";
static const char* IDS_TR_LV1_PAT_ERR_SCF="PAT scf Error";
static const char* IDS_TR_LV1_PMT_ERR_OCC="PMT Repetition Error";
static const char* IDS_TR_LV1_PMT_ERR_SCF="PMT scf Error";
static const char* IDS_TR_LV2_CRC_ERR_PAT="PAT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_CAT="CAT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_PMT="PMT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_NIT="NIT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_SDT="SDT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_BAT="BAT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_TOT="TOT CRC Error";
static const char* IDS_TR_LV2_CRC_ERR_EIT="EIT CRC Error";
static const char* IDS_TR_LV2_CAT_ERR_TID="CAT table_id Error";
static const char* IDS_TR_LV3_NIT_ACT_ERR_TID="NIT_act table_id Error";
static const char* IDS_TR_LV3_NIT_ACT_ERR_INT="NIT_act Repetition Error";
static const char* IDS_TR_LV3_SDT_ACT_ERR_TID="SDT_act table_id Error";
static const char* IDS_TR_LV3_SDT_ACT_ERR_INT="SDT_act Repetition Error";
static const char* IDS_TR_LV3_EIT_ACT_ERR_TID="EIT_act table_id Error";
static const char* IDS_TR_LV3_EIT_ACT_ERR_INT="EIT_act Repetition Error";
static const char* IDS_TR_LV3_RST_ERR_TID="RST table_id Error";
static const char* IDS_TR_LV3_RST_ERR_INT="RST Repetition Error";
static const char* IDS_TR_LV3_TDT_ERR_TID="TDT table_id Error";
static const char* IDS_TR_LV3_TDT_ERR_INT="TDT Repetition Error";
static const char* IDS_TR_LV3_INT_BAT="TDT Repetition Error";
static const char* IDS_TR_LV3_INT_EIT_SCHEDULE_ACT="EIT_schedule_act Repetition Error";
static const char* IDS_TR_LV3_INT_EIT_SCHEDULE_OTHER="EIT_schedule_other Repetition Error";
static const char* IDS_TR_LV3_INT_TOT="TOT Repetition Error";
static const char* IDS_TR_DESC_PAT_OCC="PAT does not occur at least every 0,5 s";
static const char* IDS_TR_DESC_PAT_TID="PAT table_id not equal 0";
static const char* IDS_TR_DESC_PAT_SCF="Scrambling_control_field is not 0 for PAT";
static const char* IDS_TR_DESC_PMT_OCC="PMT does not occur at least every 0,5 s";
static const char* IDS_TR_DESC_PMT_SCF="Scrambling_control_field is not 0 for PMT";
static const char* IDS_TR_DESC_PID="PID does not occur for period PID = %d";
static const char* IDS_TR_DESC_PCR_REPETITION="PCR interval more than 40 ms,PID = %d";
static const char* IDS_TR_DESC_PCR_AC="PCR accuracy is not within ¡À500 ns,PID = %d";
static const char* IDS_TR_DESC_PTS="PTS repetition period more than 700 ms,PID = %d";
static const char* IDS_TR_DESC_INT="repetition:%I64d ms,PID = %d";
static const char* IDS_TR_LV1_BRIEF_SYNC_LOST="Loss of synchronization,count %d.\r\n";
static const char* IDS_TR_LV1_BRIEF_SYNC_BYTE_ERR="Sync_byte not equal 0x47,count %d.\r\n";
static const char* IDS_TR_LV1_BRIEF_PAT_ERR="PAT Error,count %d.\r\n\t Thereinto:\r\nSections with table_id 0 do not occur at least every 0,5 s on PID 0,count %d.\r\nSection with table_id other than 0 found on PID 0,count %d. \r\nScrambling_control_field is not 0 for PID 0,count %d.\r\n";
static const char* IDS_TR_LV1_BRIEF_CC_ERR="Incorrect packet order a packet occurs more than twice lost packet,count %d.\r\n";
static const char* IDS_TR_LV1_BRIEF_PMT_ERR="PMT Error,count %d.\r\n\t Thereinto:\r\nPMT do not occur at least every 0,5 s,count %d.\r\nScrambling_control_field is not 0 for PMT,count %d.\r\n";
static const char* IDS_TR_LV1_BRIEF_PID_ERR="Referred PID does not occur for a user specified period,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_TS_ERR="Transport_error_indicator in the TS-Header is set to 1,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_CRC_ERR="CRC Error,count %d.\r\n\t Thereinto:\r\nCRC error occurred in PAT table,count %d.\r\nCRC error occurred in CAT table,count %d.\r\nCRC error occurred in PMT table,count %d.\r\nCRC error occurred in NIT table,count %d.\r\nCRC error occurred in SDT table,count %d.\r\nCRC error occurred in BAT table,count %d.\r\nCRC error occurred in TOT table,count %d.\r\nCRC error occurred in EIT table,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_REPET_ERR="Time interval between two consecutive PCR values more than 40 ms,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_DISCON_ERR="The difference between two consecutive PCR values more than 40 ms withoutthe discontinuity_ indicator set,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_PCR_AC_ERR="PCR accuracy of selected programme is not within ¡À500 ns,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_PTS_ERR="PTS repetition period more than 700 ms,count %d.\r\n";
static const char* IDS_TR_LV2_BRIEF_CAT_ERR="CAT Error,count %d.\r\n\t Thereinto:\r\nSection with table_id other than 1 found on PID 1,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_NIT_ACT_ERR="NIT_actual Error,count %d.\r\n\t Thereinto:\r\nSection with table_id other than 0x40 or 0x41 or 0x72 found on PID 0x0010,count %d.\r\nNIT_act Repetition timeout,count %d.\r\nNIT_act Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_NIT_OTHER_ERR="NIT_other Error,count %d.\r\n\t Thereinto:\r\nNIT_other Repetition timeout,count %d.\r\nNIT_other Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_SI_REPET_ERR="SI section Repetition error,count %d.\r\n\t Thereinto:\r\nSI                 section Repetition less than 25ms,count %d.\r\nNIT                section Repetition timeout,count %d.\r\nBAT                section Repetition timeout,count %d.\r\nSDT_act            section Repetition timeout,count %d.\r\nSDT_other          section Repetition timeout,count %d.\r\nEIT_PF_act         section Repetition timeout,count %d.\r\nEIT_PF_other       section Repetition timeout,count %d.\r\nEIT_SCHEDULE_act   section Repetition timeout,count %d.\r\nEIT_SCHEDULE_other section Repetition timeout,count %d.\r\nTDT                section Repetition timeout,count %d.\r\nTOT                section Repetition timeout,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_UNREFER_PID_ERR="PID not referred to by a PMT within 0,5 s,count %d.";
static const char* IDS_TR_LV3_BRIEF_SDT_ACT_ERR="SDT_actual Error,count %d.\r\n\t Thereinto:\r\nSDT_act Repetition timeout,count %d.\r\nPID=0x11 but table_id not equal 0x42,0x46,0x4A or 0x72,count %d.\r\nSDT_act Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_SDT_OTHER_ERR="SDT_other Error,count %d.\r\n\t Thereinto:\r\nSDT_other Repetition timeout,count %d.\r\nSDT_other Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_ACT_ERR="EIT_actual Error,count %d.\r\n\t Thereinto:\r\nEIT_PF_act Repetition timeout,count %d.\r\ntable_id Error,count %d.\r\nEIT_PF_act Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_OTHER_ERR="EIT_other Error,count %d.\r\n\t Thereinto:\r\nEIT_PF_other Repetition timeout,count %d.\r\nEIT_PF_other Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_EIT_PF_ERR="EIT_P or F only present one,count %d.";
static const char* IDS_TR_LV3_BRIEF_RST_ERR="RST Error,count %d\r\n\t Thereinto:\r\ntable_id Error,count %d.\r\nRST section Repetition less than 25ms,count %d.\r\n";
static const char* IDS_TR_LV3_BRIEF_TDT_ERR="TDT Error,count %d\r\n\t Thereinto:\r\nTDT Repetition timeout,count %d.\r\ntable_id Error,count %d.\r\nRST section Repetition less than 25ms,count %d.\r\n";
static const char* IDS_HLS_TP_QUALITY="TS Quality";
static const char* IDS_HLS_TP_SEGMENT_QUALITY="Segment Quality";
static const char* IDS_HLS_TP_DIAGNOSIS="Diagnosis";
static const char* IDS_HLS_TP_PROTOCOL="Protocol Analysis";
static const char* IDS_HLS_TP_PROTOCOL_HLS="HLS";
static const char* IDS_HLS_TP_PROTOCOL_HTTP="HTTP";
static const char* IDS_HLS_SQ_PATPMT="Start With PAT,PMT";
static const char* IDS_HLS_SQ_PATPMT_BRIEF="There SHOULD be a Program Association Table (PAT) and a Program Map Table (PMT) at the start of each segment. \r\n";
static const char* IDS_HLS_SQ_COMPLETE_VF="Complete Video Frame";
static const char* IDS_HLS_SQ_COMPLETE_VF_BRIEF="Complete Video Frame  at segment  or end\r\n";
static const char* IDS_HLS_SQ_COMPLETE_AF="Complete Audio Frame";
static const char* IDS_HLS_SQ_COMPLETE_AF_BRIEF="Complete Audio Frame at segment  or end\r\n";
static const char* IDS_HLS_SQ_CC="TS Packet Continuity";
static const char* IDS_HLS_SQ_CC_BRIEF="A Transport Stream or audio elementary stream segment MUST be the continuation of the encoded media at the end of the segment with the previous sequence number, where values in a continuous series, such as timestamps and Continuity Counters, continue uninterrupted - unless the media segment was the first ever to appear in the Playlist file or has an EXT-X-DISCONTINUITY tag applied to it.\r\n\r\n";
static const char* IDS_HLS_SQ_TC="Time Stamp Continuity";
static const char* IDS_HLS_SQ_TC_BRIEF="A Transport Stream or audio elementary stream segment MUST be the continuation of the encoded media at the end of the segment with the previous sequence number, where values in a continuous series, such as timestamps and Continuity Counters, continue uninterrupted - unless the media segment was the first ever to appear in the Playlist file or has an EXT-X-DISCONTINUITY tag applied to it.\r\n\r\n";
static const char* IDS_HLS_SQ_ONE_KEYFRAME="At Lest One Key Frame";
static const char* IDS_HLS_SQ_ONE_KEYFRAME_BRIEF="A segment that contains video SHOULD have at least one key frame and enough information to completely initialize a video decoder.\r\n";
static const char* IDS_HLS_TP_GLOBAL="Global";
static const char* IDS_HLS_SQ_KEYFRAME_START="Start With KeyFrame";
static const char* IDS_HLS_SQ_KEYFRAME_START_BRIEF="A segment that contains video MAY Start With KeyFrame for decoder faster";
static const char* IDS_HLS_DIA_MSG_SPTS="Segments MUST contain a single MPEG-2 Program";
static const char* IDS_HLS_DIA_MSG_VIDEOTYPE="Video codec type not follow the HLS specification";
static const char* IDS_HLS_DIA_MSG_AUDIOTYPE="Audio codec type not follow the HLS specification";
static const char* IDS_HLS_DIA_MSG_HAS_NULLPKT="NULL Packet(pid=\"0x1FFF\") exist";
static const char* IDS_HLS_DIA_MSG_HAS_HASBFRAME="B frame exist";
static const char* IDS_HLS_BUFFER_UNDERFLOW="Underflow (Last 1000)";
static const char* IDS_HLS_WAITTING_QUEUE="Waitting Queue";
static const char* IDS_HLS_DOWNLOAD_ERRORS="Download Failed";
static const char* IDS_HLS_M3U8="M3U8";
static const char* IDS_HLS_DOWNLOAD_HISTORY="Download History (Last 1000)";
static const char* IDS_HLS_HTTP_RESPONSE_HEAD_M3U8="HTTP Response Header M3U8";
static const char* IDS_HLS_HTTP_RESPONSE_HEAD_SEGMENT="HTTP Response Header Segment";
static const char* IDS_HLS_RELEVANT_SEGMENT="Relevant Segment (Top 100)";

#endif


