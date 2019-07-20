
/*!
 *************************************************************************************
 * \file annexb.h
 *
 * \brief
 *    Annex B byte stream buffer handling.
 *
 *************************************************************************************
 */

#ifndef _ANNEXB_H_
#define _ANNEXB_H_

#include "nalucommon.h"

typedef struct annex_b_struct 
{
  int  BitStreamFile;                //!< the bit stream file
  byte *iobuffer;
  byte *iobufferread;
  int bytesinbuffer;
  int is_eof;
  int iIOBufferSize;

  int IsFirstByteStreamNALU;
  int nextstartcodebytes;
  byte *Buf;  
} ANNEXB_t;

extern int  GetAnnexbNALU  (VideoParameters *p_Vid, NALU_t *nalu);
extern void OpenAnnexBFile (VideoParameters *p_Vid, char *fn);
extern void CloseAnnexBFile(VideoParameters *p_Vid);
extern void malloc_annex_b (VideoParameters *p_Vid);
extern void free_annex_b   (VideoParameters *p_Vid);
extern void init_annex_b   (ANNEXB_t *annex_b);
extern void ResetAnnexB    (ANNEXB_t *annex_b);
#endif

