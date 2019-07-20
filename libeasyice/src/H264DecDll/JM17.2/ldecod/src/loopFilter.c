
/*!
 *************************************************************************************
 * \file loopFilter.c
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

static void DeblockMb              (VideoParameters *p_Vid, StorablePicture *p, int MbQAddr);

extern void set_loop_filter_functions_mbaff(VideoParameters *p_Vid);
extern void set_loop_filter_functions_normal(VideoParameters *p_Vid);

#if (JM_PARALLEL_DEBLOCK == 0)
/*!
 *****************************************************************************************
 * \brief
 *    Filter all macroblocks in order of increasing macroblock address.
 *****************************************************************************************
 */
void DeblockPicture(VideoParameters *p_Vid, StorablePicture *p)
{
  unsigned i;
  for (i = 0; i < p->PicSizeInMbs; ++i)
  {
    DeblockMb( p_Vid, p, i ) ;
  }
}
#else
static void DeblockParallel(VideoParameters *p_Vid, StorablePicture *p, unsigned int column, int block, int n_last)
{
  int i, j;
  
  for (j = 0; j < GROUP_SIZE; j++)
  {
    i = block++ * (p_Vid->PicWidthInMbs - 2) + column;

    DeblockMb( p_Vid, p, i ) ;
    if (block == n_last) break;
  }
}

/*!
 *****************************************************************************************
 * \brief
 *    Filter all macroblocks in a diagonal manner to enable parallelization.
 *****************************************************************************************
 */
void DeblockPicture(VideoParameters *p_Vid, StorablePicture *p)
{
  int iheightMBs =(p_Vid->PicSizeInMbs/p_Vid->PicWidthInMbs);
  unsigned int i, k = p->PicWidthInMbs + 2 * (iheightMBs - 1);
  
  for (i = 0; i < k; i++)
  {
    int nn;    
    int n_last = imin(iheightMBs, (i >> 1) + 1);
    int n_start = (i < p->PicWidthInMbs) ? 0 : ((i - p->PicWidthInMbs) >> 1) + 1;

#if defined(OPENMP)
    #pragma omp parallel for
#endif
    for (nn = n_start; nn < n_last; nn += GROUP_SIZE)
      DeblockParallel(p_Vid, p, i, nn, n_last);
  }
}
#endif


void DeblockPicturePartially(VideoParameters *p_Vid, StorablePicture *p, int iStart, int iEnd)
{
  int i;

  for (i = iStart; i < imin(iEnd, (int)p->PicSizeInMbs); ++i)
  {
    DeblockMb( p_Vid, p, i ) ;
  }
}


// likely already set - see testing via asserts
static void init_neighbors(VideoParameters *p_Vid)
{
  int i,j,addr;
  int width = p_Vid->PicWidthInMbs;
  int height = p_Vid->PicHeightInMbs;
  int size = p_Vid->PicSizeInMbs;
  // do the top left corner
  p_Vid->mb_data[0].mbup = NULL;
  p_Vid->mb_data[0].mbleft = NULL;
  // do top row
  for (i = 1; i < width; i++) {
    p_Vid->mb_data[i].mbup = NULL;
    p_Vid->mb_data[i].mbleft = &(p_Vid->mb_data[i - 1]);
  }
  // do left edge
  for (i = width; i < size; i += width) {
    p_Vid->mb_data[i].mbup = &(p_Vid->mb_data[i - width]);
    p_Vid->mb_data[i].mbleft = NULL;
  }
  // do all others
  for (j = 1; j < height; j++) {
    for (i = 1; i < width; i++) {
      addr = j * width + i;
      p_Vid->mb_data[addr].mbup = &(p_Vid->mb_data[addr - width]);
      p_Vid->mb_data[addr].mbleft = &(p_Vid->mb_data[addr - 1]);
    }
  }
}


void  init_Deblock(VideoParameters *p_Vid, int mb_aff_frame_flag)
{
  if(p_Vid->yuv_format == YUV444 && p_Vid->separate_colour_plane_flag)
  {
    change_plane_JV(p_Vid, PLANE_Y, NULL);
    init_neighbors(p_Dec->p_Vid);
    change_plane_JV(p_Vid, PLANE_U, NULL);
    init_neighbors(p_Dec->p_Vid);
    change_plane_JV(p_Vid, PLANE_V, NULL);
    init_neighbors(p_Dec->p_Vid);
    change_plane_JV(p_Vid, PLANE_Y, NULL);
  }
  else 
    init_neighbors(p_Dec->p_Vid);
  if (mb_aff_frame_flag == 1) 
  {
    set_loop_filter_functions_mbaff(p_Vid);
  }
  else
  {
    set_loop_filter_functions_normal(p_Vid);
  }
}

/*!
 *****************************************************************************************
 * \brief
 *    Deblocking filter for one macroblock.
 *****************************************************************************************
 */

static void DeblockMb(VideoParameters *p_Vid, StorablePicture *p, int MbQAddr)
{
  Macroblock   *MbQ = &(p_Vid->mb_data[MbQAddr]) ; // current Mb

  // return, if filter is disabled
  if (MbQ->DFDisableIdc == 1) 
  {
    MbQ->DeblockCall = 0;
  }
  else
  {
    int           edge;
    byte Strength[16];
    int64         *p_Strength64 = (int64 *) Strength;
    short         mb_x, mb_y;

    int           filterNon8x8LumaEdgesFlag[4] = {1,1,1,1};
    int           filterLeftMbEdgeFlag;
    int           filterTopMbEdgeFlag;
    int           edge_cr;

    imgpel     **imgY = p->imgY;
    imgpel   ***imgUV = p->imgUV;
    Slice  *currSlice = MbQ->p_Slice;
    int       mvlimit = ((p->structure!=FRAME) || (p->mb_aff_frame_flag && MbQ->mb_field)) ? 2 : 4;

    seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

    MbQ->DeblockCall = 1;
    get_mb_pos (p_Vid, MbQAddr, p_Vid->mb_size[IS_LUMA], &mb_x, &mb_y);

    if (MbQ->mb_type == I8MB)
      assert(MbQ->luma_transform_size_8x8_flag);

    filterNon8x8LumaEdgesFlag[1] =
      filterNon8x8LumaEdgesFlag[3] = !(MbQ->luma_transform_size_8x8_flag);

    filterLeftMbEdgeFlag = (mb_x != 0);
    filterTopMbEdgeFlag  = (mb_y != 0);

    if (p->mb_aff_frame_flag && mb_y == MB_BLOCK_SIZE && MbQ->mb_field)
      filterTopMbEdgeFlag = 0;

    if (MbQ->DFDisableIdc==2)
    {
      // don't filter at slice boundaries
      filterLeftMbEdgeFlag = MbQ->mbAvailA;
      // if this the bottom of a frame macroblock pair then always filter the top edge
      filterTopMbEdgeFlag  = (p->mb_aff_frame_flag && !MbQ->mb_field && (MbQAddr & 0x01)) ? 1 : MbQ->mbAvailB;
    }

    if (p->mb_aff_frame_flag == 1) 
      CheckAvailabilityOfNeighbors(MbQ);

    // Vertical deblocking
    for (edge = 0; edge < 4 ; ++edge )    
    {
      // If cbp == 0 then deblocking for some macroblock types could be skipped
      if (MbQ->cbp == 0)
      {
        if (filterNon8x8LumaEdgesFlag[edge] == 0 && active_sps->chroma_format_idc != YUV444)
          continue;
        else if (edge > 0)
        {
          if (((MbQ->mb_type == PSKIP && currSlice->slice_type == P_SLICE) || (MbQ->mb_type == P16x16) || (MbQ->mb_type == P16x8)))
            continue;
          else if ((edge & 0x01) && ((MbQ->mb_type == P8x16) || (currSlice->slice_type == B_SLICE && MbQ->mb_type == BSKIP_DIRECT && active_sps->direct_8x8_inference_flag)))
            continue;
        }
      }

      if( edge || filterLeftMbEdgeFlag )
      {      
        // Strength for 4 blks in 1 stripe
        p_Vid->GetStrengthVer(Strength, MbQ, edge << 2, mvlimit, p);

        if ((p_Strength64[0]) || (p_Strength64[1])) // only if one of the 16 Strength bytes is != 0
        {
          if (filterNon8x8LumaEdgesFlag[edge])
          {
            p_Vid->EdgeLoopLumaVer( PLANE_Y, imgY, Strength, MbQ, edge << 2, p) ;
            if(currSlice->is_not_independent)
            {
              p_Vid->EdgeLoopLumaVer(PLANE_U, imgUV[0], Strength, MbQ, edge << 2, p);
              p_Vid->EdgeLoopLumaVer(PLANE_V, imgUV[1], Strength, MbQ, edge << 2, p);
            }
          }
          if (active_sps->chroma_format_idc==YUV420 || active_sps->chroma_format_idc==YUV422)
          {
            edge_cr = chroma_edge[0][edge][p->chroma_format_idc];
            if( (imgUV != NULL) && (edge_cr >= 0))
            {
              p_Vid->EdgeLoopChromaVer( imgUV[0], Strength, MbQ, edge_cr, 0, p);
              p_Vid->EdgeLoopChromaVer( imgUV[1], Strength, MbQ, edge_cr, 1, p);
            }
          }
        }        
      }
    }//end edge

    // horizontal deblocking  
    for( edge = 0; edge < 4 ; ++edge )
    {
      // If cbp == 0 then deblocking for some macroblock types could be skipped
      if (MbQ->cbp == 0)
      {
        if (filterNon8x8LumaEdgesFlag[edge] == 0 && active_sps->chroma_format_idc==YUV420)
          continue;
        else if (edge > 0)
        {
          if (((MbQ->mb_type == PSKIP && currSlice->slice_type == P_SLICE) || (MbQ->mb_type == P16x16) || (MbQ->mb_type == P8x16)))
            continue;
          else if ((edge & 0x01) && ((MbQ->mb_type == P16x8) || (currSlice->slice_type == B_SLICE && MbQ->mb_type == BSKIP_DIRECT && active_sps->direct_8x8_inference_flag)))
            continue;
        }
      }

      if( edge || filterTopMbEdgeFlag )
      {
        // Strength for 4 blks in 1 stripe
        p_Vid->GetStrengthHor(Strength, MbQ, edge << 2, mvlimit, p);

        if ((p_Strength64[0]) || (p_Strength64[1])) // only if one of the 16 Strength bytes is != 0
        {
          if (filterNon8x8LumaEdgesFlag[edge])
          {
            p_Vid->EdgeLoopLumaHor( PLANE_Y, imgY, Strength, MbQ, edge << 2, p) ;
            if(currSlice->is_not_independent)
            {
              p_Vid->EdgeLoopLumaHor(PLANE_U, imgUV[0], Strength, MbQ, edge << 2, p);
              p_Vid->EdgeLoopLumaHor(PLANE_V, imgUV[1], Strength, MbQ, edge << 2, p);
            }
          }
          
          if (active_sps->chroma_format_idc==YUV420 || active_sps->chroma_format_idc==YUV422)
          {
            edge_cr = chroma_edge[1][edge][p->chroma_format_idc];
            if( (imgUV != NULL) && (edge_cr >= 0))
            {
              p_Vid->EdgeLoopChromaHor( imgUV[0], Strength, MbQ, edge_cr, 0, p);
              p_Vid->EdgeLoopChromaHor( imgUV[1], Strength, MbQ, edge_cr, 1, p);
            }
          }
        }

        if (!edge && !MbQ->mb_field && MbQ->mixedModeEdgeFlag) //currSlice->mixedModeEdgeFlag) 
        {          
          // this is the extra horizontal edge between a frame macroblock pair and a field above it
          MbQ->DeblockCall = 2;
          p_Vid->GetStrengthHor(Strength, MbQ, MB_BLOCK_SIZE, mvlimit, p); // Strength for 4 blks in 1 stripe

          //if( *((int*)Strength) )                      // only if one of the 4 Strength bytes is != 0
          {
            if (filterNon8x8LumaEdgesFlag[edge])
            {

              p_Vid->EdgeLoopLumaHor(PLANE_Y, imgY, Strength, MbQ, MB_BLOCK_SIZE, p) ;
              if(currSlice->is_not_independent)
              {
                p_Vid->EdgeLoopLumaHor(PLANE_U, imgUV[0], Strength, MbQ, MB_BLOCK_SIZE, p) ;
                p_Vid->EdgeLoopLumaHor(PLANE_V, imgUV[1], Strength, MbQ, MB_BLOCK_SIZE, p) ;
              }
            }
            if (active_sps->chroma_format_idc==YUV420 || active_sps->chroma_format_idc==YUV422) 
            {
              edge_cr = chroma_edge[1][edge][p->chroma_format_idc];
              if( (imgUV != NULL) && (edge_cr >= 0))
              {
                p_Vid->EdgeLoopChromaHor( imgUV[0], Strength, MbQ, MB_BLOCK_SIZE, 0, p) ;
                p_Vid->EdgeLoopChromaHor( imgUV[1], Strength, MbQ, MB_BLOCK_SIZE, 1, p) ;
              }
            }
          }
          MbQ->DeblockCall = 1;
        }
      }
    }//end edge  

    MbQ->DeblockCall = 0;
  }
}

