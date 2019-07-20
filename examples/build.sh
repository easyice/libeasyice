export LD_LIBRARY_PATH=../sdk/lib:/usr/local/lib

g++ -g file.cpp -o test_app_file -I../sdk/include -L../sdk/lib -L/usr/local/lib -leasyice -ltr101290 -ldvbpsi
g++ -g udplive.cpp -o test_app_udplive -I../sdk/include -L../sdk/lib -leasyice -ltr101290 -ldvbpsi
g++ -g hls.cpp -o test_app_hls -I../sdk/include -L../sdk/lib  -lhlsanalysis -ldvbpsi -ltr101290 -lcurl -leasyice
