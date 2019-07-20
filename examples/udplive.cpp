/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "libeasyice.h"
#include <unistd.h>

void demo_callback(UDPLIVE_CALLBACK_TYPE type,const char* json,void *pApp)
{
    printf("type=%d,content=%s,pApp=%s\n",type,json,(char*)pApp);
}

int main(int argc,char** argv)
{
    const char* pdata = "pdata";
    easyice_global_init();

    EASYICE* handle  = easyice_init();
    easyice_setopt(handle,EASYICEOPT_MRL, (void*)"udp://127.0.0.1:1234");
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_LOCAL_IP, (void*)"0.0.0.0");
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_FUNCTION, (void*)demo_callback);
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_DATA, (void*)"pdata");

    //设置回调函数调用间隔周期
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_CB_UPDATE_INTERVAL, (void*)1000000);//1s
    
    //设置计算 tsRate 的间隔周期，支持检测期间动态调整
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_CALCTSRATE_INTERVAL, (void*)1000);//1s
    
    //执行后立即返回，后台线程开始启动分析任务
    EASYICEcode ret = easyice_process(handle);
    if (ret != EASYICECODE_OK)
    {
    	printf("process error\n");
        _exit(-1);
    }

   // 注意：easyice_process 调用之后才能执行录制命令
    easyice_setopt(handle,EASYICEOPT_UDPLIVE_START_RECORD, (void*)"/tmp/r.ts");


    //stop analysis after 10s
    while(1)sleep(10);
    
  //  easyice_setopt(handle,EASYICEOPT_UDPLIVE_STOP_RECORD,NULL);
    
    easyice_cleanup(handle);
    easyice_global_cleanup();
    printf("success\n");
    return 0;
}
