
/*!
 *************************************************************************************
 * \file loop_filter_normal.c
 *
 * \brief
 *    Filter to reduce blocking artifacts on a macroblock level.
 *    The filter strength is QP dependent.
 *
 * \author
 *    Contributors:
 *    - Peter List       Peter.List@t-systems.de:  Original code                                 (13-Aug-2001)
 *    - Jani Lainema     Jani.Lainema@nokia.com:   Some bug fixing, removal of recursiveness     (16-Aug-2001)
 *    - Peter List       Peter.List@t-systems.de:  inplace filtering and various simplifications (10-Jan-2002)
 *    - Anthony Joch     anthony@ubvideo.com:      Simplified switching between filters and
 *                                                 non-recursive default filter.                 (08-Jul-2002)
 *    - Cristina Gomila  cristina.gomila@thomson.net: Simplification of the chroma deblocking
 *                                                    from JVT-E089                              (21-Nov-2002)
 *    - Alexis Michael Tourapis atour@dolby.com:   Speed/Architecture improvements               (08-Feb-2007)
 *************************************************************************************
 */

#include "global.h"
#include "image.h"
#include "mb_access.h"
#include "loopfilter.h"
#include "loop_filter.h"

static void GetStrengthVer         (byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
static void GetStrengthHor         (byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p);
static void EdgeLoopLumaVer        (ColorPlane pl, imgpel** Img, byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p);
static void EdgeLoopLumaHor        (ColorPlane pl, imgpel** Img, byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, StorablePicture *p);
static void EdgeLoopChromaVer      (imgpel** Img, byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int uv, StorablePicture *p);
static void EdgeLoopChromaHor      (imgpel** Img, byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int uv, StorablePicture *p);


void set_loop_filter_functions_normal(VideoParameters *p_Vid)
{
  p_Vid->GetStrengthVer    = GetStrengthVer;
  p_Vid->GetStrengthHor    = GetStrengthHor;
  p_Vid->EdgeLoopLumaVer   = EdgeLoopLumaVer;
  p_Vid->EdgeLoopLumaHor   = EdgeLoopLumaHor;
  p_Vid->EdgeLoopChromaVer = EdgeLoopChromaVer;
  p_Vid->EdgeLoopChromaHor = EdgeLoopChromaHor;
}


static Macroblock* get_non_aff_neighbor_luma(Macroblock *mb, int xN, int yN)
{
  if (xN < 0)
    return(mb->mbleft);
  else if (yN < 0)
    return(mb->mbup);
  else
    return(mb);
}

static Macroblock* get_non_aff_neighbor_chroma(Macroblock *mb, int xN, int yN, int block_width,int block_height)
{
  if (xN < 0) 
  {
    if (yN < block_height)
      return(mb->mbleft);
    else
      return(NULL);
  }
  else if (xN < block_width) 
  {
    if (yN < 0)
      return(mb->mbup);
    else if (yN < block_height)
      return(mb);
    else
      return(NULL);
  }
  else
    return(NULL);
}

#define get_x_luma(x) (x & 15)
#define get_y_luma(y) (y & 15)
#define get_pos_x_luma(mb,x) (mb->pix_x + (x & 15))
#define get_pos_y_luma(mb,y) (mb->pix_y + (y & 15))
#define get_pos_x_chroma(mb,x,max) (mb->pix_c_x + (x & max))
#define get_pos_y_chroma(mb,y,max) (mb->pix_c_y + (y & max))

  /*!
 *********************************************************************************************
 * \brief
 *    returns a buffer of 16 Strength values for one stripe in a mb (for different Frame or Field types)
 *********************************************************************************************
 */
static void GetStrengthVer(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{
  Slice *currSlice = MbQ->p_Slice;
  int     StrValue;

  if ((currSlice->slice_type==SP_SLICE)||(currSlice->slice_type==SI_SLICE) )
  { 
    // Set strength to either 3 or 4 regardless of pixel position
    StrValue = (edge == 0 && (((p->structure==FRAME)) || ((p->structure != FRAME)))) ? 4 : 3;    
    memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
  }
  else
  {    
    if (MbQ->is_intra_block == FALSE)
    {
      Macroblock *MbP;
      int xQ = edge - 1;
      Macroblock *neighbor = get_non_aff_neighbor_luma(MbQ, xQ, 0);
      MbP = (edge) ? MbQ : neighbor;

      if (edge || MbP->is_intra_block == FALSE)
      {
        int      blkP, blkQ, idx;
        BlockPos mb = PicPos[ MbQ->mbAddrX ];
        mb.x <<= 2;
        mb.y <<= 2;
        for( idx = 0 ; idx < MB_BLOCK_SIZE ; idx += BLOCK_SIZE )
        {
          blkQ = (idx & 0xFFFC) + (edge >> 2);
          blkP = (idx & 0xFFFC) + (get_x_luma(xQ) >> 2);
          if (((MbQ->cbp_blk[0] & i64_power2(blkQ)) != 0) || ((MbP->cbp_blk[0] & i64_power2(blkP)) != 0))
            StrValue = 2;
          else if (edge && ((MbQ->mb_type == 1)  || (MbQ->mb_type == 2)))
            StrValue = 0; // if internal edge of certain types, we already know StrValue should be 0
          else // for everything else, if no coefs, but vector difference >= 1 set Strength=1
          {
            int blk_y  = mb.y + (blkQ >> 2);
            int blk_x  = mb.x + (blkQ  & 3);
            int blk_y2 = (short)(get_pos_y_luma(neighbor,  0) + idx) >> 2;
            int blk_x2 = (short)(get_pos_x_luma(neighbor, xQ)      ) >> 2;
            PicMotionParams *mv_info_p = &p->mv_info[blk_y ][blk_x ];            
            StorablePicturePtr ref_p0 = mv_info_p->ref_pic[LIST_0];
            StorablePicturePtr ref_p1 = mv_info_p->ref_pic[LIST_1];
            PicMotionParams *mv_info_q = &p->mv_info[blk_y2][blk_x2];
            StorablePicturePtr ref_q0 = mv_info_q->ref_pic[LIST_0];
            StorablePicturePtr ref_q1 = mv_info_q->ref_pic[LIST_1];

            if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) || ((ref_p0==ref_q1) && (ref_p1==ref_q0)))
            {
              // L0 and L1 reference pictures of p0 are different; q0 as well
              if (ref_p0 != ref_p1)
              {
                // compare MV for the same reference picture
                if (ref_p0 == ref_q0)
                {
                  StrValue = 
                    compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_0], mvlimit) |
                    compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_1], mvlimit);
                }
                else
                {
                  StrValue = 
                    compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_1], mvlimit) |
                    compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_0], mvlimit);
                }
              }
              else
              { // L0 and L1 reference pictures of p0 are the same; q0 as well
                StrValue = ((
                  compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_0], mvlimit) |
                  compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_1], mvlimit))
                  && (
                  compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_1], mvlimit) |
                  compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_0], mvlimit)
                  ));
              }
            }
            else
              StrValue = 1;
          }

          //*(int*)(Strength+idx) = (StrValue<<24)|(StrValue<<16)|(StrValue<<8)|StrValue;
          *(int*)(Strength+idx) = StrValue * 0x01010101;
        }
      }
      else
      {
        // Start with Strength=3. or Strength=4 for Mb-edge
        StrValue = (edge == 0 && ((((p->structure==FRAME))) || ((p->structure != FRAME)))) ? 4 : 3;
        memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
      }      
    }
    else
    {
      // Start with Strength=3. or Strength=4 for Mb-edge
      StrValue = (edge == 0 && ((((p->structure==FRAME))) || ((p->structure != FRAME)))) ? 4 : 3;
      memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
    }      
  }
}

  /*!
 *********************************************************************************************
 * \brief
 *    returns a buffer of 16 Strength values for one stripe in a mb (for different Frame or Field types)
 *********************************************************************************************
 */
static void GetStrengthHor(byte Strength[MB_BLOCK_SIZE], Macroblock *MbQ, int edge, int mvlimit, StorablePicture *p)
{  
  int     StrValue;
  Slice *currSlice = MbQ->p_Slice;

  if ((currSlice->slice_type==SP_SLICE)||(currSlice->slice_type==SI_SLICE) )
  { 
    // Set strength to either 3 or 4 regardless of pixel position
    StrValue = (edge == 0 && (((p->structure==FRAME)))) ? 4 : 3;
    memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
  }
  else
  {    
    if (MbQ->is_intra_block == FALSE)
    {
      Macroblock *MbP;
      int yQ = (edge < 16 ? edge - 1: 0);

      Macroblock *neighbor = get_non_aff_neighbor_luma(MbQ, 0, yQ);

      MbP = (edge) ? MbQ : neighbor;

      if (edge || MbP->is_intra_block == FALSE)
      {
        int      blkP, blkQ, idx;
        BlockPos mb = PicPos[ MbQ->mbAddrX ];
        mb.x <<= 2;
        mb.y <<= 2;
        for( idx = 0 ; idx < MB_BLOCK_SIZE ; idx += BLOCK_SIZE )
        {
          blkQ = ((yQ + 1) & 0xFFFC) + (idx >> 2);
          blkP = (get_y_luma(yQ) & 0xFFFC) + (idx >> 2);

          if (((MbQ->cbp_blk[0] & i64_power2(blkQ)) != 0) || ((MbP->cbp_blk[0] & i64_power2(blkP)) != 0))
            StrValue = 2;
          else if (edge && ((MbQ->mb_type == 1)  || (MbQ->mb_type == 3)))
            StrValue = 0; // if internal edge of certain types, we already know StrValue should be 0
          else // for everything else, if no coefs, but vector difference >= 1 set Strength=1
          {            
            int blk_y  = mb.y + (blkQ >> 2);
            int blk_x  = mb.x + (blkQ  & 3);
            int blk_y2 = get_pos_y_luma(neighbor,yQ) >> 2;
            int blk_x2 = (short)(get_pos_x_luma(neighbor,0) + idx) >> 2;

            PicMotionParams *mv_info_p = &p->mv_info[blk_y ][blk_x ];
            PicMotionParams *mv_info_q = &p->mv_info[blk_y2][blk_x2];

            StorablePicturePtr ref_p0 = mv_info_p->ref_pic[LIST_0];
            StorablePicturePtr ref_q0 = mv_info_q->ref_pic[LIST_0];
            StorablePicturePtr ref_p1 = mv_info_p->ref_pic[LIST_1];
            StorablePicturePtr ref_q1 = mv_info_q->ref_pic[LIST_1];            

            if ( ((ref_p0==ref_q0) && (ref_p1==ref_q1)) || ((ref_p0==ref_q1) && (ref_p1==ref_q0)))
            {
              // L0 and L1 reference pictures of p0 are different; q0 as well
              if (ref_p0 != ref_p1)
              {
                // compare MV for the same reference picture
                if (ref_p0 == ref_q0)
                {
                  StrValue = 
                    compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_0], mvlimit) |
                    compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_1], mvlimit);
                }
                else
                {
                  StrValue = 
                    compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_1], mvlimit) |
                    compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_0], mvlimit);
                }
              }
              else
              { // L0 and L1 reference pictures of p0 are the same; q0 as well
                StrValue = ((
                  compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_0], mvlimit) |
                  compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_1], mvlimit))
                  && (
                  compare_mvs(&mv_info_p->mv[LIST_0], &mv_info_q->mv[LIST_1], mvlimit) |
                  compare_mvs(&mv_info_p->mv[LIST_1], &mv_info_q->mv[LIST_0], mvlimit)
                  ));
              }
            }
            else
              StrValue = 1;
          }
          //*(int*)(Strength+idx) = (StrValue<<24)|(StrValue<<16)|(StrValue<<8)|StrValue;
          *(int*)(Strength+idx) = StrValue * 0x01010101;
        }
      }
      else
      {
        // Start with Strength=3. or Strength=4 for Mb-edge
        StrValue = (edge == 0 && (p->structure == FRAME)) ? 4 : 3;
        memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
      }      
    }
    else
    {
      // Start with Strength=3. or Strength=4 for Mb-edge
      StrValue = (edge == 0 && (p->structure == FRAME)) ? 4 : 3;
      memset(Strength, (byte) StrValue, MB_BLOCK_SIZE * sizeof(byte));
    }      
  }
}


/*!
 *****************************************************************************************
 * \brief
 *    Filters 16 pel block edge of Frame or Field coded MBs 
 *****************************************************************************************
 */
static void EdgeLoopLumaVer(ColorPlane pl, imgpel** Img, byte Strength[16], Macroblock *MbQ, int edge, StorablePicture *p)
{
  VideoParameters *p_Vid = MbQ->p_Vid;

  Macroblock *MbP = get_non_aff_neighbor_luma(MbQ, edge - 1, 0);

  if (MbP || (MbQ->DFDisableIdc== 0))
  {
    int bitdepth_scale   = pl ? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];

    // Average QP of the two blocks
    int QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

    int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
    int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);

    int Alpha  = ALPHA_TABLE[indexA] * bitdepth_scale;
    int Beta   = BETA_TABLE [indexB] * bitdepth_scale;

    if ((Alpha | Beta )!= 0)
    {
      const byte *ClipTab = CLIP_TAB[indexA];
      int max_imgpel_value = p_Vid->max_pel_value_comp[pl];      

      int pos_x1 = get_pos_x_luma(MbP, (edge - 1));
      imgpel **cur_img = &Img[get_pos_y_luma(MbP, 0)];
      int pel;

      for( pel = 0 ; pel < MB_BLOCK_SIZE ; pel += 4 )
      {
        if(*Strength == 4 )    // INTRA strong filtering
        {
          int i;
          for( i = 0 ; i < BLOCK_SIZE ; ++i )
          {              
            imgpel *SrcPtrP = *(cur_img++) + pos_x1;
            imgpel *SrcPtrQ = SrcPtrP + 1;
            imgpel  L0 = *SrcPtrP;           
            imgpel  R0 = *SrcPtrQ;

            if( iabs( R0 - L0 ) < Alpha )
            {
              imgpel  R1 = *(SrcPtrQ + 1);
              imgpel  L1 = *(SrcPtrP - 1);
              if ((iabs( R0 - R1) < Beta)  && (iabs(L0 - L1) < Beta))
              {
                if ((iabs( R0 - L0 ) < ((Alpha >> 2) + 2)))
                {
                  imgpel  R2 = *(SrcPtrQ + 2);
                  imgpel  L2 = *(SrcPtrP - 2);                  
                  int RL0 = L0 + R0;

                  if (( iabs( L0 - L2) < Beta ))
                  {
                    imgpel  L3 = *(SrcPtrP - 3);
                    *(SrcPtrP--) = (imgpel)  (( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3);
                    *(SrcPtrP--) = (imgpel)  (( L2 + L1 + RL0 + 2) >> 2);
                    *(SrcPtrP  ) = (imgpel) ((((L3 + L2) <<1) + L2 + L1 + RL0 + 4) >> 3);                
                  }
                  else
                  {
                    *SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2);
                  }

                  if (( iabs( R0 - R2) < Beta ))
                  {
                    imgpel  R3 = *(SrcPtrQ + 3);
                    *(SrcPtrQ++) = (imgpel) (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
                    *(SrcPtrQ++) = (imgpel) (( R2 + R0 + L0 + R1 + 2) >> 2);
                    *(SrcPtrQ  ) = (imgpel) ((((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3);
                  }
                  else
                  {
                    *SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
                  }
                }
                else
                {
                  *SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2);
                  *SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
                }
              }
            }             
          }
        }
        else if( *Strength != 0) // normal filtering
        {
          int C0  = ClipTab[ *Strength ] * bitdepth_scale;
          int i;
          imgpel *SrcPtrP, *SrcPtrQ;
          int edge_diff;
          if (C0 == 0)
          {
            for( i= 0 ; i < BLOCK_SIZE ; ++i )
            {             
              SrcPtrP = *(cur_img++) + pos_x1;
              SrcPtrQ = SrcPtrP + 1;
              edge_diff = *SrcPtrQ - *SrcPtrP;

              if( iabs( edge_diff ) < Alpha )
              {          
                imgpel  *SrcPtrQ1 = SrcPtrQ + 1;
                imgpel  *SrcPtrP1 = SrcPtrP - 1;

                if ((iabs( *SrcPtrQ - *SrcPtrQ1) < Beta)  && (iabs(*SrcPtrP - *SrcPtrP1) < Beta))
                {                          
                  imgpel  R2 = *(SrcPtrQ1 + 1);
                  imgpel  L2 = *(SrcPtrP1 - 1);

                  int aq  = (iabs(*SrcPtrQ - R2) < Beta);
                  int ap  = (iabs(*SrcPtrP - L2) < Beta);

                  int tc0  = (ap + aq) ;
                  int dif = iClip3( -tc0, tc0, (((edge_diff) << 2) + (*SrcPtrP1 - *SrcPtrQ1) + 4) >> 3 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1(max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1(max_imgpel_value, *SrcPtrQ - dif);
                  }
                }
              }
            }
          }
          else
          {
            for( i= 0 ; i < BLOCK_SIZE ; ++i )
            {             
              SrcPtrP = *(cur_img++) + pos_x1;
              SrcPtrQ = SrcPtrP + 1;
              edge_diff = *SrcPtrQ - *SrcPtrP;

              if( iabs( edge_diff ) < Alpha )
              {          
                imgpel  *SrcPtrQ1 = SrcPtrQ + 1;
                imgpel  *SrcPtrP1 = SrcPtrP - 1;

                if ((iabs( *SrcPtrQ - *SrcPtrQ1) < Beta)  && (iabs(*SrcPtrP - *SrcPtrP1) < Beta))
                {                          
                  int RL0 = (*SrcPtrP + *SrcPtrQ + 1) >> 1;
                  imgpel  R2 = *(SrcPtrQ1 + 1);
                  imgpel  L2 = *(SrcPtrP1 - 1);

                  int aq  = (iabs(*SrcPtrQ - R2) < Beta);
                  int ap  = (iabs(*SrcPtrP - L2) < Beta);

                  int tc0  = (C0 + ap + aq) ;
                  int dif = iClip3( -tc0, tc0, (((edge_diff) << 2) + (*SrcPtrP1 - *SrcPtrQ1) + 4) >> 3 );

                  if( ap )
                    *SrcPtrP1 += iClip3( -C0,  C0, (L2 + RL0 - (*SrcPtrP1<<1)) >> 1 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1(max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1(max_imgpel_value, *SrcPtrQ - dif);
                  }

                  if( aq )
                    *SrcPtrQ1 += iClip3( -C0,  C0, (R2 + RL0 - (*SrcPtrQ1<<1)) >> 1 );          
                }
              }
            }
          }
        }
        else
        {
          cur_img += 4;
        }
        Strength += 4;
      }     
    }
  }
}


/*!
 *****************************************************************************************
 * \brief
 *    Filters 16 pel block edge of Frame or Field coded MBs 
 *****************************************************************************************
 */
static void EdgeLoopLumaHor(ColorPlane pl, imgpel** Img, byte Strength[16], Macroblock *MbQ, 
              int edge, StorablePicture *p)
{
  VideoParameters *p_Vid = MbQ->p_Vid;

  int ypos = (edge < MB_BLOCK_SIZE ? edge - 1: 0);
  Macroblock *MbP = get_non_aff_neighbor_luma(MbQ, 0, ypos); 

  if (MbP || (MbQ->DFDisableIdc== 0))
  {
    int bitdepth_scale   = pl ? p_Vid->bitdepth_scale[IS_CHROMA] : p_Vid->bitdepth_scale[IS_LUMA];

    // Average QP of the two blocks
    int QP = pl? ((MbP->qpc[pl-1] + MbQ->qpc[pl-1] + 1) >> 1) : (MbP->qp + MbQ->qp + 1) >> 1;

    int indexA = iClip3(0, MAX_QP, QP + MbQ->DFAlphaC0Offset);
    int indexB = iClip3(0, MAX_QP, QP + MbQ->DFBetaOffset);

    int Alpha  = ALPHA_TABLE[indexA] * bitdepth_scale;
    int Beta   = BETA_TABLE [indexB] * bitdepth_scale;

    if ((Alpha | Beta )!= 0)
    {
      const byte *ClipTab = CLIP_TAB[indexA];
      int max_imgpel_value = p_Vid->max_pel_value_comp[pl];
      int width = p->iLumaStride; //p->size_x;

      imgpel *imgP = &Img[get_pos_y_luma(MbP, ypos)][get_pos_x_luma(MbP, 0)];
      imgpel *imgQ = imgP + width;
      int pel;

      for( pel = 0 ; pel < MB_BLOCK_SIZE ; pel += 4 )
      {
        if(*Strength == 4 )    // INTRA strong filtering
        {
          int pixel;
          int inc_dim2 = width * 2;
          int inc_dim3 = width * 3;
          for( pixel = 0 ; pixel < BLOCK_SIZE ; ++pixel )
          {              
            imgpel *SrcPtrP = imgP++;
            imgpel *SrcPtrQ = imgQ++;
            imgpel  L0 = *SrcPtrP;
            imgpel  R0 = *SrcPtrQ;

            if( iabs( R0 - L0 ) < Alpha )
            { 
              imgpel  L1 = *(SrcPtrP - width);
              imgpel  R1 = *(SrcPtrQ + width);

              if ((iabs( R0 - R1) < Beta)  && (iabs(L0 - L1) < Beta))
              {
                if ((iabs( R0 - L0 ) < ((Alpha >> 2) + 2)))
                {
                  imgpel  L2 = *(SrcPtrP - inc_dim2);
                  imgpel  R2 = *(SrcPtrQ + inc_dim2);                
                  int RL0 = L0 + R0;

                  if (( iabs( L0 - L2) < Beta ))
                  {
                    imgpel  L3 = *(SrcPtrP - inc_dim3);
                    *(SrcPtrP         ) = (imgpel)  (( R1 + ((L1 + RL0) << 1) +  L2 + 4) >> 3);
                    *(SrcPtrP -= width) = (imgpel)  (( L2 + L1 + RL0 + 2) >> 2);
                    *(SrcPtrP -  width) = (imgpel) ((((L3 + L2) <<1) + L2 + L1 + RL0 + 4) >> 3);                
                  }
                  else
                  {
                    *SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2);
                  }

                  if (( iabs( R0 - R2) < Beta ))
                  {
                    imgpel  R3 = *(SrcPtrQ + inc_dim3);
                    *(SrcPtrQ          ) = (imgpel)  (( L1 + ((R1 + RL0) << 1) +  R2 + 4) >> 3);
                    *(SrcPtrQ += width ) = (imgpel)  (( R2 + R0 + L0 + R1 + 2) >> 2);
                    *(SrcPtrQ +  width ) = (imgpel) ((((R3 + R2) <<1) + R2 + R1 + RL0 + 4) >> 3);
                  }
                  else
                  {
                    *SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
                  }
                }
                else
                {
                  *SrcPtrP = (imgpel) (((L1 << 1) + L0 + R1 + 2) >> 2);
                  *SrcPtrQ = (imgpel) (((R1 << 1) + R0 + L1 + 2) >> 2);
                }
              }
            }
          }
        }
        else if( *Strength != 0) // normal filtering
        {
          int C0  = ClipTab[ *Strength ] * bitdepth_scale;
          int i;
          imgpel *SrcPtrP, *SrcPtrQ;
          int edge_diff;
          if (C0 == 0)
          {
            for( i= 0 ; i < BLOCK_SIZE ; ++i )
            {
              SrcPtrP = imgP++;
              SrcPtrQ = imgQ++;
              edge_diff = *SrcPtrQ - *SrcPtrP;

              if( iabs( edge_diff ) < Alpha )
              {          
                imgpel  *SrcPtrQ1 = SrcPtrQ + width;
                imgpel  *SrcPtrP1 = SrcPtrP - width;

                if ((iabs( *SrcPtrQ - *SrcPtrQ1) < Beta)  && (iabs(*SrcPtrP - *SrcPtrP1) < Beta))
                {                          
                  imgpel  R2 = *(SrcPtrQ1 + width);
                  imgpel  L2 = *(SrcPtrP1 - width);

                  int aq  = (iabs(*SrcPtrQ - R2) < Beta);
                  int ap  = (iabs(*SrcPtrP - L2) < Beta);

                  int tc0  = (ap + aq) ;
                  int dif = iClip3( -tc0, tc0, (((edge_diff) << 2) + (*SrcPtrP1 - *SrcPtrQ1) + 4) >> 3 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1(max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1(max_imgpel_value, *SrcPtrQ - dif);
                  }
                }
              }
            }
          }
          else
          {
            for( i= 0 ; i < BLOCK_SIZE ; ++i )
            {
              SrcPtrP = imgP++;
              SrcPtrQ = imgQ++;
              edge_diff = *SrcPtrQ - *SrcPtrP;

              if( iabs( edge_diff ) < Alpha )
              {          
                imgpel  *SrcPtrQ1 = SrcPtrQ + width;
                imgpel  *SrcPtrP1 = SrcPtrP - width;

                if ((iabs( *SrcPtrQ - *SrcPtrQ1) < Beta)  && (iabs(*SrcPtrP - *SrcPtrP1) < Beta))
                {                          
                  int RL0 = (*SrcPtrP + *SrcPtrQ + 1) >> 1;
                  imgpel  R2 = *(SrcPtrQ1 + width);
                  imgpel  L2 = *(SrcPtrP1 - width);

                  int aq  = (iabs(*SrcPtrQ - R2) < Beta);
                  int ap  = (iabs(*SrcPtrP - L2) < Beta);

                  int tc0  = (C0 + ap + aq) ;
                  int dif = iClip3( -tc0, tc0, (((edge_diff) << 2) + (*SrcPtrP1 - *SrcPtrQ1) + 4) >> 3 );

                  if( ap )
                    *SrcPtrP1 += iClip3( -C0,  C0, (L2 + RL0 - (*SrcPtrP1<<1)) >> 1 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1(max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1(max_imgpel_value, *SrcPtrQ - dif);
                  }

                  if( aq )
                    *SrcPtrQ1 += iClip3( -C0,  C0, (R2 + RL0 - (*SrcPtrQ1<<1)) >> 1 );          
                }
              }
            }
          }
        }
        else
        {
          imgP += 4;
          imgQ += 4;
        }
        Strength += 4;
      }      
    }
  }
}


/*!
 *****************************************************************************************
 * \brief
 *    Filters chroma block edge for Frame or Field coded pictures
 *****************************************************************************************
 */
static void EdgeLoopChromaVer(imgpel** Img, byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p)
{
  VideoParameters *p_Vid = MbQ->p_Vid;  

  int block_width  = p_Vid->mb_cr_size_x;
  int block_height = p_Vid->mb_cr_size_y;
  int xQ = edge - 1;
  int yQ = 0;  

  Macroblock *MbP = get_non_aff_neighbor_chroma(MbQ,xQ,yQ,block_width,block_height); 

  if (MbP || (MbQ->DFDisableIdc == 0))
  {
    int      bitdepth_scale   = p_Vid->bitdepth_scale[IS_CHROMA];
    int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

    int AlphaC0Offset = MbQ->DFAlphaC0Offset;
    int BetaOffset = MbQ->DFBetaOffset;

    // Average QP of the two blocks
    int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

    int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
    int indexB = iClip3(0, MAX_QP, QP + BetaOffset);

    int Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
    int Beta    = BETA_TABLE [indexB] * bitdepth_scale;

    if ((Alpha | Beta) != 0)
    {
      const int PelNum = pelnum_cr[0][p->chroma_format_idc];
      const     byte *ClipTab = CLIP_TAB[indexA];

      int pel;
      int pos_x1 = get_pos_x_chroma(MbP, xQ, (block_width - 1));
      imgpel **cur_img = &Img[get_pos_y_chroma(MbP,yQ, (block_height - 1))];

      for( pel = 0 ; pel < PelNum ; ++pel )
      {
        int Strng = Strength[(PelNum == 8) ? (((pel >> 1) << 2) + (pel & 0x01)) : pel];

        if( Strng != 0)
        {
          imgpel *SrcPtrP = *cur_img + pos_x1;
          imgpel *SrcPtrQ = SrcPtrP + 1;
          int edge_diff = *SrcPtrQ - *SrcPtrP;

          if ( iabs( edge_diff ) < Alpha ) 
          {
            imgpel R1  = *(SrcPtrQ + 1);
            if ( iabs(*SrcPtrQ - R1) < Beta )  
            {
              imgpel L1  = *(SrcPtrP - 1);
              if ( iabs(*SrcPtrP - L1) < Beta )
              {
                if( Strng == 4 )    // INTRA strong filtering
                {
                  *SrcPtrP = (imgpel) ( ((L1 << 1) + *SrcPtrP + R1 + 2) >> 2 );
                  *SrcPtrQ = (imgpel) ( ((R1 << 1) + *SrcPtrQ + L1 + 2) >> 2 );
                }
                else
                {
                  int tc0  = ClipTab[ Strng ] * bitdepth_scale + 1;
                  int dif = iClip3( -tc0, tc0, ( ((edge_diff) << 2) + (L1 - R1) + 4) >> 3 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, *SrcPtrQ - dif);
                  }
                }
              }
            }
          }
        }
        cur_img++;
      }     
    }
  }
}


/*!
 *****************************************************************************************
 * \brief
 *    Filters chroma block edge for Frame or Field coded pictures
 *****************************************************************************************
 */
static void EdgeLoopChromaHor(imgpel** Img, byte Strength[16], Macroblock *MbQ, int edge, int uv, StorablePicture *p)
{
  VideoParameters *p_Vid = MbQ->p_Vid;  
  int block_width = p_Vid->mb_cr_size_x;
  int block_height = p_Vid->mb_cr_size_y;
  int xQ = 0;
  int yQ = (edge < 16 ? edge - 1: 0);

  Macroblock *MbP = get_non_aff_neighbor_chroma(MbQ,xQ,yQ,block_width,block_height);

  if (MbP || (MbQ->DFDisableIdc == 0))
  {
    int      bitdepth_scale   = p_Vid->bitdepth_scale[IS_CHROMA];
    int      max_imgpel_value = p_Vid->max_pel_value_comp[uv + 1];

    int AlphaC0Offset = MbQ->DFAlphaC0Offset;
    int BetaOffset = MbQ->DFBetaOffset;
    int width = p->iChromaStride; //p->size_x_cr;

    // Average QP of the two blocks
    int QP = (MbP->qpc[uv] + MbQ->qpc[uv] + 1) >> 1;

    int indexA = iClip3(0, MAX_QP, QP + AlphaC0Offset);
    int indexB = iClip3(0, MAX_QP, QP + BetaOffset);

    int Alpha   = ALPHA_TABLE[indexA] * bitdepth_scale;
    int Beta    = BETA_TABLE [indexB] * bitdepth_scale;

    if ((Alpha | Beta) != 0)
    {
      const int PelNum = pelnum_cr[1][p->chroma_format_idc];
      const     byte *ClipTab = CLIP_TAB[indexA];

      int pel;

      imgpel *imgP = &Img[get_pos_y_chroma(MbP,yQ, (block_height-1))][get_pos_x_chroma(MbP,xQ, (block_width - 1))];
      imgpel *imgQ = imgP + width ;

      for( pel = 0 ; pel < PelNum ; ++pel )
      {
        int Strng = Strength[(PelNum == 8) ? (((pel >> 1) << 2) + (pel & 0x01)) : pel];

        if( Strng != 0)
        {
          imgpel *SrcPtrP = imgP;
          imgpel *SrcPtrQ = imgQ;
          int edge_diff = *imgQ - *imgP;

          if ( iabs( edge_diff ) < Alpha ) 
          {
            imgpel R1  = *(SrcPtrQ + width);
            if ( iabs(*SrcPtrQ - R1) < Beta )  
            {
              imgpel L1  = *(SrcPtrP - width);
              if ( iabs(*SrcPtrP - L1) < Beta )
              {
                if( Strng == 4 )    // INTRA strong filtering
                {
                  *SrcPtrP = (imgpel) ( ((L1 << 1) + *SrcPtrP + R1 + 2) >> 2 );
                  *SrcPtrQ = (imgpel) ( ((R1 << 1) + *SrcPtrQ + L1 + 2) >> 2 );
                }
                else
                {
                  int tc0  = ClipTab[ Strng ] * bitdepth_scale + 1;
                  int dif = iClip3( -tc0, tc0, ( ((edge_diff) << 2) + (L1 - R1) + 4) >> 3 );

                  if (dif != 0)
                  {
                    *SrcPtrP = (imgpel) iClip1 ( max_imgpel_value, *SrcPtrP + dif);
                    *SrcPtrQ = (imgpel) iClip1 ( max_imgpel_value, *SrcPtrQ - dif);
                  }
                }
              }
            }
          }
        }
        imgP++;
        imgQ++;
      }     
    }
  }
}


