/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#include "StdAfx.h"
#include "CBit.h"
#include <string.h>

CBit::CBit(void)
{
}

/* 
-- get bits out of buffer (max 32 bit!!!)
-- return: value
*/
unsigned int CBit::getBits (u_char *buf, int byte_offset, int startbit, int bitlen)
{
	u_char *b;
	unsigned int  v;
	unsigned int mask;
	unsigned int tmp_long;
	int           bitHigh;
	b = &buf[byte_offset + (startbit >> 3)];
	startbit %= 8;

	switch ((bitlen-1) >> 3) {
	 case -1:	// -- <=0 bits: always 0
		 return 0L;
		 break;

	 case 0:		// -- 1..8 bit
		 tmp_long = (unsigned int)(
			 (*(b  )<< 8) +  *(b+1) );
		 bitHigh = 16;
		 break;

	 case 1:		// -- 9..16 bit
		 tmp_long = (unsigned int)(
			 (*(b  )<<16) + (*(b+1)<< 8) +  *(b+2) );
		 bitHigh = 24;
		 break;

	 case 2:
	 case 3:// -- 17..24 bit
		 tmp_long = (unsigned int)(
			 (*(b  )<<24) + (*(b+1)<<16) +
			 (*(b+2)<< 8) +  *(b+3) );
		 bitHigh = 32;
		 break;

	 //case 3:		// -- 25..32 bit
		// // -- to be safe, we need 32+8 bit as shift range 
		// return (unsigned int) getBits48 (b, 0, startbit, bitlen);
		// break;

	 default:	// -- 33.. bits: fail, deliver constant fail value
		 //out_nl (1," Error: getBits() request out of bound!!!! (report!!) \n");
		 return (unsigned int) 0xFEFEFEFE;
		 break;
	}

	startbit = bitHigh - startbit - bitlen;
	tmp_long = tmp_long >> startbit;
	mask     = (1ULL << bitlen) - 1;  // 1ULL !!!
	v        = tmp_long & mask;
	return v;
}

/*
-- get bits out of buffer  (max 48 bit)
-- extended bitrange, so it's slower
-- return: value
*/

long long CBit::getBits48 (u_char *buf, int byte_offset, int startbit, int bitlen)
{
	u_char *b;
	unsigned long long v;
	unsigned long long mask;
	unsigned long long tmp;

	if (bitlen > 48) {
		//out_nl (1," Error: getBits48() request out of bound!!!! (report!!) \n");
		return 0xFEFEFEFEFEFEFEFELL;
	}

	b = &buf[byte_offset + (startbit / 8)];
	startbit %= 8;

	// -- safe is 48 bitlen
	tmp = (unsigned long long)(
		((unsigned long long)*(b  )<<48) + ((unsigned long long)*(b+1)<<40) +
		((unsigned long long)*(b+2)<<32) + ((unsigned long long)*(b+3)<<24) +
		(*(b+4)<<16) + (*(b+5)<< 8) + (*(b+6)) );

	startbit = 56 - startbit - bitlen;
	tmp      = tmp >> startbit;
	mask     = (1ULL << bitlen) - 1;	// 1ULL !!!
	v        = tmp & mask;
	return v;
}


/*
-- get bits out of buffer   (max 64 bit)
-- extended bitrange, so it's slower 
-- return: value
*/

unsigned long long CBit::getBits64 (u_char *buf, int byte_offset, int startbit, int bitlen)
{
	unsigned long long x1,x2,x3;

	if (bitlen <= 32) {
		x3 = getBits (buf,byte_offset,startbit,bitlen); 
	} else {
		x1 = getBits (buf,byte_offset,startbit,32); 
		x2 = getBits (buf,byte_offset,startbit+32,bitlen-32); 
		x3 = (x1<<(bitlen-32)) + x2;
	}
	return x3;
}

void CBit::setBits (u_char *buf, int byte_offset, int startbit, int bitlen,unsigned long long srcdata)
{
	setBits48 (buf,byte_offset,startbit,bitlen,srcdata);
}

void CBit::setBits48 (u_char *buf, int byte_offset, int startbit, int bitlen,unsigned long long srcdata)
{
	u_char *b = NULL;
	if (bitlen > 48) {
		return ;//0xFEFEFEFEFEFEFEFELL;
	}
	b = &buf[byte_offset + (startbit>>3)];

	int len = 0;
	if ( 0==(startbit%8) )
	{
		if ( 0==(bitlen%8) )
		{
			len = bitlen>>3;
		}
		else
		{
			len = (bitlen>>3) +1;
		}
	}
	else if ( 0!=(startbit%8) )
	{
		if ((startbit%8 +bitlen)<8)
		{
			len = 1;
		}
		else if (0==(startbit%8 +bitlen)%8)
		{
			len = (startbit%8 +bitlen)>>3;
		}
		else
		{
			len = ((startbit%8 +bitlen)>>3) +1;
		}
	}

	int shiftlen = 0;
	if ( (bitlen+startbit) <8)
	{
		shiftlen = 8-bitlen;
	}
	else if( 0!=((bitlen+startbit)%8) )
	{
		shiftlen = 8-(bitlen+startbit)%8;
	}

	srcdata &= ((unsigned long long)1<<bitlen) - 1;
	unsigned long long shiftdata = srcdata<<shiftlen;///和buf的数据对齐
	int reverse_temp = len;
	for (int i =0;i< len;++i)
	{
		b[--reverse_temp]|=((unsigned char*)(&shiftdata))[i];
	}
}
