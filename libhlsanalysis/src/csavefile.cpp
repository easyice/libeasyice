/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "csavefile.h"
#include "global.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>



//ts老化时间(秒)
const int TS_EXPIRE_TIME = 60*3;



CSaveFile::CSaveFile()
{
        m_segment_count = 0;
        m_bFirstSave = true;
}

CSaveFile::~CSaveFile()
{

}

void CSaveFile::SetParam(const HLS_RECORD_INIT_PARAM_T& param)
{
	m_stParam = param;

}
//
//void CSaveFile::OnCheckTsExpire()
//{
//
//        list<string> file_lst;
//        findprocess(file_lst,m_path.c_str());
//        
//        if (file_lst.empty())
//        {
//                cout << "dest path is empty,OnCheckTsExpire nothing to do" << endl;
//                return ;
//        }
//        
//
//        list<string>::iterator it = file_lst.begin();
//        for (; it != file_lst.end(); ++it)
//        {
//                if (it->rfind(".") == string::npos)
//                {
//                        continue;
//                }
//                string ext_name = it->substr(it->rfind("."));
// 
//                if (strncasecmp(ext_name.c_str(),".ts",3) != 0)
//                {
//                        continue;
//                }
//                
//                time_t cur_time = time(NULL);
//                struct stat statbuf;
//                if ( stat ( it->c_str(),&statbuf ) != 0 )
//                {
//                        cout << "CheckOldThread stat error " << endl;
//                        continue;
//                }
//                if (difftime(cur_time,statbuf.st_mtime) > TS_EXPIRE_TIME)
//                {
//                        cout << "remove file:" << *it << endl; 
//                        unlink(it->c_str());
//                }
//                cout << "difftime=" << difftime(cur_time,statbuf.st_mtime) << endl;
//        }
//
//}

bool CSaveFile::IsExist(const string& url)
{
        string tsname = url.substr(url.rfind("/")+1);
		tsname = hls_analysis::removeparam(tsname);
        if (tsname.empty())
        {
                cout << "removeparam faild" << endl;
                return false;
        }

        string pathname = m_path;
		pathname += tsname;
        
		if( (access(pathname.c_str(), R_OK )) == 0  )
        {
                return true;
        }
        
        return false;
}

void CSaveFile::OnRecvTs(const string& url,const BYTE* pData, int nLen)
{
        if (m_bFirstSave)
        {
                if (m_stParam.realtime_mode)
                {
                        //OnCheckTsExpire();
                }
                m_bFirstSave = false;
        }
        if (m_stParam.savefile != NULL)
        {
                cout << "暂时不支持保存单个文件" << endl;
                return;
        }
        string tsname = url.substr(url.rfind("/")+1);
		tsname = hls_analysis::removeparam(tsname);
        if (tsname.empty())
        {
                cout << "removeparam faild" << endl;
                return;
        }

        string pathname = m_path;
		pathname += tsname;
        
        FILE* fp = fopen(pathname.c_str(),"wb");
        if (fp == NULL)
        {
                printf("open file error:%s\n",pathname.c_str());
                return;
        }
        int writed = fwrite(pData,1,nLen,fp);
        fclose(fp);
        
        printf("writefile:%s,size:%d,write:%d,err=%d\n",pathname.c_str(),nLen,writed,errno);
        
        //针对ffmpeg的循环生成文件方式
        list<string>::iterator it = m_lstTsHistory.begin();
        for(; it != m_lstTsHistory.end(); ++it)
        {
			if (strcmp(it->c_str(),pathname.c_str()) == 0)
                {
                        m_lstTsHistory.erase(it);
                        break;
                }
        }
        m_lstTsHistory.push_back(pathname);
        
        //删除旧的
        if (m_stParam.realtime_mode && m_lstTsHistory.size() > m_segment_count*3)//多保留一会
        {
                printf("remove ts:%s\n",m_lstTsHistory.begin()->c_str());
                unlink(m_lstTsHistory.begin()->c_str());
                m_lstTsHistory.pop_front();
        }
        
}

void CSaveFile::OnRecvM3u8(const string& m3u8)
{
        if (m_stParam.savefile != NULL)
        {
                cout << "暂时不支持保存单个文件" << endl;
                return;
        }
        
        FILE* fp = fopen(m_m3u8pathname.c_str(),"w");
        if (fp == NULL)
        {
                printf("open file error:%s\n",m_m3u8pathname.c_str());
                return;
        }
        fwrite(m3u8.c_str(),1,m3u8.size(),fp);
        fclose(fp);
}

bool CSaveFile::Init()
{
        if (m_stParam.outname == NULL)
        {
                m_path = m_stParam.savepath;
				m_path += m_m3u8name;
				m_path += "\\";
        }
        else
        {
                m_path = m_stParam.savepath;
				m_path += m_stParam.outname;
				m_path += "\\";
        }
        
		hls_analysis::CreateMulPath(m_path.c_str());
        //mkdir( m_path.c_str(), 0777 );
        
        m_m3u8pathname = m_path;
		m_m3u8pathname += m_m3u8name;
        
        return true;
}
