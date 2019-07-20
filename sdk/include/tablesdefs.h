/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TABLESDEFS_H
#define TABLESDEFS_H

#include <vector>
#include <map>
#include <stdio.h>
#include "commondefs.h"
#include "ztypes.h"
#include "json/json.h"

namespace tables{
    using namespace std;

    typedef unsigned int u_int;
    typedef unsigned char u_char;
    typedef unsigned short u_short;
    typedef unsigned long long ul_long;
    typedef unsigned long u_long;


    typedef struct _BAT_LIST2 {
        u_int      transport_stream_id;
        u_int      original_network_id; 
        u_int      reserved_1;
        u_int      transport_descriptors_length;

        // N2 descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["transport_stream_id"] = transport_stream_id;
			root["original_network_id"] = original_network_id;
			root["reserved_1"] = reserved_1;
			root["transport_descriptors_length"] = transport_descriptors_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}
    } BAT_LIST2;

    typedef struct  _BAT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      bouquet_id;
        u_int      reserved_3;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      reserved_4;
        u_int      bouquet_descriptors_length;

        // N  descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        u_int      reserved_5;
        u_int      transport_stream_loop_length;

        // N1 BAT_LIST2
        vector<BAT_LIST2> vec_bat_list2;

        unsigned long crc;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["bouquet_id"] = bouquet_id;
			root["reserved_3"] = reserved_3;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["reserved_4"] = reserved_4;
			root["bouquet_descriptors_length"] = bouquet_descriptors_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			root["reserved_5"] = reserved_5;
			root["transport_stream_loop_length"] = transport_stream_loop_length;

			Json::Value batArray;
			for(size_t i = 0; i < vec_bat_list2.size(); i++) {
				batArray.append(vec_bat_list2[i].to_json());
			}
			root["transport_stream"] = batArray;

			return root;
		}
    } BAT;

    typedef struct  _CAT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      section_length;
        u_int      reserved_2;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;

        // private section
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        unsigned long CRC;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["section_length"] = section_length;
			root["reserved_2"] = reserved_2;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator; 
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number; 

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			root["CRC_32"] = (long long)CRC;

			return root;
		}
    } CAT;

    typedef struct  _DIT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      transition_flag;
        u_int      reserved_3;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["transition_flag"] = transition_flag;
			root["reserved_3"] = reserved_3;
			return root;
		}
    } DIT;

    typedef struct _EIT_LIST2 {
        u_int      event_id;
        u_long     start_time_MJD;  // 16
        u_long     start_time_UTC;  // 24
        u_long     duration;
        u_int      running_status;
        u_int      free_CA_mode;
        u_int      descriptors_loop_length;

        // N2 descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["event_id"] = event_id;
			root["start_time_MJD"] = (long long)start_time_MJD;
			root["start_time_UTC"] = (long long)start_time_UTC;
			root["duration"] = (long long)duration;
			root["running_status"] = running_status;
			root["free_CA_mode"] = free_CA_mode;
			root["descriptors_loop_length"] = descriptors_loop_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}
    } EIT_LIST2;

    typedef struct  _EIT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      service_id;
        u_int      reserved_3;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      transport_stream_id;
        u_int      original_network_id;
        u_int      segment_last_section_number;
        u_int      last_table_id;

        // N1 EIT_LIST2
        vector<EIT_LIST2> vec_eit_list2;

        unsigned long crc;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["service_id"] = service_id;
			root["reserved_3"] = reserved_3;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["transport_stream_id"] = transport_stream_id;
			root["original_network_id"] = original_network_id;
			root["segment_last_section_number"] = segment_last_section_number;
			root["last_table_id"] = last_table_id;

			Json::Value eitArray;
			for(size_t i = 0; i < vec_eit_list2.size(); i++) {
				eitArray.append(vec_eit_list2[i].to_json());
			}
			root["events"] = eitArray;

			root["CRC_32"] = (long long)crc;

			return root;
		}

    } EIT;

    typedef struct  _NIT_TSL {
        u_int      transport_stream_id;
        u_int      original_network_id;
        u_int      reserved_1;
        u_int      transport_descriptor_length;

        //  N2 ... descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["transport_stream_id"] = transport_stream_id;
			root["original_network_id"] = original_network_id;
			root["reserved_1"] = reserved_1;
			root["transport_descriptor_length"] = transport_descriptor_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}

    } NIT_TSL;

    typedef struct  _NIT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      network_id;
        u_int      reserved_3;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      reserved_4;
        u_int      network_descriptor_length;

        //  N ... descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        u_int      reserved_5;
        u_int      transport_stream_loop_length;

        //  N1 ... transport stream loop
        vector<NIT_TSL> vec_nit_tsl;

        u_long     CRC;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["network_id"] = network_id;
			root["reserved_3"] = reserved_3;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["reserved_4"] = reserved_4;
			root["network_descriptor_length"] = network_descriptor_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			root["reserved_5"] = reserved_5;
			root["transport_stream_loop_length"] = transport_stream_loop_length;

			Json::Value nitArray;
			for(size_t i = 0; i < vec_nit_tsl.size(); i++) {
				nitArray.append(vec_nit_tsl[i].to_json());
			}
			root["transport_stream"] = nitArray;

			root["CRC_32"] = (long long) CRC;
			return root;
		}
    } NIT;

    typedef struct _PAT_LIST {
        u_int      program_number;
        u_int      reserved;
        u_int      network_pmt_PID;

		Json::Value to_json(){
			Json::Value root;
			root["program_number"] = program_number;
			root["reserved"] = reserved;
			root["network_pmt_PID"] = network_pmt_PID;
			return root;
		}
    } PAT_LIST;

    typedef struct  _PAT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      section_length;
        u_int      transport_stream_id;
        u_int      reserved_2;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;

        // N pat_list
        vector<PAT_LIST> vec_pat_list;

        u_long     CRC;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["section_length"] = section_length;
			root["transport_stream_id"] = transport_stream_id;
			root["reserved_2"] = reserved_2;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;

			Json::Value patArray;
			for(size_t i = 0; i < vec_pat_list.size(); i++) {
				patArray.append(vec_pat_list[i].to_json());
			}
			root["programs"] = patArray;

			root["CRC_32"] = (long long)CRC;

			return root;
		}

    } PAT;

    typedef struct _PMT_LIST2 {
        u_int      stream_type;
        u_int      reserved_1; 
        u_int      elementary_PID;
        u_int      reserved_2;
        int        ES_info_length;

        // N2 descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["stream_type"] = stream_type;
			root["reserved_1"] = reserved_1;
			root["elementary_PID"] = elementary_PID;
			root["reserved_2"] = reserved_2;
			root["ES_info_length"] = ES_info_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}

    } PMT_LIST2;

    typedef struct  _PMT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      b_null;		
        u_int      reserved_1;
        int        section_length;
        u_int      program_number;
        u_int      reserved_2;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      reserved_3;
        u_int      pcr_pid;
        u_int      reserved_4;
        int        program_info_length;

        // N  descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        // N1 PMT_LIST2
        vector<PMT_LIST2> vec_pmt_list2;

        unsigned long crc;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["b_null"] = b_null;
			root["reserved_1"] = reserved_1;
			root["section_length"] = section_length;
			root["program_number"] = program_number;
			root["reserved_2"] = reserved_2;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["reserved_3"] = reserved_3;
			root["pcr_pid"] = pcr_pid;
			root["reserved_4"] = reserved_4;
			root["program_info_length"] = program_info_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			Json::Value pmtArray;
			for(size_t i = 0; i < vec_pmt_list2.size(); i++) {
				pmtArray.append(vec_pmt_list2[i].to_json());
			}
			root["components"] = pmtArray;

			root["CRC_32"] = (long long) crc;

			return root;
		}
    } PMT;

    typedef struct _RST_LIST2 {
        u_int      transport_stream_id;
        u_int      original_network_id; 
        u_int      service_id; 
        u_int      event_id; 
        u_int      reserved_1;
        u_int      running_status;

		Json::Value to_json(){
			Json::Value root;
			root["transport_stream_id"] = transport_stream_id;
			root["original_network_id"] = original_network_id;
			root["service_id"] = service_id;
			root["event_id"] = event_id;
			root["reserved_1"] = reserved_1;
			root["running_status"] = running_status;
			return root;
		}
    } RST_LIST2;

    typedef struct  _RST {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;

        // N1 RST_LIST2
        vector<RST_LIST2> vec_rst_list2;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;

			Json::Value rstArray;
			for(size_t i = 0; i < vec_rst_list2.size(); i++) {
				rstArray.append(vec_rst_list2[i].to_json());
			}
			root["transport_stream"] = rstArray;

			return root;
		}
    } RST;

    typedef struct _SDT_LIST {
        u_int      service_id; 
        u_int      reserved_1;
        u_int      EIT_schedule_flag; 
        u_int      EIT_present_following_flag; 
        u_int      running_status; 
        u_int      free_CA_mode; 
        u_int      descriptors_loop_length; 

        // N2 ... descriptors
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["service_id"] = service_id;
			root["reserved_1"] = reserved_1;
			root["EIT_schedule_flag"] = EIT_schedule_flag;
			root["EIT_present_following_flag"] = EIT_present_following_flag;
			root["running_status"] = running_status;
			root["free_CA_mode"] = free_CA_mode;
			root["descriptors_loop_length"] = descriptors_loop_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}

    } SDT_LIST;

    typedef struct  _SDT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      transport_stream_id;
        u_int      reserved_3;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      original_network_id;
        u_int      reserved_4;

        // N...  SDT_LIST
        vector<SDT_LIST> vec_sdt_list;

        u_long     CRC;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["transport_stream_id"] = transport_stream_id;
			root["reserved_3"] = reserved_3;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["original_network_id"] = original_network_id;
			root["reserved_4"] = reserved_4;

			Json::Value sdtArray;
			for(size_t i = 0; i < vec_sdt_list.size(); i++) {
				sdtArray.append(vec_sdt_list[i].to_json());
			}
			root["services"] = sdtArray;

			root["CRC_32"] = (long long)CRC;

			return root;
		}
    } SDT;

    typedef struct _SIT_LIST2 {
        u_int      service_id;
        u_int      reserved_1; 
        u_int      running_status;
        u_int      service_loop_length;

        // N2 descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

		Json::Value to_json(){
			Json::Value root;
			root["service_id"] = service_id;
			root["reserved_1"] = reserved_1;
			root["running_status"] = running_status;
			root["service_loop_length"] = service_loop_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			return root;
		}

    } SIT_LIST2;

    typedef struct  _SIT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      reserved_3;
        u_int      reserved_4;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;
        u_int      reserved_5;
        u_int      transmission_info_loop_length;

        // N  descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        // N1 SIT_LIST2
        vector<SIT_LIST2> vec_sit_list2;

        unsigned long crc;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["reserved_3"] = reserved_3;
			root["reserved_4"] = reserved_4;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;
			root["reserved_5"] = reserved_5;
			root["transmission_info_loop_length"] = transmission_info_loop_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			Json::Value sitArray;
			for(size_t i = 0; i < vec_sit_list2.size(); i++) {
				sitArray.append(vec_sit_list2[i].to_json());
			}
			root["services"] = sitArray;

			root["CRC_32"] = (long long) crc;

			return root;
		}
    } SIT;

    typedef struct  _ST {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;

        // N  databytes
        vector<u_char> vec_databytes;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			return root;
		}
    } ST;

    typedef struct _TDT {
        u_int      table_id;///8
        u_int      section_syntax_indicator;///1
        u_int      reserved_future_use;///1
        u_int      reserved;///2
        u_int      section_length;///12
        u_int      UTC_time_MJD;///16
        u_int      UTC_time_UTC;///24
        //tm		   stru_time;
        //time_t	   sec_time;
        //u_int     CRC_32;
		
		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_future_use"] = reserved_future_use;
			root["reserved"] = reserved;
			root["section_length"] = section_length;
			root["UTC_time_MJD"] = UTC_time_MJD;
			root["UTC_time_UTC"] = UTC_time_UTC;
			return root;
		}

    }TDT;

    typedef struct _TOT {
        u_int      table_id;///8
        u_int      section_syntax_indicator;///1
        u_int      reserved_future_use;///1
        u_int      reserved;///2
        u_int      section_length;///12
        u_int      UTC_time_MJD;///16
        u_int      UTC_time_UTC;///24

        u_int      reserved2;///4
        u_int      descriptors_loop_length;///12

        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        u_int     CRC_32;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_future_use"] = reserved_future_use;
			root["reserved"] = reserved;
			root["section_length"] = section_length;
			root["UTC_time_MJD"] = UTC_time_MJD;
			root["UTC_time_UTC"] = UTC_time_UTC;
			root["reserved2"] = reserved2;
			root["descriptors_loop_length"] = descriptors_loop_length;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			root["CRC_32_32"] = CRC_32;

			return root;
		}
    }TOT;

    typedef struct  _TSDT {
        u_int      table_id;
        u_int      section_syntax_indicator;		
        u_int      reserved_1;
        u_int      reserved_2;
        u_int      section_length;
        u_int      reserved_3;
        u_int      version_number;
        u_int      current_next_indicator;
        u_int      section_number;
        u_int      last_section_number;

        // N  descriptor
        vector<u_char> vec_descriptor;
        DECODED_DESCRIPTOR_N descriptors;

        u_long     crc;

		Json::Value to_json(){
			Json::Value root;
			root["table_id"] = table_id;
			root["section_syntax_indicator"] = section_syntax_indicator;
			root["reserved_1"] = reserved_1;
			root["reserved_2"] = reserved_2;
			root["section_length"] = section_length;
			root["reserved_3"] = reserved_3;
			root["version_number"] = version_number;
			root["current_next_indicator"] = current_next_indicator;
			root["section_number"] = section_number;
			root["last_section_number"] = last_section_number;

			Json::Value desArray;
			std::list<DECODED_DESCRIPTOR>::iterator it = descriptors.begin();
			for (; it != descriptors.end(); it++) {
				desArray.append(it->to_json());
			}
			root["descriptors"] = desArray;

			root["CRC_32"] = (long long)crc;

			return root;
		}
    } TSDT;




    //一个表的数据
    typedef vector<BAT> STU_SECTION_BAT;
    typedef vector<CAT> STU_SECTION_CAT;
    typedef vector<DIT> STU_SECTION_DIT;
    typedef vector<EIT> STU_SECTION_EIT;
    typedef vector<NIT> STU_SECTION_NIT;
    typedef vector<PAT> STU_SECTION_PAT;
    typedef vector<PMT> STU_SECTION_PMT;
    typedef vector<RST> STU_SECTION_RST;
    typedef vector<SDT> STU_SECTION_SDT;
    typedef vector<SIT> STU_SECTION_SIT;
    typedef vector<ST> STU_SECTION_ST;
    typedef vector<TDT> STU_SECTION_TDT;
    typedef vector<TOT> STU_SECTION_TOT;
    typedef vector<TSDT> STU_SECTION_TSDT;


    typedef struct _TABLES
    {
        void clear()
        {
            vecTabBAT.clear();
            vecTabCAT.clear();
            vecTabDIT.clear();
            vecTabEIT.clear();
            vecTabNIT.clear();
            vecTabPAT.clear();
            mapTabPMT.clear();
            vecTabRST.clear();
            vecTabSDT.clear();
            vecTabSIT.clear();
            vecTabST.clear();
            vecTabTDT.clear();
            vecTabTOT.clear();
            vecTabTSDT.clear();
        }
        vector<STU_SECTION_BAT> vecTabBAT;	//可能会有多个表,例如，多个PMT
        vector<STU_SECTION_CAT> vecTabCAT;
        vector<STU_SECTION_DIT> vecTabDIT;
        vector<STU_SECTION_EIT> vecTabEIT;
        vector<STU_SECTION_NIT> vecTabNIT;
        vector<STU_SECTION_PAT> vecTabPAT;
        map<int,STU_SECTION_PMT> mapTabPMT;//program_number,STU_SECTION_PMT
        vector<STU_SECTION_RST> vecTabRST;
        vector<STU_SECTION_SDT> vecTabSDT;
        vector<STU_SECTION_SIT> vecTabSIT;
        vector<STU_SECTION_ST> vecTabST;
        vector<STU_SECTION_TDT> vecTabTDT;
        vector<STU_SECTION_TOT> vecTabTOT;
        vector<STU_SECTION_TSDT> vecTabTSDT;
		Json::Value to_json(){
			Json::Value root;
			Json::Value SIMap;
			Json::Value PSIMap;

			Json::Value batArray;
			for (size_t i = 0; i < vecTabBAT.size(); i++) {
				Json::Value batsubArray;
				for (size_t j = 0; j < vecTabBAT[i].size(); j++) {
					batsubArray.append(vecTabBAT[i][j].to_json());
				}
				batArray.append(batsubArray);
			}
			SIMap["BAT"] = batArray;

			Json::Value catArray;
			for (size_t i = 0; i < vecTabCAT.size(); i++) {
				Json::Value catsubArray;
				for (size_t j = 0; j < vecTabCAT[i].size(); j++) {
					catsubArray.append(vecTabCAT[i][j].to_json());
				}
				catArray.append(catsubArray);
			}
			PSIMap["CAT"] = catArray;

			Json::Value ditArray;
			for (size_t i = 0; i < vecTabDIT.size(); i++) {
				Json::Value ditsubArray;
				for (size_t j = 0; j < vecTabDIT[i].size(); j++) {
					ditsubArray.append(vecTabDIT[i][j].to_json());
				}
				ditArray.append(ditsubArray);
			}
			SIMap["DIT"] = ditArray;

			Json::Value eitArray;
			for (size_t i = 0; i < vecTabEIT.size(); i++) {
				Json::Value eitsubArray;
				for (size_t j = 0; j < vecTabEIT[i].size(); j++) {
					eitsubArray.append(vecTabEIT[i][j].to_json());
				}
				eitArray.append(eitsubArray);
			}
			SIMap["EIT"] = eitArray;

			Json::Value nitArray;
			for (size_t i = 0; i < vecTabNIT.size(); i++) {
				Json::Value nitsubArray;
				for (size_t j = 0; j < vecTabNIT[i].size(); j++) {
					nitsubArray.append(vecTabNIT[i][j].to_json());
				}
				nitArray.append(nitsubArray);
			}
			PSIMap["NIT"] = nitArray;

			Json::Value patArray;
			for (size_t i = 0; i < vecTabPAT.size(); i++) {
				Json::Value patsubArray;
				for (size_t j = 0; j < vecTabPAT[i].size(); j++) {
					patsubArray.append(vecTabPAT[i][j].to_json());
				}
				patArray.append(patsubArray);
			}
			PSIMap["PAT"] = patArray;

			Json::Value pmtMap;
            map<int,STU_SECTION_PMT>::iterator it = mapTabPMT.begin();
			for(; it != mapTabPMT.end(); it++) {
				Json::Value pmtsubArray;
				char key[32];
				sprintf(key, "%d", it->first);
				for (size_t j = 0; j < it->second.size(); j++) {
					pmtsubArray.append(it->second[j].to_json());
				}
				pmtMap[key] = pmtsubArray;
			}
			PSIMap["PMT"] = pmtMap;

			Json::Value rstArray;
			for (size_t i = 0; i < vecTabRST.size(); i++) {
				Json::Value rstsubArray;
				for (size_t j = 0; j < vecTabRST[i].size(); j++) {
					rstsubArray.append(vecTabRST[i][j].to_json());
				}
				rstArray.append(rstsubArray);
			}
			SIMap["RST"] = rstArray;

			Json::Value sdtArray;
			for (size_t i = 0; i < vecTabSDT.size(); i++) {
				Json::Value sdtsubArray;
				for (size_t j = 0; j < vecTabSDT[i].size(); j++) {
					sdtsubArray.append(vecTabSDT[i][j].to_json());
				}
				sdtArray.append(sdtsubArray);
			}
			SIMap["SDT"] = sdtArray;

			Json::Value sitArray;
			for (size_t i = 0; i < vecTabSIT.size(); i++) {
				Json::Value sitsubArray;
				for (size_t j = 0; j < vecTabSIT[i].size(); j++) {
					sitsubArray.append(vecTabSIT[i][j].to_json());
				}
				sitArray.append(sitsubArray);
			}
			SIMap["SIT"] = sitArray;

			Json::Value stArray;
			for (size_t i = 0; i < vecTabST.size(); i++) {
				Json::Value stsubArray;
				for (size_t j = 0; j < vecTabST[i].size(); j++) {
					stsubArray.append(vecTabST[i][j].to_json());
				}
				stArray.append(stsubArray);
			}
			SIMap["ST"] = stArray;

			Json::Value tdtArray;
			for (size_t i = 0; i < vecTabTDT.size(); i++) {
				Json::Value tdtsubArray;
				for (size_t j = 0; j < vecTabTDT[i].size(); j++) {
					tdtsubArray.append(vecTabTDT[i][j].to_json());
				}
				tdtArray.append(tdtsubArray);
			}
			SIMap["TDT"] = tdtArray;

			Json::Value totArray;
			for (size_t i = 0; i < vecTabTOT.size(); i++) {
				Json::Value totsubArray;
				for (size_t j = 0; j < vecTabTOT[i].size(); j++) {
					totsubArray.append(vecTabTOT[i][j].to_json());
				}
				totArray.append(totsubArray);
			}
			SIMap["TOT"] = totArray;

			Json::Value tsdtArray;
			for (size_t i = 0; i < vecTabTSDT.size(); i++) {
				Json::Value tsdtsubArray;
				for (size_t j = 0; j < vecTabTSDT[i].size(); j++) {
					tsdtsubArray.append(vecTabTSDT[i][j].to_json());
				}
				tsdtArray.append(tsdtsubArray);
			}
			SIMap["TSDT"] = tsdtArray;

			root["SI"] = SIMap;
			root["PSI"] = PSIMap;
			return root;
		}
    }TABLES;

    //
    //typedef struct _SECTION_DATA
    //{
    //	_SECTION_DATA() : bFinish(false){}
    //
    //   vector<vector<BYTE>> vecSections;
    //
    //   //根据last_section_number判断是否已经组完
    //   bool bFinish;
    //}SECTION_DATA;
    //不准备支持表版本号更新的情况
    //在同一pid下，一个表传送完之前，认为不会有其他表的数据出现



    //一个section
    typedef struct _SECTION
    {
        _SECTION() : section_length(0){}
        int section_length;	//这是解析出来的应该是多长。
        vector<BYTE> vecData;
    }SECTION;

    typedef vector<SECTION> TABLE_SECTIONS;

    //某个表的section,一个完整独立的表
    typedef struct _TABLE_ID_SECTION
    {
        _TABLE_ID_SECTION() : bFinish(false){}
        int table_id;
        bool bFinish;	//根据last_section_number判断是否已经组完。如果全都解析效率太慢，尤其涉及到描述符解析
        TABLE_SECTIONS sections;
    }TABLE_ID_SECTION;


    //某个一个pid的section.
    //一个pid可能会包含多个表.PMT的多个表，也放到这里
    typedef vector<TABLE_ID_SECTION> PID_SECTIONS;

    typedef struct _PID_SECTION_BUILDING
    {
        _PID_SECTION_BUILDING()
        {
            curSectionIdx = 0;
        }

        TABLE_ID_SECTION* get_cur_table_id_section()
        {
            if (pid_sections.empty())
            {
                return NULL;
            }
            return &pid_sections[curSectionIdx];
        }
        int curSectionIdx;
        PID_SECTIONS pid_sections;
    }PID_SECTION_BUILDING;


    //根据pid组section 的缓冲
    typedef map<int,PID_SECTION_BUILDING> SECTION_BUFFER;

    typedef struct _TREAM_TYPE_DES
    {
        u_int    from;          /* e.g. from id 1  */
        u_int    to;            /*      to   id 3  */
        const char*	 descriptor;     /*      描述*/
    } TREAM_TYPE_DES;

    static TREAM_TYPE_DES tream_type_des[] = 
    {
        {  0x00, 0x00,  "reserved"	},
        {  0x01, 0x01,  "MPEG-1 video"	},
        {  0x02, 0x02,  "MPEG-2 video"	},
        {  0x03, 0x03,  "MPEG-1 audio"	},
        {  0x04, 0x04,  "MPEG-2 Part 3 audio"	},
        {  0x05, 0x05,  "MPEG-2 private sections"	},
        {  0x06, 0x06,  "MPEG-2 PES private"	},
        {  0x07, 0x07,  "ISO/IEC 13522 MHEG"	},
        {  0x08, 0x08,  "MPEG-2 DSM CC"	},
        {  0x09, 0x09,  "ITU-T H.222.1"	},
        {  0x0A, 0x0A,  "MPEG-2 DSM-CC Multi-protocol Encapsulation"	},

        {  0x0B, 0x0B,  "MPEG-2 DSM-CC U-N Messages"	},
        {  0x0C, 0x0C,  "MPEG-2 DSM-CC Stream Descriptors"	},

        {  0x0D, 0x0D,  "MPEG-2 DSM-CC Sections"	},
        {  0x0E, 0x0E,  "MPEG-2 auxiliary"	},

        {  0x0F, 0x0F,  "MPEG-2 Part 7 audio -> AAC"	},
        {  0x10, 0x10,  "MPEG-4 Part 2 visual" 	},

        {  0x11, 0x11,  "MPEG-4 audio -> AAC"	},
        {  0x12, 0x12,  "MPEG-4 SL-packetized stream or FlexMux stream carried in PES packets"	},

        {  0x13, 0x13,  "MPEG-4 SL-packetized stream or FlexMux stream carried in MPEG-4 sections"	},
        {  0x14, 0x14,  "MPEG-2 Synchronized Download Protocol"	},
        {  0x15, 0x15,  "Metadata carried in PES packets"	},
        {  0x16, 0x16,  "Metadata carried in metadata_sections"	},
        {  0x17, 0x17,  "Metadata carried in MPEG-2 Data Carousel"	},
        {  0x18, 0x18,  "Metadata carried in MPEG-2 Object Carousel"	},
        {  0x19, 0x19,  "Metadata carried in MPEG-2 Synchronized Download Protocol"	},
        {  0x1A, 0x1A,  "IPMP stream"	},
        {  0x1B, 0x1B,  "MPEG-4 Part 10 video -> H264"	},

        {  0x1C, 0x23,  "reserved"	},

        {  0x24, 0x24,  "HEVC"	},

        {  0x25, 0x41,  "reserved"	},

        {  0x42, 0x42,  "CAVS"	},

        {  0x43, 0x7F,  "reserved"	},

        {  0x80, 0x80,  "Audio PCM BLURAY"	},
        {  0x81, 0x81,  "Audio AC3"	},
        {  0x82, 0x82,  "Audio DTS"	},
        {  0x83, 0x83,  "Audio TRUEHD"	},
        {  0x84, 0x84,  "Audio AC3"	},
        {  0x85, 0x86,  "Audio DTS HD"	},
        {  0x87, 0x87,  "Audio AC3"	},
        {  0x8A, 0x8A,  "Audio DTS"	},
        {  0x90, 0x90,  "Subtitle"	},

        {  0xA1, 0xA1,  "Audio EAC3"	},
        {  0xA2, 0xA2,  "Audio DTS"	},
        {  0xD1, 0xD1,  "Video DIRAC"	},
        {  0xEA, 0xEA,  "Video VC1"	},

        {  0x80, 0xFF,  "user private"	},
        {  0xFF, 0,  "NULL"	}

    };

    static const char* GetStreamTypeDescriptor(u_int stream_type)
    {
        if (stream_type < 0 || stream_type > 0xFF)
            return "ERROR";

        TREAM_TYPE_DES *t = tream_type_des;
        while(t->from != 0xFF)
        {
            if (t->from <= stream_type && t->to >= stream_type)
                return t->descriptor;
            t++;
        }
        return "UNKNOWN";
    }

    static TREAM_TYPE_DES running_status_def[] =
    {
        {  0, 0,  "undefined"	},
        {  1, 1,  "not running"	},
        {  2, 2,  "starts in a few seconds"	},
        {  3, 3,  "pausing"	},
        {  4, 4,  "running"	},
        {  5, 7,  "reserved"	},
        {  0xFF, 0,  "NULL"	}

    };

    static const char* Get_running_status(u_int running_status)
    {
        if (running_status > 7)
            return "ERROR";

        TREAM_TYPE_DES *t = running_status_def;
        while(t->from != 0xFF)
        {
            if (t->from <= running_status && t->to >= running_status)
                return t->descriptor;
            t++;
        }
        return "UNKNOWN";
    }


}	//!namespace tables
#endif
