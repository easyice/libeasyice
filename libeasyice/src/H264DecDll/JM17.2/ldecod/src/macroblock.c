/*!
 ***********************************************************************
 * \file macroblock.c
 *
 * \brief
 *     Decode a Macroblock
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
 *    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *    - Jani Lainema                    <jani.lainema@nokia.com>
 *    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
 *    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
 *    - Detlev Marpe                    <marpe@hhi.de>
 *    - Gabi Blaettermann
 *    - Ye-Kui Wang                     <wyk@ieee.org>
 *    - Lowell Winger                   <lwinger@lsil.com>
 *    - Alexis Michael Tourapis         <alexismt@ieee.org>
 ***********************************************************************
*/

#include "contributors.h"

#include <math.h>

#include "block.h"
#include "global.h"
#include "mbuffer.h"
#include "mbuffer_mvc.h"
#include "elements.h"
//#include "errorconcealment.h"
#include "macroblock.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "image.h"
#include "mb_access.h"
#include "biaridecod.h"
#include "transform.h"
#include "mc_prediction.h"
#include "quant.h"
#include "mv_prediction.h"
#include "mb_prediction.h"
#include "fast_memory.h"

#if TRACE
#define TRACE_STRING(s) strncpy(currSE.tracestring, s, TRACESTRING_SIZE)
#define TRACE_DECBITS(i) dectracebitcnt(1)
#define TRACE_PRINTF(s) sprintf(type, "%s", s);
#define TRACE_STRING_P(s) strncpy(currSE->tracestring, s, TRACESTRING_SIZE)
#else
#define TRACE_STRING(s)
#define TRACE_DECBITS(i)
#define TRACE_PRINTF(s) 
#define TRACE_STRING_P(s)
#endif

//! look up tables for FRExt_chroma support
void dectracebitcnt(int count);

static void read_CBP_and_coeffs_from_NAL_CABAC_420 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CABAC_400 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CABAC_422 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CABAC_444 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CAVLC_420 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CAVLC_400 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CAVLC_422 (Macroblock *currMB);
static void read_CBP_and_coeffs_from_NAL_CAVLC_444 (Macroblock *currMB);
static void read_motion_info_from_NAL_p_slice  (Macroblock *currMB);
static void read_motion_info_from_NAL_b_slice  (Macroblock *currMB);
static void read_ipred_modes                   (Macroblock *currMB);
static void read_IPCM_coeffs_from_NAL          (Slice *currSlice, struct datapartition *dP);
static void read_one_macroblock_i_slice        (Macroblock *currMB);
static void read_one_macroblock_p_slice        (Macroblock *currMB);
static void read_one_macroblock_b_slice        (Macroblock *currMB);
static int  decode_one_component_i_slice       (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
static int  decode_one_component_p_slice       (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
static int  decode_one_component_b_slice       (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
static int  decode_one_component_sp_slice      (Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture);
extern void update_direct_types                (Slice *currSlice);

static inline void reset_mv_info(PicMotionParams *mv_info)
{
  mv_info->ref_pic[LIST_0] = NULL;
  mv_info->ref_pic[LIST_1] = NULL;
  mv_info->mv[LIST_0] = zero_mv;
  mv_info->mv[LIST_1] = zero_mv;
  mv_info->ref_idx[LIST_0] = -1;
  mv_info->ref_idx[LIST_1] = -1;
}

/*!
 ************************************************************************
 * \brief
 *    Set context for reference frames
 ************************************************************************
 */
static inline int BType2CtxRef (int btype)
{
  return (btype >= 4);
}

/*!
 ************************************************************************
 * \brief
 *    Function for reading the reference picture indices using VLC
 ************************************************************************
 */
static char readRefPictureIdx_VLC(Macroblock *currMB, SyntaxElement *currSE, DataPartition *dP, char b8mode, int list)
{
#if TRACE
  char tstring[20];   
  sprintf( tstring, "ref_idx_l%d", list); 
  strncpy(currSE->tracestring, tstring, TRACESTRING_SIZE);
#endif
  currSE->context = BType2CtxRef (b8mode);
  currSE->value2 = list;
  dP->readSyntaxElement (currMB, currSE, dP);
  return (char) currSE->value1;
}

/*!
 ************************************************************************
 * \brief
 *    Function for reading the reference picture indices using FLC
 ************************************************************************
 */
static char readRefPictureIdx_FLC(Macroblock *currMB, SyntaxElement *currSE, DataPartition *dP, char b8mode, int list)
{
#if TRACE
  char tstring[20];   
  sprintf( tstring, "ref_idx_l%d", list); 
  strncpy(currSE->tracestring, tstring, TRACESTRING_SIZE);
#endif

  currSE->context = BType2CtxRef (b8mode);
  currSE->len = 1;
  readSyntaxElement_FLC(currSE, dP->bitstream);
  currSE->value1 = 1 - currSE->value1;

  return (char) currSE->value1;
}

/*!
 ************************************************************************
 * \brief
 *    Dummy Function for reading the reference picture indices
 ************************************************************************
 */
static char readRefPictureIdx_Null(Macroblock *currMB, SyntaxElement *currSE, DataPartition *dP, char b8mode, int list)
{
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Function to prepare reference picture indice function pointer
 ************************************************************************
 */
static void prepareListforRefIdx ( Macroblock *currMB, SyntaxElement *currSE, DataPartition *dP, int num_ref_idx_active, int refidx_present)
{
  currMB->readRefPictureIdx = readRefPictureIdx_Null; // Initialize readRefPictureIdx
  if(num_ref_idx_active > 1)
  {
    if (currMB->p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
    {
      currSE->mapping = linfo_ue;
      if (refidx_present)
      {
        if (num_ref_idx_active == 2)
          currMB->readRefPictureIdx = readRefPictureIdx_FLC;        
        else
          currMB->readRefPictureIdx = readRefPictureIdx_VLC;
      }
    }
    else
    {
      currSE->reading = readRefFrame_CABAC;
      if (refidx_present)
        currMB->readRefPictureIdx = readRefPictureIdx_VLC;
    }
  }    
}

void set_chroma_qp(Macroblock* currMB)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  StorablePicture *dec_picture = currMB->p_Slice->dec_picture;
  int i;
  for (i=0; i<2; ++i)
  {
    currMB->qpc[i] = iClip3 ( -p_Vid->bitdepth_chroma_qp_scale, 51, currMB->qp + dec_picture->chroma_qp_offset[i] );
    currMB->qpc[i] = currMB->qpc[i] < 0 ? currMB->qpc[i] : QP_SCALE_CR[currMB->qpc[i]];
    currMB->qp_scaled[i + 1] = currMB->qpc[i] + p_Vid->bitdepth_chroma_qp_scale;
  }
}

/*!
************************************************************************
* \brief
*    updates chroma QP according to luma QP and bit depth
************************************************************************
*/
void update_qp(Macroblock *currMB, int qp)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  currMB->qp = qp;
  currMB->qp_scaled[0] = qp + p_Vid->bitdepth_luma_qp_scale;
  set_chroma_qp(currMB);
  currMB->is_lossless = (Boolean) ((currMB->qp_scaled[0] == 0) && (p_Vid->lossless_qpprime_flag == 1));
}

static void read_delta_quant(SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, const byte *partMap, int type)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
 
  currSE->type = type;

  dP = &(currSlice->partArr[partMap[currSE->type]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
  {
    currSE->mapping = linfo_se;
  }
  else
    currSE->reading= read_dQuant_CABAC;

  TRACE_STRING_P("mb_qp_delta");

  dP->readSyntaxElement(currMB, currSE, dP);
  currMB->delta_quant = (short) currSE->value1;
  if ((currMB->delta_quant < -(26 + p_Vid->bitdepth_luma_qp_scale/2)) || (currMB->delta_quant > (25 + p_Vid->bitdepth_luma_qp_scale/2)))
    error ("mb_qp_delta is out of range", 500);

  currSlice->qp = ((currSlice->qp + currMB->delta_quant + 52 + 2*p_Vid->bitdepth_luma_qp_scale)%(52+p_Vid->bitdepth_luma_qp_scale)) - p_Vid->bitdepth_luma_qp_scale;
  update_qp(currMB, currSlice->qp);
}

/*!
 ************************************************************************
 * \brief
 *    Function to read reference picture indice values
 ************************************************************************
 */
static void readMBRefPictureIdx (SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, PicMotionParams **mv_info, int list, int step_v0, int step_h0)
{
  int k, j, i, j0, i0;
  char refframe;

  for (j0 = 0; j0 < 4; j0 += step_v0)
  {
    currMB->subblock_y = j0 << 2;
    for (i0 = 0; i0 < 4; i0 += step_h0)
    {      
      k = 2 * (j0 >> 1) + (i0 >> 1);

      if ((currMB->b8pdir[k] == list || currMB->b8pdir[k] == BI_PRED) && currMB->b8mode[k] != 0)
      {
        currMB->subblock_x = i0 << 2;
        refframe = currMB->readRefPictureIdx(currMB, currSE, dP, currMB->b8mode[k], list);
        for (j = j0; j < j0 + step_v0; ++j)
          for (i = currMB->block_x + i0; i < currMB->block_x + i0 + step_h0; ++i)
            mv_info[j][i].ref_idx[list] = refframe;
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Function to read reference picture indice values
 ************************************************************************
 */
static void readMBMotionVectors (SyntaxElement *currSE, DataPartition *dP, Macroblock *currMB, int list, int step_h0, int step_v0)
{
  int i, j, k, i4, j4, ii, jj, kk, i0, j0;
  short curr_mvd[2];
  MotionVector pred_mv, curr_mv;
  short (*mvd)[4][2];
  //VideoParameters *p_Vid = currMB->p_Vid;
  PicMotionParams **mv_info = currMB->p_Slice->dec_picture->mv_info;
  PixelPos block[4]; // neighbor blocks

  for (j0=0; j0<4; j0+=step_v0)
  {
    for (i0=0; i0<4; i0+=step_h0)
    {       
      kk = 2 * (j0 >> 1) + (i0 >> 1);

      if ((currMB->b8pdir[kk] == list || currMB->b8pdir[kk]== BI_PRED) && (currMB->b8mode[kk] != 0))//has forward vector
      {
        char cur_ref_idx = mv_info[currMB->block_y+j0][currMB->block_x+i0].ref_idx[list];
        int mv_mode  = currMB->b8mode[kk];
        int step_h = BLOCK_STEP [mv_mode][0];
        int step_v = BLOCK_STEP [mv_mode][1];
        int step_h4 = step_h << 2;
        int step_v4 = step_v << 2;

        for (j = j0; j < j0 + step_v0; j += step_v)
        {
          currMB->subblock_y = j << 2; // position used for context determination
          j4  = currMB->block_y + j;
          mvd = &currMB->mvd [list][j];

          for (i = i0; i < i0 + step_h0; i += step_h)
          {
            currMB->subblock_x = i << 2; // position used for context determination
            i4 = currMB->block_x + i;

            get_neighbors(currMB, block, BLOCK_SIZE * i, BLOCK_SIZE * j, step_h4);

            // first get MV predictor
            currMB->GetMVPredictor (currMB, block, &pred_mv, cur_ref_idx, mv_info, list, BLOCK_SIZE * i, BLOCK_SIZE * j, step_h4, step_v4);

            for (k=0; k < 2; ++k)
            {
#if TRACE
              char tstring[20];   
              sprintf( tstring, "mvd_l%d", list); 
              strncpy(currSE->tracestring, tstring, TRACESTRING_SIZE);
#endif
              currSE->value2   = (k << 1) + list; // identifies the component; only used for context determination
              dP->readSyntaxElement(currMB, currSE, dP);
              curr_mvd[k] = (short) currSE->value1;              
            }

            curr_mv.mv_x = (short)(curr_mvd[0] + pred_mv.mv_x);  // compute motion vector 
            curr_mv.mv_y = (short)(curr_mvd[1] + pred_mv.mv_y);  // compute motion vector 

            for(jj = j4; jj < j4 + step_v; ++jj)
            {
              PicMotionParams *mvinfo = mv_info[jj] + i4;
              for(ii = i4; ii < i4 + step_h; ++ii)
              {
                (mvinfo++)->mv[list] = curr_mv;
              }            
            }

            // Init first line (mvd)
            for(ii = i; ii < i + step_h; ++ii)
            {
              //*((int *) &mvd[0][ii][0]) = *((int *) curr_mvd);
              mvd[0][ii][0] = curr_mvd[0];
              mvd[0][ii][1] = curr_mvd[1];

              //memcpy(&mvd[0][ii][0], curr_mvd,  2 * sizeof(short));
            }              

            // now copy all other lines
            for(jj = 1; jj < step_v; ++jj)
            {
              memcpy(&mvd[jj][i][0], &mvd[0][i][0],  2 * step_h * sizeof(short));
            }
          }
        }
      }
    }
  }
}

void invScaleCoeff(Macroblock *currMB, int level, int run, int qp_per, int i, int j, int i0, int j0, int coef_ctr, const byte (*pos_scan4x4)[2], int (*InvLevelScale4x4)[4])
{
  if (level != 0)    /* leave if level == 0 */
  {
    coef_ctr += run + 1;

    i0 = pos_scan4x4[coef_ctr][0];
    j0 = pos_scan4x4[coef_ctr][1];

    currMB->cbp_blk[0] |= i64_power2((j << 2) + i) ;
    currMB->p_Slice->cof[0][(j<<2) + j0][(i<<2) + i0]= rshift_rnd_sf((level * InvLevelScale4x4[j0][i0]) << qp_per, 4);
  }
}

/*!
 ************************************************************************
 * \brief
 *    initializes the current macroblock
 ************************************************************************
 */
void start_macroblock(Slice *currSlice, Macroblock **currMB)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  StorablePicture *dec_picture = currSlice->dec_picture; //p_Vid->dec_picture;
  int mb_nr = currSlice->current_mb_nr;
  
  *currMB = &currSlice->mb_data[mb_nr]; //&p_Vid->mb_data[mb_nr];   // initialization code deleted, see below, StW  

  (*currMB)->p_Slice = currSlice;
  (*currMB)->p_Vid   = p_Vid;  
  (*currMB)->mbAddrX = mb_nr;

  //assert (mb_nr < (int) p_Vid->PicSizeInMbs);

  /* Update coordinates of the current macroblock */
  if (currSlice->mb_aff_frame_flag)
  {
    (*currMB)->mb.x = (short) (   (mb_nr) % ((2*p_Vid->width) / MB_BLOCK_SIZE));
    (*currMB)->mb.y = (short) (2*((mb_nr) / ((2*p_Vid->width) / MB_BLOCK_SIZE)));

    (*currMB)->mb.y += ((*currMB)->mb.x & 0x01);
    (*currMB)->mb.x >>= 1;
  }
  else
  {
    (*currMB)->mb = PicPos[mb_nr];
  }

  /* Define pixel/block positions */
  (*currMB)->block_x     = (*currMB)->mb.x * BLOCK_SIZE;           /* horizontal block position */
  (*currMB)->block_y     = (*currMB)->mb.y * BLOCK_SIZE;           /* vertical block position */
  (*currMB)->block_y_aff = (*currMB)->block_y;                     /* interlace relative vertical position */
  (*currMB)->pix_x       = (*currMB)->mb.x * MB_BLOCK_SIZE;        /* horizontal luma pixel position */
  (*currMB)->pix_y       = (*currMB)->mb.y * MB_BLOCK_SIZE;        /* vertical luma pixel position */
  (*currMB)->pix_c_x     = (*currMB)->mb.x * p_Vid->mb_cr_size_x;  /* horizontal chroma pixel position */
  (*currMB)->pix_c_y     = (*currMB)->mb.y * p_Vid->mb_cr_size_y;  /* vertical chroma pixel position */

  // reset intra mode
  (*currMB)->is_intra_block = FALSE;

  // Save the slice number of this macroblock. When the macroblock below
  // is coded it will use this to decide if prediction for above is possible
  (*currMB)->slice_nr = (short) currSlice->current_slice_nr;

  dec_picture->slice_id[(*currMB)->mb.y][(*currMB)->mb.x] = (short) currSlice->current_slice_nr;
  //dec_picture->max_slice_id = (short) imax(currSlice->current_slice_nr, dec_picture->max_slice_id);

  CheckAvailabilityOfNeighbors(*currMB);

  // Select appropriate MV predictor function
  init_motion_vector_prediction(*currMB, currSlice->mb_aff_frame_flag);

  set_read_and_store_CBP(currMB, currSlice->active_sps->chroma_format_idc);

  // Reset syntax element entries in MB struct
   //update_qp(*currMB, currSlice->qp);
  (*currMB)->mb_type         = 0;
  (*currMB)->delta_quant     = 0;
  (*currMB)->cbp             = 0;    
  (*currMB)->c_ipred_mode    = DC_PRED_8;

  if (currSlice->slice_type != I_SLICE)
  {
    if (currSlice->slice_type != B_SLICE)
      fast_memset((*currMB)->mvd[0][0][0], 0, MB_BLOCK_PARTITIONS * 2 * sizeof(short));
    else
      fast_memset((*currMB)->mvd[0][0][0], 0, 2 * MB_BLOCK_PARTITIONS * 2 * sizeof(short));
  }
  
  {
    int i;
    for (i = 0; i < 3; i++) 
    {
      (*currMB)->cbp_blk[i] = 0;
      (*currMB)->cbp_bits[i] = 0;
      (*currMB)->cbp_bits_8x8[i] = 0;
    }
  }

  // initialize currSlice->mb_rres
  if (currSlice->is_reset_coeff == FALSE)
  {
    fast_memset(currSlice->mb_rres[0][0], 0, MB_PIXELS * sizeof(int));
    fast_memset(currSlice->mb_rres[1][0], 0, p_Vid->mb_cr_size_x * p_Vid->mb_cr_size_y * sizeof(int));
    fast_memset(currSlice->mb_rres[2][0], 0, p_Vid->mb_cr_size_x * p_Vid->mb_cr_size_y * sizeof(int));
    fast_memset(currSlice->cof[0][0], 0, MB_PIXELS * sizeof(int)); // reset luma coeffs   
    fast_memset(currSlice->cof[1][0], 0, MB_PIXELS * sizeof(int));
    fast_memset(currSlice->cof[2][0], 0, MB_PIXELS * sizeof(int));
    //fast_memset(currSlice->fcf[0][0], 0, MB_PIXELS * sizeof(int)); // reset luma coeffs   
    //fast_memset(currSlice->fcf[1][0], 0, MB_PIXELS * sizeof(int));
    //fast_memset(currSlice->fcf[2][0], 0, MB_PIXELS * sizeof(int));

    currSlice->is_reset_coeff = TRUE;
  }
  
  // store filtering parameters for this MB
  (*currMB)->DFDisableIdc    = currSlice->DFDisableIdc;
  (*currMB)->DFAlphaC0Offset = currSlice->DFAlphaC0Offset;
  (*currMB)->DFBetaOffset    = currSlice->DFBetaOffset;
  (*currMB)->list_offset = 0;
  (*currMB)->mixedModeEdgeFlag=0;
}

/*!
 ************************************************************************
 * \brief
 *    set coordinates of the next macroblock
 *    check end_of_slice condition
 ************************************************************************
 */
Boolean exit_macroblock(Slice *currSlice, int eos_bit)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

 //! The if() statement below resembles the original code, which tested
  //! p_Vid->current_mb_nr == p_Vid->PicSizeInMbs.  Both is, of course, nonsense
  //! In an error prone environment, one can only be sure to have a new
  //! picture by checking the tr of the next slice header!

// printf ("exit_macroblock: FmoGetLastMBOfPicture %d, p_Vid->current_mb_nr %d\n", FmoGetLastMBOfPicture(), p_Vid->current_mb_nr);
  ++(currSlice->num_dec_mb);

  if(currSlice->current_mb_nr == p_Vid->PicSizeInMbs - 1) //if (p_Vid->num_dec_mb == p_Vid->PicSizeInMbs)
  {
    return TRUE;
  }
  // ask for last mb in the slice  CAVLC
  else
  {

    currSlice->current_mb_nr = FmoGetNextMBNr (p_Vid, currSlice->current_mb_nr);

    if (currSlice->current_mb_nr == -1)     // End of Slice group, MUST be end of slice
    {
      assert (currSlice->nal_startcode_follows (currSlice, eos_bit) == TRUE);
      return TRUE;
    }

    if(currSlice->nal_startcode_follows(currSlice, eos_bit) == FALSE)
      return FALSE;

    if(currSlice->slice_type == I_SLICE  || currSlice->slice_type == SI_SLICE || p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
      return TRUE;
    if(currSlice->cod_counter <= 0)
      return TRUE;
    return FALSE;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Interpret the mb mode for P-Frames
 ************************************************************************
 */
static void interpret_mb_mode_P(Macroblock *currMB)
{
  static const short ICBPTAB[6] = {0,16,32,15,31,47};
  short  mbmode = currMB->mb_type;

  if(mbmode < 4)
  {
    currMB->mb_type = mbmode;
    memset(currMB->b8mode, mbmode, 4 * sizeof(char));
    memset(currMB->b8pdir, 0, 4 * sizeof(char));
  }
  else if((mbmode == 4 || mbmode == 5))
  {
    currMB->mb_type = P8x8;
    currMB->p_Slice->allrefzero = (mbmode == 5);
  }
  else if(mbmode == 6)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I4MB;
    memset(currMB->b8mode,IBLOCK, 4 * sizeof(char));
    memset(currMB->b8pdir,    -1, 4 * sizeof(char));
  }
  else if(mbmode == 31)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = IPCM;
    currMB->cbp = -1;
    currMB->i16mode = 0;

    memset(currMB->b8mode, 0, 4 * sizeof(char));
    memset(currMB->b8pdir,-1, 4 * sizeof(char));
  }
  else
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I16MB;
    currMB->cbp = ICBPTAB[((mbmode-7))>>2];
    currMB->i16mode = ((mbmode-7)) & 0x03;
    memset(currMB->b8mode, 0, 4 * sizeof(char));
    memset(currMB->b8pdir,-1, 4 * sizeof(char));
  }
}

/*!
 ************************************************************************
 * \brief
 *    Interpret the mb mode for I-Frames
 ************************************************************************
 */
static void interpret_mb_mode_I(Macroblock *currMB)
{
  static const short ICBPTAB[6] = {0,16,32,15,31,47};
  short mbmode   = currMB->mb_type;

  if (mbmode == 0)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I4MB;
    memset(currMB->b8mode,IBLOCK,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }
  else if(mbmode == 25)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type=IPCM;
    currMB->cbp= -1;
    currMB->i16mode = 0;

    memset(currMB->b8mode,0,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }
  else
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I16MB;
    currMB->cbp= ICBPTAB[(mbmode-1)>>2];
    currMB->i16mode = (mbmode-1) & 0x03;
    memset(currMB->b8mode, 0, 4 * sizeof(char));
    memset(currMB->b8pdir,-1, 4 * sizeof(char));
  }
}

/*!
 ************************************************************************
 * \brief
 *    Interpret the mb mode for B-Frames
 ************************************************************************
 */
static void interpret_mb_mode_B(Macroblock *currMB)
{
  static const short offset2pdir16x16[12]   = {0, 0, 1, 2, 0,0,0,0,0,0,0,0};
  static const short offset2pdir16x8[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},{1,0},
                                             {0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2},{0,0}};
  static const short offset2pdir8x16[22][2] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,1},{0,0},{0,1},{0,0},
                                             {1,0},{0,0},{0,2},{0,0},{1,2},{0,0},{2,0},{0,0},{2,1},{0,0},{2,2}};

  static const short ICBPTAB[6] = {0,16,32,15,31,47};

  short i, mbmode;
  short mbtype  = currMB->mb_type;

  //--- set mbtype, b8type, and b8pdir ---
  if (mbtype == 0)       // direct
  {
    mbmode=0;
    memset(currMB->b8mode,0,4 * sizeof(char));
    memset(currMB->b8pdir,2,4 * sizeof(char));
  }
  else if (mbtype == 23) // intra4x4
  {
    currMB->is_intra_block = TRUE;
    mbmode=I4MB;
    memset(currMB->b8mode,IBLOCK,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }
  else if ((mbtype > 23) && (mbtype < 48) ) // intra16x16
  {
    currMB->is_intra_block = TRUE;
    mbmode=I16MB;
    memset(currMB->b8mode,0,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));

    currMB->cbp     = ICBPTAB[(mbtype-24)>>2];
    currMB->i16mode = (mbtype-24) & 0x03;
  }
  else if (mbtype == 22) // 8x8(+split)
  {
    mbmode=P8x8;       // b8mode and pdir is transmitted in additional codewords
  }
  else if (mbtype < 4)   // 16x16
  {
    mbmode=1;
    memset(currMB->b8mode, 1,4 * sizeof(char));
    memset(currMB->b8pdir, offset2pdir16x16[mbtype], 4 * sizeof(char));
  }
  else if(mbtype == 48)
  {
    currMB->is_intra_block = TRUE;
    mbmode=IPCM;
    memset(currMB->b8mode, 0,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));

    currMB->cbp= -1;
    currMB->i16mode = 0;
  }

  else if ((mbtype & 0x01) == 0) // 16x8
  {
    mbmode=2;
    memset(currMB->b8mode, 2,4 * sizeof(char));
    for(i=0;i<4;++i)
    {
      currMB->b8pdir[i] = (char) offset2pdir16x8 [mbtype][i>>1];
    }
  }
  else
  {
    mbmode=3;
    memset(currMB->b8mode, 3,4 * sizeof(char));
    for(i=0;i<4; ++i)
    {
      currMB->b8pdir[i] = (char) offset2pdir8x16 [mbtype][i&0x01];
    }
  }
  currMB->mb_type = mbmode;
}

/*!
 ************************************************************************
 * \brief
 *    Interpret the mb mode for SI-Frames
 ************************************************************************
 */
static void interpret_mb_mode_SI(Macroblock *currMB)
{
  //VideoParameters *p_Vid = currMB->p_Vid;
  const int ICBPTAB[6] = {0,16,32,15,31,47};
  short mbmode   = currMB->mb_type;

  if (mbmode == 0)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = SI4MB;
    memset(currMB->b8mode,IBLOCK,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
    currMB->p_Slice->siblock[currMB->mb.y][currMB->mb.x]=1;
  }
  else if (mbmode == 1)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I4MB;
    memset(currMB->b8mode,IBLOCK,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }
  else if(mbmode == 26)
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type=IPCM;
    currMB->cbp= -1;
    currMB->i16mode = 0;
    memset(currMB->b8mode,0,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }

  else
  {
    currMB->is_intra_block = TRUE;
    currMB->mb_type = I16MB;
    currMB->cbp= ICBPTAB[(mbmode-2)>>2];
    currMB->i16mode = (mbmode-2) & 0x03;
    memset(currMB->b8mode,0,4 * sizeof(char));
    memset(currMB->b8pdir,-1,4 * sizeof(char));
  }
}


/*!
 ************************************************************************
 * \brief
 *    Set mode interpretation based on slice type
 ************************************************************************
 */
void setup_slice_methods(Slice *currSlice)
{
  switch (currSlice->slice_type)
  {
  case P_SLICE: 
    currSlice->interpret_mb_mode         = interpret_mb_mode_P;
    currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_p_slice;
    currSlice->read_one_macroblock       = read_one_macroblock_p_slice;
    currSlice->decode_one_component      = decode_one_component_p_slice;
    currSlice->update_direct_mv_info     = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->init_lists                = currSlice->view_id ? init_lists_p_slice_mvc : init_lists_p_slice;
#else
    currSlice->init_lists                = init_lists_p_slice;
#endif
    break;
  case SP_SLICE:
    currSlice->interpret_mb_mode         = interpret_mb_mode_P;
    currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_p_slice;
    currSlice->read_one_macroblock       = read_one_macroblock_p_slice;
    currSlice->decode_one_component      = decode_one_component_sp_slice;
    currSlice->update_direct_mv_info     = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->init_lists                = currSlice->view_id ? init_lists_p_slice_mvc : init_lists_p_slice;
#else
    currSlice->init_lists                = init_lists_p_slice;
#endif
    break;
  case B_SLICE:
    currSlice->interpret_mb_mode         = interpret_mb_mode_B;
    currSlice->read_motion_info_from_NAL = read_motion_info_from_NAL_b_slice;
    currSlice->read_one_macroblock       = read_one_macroblock_b_slice;
    currSlice->decode_one_component      = decode_one_component_b_slice;

    update_direct_types(currSlice);

#if (MVC_EXTENSION_ENABLE)
    currSlice->init_lists                = currSlice->view_id ? init_lists_b_slice_mvc : init_lists_b_slice;
#else
    currSlice->init_lists                = init_lists_b_slice;
#endif
    break;
  case I_SLICE: 
    currSlice->interpret_mb_mode         = interpret_mb_mode_I;
    currSlice->read_motion_info_from_NAL = NULL;
    currSlice->read_one_macroblock       = read_one_macroblock_i_slice;
    currSlice->decode_one_component      = decode_one_component_i_slice;
    currSlice->update_direct_mv_info     = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->init_lists                = currSlice->view_id ? init_lists_i_slice_mvc : init_lists_i_slice;
#else
    currSlice->init_lists                = init_lists_i_slice;
#endif
    break;
  case SI_SLICE: 
    currSlice->interpret_mb_mode         = interpret_mb_mode_SI;
    currSlice->read_motion_info_from_NAL = NULL;
    currSlice->read_one_macroblock       = read_one_macroblock_i_slice;
    currSlice->decode_one_component      = decode_one_component_i_slice;
    currSlice->update_direct_mv_info     = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->init_lists                = currSlice->view_id ? init_lists_i_slice_mvc : init_lists_i_slice;
#else
    currSlice->init_lists                = init_lists_i_slice;
#endif
    break;
  default:
    printf("Unsupported slice type\n");
    break;
  }

  if (currSlice->mb_aff_frame_flag)
  {
    currSlice->intrapred_chroma = intrapred_chroma_mbaff;        
  }
  else
    currSlice->intrapred_chroma = intrapred_chroma;   

  switch(currSlice->p_Vid->active_pps->entropy_coding_mode_flag)
  {
  case CABAC:
    if ( currSlice->p_Vid->active_sps->chroma_format_idc==YUV444 && (currSlice->p_Vid->separate_colour_plane_flag == 0) ) 
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CABAC_444;
    else if (currSlice->p_Vid->active_sps->chroma_format_idc == YUV422)
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CABAC_422;
    else if (currSlice->p_Vid->active_sps->chroma_format_idc == YUV400)
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CABAC_400;
    else
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CABAC_420;
    break;
  case CAVLC:
    if ( currSlice->p_Vid->active_sps->chroma_format_idc==YUV444 && (currSlice->p_Vid->separate_colour_plane_flag == 0) ) 
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CAVLC_444;
    else if (currSlice->p_Vid->active_sps->chroma_format_idc == YUV422)
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CAVLC_422;
    else if (currSlice->p_Vid->active_sps->chroma_format_idc == YUV400)
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CAVLC_400;
    else
      currSlice->read_CBP_and_coeffs_from_NAL = read_CBP_and_coeffs_from_NAL_CAVLC_420;
    break;
  default:
    printf("Unsupported entropy coding mode\n");
    break;
  }
}

/*!
 ************************************************************************
 * \brief
 *    init macroblock I and P frames
 ************************************************************************
 */
static void init_macroblock(Macroblock *currMB)
{
  //VideoParameters *p_Vid = currMB->p_Vid;
  int j, i;
  PicMotionParams **mv_info = &currMB->p_Slice->dec_picture->mv_info[currMB->block_y]; //&p_Vid->dec_picture->mv_info[currMB->block_y];

  // reset vectors and pred. modes
  for(j = 0; j < BLOCK_SIZE; ++j)
  {                        
    i = currMB->block_x;
    reset_mv_info(*mv_info + (i++));
    reset_mv_info(*mv_info + (i++));
    reset_mv_info(*mv_info + (i++));
    reset_mv_info(*(mv_info++) + i);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Sets mode for 8x8 block
 ************************************************************************
 */
void SetB8Mode (Macroblock* currMB, int value, int i)
{
  Slice* currSlice = currMB->p_Slice;
  static const char p_v2b8 [ 5] = {4, 5, 6, 7, IBLOCK};
  static const char p_v2pd [ 5] = {0, 0, 0, 0, -1};
  static const char b_v2b8 [14] = {0, 4, 4, 4, 5, 6, 5, 6, 5, 6, 7, 7, 7, IBLOCK};
  static const char b_v2pd [14] = {2, 0, 1, 2, 0, 0, 1, 1, 2, 2, 0, 1, 2, -1};

  if (currSlice->slice_type==B_SLICE)
  {
    currMB->b8mode[i] = b_v2b8[value];
    currMB->b8pdir[i] = b_v2pd[value];
  }
  else
  {
    currMB->b8mode[i] = p_v2b8[value];
    currMB->b8pdir[i] = p_v2pd[value];
  }
}


void reset_coeffs(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

  // CAVLC
  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC)
    fast_memset(p_Vid->nz_coeff[currSlice->current_mb_nr][0][0], 0, 3 * BLOCK_PIXELS * sizeof(byte));
}

void field_flag_inference(Macroblock *currMB)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  if (currMB->mbAvailA)
  {
    currMB->mb_field = p_Vid->mb_data[currMB->mbAddrA].mb_field;
  }
  else
  {
    // check top macroblock pair
    currMB->mb_field = currMB->mbAvailB ? p_Vid->mb_data[currMB->mbAddrB].mb_field : FALSE;
  }
}


void skip_macroblock(Macroblock *currMB)
{
  MotionVector pred_mv;
  int zeroMotionAbove;
  int zeroMotionLeft;
  PixelPos mb[4];    // neighbor blocks
  int   i, j;
  int   a_mv_y = 0;
  int   a_ref_idx = 0;
  int   b_mv_y = 0;
  int   b_ref_idx = 0;
  int   img_block_y   = currMB->block_y;
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  int   list_offset = currMB->list_offset; //((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (currMB->mbAddrX & 0x01) ? 4 : 2 : 0;
  StorablePicture *dec_picture = currSlice->dec_picture;
  MotionVector *a_mv = NULL;
  MotionVector *b_mv = NULL;

  get_neighbors(currMB, mb, 0, 0, MB_BLOCK_SIZE);

  if (mb[0].available)
  {
    a_mv      = &dec_picture->mv_info[mb[0].pos_y][mb[0].pos_x].mv[LIST_0];
    a_mv_y    = a_mv->mv_y;    
    a_ref_idx = dec_picture->mv_info[mb[0].pos_y][mb[0].pos_x].ref_idx[LIST_0];

    if (currMB->mb_field && !p_Vid->mb_data[mb[0].mb_addr].mb_field)
    {
      a_mv_y    /=2;
      a_ref_idx *=2;
    }
    if (!currMB->mb_field && p_Vid->mb_data[mb[0].mb_addr].mb_field)
    {
      a_mv_y    *=2;
      a_ref_idx >>=1;
    }
  }

  if (mb[1].available)
  {
    b_mv      = &dec_picture->mv_info[mb[1].pos_y][mb[1].pos_x].mv[LIST_0];
    b_mv_y    = b_mv->mv_y;
    b_ref_idx = dec_picture->mv_info[mb[1].pos_y][mb[1].pos_x].ref_idx[LIST_0];

    if (currMB->mb_field && !p_Vid->mb_data[mb[1].mb_addr].mb_field)
    {
      b_mv_y    /=2;
      b_ref_idx *=2;
    }
    if (!currMB->mb_field && p_Vid->mb_data[mb[1].mb_addr].mb_field)
    {
      b_mv_y    *=2;
      b_ref_idx >>=1;
    }
  }

  zeroMotionLeft  = !mb[0].available ? 1 : a_ref_idx==0 && a_mv->mv_x == 0 && a_mv_y==0 ? 1 : 0;
  zeroMotionAbove = !mb[1].available ? 1 : b_ref_idx==0 && b_mv->mv_x == 0 && b_mv_y==0 ? 1 : 0;

  currMB->cbp = 0;
  reset_coeffs(currSlice);

  if (zeroMotionAbove || zeroMotionLeft)
  {
    StorablePicture *cur_pic = currSlice->listX[LIST_0 + list_offset][0];
    PicMotionParams *mv_info = NULL;
    for(j = img_block_y; j < img_block_y + BLOCK_SIZE; ++j)
    {
      for(i = currMB->block_x; i < currMB->block_x + BLOCK_SIZE; ++i)
      {
        mv_info = &dec_picture->mv_info[j][i];
        mv_info->ref_pic[LIST_0] = cur_pic;
        mv_info->mv     [LIST_0] = zero_mv;
        mv_info->ref_idx[LIST_0] = 0;
      }
    }
  }
  else
  {
    PicMotionParams *mv_info = NULL;
    StorablePicture *cur_pic = currSlice->listX[LIST_0 + list_offset][0];
    currMB->GetMVPredictor (currMB, mb, &pred_mv, 0, dec_picture->mv_info, LIST_0, 0, 0, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

    // Set first block line (position img_block_y)
    for(j = img_block_y; j < img_block_y + BLOCK_SIZE; ++j)
    {
      for(i = currMB->block_x; i < currMB->block_x + BLOCK_SIZE; ++i)
      {
        mv_info = &dec_picture->mv_info[j][i];
        mv_info->ref_pic[LIST_0] = cur_pic;
        mv_info->mv     [LIST_0] = pred_mv;
        mv_info->ref_idx[LIST_0] = 0;
      }
    }
  }
}

static void concealIPCMcoeffs(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  StorablePicture *dec_picture = currSlice->dec_picture;
  int i, j, k;

  for(i=0;i<MB_BLOCK_SIZE;++i)
  {
    for(j=0;j<MB_BLOCK_SIZE;++j)
    {
      currSlice->cof[0][i][j] = p_Vid->dc_pred_value_comp[0];
      //p_Vid->fcf[0][i][j] = p_Vid->dc_pred_value_comp[0];
    }
  }

  if ((dec_picture->chroma_format_idc != YUV400) && (p_Vid->separate_colour_plane_flag == 0))
  {
    for (k = 0; k < 2; ++k)
    {
      for(i=0;i<p_Vid->mb_cr_size_y;++i)
      {
        for(j=0;j<p_Vid->mb_cr_size_x;++j)
        {
          currSlice->cof[k][i][j] = p_Vid->dc_pred_value_comp[k];
          //p_Vid->fcf[k][i][j] = p_Vid->dc_pred_value_comp[k];
        }
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Get the syntax elements from the NAL
 ************************************************************************
 */
static void read_one_macroblock_i_slice(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  SyntaxElement currSE;
  int mb_nr = currMB->mbAddrX; 

  DataPartition *dP;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  StorablePicture *dec_picture = currSlice->dec_picture; 
  PicMotionParamsOld *motion = &dec_picture->motion;

  currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : currSlice->mb_data[mb_nr-1].mb_field; 

  update_qp(currMB, currSlice->qp);
  currSE.type = SE_MBTYPE;

  //  read MB mode *****************************************************************
  dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)   
    currSE.mapping = linfo_ue;

  // read MB aff
  if (currSlice->mb_aff_frame_flag && (mb_nr&0x01)==0)
  {
    TRACE_STRING("mb_field_decoding_flag");
    if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
    {
      currSE.len = (int64) 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }
    else
    {
      currSE.reading = readFieldModeInfo_CABAC;
      dP->readSyntaxElement(currMB, &currSE, dP);
    }
    currMB->mb_field = (Boolean) currSE.value1;
  }

  if(p_Vid->active_pps->entropy_coding_mode_flag  == CABAC)
    CheckAvailabilityOfNeighborsCABAC(currMB);

  //  read MB type
  TRACE_STRING("mb_type");
  currSE.reading = readMB_typeInfo_CABAC_i_slice;
  dP->readSyntaxElement(currMB, &currSE, dP);

  currMB->mb_type = (short) currSE.value1;
  if(!dP->bitstream->ei_flag)
    currMB->ei_flag = 0;

  motion->mb_field[mb_nr] = (byte) currMB->mb_field;

  currMB->block_y_aff = ((currSlice->mb_aff_frame_flag) && (currMB->mb_field)) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

  currSlice->siblock[currMB->mb.y][currMB->mb.x] = 0;

  currSlice->interpret_mb_mode(currMB);

  //init NoMbPartLessThan8x8Flag
  currMB->NoMbPartLessThan8x8Flag = TRUE;

  //============= Transform Size Flag for INTRA MBs =============
  //-------------------------------------------------------------
  //transform size flag for INTRA_4x4 and INTRA_8x8 modes
  if (currMB->mb_type == I4MB && currSlice->Transform8x8Mode)
  {
    currSE.type   =  SE_HEADER;
    dP = &(currSlice->partArr[partMap[SE_HEADER]]);
    currSE.reading = readMB_transform_size_flag_CABAC;
    TRACE_STRING("transform_size_8x8_flag");

    // read CAVLC transform_size_8x8_flag
    if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
    {
      currSE.len = (int64) 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }
    else
    {
      dP->readSyntaxElement(currMB, &currSE, dP);
    }

    currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;

    if (currMB->luma_transform_size_8x8_flag)
    {      
      currMB->mb_type = I8MB;
      memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
      memset(&currMB->b8pdir, -1, 4 * sizeof(char));
    }
  }
  else
  {
    currMB->luma_transform_size_8x8_flag = FALSE;
  }

  //--- init macroblock data ---
  init_macroblock(currMB);

  if(currMB->mb_type != IPCM)
  {
    // intra prediction modes for a macroblock 4x4 **********************************************
    read_ipred_modes(currMB);

    // read CBP and Coeffs  ***************************************************************
    currSlice->read_CBP_and_coeffs_from_NAL (currMB);
  }
  else
  {
    //read pcm_alignment_zero_bit and pcm_byte[i]

    // here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
    // same category as MBTYPE
    if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
    {
      concealIPCMcoeffs(currMB);
    }
    else
    {
      dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
      read_IPCM_coeffs_from_NAL(currSlice, dP);
    }
  }

  return;
}

/*!
 ************************************************************************
 * \brief
 *    Get the syntax elements from the NAL
 ************************************************************************
 */
static void read_one_macroblock_p_slice(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;

  int i;

  SyntaxElement currSE;
  int mb_nr = currMB->mbAddrX; 

  DataPartition *dP;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  Macroblock *topMB = NULL;
  int  prevMbSkipped = 0;
  int  check_bottom, read_bottom, read_top;  
  StorablePicture *dec_picture = currSlice->dec_picture;
  PicMotionParamsOld *motion = &dec_picture->motion;

  if (currSlice->mb_aff_frame_flag)
  {
    if (mb_nr&0x01)
    {
      topMB= &p_Vid->mb_data[mb_nr-1];
      prevMbSkipped = (topMB->mb_type == 0);
    }
    else
      prevMbSkipped = 0;
  }

  currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : p_Vid->mb_data[mb_nr-1].mb_field;

  update_qp(currMB, currSlice->qp);
  currSE.type = SE_MBTYPE;

  //  read MB mode *****************************************************************
  dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)   
    currSE.mapping = linfo_ue;

  if (p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
  {
    // read MB skip_flag
    if (currSlice->mb_aff_frame_flag && ((mb_nr&0x01) == 0||prevMbSkipped))
      field_flag_inference(currMB);

    CheckAvailabilityOfNeighborsCABAC(currMB);
    TRACE_STRING("mb_skip_flag");
    currSE.reading = read_skip_flag_CABAC_p_slice;
    dP->readSyntaxElement(currMB, &currSE, dP);

    currMB->mb_type   = (short) currSE.value1;
    currMB->skip_flag = (char) (!(currSE.value1));

    if (!dP->bitstream->ei_flag)
      currMB->ei_flag = 0;

    // read MB AFF
    if (currSlice->mb_aff_frame_flag)
    {
      check_bottom=read_bottom=read_top=0;
      if ((mb_nr&0x01)==0)
      {
        check_bottom =  currMB->skip_flag;
        read_top = !check_bottom;
      }
      else
      {
        read_bottom = (topMB->skip_flag && (!currMB->skip_flag));
      }

      if (read_bottom || read_top)
      {
        TRACE_STRING("mb_field_decoding_flag");
        currSE.reading = readFieldModeInfo_CABAC;
        dP->readSyntaxElement(currMB, &currSE, dP);
        currMB->mb_field = (Boolean) currSE.value1;
      }

      if (check_bottom)
        check_next_mb_and_get_field_mode_CABAC_p_slice(currSlice, &currSE, dP);

      //update the list offset;
      currMB->list_offset = (currMB->mb_field)? ((mb_nr&0x01)? 4: 2): 0;

      //if (currMB->mb_type != 0 )
      CheckAvailabilityOfNeighborsCABAC(currMB);
    }
    
    // read MB type
    if (currMB->mb_type != 0 )
    {
      currSE.reading = readMB_typeInfo_CABAC_p_slice;
      TRACE_STRING("mb_type");
      dP->readSyntaxElement(currMB, &currSE, dP);
      currMB->mb_type = (short) currSE.value1;
      if(!dP->bitstream->ei_flag)
        currMB->ei_flag = 0;
    }
  }
  // VLC Non-Intra
  else
  {
    if(currSlice->cod_counter == -1)
    {
      TRACE_STRING("mb_skip_run");
      dP->readSyntaxElement(currMB, &currSE, dP);
      currSlice->cod_counter = currSE.value1;
    }

    if (currSlice->cod_counter==0)
    {
      // read MB aff
      if ((currSlice->mb_aff_frame_flag) && (((mb_nr&0x01)==0) || ((mb_nr&0x01) && prevMbSkipped)))
      {
        TRACE_STRING("mb_field_decoding_flag");
        currSE.len = (int64) 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        currMB->mb_field = (Boolean) currSE.value1;
      }

      // read MB type
      TRACE_STRING("mb_type");
      dP->readSyntaxElement(currMB, &currSE, dP);
      if(currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE)
        ++(currSE.value1);
      currMB->mb_type = (short) currSE.value1;
      if(!dP->bitstream->ei_flag)
        currMB->ei_flag = 0;
      currSlice->cod_counter--;
      currMB->skip_flag = 0;
    }
    else
    {
      currSlice->cod_counter--;
      currMB->mb_type = 0;
      currMB->ei_flag = 0;
      currMB->skip_flag = 1;

      // read field flag of bottom block
      if(currSlice->mb_aff_frame_flag)
      {
        if(currSlice->cod_counter == 0 && ((mb_nr&0x01) == 0))
        {
          TRACE_STRING("mb_field_decoding_flag (of coded bottom mb)");
          currSE.len = (int64) 1;
          readSyntaxElement_FLC(&currSE, dP->bitstream);
          dP->bitstream->frame_bitoffset--;
          TRACE_DECBITS(1);
          currMB->mb_field = (Boolean) currSE.value1;
        }
        else if (currSlice->cod_counter > 0 && ((mb_nr & 0x01) == 0))
        {
          // check left macroblock pair first
          if (mb_is_available(mb_nr - 2, currMB) && ((mb_nr % (p_Vid->PicWidthInMbs * 2))!=0))
          {
            currMB->mb_field = p_Vid->mb_data[mb_nr-2].mb_field;
          }
          else
          {
            // check top macroblock pair
            if (mb_is_available(mb_nr - 2*p_Vid->PicWidthInMbs, currMB))
            {
              currMB->mb_field = p_Vid->mb_data[mb_nr-2*p_Vid->PicWidthInMbs].mb_field;
            }
            else
              currMB->mb_field = FALSE;
          }
        }
      }
    }
    //update the list offset;
    currMB->list_offset = (currMB->mb_field)? ((mb_nr&0x01)? 4: 2): 0;

  }

  motion->mb_field[mb_nr] = (byte) currMB->mb_field;

  currMB->block_y_aff = (currMB->mb_field) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

  currSlice->siblock[currMB->mb.y][currMB->mb.x] = 0;

  currSlice->interpret_mb_mode(currMB);

  if(currSlice->mb_aff_frame_flag)
  {
    if(currMB->mb_field)
    {
      currSlice->num_ref_idx_active[LIST_0] <<=1;
      currSlice->num_ref_idx_active[LIST_1] <<=1;
    }
  }

  //init NoMbPartLessThan8x8Flag
  currMB->NoMbPartLessThan8x8Flag = TRUE;

  //====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
  if (currMB->mb_type == P8x8)
  {
    currSE.type = SE_MBTYPE;
    dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

    if (p_Vid->active_pps->entropy_coding_mode_flag ==CAVLC || dP->bitstream->ei_flag) 
      currSE.mapping = linfo_ue;
    else
      currSE.reading = readB8_typeInfo_CABAC_p_slice;

    for (i = 0; i < 4; ++i)
    {
      TRACE_STRING("sub_mb_type");
      dP->readSyntaxElement (currMB, &currSE, dP);
      SetB8Mode (currMB, currSE.value1, i);

      //set NoMbPartLessThan8x8Flag for P8x8 mode
      currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && p_Vid->active_sps->direct_8x8_inference_flag) ||
        (currMB->b8mode[i]==4);
    }
    //--- init macroblock data ---
    init_macroblock       (currMB);
    currSlice->read_motion_info_from_NAL (currMB);
  }

  //============= Transform Size Flag for INTRA MBs =============
  //-------------------------------------------------------------
  //transform size flag for INTRA_4x4 and INTRA_8x8 modes
  if (currMB->mb_type == I4MB && currSlice->Transform8x8Mode)
  {
    currSE.type   =  SE_HEADER;
    dP = &(currSlice->partArr[partMap[SE_HEADER]]);
    currSE.reading = readMB_transform_size_flag_CABAC;
    TRACE_STRING("transform_size_8x8_flag");

    // read CAVLC transform_size_8x8_flag
    if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
    {
      currSE.len = (int64) 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }
    else
    {
      dP->readSyntaxElement(currMB, &currSE, dP);
    }

    currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;

    if (currMB->luma_transform_size_8x8_flag)
    {      
      currMB->mb_type = I8MB;
      memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
      memset(&currMB->b8pdir, -1, 4 * sizeof(char));
    }
  }
  else
  {
    currMB->luma_transform_size_8x8_flag = FALSE;
  }

  if(p_Vid->active_pps->constrained_intra_pred_flag)
  {
    if( currMB->is_intra_block == FALSE)
    {
      currSlice->intra_block[mb_nr] = 0;
    }
  }


  //--- init macroblock data ---
  if (currMB->mb_type != P8x8)
    init_macroblock(currMB);

  if (currMB->mb_type == 0)
  {
    skip_macroblock(currMB);
  }
  else if(currMB->mb_type != IPCM)
  {
    // intra prediction modes for a macroblock 4x4 **********************************************
    if (currMB->is_intra_block == TRUE)
      read_ipred_modes(currMB);

    // read inter frame vector data *********************************************************
    if (currMB->is_intra_block == FALSE && (currMB->mb_type!=0) && (currMB->mb_type != P8x8))
    {
      currSlice->read_motion_info_from_NAL (currMB);
    }
    // read CBP and Coeffs  ***************************************************************
    currSlice->read_CBP_and_coeffs_from_NAL (currMB);
  }
  else
  {
    //read pcm_alignment_zero_bit and pcm_byte[i]

    // here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
    // same category as MBTYPE
    if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
    {
      concealIPCMcoeffs(currMB);
    }
    else
    {
      dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
      read_IPCM_coeffs_from_NAL(currSlice, dP);
    }
  }

  return;
}

/*!
 ************************************************************************
 * \brief
 *    Get the syntax elements from the NAL
 ************************************************************************
 */
static void read_one_macroblock_b_slice(Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  int i;

  SyntaxElement currSE;
  int mb_nr = currMB->mbAddrX; 

  DataPartition *dP;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  Macroblock *topMB = NULL;
  int  prevMbSkipped = 0;
  int  check_bottom, read_bottom, read_top;  
  StorablePicture *dec_picture = currSlice->dec_picture;
  PicMotionParamsOld *motion = &dec_picture->motion;

  if (currSlice->mb_aff_frame_flag)
  {
    if (mb_nr&0x01)
    {
      topMB= &p_Vid->mb_data[mb_nr-1];
      prevMbSkipped = topMB->skip_flag;
    }
    else
      prevMbSkipped = 0;
  }

  currMB->mb_field = ((mb_nr&0x01) == 0)? FALSE : p_Vid->mb_data[mb_nr-1].mb_field;

  update_qp(currMB, currSlice->qp);
  currSE.type = SE_MBTYPE;

  //  read MB mode *****************************************************************
  dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)   
    currSE.mapping = linfo_ue;

  if (p_Vid->active_pps->entropy_coding_mode_flag == CABAC)
  {
    // read MB skip_flag
    if (currSlice->mb_aff_frame_flag && ((mb_nr&0x01) == 0||prevMbSkipped))
      field_flag_inference(currMB);

    CheckAvailabilityOfNeighborsCABAC(currMB);
    TRACE_STRING("mb_skip_flag");
    currSE.reading = read_skip_flag_CABAC_b_slice;
    dP->readSyntaxElement(currMB, &currSE, dP);

    currMB->mb_type   = (short) currSE.value1;
    currMB->skip_flag = (char) (!(currSE.value1));

    currMB->cbp = currSE.value2;

    if (!dP->bitstream->ei_flag)
      currMB->ei_flag = 0;

    if (currSE.value1 == 0 && currSE.value2 == 0)
      currSlice->cod_counter=0;

    // read MB AFF
    if (currSlice->mb_aff_frame_flag)
    {
      check_bottom=read_bottom=read_top=0;
      if ((mb_nr & 0x01) == 0)
      {
        check_bottom =  currMB->skip_flag;
        read_top = !check_bottom;
      }
      else
      {
        read_bottom = (topMB->skip_flag && (!currMB->skip_flag));
      }

      if (read_bottom || read_top)
      {
        TRACE_STRING("mb_field_decoding_flag");
        currSE.reading = readFieldModeInfo_CABAC;
        dP->readSyntaxElement(currMB, &currSE, dP);
        currMB->mb_field = (Boolean) currSE.value1;
      }
      if (check_bottom)
        check_next_mb_and_get_field_mode_CABAC_b_slice(currSlice, &currSE, dP);

      //update the list offset;
      currMB->list_offset = (currMB->mb_field)? ((mb_nr&0x01)? 4: 2): 0;
      //if (currMB->mb_type != 0 )
      CheckAvailabilityOfNeighborsCABAC(currMB);
    }

    // read MB type
    if (currMB->mb_type != 0 )
    {
      currSE.reading = readMB_typeInfo_CABAC_b_slice;
      TRACE_STRING("mb_type");
      dP->readSyntaxElement(currMB, &currSE, dP);
      currMB->mb_type = (short) currSE.value1;
      if(!dP->bitstream->ei_flag)
        currMB->ei_flag = 0;
    }
  }
  else // VLC Non-Intra
  {
    if(currSlice->cod_counter == -1)
    {
      TRACE_STRING("mb_skip_run");
      dP->readSyntaxElement(currMB, &currSE, dP);
      currSlice->cod_counter = currSE.value1;
    }
    if (currSlice->cod_counter==0)
    {
      // read MB aff
      if ((currSlice->mb_aff_frame_flag) && (((mb_nr&0x01)==0) || ((mb_nr&0x01) && prevMbSkipped)))
      {
        TRACE_STRING("mb_field_decoding_flag");
        currSE.len = (int64) 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        currMB->mb_field = (Boolean) currSE.value1;
      }

      // read MB type
      TRACE_STRING("mb_type");
      dP->readSyntaxElement(currMB, &currSE, dP);
      if(currSlice->slice_type == P_SLICE || currSlice->slice_type == SP_SLICE)
        ++(currSE.value1);
      currMB->mb_type = (short) currSE.value1;
      if(!dP->bitstream->ei_flag)
        currMB->ei_flag = 0;
      currSlice->cod_counter--;
      currMB->skip_flag = 0;
    }
    else
    {
      currSlice->cod_counter--;
      currMB->mb_type = 0;
      currMB->ei_flag = 0;
      currMB->skip_flag = 1;

      // read field flag of bottom block
      if(currSlice->mb_aff_frame_flag)
      {
        if(currSlice->cod_counter == 0 && ((mb_nr&0x01) == 0))
        {
          TRACE_STRING("mb_field_decoding_flag (of coded bottom mb)");
          currSE.len = (int64) 1;
          readSyntaxElement_FLC(&currSE, dP->bitstream);
          dP->bitstream->frame_bitoffset--;
          TRACE_DECBITS(1);
          currMB->mb_field = (Boolean) currSE.value1;
        }
        else if (currSlice->cod_counter > 0 && ((mb_nr & 0x01) == 0))
        {
          // check left macroblock pair first
          if (mb_is_available(mb_nr - 2, currMB) && ((mb_nr % (p_Vid->PicWidthInMbs * 2))!=0))
          {
            currMB->mb_field = p_Vid->mb_data[mb_nr-2].mb_field;
          }
          else
          {
            // check top macroblock pair
            if (mb_is_available(mb_nr - 2*p_Vid->PicWidthInMbs, currMB))
            {
              currMB->mb_field = p_Vid->mb_data[mb_nr-2*p_Vid->PicWidthInMbs].mb_field;
            }
            else
              currMB->mb_field = FALSE;
          }
        }
      }
    }
    //update the list offset;
    currMB->list_offset = (currMB->mb_field)? ((mb_nr&0x01)? 4: 2): 0;

  }

  motion->mb_field[mb_nr] = (byte) currMB->mb_field;

  currMB->block_y_aff = (currMB->mb_field) ? (mb_nr&0x01) ? (currMB->block_y - 4)>>1 : currMB->block_y >> 1 : currMB->block_y;

  currSlice->siblock[currMB->mb.y][currMB->mb.x] = 0;

  currSlice->interpret_mb_mode(currMB);

  if(currSlice->mb_aff_frame_flag)
  {
    if(currMB->mb_field)
    {
      currSlice->num_ref_idx_active[LIST_0] <<=1;
      currSlice->num_ref_idx_active[LIST_1] <<=1;
    }
  }

  //====== READ 8x8 SUB-PARTITION MODES (modes of 8x8 blocks) and Intra VBST block modes ======
  if (currMB->mb_type == P8x8)
  {
    currMB->NoMbPartLessThan8x8Flag = TRUE;
    currSE.type = SE_MBTYPE;
    dP = &(currSlice->partArr[partMap[SE_MBTYPE]]);

    if (currSlice->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag) 
      currSE.mapping = linfo_ue;
    else
      currSE.reading = readB8_typeInfo_CABAC_b_slice;

    for (i = 0; i < 4; ++i)
    {
      TRACE_STRING("sub_mb_type");
      dP->readSyntaxElement (currMB, &currSE, dP);
      SetB8Mode (currMB, currSE.value1, i);

      //set NoMbPartLessThan8x8Flag for P8x8 mode
      currMB->NoMbPartLessThan8x8Flag &= (currMB->b8mode[i]==0 && currSlice->active_sps->direct_8x8_inference_flag) ||
        (currMB->b8mode[i]==4);
    }
    //--- init macroblock data ---
    init_macroblock       (currMB);
    currSlice->read_motion_info_from_NAL (currMB);
  }
  else
  {
    //init NoMbPartLessThan8x8Flag
    currMB->NoMbPartLessThan8x8Flag = ((currMB->mb_type == 0) && !(currSlice->active_sps->direct_8x8_inference_flag))? FALSE: TRUE;
  }

  //============= Transform Size Flag for INTRA MBs =============
  //-------------------------------------------------------------
  //transform size flag for INTRA_4x4 and INTRA_8x8 modes
  if (currMB->mb_type == I4MB && currSlice->Transform8x8Mode)
  {
    currSE.type   =  SE_HEADER;
    dP = &(currSlice->partArr[partMap[SE_HEADER]]);
    currSE.reading = readMB_transform_size_flag_CABAC;
    TRACE_STRING("transform_size_8x8_flag");

    // read CAVLC transform_size_8x8_flag
    if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
    {
      currSE.len = (int64) 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }
    else
    {
      dP->readSyntaxElement(currMB, &currSE, dP);
    }

    currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;

    if (currMB->luma_transform_size_8x8_flag)
    {      
      currMB->mb_type = I8MB;
      memset(&currMB->b8mode, I8MB, 4 * sizeof(char));
      memset(&currMB->b8pdir, -1, 4 * sizeof(char));
    }
  }
  else
  {
    currMB->luma_transform_size_8x8_flag = FALSE;
  }

  if(p_Vid->active_pps->constrained_intra_pred_flag && currMB->is_intra_block == FALSE)
  {
    currSlice->intra_block[mb_nr] = 0;
  }

  //--- init macroblock data ---
  if (currMB->mb_type != P8x8)
    init_macroblock(currMB);

  if ((currMB->mb_type == BSKIP_DIRECT) && currSlice->cod_counter >= 0)
  {
    currMB->cbp = 0;
    reset_coeffs(currSlice);

    if (p_Vid->active_pps->entropy_coding_mode_flag ==CABAC)
      currSlice->cod_counter=-1;
  }
  else if(currMB->mb_type != IPCM)
  {
    // intra prediction modes for a macroblock 4x4 **********************************************
    if (currMB->is_intra_block == TRUE)
      read_ipred_modes(currMB);

    // read inter frame vector data *********************************************************
    if (currMB->is_intra_block == FALSE && (currMB->mb_type!=0) && (currMB->mb_type != P8x8))
    {
      currSlice->read_motion_info_from_NAL (currMB);
    }
    // read CBP and Coeffs  ***************************************************************
    currSlice->read_CBP_and_coeffs_from_NAL (currMB);
  }
  else
  {
    //read pcm_alignment_zero_bit and pcm_byte[i]

    // here dP is assigned with the same dP as SE_MBTYPE, because IPCM syntax is in the
    // same category as MBTYPE
    if ( currSlice->dp_mode && currSlice->dpB_NotPresent )
    {
      concealIPCMcoeffs(currMB);
    }
    else
    {
      dP = &(currSlice->partArr[partMap[SE_LUM_DC_INTRA]]);
      read_IPCM_coeffs_from_NAL(currSlice, dP);
    }
  }

  return;
}


/*!
 ************************************************************************
 * \brief
 *    Initialize decoding engine after decoding an IPCM macroblock
 *    (for IPCM CABAC  28/11/2003)
 *
 * \author
 *    Dong Wang <Dong.Wang@bristol.ac.uk>
 ************************************************************************
 */
static void init_decoding_engine_IPCM(Slice *currSlice)
{   
  Bitstream *currStream;
  int ByteStartPosition;
  int PartitionNumber;
  int i;

  if(currSlice->dp_mode==PAR_DP_1)
    PartitionNumber=1;
  else if(currSlice->dp_mode==PAR_DP_3)
    PartitionNumber=3;
  else
  {
    printf("Partition Mode is not supported\n");
    exit(1);
  }

  for(i=0;i<PartitionNumber;++i)
  {
    currStream = currSlice->partArr[i].bitstream;
    ByteStartPosition = currStream->read_len;

    arideco_start_decoding (&currSlice->partArr[i].de_cabac, currStream->streamBuffer, ByteStartPosition, &currStream->read_len);
  }
}




/*!
 ************************************************************************
 * \brief
 *    Read IPCM pcm_alignment_zero_bit and pcm_byte[i] from stream to currSlice->cof
 *    (for IPCM CABAC and IPCM CAVLC)
 *
 * \author
 *    Dong Wang <Dong.Wang@bristol.ac.uk>
 ************************************************************************
 */

static void read_IPCM_coeffs_from_NAL(Slice *currSlice, struct datapartition *dP)
{
  VideoParameters *p_Vid = currSlice->p_Vid;

  StorablePicture *dec_picture = currSlice->dec_picture;
  SyntaxElement currSE;
  int i,j;

  //For CABAC, we don't need to read bits to let stream byte aligned
  //  because we have variable for integer bytes position
  if(p_Vid->active_pps->entropy_coding_mode_flag  == CABAC)
  {
    readIPCM_CABAC(currSlice, dP);
    init_decoding_engine_IPCM(currSlice);
  }
  else
  {
    //read bits to let stream byte aligned

    if(((dP->bitstream->frame_bitoffset) & 0x07) != 0)
    {
      TRACE_STRING("pcm_alignment_zero_bit");
      currSE.len = (8 - ((dP->bitstream->frame_bitoffset) & 0x07));
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }

    //read luma and chroma IPCM coefficients
    currSE.len=p_Vid->bitdepth_luma;
    TRACE_STRING("pcm_sample_luma");

    for(i=0;i<MB_BLOCK_SIZE;++i)
    {
      for(j=0;j<MB_BLOCK_SIZE;++j)
      {
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        currSlice->cof[0][i][j] = currSE.value1;
        //p_Vid->fcf[0][i][j] = currSE.value1;
      }
    }
    currSE.len=p_Vid->bitdepth_chroma;
    if ((dec_picture->chroma_format_idc != YUV400) && (p_Vid->separate_colour_plane_flag == 0))
    {
      TRACE_STRING("pcm_sample_chroma (u)");
      for(i=0;i<p_Vid->mb_cr_size_y;++i)
      {
        for(j=0;j<p_Vid->mb_cr_size_x;++j)
        {
          readSyntaxElement_FLC(&currSE, dP->bitstream);
          currSlice->cof[1][i][j] = currSE.value1;
          //p_Vid->fcf[1][i][j] = currSE.value1;
        }
      }
      TRACE_STRING("pcm_sample_chroma (v)");
      for(i=0;i<p_Vid->mb_cr_size_y;++i)
      {
        for(j=0;j<p_Vid->mb_cr_size_x;++j)
        {
          readSyntaxElement_FLC(&currSE, dP->bitstream);
          currSlice->cof[2][i][j] = currSE.value1;
          //p_Vid->fcf[2][i][j] = currSE.value1;
        }
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    If data partition B is lost, conceal PCM sample values with DC.
 *
 ************************************************************************
 */


static void read_ipred_modes(Macroblock *currMB)
{
  int b8,i,j,bi,bj,bx,by,dec;
  SyntaxElement currSE;
  DataPartition *dP;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  VideoParameters *p_Vid = currMB->p_Vid;

  StorablePicture *dec_picture = currSlice->dec_picture;
  int ts, ls;
  int mostProbableIntraPredMode;
  int upIntraPredMode;
  int leftIntraPredMode;
  char IntraChromaPredModeFlag = (char) (currMB->is_intra_block == TRUE);
  int bs_x, bs_y;
  int ii,jj;

  PixelPos left_block, top_block;

  currSE.type = SE_INTRAPREDMODE;

  TRACE_STRING("intra4x4_pred_mode");
  dP = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);

  if (!(p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag))
    currSE.reading = readIntraPredMode_CABAC;

  for(b8 = 0; b8 < 4; ++b8)  //loop 8x8 blocks
  {
    if((currMB->b8mode[b8]==IBLOCK )||(currMB->b8mode[b8]==I8MB))
    {
      bs_x = bs_y = (currMB->b8mode[b8] == I8MB)?8:4;

      IntraChromaPredModeFlag = 1;

      ii = (bs_x>>2);
      jj = (bs_y>>2);

      for(j = 0; j < 2; j += jj)  //loop subblocks
      {
        by = (b8 & 2) + j;
        bj = currMB->block_y + by;

        for(i = 0; i < 2; i += ii)
        {
          bx = ((b8 & 1) << 1) + i;
          bi = currMB->block_x + bx;
          //get from stream
          if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag)
            readSyntaxElement_Intra4x4PredictionMode(&currSE, dP->bitstream);
          else
          {
            currSE.context=(b8<<2)+(j<<1) +i;
            dP->readSyntaxElement(currMB, &currSE, dP);
          }

          get4x4Neighbour(currMB, (bx<<2) - 1, (by<<2),     p_Vid->mb_size[IS_LUMA], &left_block);
          get4x4Neighbour(currMB, (bx<<2),     (by<<2) - 1, p_Vid->mb_size[IS_LUMA], &top_block );

          //get from array and decode

          if (p_Vid->active_pps->constrained_intra_pred_flag)
          {
            left_block.available = left_block.available ? currSlice->intra_block[left_block.mb_addr] : 0;
            top_block.available  = top_block.available  ? currSlice->intra_block[top_block.mb_addr]  : 0;
          }

          // !! KS: not sure if the following is still correct...
          ts = ls = 0;   // Check to see if the neighboring block is SI
          if (currMB->mb_type == I4MB && currSlice->slice_type == SI_SLICE)           // need support for MBINTLC1
          {
            if (left_block.available)
              if (currSlice->siblock [PicPos[left_block.mb_addr].y][PicPos[left_block.mb_addr].x])
                ls=1;

            if (top_block.available)
              if (currSlice->siblock [PicPos[top_block.mb_addr].y][PicPos[top_block.mb_addr].x])
                ts=1;
          }

          upIntraPredMode            = (top_block.available  &&(ts == 0)) ? currSlice->ipredmode[top_block.pos_y ][top_block.pos_x ] : -1;
          leftIntraPredMode          = (left_block.available &&(ls == 0)) ? currSlice->ipredmode[left_block.pos_y][left_block.pos_x] : -1;

          mostProbableIntraPredMode  = (upIntraPredMode < 0 || leftIntraPredMode < 0) ? DC_PRED : upIntraPredMode < leftIntraPredMode ? upIntraPredMode : leftIntraPredMode;

          dec = (currSE.value1 == -1) ? mostProbableIntraPredMode : currSE.value1 + (currSE.value1 >= mostProbableIntraPredMode);

          //set
          //??           memset(&(p_Vid->ipredmode[bj + jj][bi]), dec, (bs_x>>2) * sizeof(char)); // sets 1 or 2 bytes
          if (bs_x == 8) 
          {
            for (jj = 0; jj < (bs_y >> 2); ++jj) 
            {   //loop 4x4s in the subblock for 8x8 prediction setting
              currSlice->ipredmode[bj + jj][bi  ] = (byte) dec;
              currSlice->ipredmode[bj + jj][bi+1] = (byte) dec;
            }
          }
          else  // bs_x == 4
          {
            for (jj = 0; jj < (bs_y >> 2); ++jj)   //loop 4x4s in the subblock for 8x8 prediction setting
              currSlice->ipredmode[bj + jj][bi] = (byte) dec;
          }
        }
      }
    }
  }

  if (IntraChromaPredModeFlag && (dec_picture->chroma_format_idc != YUV400) && (dec_picture->chroma_format_idc != YUV444))
  {
    currSE.type = SE_INTRAPREDMODE;
    TRACE_STRING("intra_chroma_pred_mode");
    dP = &(currSlice->partArr[partMap[SE_INTRAPREDMODE]]);

    if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag) 
    {
      currSE.mapping = linfo_ue;
    }
    else
      currSE.reading = readCIPredMode_CABAC;

    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->c_ipred_mode = (char) currSE.value1;

    if (currMB->c_ipred_mode < DC_PRED_8 || currMB->c_ipred_mode > PLANE_8)
    {
      error("illegal chroma intra pred mode!\n", 600);
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Get current block spatial neighbors
 ************************************************************************
 */
void get_neighbors(Macroblock *currMB,       // <--  current Macroblock
                   PixelPos   *block,     // <--> neighbor blocks
                   int         mb_x,         // <--  block x position
                   int         mb_y,         // <--  block y position
                   int         blockshape_x  // <--  block width
                   )
{
  VideoParameters *p_Vid = currMB->p_Vid;
  int *mb_size = p_Vid->mb_size[IS_LUMA];
  
  get4x4Neighbour(currMB, mb_x - 1,            mb_y    , mb_size, block    );
  get4x4Neighbour(currMB, mb_x,                mb_y - 1, mb_size, block + 1);
  get4x4Neighbour(currMB, mb_x + blockshape_x, mb_y - 1, mb_size, block + 2);  

  if (mb_y > 0)
  {
    if (mb_x < 8)  // first column of 8x8 blocks
    {
      if (mb_y == 8 )
      {
        if (blockshape_x == MB_BLOCK_SIZE)      
          block[2].available  = 0;
      }
      else if (mb_x + blockshape_x == 8)
      {
        block[2].available = 0;
      }
    }
    else if (mb_x + blockshape_x == MB_BLOCK_SIZE)
    {
      block[2].available = 0;
    }
  }

  if (!block[2].available)
  {
    get4x4Neighbour(currMB, mb_x - 1, mb_y - 1, mb_size, block + 3);
    block[2] = block[3];
  }
}

/*!
 ************************************************************************
 * \brief
 *    Read motion info
 ************************************************************************
 */
static void read_motion_info_from_NAL_p_slice (Macroblock *currMB)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;

  SyntaxElement currSE;
  DataPartition *dP = NULL;
  const byte *partMap       = assignSE2partition[currSlice->dp_mode];
  short partmode        = ((currMB->mb_type == P8x8) ? 4 : currMB->mb_type);
  int step_h0         = BLOCK_STEP [partmode][0];
  int step_v0         = BLOCK_STEP [partmode][1];

  int j4;
  StorablePicture *dec_picture = currSlice->dec_picture;
  PicMotionParams *mv_info = NULL;

  int list_offset = currMB->list_offset; // ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field))? (mb_nr&0x01) ? 4 : 2 : 0;
  StorablePicture **list0 = currSlice->listX[LIST_0 + list_offset];

  //=====  READ REFERENCE PICTURE INDICES =====
  currSE.type = SE_REFFRAME;
  dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
  //  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
  prepareListforRefIdx (currMB, &currSE, dP, currSlice->num_ref_idx_active[LIST_0], (currMB->mb_type != P8x8) || (!currSlice->allrefzero));
  readMBRefPictureIdx  (&currSE, dP, currMB, &dec_picture->mv_info[currMB->block_y], LIST_0, step_v0, step_h0);

  //=====  READ MOTION VECTORS =====
  currSE.type = SE_MVD;
  dP = &(currSlice->partArr[partMap[SE_MVD]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag) 
    currSE.mapping = linfo_se;
  else                                                  
    currSE.reading = read_MVD_CABAC;

  // LIST_0 Motion vectors
  readMBMotionVectors (&currSE, dP, currMB, LIST_0, step_h0, step_v0);

  // record reference picture Ids for deblocking decisions  
  for(j4 = currMB->block_y; j4 < (currMB->block_y + 4);++j4)
  {
    mv_info = &dec_picture->mv_info[j4][currMB->block_x];
    mv_info->ref_pic[LIST_0] = list0[(short) mv_info->ref_idx[LIST_0]];
    mv_info++;
    mv_info->ref_pic[LIST_0] = list0[(short) mv_info->ref_idx[LIST_0]];
    mv_info++;
    mv_info->ref_pic[LIST_0] = list0[(short) mv_info->ref_idx[LIST_0]];
    mv_info++;
    mv_info->ref_pic[LIST_0] = list0[(short) mv_info->ref_idx[LIST_0]];
  }
}


/*!
************************************************************************
* \brief
*    Read motion info
************************************************************************
*/
static void read_motion_info_from_NAL_b_slice (Macroblock *currMB)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  StorablePicture *dec_picture = currSlice->dec_picture;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int partmode        = ((currMB->mb_type == P8x8) ? 4 : currMB->mb_type);
  int step_h0         = BLOCK_STEP [partmode][0];
  int step_v0         = BLOCK_STEP [partmode][1];

  int j4, i4;

  int list_offset = currMB->list_offset; // ((currSlice->mb_aff_frame_flag)&&(currMB->mb_field))? (mb_nr&0x01) ? 4 : 2 : 0;
  StorablePicture **list0 = currSlice->listX[LIST_0 + list_offset];
  StorablePicture **list1 = currSlice->listX[LIST_1 + list_offset];

  if (currMB->mb_type == P8x8)
    currSlice->update_direct_mv_info(currMB);   

  //=====  READ REFERENCE PICTURE INDICES =====
  currSE.type = SE_REFFRAME;
  dP = &(currSlice->partArr[partMap[SE_REFFRAME]]);
  //  For LIST_0, if multiple ref. pictures, read LIST_0 reference picture indices for the MB ***********
  prepareListforRefIdx (currMB, &currSE, dP, currSlice->num_ref_idx_active[LIST_0], TRUE);
  readMBRefPictureIdx  (&currSE, dP, currMB, &dec_picture->mv_info[currMB->block_y], LIST_0, step_v0, step_h0);

  //  For LIST_1, if multiple ref. pictures, read LIST_1 reference picture indices for the MB ***********
  prepareListforRefIdx (currMB, &currSE, dP, currSlice->num_ref_idx_active[LIST_1], TRUE);
  readMBRefPictureIdx  (&currSE, dP, currMB, &dec_picture->mv_info[currMB->block_y], LIST_1, step_v0, step_h0);

  //=====  READ MOTION VECTORS =====
  currSE.type = SE_MVD;
  dP = &(currSlice->partArr[partMap[SE_MVD]]);

  if (p_Vid->active_pps->entropy_coding_mode_flag == CAVLC || dP->bitstream->ei_flag) 
    currSE.mapping = linfo_se;
  else                                                  
    currSE.reading = read_MVD_CABAC;

  // LIST_0 Motion vectors
  readMBMotionVectors (&currSE, dP, currMB, LIST_0, step_h0, step_v0);
  // LIST_1 Motion vectors
  readMBMotionVectors (&currSE, dP, currMB, LIST_1, step_h0, step_v0);

  // record reference picture Ids for deblocking decisions

  for(j4 = currMB->block_y; j4 < (currMB->block_y + 4);++j4)
  {
    for(i4 = currMB->block_x; i4 < (currMB->block_x + 4);++i4)
    {
      PicMotionParams *mv_info = &dec_picture->mv_info[j4][i4];
      short ref_idx = mv_info->ref_idx[LIST_0];

      mv_info->ref_pic[LIST_0] = (ref_idx >= 0) ? list0[ref_idx] : NULL;        
      ref_idx = mv_info->ref_idx[LIST_1];
      mv_info->ref_pic[LIST_1] = (ref_idx >= 0) ? list1[ref_idx] : NULL;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Get the Prediction from the Neighboring Blocks for Number of 
 *    Nonzero Coefficients
 *
 *    Luma Blocks
 ************************************************************************
 */
int predict_nnz(Macroblock *currMB, int block_type, int i,int j)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;

  PixelPos pix;

  int pred_nnz = 0;
  int cnt      = 0;

  // left block
  get4x4Neighbour(currMB, i - 1, j, p_Vid->mb_size[IS_LUMA], &pix);

  if ((currMB->is_intra_block == TRUE) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (currSlice->dp_mode==PAR_DP_3))
  {
    pix.available &= currSlice->intra_block[pix.mb_addr];
    if (!pix.available)
      ++cnt;
  }

  if (pix.available)
  { 
    switch (block_type)
    {
    case LUMA:
      pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][0][pix.y][pix.x];
      ++cnt;
      break;
    case CB:
      pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][1][pix.y][pix.x];
      ++cnt;
      break;
    case CR:
      pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][2][pix.y][pix.x];
      ++cnt;
      break;
    default:
      error("writeCoeff4x4_CAVLC: Invalid block type", 600);
      break;
    }
  }

  // top block
  get4x4Neighbour(currMB, i, j - 1, p_Vid->mb_size[IS_LUMA], &pix);

  if ((currMB->is_intra_block == TRUE) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (currSlice->dp_mode==PAR_DP_3))
  {
    pix.available &= currSlice->intra_block[pix.mb_addr];
    if (!pix.available)
      ++cnt;
  }

  if (pix.available)
  {
    switch (block_type)
    {
    case LUMA:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][0][pix.y][pix.x];
      ++cnt;
      break;
    case CB:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][1][pix.y][pix.x];
      ++cnt;
      break;
    case CR:
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][2][pix.y][pix.x];
      ++cnt;
      break;
    default:
      error("writeCoeff4x4_CAVLC: Invalid block type", 600);
      break;
    }
  }

  if (cnt==2)
  {
    ++pred_nnz;
    pred_nnz>>=1;
  }

  return pred_nnz;
}


/*!
 ************************************************************************
 * \brief
 *    Get the Prediction from the Neighboring Blocks for Number of 
 *    Nonzero Coefficients
 *
 *    Chroma Blocks
 ************************************************************************
 */
int predict_nnz_chroma(Macroblock *currMB, int i,int j)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  StorablePicture *dec_picture = currMB->p_Slice->dec_picture;
  Slice *currSlice = currMB->p_Slice;
  PixelPos pix;

  int pred_nnz = 0;
  int cnt      = 0;

  if (dec_picture->chroma_format_idc != YUV444)
  {
    //YUV420 and YUV422
    // left block
    get4x4Neighbour(currMB, ((i&0x01)<<2) - 1, j, p_Vid->mb_size[IS_CHROMA], &pix);

    if ((currMB->is_intra_block == TRUE) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (currSlice->dp_mode==PAR_DP_3))
    {
      pix.available &= currSlice->intra_block[pix.mb_addr];
      if (!pix.available)
        ++cnt;
    }

    if (pix.available)
    {
      pred_nnz = p_Vid->nz_coeff [pix.mb_addr ][1][pix.y][2 * (i>>1) + pix.x];
      ++cnt;
    }

    // top block
    get4x4Neighbour(currMB, ((i&0x01)<<2), j - 1, p_Vid->mb_size[IS_CHROMA], &pix);

    if ((currMB->is_intra_block == TRUE) && pix.available && p_Vid->active_pps->constrained_intra_pred_flag && (currSlice->dp_mode==PAR_DP_3))
    {
      pix.available &= currSlice->intra_block[pix.mb_addr];
      if (!pix.available)
        ++cnt;
    }

    if (pix.available)
    {
      pred_nnz += p_Vid->nz_coeff [pix.mb_addr ][1][pix.y][2 * (i>>1) + pix.x];
      ++cnt;
    }

    if (cnt==2)
    {
      ++pred_nnz;
      pred_nnz >>= 1;
    }
  }

  return pred_nnz;
}


/*!
 ************************************************************************
 * \brief
 *    Reads coeff of an 4x4 block (CAVLC)
 *
 * \author
 *    Karl Lillevold <karll@real.com>
 *    contributions by James Au <james@ubvideo.com>
 ************************************************************************
 */
static void readCoeff4x4_CAVLC (Macroblock *currMB, 
                                int block_type,
                                int i, int j, int levarr[16], int runarr[16],
                                int *number_coefficients)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  int mb_nr = currMB->mbAddrX;
  SyntaxElement currSE;
  DataPartition *dP;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  Bitstream *currStream;

  int k, code, vlcnum;
  int numcoeff = 0, numtrailingones, numcoeff_vlc;
  int level_two_or_higher;
  int numones, totzeros, abslevel, cdc=0, cac=0;
  int zerosleft, ntr, dptype = 0;
  int max_coeff_num = 0, nnz;
  char type[15];
  static const int incVlc[] = {0, 3, 6, 12, 24, 48, 32768};    // maximum vlc = 6

  switch (block_type)
  {
  case LUMA:
    max_coeff_num = 16;
    TRACE_PRINTF("Luma");
    dptype = (currMB->is_intra_block == TRUE) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  case LUMA_INTRA16x16DC:
    max_coeff_num = 16;
    TRACE_PRINTF("Lum16DC");
    dptype = SE_LUM_DC_INTRA;
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  case LUMA_INTRA16x16AC:
    max_coeff_num = 15;
    TRACE_PRINTF("Lum16AC");
    dptype = SE_LUM_AC_INTRA;
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  case CB:
    max_coeff_num = 16;
    TRACE_PRINTF("Luma_add1");
    dptype = ((currMB->is_intra_block == TRUE)) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
    p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
    break;
  case CB_INTRA16x16DC:
    max_coeff_num = 16;
    TRACE_PRINTF("Luma_add1_16DC");
    dptype = SE_LUM_DC_INTRA;
    p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
    break;
  case CB_INTRA16x16AC:
    max_coeff_num = 15;
    TRACE_PRINTF("Luma_add1_16AC");
    dptype = SE_LUM_AC_INTRA;
    p_Vid->nz_coeff[mb_nr][1][j][i] = 0; 
    break;
  case CR:
    max_coeff_num = 16;
    TRACE_PRINTF("Luma_add2");
    dptype = ((currMB->is_intra_block == TRUE)) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER;
    p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
    break;
  case CR_INTRA16x16DC:
    max_coeff_num = 16;
    TRACE_PRINTF("Luma_add2_16DC");
    dptype = SE_LUM_DC_INTRA;
    p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
    break;
  case CR_INTRA16x16AC:
    max_coeff_num = 15;
    TRACE_PRINTF("Luma_add1_16AC");
    dptype = SE_LUM_AC_INTRA;
    p_Vid->nz_coeff[mb_nr][2][j][i] = 0; 
    break;        
  case CHROMA_DC:
    max_coeff_num = p_Vid->num_cdc_coeff;
    cdc = 1;
    TRACE_PRINTF("ChrDC");
    dptype = (currMB->is_intra_block == TRUE) ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER;
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  case CHROMA_AC:
    max_coeff_num = 15;
    cac = 1;
    TRACE_PRINTF("ChrDC");
    dptype = (currMB->is_intra_block == TRUE) ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER;
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  default:
    error ("readCoeff4x4_CAVLC: invalid block type", 600);
    p_Vid->nz_coeff[mb_nr][0][j][i] = 0; 
    break;
  }

  currSE.type = dptype;
  dP = &(currSlice->partArr[partMap[dptype]]);
  currStream = dP->bitstream;  

  if (!cdc)
  {    
    // luma or chroma AC    
    if(block_type==LUMA || block_type==LUMA_INTRA16x16DC || block_type==LUMA_INTRA16x16AC ||block_type==CHROMA_AC)
    {
      nnz = (!cac) ? predict_nnz(currMB, LUMA, i<<2, j<<2) : predict_nnz_chroma(currMB, i, ((j-4)<<2));
    }
    else if (block_type==CB || block_type==CB_INTRA16x16DC || block_type==CB_INTRA16x16AC)
    {   
      nnz = predict_nnz(currMB, CB, i<<2, j<<2);
    }
    else
    { 
      nnz = predict_nnz(currMB, CR, i<<2, j<<2);
    }

    if (nnz < 2)
    {
      numcoeff_vlc = 0;
    }
    else if (nnz < 4)
    {
      numcoeff_vlc = 1;
    }
    else if (nnz < 8)
    {
      numcoeff_vlc = 2;
    }
    else //
    {
      numcoeff_vlc = 3;
    }

    currSE.value1 = numcoeff_vlc;

    readSyntaxElement_NumCoeffTrailingOnes(&currSE, currStream, type);

    numcoeff        =  currSE.value1;
    numtrailingones =  currSE.value2;

    if(block_type==LUMA || block_type==LUMA_INTRA16x16DC || block_type==LUMA_INTRA16x16AC ||block_type==CHROMA_AC)
      p_Vid->nz_coeff[mb_nr][0][j][i] = (byte) numcoeff;
    else if (block_type==CB || block_type==CB_INTRA16x16DC || block_type==CB_INTRA16x16AC)
      p_Vid->nz_coeff[mb_nr][1][j][i] = (byte) numcoeff;
    else
      p_Vid->nz_coeff[mb_nr][2][j][i] = (byte) numcoeff;        
  }
  else
  {
    // chroma DC
    readSyntaxElement_NumCoeffTrailingOnesChromaDC(p_Vid, &currSE, currStream);

    numcoeff        =  currSE.value1;
    numtrailingones =  currSE.value2;
  }

  memset(levarr, 0, max_coeff_num * sizeof(int));
  memset(runarr, 0, max_coeff_num * sizeof(int));

  numones = numtrailingones;
  *number_coefficients = numcoeff;

  if (numcoeff)
  {
    if (numtrailingones)
    {      
      currSE.len = numtrailingones;

#if TRACE
      snprintf(currSE.tracestring,
        TRACESTRING_SIZE, "%s trailing ones sign (%d,%d)", type, i, j);
#endif

      readSyntaxElement_FLC (&currSE, currStream);

      code = currSE.inf;
      ntr = numtrailingones;
      for (k = numcoeff - 1; k > numcoeff - 1 - numtrailingones; k--)
      {
        ntr --;
        levarr[k] = (code>>ntr)&1 ? -1 : 1;
      }
    }

    // decode levels
    level_two_or_higher = (numcoeff > 3 && numtrailingones == 3)? 0 : 1;
    vlcnum = (numcoeff > 10 && numtrailingones < 3) ? 1 : 0;

    for (k = numcoeff - 1 - numtrailingones; k >= 0; k--)
    {

#if TRACE
      snprintf(currSE.tracestring,
        TRACESTRING_SIZE, "%s lev (%d,%d) k=%d vlc=%d ", type, i, j, k, vlcnum);
#endif

      if (vlcnum == 0)
        readSyntaxElement_Level_VLC0(&currSE, currStream);
      else
        readSyntaxElement_Level_VLCN(&currSE, vlcnum, currStream);

      if (level_two_or_higher)
      {
        currSE.inf += (currSE.inf > 0) ? 1 : -1;
        level_two_or_higher = 0;
      }

      levarr[k] = currSE.inf;
      abslevel = iabs(levarr[k]);
      if (abslevel  == 1)
        ++numones;

      // update VLC table
      if (abslevel  > incVlc[vlcnum])
        ++vlcnum;

      if (k == numcoeff - 1 - numtrailingones && abslevel >3)
        vlcnum = 2;      
    }

    if (numcoeff < max_coeff_num)
    {
      // decode total run
      vlcnum = numcoeff - 1;
      currSE.value1 = vlcnum;

#if TRACE
      snprintf(currSE.tracestring,
        TRACESTRING_SIZE, "%s totalrun (%d,%d) vlc=%d ", type, i,j, vlcnum);
#endif
      if (cdc)
        readSyntaxElement_TotalZerosChromaDC(p_Vid, &currSE, currStream);
      else
        readSyntaxElement_TotalZeros(&currSE, currStream);

      totzeros = currSE.value1;
    }
    else
    {
      totzeros = 0;
    }

    // decode run before each coefficient
    zerosleft = totzeros;
    i = numcoeff - 1;

    if (zerosleft > 0 && i > 0)
    {
      do
      {
        // select VLC for runbefore
        vlcnum = imin(zerosleft - 1, RUNBEFORE_NUM_M1);

        currSE.value1 = vlcnum;
#if TRACE
        snprintf(currSE.tracestring,
          TRACESTRING_SIZE, "%s run (%d,%d) k=%d vlc=%d ",
          type, i, j, i, vlcnum);
#endif

        readSyntaxElement_Run(&currSE, currStream);
        runarr[i] = currSE.value1;

        zerosleft -= runarr[i];
        i --;
      } while (zerosleft != 0 && i != 0);
    }
    runarr[i] = zerosleft;    
  } // if numcoeff
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a SMB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4SMB_CABAC (Macroblock *currMB, SyntaxElement *currSE, ColorPlane pl, int block_y, int block_x, int start_scan, int64 *cbp_blk)
{
  int i,j,k;
  int i0, j0;
  int level = 1;
  DataPartition *dP;
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];

  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];
  int **cof = currSlice->cof[pl];

  for (j = block_y; j < block_y + BLOCK_SIZE_8x8; j += 4)
  {
    currMB->subblock_y = j; // position for coeff_count ctx

    for (i = block_x; i < block_x + BLOCK_SIZE_8x8; i += 4)
    {
      currMB->subblock_x = i; // position for coeff_count ctx
      pos_scan_4x4 = pos_scan4x4[start_scan];
      level = 1;

      if (start_scan == 0)
      {
        /*
        * make distinction between INTRA and INTER coded
        * luminance coefficients
        */
        currSE->type = (currMB->is_intra_block ? SE_LUM_DC_INTRA : SE_LUM_DC_INTER);  
        dP = &(currSlice->partArr[partMap[currSE->type]]);
        if (dP->bitstream->ei_flag)  
          currSE->mapping = linfo_levrun_inter;
        else                                                     
          currSE->reading = readRunLevel_CABAC;

#if TRACE
        if (pl == PLANE_Y)
          sprintf(currSE->tracestring, "Luma sng ");
        else if (pl == PLANE_U)
          sprintf(currSE->tracestring, "Cb   sng ");
        else
          sprintf(currSE->tracestring, "Cr   sng ");  
#endif

        dP->readSyntaxElement(currMB, currSE, dP);
        level = currSE->value1;

        if (level != 0)    /* leave if level == 0 */
        {
          pos_scan_4x4 += 2 * currSE->value2;

          i0 = *pos_scan_4x4++;
          j0 = *pos_scan_4x4++;

          *cbp_blk |= i64_power2(j + (i >> 2)) ;
          //cof[j + j0][i + i0]= rshift_rnd_sf((level * InvLevelScale4x4[j0][i0]) << qp_per, 4);
          cof[j + j0][i + i0]= level;
          //p_Vid->fcf[pl][j + j0][i + i0]= level;
        }
      }

      if (level != 0)
      {
        // make distinction between INTRA and INTER coded luminance coefficients
        currSE->type = (currMB->is_intra_block ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER);  
        dP = &(currSlice->partArr[partMap[currSE->type]]);

        if (dP->bitstream->ei_flag)  
          currSE->mapping = linfo_levrun_inter;
        else                                                     
          currSE->reading = readRunLevel_CABAC;

        for(k = 1; (k < 17) && (level != 0); ++k)
        {
#if TRACE
          if (pl == PLANE_Y)
            sprintf(currSE->tracestring, "Luma sng ");
          else if (pl == PLANE_U)
            sprintf(currSE->tracestring, "Cb   sng ");
          else
            sprintf(currSE->tracestring, "Cr   sng ");  
#endif

          dP->readSyntaxElement(currMB, currSE, dP);
          level = currSE->value1;

          if (level != 0)    /* leave if level == 0 */
          {
            pos_scan_4x4 += 2 * currSE->value2;

            i0 = *pos_scan_4x4++;
            j0 = *pos_scan_4x4++;

            cof[j + j0][i + i0] = level;
            //p_Vid->fcf[pl][j + j0][i + i0]= level;
          }
        }
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of all 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4MB_CABAC (Macroblock *currMB, SyntaxElement *currSE, ColorPlane pl, int (*InvLevelScale4x4)[4], int qp_per, int cbp)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  int start_scan = IS_I16MB (currMB)? 1 : 0; 
  int block_y, block_x;
  int i, j;
  int64 *cbp_blk = &currMB->cbp_blk[pl];

  if( pl == PLANE_Y || (p_Vid->separate_colour_plane_flag != 0) )
    currSE->context = (IS_I16MB(currMB) ? LUMA_16AC: LUMA_4x4);
  else if (pl == PLANE_U)
    currSE->context = (IS_I16MB(currMB) ? CB_16AC: CB_4x4);
  else
    currSE->context = (IS_I16MB(currMB) ? CR_16AC: CR_4x4);  

  if (currMB->is_lossless == FALSE)
  {
    for (block_y = 0; block_y < MB_BLOCK_SIZE; block_y += BLOCK_SIZE_8x8) /* all modes */
    {
      int **cof = &currSlice->cof[pl][block_y];
      for (block_x = 0; block_x < MB_BLOCK_SIZE; block_x += BLOCK_SIZE_8x8)
      {
        if (cbp & (1 << ((block_y >> 2) + (block_x >> 3))))  // are there any coeff in current block at all
        {
          readCompCoeff4x4SMB_CABAC (currMB, currSE, pl, block_y, block_x, start_scan, cbp_blk);

          if (start_scan == 0)
          {
            for (j = 0; j < BLOCK_SIZE_8x8; ++j)
            {
              int *coef = &cof[j][block_x];
              int jj = j & 0x03;
              for (i = 0; i < BLOCK_SIZE_8x8; i+=4)
              {
                if (*coef)
                  *coef = rshift_rnd_sf((*coef * InvLevelScale4x4[jj][0]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef = rshift_rnd_sf((*coef * InvLevelScale4x4[jj][1]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef = rshift_rnd_sf((*coef * InvLevelScale4x4[jj][2]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef = rshift_rnd_sf((*coef * InvLevelScale4x4[jj][3]) << qp_per, 4);
                coef++;
              }
            }
          }
          else
          {                        
            for (j = 0; j < BLOCK_SIZE_8x8; ++j)
            {
              int *coef = &cof[j][block_x];
              int jj = j & 0x03;
              for (i = 0; i < BLOCK_SIZE_8x8; i += 4)
              {
                if ((jj != 0) && *coef)
                  *coef= rshift_rnd_sf((*coef * InvLevelScale4x4[jj][0]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef= rshift_rnd_sf((*coef * InvLevelScale4x4[jj][1]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef= rshift_rnd_sf((*coef * InvLevelScale4x4[jj][2]) << qp_per, 4);
                coef++;
                if (*coef)
                  *coef= rshift_rnd_sf((*coef * InvLevelScale4x4[jj][3]) << qp_per, 4);
                coef++;
              }
            }
          }        
        }
      }
    }
  }
  else
  {
    for (block_y = 0; block_y < MB_BLOCK_SIZE; block_y += BLOCK_SIZE_8x8) /* all modes */
    {
      for (block_x = 0; block_x < MB_BLOCK_SIZE; block_x += BLOCK_SIZE_8x8)
      {
        if (cbp & (1 << ((block_y >> 2) + (block_x >> 3))))  // are there any coeff in current block at all
        {
          readCompCoeff4x4SMB_CABAC (currMB, currSE, pl, block_y, block_x, start_scan, cbp_blk);
        }
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of one 8x8 block
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8_CABAC (Macroblock *currMB, SyntaxElement *currSE, ColorPlane pl, int b8)
{
  if (currMB->cbp & (1<<b8))  // are there any coefficients in the current block
  {
    VideoParameters *p_Vid = currMB->p_Vid;
    int transform_pl = (p_Vid->separate_colour_plane_flag != 0) ? currMB->p_Slice->colour_plane_id : pl;

    int **tcoeffs;
    int i,j,k;
    int level = 1;

    DataPartition *dP;
    Slice *currSlice = currMB->p_Slice;
    const byte *partMap = assignSE2partition[currSlice->dp_mode];
    int boff_x, boff_y;

    int64 cbp_mask = (int64) 51 << (4 * b8 - 2 * (b8 & 0x01)); // corresponds to 110011, as if all four 4x4 blocks contain coeff, shifted to block position            
    int64 *cur_cbp = &currMB->cbp_blk[pl];

    // select scan type
    const byte (*pos_scan8x8) = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8[0] : FIELD_SCAN8x8[0];

    int qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[transform_pl/*pl*/] ];
    int qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[transform_pl/*pl*/] ];

    int (*InvLevelScale8x8)[8] = (currMB->is_intra_block == TRUE) ? currSlice->InvLevelScale8x8_Intra[transform_pl][qp_rem] : currSlice->InvLevelScale8x8_Inter[transform_pl][qp_rem];

    // === set offset in current macroblock ===
    boff_x = (b8&0x01) << 3;
    boff_y = (b8 >> 1) << 3;
    tcoeffs = &currSlice->mb_rres[pl][boff_y];

    currMB->subblock_x = boff_x; // position for coeff_count ctx
    currMB->subblock_y = boff_y; // position for coeff_count ctx

    if (pl==PLANE_Y || (p_Vid->separate_colour_plane_flag != 0))  
      currSE->context = LUMA_8x8;
    else if (pl==PLANE_U)
      currSE->context = CB_8x8;
    else
      currSE->context = CR_8x8;  

    currSE->reading = readRunLevel_CABAC;

    // Read DC
    currSE->type = ((currMB->is_intra_block == 1) ? SE_LUM_DC_INTRA : SE_LUM_DC_INTER ); // Intra or Inter?
    dP = &(currSlice->partArr[partMap[currSE->type]]);

#if TRACE
    if (pl==PLANE_Y)
      sprintf(currSE->tracestring, "Luma8x8 DC sng ");
    else if (pl==PLANE_U)
      sprintf(currSE->tracestring, "Cb  8x8 DC sng "); 
    else 
      sprintf(currSE->tracestring, "Cr  8x8 DC sng "); 
#endif        

    dP->readSyntaxElement(currMB, currSE, dP);
    level = currSE->value1;

    //============ decode =============
    if (level != 0)    /* leave if level == 0 */
    {
      *cur_cbp |= cbp_mask; 

      pos_scan8x8 += 2 * (currSE->value2);

      i = *pos_scan8x8++;
      j = *pos_scan8x8++;

      tcoeffs[j][boff_x + i] = rshift_rnd_sf((level * InvLevelScale8x8[j][i]) << qp_per, 6); // dequantization
      //tcoeffs[ j][boff_x + i] = level;

      // AC coefficients
      currSE->type    = ((currMB->is_intra_block == 1) ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER);
      dP = &(currSlice->partArr[partMap[currSE->type]]);

      for(k = 1;(k < 65) && (level != 0);++k)
      {
#if TRACE
        if (pl==PLANE_Y)
          sprintf(currSE->tracestring, "Luma8x8 sng ");
        else if (pl==PLANE_U)
          sprintf(currSE->tracestring, "Cb  8x8 sng "); 
        else 
          sprintf(currSE->tracestring, "Cr  8x8 sng "); 
#endif

        dP->readSyntaxElement(currMB, currSE, dP);
        level = currSE->value1;

        //============ decode =============
        if (level != 0)    /* leave if level == 0 */
        {
          pos_scan8x8 += 2 * (currSE->value2);

          i = *pos_scan8x8++;
          j = *pos_scan8x8++;

          tcoeffs[ j][boff_x + i] = rshift_rnd_sf((level * InvLevelScale8x8[j][i]) << qp_per, 6); // dequantization
          //tcoeffs[ j][boff_x + i] = level;
        }
      }
      /*
      for (j = 0; j < 8; j++)
      {
      for (i = 0; i < 8; i++)
      {
      if (tcoeffs[ j][boff_x + i])
      tcoeffs[ j][boff_x + i] = rshift_rnd_sf((tcoeffs[ j][boff_x + i] * InvLevelScale8x8[j][i]) << qp_per, 6); // dequantization
      }
      }
      */
    }        
  }
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of one 8x8 block
*    from the NAL (CABAC Mode - lossless)
************************************************************************
*/
static void readCompCoeff8x8_CABAC_lossless (Macroblock *currMB, SyntaxElement *currSE, ColorPlane pl, int b8)
{
  if (currMB->cbp & (1<<b8))  // are there any coefficients in the current block
  {
    VideoParameters *p_Vid = currMB->p_Vid;

    int **tcoeffs;
    int i,j,k;
    int level = 1;

    DataPartition *dP;
    Slice *currSlice = currMB->p_Slice;
    const byte *partMap = assignSE2partition[currSlice->dp_mode];
    int boff_x, boff_y;

    int64 cbp_mask = (int64) 51 << (4 * b8 - 2 * (b8 & 0x01)); // corresponds to 110011, as if all four 4x4 blocks contain coeff, shifted to block position            
    int64 *cur_cbp = &currMB->cbp_blk[pl];

    // select scan type
    const byte (*pos_scan8x8) = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8[0] : FIELD_SCAN8x8[0];

    // === set offset in current macroblock ===
    boff_x = (b8&0x01) << 3;
    boff_y = (b8 >> 1) << 3;
    tcoeffs = &currSlice->mb_rres[pl][boff_y];

    currMB->subblock_x = boff_x; // position for coeff_count ctx
    currMB->subblock_y = boff_y; // position for coeff_count ctx

    if (pl==PLANE_Y || (p_Vid->separate_colour_plane_flag != 0))  
      currSE->context = LUMA_8x8;
    else if (pl==PLANE_U)
      currSE->context = CB_8x8;
    else
      currSE->context = CR_8x8;  

    currSE->reading = readRunLevel_CABAC;

    for(k=0; (k < 65) && (level != 0);++k)
    {
      //============ read =============
      /*
      * make distinction between INTRA and INTER coded
      * luminance coefficients
      */

      currSE->type    = ((currMB->is_intra_block == 1)
        ? (k==0 ? SE_LUM_DC_INTRA : SE_LUM_AC_INTRA) 
        : (k==0 ? SE_LUM_DC_INTER : SE_LUM_AC_INTER));

#if TRACE
      if (pl==PLANE_Y)
        sprintf(currSE->tracestring, "Luma8x8 sng ");
      else if (pl==PLANE_U)
        sprintf(currSE->tracestring, "Cb  8x8 sng "); 
      else 
        sprintf(currSE->tracestring, "Cr  8x8 sng "); 
#endif

      dP = &(currSlice->partArr[partMap[currSE->type]]);
      currSE->reading = readRunLevel_CABAC;

      dP->readSyntaxElement(currMB, currSE, dP);
      level = currSE->value1;

      //============ decode =============
      if (level != 0)    /* leave if level == 0 */
      {
        pos_scan8x8 += 2 * (currSE->value2);

        i = *pos_scan8x8++;
        j = *pos_scan8x8++;

        *cur_cbp |= cbp_mask;

        tcoeffs[j][boff_x + i] = level;
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 8x8 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8MB_CABAC (Macroblock *currMB, SyntaxElement *currSE, ColorPlane pl)
{
  //======= 8x8 transform size & CABAC ========
  if(currMB->is_lossless == FALSE)
  {
    readCompCoeff8x8_CABAC (currMB, currSE, pl, 0); 
    readCompCoeff8x8_CABAC (currMB, currSE, pl, 1); 
    readCompCoeff8x8_CABAC (currMB, currSE, pl, 2); 
    readCompCoeff8x8_CABAC (currMB, currSE, pl, 3); 
  }
  else
  {
    readCompCoeff8x8_CABAC_lossless (currMB, currSE, pl, 0); 
    readCompCoeff8x8_CABAC_lossless (currMB, currSE, pl, 1); 
    readCompCoeff8x8_CABAC_lossless (currMB, currSE, pl, 2); 
    readCompCoeff8x8_CABAC_lossless (currMB, currSE, pl, 3); 
  }
}

/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff4x4MB_CAVLC (Macroblock *currMB, ColorPlane pl, int (*InvLevelScale4x4)[4], int qp_per, int cbp, byte **nzcoeff)
{
  int block_y, block_x, b8;
  int i, j, k;
  int i0, j0;
  int levarr[16] = {0}, runarr[16] = {0}, numcoeff;
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];
  int start_scan = IS_I16MB(currMB) ? 1 : 0;
  int64 *cur_cbp = &currMB->cbp_blk[pl];
  int coef_ctr, cur_context; 

  if (IS_I16MB(currMB))
  {
    if (pl == PLANE_Y)
      cur_context = LUMA_INTRA16x16AC;
    else if (pl == PLANE_U)
      cur_context = CB_INTRA16x16AC;
    else
      cur_context = CR_INTRA16x16AC;
  }
  else
  {
    if (pl == PLANE_Y)
      cur_context = LUMA;
    else if (pl == PLANE_U)
      cur_context = CB;
    else
      cur_context = CR;
  }

  if (currMB->is_lossless == FALSE)
  {
    int block_y4, block_x4;
    for (block_y = 0; block_y < 4; block_y += 2) /* all modes */
    {
      block_y4 = block_y << 2;
      for (block_x = 0; block_x < 4; block_x += 2)
      {
        block_x4 = block_x << 2;
        b8 = (block_y + (block_x >> 1));

        if (cbp & (1 << b8))  // test if the block contains any coefficients
        {
          for (j = block_y4; j < block_y4 + 8; j += BLOCK_SIZE)
          {
            for (i = block_x4; i < block_x4 + 8; i += BLOCK_SIZE)
            {
              readCoeff4x4_CAVLC(currMB, cur_context, i >> 2, j >> 2, levarr, runarr, &numcoeff);
              pos_scan_4x4 = pos_scan4x4[start_scan];

              for (k = 0; k < numcoeff; ++k)
              {
                if (levarr[k] != 0)
                {
                  pos_scan_4x4 += (runarr[k] << 1);

                  i0 = *pos_scan_4x4++;
                  j0 = *pos_scan_4x4++;

                  // inverse quant for 4x4 transform only
                  *cur_cbp |= i64_power2(j + (i >> 2));

                  currSlice->cof[pl][j + j0][i + i0]= rshift_rnd_sf((levarr[k] * InvLevelScale4x4[j0][i0])<<qp_per, 4);
                  //p_Vid->fcf[0][(j<<2) + j0][(i<<2) + i0]= levarr[k];
                }
              }
            }
          }
        }
        else
        {
          memset(&nzcoeff[block_y    ][block_x], 0, 2 * sizeof(byte));
          memset(&nzcoeff[block_y + 1][block_x], 0, 2 * sizeof(byte));
        }
      }
    }
  }
  else
  {   
    for (block_y=0; block_y < 4; block_y += 2) /* all modes */
    {
      for (block_x=0; block_x < 4; block_x += 2)
      {
        b8 = 2*(block_y>>1) + (block_x>>1);

        if (cbp & (1<<b8))  /* are there any coeff in current block at all */
        {
          for (j=block_y; j < block_y+2; ++j)
          {
            for (i=block_x; i < block_x+2; ++i)
            {
              readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

              coef_ctr = start_scan - 1;

              for (k = 0; k < numcoeff; ++k)
              {
                if (levarr[k] != 0)
                {
                  coef_ctr += runarr[k]+1;

                  i0=pos_scan4x4[coef_ctr][0];
                  j0=pos_scan4x4[coef_ctr][1];

                  *cur_cbp |= i64_power2((j<<2) + i);
                  currSlice->cof[pl][(j<<2) + j0][(i<<2) + i0]= levarr[k];
                  //p_Vid->fcf[0][(j<<2) + j0][(i<<2) + i0]= levarr[k];
                }
              }
            }
          }
        }
        else
        {
          memset(&nzcoeff[block_y    ][block_x], 0, 2 * sizeof(byte));
          memset(&nzcoeff[block_y + 1][block_x], 0, 2 * sizeof(byte));
        }
      }
    }
  }  
}


/*!
************************************************************************
* \brief
*    Get coefficients (run/level) of 4x4 blocks in a MB
*    from the NAL (CABAC Mode)
************************************************************************
*/
static void readCompCoeff8x8MB_CAVLC (Macroblock *currMB, ColorPlane pl, int (*InvLevelScale8x8)[8], int qp_per, int cbp, byte **nzcoeff)
{
  int block_y, block_x, b4, b8;
  int i, j, k;
  int i0, j0;
  int levarr[16] = {0}, runarr[16] = {0}, numcoeff;
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  const byte (*pos_scan8x8)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN8x8 : FIELD_SCAN8x8;
  int start_scan = IS_I16MB(currMB) ? 1 : 0;
  int64 *cur_cbp = &currMB->cbp_blk[pl];
  int coef_ctr, cur_context; 

  if (IS_I16MB(currMB))
  {
    if (pl == PLANE_Y)
      cur_context = LUMA_INTRA16x16AC;
    else if (pl == PLANE_U)
      cur_context = CB_INTRA16x16AC;
    else
      cur_context = CR_INTRA16x16AC;
  }
  else
  {
    if (pl == PLANE_Y)
      cur_context = LUMA;
    else if (pl == PLANE_U)
      cur_context = CB;
    else
      cur_context = CR;
  }

  if (currMB->is_lossless == FALSE)
  {    
    int block_y4, block_x4;
    for (block_y = 0; block_y < 4; block_y += 2) /* all modes */
    {
      block_y4 = block_y << 2;

      for (block_x = 0; block_x < 4; block_x += 2)
      {
        block_x4 = block_x << 2;
        b8 = block_y + (block_x>>1);

        if (cbp & (1<<b8))  /* are there any coeff in current block at all */
        {
          for (j = block_y; j < block_y + 2; ++j)
          {
            for (i = block_x; i < block_x + 2; ++i)
            {
              readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

              coef_ctr = start_scan - 1;

              for (k = 0; k < numcoeff; ++k)
              {
                if (levarr[k] != 0)
                {
                  coef_ctr += runarr[k] + 1;

                  // do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
                  //as containing coefficients
                  *cur_cbp |= 51 << (block_y4 + block_x);

                  b4 = (coef_ctr << 2) + 2*(j - block_y) + (i - block_x);

                  i0 = pos_scan8x8[b4][0];
                  j0 = pos_scan8x8[b4][1];

                  currSlice->mb_rres[pl][block_y4 +j0][block_x4 +i0] = rshift_rnd_sf((levarr[k] * InvLevelScale8x8[j0][i0])<<qp_per, 6); // dequantization
                }
              }//else (!currMB->luma_transform_size_8x8_flag)
            }
          }
        }
        else
        {
          memset(&nzcoeff[block_y    ][block_x], 0, 2 * sizeof(byte));
          memset(&nzcoeff[block_y + 1][block_x], 0, 2 * sizeof(byte));
        }
      }
    }
  }
  else // inverse quant for 8x8 transform
  {
    for (block_y=0; block_y < 4; block_y += 2) /* all modes */
    {
      for (block_x=0; block_x < 4; block_x += 2)
      {
        b8 = 2*(block_y>>1) + (block_x>>1);

        if (cbp & (1<<b8))  /* are there any coeff in current block at all */
        {
          int iz, jz;

          for (j=block_y; j < block_y+2; ++j)
          {
            for (i=block_x; i < block_x+2; ++i)
            {

              readCoeff4x4_CAVLC(currMB, cur_context, i, j, levarr, runarr, &numcoeff);

              coef_ctr = start_scan - 1;

              for (k = 0; k < numcoeff; ++k)
              {
                if (levarr[k] != 0)
                {
                  coef_ctr += runarr[k]+1;

                  // do same as CABAC for deblocking: any coeff in the 8x8 marks all the 4x4s
                  //as containing coefficients
                  *cur_cbp  |= 51 << ((block_y<<2) + block_x);

                  b4 = 2*(j-block_y)+(i-block_x);

                  iz=pos_scan8x8[coef_ctr*4+b4][0];
                  jz=pos_scan8x8[coef_ctr*4+b4][1];

                  currSlice->mb_rres[pl][block_y*4 +jz][block_x*4 +iz] = levarr[k];
                }
              }
            }
          }
        }
        else
        {
          memset(&nzcoeff[block_y    ][block_x], 0, 2 * sizeof(byte));
          memset(&nzcoeff[block_y + 1][block_x], 0, 2 * sizeof(byte));
        }
      }
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Data partitioning: Check if neighboring macroblock is needed for 
 *    CAVLC context decoding, and disable current MB if data partition
 *    is missing.
 ************************************************************************
 */
static void check_dp_neighbors (Macroblock *currMB)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  PixelPos up, left;

  p_Vid->getNeighbour(currMB, -1,  0, p_Vid->mb_size[1], &left);
  p_Vid->getNeighbour(currMB,  0, -1, p_Vid->mb_size[1], &up);

  if ((currMB->is_intra_block == FALSE) || (!(p_Vid->active_pps->constrained_intra_pred_flag)) )
  {
    if (left.available)
    {
      currMB->dpl_flag |= p_Vid->mb_data[left.mb_addr].dpl_flag;
    }
    if (up.available)
    {
      currMB->dpl_flag |= p_Vid->mb_data[up.mb_addr].dpl_flag;
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CABAC_420(Macroblock *currMB)
{
  int i,j;
  int level;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int i0, j0;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;
  int smb = ((p_Vid->type==SP_SLICE) && (currMB->is_intra_block == FALSE)) || (p_Vid->type == SI_SLICE && currMB->mb_type == SI4MB);

  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);  

  StorablePicture *dec_picture = currSlice->dec_picture;
  int yuv = dec_picture->chroma_format_idc - 1;

  int (*InvLevelScale4x4)[4] = NULL;

  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];

  if (!IS_I16MB (currMB))
  {
    int need_transform_size_flag;
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    if (dP->bitstream->ei_flag)
    {
      currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
        ? currSlice->linfo_cbp_intra
        : currSlice->linfo_cbp_inter;
    }
    else
    {
      currSE.reading = read_CBP_CABAC;
    }

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      if (dP->bitstream->ei_flag)
      {
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      } 
      else
      {
        dP->readSyntaxElement(currMB, &currSE, dP);
      }
      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;
  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      int k;
      pos_scan_4x4 = pos_scan4x4[0];

      currSE.type = SE_LUM_DC_INTRA;
      dP = &(currSlice->partArr[partMap[currSE.type]]);

      currSE.context      = LUMA_16DC;
      currSE.type         = SE_LUM_DC_INTRA;

      if (dP->bitstream->ei_flag)
      {
        currSE.mapping = linfo_levrun_inter;
      }
      else
      {
        currSE.reading = readRunLevel_CABAC;
      }

      level = 1;                            // just to get inside the loop

      for(k = 0; (k < 17) && (level != 0); ++k)
      {
#if TRACE
        snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC luma 16x16 ");
#endif
        dP->readSyntaxElement(currMB, &currSE, dP);
        level = currSE.value1;

        if (level != 0)    /* leave if level == 0 */
        {
          pos_scan_4x4 += (2 * currSE.value2);

          i0 = ((*pos_scan_4x4++) << 2);
          j0 = ((*pos_scan_4x4++) << 2);

          currSlice->cof[0][j0][i0] = level;// add new intra DC coeff
          //p_Vid->fcf[0][j0][i0] = level;// add new intra DC coeff
        }
      }


      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  // luma coefficients
  //======= Other Modes & CABAC ========
  //------------------------------------          
  if (cbp)
  {
    if(currMB->luma_transform_size_8x8_flag) 
    {
      //======= 8x8 transform size & CABAC ========
      readCompCoeff8x8MB_CABAC (currMB, &currSE, PLANE_Y); 
    }
    else
    {
      InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
      readCompCoeff4x4MB_CABAC (currMB, &currSE, PLANE_Y, InvLevelScale4x4, qp_per, cbp);        
    }
  }

  //init quant parameters for chroma 
  for(i=0; i < 2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }

  //========================== CHROMA DC ============================
  //-----------------------------------------------------------------
  // chroma DC coeff
  if(cbp>15)
  {
    int uv, ll, k, coef_ctr;
    for (ll=0;ll<3;ll+=2)
    {
      uv = ll>>1;          

      InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
      //===================== CHROMA DC YUV420 ======================
      memset(currSlice->cofu, 0, 4 *sizeof(int));
      coef_ctr=-1;

      level = 1;
      currMB->is_v_block     = ll;
      currSE.context      = CHROMA_DC;
      currSE.type         = (intra ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER);

      dP = &(currSlice->partArr[partMap[currSE.type]]);

      if (dP->bitstream->ei_flag)
        currSE.mapping = linfo_levrun_c2x2;
      else
        currSE.reading = readRunLevel_CABAC;

      for(k = 0; (k < (p_Vid->num_cdc_coeff + 1))&&(level!=0);++k)
      {
#if TRACE
        snprintf(currSE.tracestring, TRACESTRING_SIZE, "2x2 DC Chroma ");
#endif

        dP->readSyntaxElement(currMB, &currSE, dP);
        level = currSE.value1;

        if (level != 0)
        {
          currMB->cbp_blk[0] |= 0xf0000 << (ll<<1) ;
          coef_ctr += currSE.value2 + 1;

          // Bug: currSlice->cofu has only 4 entries, hence coef_ctr MUST be <4 (which is
          // caught by the assert().  If it is bigger than 4, it starts patching the
          // p_Vid->predmode pointer, which leads to bugs later on.
          //
          // This assert() should be left in the code, because it captures a very likely
          // bug early when testing in error prone environments (or when testing NAL
          // functionality).

          assert (coef_ctr < p_Vid->num_cdc_coeff);
          currSlice->cofu[coef_ctr]=level;
        }
      }


      if (smb || (currMB->is_lossless == TRUE)) // check to see if MB type is SPred or SIntra4x4
      {
        currSlice->cof[uv + 1][0][0] = currSlice->cofu[0];
        currSlice->cof[uv + 1][0][4] = currSlice->cofu[1];
        currSlice->cof[uv + 1][4][0] = currSlice->cofu[2];
        currSlice->cof[uv + 1][4][4] = currSlice->cofu[3];
        //p_Vid->fcf[uv + 1][0][0] = currSlice->cofu[0];
        //p_Vid->fcf[uv + 1][4][0] = currSlice->cofu[1];
        //p_Vid->fcf[uv + 1][0][4] = currSlice->cofu[2];
        //p_Vid->fcf[uv + 1][4][4] = currSlice->cofu[3];
      }
      else
      {
        int temp[4];
        int scale_dc = InvLevelScale4x4[0][0];
        ihadamard2x2(currSlice->cofu, temp);
        //p_Vid->fcf[uv + 1][0][0] = temp[0];
        //p_Vid->fcf[uv + 1][0][4] = temp[1];
        //p_Vid->fcf[uv + 1][4][0] = temp[2];
        //p_Vid->fcf[uv + 1][4][4] = temp[3];

        currSlice->cof[uv + 1][0][0] = (((temp[0] * scale_dc)<<qp_per_uv[uv])>>5);
        currSlice->cof[uv + 1][0][4] = (((temp[1] * scale_dc)<<qp_per_uv[uv])>>5);
        currSlice->cof[uv + 1][4][0] = (((temp[2] * scale_dc)<<qp_per_uv[uv])>>5);
        currSlice->cof[uv + 1][4][4] = (((temp[3] * scale_dc)<<qp_per_uv[uv])>>5);
      }          
    }      
  }

  //========================== CHROMA AC ============================
  //-----------------------------------------------------------------
  // chroma AC coeff, all zero fram start_scan
  if (cbp >31)
  {
    currSE.context      = CHROMA_AC;
    currSE.type         = (currMB->is_intra_block ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER);

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    if (dP->bitstream->ei_flag)
      currSE.mapping = linfo_levrun_inter;
    else
      currSE.reading = readRunLevel_CABAC;

    if(currMB->is_lossless == FALSE)
    {
      int b4, b8, uv, k;
      for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
      {
        currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
        InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

        for (b4 = 0; b4 < 4; ++b4)
        {
          i = cofuv_blk_x[yuv][b8][b4];
          j = cofuv_blk_y[yuv][b8][b4];

          currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
          currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

          pos_scan_4x4 = pos_scan4x4[1];
          level = 1;

          for(k = 0; (k < 16) && (level != 0);++k)
          {
#if TRACE
            snprintf(currSE.tracestring, TRACESTRING_SIZE, "AC Chroma ");
#endif

            dP->readSyntaxElement(currMB, &currSE, dP);
            level = currSE.value1;

            if (level != 0)
            {
              currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
              pos_scan_4x4 += (currSE.value2 << 1);

              i0 = *pos_scan_4x4++;
              j0 = *pos_scan_4x4++;

              currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = rshift_rnd_sf((level * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
              //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
            }
          } //for(k=0;(k<16)&&(level!=0);++k)
        }
      }
    }
    else
    {
      int b4, b8, k;
      int uv;
      for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
      {
        currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

        for (b4=0; b4 < 4; ++b4)
        {
          i = cofuv_blk_x[yuv][b8][b4];
          j = cofuv_blk_y[yuv][b8][b4];

          pos_scan_4x4 = pos_scan4x4[1];
          level=1;

          currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
          currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

          for(k=0;(k<16)&&(level!=0);++k)
          {
#if TRACE
            snprintf(currSE.tracestring, TRACESTRING_SIZE, "AC Chroma ");
#endif
            dP->readSyntaxElement(currMB, &currSE, dP);
            level = currSE.value1;

            if (level != 0)
            {
              currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
              pos_scan_4x4 += (currSE.value2 << 1);

              i0 = *pos_scan_4x4++;
              j0 = *pos_scan_4x4++;

              currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
              //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
            }
          } 
        }
      } 
    } //for (b4=0; b4 < 4; b4++)      
  }  
}


/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CABAC_400(Macroblock *currMB)
{
  int k;
  int level;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int i0, j0;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int intra = (currMB->is_intra_block == TRUE);

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];


  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    if (dP->bitstream->ei_flag)
    {
      currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
        ? currSlice->linfo_cbp_intra
        : currSlice->linfo_cbp_inter;
    }
    else
    {
      currSE.reading = read_CBP_CABAC;
    }

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      if (dP->bitstream->ei_flag)
      {
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      } 
      else
      {
        dP->readSyntaxElement(currMB, &currSE, dP);
      }
      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      {
        currSE.type = SE_LUM_DC_INTRA;
        dP = &(currSlice->partArr[partMap[currSE.type]]);

        currSE.context      = LUMA_16DC;
        currSE.type         = SE_LUM_DC_INTRA;

        if (dP->bitstream->ei_flag)
        {
          currSE.mapping = linfo_levrun_inter;
        }
        else
        {
          currSE.reading = readRunLevel_CABAC;
        }

        level = 1;                            // just to get inside the loop

        for(k = 0; (k < 17) && (level != 0); ++k)
        {
#if TRACE
          snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC luma 16x16 ");
#endif
          dP->readSyntaxElement(currMB, &currSE, dP);
          level = currSE.value1;

          if (level != 0)    /* leave if level == 0 */
          {
            pos_scan_4x4 += (2 * currSE.value2);

            i0 = ((*pos_scan_4x4++) << 2);
            j0 = ((*pos_scan_4x4++) << 2);

            currSlice->cof[0][j0][i0] = level;// add new intra DC coeff
            //p_Vid->fcf[0][j0][i0] = level;// add new intra DC coeff
          }
        }
      }

      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //======= Other Modes & CABAC ========
  //------------------------------------          
  if (cbp)
  {
    if(currMB->luma_transform_size_8x8_flag) 
    {
      //======= 8x8 transform size & CABAC ========
      readCompCoeff8x8MB_CABAC (currMB, &currSE, PLANE_Y); 
    }
    else
    {
      InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
      readCompCoeff4x4MB_CABAC (currMB, &currSE, PLANE_Y, InvLevelScale4x4, qp_per, cbp);        
    }
  }  
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CABAC_444(Macroblock *currMB)
{
  int i, k;
  int level;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr, i0, j0;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int uv; 
  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);


  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];

  // QPI
  //init constants for every chroma qp offset
  for (i=0; i<2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }


  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    if (dP->bitstream->ei_flag)
    {
      currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
        ? currSlice->linfo_cbp_intra
        : currSlice->linfo_cbp_inter;
    }
    else
    {
      currSE.reading = read_CBP_CABAC;
    }

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      if (dP->bitstream->ei_flag)
      {
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      } 
      else
      {
        dP->readSyntaxElement(currMB, &currSE, dP);
      }
      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;
  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      {
        currSE.type = SE_LUM_DC_INTRA;
        dP = &(currSlice->partArr[partMap[currSE.type]]);

        currSE.context      = LUMA_16DC;
        currSE.type         = SE_LUM_DC_INTRA;

        if (dP->bitstream->ei_flag)
        {
          currSE.mapping = linfo_levrun_inter;
        }
        else
        {
          currSE.reading = readRunLevel_CABAC;
        }

        level = 1;                            // just to get inside the loop

        for(k = 0; (k < 17) && (level != 0); ++k)
        {
#if TRACE
          snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC luma 16x16 ");
#endif
          dP->readSyntaxElement(currMB, &currSE, dP);
          level = currSE.value1;

          if (level != 0)    /* leave if level == 0 */
          {
            pos_scan_4x4 += (2 * currSE.value2);

            i0 = ((*pos_scan_4x4++) << 2);
            j0 = ((*pos_scan_4x4++) << 2);

            currSlice->cof[0][j0][i0] = level;// add new intra DC coeff
            //p_Vid->fcf[0][j0][i0] = level;// add new intra DC coeff
          }
        }
      }

      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //init quant parameters for chroma 
  for(i=0; i < 2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }


  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  {
    //======= Other Modes & CABAC ========
    //------------------------------------          
    if (cbp)
    {
      if(currMB->luma_transform_size_8x8_flag) 
      {
        //======= 8x8 transform size & CABAC ========
        readCompCoeff8x8MB_CABAC (currMB, &currSE, PLANE_Y); 
      }
      else
      {
        readCompCoeff4x4MB_CABAC (currMB, &currSE, PLANE_Y, InvLevelScale4x4, qp_per, cbp);        
      }
    }
  }

  for (uv = 0; uv < 2; ++uv )
  {
    /*----------------------16x16DC Luma_Add----------------------*/
    if (IS_I16MB (currMB)) // read DC coeffs for new intra modes       
    {
      {              
        currSE.type = SE_LUM_DC_INTRA;
        dP = &(currSlice->partArr[partMap[currSE.type]]);

        if( (p_Vid->separate_colour_plane_flag != 0) )
          currSE.context = LUMA_16DC; 
        else
          currSE.context = (uv==0) ? CB_16DC : CR_16DC;

        if (dP->bitstream->ei_flag)
        {
          currSE.mapping = linfo_levrun_inter;
        }
        else
        {
          currSE.reading = readRunLevel_CABAC;
        }

        coef_ctr = -1;
        level = 1;                            // just to get inside the loop

        for(k=0;(k<17) && (level!=0);++k)
        {
#if TRACE
          if (uv == 0)
            snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC Cb   16x16 "); 
          else
            snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC Cr   16x16 ");
#endif

          dP->readSyntaxElement(currMB, &currSE, dP);
          level = currSE.value1;

          if (level != 0)                     // leave if level == 0
          {
            coef_ctr += currSE.value2 + 1;

            i0 = pos_scan4x4[coef_ctr][0];
            j0 = pos_scan4x4[coef_ctr][1];
            currSlice->cof[uv + 1][j0<<2][i0<<2] = level;
            //p_Vid->fcf[uv + 1][j0<<2][i0<<2] = level;
          }                        
        } //k loop
      } // else CAVLC

      if(currMB->is_lossless == FALSE)
      {
        itrans_2(currMB, (ColorPlane) (uv + 1)); // transform new intra DC
      }
    } //IS_I16MB

    update_qp(currMB, currSlice->qp);

    qp_per = p_Vid->qp_per_matrix[ (currSlice->qp + p_Vid->bitdepth_luma_qp_scale) ];
    qp_rem = p_Vid->qp_rem_matrix[ (currSlice->qp + p_Vid->bitdepth_luma_qp_scale) ];

    //init constants for every chroma qp offset
    qp_per_uv[uv] = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];
    qp_rem_uv[uv] = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];

    InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

    {  
      if (cbp)
      {
        if(currMB->luma_transform_size_8x8_flag) 
        {
          //======= 8x8 transform size & CABAC ========
          readCompCoeff8x8MB_CABAC (currMB, &currSE, (ColorPlane) (PLANE_U + uv)); 
        }
        else //4x4
        {        
          readCompCoeff4x4MB_CABAC (currMB, &currSE, (ColorPlane) (PLANE_U + uv), InvLevelScale4x4,  qp_per_uv[uv], cbp);
        }
      }
    }
  } 
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CABAC_422(Macroblock *currMB)
{
  int i,j,k;
  int level;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr, i0, j0, b8;
  int ll;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int uv; 
  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);

  int b4;
  StorablePicture *dec_picture = currSlice->dec_picture;
  int yuv = dec_picture->chroma_format_idc - 1;
  int m6[4];

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];

  // QPI
  //init constants for every chroma qp offset
  for (i=0; i<2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }

  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    if (dP->bitstream->ei_flag)
    {
      currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
        ? currSlice->linfo_cbp_intra
        : currSlice->linfo_cbp_inter;
    }
    else
    {
      currSE.reading = read_CBP_CABAC;
    }

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      if (dP->bitstream->ei_flag)
      {
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      } 
      else
      {
        dP->readSyntaxElement(currMB, &currSE, dP);
      }
      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;
  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      {
        currSE.type = SE_LUM_DC_INTRA;
        dP = &(currSlice->partArr[partMap[currSE.type]]);

        currSE.context      = LUMA_16DC;
        currSE.type         = SE_LUM_DC_INTRA;

        if (dP->bitstream->ei_flag)
        {
          currSE.mapping = linfo_levrun_inter;
        }
        else
        {
          currSE.reading = readRunLevel_CABAC;
        }

        level = 1;                            // just to get inside the loop

        for(k = 0; (k < 17) && (level != 0); ++k)
        {
#if TRACE
          snprintf(currSE.tracestring, TRACESTRING_SIZE, "DC luma 16x16 ");
#endif
          dP->readSyntaxElement(currMB, &currSE, dP);
          level = currSE.value1;

          if (level != 0)    /* leave if level == 0 */
          {
            pos_scan_4x4 += (2 * currSE.value2);

            i0 = ((*pos_scan_4x4++) << 2);
            j0 = ((*pos_scan_4x4++) << 2);

            currSlice->cof[0][j0][i0] = level;// add new intra DC coeff
            //p_Vid->fcf[0][j0][i0] = level;// add new intra DC coeff
          }
        }
      }

      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //init quant parameters for chroma 
  for(i=0; i < 2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }

  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  {
    //======= Other Modes & CABAC ========
    //------------------------------------          
    if (cbp)
    {
      if(currMB->luma_transform_size_8x8_flag) 
      {
        //======= 8x8 transform size & CABAC ========
        readCompCoeff8x8MB_CABAC (currMB, &currSE, PLANE_Y); 
      }
      else
      {
        readCompCoeff4x4MB_CABAC (currMB, &currSE, PLANE_Y, InvLevelScale4x4, qp_per, cbp);        
      }
    }
  }

  //========================== CHROMA DC ============================
  //-----------------------------------------------------------------
  // chroma DC coeff
  if(cbp>15)
  {      
    for (ll=0;ll<3;ll+=2)
    {
      int (*InvLevelScale4x4)[4] = NULL;
      uv = ll>>1;
      {
        int **imgcof = currSlice->cof[uv + 1];
        int m3[2][4] = {{0,0,0,0},{0,0,0,0}};
        int m4[2][4] = {{0,0,0,0},{0,0,0,0}};
        int qp_per_uv_dc = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
        int qp_rem_uv_dc = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
        if (intra)
          InvLevelScale4x4 = currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv_dc];
        else 
          InvLevelScale4x4 = currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv_dc];


        //===================== CHROMA DC YUV422 ======================
        {
          coef_ctr=-1;
          level=1;
          for(k=0;(k<9)&&(level!=0);++k)
          {
            currSE.context      = CHROMA_DC_2x4;
            currSE.type         = ((currMB->is_intra_block == TRUE) ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER);
            currMB->is_v_block     = ll;

#if TRACE
            snprintf(currSE.tracestring, TRACESTRING_SIZE, "2x4 DC Chroma ");
#endif
            dP = &(currSlice->partArr[partMap[currSE.type]]);

            if (dP->bitstream->ei_flag)
              currSE.mapping = linfo_levrun_c2x2;
            else
              currSE.reading = readRunLevel_CABAC;

            dP->readSyntaxElement(currMB, &currSE, dP);

            level = currSE.value1;

            if (level != 0)
            {
              currMB->cbp_blk[0] |= ((int64)0xff0000) << (ll<<2) ;
              coef_ctr += currSE.value2 + 1;
              assert (coef_ctr < p_Vid->num_cdc_coeff);
              i0=SCAN_YUV422[coef_ctr][0];
              j0=SCAN_YUV422[coef_ctr][1];

              m3[i0][j0]=level;
            }
          }
        }
        // inverse CHROMA DC YUV422 transform
        // horizontal
        if(currMB->is_lossless == FALSE)
        {
          m4[0][0] = m3[0][0] + m3[1][0];
          m4[0][1] = m3[0][1] + m3[1][1];
          m4[0][2] = m3[0][2] + m3[1][2];
          m4[0][3] = m3[0][3] + m3[1][3];

          m4[1][0] = m3[0][0] - m3[1][0];
          m4[1][1] = m3[0][1] - m3[1][1];
          m4[1][2] = m3[0][2] - m3[1][2];
          m4[1][3] = m3[0][3] - m3[1][3];

          for (i = 0; i < 2; ++i)
          {
            m6[0] = m4[i][0] + m4[i][2];
            m6[1] = m4[i][0] - m4[i][2];
            m6[2] = m4[i][1] - m4[i][3];
            m6[3] = m4[i][1] + m4[i][3];

            imgcof[ 0][i<<2] = m6[0] + m6[3];
            imgcof[ 4][i<<2] = m6[1] + m6[2];
            imgcof[ 8][i<<2] = m6[1] - m6[2];
            imgcof[12][i<<2] = m6[0] - m6[3];
          }//for (i=0;i<2;++i)

          for(j = 0;j < p_Vid->mb_cr_size_y; j += BLOCK_SIZE)
          {
            for(i=0;i < p_Vid->mb_cr_size_x;i+=BLOCK_SIZE)
            {
              imgcof[j][i] = rshift_rnd_sf((imgcof[j][i] * InvLevelScale4x4[0][0]) << qp_per_uv_dc, 6);
            }
          }
        }
        else
        {
          for(j=0;j<4;++j)
          {
            for(i=0;i<2;++i)                
            {
              currSlice->cof[uv + 1][j<<2][i<<2] = m3[i][j];
              //p_Vid->fcf[uv + 1][j<<2][i<<2] = m3[i][j];
            }
          }
        }

      }
    }//for (ll=0;ll<3;ll+=2)      
  }

  //========================== CHROMA AC ============================
  //-----------------------------------------------------------------
  // chroma AC coeff, all zero fram start_scan
  if (cbp<=31)
  {
  }
  else
  {
    {
      currSE.context      = CHROMA_AC;
      currSE.type         = (currMB->is_intra_block ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER);

      dP = &(currSlice->partArr[partMap[currSE.type]]);

      if (dP->bitstream->ei_flag)
        currSE.mapping = linfo_levrun_inter;
      else
        currSE.reading = readRunLevel_CABAC;

      if(currMB->is_lossless == FALSE)
      {          
        for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
        {
          currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
          InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

          for (b4 = 0; b4 < 4; ++b4)
          {
            i = cofuv_blk_x[yuv][b8][b4];
            j = cofuv_blk_y[yuv][b8][b4];

            currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
            currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

            pos_scan_4x4 = pos_scan4x4[1];
            level=1;

            for(k = 0; (k < 16) && (level != 0);++k)
            {
#if TRACE
              snprintf(currSE.tracestring, TRACESTRING_SIZE, "AC Chroma ");
#endif

              dP->readSyntaxElement(currMB, &currSE, dP);
              level = currSE.value1;

              if (level != 0)
              {
                currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
                pos_scan_4x4 += (currSE.value2 << 1);

                i0 = *pos_scan_4x4++;
                j0 = *pos_scan_4x4++;

                currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = rshift_rnd_sf((level * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
                //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
              }
            } //for(k=0;(k<16)&&(level!=0);++k)
          }
        }
      }
      else
      {
        for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
        {
          currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

          for (b4=0; b4 < 4; ++b4)
          {
            i = cofuv_blk_x[yuv][b8][b4];
            j = cofuv_blk_y[yuv][b8][b4];

            pos_scan_4x4 = pos_scan4x4[1];
            level=1;

            currMB->subblock_y = subblk_offset_y[yuv][b8][b4];
            currMB->subblock_x = subblk_offset_x[yuv][b8][b4];

            for(k=0;(k<16)&&(level!=0);++k)
            {
#if TRACE
              snprintf(currSE.tracestring, TRACESTRING_SIZE, "AC Chroma ");
#endif
              dP->readSyntaxElement(currMB, &currSE, dP);
              level = currSE.value1;

              if (level != 0)
              {
                currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
                pos_scan_4x4 += (currSE.value2 << 1);

                i0 = *pos_scan_4x4++;
                j0 = *pos_scan_4x4++;

                currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
                //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = level;
              }
            } 
          }
        } 
      } //for (b4=0; b4 < 4; b4++)
    } //for (b8=0; b8 < p_Vid->num_blk8x8_uv; b8++)
  } //if (dec_picture->chroma_format_idc != YUV400)  
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CAVLC_400(Macroblock *currMB)
{
  int k;
  int mb_nr = currMB->mbAddrX;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int i0, j0;

  int levarr[16], runarr[16], numcoeff;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int intra = (currMB->is_intra_block == TRUE);

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  int (*InvLevelScale8x8)[8] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];


  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
      ? currSlice->linfo_cbp_intra
      : currSlice->linfo_cbp_inter;

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      currSE.len = 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);

      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else  // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;
  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      readCoeff4x4_CAVLC(currMB, LUMA_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

      for(k = 0; k < numcoeff; ++k)
      {
        if (levarr[k] != 0)                     // leave if level == 0
        {
          pos_scan_4x4 += 2 * runarr[k];

          i0 = ((*pos_scan_4x4++) << 2);
          j0 = ((*pos_scan_4x4++) << 2);

          currSlice->cof[0][j0][i0] = levarr[k];// add new intra DC coeff
          //p_Vid->fcf[0][j0][i0] = levarr[k];// add new intra DC coeff
        }
      }


      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
  InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale8x8_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  if (cbp)
  {
    if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
    {
      readCompCoeff4x4MB_CAVLC (currMB, PLANE_Y, InvLevelScale4x4, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
    else // 8x8 transform
    {
      readCompCoeff8x8MB_CAVLC (currMB, PLANE_Y, InvLevelScale8x8, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
  }
  else
  {
    fast_memset(p_Vid->nz_coeff[mb_nr][0][0], 0, BLOCK_PIXELS * sizeof(byte));
  }
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CAVLC_422(Macroblock *currMB)
{
  int i,j,k;
  int level;
  int mb_nr = currMB->mbAddrX;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr, i0, j0, b8;
  int ll;
  int levarr[16], runarr[16], numcoeff;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int uv; 
  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);

  int b4;
  StorablePicture *dec_picture = currSlice->dec_picture;
  int yuv = dec_picture->chroma_format_idc - 1;
  int m6[4];

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  int (*InvLevelScale8x8)[8] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];


  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
      ? currSlice->linfo_cbp_intra
      : currSlice->linfo_cbp_inter;

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      currSE.len = 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);

      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else  // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;

    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      readCoeff4x4_CAVLC(currMB, LUMA_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

      for(k = 0; k < numcoeff; ++k)
      {
        if (levarr[k] != 0)                     // leave if level == 0
        {
          pos_scan_4x4 += 2 * runarr[k];

          i0 = ((*pos_scan_4x4++) << 2);
          j0 = ((*pos_scan_4x4++) << 2);

          currSlice->cof[0][j0][i0] = levarr[k];// add new intra DC coeff
          //p_Vid->fcf[0][j0][i0] = levarr[k];// add new intra DC coeff
        }
      }


      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //init quant parameters for chroma 
  for(i=0; i < 2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }

  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
  InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale8x8_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  if (cbp)
  {
    if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
    {
      readCompCoeff4x4MB_CAVLC (currMB, PLANE_Y, InvLevelScale4x4, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
    else // 8x8 transform
    {
      readCompCoeff8x8MB_CAVLC (currMB, PLANE_Y, InvLevelScale8x8, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
  }
  else
  {
    fast_memset(p_Vid->nz_coeff[mb_nr][0][0], 0, BLOCK_PIXELS * sizeof(byte));
  }

  //========================== CHROMA DC ============================
  //-----------------------------------------------------------------
  // chroma DC coeff
  if(cbp>15)
  {    
    for (ll=0;ll<3;ll+=2)
    {
      int (*InvLevelScale4x4)[4] = NULL;
      uv = ll>>1;
      {
        int **imgcof = currSlice->cof[uv + 1];
        int m3[2][4] = {{0,0,0,0},{0,0,0,0}};
        int m4[2][4] = {{0,0,0,0},{0,0,0,0}};
        int qp_per_uv_dc = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
        int qp_rem_uv_dc = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + 3 + p_Vid->bitdepth_chroma_qp_scale) ];       //for YUV422 only
        if (intra)
          InvLevelScale4x4 = currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv_dc];
        else 
          InvLevelScale4x4 = currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv_dc];


        //===================== CHROMA DC YUV422 ======================
        readCoeff4x4_CAVLC(currMB, CHROMA_DC, 0, 0, levarr, runarr, &numcoeff);
        coef_ctr=-1;
        level=1;
        for(k = 0; k < numcoeff; ++k)
        {
          if (levarr[k] != 0)
          {
            currMB->cbp_blk[0] |= ((int64)0xff0000) << (ll<<2);
            coef_ctr += runarr[k]+1;
            i0 = SCAN_YUV422[coef_ctr][0];
            j0 = SCAN_YUV422[coef_ctr][1];

            m3[i0][j0]=levarr[k];
          }
        }

        // inverse CHROMA DC YUV422 transform
        // horizontal
        if(currMB->is_lossless == FALSE)
        {
          m4[0][0] = m3[0][0] + m3[1][0];
          m4[0][1] = m3[0][1] + m3[1][1];
          m4[0][2] = m3[0][2] + m3[1][2];
          m4[0][3] = m3[0][3] + m3[1][3];

          m4[1][0] = m3[0][0] - m3[1][0];
          m4[1][1] = m3[0][1] - m3[1][1];
          m4[1][2] = m3[0][2] - m3[1][2];
          m4[1][3] = m3[0][3] - m3[1][3];

          for (i = 0; i < 2; ++i)
          {
            m6[0] = m4[i][0] + m4[i][2];
            m6[1] = m4[i][0] - m4[i][2];
            m6[2] = m4[i][1] - m4[i][3];
            m6[3] = m4[i][1] + m4[i][3];

            imgcof[ 0][i<<2] = m6[0] + m6[3];
            imgcof[ 4][i<<2] = m6[1] + m6[2];
            imgcof[ 8][i<<2] = m6[1] - m6[2];
            imgcof[12][i<<2] = m6[0] - m6[3];
          }//for (i=0;i<2;++i)

          for(j = 0;j < p_Vid->mb_cr_size_y; j += BLOCK_SIZE)
          {
            for(i=0;i < p_Vid->mb_cr_size_x;i+=BLOCK_SIZE)
            {
              imgcof[j][i] = rshift_rnd_sf((imgcof[j][i] * InvLevelScale4x4[0][0]) << qp_per_uv_dc, 6);
            }
          }
        }
        else
        {
          for(j=0;j<4;++j)
          {
            currSlice->cof[uv + 1][j<<2][0] = m3[0][j];
            currSlice->cof[uv + 1][j<<2][4] = m3[1][j];
          }
        }

      }
    }//for (ll=0;ll<3;ll+=2)    
  }

  //========================== CHROMA AC ============================
  //-----------------------------------------------------------------
  // chroma AC coeff, all zero fram start_scan
  if (cbp<=31)
  {
    fast_memset(p_Vid->nz_coeff [mb_nr ][1][0], 0, 2 * BLOCK_PIXELS * sizeof(byte));
  }
  else
  {
    if(currMB->is_lossless == FALSE)
    {
      for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
      {
        currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
        InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

        for (b4=0; b4 < 4; ++b4)
        {
          i = cofuv_blk_x[yuv][b8][b4];
          j = cofuv_blk_y[yuv][b8][b4];

          readCoeff4x4_CAVLC(currMB, CHROMA_AC, i + 2*uv, j + 4, levarr, runarr, &numcoeff);
          coef_ctr = 0;

          for(k = 0; k < numcoeff;++k)
          {
            if (levarr[k] != 0)
            {
              currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
              coef_ctr += runarr[k] + 1;

              i0=pos_scan4x4[coef_ctr][0];
              j0=pos_scan4x4[coef_ctr][1];

              currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = rshift_rnd_sf((levarr[k] * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
              //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = levarr[k];
            }
          }
        }
      }        
    }
    else
    {
      for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
      {
        currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

        for (b4=0; b4 < 4; ++b4)
        {
          i = cofuv_blk_x[yuv][b8][b4];
          j = cofuv_blk_y[yuv][b8][b4];

          readCoeff4x4_CAVLC(currMB, CHROMA_AC, i + 2*uv, j + 4, levarr, runarr, &numcoeff);
          coef_ctr = 0;

          for(k = 0; k < numcoeff;++k)
          {
            if (levarr[k] != 0)
            {
              currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
              coef_ctr += runarr[k] + 1;

              i0=pos_scan4x4[coef_ctr][0];
              j0=pos_scan4x4[coef_ctr][1];

              currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = levarr[k];
            }
          }
        }
      }        
    }
  } //if (dec_picture->chroma_format_idc != YUV400)
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CAVLC_444(Macroblock *currMB)
{
  int i,k;
  int level;
  int mb_nr = currMB->mbAddrX;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr, i0, j0;
  int levarr[16], runarr[16], numcoeff;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;

  int uv; 
  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  int (*InvLevelScale8x8)[8] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];

  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
      ? currSlice->linfo_cbp_intra
      : currSlice->linfo_cbp_inter;

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;


    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      currSE.len = 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);

      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else  // read DC coeffs for new intra modes
  {
    cbp = currMB->cbp;

    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      readCoeff4x4_CAVLC(currMB, LUMA_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

      for(k = 0; k < numcoeff; ++k)
      {
        if (levarr[k] != 0)                     // leave if level == 0
        {
          pos_scan_4x4 += 2 * runarr[k];

          i0 = ((*pos_scan_4x4++) << 2);
          j0 = ((*pos_scan_4x4++) << 2);

          currSlice->cof[0][j0][i0] = levarr[k];// add new intra DC coeff
          //p_Vid->fcf[0][j0][i0] = levarr[k];// add new intra DC coeff
        }
      }


      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //init quant parameters for chroma 
  for(i=0; i < 2; ++i)
  {
    qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
    qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
  }

  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
  InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale8x8_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  if (cbp)
  {
    if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
    {
      readCompCoeff4x4MB_CAVLC (currMB, PLANE_Y, InvLevelScale4x4, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
    else // 8x8 transform
    {
      readCompCoeff8x8MB_CAVLC (currMB, PLANE_Y, InvLevelScale8x8, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
  }
  else
  {
    fast_memset(p_Vid->nz_coeff[mb_nr][0][0], 0, BLOCK_PIXELS * sizeof(byte));
  }

  for (uv = 0; uv < 2; ++uv )
  {

    /*----------------------16x16DC Luma_Add----------------------*/
    if (IS_I16MB (currMB)) // read DC coeffs for new intra modes       
    {
      if (uv == 0)
        readCoeff4x4_CAVLC(currMB, CB_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);
      else
        readCoeff4x4_CAVLC(currMB, CR_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

      coef_ctr=-1;
      level = 1;                            // just to get inside the loop

      for(k = 0; k < numcoeff; ++k)
      {
        if (levarr[k] != 0)                     // leave if level == 0
        {
          coef_ctr += runarr[k] + 1;

          i0 = pos_scan4x4[coef_ctr][0];
          j0 = pos_scan4x4[coef_ctr][1];
          currSlice->cof[uv + 1][j0<<2][i0<<2] = levarr[k];// add new intra DC coeff
          //p_Vid->fcf[uv + 1][j0<<2][i0<<2] = levarr[k];// add new intra DC coeff
        } //if leavarr[k]
      } //k loop

      if(currMB->is_lossless == FALSE)
      {
        itrans_2(currMB, (ColorPlane) (uv + 1)); // transform new intra DC
      }
    } //IS_I16MB

    update_qp(currMB, currSlice->qp);

    qp_per = p_Vid->qp_per_matrix[ (currSlice->qp + p_Vid->bitdepth_luma_qp_scale) ];
    qp_rem = p_Vid->qp_rem_matrix[ (currSlice->qp + p_Vid->bitdepth_luma_qp_scale) ];

    //init constants for every chroma qp offset
    qp_per_uv[uv] = p_Vid->qp_per_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];
    qp_rem_uv[uv] = p_Vid->qp_rem_matrix[ (currMB->qpc[uv] + p_Vid->bitdepth_chroma_qp_scale) ];

    InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
    InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale8x8_Inter[uv + 1][qp_rem_uv[uv]];

    if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
    {
      readCompCoeff4x4MB_CAVLC (currMB, (ColorPlane) (PLANE_U + uv), InvLevelScale4x4, qp_per_uv[uv], cbp, p_Vid->nz_coeff[mb_nr][PLANE_U + uv]);
    }
    else // 8x8 transform
    {
      readCompCoeff8x8MB_CAVLC (currMB, (ColorPlane) (PLANE_U + uv), InvLevelScale8x8, qp_per_uv[uv], cbp, p_Vid->nz_coeff[mb_nr][PLANE_U + uv]);
    }   
  }   
}

/*!
 ************************************************************************
 * \brief
 *    Get coded block pattern and coefficients (run/level)
 *    from the NAL
 ************************************************************************
 */
static void read_CBP_and_coeffs_from_NAL_CAVLC_420(Macroblock *currMB)
{
  int i,j,k;
  int mb_nr = currMB->mbAddrX;
  int cbp;
  SyntaxElement currSE;
  DataPartition *dP = NULL;
  Slice *currSlice = currMB->p_Slice;
  const byte *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr, i0, j0, b8;
  int ll;
  int levarr[16], runarr[16], numcoeff;

  int qp_per, qp_rem;
  VideoParameters *p_Vid = currMB->p_Vid;
  int smb = ((p_Vid->type==SP_SLICE) && (currMB->is_intra_block == FALSE)) || (p_Vid->type == SI_SLICE && currMB->mb_type == SI4MB);

  int uv; 
  int qp_per_uv[2];
  int qp_rem_uv[2];

  int intra = (currMB->is_intra_block == TRUE);
  int temp[4];

  int b4;
  StorablePicture *dec_picture = currSlice->dec_picture;
  int yuv = dec_picture->chroma_format_idc - 1;

  int need_transform_size_flag;

  int (*InvLevelScale4x4)[4] = NULL;
  int (*InvLevelScale8x8)[8] = NULL;
  // select scan type
  const byte (*pos_scan4x4)[2] = ((p_Vid->structure == FRAME) && (!currMB->mb_field)) ? SNGL_SCAN : FIELD_SCAN;
  const byte *pos_scan_4x4 = pos_scan4x4[0];

  // read CBP if not new intra mode
  if (!IS_I16MB (currMB))
  {
    //=====   C B P   =====
    //---------------------
    currSE.type = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB) 
      ? SE_CBP_INTRA
      : SE_CBP_INTER;

    dP = &(currSlice->partArr[partMap[currSE.type]]);

    currSE.mapping = (currMB->mb_type == I4MB || currMB->mb_type == SI4MB || currMB->mb_type == I8MB)
      ? currSlice->linfo_cbp_intra
      : currSlice->linfo_cbp_inter;

    TRACE_STRING("coded_block_pattern");
    dP->readSyntaxElement(currMB, &currSE, dP);
    currMB->cbp = cbp = currSE.value1;

    //============= Transform size flag for INTER MBs =============
    //-------------------------------------------------------------
    need_transform_size_flag = (((currMB->mb_type >= 1 && currMB->mb_type <= 3)||
      (IS_DIRECT(currMB) && p_Vid->active_sps->direct_8x8_inference_flag) ||
      (currMB->NoMbPartLessThan8x8Flag))
      && currMB->mb_type != I8MB && currMB->mb_type != I4MB
      && (currMB->cbp&15)
      && currSlice->Transform8x8Mode);

    if (need_transform_size_flag)
    {
      currSE.type   =  SE_HEADER;
      dP = &(currSlice->partArr[partMap[SE_HEADER]]);
      currSE.reading = readMB_transform_size_flag_CABAC;
      TRACE_STRING("transform_size_8x8_flag");

      // read CAVLC transform_size_8x8_flag
      currSE.len = 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);

      currMB->luma_transform_size_8x8_flag = (Boolean) currSE.value1;
    }

    //=====   DQUANT   =====
    //----------------------
    // Delta quant only if nonzero coeffs
    if (cbp !=0)
    {
      read_delta_quant(&currSE, dP, currMB, partMap, ((currMB->is_intra_block == FALSE)) ? SE_DELTA_QUANT_INTER : SE_DELTA_QUANT_INTRA);

      if (currSlice->dp_mode)
      {
        if ((currMB->is_intra_block == FALSE) && currSlice->dpC_NotPresent ) 
          currMB->dpl_flag = 1;

        if( intra && currSlice->dpB_NotPresent )
        {
          currMB->ei_flag = 1;
          currMB->dpl_flag = 1;
        }

        // check for prediction from neighbours
        check_dp_neighbors (currMB);
        if (currMB->dpl_flag)
        {
          cbp = 0; 
          currMB->cbp = cbp;
        }
      }
    }
  }
  else
  {
    cbp = currMB->cbp;  
    read_delta_quant(&currSE, dP, currMB, partMap, SE_DELTA_QUANT_INTRA);

    if (currSlice->dp_mode)
    {  
      if (currSlice->dpB_NotPresent)
      {
        currMB->ei_flag  = 1;
        currMB->dpl_flag = 1;
      }
      check_dp_neighbors (currMB);
      if (currMB->dpl_flag)
      {
        currMB->cbp = cbp = 0; 
      }
    }

    if (!currMB->dpl_flag)
    {
      pos_scan_4x4 = pos_scan4x4[0];

      readCoeff4x4_CAVLC(currMB, LUMA_INTRA16x16DC, 0, 0, levarr, runarr, &numcoeff);

      for(k = 0; k < numcoeff; ++k)
      {
        if (levarr[k] != 0)                     // leave if level == 0
        {
          pos_scan_4x4 += 2 * runarr[k];

          i0 = ((*pos_scan_4x4++) << 2);
          j0 = ((*pos_scan_4x4++) << 2);

          currSlice->cof[0][j0][i0] = levarr[k];// add new intra DC coeff
          //p_Vid->fcf[0][j0][i0] = levarr[k];// add new intra DC coeff
        }
      }


      if(currMB->is_lossless == FALSE)
        itrans_2(currMB, (ColorPlane) currSlice->colour_plane_id);// transform new intra DC
    }
  }

  update_qp(currMB, currSlice->qp);

  qp_per = p_Vid->qp_per_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];
  qp_rem = p_Vid->qp_rem_matrix[ currMB->qp_scaled[currSlice->colour_plane_id] ];

  //init quant parameters for chroma 
  if (dec_picture->chroma_format_idc != YUV400)
  {
    for(i=0; i < 2; ++i)
    {
      qp_per_uv[i] = p_Vid->qp_per_matrix[ currMB->qp_scaled[i + 1] ];
      qp_rem_uv[i] = p_Vid->qp_rem_matrix[ currMB->qp_scaled[i + 1] ];
    }
  }

  InvLevelScale4x4 = intra? currSlice->InvLevelScale4x4_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale4x4_Inter[currSlice->colour_plane_id][qp_rem];
  InvLevelScale8x8 = intra? currSlice->InvLevelScale8x8_Intra[currSlice->colour_plane_id][qp_rem] : currSlice->InvLevelScale8x8_Inter[currSlice->colour_plane_id][qp_rem];

  // luma coefficients
  if (cbp)
  {
    if (!currMB->luma_transform_size_8x8_flag) // 4x4 transform
    {
      readCompCoeff4x4MB_CAVLC (currMB, PLANE_Y, InvLevelScale4x4, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
    else // 8x8 transform
    {
      readCompCoeff8x8MB_CAVLC (currMB, PLANE_Y, InvLevelScale8x8, qp_per, cbp, p_Vid->nz_coeff[mb_nr][PLANE_Y]);
    }
  }
  else
  {
    fast_memset(p_Vid->nz_coeff[mb_nr][0][0], 0, BLOCK_PIXELS * sizeof(byte));
  }

  if (dec_picture->chroma_format_idc != YUV444)
  {
    //========================== CHROMA DC ============================
    //-----------------------------------------------------------------
    // chroma DC coeff
    if(cbp>15)
    {
      if (dec_picture->chroma_format_idc == YUV420)
      {
        for (ll=0;ll<3;ll+=2)
        {
          uv = ll>>1;          

          InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];
          //===================== CHROMA DC YUV420 ======================
          memset(currSlice->cofu, 0, 4 *sizeof(int));
          coef_ctr=-1;

          readCoeff4x4_CAVLC(currMB, CHROMA_DC, 0, 0, levarr, runarr, &numcoeff);

          for(k = 0; k < numcoeff; ++k)
          {
            if (levarr[k] != 0)
            {
              currMB->cbp_blk[0] |= 0xf0000 << (ll<<1) ;
              coef_ctr += runarr[k] + 1;
              currSlice->cofu[coef_ctr]=levarr[k];
            }
          }


          if (smb || (currMB->is_lossless == TRUE)) // check to see if MB type is SPred or SIntra4x4
          {
            currSlice->cof[uv + 1][0][0] = currSlice->cofu[0];
            currSlice->cof[uv + 1][0][4] = currSlice->cofu[1];
            currSlice->cof[uv + 1][4][0] = currSlice->cofu[2];
            currSlice->cof[uv + 1][4][4] = currSlice->cofu[3];
            //p_Vid->fcf[uv + 1][0][0] = currSlice->cofu[0];
            //p_Vid->fcf[uv + 1][4][0] = currSlice->cofu[1];
            //p_Vid->fcf[uv + 1][0][4] = currSlice->cofu[2];
            //p_Vid->fcf[uv + 1][4][4] = currSlice->cofu[3];
          }
          else
          {
            ihadamard2x2(currSlice->cofu, temp);
            //p_Vid->fcf[uv + 1][0][0] = temp[0];
            //p_Vid->fcf[uv + 1][0][4] = temp[1];
            //p_Vid->fcf[uv + 1][4][0] = temp[2];
            //p_Vid->fcf[uv + 1][4][4] = temp[3];

            currSlice->cof[uv + 1][0][0] = (((temp[0] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
            currSlice->cof[uv + 1][0][4] = (((temp[1] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
            currSlice->cof[uv + 1][4][0] = (((temp[2] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
            currSlice->cof[uv + 1][4][4] = (((temp[3] * InvLevelScale4x4[0][0])<<qp_per_uv[uv])>>5);
          }          
        }
      }
    }

    //========================== CHROMA AC ============================
    //-----------------------------------------------------------------
    // chroma AC coeff, all zero fram start_scan
    if (cbp<=31)
    {
      fast_memset(p_Vid->nz_coeff [mb_nr ][1][0], 0, 2 * BLOCK_PIXELS * sizeof(byte));
    }
    else
    {
      if(currMB->is_lossless == FALSE)
      {
        for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
        {
          currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));
          InvLevelScale4x4 = intra ? currSlice->InvLevelScale4x4_Intra[uv + 1][qp_rem_uv[uv]] : currSlice->InvLevelScale4x4_Inter[uv + 1][qp_rem_uv[uv]];

          for (b4=0; b4 < 4; ++b4)
          {
            i = cofuv_blk_x[yuv][b8][b4];
            j = cofuv_blk_y[yuv][b8][b4];

            readCoeff4x4_CAVLC(currMB, CHROMA_AC, i + 2*uv, j + 4, levarr, runarr, &numcoeff);
            coef_ctr = 0;

            for(k = 0; k < numcoeff;++k)
            {
              if (levarr[k] != 0)
              {
                currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
                coef_ctr += runarr[k] + 1;

                i0=pos_scan4x4[coef_ctr][0];
                j0=pos_scan4x4[coef_ctr][1];

                currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = rshift_rnd_sf((levarr[k] * InvLevelScale4x4[j0][i0])<<qp_per_uv[uv], 4);
                //p_Vid->fcf[uv + 1][(j<<2) + j0][(i<<2) + i0] = levarr[k];
              }
            }
          }
        }        
      }
      else
      {
        for (b8=0; b8 < p_Vid->num_blk8x8_uv; ++b8)
        {
          currMB->is_v_block = uv = (b8 > ((p_Vid->num_uv_blocks) - 1 ));

          for (b4=0; b4 < 4; ++b4)
          {
            i = cofuv_blk_x[yuv][b8][b4];
            j = cofuv_blk_y[yuv][b8][b4];

            readCoeff4x4_CAVLC(currMB, CHROMA_AC, i + 2*uv, j + 4, levarr, runarr, &numcoeff);
            coef_ctr = 0;

            for(k = 0; k < numcoeff;++k)
            {
              if (levarr[k] != 0)
              {
                currMB->cbp_blk[0] |= i64_power2(cbp_blk_chroma[b8][b4]);
                coef_ctr += runarr[k] + 1;

                i0=pos_scan4x4[coef_ctr][0];
                j0=pos_scan4x4[coef_ctr][1];

                currSlice->cof[uv + 1][(j<<2) + j0][(i<<2) + i0] = levarr[k];
              }
            }
          }
        }        
      } 
    } //if (dec_picture->chroma_format_idc != YUV400)
  }
}


/*!
 ************************************************************************
 * \brief
 *    decode one color component in an I slice
 ************************************************************************
 */

static int decode_one_component_i_slice(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture)
{
  //For residual DPCM
  currMB->ipmode_DPCM = NO_INTRA_PMODE; 
  if(currMB->mb_type == IPCM)
    mb_pred_ipcm(currMB);
  else if (currMB->mb_type==I16MB)
    mb_pred_intra16x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == I4MB)
    mb_pred_intra4x4(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == I8MB) 
    mb_pred_intra8x8(currMB, curr_plane, currImg, dec_picture);

  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    decode one color component for a p slice
 ************************************************************************
 */
static int decode_one_component_p_slice(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture)
{
  //For residual DPCM
  currMB->ipmode_DPCM = NO_INTRA_PMODE; 
  if(currMB->mb_type == IPCM)
    mb_pred_ipcm(currMB);
  else if (currMB->mb_type==I16MB)
    mb_pred_intra16x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == I4MB)
    mb_pred_intra4x4(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == I8MB) 
    mb_pred_intra8x8(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == PSKIP)
    mb_pred_skip(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == P16x16)
    mb_pred_p_inter16x16(currMB, curr_plane, dec_picture);  
  else if (currMB->mb_type == P16x8)
    mb_pred_p_inter16x8(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == P8x16)
    mb_pred_p_inter8x16(currMB, curr_plane, dec_picture);
  else
    mb_pred_p_inter8x8(currMB, curr_plane, dec_picture);

  return 1;
}


/*!
 ************************************************************************
 * \brief
 *    decode one color component for a sp slice
 ************************************************************************
 */
static int decode_one_component_sp_slice(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture)
{   
  //For residual DPCM
  currMB->ipmode_DPCM = NO_INTRA_PMODE; 

  if (currMB->mb_type == IPCM)
    mb_pred_ipcm(currMB);
  else if (currMB->mb_type==I16MB)
    mb_pred_intra16x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == I4MB)
    mb_pred_intra4x4(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == I8MB) 
    mb_pred_intra8x8(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == PSKIP)
    mb_pred_sp_skip(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == P16x16)
    mb_pred_p_inter16x16(currMB, curr_plane, dec_picture);  
  else if (currMB->mb_type == P16x8)
    mb_pred_p_inter16x8(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == P8x16)
    mb_pred_p_inter8x16(currMB, curr_plane, dec_picture);
  else
    mb_pred_p_inter8x8(currMB, curr_plane, dec_picture);

  return 1;
}



/*!
 ************************************************************************
 * \brief
 *    decode one color component for a b slice
 ************************************************************************
 */

static int decode_one_component_b_slice(Macroblock *currMB, ColorPlane curr_plane, imgpel **currImg, StorablePicture *dec_picture)
{  
  //For residual DPCM
  currMB->ipmode_DPCM = NO_INTRA_PMODE; 

  if(currMB->mb_type == IPCM)
    mb_pred_ipcm(currMB);
  else if (currMB->mb_type==I16MB)
    mb_pred_intra16x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == I4MB)
    mb_pred_intra4x4(currMB, curr_plane, currImg, dec_picture);
  else if (currMB->mb_type == I8MB) 
    mb_pred_intra8x8(currMB, curr_plane, currImg, dec_picture);  
  else if (currMB->mb_type == P16x16)
    mb_pred_p_inter16x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == P16x8)
    mb_pred_p_inter16x8(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == P8x16)
    mb_pred_p_inter8x16(currMB, curr_plane, dec_picture);
  else if (currMB->mb_type == BSKIP_DIRECT)
  {
    Slice *currSlice = currMB->p_Slice;
    if (currSlice->direct_spatial_mv_pred_flag == 0)
    {
      if (currSlice->active_sps->direct_8x8_inference_flag)
        mb_pred_b_d8x8temporal (currMB, curr_plane, currImg, dec_picture);
      else
        mb_pred_b_d4x4temporal (currMB, curr_plane, currImg, dec_picture);
    }
    else
    {
      if (currSlice->active_sps->direct_8x8_inference_flag)
        mb_pred_b_d8x8spatial (currMB, curr_plane, currImg, dec_picture);
      else
        mb_pred_b_d4x4spatial (currMB, curr_plane, currImg, dec_picture);
    }
  }
  else
    mb_pred_b_inter8x8 (currMB, curr_plane, dec_picture);

 return 1;
}

// probably a better way (or place) to do this, but I'm not sure what (where) it is [CJV]
// this is intended to make get_block_luma faster, but I'm still performing
// this at the MB level, and it really should be done at the slice level
static void init_cur_imgy(VideoParameters *p_Vid,Slice *currSlice,int pl)
{
  int i,j;
  if ((p_Vid->separate_colour_plane_flag == 0)) {
    StorablePicture *vidref = p_Vid->no_reference_picture;
    int noref = (currSlice->framepoc < p_Vid->recovery_poc);    
    if (pl==PLANE_Y) {
      for (j = 0; j < 6; j++) {     // for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) { 
        for (i = 0; i < currSlice->listXsize[j] ; i++) 
        {
          StorablePicture *curr_ref = currSlice->listX[j][i];
          if (curr_ref) 
          {
            curr_ref->no_ref = noref && (curr_ref == vidref);
            curr_ref->cur_imgY = curr_ref->imgY;
          }
        }
      }
    }
    else {
      for (j = 0; j < 6; j++) { //for (j = 0; j < (currSlice->slice_type==B_SLICE?2:1); j++) {
        for (i = 0; i < currSlice->listXsize[j]; i++) {
          StorablePicture *curr_ref = currSlice->listX[j][i];
          if (curr_ref) {
            curr_ref->no_ref = noref && (curr_ref == vidref);
            curr_ref->cur_imgY = curr_ref->imgUV[pl-1]; 
          }
        }
      }
    }
  }
}


/*!
 ************************************************************************
 * \brief
 *    decode one macroblock
 ************************************************************************
 */

int decode_one_macroblock(Macroblock *currMB, StorablePicture *dec_picture)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;  


  // macroblock decoding **************************************************
  if (currSlice->is_not_independent)  
  {
    if (!currMB->is_intra_block)
    {
      init_cur_imgy(p_Vid,currSlice,PLANE_Y);
      currSlice->decode_one_component(currMB, PLANE_Y, dec_picture->imgY, dec_picture);
      init_cur_imgy(p_Vid,currSlice,PLANE_U);
      currSlice->decode_one_component(currMB, PLANE_U, dec_picture->imgUV[0], dec_picture);
      init_cur_imgy(p_Vid,currSlice,PLANE_V);
      currSlice->decode_one_component(currMB, PLANE_V, dec_picture->imgUV[1], dec_picture);
    }
    else
    {
      currSlice->decode_one_component(currMB, PLANE_Y, dec_picture->imgY, dec_picture);
      currSlice->decode_one_component(currMB, PLANE_U, dec_picture->imgUV[0], dec_picture);
      currSlice->decode_one_component(currMB, PLANE_V, dec_picture->imgUV[1], dec_picture);      
    }
    currSlice->is_reset_coeff = FALSE;
  }
  else
  {
    currSlice->decode_one_component(currMB, PLANE_Y, dec_picture->imgY, dec_picture);
  }

  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    change target plane
 *    for 4:4:4 Independent mode
 ************************************************************************
 */
void change_plane_JV( VideoParameters *p_Vid, int nplane, Slice *pSlice)
{
  //Slice *currSlice = p_Vid->currentSlice;
  //p_Vid->colour_plane_id = nplane;
  p_Vid->mb_data = p_Vid->mb_data_JV[nplane];
  p_Vid->dec_picture  = p_Vid->dec_picture_JV[nplane];
  p_Vid->siblock = p_Vid->siblock_JV[nplane];
  p_Vid->ipredmode = p_Vid->ipredmode_JV[nplane];
  p_Vid->intra_block = p_Vid->intra_block_JV[nplane];
  if(pSlice)
  {
    pSlice->mb_data = p_Vid->mb_data_JV[nplane];
    pSlice->dec_picture  = p_Vid->dec_picture_JV[nplane];
    pSlice->siblock = p_Vid->siblock_JV[nplane];
    pSlice->ipredmode = p_Vid->ipredmode_JV[nplane];
    pSlice->intra_block = p_Vid->intra_block_JV[nplane];
  }
}

/*!
 ************************************************************************
 * \brief
 *    make frame picture from each plane data
 *    for 4:4:4 Independent mode
 ************************************************************************
 */
void make_frame_picture_JV(VideoParameters *p_Vid)
{
  int uv, line;
  int nsize;
  p_Vid->dec_picture = p_Vid->dec_picture_JV[0];
  //copy;
  if(p_Vid->dec_picture->used_for_reference && p_Vid->dec_picture->slice_type != I_SLICE && p_Vid->dec_picture->slice_type!=SI_SLICE) 
  {
    nsize = (p_Vid->dec_picture->size_y/BLOCK_SIZE)*(p_Vid->dec_picture->size_x/BLOCK_SIZE)*sizeof(PicMotionParams);
    memcpy( &(p_Vid->dec_picture->JVmv_info[PLANE_Y][0][0]), &(p_Vid->dec_picture_JV[PLANE_Y]->mv_info[0][0]), nsize);
    memcpy( &(p_Vid->dec_picture->JVmv_info[PLANE_U][0][0]), &(p_Vid->dec_picture_JV[PLANE_U]->mv_info[0][0]), nsize);
    memcpy( &(p_Vid->dec_picture->JVmv_info[PLANE_V][0][0]), &(p_Vid->dec_picture_JV[PLANE_V]->mv_info[0][0]), nsize);
  }

  // This could be done with pointers and seems not necessary
  for( uv=0; uv<2; uv++ )
  {
    for( line=0; line<p_Vid->height; line++ )
    {
      nsize = sizeof(imgpel) * p_Vid->width;
      memcpy( p_Vid->dec_picture->imgUV[uv][line], p_Vid->dec_picture_JV[uv+1]->imgY[line], nsize );
    }
    free_storable_picture(p_Vid->dec_picture_JV[uv+1]);
  }
}


