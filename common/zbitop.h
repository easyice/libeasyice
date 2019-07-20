/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ZBITOP_H
#define ZBITOP_H

#include "ztypes.h"

#include <iostream>

using namespace std;



class ZBit
{

    public:
        /**
        * 设置一个指定变量的指定的位为1
        * 变量的最低位 的位置为0
        * @param Var 需要设置的变量
        * @param Bit 需要设置的变量的位，变量的最低位为0
         */
        template <typename _Ty>
        static inline void VarSet ( _Ty &Var, int Bit )
        {
            Var |= ( ( ( _Ty ) ( 1 ) ) << Bit );
        }

        /**
         * 设置一个指定变量的指定的位为 0
         * 变量的最低位 的位置为0
         * @param Var 需要设置的变量
         * @param Bit 需要设置的变量的位，变量的最低位为0
         */
        template <typename _Ty>
        static inline void VarZero ( _Ty &Var, int Bit )
        {
            Var &= ~ ( ( ( _Ty ) ( 1 ) ) << Bit );
        }

        /**
         * 测试一个指定变量的指定的位
         * 变量的最低位 的位置为0
         * @param Var 需要设置的变量
         * @param Bit 需要设置的变量的位，变量的最低位为0
         * @return 布尔变量，true->指定位为1,false->为0
         */
        template <typename _Ty>
        static  inline bool VarTest ( const _Ty &Var, int Bit )
        {
            return Var >> Bit  & 1;
        }

        /**
         * 打印一个变量的所有位，高位在前，一般用于调试
         * @param Var 指定变量
         */
        template <typename _Ty>
        static inline void VarPrint ( const _Ty &Var )
        {
            cout << endl;

            for ( int i = sizeof ( _Ty ) * 8 - 1;i >= 0;i-- )
            {
                cout << VarTest ( Var, i );

                if ( i % 8 == 0 )
                    cout << " ";
            }

            cout << endl;
        }

        /**
         * 设置指定缓冲区的指定位为1
         * @param pVar 缓冲区
         * @param Bit 位数指示，从第一个字节的最高位为0开始计数
         */
        static inline void PointSet ( void *pVar, int Bit )
        {
            VarSet ( * ( ( BYTE * ) ( pVar ) + Bit / 8 ), 7 - Bit % 8 );
        }

        /**
        * 设置指定缓冲区的指定位为0
        * @param pVar 缓冲区
        * @param Bit 位数指示，从第一个字节的最高位为0开始计数
         */
        static inline void PointZero ( void *pVar, int Bit )
        {
            VarZero ( * ( ( BYTE * ) ( pVar ) + Bit / 8 ), 7 - Bit % 8 );
        }

        /**
         * 测试指定缓冲区的指定位
         * @param pVar 缓冲区
         * @param Bit 位数指示，从第一个字节的最高位为0开始计数
         * @return true/false
         */
        static inline bool  PointTest ( const void *pVar, int Bit )
        {
            return VarTest ( * ( ( BYTE * ) ( pVar ) + Bit / 8 ), 7 - Bit % 8 );
        }

        /**
         * 打印指定缓冲区的的位列表
         * @param pVar 缓冲区
         * @param BitCount 位总数
         */
        static inline void PointPrint ( const void *pVar, int BitCount )
        {
            cout << endl;

            for ( int i = 0;i < BitCount;i++ )
            {
                if ( ( i % 8 == 0 ) && ( i != 0 ) )
                    cout << " ";

                cout << PointTest ( pVar, i );
            }

            cout << endl;
        }


        /**
         * 从源缓冲区的指定位偏移拷贝指定的位数到目标缓冲区的指定位偏离处
         * 所有的位偏移为从最高位为0起计数
         * @param pSrc 源数据指针
         * @param SrcBitPos 源数据起始偏离的位
         * @param pDest 目标数据区指针
         * @param DestBitPos 目标区的位偏移
         * @param BitCount 需要拷贝的位数
         */
        static inline void CopyPointBits ( const void *pSrc, int SrcBitPos, void *pDest, int DestBitPos, int BitCount )
        {
            int i;

            for ( i = 0;i < BitCount;i++ )
            {
                if ( PointTest ( pSrc, SrcBitPos + i ) )
                {
                    PointSet ( pDest, DestBitPos + i );
                }
                else
                {
                    PointZero ( pDest, DestBitPos + i );
                }
            }
        }

        /**
         * 从数据源中拷贝指定的位到目标位置，0为最低位
         * SRC     1111 0110 1010 0101
         * DEST    0000 0000 0000 0000 0000 0000
         * CP Src-3 5 TO Dest 9
         *          0000 0000 0101 1000 0000 0000
         * @param pDest
         * @param pSrc
         */
        template<typename _Ty>
        static inline void CopyVarBits ( const _Ty &Src, int SrcBitPos, _Ty &Dest, int DestBitPos, int BitCount )
        {
            //对齐数据
            _Ty src_value;
            _Ty mask = 0;

            //首先对齐Src到Dest

            if ( SrcBitPos > DestBitPos )
            {
                src_value = Src >> ( SrcBitPos - DestBitPos );
            }
            else
            {
                src_value = Src << ( DestBitPos - SrcBitPos );
            }

            mask = ( ( _Ty ) ( ( ( ~mask ) >> ( sizeof ( _Ty ) * 8 - DestBitPos - 1 - BitCount ) ) << ( sizeof ( _Ty ) * 8 - BitCount ) ) ) >> ( sizeof ( _Ty ) * 8 - DestBitPos - 1 );

            Dest &= ( ~mask ) ;
            Dest |= ( src_value & mask );
        }

        /**
         * 从一个位串缓冲区取出指定的位串，并放置到指定的变量中
         * 需要注意，位串缓冲区高位为第0个位，但变量在x86平台是低位在前
         * 处理位串高低字节转换
         * !!不要直接使用此函数，使用宏 ZBIT_VAR_FROM_POINT 代替
         * @param OutVar
         * @param pVar
         * @param AvailBitsCount 有效位总数
        */
        template<typename _Ty>
        static inline void VarFromPoint ( _Ty &OutVar, const void *pSrc, const short *BitsPatternArray )
        {
            int i = 0;
            int src_bit_pos = 0;
            int src_byte_pos = 0;
            int need_proc_bit_count = 0;
            int src_can_bits = 0;
            int copy_bit_count = 0;
            OutVar = 0;
            BYTE src_ch;

            while ( BitsPatternArray[i] != 0 )
            {
                if ( BitsPatternArray[i] > 0 )
                {
                    //拷贝位到输出缓冲区
                    copy_bit_count = BitsPatternArray[i];

                    while ( copy_bit_count != 0 )
                    {
                        //获取数据并添加到输出缓冲区
                        src_can_bits = 8 - src_bit_pos;
                        need_proc_bit_count = ( copy_bit_count >= src_can_bits ) ? src_can_bits : copy_bit_count;

                        src_ch = * ( ( ( BYTE * ) pSrc ) + src_byte_pos );
                        OutVar = ( OutVar << need_proc_bit_count ) | ( ( ( BYTE ) ( src_ch << src_bit_pos ) ) >> ( 8 - need_proc_bit_count ) );
                        //处理源位置
                        src_byte_pos += ( src_bit_pos + need_proc_bit_count ) / 8;
                        src_bit_pos = ( src_bit_pos + need_proc_bit_count ) % 8;
                        copy_bit_count -= need_proc_bit_count;
                    }
                }
                else
                {
                    //在Src中跳过指定位
                    src_byte_pos += ( src_bit_pos + ( -BitsPatternArray[i] ) ) / 8;
                    src_bit_pos = ( src_bit_pos + ( -BitsPatternArray[i] ) ) % 8;
                }

                i++;
            }
        }

        template<typename _Ty>
        static inline void VarToPoint ( const _Ty &Var, void *pDest, const short *BitsPatternArray )
        {
            int i = 0;
            int dest_bit_pos = 0;
            int dest_byte_pos = 0;
            int need_proc_bit_count = 0;
            int dest_can_bits = 0;
            int copy_bit_count = 0;

            //首先计算总位数
            int src_proc_bit_count = 0;

            for ( i = 0;BitsPatternArray[i] != 0;i++ )
            {
                if ( BitsPatternArray[i] > 0 )
                    src_proc_bit_count += BitsPatternArray[i];
            }

            //将指定的位复制到目标缓冲区
            for ( i = 0;BitsPatternArray[i] != 0;i++ )
            {
                if ( BitsPatternArray[i] > 0 )
                {
                    //拷贝位到输出缓冲区
                    copy_bit_count = BitsPatternArray[i];

                    while ( copy_bit_count != 0 )
                    {
                        //获取数据并添加到输出缓冲区
                        dest_can_bits = 8 - dest_bit_pos;
                        need_proc_bit_count = ( copy_bit_count >= dest_can_bits ) ? dest_can_bits : copy_bit_count;
                        CopyVarBits ( ( BYTE ) ( Var >> ( src_proc_bit_count - need_proc_bit_count ) ), need_proc_bit_count - 1,
                                      * ( ( ( BYTE * ) pDest ) + dest_byte_pos ) , ( 7 - dest_bit_pos ),
                                      need_proc_bit_count );
                        //处理源位置
                        src_proc_bit_count -= need_proc_bit_count;
                        copy_bit_count -= need_proc_bit_count;
                        dest_byte_pos += ( dest_bit_pos + need_proc_bit_count ) / 8;
                        dest_bit_pos = ( dest_bit_pos + need_proc_bit_count ) % 8;
                    }

                }
                else
                {
                    //在Dest中跳过指定位
                    dest_byte_pos += ( dest_bit_pos + ( -BitsPatternArray[i] ) ) / 8;
                    dest_bit_pos = ( dest_bit_pos + ( -BitsPatternArray[i] ) ) % 8;
                }
            }
        }

        template<typename _Ty>
        static inline void MakeVar ( _Ty &Var, int BitsCount, const char *BitsPatternArray )
        {
            int i;
            Var = 0;

            for ( i = 0;BitsPatternArray[i] >= 0 ;i++ )
            {
                if ( BitsPatternArray[i] )
                    VarSet ( Var, BitsCount - i - 1 );
                else
                    VarZero ( Var, BitsCount - i - 1 );
            }
        }

        static inline void MakePoint ( void *pOut, int BitsCount, const char *BitsPatternArray )
        {
            int i;

            for ( i = 0;BitsPatternArray[i] >= 0 ;i++ )
            {
                if ( BitsPatternArray[i] )
                    PointSet ( pOut, BitsCount - i - 1 );
                else
                    PointZero ( pOut, BitsCount - i - 1 );
            }
        }
};

#define ZBIT_VAR_FROM_POINT(out_var,p_src,...) { const short bit_pattern[] = { __VA_ARGS__ , 0 };ZBit::VarFromPoint(out_var,(void *)(p_src),(const short *)bit_pattern);}
#define ZBIT_VAR_TO_POINT(src_var,p_dest,...) { const short bit_pattern[] = { __VA_ARGS__ , 0 };ZBit::VarToPoint(src_var,(void *)(p_dest),(const short *)bit_pattern);}
#define ZBIT_MAKE_VAR(out_var,...) { const char bit_pattern[] = { __VA_ARGS__ , -1 };ZBit::MakeVar(out_var,sizeof(bit_pattern)-1,(const char *)bit_pattern);}

#define ZBIT_MAKE_POINT(p_point,...) { const char bit_pattern[] = { __VA_ARGS__ , -1 };ZBit::MakePoint(p_point,sizeof(bit_pattern)-1,(const char *)bit_pattern);}



#endif
