#!/bin/bash

export LD_LIBRARY_PATH=../sdk/lib

#valgrind --leak-check=full  --show-reachable=yes ./test_app_udplive
gdb ./test_app_hls
