/*
MIT License

Copyright  (c) 2009-2019 easyice

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _GMDEC_H_
#define _GMDEC_H_

#include <stdio.h>
#include "gmdefs.h"


/*!
 *  definition of H.264 syntax elements
 *  order of elements follow dependencies for picture reconstruction
 */
/*!
 * \brief   Assignment of old TYPE partition elements to new
 *          elements
 *
 *  old element     | new elements
 *  ----------------+-------------------------------------------------------------------
 *  TYPE_HEADER     | SE_HEADER, SE_PTYPE
 *  TYPE_MBHEADER   | SE_MBTYPE, SE_REFFRAME, SE_INTRAPREDMODE
 *  TYPE_MVD        | SE_MVD
 *  TYPE_CBP        | SE_CBP_INTRA, SE_CBP_INTER
 *  SE_DELTA_QUANT_INTER
 *  SE_DELTA_QUANT_INTRA
 *  TYPE_COEFF_Y    | SE_LUM_DC_INTRA, SE_LUM_AC_INTRA, SE_LUM_DC_INTER, SE_LUM_AC_INTER
 *  TYPE_2x2DC      | SE_CHR_DC_INTRA, SE_CHR_DC_INTER
 *  TYPE_COEFF_C    | SE_CHR_AC_INTRA, SE_CHR_AC_INTER
 *  TYPE_EOS        | SE_EOS
*/

#define SE_HEADER           0
#define SE_PTYPE            1
#define SE_MBTYPE           2
#define SE_REFFRAME         3
#define SE_INTRAPREDMODE    4
#define SE_MVD              5
#define SE_CBP_INTRA        6
#define SE_LUM_DC_INTRA     7
#define SE_CHR_DC_INTRA     8
#define SE_LUM_AC_INTRA     9
#define SE_CHR_AC_INTRA     10
#define SE_CBP_INTER        11
#define SE_LUM_DC_INTER     12
#define SE_CHR_DC_INTER     13
#define SE_LUM_AC_INTER     14
#define SE_CHR_AC_INTER     15
#define SE_DELTA_QUANT_INTER      16
#define SE_DELTA_QUANT_INTRA      17
#define SE_BFRAME           18
#define SE_EOS              19
#define SE_MAX_ELEMENTS     20

//Start code and Emulation Prevention need this to be defined in identical manner at encoder and decoder
#define ZEROBYTES_SHORTSTARTCODE 2 //indicates the number of zero bytes in the short start-code prefix


//AVC Profile IDC definitions
typedef enum {
  FREXT_CAVLC444 = 44,       //!< YUV 4:4:4/14 "CAVLC 4:4:4"
  BASELINE       = 66,       //!< YUV 4:2:0/8  "Baseline"
  MAIN           = 77,       //!< YUV 4:2:0/8  "Main"
  EXTENDED       = 88,       //!< YUV 4:2:0/8  "Extended"
  FREXT_HP       = 100,      //!< YUV 4:2:0/8  "High"
  FREXT_Hi10P    = 110,      //!< YUV 4:2:0/10 "High 10"
  FREXT_Hi422    = 122,      //!< YUV 4:2:2/10 "High 4:2:2"
  FREXT_Hi444    = 244,      //!< YUV 4:4:4/14 "High 4:4:4"
  MVC_HIGH       = 118,      //!< YUV 4:2:0/8  "Multiview High"
  STEREO_HIGH    = 128       //!< YUV 4:2:0/8  "Stereo High"
} ProfileIDC;

typedef enum {
  CF_UNKNOWN = -1,     //!< Unknown color format
  YUV400     =  0,     //!< Monochrome
  YUV420     =  1,     //!< 4:2:0
  YUV422     =  2,     //!< 4:2:2
  YUV444     =  3      //!< 4:4:4
} ColorFormat;

#define NO_EC               0   //!< no error concealment necessary
#define EC_REQ              1   //!< error concealment required
#define EC_SYNC             2   //!< search and sync on next header element

#define MAXPARTITIONMODES   2   //!< maximum possible partition modes as defined in assignSE2partition[][]



static const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

/***********************************************************************
 * D a t a    t y p e s   f o r  C A B A C
 ***********************************************************************
 */

//! struct to characterize the state of the arithmetic coding engine
typedef struct
{
  unsigned int    Dlow, Drange;
  unsigned int    Dvalue;
  unsigned int    Dbuffer;
  int             Dbits_to_go;
  byte            *Dcodestrm;
  int             *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment *DecodingEnvironmentPtr;


//! Syntaxelement
typedef struct syntaxelement
{
  int           type;                  //!< type of syntax element for data part.
  int           value1;                //!< numerical value of syntax element
  int           value2;                //!< for blocked symbols, e.g. run/level
  int           len;                   //!< length of code
  int           inf;                   //!< info part of UVLC code
  unsigned int  bitpattern;            //!< UVLC bitpattern
  int           context;               //!< CABAC context
  int           k;                     //!< CABAC context for coeff_count,uv

#if TRACE
  #define       TRACESTRING_SIZE 100           //!< size of trace string
  char          tracestring[TRACESTRING_SIZE]; //!< trace string
#endif

  //! for mapping of UVLC to syntaxElement
  void    (*mapping)(int len, int info, int *value1, int *value2);
  //! used for CABAC: refers to actual coding method of each individual syntax element type
  void  (*reading)(struct syntaxelement *, struct inp_par *, struct img_par *, DecodingEnvironmentPtr);

} SyntaxElement;

//! Bitstream
typedef struct
{
  // CABAC Decoding
  int           read_len;           //!< actual position in the codebuffer, CABAC only
  int           code_len;           //!< overall codebuffer length, CABAC only
  // UVLC Decoding
  int           frame_bitoffset;    //!< actual position in the codebuffer, bit-oriented, UVLC only
  int           bitstream_length;   //!< over codebuffer lnegth, byte oriented, UVLC only
  // ErrorConcealment
  byte          *streamBuffer;      //!< actual codebuffer for read bytes
  int           ei_flag;            //!< error indication, 0: no error, else unspecified error
} Bitstream;



int se_v (const char *tracestring, Bitstream *bitstream);
int ue_v (const char *tracestring, Bitstream *bitstream);
int u_1 (const char *tracestring, Bitstream *bitstream);
int u_v (int LenInBits, const char *tracestring, Bitstream *bitstream);

// UVLC mapping
void linfo_ue(int len, int info, int *value1, int *dummy);
void linfo_se(int len, int info, int *value1, int *dummy);

int  readSyntaxElement_VLC (SyntaxElement *sym, Bitstream *currStream);

int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream);

int  GetVLCSymbol (byte buffer[],int totbitoffset,int *info, int bytecount);

int GetBits (byte buffer[],int totbitoffset,int *info, int bytecount, 
             int numbits);
int ShowBits (byte buffer[],int totbitoffset,int bytecount, int numbits);


//=====================================================================
#define SIZEslice_group_id      (sizeof (int) * 60000)    // should be sufficient for HUGE pictures, need one int per MB in a picture

#define MAXSPS  32
#define MAXPPS  256

#define IMGTYPE                   1    //!< Define imgpel size type. 0 implies byte (cannot handle >8 bit depths) and 1 implies unsigned short
#if (IMGTYPE == 0)
typedef byte   imgpel;           //!< pixel type
typedef uint16 distpel;          //!< distortion type (for pixels)
typedef int32  distblk;          //!< distortion type (for Macroblock)
typedef int32  transpel;         //!< transformed coefficient type
#elif (IMGTYPE == 2)
typedef float imgpel;
typedef float distpel;
typedef float distblk;
typedef int32 transpel;
#else
typedef uint16 imgpel;
typedef uint32 distpel;
typedef int64  distblk;
typedef int32  transpel;
#endif

//! Boolean Type
#ifdef FALSE
#  define Boolean int
#else
typedef enum {
  FALSE,
  TRUE
} Boolean;
#endif

#define MAXIMUMVALUEOFcpb_cnt   32
typedef struct
{
  unsigned int cpb_cnt_minus1;                                   // ue(v)
  unsigned int bit_rate_scale;                                   // u(4)
  unsigned int cpb_size_scale;                                   // u(4)
  unsigned int bit_rate_value_minus1 [MAXIMUMVALUEOFcpb_cnt];    // ue(v)
  unsigned int cpb_size_value_minus1 [MAXIMUMVALUEOFcpb_cnt];    // ue(v)
  unsigned int cbr_flag              [MAXIMUMVALUEOFcpb_cnt];    // u(1)
  unsigned int initial_cpb_removal_delay_length_minus1;          // u(5)
  unsigned int cpb_removal_delay_length_minus1;                  // u(5)
  unsigned int dpb_output_delay_length_minus1;                   // u(5)
  unsigned int time_offset_length;                               // u(5)
} hrd_parameters_t;


typedef struct
{
  Boolean      aspect_ratio_info_present_flag;                   // u(1)
  unsigned int aspect_ratio_idc;                                 // u(8)
  unsigned short sar_width;                                        // u(16)
  unsigned short sar_height;                                       // u(16)
  Boolean      overscan_info_present_flag;                       // u(1)
  Boolean      overscan_appropriate_flag;                        // u(1)
  Boolean      video_signal_type_present_flag;                   // u(1)
  unsigned int video_format;                                     // u(3)
  Boolean      video_full_range_flag;                            // u(1)
  Boolean      colour_description_present_flag;                  // u(1)
  unsigned int colour_primaries;                                 // u(8)
  unsigned int transfer_characteristics;                         // u(8)
  unsigned int matrix_coefficients;                              // u(8)
  Boolean      chroma_location_info_present_flag;                // u(1)
  unsigned int  chroma_sample_loc_type_top_field;                // ue(v)
  unsigned int  chroma_sample_loc_type_bottom_field;             // ue(v)
  Boolean      timing_info_present_flag;                         // u(1)
  unsigned int num_units_in_tick;                                // u(32)
  unsigned int time_scale;                                       // u(32)
  Boolean      fixed_frame_rate_flag;                            // u(1)
  Boolean      nal_hrd_parameters_present_flag;                  // u(1)
  hrd_parameters_t nal_hrd_parameters;                           // hrd_paramters_t
  Boolean      vcl_hrd_parameters_present_flag;                  // u(1)
  hrd_parameters_t vcl_hrd_parameters;                           // hrd_paramters_t
  // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
  Boolean      low_delay_hrd_flag;                               // u(1)
  Boolean      pic_struct_present_flag;                          // u(1)
  Boolean      bitstream_restriction_flag;                       // u(1)
  Boolean      motion_vectors_over_pic_boundaries_flag;          // u(1)
  unsigned int max_bytes_per_pic_denom;                          // ue(v)
  unsigned int max_bits_per_mb_denom;                            // ue(v)
  unsigned int log2_max_mv_length_vertical;                      // ue(v)
  unsigned int log2_max_mv_length_horizontal;                    // ue(v)
  unsigned int num_reorder_frames;                               // ue(v)
  unsigned int max_dec_frame_buffering;                          // ue(v)
} vui_seq_parameters_t;


#define MAXnum_slice_groups_minus1  8
typedef struct
{
  Boolean   Valid;                  // indicates the parameter set is valid
  unsigned int pic_parameter_set_id;                             // ue(v)
  unsigned int seq_parameter_set_id;                             // ue(v)
  Boolean   entropy_coding_mode_flag;                            // u(1)
  Boolean   transform_8x8_mode_flag;                             // u(1)

  Boolean   pic_scaling_matrix_present_flag;                     // u(1)
  int       pic_scaling_list_present_flag[12];                   // u(1)
  int       ScalingList4x4[6][16];                               // se(v)
  int       ScalingList8x8[6][64];                               // se(v)
  Boolean   UseDefaultScalingMatrix4x4Flag[6];
  Boolean   UseDefaultScalingMatrix8x8Flag[6];

  // if( pic_order_cnt_type < 2 )  in the sequence parameter set
  Boolean      bottom_field_pic_order_in_frame_present_flag;                           // u(1)
  unsigned int num_slice_groups_minus1;                          // ue(v)
  unsigned int slice_group_map_type;                        // ue(v)
  // if( slice_group_map_type = = 0 )
  unsigned int run_length_minus1[MAXnum_slice_groups_minus1]; // ue(v)
  // else if( slice_group_map_type = = 2 )
  unsigned int top_left[MAXnum_slice_groups_minus1];         // ue(v)
  unsigned int bottom_right[MAXnum_slice_groups_minus1];     // ue(v)
  // else if( slice_group_map_type = = 3 || 4 || 5
  Boolean   slice_group_change_direction_flag;            // u(1)
  unsigned int slice_group_change_rate_minus1;               // ue(v)
  // else if( slice_group_map_type = = 6 )
  unsigned int pic_size_in_map_units_minus1;             // ue(v)
  byte      *slice_group_id;                              // complete MBAmap u(v)

  int num_ref_idx_l0_active_minus1;                     // ue(v)
  int num_ref_idx_l1_active_minus1;                     // ue(v)
  Boolean   weighted_pred_flag;                               // u(1)
  unsigned int  weighted_bipred_idc;                              // u(2)
  int       pic_init_qp_minus26;                              // se(v)
  int       pic_init_qs_minus26;                              // se(v)
  int       chroma_qp_index_offset;                           // se(v)

  int       second_chroma_qp_index_offset;                    // se(v)

  Boolean   deblocking_filter_control_present_flag;           // u(1)
  Boolean   constrained_intra_pred_flag;                      // u(1)
  Boolean   redundant_pic_cnt_present_flag;                   // u(1)
} pic_parameter_set_rbsp_t;


#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256
typedef struct
{
  Boolean   Valid;                  // indicates the parameter set is valid

  unsigned int profile_idc;                                       // u(8)
  Boolean   constrained_set0_flag;                                // u(1)
  Boolean   constrained_set1_flag;                                // u(1)
  Boolean   constrained_set2_flag;                                // u(1)
  Boolean   constrained_set3_flag;                                // u(1)
#if (MVC_EXTENSION_ENABLE)
  Boolean   constrained_set4_flag;                                // u(1)
#endif
  unsigned  int level_idc;                                        // u(8)
  unsigned  int seq_parameter_set_id;                             // ue(v)
  unsigned  int chroma_format_idc;                                // ue(v)

  Boolean   seq_scaling_matrix_present_flag;                   // u(1)
  int       seq_scaling_list_present_flag[12];                 // u(1)
  int       ScalingList4x4[6][16];                             // se(v)
  int       ScalingList8x8[6][64];                             // se(v)
  Boolean   UseDefaultScalingMatrix4x4Flag[6];
  Boolean   UseDefaultScalingMatrix8x8Flag[6];

  unsigned int bit_depth_luma_minus8;                            // ue(v)
  unsigned int bit_depth_chroma_minus8;                          // ue(v)
  unsigned int log2_max_frame_num_minus4;                        // ue(v)
  unsigned int pic_order_cnt_type;
  // if( pic_order_cnt_type == 0 )
  unsigned int log2_max_pic_order_cnt_lsb_minus4;                 // ue(v)
  // else if( pic_order_cnt_type == 1 )
    Boolean delta_pic_order_always_zero_flag;               // u(1)
    int     offset_for_non_ref_pic;                         // se(v)
    int     offset_for_top_to_bottom_field;                 // se(v)
    unsigned int num_ref_frames_in_pic_order_cnt_cycle;          // ue(v)
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
      int   offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)
  unsigned int num_ref_frames;                                   // ue(v)
  Boolean   gaps_in_frame_num_value_allowed_flag;             // u(1)
  unsigned int pic_width_in_mbs_minus1;                          // ue(v)
  unsigned int pic_height_in_map_units_minus1;                   // ue(v)
  Boolean   frame_mbs_only_flag;                              // u(1)
  // if( !frame_mbs_only_flag )
    Boolean   mb_adaptive_frame_field_flag;                   // u(1)
  Boolean   direct_8x8_inference_flag;                        // u(1)
  Boolean   frame_cropping_flag;                              // u(1)
    unsigned int frame_cropping_rect_left_offset;                // ue(v)
    unsigned int frame_cropping_rect_right_offset;               // ue(v)
    unsigned int frame_cropping_rect_top_offset;                 // ue(v)
    unsigned int frame_cropping_rect_bottom_offset;              // ue(v)
  Boolean   vui_parameters_present_flag;                      // u(1)
    vui_seq_parameters_t vui_seq_parameters;                  // vui_seq_parameters_t
    unsigned  separate_colour_plane_flag;                       // u(1)
#if (MVC_EXTENSION_ENABLE)
    int max_dec_frame_buffering;
#endif
} seq_parameter_set_rbsp_t;

//! Slice
typedef struct slice
{
  //struct video_par    *p_Vid;
  //struct inp_par      *p_Inp;
  //pic_parameter_set_rbsp_t *active_pps;
  //seq_parameter_set_rbsp_t *active_sps;
  int svc_extension_flag;

  // dpb pointer
  //struct decoded_picture_buffer *p_Dpb;

  //slice property;
  int idr_flag;
  int idr_pic_id;
  int nal_reference_idc;                       //!< nal_reference_idc from NAL unit
  int Transform8x8Mode;
  //Boolean is_not_independent;

  int toppoc;      //poc for this top field // POC200301
  int bottompoc;   //poc of bottom field of frame
  int framepoc;    //poc of this frame // POC200301

  //the following is for slice header syntax elements of poc
  // for poc mode 0.
  unsigned int pic_order_cnt_lsb;
  int delta_pic_order_cnt_bottom;
  // for poc mode 1.
  int delta_pic_order_cnt[2];

  // ////////////////////////
  // for POC mode 0:
  signed   int PicOrderCntMsb;

  //signed   int PrevPicOrderCntMsb;
  //unsigned int PrevPicOrderCntLsb;

  // for POC mode 1:
  unsigned int AbsFrameNum;
  int ThisPOC;
  //signed int ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
  //unsigned int PreviousFrameNum, FrameNumOffset;
  //int ExpectedDeltaPerPicOrderCntCycle;
  //int PreviousFrameNumOffset;
  // /////////////////////////

  //information need to move to slice;
  unsigned int current_mb_nr; // bitstream order
  unsigned int num_dec_mb;
  short        current_slice_nr;
  //int mb_x;
  //int mb_y;
  //int block_x;
  //int block_y;
  //int pix_c_x;
  //int pix_c_y;
  int cod_counter;                   //!< Current count of number of skipped macroblocks in a row
  int allrefzero;
  //end;

  int                 mb_aff_frame_flag;
  int                 direct_spatial_mv_pred_flag;       //!< Indicator for direct mode type (1 for Spatial, 0 for Temporal)
  int                 num_ref_idx_active[2];             //!< number of available list references
  //int                 num_ref_idx_l0_active;             //!< number of available list 0 references
  //int                 num_ref_idx_l1_active;             //!< number of available list 1 references

  int                 ei_flag;       //!< 0 if the partArr[0] contains valid information
  int                 qp;
  int                 slice_qp_delta;
  int                 qs;
  int                 slice_qs_delta;
  int                 slice_type;    //!< slice type
  int                 model_number;  //!< cabac model number
  unsigned int        frame_num;   //frame_num for this frame
  unsigned int        field_pic_flag;
  byte                bottom_field_flag;
  //PictureStructure    structure;     //!< Identify picture structure type
  int                 start_mb_nr;   //!< MUST be set by NAL even in case of ei_flag == 1
  int                 end_mb_nr_plus1;
  int                 max_part_nr;
  int                 dp_mode;       //!< data partitioning mode
  int                 current_header;
  int                 next_header;
  int                 last_dquant;

  //slice header information;
  //int colour_plane_id;               //!< colour_plane_id of the current coded slice
  //int redundant_pic_cnt;
  //int sp_switch;                              //!< 1 for switching sp, 0 for normal sp  
  //int slice_group_change_cycle;
  //int redundant_slice_ref_idx;     //!< reference index of redundant slice
  //int no_output_of_prior_pics_flag;
  //int long_term_reference_flag;
  //int adaptive_ref_pic_buffering_flag;
  //DecRefPicMarking_t *dec_ref_pic_marking_buffer;                    //!< stores the memory management control operations

  //char listXsize[6];
  //struct storable_picture **listX[6];

  //  int                 last_mb_nr;    //!< only valid when entropy coding == CABAC
  //DataPartition       *partArr;      //!< array of partitions
  //MotionInfoContexts  *mot_ctx;      //!< pointer to struct of context models for use in CABAC
  //TextureInfoContexts *tex_ctx;      //!< pointer to struct of context models for use in CABAC

  //int mvscale[6][MAX_REFERENCE_PICTURES];

  //int                 ref_pic_list_reordering_flag[2];
  //int                 *reordering_of_pic_nums_idc[2];
  //int                 *abs_diff_pic_num_minus1[2];
  //int                 *long_term_pic_idx[2];

#if (MVC_EXTENSION_ENABLE)
  int									*abs_diff_view_idx_minus1[2];

  int				          view_id;
  int                 inter_view_flag;
  int                 anchor_pic_flag;

  NALUnitHeaderMVCExt_t NaluHeaderMVCExt;
#endif

  //short               DFDisableIdc;     //!< Disable deblocking filter on slice
  //short               DFAlphaC0Offset;  //!< Alpha and C0 offset for filtering slice
  //short               DFBetaOffset;     //!< Beta offset for filtering slice

  int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to

  //int                 dpB_NotPresent;    //!< non-zero, if data partition B is lost
  //int                 dpC_NotPresent;    //!< non-zero, if data partition C is lost

  //Boolean is_reset_coeff;
  //imgpel  ***mb_pred;
  //imgpel  ***mb_rec;
  //int     ***mb_rres;
  //int     ***cof;
  //int     ***fcf;

  //int cofu[16];

  //imgpel **tmp_block_l0;
  //imgpel **tmp_block_l1;  
  //int    **tmp_res;
  //imgpel **tmp_block_l2;
  //imgpel **tmp_block_l3;  

  //// Scaling matrix info
  //int  InvLevelScale4x4_Intra[3][6][4][4];
  //int  InvLevelScale4x4_Inter[3][6][4][4];
  //int  InvLevelScale8x8_Intra[3][6][8][8];
  //int  InvLevelScale8x8_Inter[3][6][8][8];

  //int  *qmatrix[12];

  //// Cabac
  //int  coeff[64]; // one more for EOB
  //int  coeff_ctr;
  //int  pos;  


  //weighted prediction
  //unsigned short weighted_pred_flag;
  //unsigned short weighted_bipred_idc;

  //unsigned short luma_log2_weight_denom;
  //unsigned short chroma_log2_weight_denom;
  //int ***wp_weight;  // weight in [list][index][component] order
  //int ***wp_offset;  // offset in [list][index][component] order
  //int ****wbp_weight; //weight in [list][fw_index][bw_index][component] order
  //short wp_round_luma;
  //short wp_round_chroma;

#if (MVC_EXTENSION_ENABLE)
  int listinterviewidx0;
  int listinterviewidx1;
  struct frame_store **fs_listinterview0;
  struct frame_store **fs_listinterview1;
#endif

  // for signalling to the neighbour logic that this is a deblocker call
  //byte mixedModeEdgeFlag;
  int max_mb_vmv_r;                          //!< maximum vertical motion vector range in luma quarter pixel units for the current level_idc
 // int ref_flag[17];                //!< 0: i-th previous frame is incorrect

  //void (*read_CBP_and_coeffs_from_NAL) (Macroblock *currMB);
  //int  (*decode_one_component     )    (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, struct storable_picture *dec_picture);
  //int  (*readSlice                )    (struct video_par *, struct inp_par *);  
  //int  (*nal_startcode_follows    )    (struct slice*, int );
  //void (*read_motion_info_from_NAL)    (Macroblock *currMB);
  //void (*read_one_macroblock      )    (Macroblock *currMB);
  //void (*interpret_mb_mode        )    (Macroblock *currMB);
  //void (*init_lists               )    (struct slice *currSlice);
  //void (*intrapred_chroma         )    (Macroblock *currMB);
  //void (*linfo_cbp_intra          )    (int len, int info, int *cbp, int *dummy);
  //void (*linfo_cbp_inter          )    (int len, int info, int *cbp, int *dummy);    
  //void (*update_direct_mv_info    )    (Macroblock *currMB);
 /* int erc_mvperMB;
  Macroblock *mb_data;
  struct storable_picture *dec_picture;
  int **siblock;
  byte **ipredmode;*/
  //char  *intra_block;
  //char  chroma_vector_adjustment[6][16];
} Slice;


void no_mem_exit(const char *where);

pic_parameter_set_rbsp_t *AllocPPS ();
seq_parameter_set_rbsp_t *AllocSPS ();

void FreePPS (pic_parameter_set_rbsp_t *pps);
void FreeSPS (seq_parameter_set_rbsp_t *sps);

void InitVUI(seq_parameter_set_rbsp_t *sps);
int ReadVUI(Bitstream *s, seq_parameter_set_rbsp_t *sps);
void Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, Bitstream *s);
int InterpretSPS(Bitstream* s,seq_parameter_set_rbsp_t *sps);
int ReadHRDParameters(Bitstream *s, hrd_parameters_t *hrd);
int ModifySPS(Bitstream* s,seq_parameter_set_rbsp_t *sps);


//pic_parameter_set_rbsp_t *active_pps;
//seq_parameter_set_rbsp_t *active_sps;

int UnPkt (byte *buf,int len);


#endif

