#!/bin/bash

export LD_LIBRARY_PATH=../sdk/lib

./test_app_file >/dev/null 2>&1
if [ $? -ne 0 ];then
    echo "./test_app_file run error"
    exit
fi

cat helloworld.ts.tr101290.json |jq . >/dev/null
if [ $? -ne 0 ];then
    echo "helloworld.ts.tr101290.json parse error!"
    exit
fi


cat helloworld.ts.pids.json |jq . >/dev/null
if [ $? -ne 0 ];then
    echo "helloworld.ts.pids.json parse error!"
    exit
fi


cat helloworld.ts.programinfo.json |jq . >/dev/null
if [ $? -ne 0 ];then
    echo "helloworld.ts.tr101290.json parse error!"
    exit
fi


cat helloworld.ts.psi.json |jq . >/dev/null
if [ $? -ne 0 ];then
    echo "helloworld.ts.psi.json parse error!"
    exit
fi

cat helloworld.ts.ffprobe.json |jq . >/dev/null
if [ $? -ne 0 ];then
    echo "helloworld.ts.ffprobe.json parse error!"
    exit
fi

echo "all test passed! :)"

