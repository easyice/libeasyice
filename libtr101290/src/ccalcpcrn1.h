/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#ifndef CCALCPCRN1_H
#define CCALCPCRN1_H

class CCalcPcrN1
{
public:
	CCalcPcrN1();
	~CCalcPcrN1();
	
	///增加一个PCR包
	void AddPcrPacket(long long pcr);

	void Reset();
	
	///增加一个非PCR包
	void AddPayloadPacket();
	
	//没有计算到时返回  -1
	long long GetPcr();

	//添加PCR包前获取上一个PCR值
	long long GetPcrPrev();
	
	//Byte/s 为输出模块调整PCR时设置初始码率，用以产生第二个PCR值
	void SetInitRate(double rate);
private:
	long long m_pcrBefor;
	double m_fTransportRate;
	
	///包含了PCR包数
	long long m_nPacketCountOfPcr;
};

#endif // CCALCPCRN1_H
