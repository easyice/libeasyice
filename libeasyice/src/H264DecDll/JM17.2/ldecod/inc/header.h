/*!
 *************************************************************************************
 * \file header.h
 *
 * \brief
 *    Prototypes for header.c
 *************************************************************************************
 */

#ifndef _HEADER_H_
#define _HEADER_H_

extern int FirstPartOfSliceHeader(Slice *currSlice,Bitstream *currStream);
extern int RestOfSliceHeader     (Slice *currSlice,seq_parameter_set_rbsp_t *active_sps,pic_parameter_set_rbsp_t *active_pps,Bitstream *currStream);

extern void dec_ref_pic_marking(/*VideoParameters *p_Vid,*/ Bitstream *currStream, Slice *pSlice);

extern void decode_poc(VideoParameters *p_Vid, Slice *pSlice);
extern int  dumppoc   (VideoParameters *p_Vid);

#endif

