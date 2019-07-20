/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "FileAnalysis.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../sdkdefs.h"
#include "CheckMediaInfo.h"
#include "EiLog.h"
#include "json/json.h"
#include "TrView.h"

using namespace std;

//about 4.9MB, support probe,and pkt len as 188,204,192
const int READ_BUF_SIZE = 188*204*128;



int FileAnalysis::TryToReSync(BYTE* pData,int nLength,int& nSycByte)
{
	nSycByte = 0;
	int maxLength = nLength - 204*3;
	if (maxLength < 204*3)
		return -1;

	for (int i = 0; i < maxLength; i++)
	{
		if(pData[i]==0x47&&pData[i+188]==0x47&&pData[i+188*2]==0x47&&pData[i+188*3]==0x47&&pData[i+188*4]==0x47)
		{	
			nSycByte = i;
			return 188;
		}
		else if(pData[i]==0x47&&pData[i+204]==0x47&&pData[i+204*2]==0x47&&pData[i+204*3]==0x47&&pData[i+204*4]==0x47)
		{
			nSycByte = i;
			return 204;
		}
	}
	return -1;
}


int FileAnalysis::GetTsLength(const char* PathName,int& nSyncByte)
{
	nSyncByte = 0;
	int ret = 0;
	FILE *fp = NULL;

	int bufferLength = 204*100;
	BYTE *pBuffer = new BYTE[bufferLength];
	memset(pBuffer,0,bufferLength);
	fp = fopen(PathName,"rb");
	if (fp == NULL)
	{
		delete [] pBuffer;
		return -1;
	}

	size_t numread = fread(pBuffer,sizeof(BYTE),bufferLength,fp);
	//if (numread != sizeof(pBuffer))
	//{
	//	fclose(fp);
	//	delete [] pBuffer;
	//	return ERROR_FILE_NOT_DATA;
	//}
	if(pBuffer[0]==0x47&&pBuffer[188]==0x47&&pBuffer[188*2]==0x47&&pBuffer[188*3]==0x47&&pBuffer[188*4]==0x47)
			ret=188;
	else if(pBuffer[0]==0x47&&pBuffer[204]==0x47&&pBuffer[204*2]==0x47&&pBuffer[204*3]==0x47&&pBuffer[204*4]==0x47)
			ret=204;
	else
	{
		//尝试重新同步
			ret = TryToReSync(pBuffer,bufferLength,nSyncByte);
	}
	delete [] pBuffer;
	fclose(fp);
	return ret;

}





FileAnalysis::FileAnalysis()
{
    m_pTrcore = new Clibtr101290();
    m_pMpegDec = new CMpegDec();
    nlibtr101290 = CALL_PASSWD;

    m_pTrView = new CTrView();
    m_pTrcore->SetReportCB(CTrView::OnTrReport,m_pTrView);

    m_pEiMediaInfo = new CEiMediaInfo();
}


FileAnalysis::~FileAnalysis()
{
    delete m_pMpegDec;
    delete m_pTrcore;
    delete m_pTrView;
    delete m_pEiMediaInfo;
}

int FileAnalysis::OpenMRL(const EASYICE* handle)
{
    struct stat filestat;
	if (stat(handle->mrl, &filestat) < 0)
    {
        ei_log(LV_ERROR,"libeasyice","stat file faild:%s",handle->mrl);
		return -1;
    }
    {
		struct tm *p;
		p=gmtime(&filestat.st_mtime);
		char s[100];
		strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", p);  
	}

	int nSyncByte = 0;
	int nTsLength = GetTsLength(handle->mrl,nSyncByte);
	if (nTsLength <= 0)
	{
        ei_log(LV_ERROR,"libeasyice","GetTsLength error:%s",handle->mrl);
		return -1;
	}


	FILE* fp = fopen(handle->mrl,"rb");
	if (fp == NULL)
	{
        ei_log(LV_ERROR,"libeasyice","open file faild:%s",handle->mrl);
		return -1;
	}

	if (nSyncByte != 0)
	{
		fseek(fp,nSyncByte,SEEK_SET);
	}
    
	//easyice checking
	BYTE* buf = new BYTE[READ_BUF_SIZE];
	m_pMpegDec->Init(nTsLength,filestat.st_size/nTsLength);
    m_pMpegDec->SetProgressFunCb((easyice_progress_callback)handle->progress_cb_func,handle->progress_cb_data);

    m_pTrcore->SetStartOffset(nSyncByte);
    m_pTrcore->SetTsLen(nTsLength);

	long long total_size = 0;
    bool bfirst = true;
    int size = 0;
	while(1)
	{
		if (handle->b_file_check_park && total_size > 50000000)//50M
		{
			break;
		}
		if ((size = fread(buf,1,READ_BUF_SIZE,fp)) <= 0)
        {
            break;
        }
        if (bfirst)
        {
            m_pMpegDec->ProbeMediaInfo(buf,size);
            bfirst = false;
        }
		m_pMpegDec->ProcessBuffer(buf,size);

        for (int i = 0; i < size; i+= nTsLength)
        {
            m_pTrcore->AddPacket(buf+i);
        }
        total_size += size;
    }

    m_pMpegDec->Finish();

    //mediainfo check
    string mi = m_pEiMediaInfo->CheckMediaInfo(handle->mrl);
    string ffprobe = m_pEiMediaInfo->ffprobe_all(handle->mrl);

    //dump output
    WriteOutputFiles(handle->mrl,mi,ffprobe);
    
    fclose(fp);
	delete [] buf;
	
    return 0;
}

void FileAnalysis::WriteFile(const string& filename,const string& data)
{
    FILE * fp = fopen(filename.c_str(),"w");
    if (fp == NULL)
    {
        ei_log(LV_ERROR,"libeasyice","open file for write error:%s",filename.c_str());
        return;
    }

    fwrite(data.c_str(),1,data.length(),fp);
    fclose(fp);
}


void FileAnalysis::WriteOutputFiles(const char* mrl,const string& mi,const string& ffprobe)
{
    //mediainfo
    WriteFile((string)mrl+".mediainfo.json",mi);
    
    //ffprobe
    WriteFile((string)mrl+".ffprobe.json",ffprobe);

    //pid list
	Json::Value rootPid;
    std::map<int,MSG_PID_LIST>::iterator it_pids = m_pMpegDec->m_mapPidList.begin();
    for (;it_pids != m_pMpegDec->m_mapPidList.end();++it_pids)
    {
        int pid = it_pids->second.PID;
        int total = it_pids->second.total;
        float pct = it_pids->second.percent * 100;
        string type = Get_PacketTypeDes(it_pids->second.type);
		char key[32];
		sprintf(key,"%d", it_pids->first);
		rootPid[key] = it_pids->second.to_json();
    }
	string outPid = rootPid.toStyledString();  
    WriteFile((string)mrl+".pids.json",outPid);
	//ei_log(LV_DEBUG,"libeasyice","pid json: %s", outPid.c_str()) ;


    //psi
    Json::Value rootTable;
    TABLES* tables = m_pMpegDec->m_TableAnalyzer.GetTables();
    rootTable = tables->to_json();
    string outTable = rootTable.toStyledString();
    WriteFile((string)mrl+".psi.json",outTable);
    //ei_log(LV_DEBUG,"libeasyice","table json: %s", outTable.c_str()) ;
	
    
    //program info
    ALL_PROGRAM_INFO* allprograminfo = m_pMpegDec->GetAllProgramInfo();

	if (allprograminfo->empty())
	{
        ei_log(LV_ERROR,"libeasyice","allprograminfo is empty!") ;
		return;
	}
    

    Json::Value rootProg;
	ALL_PROGRAM_INFO::iterator it_all = allprograminfo->begin();
    for (;it_all != allprograminfo->end();++it_all)
    {
        int prgmram_num = it_all->first;
        PROGRAM_INFO* pi = it_all->second;
	    std::vector<GOP_LIST>::iterator it_gop = pi->gopList.begin();
        std::vector<RATE_LIST>::iterator it_rate = pi->rateList.begin();
        
        // the all timestamps
        PROGRAM_TIMESTAMPS tts = pi->tts;


        char key[32];
        sprintf(key, "%d", prgmram_num);
        rootProg[key] = it_all->second->to_json();
    }

    string outProg = rootProg.toStyledString();  
    WriteFile((string)mrl+".programinfo.json",outProg);
    //ei_log(LV_DEBUG,"libeasyice","program json: %s", outProg.c_str()) ;
    
    //tr101290
    WriteFile((string)mrl+".tr101290.json",m_pTrView->ToJson());
}

void FileAnalysis::OnTrReport(REPORT_PARAM_T param)
{
//    m_pTrView->OnTrReport(param);
}


