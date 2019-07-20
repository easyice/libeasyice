/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <easyice.h>
#include <stdio.h>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>


using namespace std;



bool g_bEnableDebug = false;
int  g_gop_size_threshold = 50;
bool g_check_part = false;


static string g_monModName = "tsinfo";
static string g_monModVer  = "1.0";


static string input_file;
void usage(char *name)
{
    cout << "USAGE: "<< name <<" [OPTION] -i inputfile" << endl;
    cout << " -i set input file" << endl;
    cout << " -s set max gop size threshold" << endl;
    cout << " -b check file size" << endl;
    cout << " -d enable debug model" << endl;
    cout << " -h this help" << endl;
    _exit(0);
}



void scan_args(int argc,char **argv)
{
    struct option longopts[]=
    {
        {"input",required_argument,NULL,'l'},
        {"debug",no_argument,NULL,'d'},
        {"help",no_argument,NULL,'h'},
        {"checksize",no_argument,NULL,'c'},
        

        {0,0,0,0},
    };

    int c;
    while ( ( c = getopt_long_only ( argc,argv,"chdl:s:",longopts,NULL ) ) != -1 )
    {
        switch ( c )
        {
            case 'l':
                input_file = optarg;
                break;
            case 's':
                g_gop_size_threshold = atoi(optarg);
                break;
            case 'c':
                g_check_part = true;
                break;
            case 'd':
                g_bEnableDebug = true;
                break;
            case 'h':
                usage(argv[0]);
                break;
            default:
                break;
        }
    }
}

int main(int argc ,char** argv)
{
    scan_args(argc,argv);
    if (input_file.empty())
    {
        //usage(argv[0]);
        return 0;
    }

    ei_mediainfo(input_file.c_str());
    return 0;
}
