#!/bin/bash

export LD_LIBRARY_PATH=../sdk/lib

./tsplay test123_239.1.2.3_12345.ts  127.0.0.1:1234 >/dev/null 2>&1 &
#valgrind --leak-check=full  --show-reachable=yes ./test_app_udplive
./test_app_udplive >/dev/null 2>&1 &
