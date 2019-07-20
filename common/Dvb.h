/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include "ztypes.h"


typedef WORD PID;
#define INVALID_PID 0xffff
#define TS_PACKET_IDENT	0x47

typedef long long __int64;;
typedef unsigned long long PCR;
#define INVALID_PCR (PCR)(0xffffffffffffffff)
#define MAX_PCR	0x40000000000


/*

#define TS_PACKET_LENGTH_188	188
#define TS_PACKET_LENGTH_206	206
#define TS_PACKET_LENGTH_STANDARD	TS_PACKET_LENGTH_188
*/


#define TS_REF_CLOCK	27000000

//缺省PCR时间间隔
#define DEFAULT_TS_PCR_INTERVAL (TS_REF_CLOCK / 20)

#define MS_TO_PCR(time_ms) (time_ms * 27000)

typedef WORD PROGRAM_NUMBER;
typedef BYTE SECTION_NUMBER;

#define GET_TS_PID(pBuf) (WORD)((((*(pBuf)) & 0x1f)<<8) | (*(pBuf+1)))

#define GET_PCR_INTERVAL(from,to) ((from <= to) ? (to-from) : (MAX_PCR - from + to))


typedef enum _TS_PACKET_LENGTH_TYPE
{
	TS_PACKET_LENGTH_STANDARD =188,
	TS_PACKET_LENGTH_188 =188,
	TS_PACKET_LENGTH_204 = 204,
	TS_PACKET_LENGTH_INVALID = 0

}TS_PACKET_LENGTH_TYPE;

