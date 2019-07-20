# libeasyice

libeasyice is the library of EasyICE，include the full analyzer of the EasyICE.


## Features

ts file analyzer：

* PES layer decode
* rate analyzer by per pcr
* timestamp analyzer
* gop list
* pid list
* PSI decode
* TR101290

udp live streaming analyzer：

* PSI decode
* pid list
* pcr_oj,pcr_ac,pcr_interval,tsrate
* record
* MPTS（except the tsrate，it‘s overall rate of mpts）
* TR101290
* udp unicast,multicast(igmp v2,igmp v3),broadcast support

hls analyzer：

* ts segment quality detection
* hls layer protocol analysis
* http layer protocol analysis
* hls diagnostic



## Simple Build Instructions

libeasyice require ffmpeg and mediainfo,all the dependent file already in `deps` directory，you can run 
    cd deps
    sh install.sh

the build require centos 7.2 or above. 

1. build sdk
    cd libeasyice/make && make
    cd libhlsanalysis/make && make
    cd libtr101290/make && make

2. build examples
    cd examples
    sh build.sh


the sdk build files is output to `sdk` directory，it's also include all the dependent files,such as `ffmpeg` ,`mediainfo`.


## SDK introduction

* if the input is a `FILE`，the analyzer result output to  json file.
* if the input is a `LIVE streaming`(such as udp,hls)，the analyzer result output by callback funcation.

The example of the sdk usage is in `examples` directory ,for example the file analyzer demo is `file.cpp`


## SDK APIs

the API is thread-safe,and all API is export in `c` language.

There are two global APIs，It can only be called once in progress lifetime:

```
easyice_global_init();
easyice_global_cleanup();
```

task APIs：

```
easyice_init();
easyice_setopt();
easyice_process();
easyice_cleanup()
```

protocol support：

```
file://
http://
https://
udp://
rtp://
```

##  File Analyzer

file analysis output results multiple files, which are stored in the same path as input file:

```
$INPUTFILE.programinfo.json
include gop list,ts rate（码率抖动）,timestamp（时间线）

$INPUTFILE.mediafinfo.json
PES layer info，frame rate，frame size

$INPUTFILE.ffprobe.json
program info ,pes info ,frame rate，frame size...

$INPUTFILE.tables.json
tree of PSI 

$INPUTFILE.tr101290.json
tr101290 results

$INPUTFILE.pids.json
pid statistics
```

json struct definition see: tablesdefs.h commondefs.h

implemented of serialization is: libeasyice/libeasyice/src/EasyICEDLL/FileAnalysis.cpp#WriteOutputFiles

The principle of serialization is that the deserialized result can be filled into the original structure.

note：mediainfo result is text,not json.


## UDP Live Streaming Analyzer

This Analyzer's result output by callback funcation, the callback function has only one, use different parameters to distinguish different types of callback content.The callback period can be set at initialization time, the default is 1 second, support to the millisecond level. The callback content is described in json struct.

During the callback period, the callback function is still called if no data is generated,and the content in the callback is empty,such as:

```
{"1" : null}
```

Callback function :

```
 typedef void (*easyice_udplive_callback)(UDPLIVE_CALLBACK_TYPE type,const char* json,void *pApp);
```

The UDPLIVE_CALLBACK_TYPE definition:

**UDPLIVE_CALLBACK_MEDIAFINO**
mediainfo checking result，this callback will be only once,it's a text content，not json.
   
**UDPLIVE_CALLBACK_FFPROBE**,
ffprobe reslut,callback only once

**UDPLIVE_CALLBACK_PIDS**

```
{
   "0" : {
      "PID" : 0,
      "percent" : 0.024701522663235664,
      "total" : 60,
      "type" : "PAT"
   }
} 
```

**UDPLIVE_CALLBACK_PSI**
PSI info，such as file analyzer,callback only once

**UDPLIVE_CALLBACK_TR101290**
TR101290 result ,such as file analyzer

**UDPLIVE_CALLBACK_PCR**

```
{
   "1" : [
      {
         "llPcr_Ac" : 24800348,
         "llPcr_Oj" : -95425189,
         "llPcr_interval" : -18000,
         "llTime" : 1514959783542496
      }
   ]
}
```
`"1"` ： program_number
`llPcr_Ac` microsecond,-1 is an invalid value
`llPcr_Oj` millisecond
`llPcr_interval` millisecond,-1 is an invalid value
`llTime` microsecond


**UDPLIVE_CALLBACK_RATE**

```
{
   "tsrate" : [
      {
         "fRate" : 536928,
         "llTime" : 1515214903129712
      }
   ]
}
```

`fRate` bit/s，for mpts，it's the overall ts bitrate of the stream.
`llTime` microsecond



**UDPLIVE_CALLBACK_PROGRAM_INFO_BRIEF**
A brief description of program information, including program number and program's PID information.

```
{
   "1" : [
      {
         "pid" : 256,
         "type" : "H.264"
      },
      {
         "pid" : 257,
         "type" : "AAC"
      },
      {
         "pid" : 256,
         "type" : "PCR"
      }
   ]
   "remote_address" : "127.0.0.1",
   "remote_port" : 35407
}
```
`"1"` ： program_number
`remote_address`: udp source ip
`remote_port`: udp source port

usage: examples/udplive.cpp


## HLS Analyzer

HLS Analyzer support  HTTP，HTTPS，LiveStreaming，and HLS on demand.

This Analyzer's result output by callback funcation, the callback function has only one, use different parameters to distinguish different types of callback content.

Callback function :

```
typedef void (*PF_HLS_REPORT_CB)(HLS_REPORT_PARAM_T param);
```

usage: examples/hls.cpp




