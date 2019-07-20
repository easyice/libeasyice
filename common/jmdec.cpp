/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "jmdec.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "zbitop.h"

// A little trick to avoid those horrible #if TRACE all over the source code
#if TRACE
#define SYMTRACESTRING(s) strncpy(sym->tracestring,s,TRACESTRING_SIZE)
#else
#define SYMTRACESTRING(s) // do nothing
#endif


//int UsedBits;      // for internal statistics, is adjusted by se_v, ue_v, u_1


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an ue(v) syntax element, the length in bits is stored in
 *    the global p_Dec->UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int ue_v (const char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;

  //assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (&symbol, bitstream);
  //p_Dec->UsedBits+=symbol.len;
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an se(v) syntax element, the length in bits is stored in
 *    the global p_Dec->UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int se_v (const char *tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;

  //assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_se;   // Mapping rule: signed integer
  SYMTRACESTRING(tracestring);
  readSyntaxElement_VLC (&symbol, bitstream);
  //p_Dec->UsedBits+=symbol.len;
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(v) syntax element, the length in bits is stored in
 *    the global p_Dec->UsedBits variable
 *
 * \param LenInBits
 *    length of the syntax element
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int u_v (int LenInBits, const char*tracestring, Bitstream *bitstream)
{
  SyntaxElement symbol;
  symbol.inf = 0;

  //assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  symbol.len = LenInBits;
  SYMTRACESTRING(tracestring);
  readSyntaxElement_FLC (&symbol, bitstream);
  //p_Dec->UsedBits+=symbol.len;

  return symbol.inf;
}

                
/*! 
 *************************************************************************************
 * \brief
 *    ue_v, reads an u(1) syntax element, the length in bits is stored in 
 *    the global UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int u_1 (const char *tracestring, Bitstream *bitstream)
{
  return u_v (1, tracestring, bitstream);
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for ue(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    number in the code table
 ************************************************************************
 */
void linfo_ue(int len, int info, int *value1, int *dummy)
{
  //assert ((len >> 1) < 32);
  *value1 = (int) (((unsigned int) 1 << (len >> 1)) + (unsigned int) (info) - 1);
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for se(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    signed mvd
 ************************************************************************
 */
void linfo_se(int len,  int info, int *value1, int *dummy)
{
  //assert ((len >> 1) < 32);
  unsigned int n = ((unsigned int) 1 << (len >> 1)) + (unsigned int) info - 1;
  *value1 = (n + 1) >> 1;
  if((n & 0x01) == 0)                           // lsb is signed bit
    *value1 = -*value1;
}


/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param info
 *    returns value of the read bits
 * \param bitcount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */
int GetBits (byte buffer[],int totbitoffset,int *info, int bitcount,
             int numbits)
{
  if ((totbitoffset + numbits ) > bitcount) 
  {
    return -1;
  }
  else
  {
    int bitoffset  = 7 - (totbitoffset & 0x07); // bit from start of byte
    int byteoffset = (totbitoffset >> 3); // byte from start of buffer
    int bitcounter = numbits;
    byte *curbyte  = &(buffer[byteoffset]);
    int inf = 0;

    while (numbits--)
    {
      inf <<=1;    
      inf |= ((*curbyte)>> (bitoffset--)) & 0x01;    
      if (bitoffset == -1 ) 
      { //Move onto next byte to get all of numbits
        curbyte++;
        bitoffset = 7;
      }
      // Above conditional could also be avoided using the following:
      // curbyte   -= (bitoffset >> 3);
      // bitoffset &= 0x07;
    }
    *info = inf;

    return bitcounter;           // return absolute offset in bit from start of frame
  }
}

/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    buffer containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param bitcount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */

int ShowBits (byte buffer[],int totbitoffset,int bitcount, int numbits)
{
  if ((totbitoffset + numbits )  > bitcount) 
  {
    return -1;
  }
  else
  {
    int bitoffset  = 7 - (totbitoffset & 0x07); // bit from start of byte
    int byteoffset = (totbitoffset >> 3); // byte from start of buffer
    byte *curbyte  = &(buffer[byteoffset]);
    int inf        = 0;

    while (numbits--)
    {
      inf <<=1;    
      inf |= ((*curbyte)>> (bitoffset--)) & 0x01;

      if (bitoffset == -1 ) 
      { //Move onto next byte to get all of numbits
        curbyte++;
        bitoffset = 7;
      }
    }
    return inf;           // return absolute offset in bit from start of frame
  }
}

/*!
 ************************************************************************
 * \brief
 *  read one exp-golomb VLC symbol
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param  info
 *    returns the value of the symbol
 * \param bytecount
 *    buffer length
 * \return
 *    bits read
 ************************************************************************
 */
int GetVLCSymbol (byte buffer[],int totbitoffset,int *info, int bytecount)
{
  long byteoffset = (totbitoffset >> 3);         // byte from start of buffer
  int  bitoffset  = (7 - (totbitoffset & 0x07)); // bit from start of byte
  int  bitcounter = 1;
  int  len        = 0;
  byte *cur_byte  = &(buffer[byteoffset]);
  int  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;  // control bit for current bit posision

  while (ctr_bit == 0)
  {                 // find leading 1 bit
    len++;
    bitcounter++;
    bitoffset--;
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    byteoffset+= (bitoffset == 7);      
    ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  if (byteoffset + ((len + 7) >> 3) > bytecount)
    return -1;
  else
  {
    // make infoword
    int inf = 0;                          // shortest possible code is 1, then info is always 0    

    while (len--)
    {
      bitoffset --;    
      bitoffset &= 0x07;
      cur_byte  += (bitoffset == 7);
      bitcounter++;
      inf <<= 1;    
      inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
    }

    *info = inf;
    return bitcounter;           // return absolute offset in bit from start of frame
  }
}

/*!
 ************************************************************************
 * \brief
 *    read next UVLC codeword from UVLC-partition and
 *    map it to the corresponding syntax element
 ************************************************************************
 */
int readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
{

  sym->len =  GetVLCSymbol (currStream->streamBuffer, currStream->frame_bitoffset, &(sym->inf), currStream->bitstream_length);
  if (sym->len == -1)
    return -1;

  currStream->frame_bitoffset += sym->len;
  sym->mapping(sym->len, sym->inf, &(sym->value1), &(sym->value2));

#if TRACE
  tracebits(sym->tracestring, sym->len, sym->inf, sym->value1);
#endif

  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    read FLC codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream)
{
  int BitstreamLengthInBits  = (currStream->bitstream_length << 3) + 7;
  
  if ((GetBits(currStream->streamBuffer, currStream->frame_bitoffset, &(sym->inf), BitstreamLengthInBits, sym->len)) < 0)
    return -1;

  sym->value1 = sym->inf;
  currStream->frame_bitoffset += sym->len; // move bitstream pointer

#if TRACE
  tracebits2(sym->tracestring, sym->len, sym->inf);
#endif

  return 1;
}


//========================sps

/*!
 ************************************************************************
 * \brief
 *    Exit program if memory allocation failed (using error())
 * \param where
 *    string indicating which memory allocation failed
 ************************************************************************
 */
void no_mem_exit(const char *where)
{
  printf("Could not allocate memory: %s",where);
   //snprintf(errortext, ET_SIZE, "Could not allocate memory: %s",where);
   //error (errortext, 100);
}

/*!
 *************************************************************************************
 * \brief
 *    Allocates memory for a picture paramater set
 *
 * \return
 *    pointer to a pps
 *************************************************************************************
 */

pic_parameter_set_rbsp_t *AllocPPS ()
 {
   pic_parameter_set_rbsp_t *p;

   if ((p=(pic_parameter_set_rbsp_t*)calloc (1, sizeof (pic_parameter_set_rbsp_t))) == NULL)
     no_mem_exit ("AllocPPS: PPS");
   p->slice_group_id = NULL;
   return p;
 }


/*!
 *************************************************************************************
 * \brief
 *    Allocates memory for am sequence paramater set
 *
 * \return
 *    pointer to a sps
 *************************************************************************************
 */

seq_parameter_set_rbsp_t *AllocSPS ()
 {
   seq_parameter_set_rbsp_t *p;

   if ((p=(seq_parameter_set_rbsp_t*)calloc (1, sizeof (seq_parameter_set_rbsp_t))) == NULL)
     no_mem_exit ("AllocSPS: SPS");
   return p;
 }


/*!
 *************************************************************************************
 * \brief
 *    Frees a picture parameter set
 *
 * \param pps to be freed
 *   Picture parameter set to be freed
 *************************************************************************************
 */

 void FreePPS (pic_parameter_set_rbsp_t *pps)
 {
   assert (pps != NULL);
   if (pps->slice_group_id != NULL) 
     free (pps->slice_group_id);
   free (pps);
 }


 /*!
 *************************************************************************************
 * \brief
 *    Frees a sps
 *
 * \param sps
 *   Sequence parameter set to be freed
 *************************************************************************************
 */

 void FreeSPS (seq_parameter_set_rbsp_t *sps)
 {
   assert (sps != NULL);
   free (sps);
 }

void InitVUI(seq_parameter_set_rbsp_t *sps)
{
  sps->vui_seq_parameters.matrix_coefficients = 2;
}


int ReadVUI(Bitstream *s, seq_parameter_set_rbsp_t *sps)
{
  if (sps->vui_parameters_present_flag)
  {
    sps->vui_seq_parameters.aspect_ratio_info_present_flag = (Boolean)u_1  ("VUI: aspect_ratio_info_present_flag"   , s);
    if (sps->vui_seq_parameters.aspect_ratio_info_present_flag)
    {
      sps->vui_seq_parameters.aspect_ratio_idc             = u_v  ( 8, "VUI: aspect_ratio_idc"              , s);
      if (255==sps->vui_seq_parameters.aspect_ratio_idc)
      {
        sps->vui_seq_parameters.sar_width                  = (unsigned short) u_v  (16, "VUI: sar_width"                     , s);
        sps->vui_seq_parameters.sar_height                 = (unsigned short) u_v  (16, "VUI: sar_height"                    , s);
      }
    }

    sps->vui_seq_parameters.overscan_info_present_flag     = (Boolean)u_1  ("VUI: overscan_info_present_flag"        , s);
    if (sps->vui_seq_parameters.overscan_info_present_flag)
    {
      sps->vui_seq_parameters.overscan_appropriate_flag    = (Boolean)u_1  ("VUI: overscan_appropriate_flag"         , s);
    }

    sps->vui_seq_parameters.video_signal_type_present_flag = (Boolean)u_1  ("VUI: video_signal_type_present_flag"    , s);
    if (sps->vui_seq_parameters.video_signal_type_present_flag)
    {
      sps->vui_seq_parameters.video_format                    = u_v  ( 3,"VUI: video_format"                      , s);
      sps->vui_seq_parameters.video_full_range_flag           = (Boolean)u_1  (   "VUI: video_full_range_flag"             , s);
      sps->vui_seq_parameters.colour_description_present_flag = (Boolean)u_1  (   "VUI: color_description_present_flag"    , s);
      if(sps->vui_seq_parameters.colour_description_present_flag)
      {
        sps->vui_seq_parameters.colour_primaries              = u_v  ( 8,"VUI: colour_primaries"                  , s);
        sps->vui_seq_parameters.transfer_characteristics      = u_v  ( 8,"VUI: transfer_characteristics"          , s);
        sps->vui_seq_parameters.matrix_coefficients           = u_v  ( 8,"VUI: matrix_coefficients"               , s);
      }
    }
    sps->vui_seq_parameters.chroma_location_info_present_flag = (Boolean)u_1  (   "VUI: chroma_loc_info_present_flag"      , s);
    if(sps->vui_seq_parameters.chroma_location_info_present_flag)
    {
      sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = ue_v  ( "VUI: chroma_sample_loc_type_top_field"    , s);
      sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = ue_v  ( "VUI: chroma_sample_loc_type_bottom_field" , s);
    }
    sps->vui_seq_parameters.timing_info_present_flag          = (Boolean)u_1  ("VUI: timing_info_present_flag"           , s);
    if (sps->vui_seq_parameters.timing_info_present_flag)
    {
      sps->vui_seq_parameters.num_units_in_tick               = u_v  (32,"VUI: num_units_in_tick"               , s);
      sps->vui_seq_parameters.time_scale                      = u_v  (32,"VUI: time_scale"                      , s);
      sps->vui_seq_parameters.fixed_frame_rate_flag           = (Boolean)u_1  (   "VUI: fixed_frame_rate_flag"           , s);
    }
    sps->vui_seq_parameters.nal_hrd_parameters_present_flag   = (Boolean)u_1  ("VUI: nal_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag)
    {
      ReadHRDParameters(s, &(sps->vui_seq_parameters.nal_hrd_parameters));
    }
    sps->vui_seq_parameters.vcl_hrd_parameters_present_flag   = (Boolean)u_1  ("VUI: vcl_hrd_parameters_present_flag"    , s);
    if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      ReadHRDParameters(s, &(sps->vui_seq_parameters.vcl_hrd_parameters));
    }
    if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
    {
      sps->vui_seq_parameters.low_delay_hrd_flag             =  (Boolean)u_1  ("VUI: low_delay_hrd_flag"                 , s);
    }
    sps->vui_seq_parameters.pic_struct_present_flag          =  (Boolean)u_1  ("VUI: pic_struct_present_flag   "         , s);
    sps->vui_seq_parameters.bitstream_restriction_flag       =  (Boolean)u_1  ("VUI: bitstream_restriction_flag"         , s);
    if (sps->vui_seq_parameters.bitstream_restriction_flag)
    {
      sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  (Boolean)u_1  ("VUI: motion_vectors_over_pic_boundaries_flag", s);
      sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  ue_v ("VUI: max_bytes_per_pic_denom"                , s);
      sps->vui_seq_parameters.max_bits_per_mb_denom                   =  ue_v ("VUI: max_bits_per_mb_denom"                  , s);
      sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  ue_v ("VUI: log2_max_mv_length_horizontal"          , s);
      sps->vui_seq_parameters.log2_max_mv_length_vertical             =  ue_v ("VUI: log2_max_mv_length_vertical"            , s);
      sps->vui_seq_parameters.num_reorder_frames                      =  ue_v ("VUI: num_reorder_frames"                     , s);
      sps->vui_seq_parameters.max_dec_frame_buffering                 =  ue_v ("VUI: max_dec_frame_buffering"                , s);
    }
  }

  return 0;
}

// syntax for scaling list matrix values
void Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, Bitstream *s)
{
  int j, scanj;
  int delta_scale, lastScale, nextScale;

  lastScale      = 8;
  nextScale      = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = se_v (   "   : delta_sl   "                           , s);
      nextScale = (lastScale + delta_scale + 256) % 256;
      *UseDefaultScalingMatrix = (Boolean) (scanj==0 && nextScale==0);
    }

    scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
    lastScale = scalingList[scanj];
  }
}

// fill sps with content of p
int InterpretSPS(Bitstream* s,seq_parameter_set_rbsp_t *sps)
{
  unsigned i;
  unsigned n_ScalingList;
  int reserved_zero;

  assert (sps != NULL);

//  int UsedBits = 0;

  sps->profile_idc                            = u_v  (8, "SPS: profile_idc"                           , s);

  if ((sps->profile_idc!=BASELINE       ) &&
      (sps->profile_idc!=MAIN           ) &&
      (sps->profile_idc!=EXTENDED       ) &&
      (sps->profile_idc!=FREXT_HP       ) &&
      (sps->profile_idc!=FREXT_Hi10P    ) &&
      (sps->profile_idc!=FREXT_Hi422    ) &&
      (sps->profile_idc!=FREXT_Hi444    ) &&
      (sps->profile_idc!=FREXT_CAVLC444 )
#if (MVC_EXTENSION_ENABLE)
      && (sps->profile_idc!=MVC_HIGH)
      && (sps->profile_idc!=STEREO_HIGH)
#endif
      )
  {
    printf("Invalid Profile IDC (%d) encountered. \n", sps->profile_idc);
    return -1;
  }

  sps->constrained_set0_flag                  = (Boolean)u_1  (   "SPS: constrained_set0_flag"                 , s);
  sps->constrained_set1_flag                  = (Boolean)u_1  (   "SPS: constrained_set1_flag"                 , s);
  sps->constrained_set2_flag                  = (Boolean)u_1  (   "SPS: constrained_set2_flag"                 , s);
  sps->constrained_set3_flag                  = (Boolean)u_1  (   "SPS: constrained_set3_flag"                 , s);
#if (MVC_EXTENSION_ENABLE)
  sps->constrained_set4_flag                  = u_1  (   "SPS: constrained_set4_flag"                 , s);
  reserved_zero                               = u_v  (3, "SPS: reserved_zero_3bits"                   , s);
#else
  reserved_zero                               = u_v  (4, "SPS: reserved_zero_4bits"                   , s);
#endif
  //assert (reserved_zero==0);

  sps->level_idc                              = u_v  (8, "SPS: level_idc"                             , s);

  sps->seq_parameter_set_id                   = ue_v ("SPS: seq_parameter_set_id"                     , s);

  // Fidelity Range Extensions stuff
  sps->chroma_format_idc = 1;
  sps->bit_depth_luma_minus8   = 0;
  sps->bit_depth_chroma_minus8 = 0;
  //p_Vid->lossless_qpprime_flag   = 0;
  byte lossless_qpprime_flag   = 0;
  sps->separate_colour_plane_flag = 0;

  if((sps->profile_idc==FREXT_HP   ) ||
     (sps->profile_idc==FREXT_Hi10P) ||
     (sps->profile_idc==FREXT_Hi422) ||
     (sps->profile_idc==FREXT_Hi444) ||
     (sps->profile_idc==FREXT_CAVLC444)
#if (MVC_EXTENSION_ENABLE)
     || (sps->profile_idc==MVC_HIGH)
     || (sps->profile_idc==STEREO_HIGH)
#endif
     )
  {
    sps->chroma_format_idc                      = ue_v ("SPS: chroma_format_idc"                       , s);

    if(sps->chroma_format_idc == YUV444)
    {
      sps->separate_colour_plane_flag           = u_1  ("SPS: separate_colour_plane_flag"              , s);
    }

    sps->bit_depth_luma_minus8                  = ue_v ("SPS: bit_depth_luma_minus8"                   , s);
    sps->bit_depth_chroma_minus8                = ue_v ("SPS: bit_depth_chroma_minus8"                 , s);
    //checking;
    if((sps->bit_depth_luma_minus8+8 > sizeof(imgpel)*8) || (sps->bit_depth_chroma_minus8+8> sizeof(imgpel)*8))
      printf ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.\n");

    lossless_qpprime_flag                  = u_1  ("SPS: lossless_qpprime_y_zero_flag"            , s);

    sps->seq_scaling_matrix_present_flag        = (Boolean)u_1  (   "SPS: seq_scaling_matrix_present_flag"       , s);
    
    if(sps->seq_scaling_matrix_present_flag)
    {
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        sps->seq_scaling_list_present_flag[i]   = u_1  (   "SPS: seq_scaling_list_present_flag"         , s);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
  }

  sps->log2_max_frame_num_minus4              = ue_v ("SPS: log2_max_frame_num_minus4"                , s);
  sps->pic_order_cnt_type                     = ue_v ("SPS: pic_order_cnt_type"                       , s);

  if (sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4"           , s);
  else if (sps->pic_order_cnt_type == 1)
  {
    sps->delta_pic_order_always_zero_flag      = (Boolean)u_1  ("SPS: delta_pic_order_always_zero_flag"       , s);
    sps->offset_for_non_ref_pic                = se_v ("SPS: offset_for_non_ref_pic"                 , s);
    sps->offset_for_top_to_bottom_field        = se_v ("SPS: offset_for_top_to_bottom_field"         , s);
    sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle"  , s);
    for(i=0; (i<sps->num_ref_frames_in_pic_order_cnt_cycle) && (i < MAXnum_ref_frames_in_pic_order_cnt_cycle); i++)
      sps->offset_for_ref_frame[i]               = se_v ("SPS: offset_for_ref_frame[i]"              , s);
  }
  sps->num_ref_frames                        = ue_v ("SPS: num_ref_frames"                         , s);
  sps->gaps_in_frame_num_value_allowed_flag  = (Boolean)u_1  ("SPS: gaps_in_frame_num_value_allowed_flag"   , s);
  sps->pic_width_in_mbs_minus1               = ue_v ("SPS: pic_width_in_mbs_minus1"                , s);
  sps->pic_height_in_map_units_minus1        = ue_v ("SPS: pic_height_in_map_units_minus1"         , s);
  sps->frame_mbs_only_flag                   = (Boolean)u_1  ("SPS: frame_mbs_only_flag"                    , s);
  if (!sps->frame_mbs_only_flag)
  {
    sps->mb_adaptive_frame_field_flag        = (Boolean)u_1  ("SPS: mb_adaptive_frame_field_flag"           , s);
  }
  sps->direct_8x8_inference_flag             = (Boolean)u_1  ("SPS: direct_8x8_inference_flag"              , s);
  sps->frame_cropping_flag                   = (Boolean)u_1  ("SPS: frame_cropping_flag"                    , s);

  if (sps->frame_cropping_flag)
  {
    sps->frame_cropping_rect_left_offset      = ue_v ("SPS: frame_cropping_rect_left_offset"           , s);
    sps->frame_cropping_rect_right_offset     = ue_v ("SPS: frame_cropping_rect_right_offset"          , s);
    sps->frame_cropping_rect_top_offset       = ue_v ("SPS: frame_cropping_rect_top_offset"            , s);
    sps->frame_cropping_rect_bottom_offset    = ue_v ("SPS: frame_cropping_rect_bottom_offset"         , s);
  }
  sps->vui_parameters_present_flag           = (Boolean) u_1  ("SPS: vui_parameters_present_flag"      , s);

  InitVUI(sps);
  ReadVUI(s, sps);

  sps->Valid = TRUE;
  return 0;
  
}

int ModifySPS(Bitstream* s,seq_parameter_set_rbsp_t *sps)
{
  unsigned i;
  unsigned n_ScalingList;
  int reserved_zero;

  assert (sps != NULL);

//  int UsedBits = 0;

  sps->profile_idc                            = u_v  (8, "SPS: profile_idc"                           , s);

  if ((sps->profile_idc!=BASELINE       ) &&
      (sps->profile_idc!=MAIN           ) &&
      (sps->profile_idc!=EXTENDED       ) &&
      (sps->profile_idc!=FREXT_HP       ) &&
      (sps->profile_idc!=FREXT_Hi10P    ) &&
      (sps->profile_idc!=FREXT_Hi422    ) &&
      (sps->profile_idc!=FREXT_Hi444    ) &&
      (sps->profile_idc!=FREXT_CAVLC444 )
#if (MVC_EXTENSION_ENABLE)
      && (sps->profile_idc!=MVC_HIGH)
      && (sps->profile_idc!=STEREO_HIGH)
#endif
      )
  {
    printf("Invalid Profile IDC (%d) encountered. \n", sps->profile_idc);
    return -1;
  }

  sps->constrained_set0_flag                  = (Boolean)u_1  (   "SPS: constrained_set0_flag"                 , s);
  sps->constrained_set1_flag                  = (Boolean)u_1  (   "SPS: constrained_set1_flag"                 , s);
  sps->constrained_set2_flag                  = (Boolean)u_1  (   "SPS: constrained_set2_flag"                 , s);
  sps->constrained_set3_flag                  = (Boolean)u_1  (   "SPS: constrained_set3_flag"                 , s);
#if (MVC_EXTENSION_ENABLE)
  sps->constrained_set4_flag                  = u_1  (   "SPS: constrained_set4_flag"                 , s);
  reserved_zero                               = u_v  (3, "SPS: reserved_zero_3bits"                   , s);
#else
  reserved_zero                               = u_v  (4, "SPS: reserved_zero_4bits"                   , s);
#endif
  //assert (reserved_zero==0);

  sps->level_idc                              = u_v  (8, "SPS: level_idc"                             , s);

  sps->seq_parameter_set_id                   = ue_v ("SPS: seq_parameter_set_id"                     , s);

  // Fidelity Range Extensions stuff
  sps->chroma_format_idc = 1;
  sps->bit_depth_luma_minus8   = 0;
  sps->bit_depth_chroma_minus8 = 0;
  //p_Vid->lossless_qpprime_flag   = 0;
  byte lossless_qpprime_flag   = 0;
  sps->separate_colour_plane_flag = 0;

  if((sps->profile_idc==FREXT_HP   ) ||
     (sps->profile_idc==FREXT_Hi10P) ||
     (sps->profile_idc==FREXT_Hi422) ||
     (sps->profile_idc==FREXT_Hi444) ||
     (sps->profile_idc==FREXT_CAVLC444)
#if (MVC_EXTENSION_ENABLE)
     || (sps->profile_idc==MVC_HIGH)
     || (sps->profile_idc==STEREO_HIGH)
#endif
     )
  {
    sps->chroma_format_idc                      = ue_v ("SPS: chroma_format_idc"                       , s);

    if(sps->chroma_format_idc == YUV444)
    {
      sps->separate_colour_plane_flag           = u_1  ("SPS: separate_colour_plane_flag"              , s);
    }

    sps->bit_depth_luma_minus8                  = ue_v ("SPS: bit_depth_luma_minus8"                   , s);
    sps->bit_depth_chroma_minus8                = ue_v ("SPS: bit_depth_chroma_minus8"                 , s);
    //checking;
    if((sps->bit_depth_luma_minus8+8 > sizeof(imgpel)*8) || (sps->bit_depth_chroma_minus8+8> sizeof(imgpel)*8))
      printf ("Source picture has higher bit depth than imgpel data type. \nPlease recompile with larger data type for imgpel.\n");

    lossless_qpprime_flag                  = u_1  ("SPS: lossless_qpprime_y_zero_flag"            , s);

    sps->seq_scaling_matrix_present_flag        = (Boolean)u_1  (   "SPS: seq_scaling_matrix_present_flag"       , s);
    
    if(sps->seq_scaling_matrix_present_flag)
    {
      n_ScalingList = (sps->chroma_format_idc != YUV444) ? 8 : 12;
      for(i=0; i<n_ScalingList; i++)
      {
        sps->seq_scaling_list_present_flag[i]   = u_1  (   "SPS: seq_scaling_list_present_flag"         , s);
        if(sps->seq_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(sps->ScalingList4x4[i], 16, &sps->UseDefaultScalingMatrix4x4Flag[i], s);
          else
            Scaling_List(sps->ScalingList8x8[i-6], 64, &sps->UseDefaultScalingMatrix8x8Flag[i-6], s);
        }
      }
    }
  }

  sps->log2_max_frame_num_minus4              = ue_v ("SPS: log2_max_frame_num_minus4"                , s);
  sps->pic_order_cnt_type                     = ue_v ("SPS: pic_order_cnt_type"                       , s);

  if (sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4"           , s);
  else if (sps->pic_order_cnt_type == 1)
  {
    sps->delta_pic_order_always_zero_flag      = (Boolean)u_1  ("SPS: delta_pic_order_always_zero_flag"       , s);
    sps->offset_for_non_ref_pic                = se_v ("SPS: offset_for_non_ref_pic"                 , s);
    sps->offset_for_top_to_bottom_field        = se_v ("SPS: offset_for_top_to_bottom_field"         , s);
    sps->num_ref_frames_in_pic_order_cnt_cycle = ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle"  , s);
    for(i=0; (i<sps->num_ref_frames_in_pic_order_cnt_cycle) && (i < MAXnum_ref_frames_in_pic_order_cnt_cycle); i++)
      sps->offset_for_ref_frame[i]               = se_v ("SPS: offset_for_ref_frame[i]"              , s);
  }
  
  int tmp = s->frame_bitoffset;
  //printf("tmp:%d\n",tmp);
  sps->num_ref_frames                        = ue_v ("SPS: num_ref_frames"                         , s);
  //printf("num_ref_frames len :%d\n",s->frame_bitoffset-tmp);
  //ZBIT_VAR_TO_POINT(0x10,s->streamBuffer,-tmp,s->frame_bitoffset-tmp);
  tmp = s->frame_bitoffset;
  ZBIT_VAR_TO_POINT(1,s->streamBuffer,-tmp,1);
  sps->gaps_in_frame_num_value_allowed_flag  = (Boolean)u_1  ("SPS: gaps_in_frame_num_value_allowed_flag"   , s);
  
  return 0;
  
}

int ReadHRDParameters(Bitstream *s, hrd_parameters_t *hrd)
{
  unsigned int SchedSelIdx;

  hrd->cpb_cnt_minus1                                      = ue_v (   "VUI: cpb_cnt_minus1"                       , s);
  hrd->bit_rate_scale                                      = u_v  ( 4,"VUI: bit_rate_scale"                       , s);
  hrd->cpb_size_scale                                      = u_v  ( 4,"VUI: cpb_size_scale"                       , s);

  for( SchedSelIdx = 0; (SchedSelIdx <= hrd->cpb_cnt_minus1) && (SchedSelIdx < MAXIMUMVALUEOFcpb_cnt); SchedSelIdx++ )
  {
    hrd->bit_rate_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: bit_rate_value_minus1"                  , s);
    hrd->cpb_size_value_minus1[ SchedSelIdx ]             = ue_v  ( "VUI: cpb_size_value_minus1"                  , s);
    hrd->cbr_flag[ SchedSelIdx ]                          = u_1   ( "VUI: cbr_flag"                               , s);
  }

  hrd->initial_cpb_removal_delay_length_minus1            = u_v  ( 5,"VUI: initial_cpb_removal_delay_length_minus1" , s);
  hrd->cpb_removal_delay_length_minus1                    = u_v  ( 5,"VUI: cpb_removal_delay_length_minus1"         , s);
  hrd->dpb_output_delay_length_minus1                     = u_v  ( 5,"VUI: dpb_output_delay_length_minus1"          , s);
  hrd->time_offset_length                                 = u_v  ( 5,"VUI: time_offset_length"          , s);

  return 0;
}

/*!
************************************************************************
* \brief
*    Converts Encapsulated Byte Sequence Packets to RBSP
* \param streamBuffer
*    pointer to data stream
* \param end_bytepos
*    size of data stream
* \param begin_bytepos
*    Position after beginning
************************************************************************/


int EBSPtoRBSP(byte *streamBuffer, int end_bytepos, int begin_bytepos)
{
  int i, j, count;
  count = 0;

  if(end_bytepos < begin_bytepos)
    return end_bytepos;

  j = begin_bytepos;

  for(i = begin_bytepos; i < end_bytepos; ++i)
  { //starting from begin_bytepos to avoid header information
    //in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
    if(count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] < 0x03) 
      return -1;
    if(count == ZEROBYTES_SHORTSTARTCODE && streamBuffer[i] == 0x03)
    {
      //check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
      if((i < end_bytepos-1) && (streamBuffer[i+1] > 0x03))
        return -1;
      //if cabac_zero_word is used, the final byte of this NAL unit(0x03) is discarded, and the last two bytes of RBSP must be 0x0000
      if(i == end_bytepos-1)
        return j;

      ++i;
      count = 0;
    }
    streamBuffer[j] = streamBuffer[i];
    if(streamBuffer[i] == 0x00)
      ++count;
    else
      count = 0;
    ++j;
  }

  return j;
}

/*!
 *************************************************************************************
 * \brief
 *    Converts a NALU to an RBSP
 *
 * \param
 *    nalu: nalu structure to be filled
 *
 * \return
 *    length of the RBSP in bytes
 *************************************************************************************
 */

static int NALUtoRBSP (byte *buf,int len)
{
  assert (buf != NULL);

  int retlen = EBSPtoRBSP (buf, len, 1) ;

  return retlen ;
}

int UnPkt (byte *buf,int len)
{
  assert (buf != NULL);

  int retlen = EBSPtoRBSP (buf, len, 0) ;

  return retlen ;
}

