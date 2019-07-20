/*!
 *************************************************************************************
 * \file intra8x8_pred.c
 *
 * \brief
 *    Functions for intra 8x8 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Yuri Vatis
 *      - Jan Muenster
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra8x8_pred.h"
#include "mb_access.h"
#include "image.h"


extern int intrapred8x8_normal(Macroblock *currMB, ColorPlane pl, int ioff, int joff);
extern int intrapred8x8_mbaff(Macroblock *currMB, ColorPlane pl, int ioff, int joff);

/*!
 ************************************************************************
 * \brief
 *    Make intra 8x8 prediction according to all 9 prediction modes.
 *    The routine uses left and upper neighbouring points from
 *    previous coded blocks to do this (if available). Notice that
 *    inaccessible neighbouring points are signalled with a negative
 *    value in the predmode array .
 *
 *  \par Input:
 *     Starting point of current 8x8 block image position
 *
 ************************************************************************
 */
int intrapred8x8(Macroblock *currMB,    //!< Current Macroblock
                 ColorPlane pl,         //!< Current color plane
                 int ioff,              //!< ioff
                 int joff)              //!< joff

{  
  //VideoParameters *p_Vid = currMB->p_Vid;
  int block_x = (currMB->block_x) + (ioff >> 2);
  int block_y = (currMB->block_y) + (joff >> 2);
  byte predmode = currMB->p_Slice->ipredmode[block_y][block_x];

  currMB->ipmode_DPCM = predmode;  //For residual DPCM

  if (currMB->p_Slice->mb_aff_frame_flag)
  {
    return intrapred8x8_mbaff(currMB, pl, ioff, joff);   
  }
  else
  {
    return intrapred8x8_normal(currMB, pl, ioff, joff);
  }
}


