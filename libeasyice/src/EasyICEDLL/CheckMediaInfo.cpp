/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include "CheckMediaInfo.h"
#include "rapidjson/document.h"
#include <stdio.h>
#include <sstream>
#include "EiLog.h"


using namespace rapidjson;



static string parseProbeInfoStream(const string& probe_json, const string& item){
    string res;
    Document document;
    document.Parse(probe_json.c_str());
    if(!document.IsObject()){
        cerr<<"Not illegal json: "<<probe_json;
        return res;
    }

    Value::ConstMemberIterator iter;

    if ((iter = document.FindMember("streams")) != document.MemberEnd() && iter->value.IsArray())
    {
        Value::ConstValueIterator iter2 = iter->value.Begin();
        if ( iter2 == iter->value.End()){
            return res;
        }
        Value::ConstMemberIterator iter3;
        if ((iter3 = iter2->FindMember(item.c_str())) != iter2->MemberEnd() && iter3->value.IsString())
        {
            return iter3->value.GetString();
        }else{
            return res;
        }
    }
    return res;
}

static string parseProbeInfoFormat(const string& probe_json, const string& item){
    string res;  
    Document document;
    document.Parse(probe_json.c_str());
    if(!document.IsObject()){
        cerr<<"Not illegal json: "<<probe_json;
        return res;
    }         

    Value::ConstMemberIterator iter;

    if ((iter = document.FindMember("format")) != document.MemberEnd())
    {

        Value::ConstMemberIterator iter2;
        if ((iter2 = iter->value.FindMember(item.c_str())) != iter->value.MemberEnd() && iter2->value.IsString())
        {
            return iter2->value.GetString();
        }else{
            return res;
        }
    }
    return res;
}

static string ffprobe_fps(const string& mrl)
{
    string cmd = "ffprobe -v quiet -show_entries stream=avg_frame_rate -select_streams v -of json "+mrl;

    char buffer[1024];

    FILE* fp = popen(cmd.c_str(),"r");

    if(fp == NULL)
    {
        cerr << "exec ffprobe cmd faild, mypopen return error : " <<endl;
        return "";
    }
    stringstream ss;

    while(fgets(buffer, 1024, fp) != NULL)
    {
        ss<<buffer;
    }
    pclose(fp);
    return parseProbeInfoStream(ss.str(), "avg_frame_rate");
}

static string ffprobe_dur(const string& mrl)
{
    string cmd = "ffprobe -v quiet -show_entries format=duration -select_streams v -of json "+mrl;

    char buffer[1024];

    FILE* fp = popen(cmd.c_str(),"r");

    if(fp == NULL)
    {             
        cerr << "exec ffprobe cmd faild, mypopen return error : " <<endl;
        return "";
    }             
    stringstream ss;

    while(fgets(buffer, 1024, fp) != NULL)
    {             
        ss<<buffer;
    }             
    pclose(fp);   
    return parseProbeInfoFormat(ss.str(), "duration");
}

static string ffprobe_rate(const string& mrl)
{   
    string cmd = "ffprobe -v quiet -show_entries format=bit_rate -select_streams v -of json "+mrl;

    char buffer[1024];

    FILE* fp = popen(cmd.c_str(),"r");

    if(fp == NULL) 
    {              
        cerr << "exec ffprobe cmd faild, mypopen return error : " <<endl;
        return ""; 
    }              
    stringstream ss;

    while(fgets(buffer, 1024, fp) != NULL)
    {             
        ss<<buffer;
    }

    pclose(fp);   
    return parseProbeInfoFormat(ss.str(), "bit_rate");
}

CEiMediaInfo::CEiMediaInfo()
{
    m_nBufferLen = 1024*512;
    m_pBuffer = new char[m_nBufferLen];
}

CEiMediaInfo::~CEiMediaInfo()
{
    delete [] m_pBuffer;
}

MediaInfoDLL::String CEiMediaInfo::CheckMediaInfo(BYTE* pData,int length,int nTsLength)
{
    using namespace MediaInfoDLL;

    //Initilaizing MediaInfo
    MediaInfo MI;
    MI.Open_Buffer_Init(length, 0);

    int pos = 0;
    int slice = nTsLength;
    if (slice < length)
    {
        slice = length;
    }
    do
    {
        if (MI.Open_Buffer_Continue(pData+pos, slice) == 0)
            break;

        //Testing if MediaInfo request to go elsewhere
        if (MI.Open_Buffer_Continue_GoTo_Get()!=-1)
        {
            pos = MI.Open_Buffer_Continue_GoTo_Get();	////Position the buffer
            MI.Open_Buffer_Init(length, pos);  //Informaing MediaInfo we have seek
        }
        pos += slice;
    }while(pos < length);

    //Finalizing
    MI.Open_Buffer_Finalize(); //This is the end of the stream, MediaInfo must finnish some work

    //Inform with Complete=false
    //MI.Open(pathname);
    String To_Display;
    MI.Option("Complete");
    To_Display += MI.Inform().c_str();


    To_Display += "\r\n\r\nVideo StreamCount: ";
    To_Display += MI.Get(Stream_Video, 0, "StreamCount", Info_Text, Info_Name).c_str();

    To_Display += "\r\n\r\nAudio StreamCount: ";
    To_Display += MI.Get(Stream_Audio, 0, "StreamCount", Info_Text, Info_Name).c_str();

    MI.Close();

    return To_Display;
}

void CEiMediaInfo::CheckMediaInfo(BYTE* pData,int length,int nTsLength,EI_MEDIAINFO_T* pEmi)
{
    using namespace MediaInfoDLL;
    pEmi->vecAStream.clear();
    pEmi->vecVStream.clear();


    //Initilaizing MediaInfo
    MediaInfo MI;
    //MediaInfo::Option("Internet"), "No"));
    MI.Option("Internet", "No");

    ////
    MI.Open_Buffer_Init(length, 0);

    int pos = 0;
    int slice = nTsLength;
    if (slice < length)
    {
        slice = length;
    }
    do
    {
        if (MI.Open_Buffer_Continue(pData+pos, slice) == 0)
            break;

        //Testing if MediaInfo request to go elsewhere
        if (MI.Open_Buffer_Continue_GoTo_Get()!=-1)
        {
            pos = MI.Open_Buffer_Continue_GoTo_Get();	////Position the buffer
            MI.Open_Buffer_Init(length, pos);  //Informaing MediaInfo we have seek
        }
        pos += slice;
    }while(pos < length);

    //Finalizing
    MI.Open_Buffer_Finalize(); //This is the end of the stream, MediaInfo must finnish some work

    //Inform with Complete=false
    //MI.Open(pathname);
    String To_Display;
    MI.Option("Complete");
    To_Display += MI.Inform().c_str();

    To_Display += "\r\n\r\nVideo StreamCount: ";
    To_Display += MI.Get(Stream_Video, 0, "StreamCount", Info_Text, Info_Name).c_str();

    To_Display += "\r\n\r\nAudio StreamCount: ";
    To_Display += MI.Get(Stream_Audio, 0, "StreamCount", Info_Text, Info_Name).c_str();

    pEmi->mi = To_Display;

    int val = MI.Count_Get(Stream_Video);
    for (int i = 0; i < val; i++)
    {
        EI_VIDEOINFO_T ev;
        ev.id =  atoi(MI.Get(Stream_Video, i, "ID", Info_Text, Info_Name).c_str());
        ev.format = MI.Get(Stream_Video, i, "Format", Info_Text, Info_Name);
        pEmi->vecVStream.push_back(ev);
    }

    val = MI.Count_Get(Stream_Audio);
    for (int i = 0; i < val; i++)
    {
        EI_AUDIOINFO_T ea;
        ea.id =  atoi(MI.Get(Stream_Audio, i, "ID", Info_Text, Info_Name).c_str());
        ea.format = MI.Get(Stream_Audio, i, "Format", Info_Text, Info_Name);
        pEmi->vecAStream.push_back(ea);
    }

    MI.Close();
}

MediaInfoDLL::String CEiMediaInfo::CheckMediaInfo(const string& PathName)
{
    using namespace MediaInfoDLL;

    //Initilaizing MediaInfo
    MediaInfo MI;

    //Inform with Complete=false
    MI.Open(PathName);
    String To_Display;
    String To_Debug;
    MI.Option("Complete");

    To_Debug += MI.Inform().c_str();
    //To_Display += "\r\n\r\nVideo StreamCount: ";
    //To_Display += MI.Get(Stream_Video, 0, "StreamCount", Info_Text, Info_Name).c_str();

    ei_log(LV_DEBUG,"libeasyice","MediaInfoDLL check result:\n %s", To_Debug.c_str());

    //To_Display += "<CompleteName>";
    //To_Display += MI.Get(Stream_General, 0, "CompleteName", Info_Text, Info_Name).c_str();
    //To_Display += "</CompleteName>\n";

    To_Display += "<Duration>";
    string ff_dur = ffprobe_dur(PathName);
    ei_log(LV_DEBUG,"libeasyice","ffprobe check duration:\n %s",ff_dur.c_str());
    if (!ff_dur.empty())
    {
        float fff_dur = atof(ff_dur.c_str())/60;
        char ch_dur[1024];
        sprintf(ch_dur, "%.1f", fff_dur);
        To_Display += ch_dur;
    }
    To_Display += "</Duration>\n";

    //To_Display += "<FileSize>";
    //To_Display += MI.Get(Stream_General, 0, "FileSize", Info_Text, Info_Name).c_str();
    //To_Display += "</FileSize>\n";

    To_Display += "<OverallBitRate>";
    string mi_rate = MI.Get(Stream_General, 0, "OverallBitRate", Info_Text, Info_Name).c_str();
    stringstream ss;
    if (mi_rate.empty())
    {
        string ff_rate = ffprobe_rate(PathName);
        ei_log(LV_DEBUG,"libeasyice","ffprobe check bit_rate:\n %s",ff_rate.c_str());
        if (!ff_rate.empty())
        {
            ss << atoi(ff_rate.c_str())/1024;
            To_Display += ss.str();
        }
    } else {
        ss << atoi(mi_rate.c_str())/1024;
        To_Display += ss.str();
    }
    To_Display += "</OverallBitRate>\n";

    To_Display += "<FrameRate>";
    string mi_fps = MI.Get(Stream_General, 0, "FrameRate", Info_Text, Info_Name).c_str();
    if (mi_fps.empty())
    {
        mi_fps = ffprobe_fps(PathName);
        ei_log(LV_DEBUG,"libeasyice","ffprobe check fps:\n %s",mi_fps.c_str());
    } 
    if (!mi_fps.empty()) {
        if (mi_fps.find("/") != string::npos) {
            int pre = atoi(mi_fps.substr(0, mi_fps.find("/")).c_str());
            int suf = atoi(mi_fps.substr(mi_fps.find("/") + 1).c_str());
            char ch_fps[100];
            if (suf > 0) {
                double resultfps = (double)pre/suf;
                sprintf(ch_fps, "%.3f", resultfps);
            } else {
                sprintf(ch_fps, "%d", pre);
            }
            mi_fps = ch_fps;
        } 

        if (mi_fps.find(".") != string::npos) {
            int k = mi_fps.length() - 1;
            for ( ; k >= mi_fps.find("."); k--) {
                if (mi_fps[k] != '0' && mi_fps[k] != '.') 
                    break;
            }
            mi_fps = mi_fps.substr(0, k + 1);
        }

        To_Display += mi_fps;
    }
    
    To_Display += "</FrameRate>\n";

    MI.Close();

    //return To_Display;
    return To_Debug;
}

string CEiMediaInfo::ffprobe_all(const string& PathName)
{
    string cmd = "ffprobe -v quiet -show_format -show_streams  -show_programs -of json "+ PathName;


    FILE* fp = popen(cmd.c_str(),"r");

    if(fp == NULL)
    {
        cerr << "exec ffprobe cmd faild, mypopen return error : " <<endl;
        return "";
    }

    stringstream ss;
    while(fgets(m_pBuffer, m_nBufferLen, fp) != NULL)
    {
        ss<<m_pBuffer;
    }
    pclose(fp);
    return ss.str();
}

