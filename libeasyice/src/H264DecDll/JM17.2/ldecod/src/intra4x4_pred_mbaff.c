/*!
 *************************************************************************************
 * \file intra4x4_pred.c
 *
 * \brief
 *    Functions for intra 4x4 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra4x4_pred.h"
#include "mb_access.h"
#include "image.h"

// Notation for comments regarding prediction and predictors.
// The pels of the 4x4 block are labelled a..p. The predictor pels above
// are labelled A..H, from the left I..L, and from above left X, as follows:
//
//  X A B C D E F G H
//  I a b c d
//  J e f g h
//  K i j k l
//  L m n o p
//

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 DC prediction mode
 *
 * \param currMB
 *    current MB structure
 * \param pl
 *    color plane
 * \param ioff
 *    pixel offset X within MB
 * \param joff
 *    pixel offset Y within MB
 * \return
 *    DECODING_OK   decoding of intra prediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_dc_pred_mbaff(Macroblock *currMB, 
                                   ColorPlane pl,               
                                   int ioff,
                                   int joff)
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;  

  int i,j;
  int s0 = 0;  
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4], pix_b;

  int block_available_up;
  int block_available_left;  

  imgpel **mb_pred = currSlice->mb_pred[pl];    

  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff - 1, joff + i, p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }
  p_Vid->getNeighbour(currMB, ioff    , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_b);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
  }

  // form predictor pels
  if (block_available_up)
  {
    s0 += imgY[pix_b.pos_y][pix_b.pos_x + 0];
    s0 += imgY[pix_b.pos_y][pix_b.pos_x + 1];
    s0 += imgY[pix_b.pos_y][pix_b.pos_x + 2];
    s0 += imgY[pix_b.pos_y][pix_b.pos_x + 3];
  }

  if (block_available_left)
  {
    s0 += imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    s0 += imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    s0 += imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    s0 += imgY[pix_a[3].pos_y][pix_a[3].pos_x];
  }

  if (block_available_up && block_available_left)
  {
    // no edge
    s0 = (s0 + 4)>>3;
  }
  else if (!block_available_up && block_available_left)
  {
    // upper edge
    s0 = (s0 + 2)>>2;
  }
  else if (block_available_up && !block_available_left)
  {
    // left edge
    s0 = (s0 + 2)>>2;
  }
  else //if (!block_available_up && !block_available_left)
  {
    // top left corner, nothing to predict from
    s0 = p_Vid->dc_pred_value_comp[pl];
  }

  for (j=joff; j < joff + BLOCK_SIZE; ++j)
  {
    // store DC prediction
    mb_pred[j][ioff    ] = (imgpel) s0;
    mb_pred[j][ioff + 1] = (imgpel) s0;
    mb_pred[j][ioff + 2] = (imgpel) s0;
    mb_pred[j][ioff + 3] = (imgpel) s0;
  }
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 vertical prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_vert_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                     ColorPlane pl,         //!< current image plane
                                     int ioff,              //!< pixel offset X within MB
                                     int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
   
  int block_available_up;
  PixelPos pix_b;

  getAffNeighbour(currMB, ioff, joff - 1 , p_Vid->mb_size[IS_LUMA], &pix_b);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    block_available_up = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
  }
  else
  {
    block_available_up = pix_b.available;
  }

  if (!block_available_up)
  {
    printf ("warning: Intra_4x4_Vertical prediction mode not allowed at mb %d\n", (int) currSlice->current_mb_nr);
  }
  else
  {
    imgpel **mb_pred = currSlice->mb_pred[pl];
    imgpel *imgY = (pl) ? &currSlice->dec_picture->imgUV[pl - 1][pix_b.pos_y][pix_b.pos_x] : &currSlice->dec_picture->imgY[pix_b.pos_y][pix_b.pos_x];
    memcpy(&(mb_pred[joff++][ioff]), imgY, BLOCK_SIZE * sizeof(imgpel));
    memcpy(&(mb_pred[joff++][ioff]), imgY, BLOCK_SIZE * sizeof(imgpel));
    memcpy(&(mb_pred[joff++][ioff]), imgY, BLOCK_SIZE * sizeof(imgpel));
    memcpy(&(mb_pred[joff  ][ioff]), imgY, BLOCK_SIZE * sizeof(imgpel));
  }
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 horizontal prediction mode
 *
 * \param currMB
 *    current MB structure
 * \param pl
 *    color plane
 * \param ioff
 *    pixel offset X within MB
 * \param joff
 *    pixel offset Y within MB
 *
 * \return
 *    DECODING_OK   decoding of intra prediction mode was successful
 *
 ***********************************************************************
 */
static int intra4x4_hor_pred_mbaff(Macroblock *currMB, 
                                    ColorPlane pl,               
                                    int ioff,
                                    int joff)
{
  VideoParameters *p_Vid = currMB->p_Vid;
  Slice *currSlice = currMB->p_Slice;

  int i,j;
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4];

  int block_available_left;

  imgpel *predrow, prediction, **mb_pred = currSlice->mb_pred[pl];    

  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff -1 , joff +i , p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
  }

  if (!block_available_left)
    printf ("warning: Intra_4x4_Horizontal prediction mode not allowed at mb %d\n",(int) currSlice->current_mb_nr);

  for(j=0;j<BLOCK_SIZE;++j)
  {
    predrow = mb_pred[j+joff];
    prediction = imgY[pix_a[j].pos_y][pix_a[j].pos_x];
    for(i = ioff;i < ioff + BLOCK_SIZE;++i)
      predrow[i]= prediction; /* store predicted 4x4 block */
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 diagonal down right prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_diag_down_right_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                                      ColorPlane pl,         //!< current image plane
                                                      int ioff,              //!< pixel offset X within MB
                                                      int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4];
  PixelPos pix_b, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;

  imgpel **mb_pred = currSlice->mb_pred[pl];    

  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff -1 , joff +i , p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }

  p_Vid->getNeighbour(currMB, ioff    , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_b);
  p_Vid->getNeighbour(currMB, ioff -1 , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_d);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? currSlice->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_4x4_Diagonal_Down_Right prediction mode not allowed at mb %d\n",(int) currSlice->current_mb_nr);
  else
  {
    imgpel PredPixel[7];
    imgpel *pred_pel = &imgY[pix_b.pos_y][pix_b.pos_x];
    // form predictor pels
    imgpel p_a = *pred_pel++;
    imgpel p_b = *pred_pel++;
    imgpel p_c = *pred_pel++;
    imgpel p_d = *pred_pel  ;

    imgpel p_i = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    imgpel p_j = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    imgpel p_k = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    imgpel p_l = imgY[pix_a[3].pos_y][pix_a[3].pos_x];

    imgpel p_x = imgY[pix_d.pos_y][pix_d.pos_x];
    
    PredPixel[0] = (imgpel) ((p_l + 2*p_k + p_j + 2) >> 2);
    PredPixel[1] = (imgpel) ((p_k + 2*p_j + p_i + 2) >> 2);
    PredPixel[2] = (imgpel) ((p_j + 2*p_i + p_x + 2) >> 2);
    PredPixel[3] = (imgpel) ((p_i + 2*p_x + p_a + 2) >> 2);
    PredPixel[4] = (imgpel) ((p_x + 2*p_a + p_b + 2) >> 2);
    PredPixel[5] = (imgpel) ((p_a + 2*p_b + p_c + 2) >> 2);
    PredPixel[6] = (imgpel) ((p_b + 2*p_c + p_d + 2) >> 2);

    memcpy(&mb_pred[joff++][ioff], &PredPixel[3], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[2], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[1], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff  ][ioff], &PredPixel[0], 4 * sizeof(imgpel));
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 diagonal down left prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_diag_down_left_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                        ColorPlane pl,         //!< current image plane
                                        int ioff,              //!< pixel offset X within MB
                                        int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  PixelPos pix_b, pix_c;

  int block_available_up;
  int block_available_up_right;

  p_Vid->getNeighbour(currMB, ioff    , joff - 1, p_Vid->mb_size[IS_LUMA], &pix_b);
  p_Vid->getNeighbour(currMB, ioff + 4, joff - 1, p_Vid->mb_size[IS_LUMA], &pix_c);

  pix_c.available = pix_c.available && !((ioff==4) && ((joff==4)||(joff==12)));

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? currSlice->intra_block [pix_c.mb_addr] : 0;
  }
  else
  {
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
  }

  if (!block_available_up)
    printf ("warning: Intra_4x4_Diagonal_Down_Left prediction mode not allowed at mb %d\n", (int) currSlice->current_mb_nr);
  else
  {
    imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;
    imgpel **mb_pred = currSlice->mb_pred[pl];    

    imgpel p_e, p_f, p_g, p_h;
    imgpel PredPixel[8];
    imgpel *pred_pel = &imgY[pix_b.pos_y][pix_b.pos_x];

    // form predictor pels
    imgpel p_a = *pred_pel++;
    imgpel p_b = *pred_pel++;
    imgpel p_c = *pred_pel++;
    imgpel p_d = *pred_pel  ;

    if (block_available_up_right)
    {
      pred_pel = &imgY[pix_c.pos_y][pix_c.pos_x];
      p_e = *pred_pel++;
      p_f = *pred_pel++;
      p_g = *pred_pel++;
      p_h = *pred_pel  ;
    }
    else
    {
      p_e = p_f = p_g = p_h = p_d;
    }

    PredPixel[0] = (imgpel) ((p_a + p_c + 2*(p_b) + 2) >> 2);
    PredPixel[1] = (imgpel) ((p_b + p_d + 2*(p_c) + 2) >> 2);
    PredPixel[2] = (imgpel) ((p_c + p_e + 2*(p_d) + 2) >> 2);
    PredPixel[3] = (imgpel) ((p_d + p_f + 2*(p_e) + 2) >> 2);
    PredPixel[4] = (imgpel) ((p_e + p_g + 2*(p_f) + 2) >> 2);
    PredPixel[5] = (imgpel) ((p_f + p_h + 2*(p_g) + 2) >> 2);
    PredPixel[6] = (imgpel) ((p_g + 3*(p_h) + 2) >> 2);

    memcpy(&mb_pred[joff++][ioff], &PredPixel[0], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[1], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[2], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff  ][ioff], &PredPixel[3], 4 * sizeof(imgpel));
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 vertical right prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_vert_right_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                          ColorPlane pl,         //!< current image plane
                                          int ioff,              //!< pixel offset X within MB
                                          int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4];
  PixelPos pix_b, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;

  imgpel **mb_pred = currSlice->mb_pred[pl];    

  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff -1 , joff +i , p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }

  p_Vid->getNeighbour(currMB, ioff    , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_b);
  p_Vid->getNeighbour(currMB, ioff -1 , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_d);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? currSlice->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_4x4_Vertical_Right prediction mode not allowed at mb %d\n", (int) currSlice->current_mb_nr);
  {
    imgpel PredPixel[10];
    imgpel *pred_pel = &imgY[pix_b.pos_y][pix_b.pos_x];

    // form predictor pels
    imgpel p_a = *pred_pel++;
    imgpel p_b = *pred_pel++;
    imgpel p_c = *pred_pel++;
    imgpel p_d = *pred_pel  ;

    imgpel p_i = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    imgpel p_j = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    imgpel p_k = imgY[pix_a[2].pos_y][pix_a[2].pos_x];

    imgpel p_x = imgY[pix_d.pos_y][pix_d.pos_x];
    
    PredPixel[0] = (imgpel) ((p_x + 2*p_i + p_j + 2) >> 2);
    PredPixel[1] = (imgpel) ((p_x + p_a + 1) >> 1);
    PredPixel[2] = (imgpel) ((p_a + p_b + 1) >> 1);
    PredPixel[3] = (imgpel) ((p_b + p_c + 1) >> 1);
    PredPixel[4] = (imgpel) ((p_c + p_d + 1) >> 1);
    PredPixel[5] = (imgpel) ((p_i + 2*p_j + p_k + 2) >> 2);
    PredPixel[6] = (imgpel) ((p_i + 2*p_x + p_a + 2) >> 2);
    PredPixel[7] = (imgpel) ((p_x + 2*p_a + p_b + 2) >> 2);
    PredPixel[8] = (imgpel) ((p_a + 2*p_b + p_c + 2) >> 2);
    PredPixel[9] = (imgpel) ((p_b + 2*p_c + p_d + 2) >> 2);

    memcpy(&mb_pred[joff++][ioff], &PredPixel[1], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[6], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[0], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff  ][ioff], &PredPixel[5], 4 * sizeof(imgpel));    
    
  }

  return DECODING_OK;
}


/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 vertical left prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_vert_left_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                          ColorPlane pl,         //!< current image plane
                                          int ioff,              //!< pixel offset X within MB
                                          int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  PixelPos pix_b, pix_c;

  int block_available_up;
  int block_available_up_right;

  p_Vid->getNeighbour(currMB, ioff    , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_b);
  p_Vid->getNeighbour(currMB, ioff +4 , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_c);

  pix_c.available = pix_c.available && !((ioff==4) && ((joff==4)||(joff==12)));
  
  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
    block_available_up_right = pix_c.available ? currSlice->intra_block [pix_c.mb_addr] : 0;
  }
  else
  {
    block_available_up       = pix_b.available;
    block_available_up_right = pix_c.available;
  }


  if (!block_available_up)
    printf ("warning: Intra_4x4_Vertical_Left prediction mode not allowed at mb %d\n", (int) currSlice->current_mb_nr);
  {
    imgpel PredPixel[10];
    imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;
    imgpel **mb_pred = currSlice->mb_pred[pl];    
    imgpel *pred_pel = &imgY[pix_b.pos_y][pix_b.pos_x];

    // form predictor pels
    imgpel p_a = *pred_pel++;
    imgpel p_b = *pred_pel++;
    imgpel p_c = *pred_pel++;
    imgpel p_d = *pred_pel  ;
    imgpel p_e, p_f, p_g;

    if (block_available_up_right)
    {
      imgpel *pred_pel = &imgY[pix_c.pos_y][pix_c.pos_x];
      p_e = *pred_pel++;
      p_f = *pred_pel++;
      p_g = *pred_pel++;
    }
    else
    {
      p_e = p_f = p_g = p_d;
    }

    PredPixel[0] = (imgpel) ((p_a + p_b + 1) >> 1);
    PredPixel[1] = (imgpel) ((p_b + p_c + 1) >> 1);
    PredPixel[2] = (imgpel) ((p_c + p_d + 1) >> 1);
    PredPixel[3] = (imgpel) ((p_d + p_e + 1) >> 1);
    PredPixel[4] = (imgpel) ((p_e + p_f + 1) >> 1);
    PredPixel[5] = (imgpel) ((p_a + 2*p_b + p_c + 2) >> 2);
    PredPixel[6] = (imgpel) ((p_b + 2*p_c + p_d + 2) >> 2);
    PredPixel[7] = (imgpel) ((p_c + 2*p_d + p_e + 2) >> 2);
    PredPixel[8] = (imgpel) ((p_d + 2*p_e + p_f + 2) >> 2);
    PredPixel[9] = (imgpel) ((p_e + 2*p_f + p_g + 2) >> 2);

    memcpy(&mb_pred[joff++][ioff], &PredPixel[0], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[5], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[1], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff  ][ioff], &PredPixel[6], 4 * sizeof(imgpel));
  }
  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 horizontal up prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_hor_up_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                      ColorPlane pl,         //!< current image plane
                                      int ioff,              //!< pixel offset X within MB
                                      int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i;
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4];

  int block_available_left;

  imgpel **mb_pred = currSlice->mb_pred[pl];    

  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff -1 , joff +i , p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
  }

  if (!block_available_left)
    printf ("warning: Intra_4x4_Horizontal_Up prediction mode not allowed at mb %d\n",(int) currSlice->current_mb_nr);
  else
  {
    imgpel PredPixel[10];

    // form predictor pels
    imgpel p_i = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    imgpel p_j = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    imgpel p_k = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    imgpel p_l = imgY[pix_a[3].pos_y][pix_a[3].pos_x];

    PredPixel[0] = (imgpel) ((p_i + p_j + 1) >> 1);
    PredPixel[1] = (imgpel) ((p_i + 2*p_j + p_k + 2) >> 2);
    PredPixel[2] = (imgpel) ((p_j + p_k + 1) >> 1);
    PredPixel[3] = (imgpel) ((p_j + 2*p_k + p_l + 2) >> 2);
    PredPixel[4] = (imgpel) ((p_k + p_l + 1) >> 1);
    PredPixel[5] = (imgpel) ((p_k + 2*p_l + p_l + 2) >> 2);
    PredPixel[6] = (imgpel) p_l;
    PredPixel[7] = (imgpel) p_l;
    PredPixel[8] = (imgpel) p_l;
    PredPixel[9] = (imgpel) p_l;

    memcpy(&mb_pred[joff++][ioff], &PredPixel[0], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[2], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[4], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[6], 4 * sizeof(imgpel));
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 horizontal down prediction mode
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *
 ***********************************************************************
 */
static int intra4x4_hor_down_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                                         ColorPlane pl,         //!< current image plane
                                         int ioff,              //!< pixel offset X within MB
                                         int joff)              //!< pixel offset Y within MB
{
  Slice *currSlice = currMB->p_Slice;
  VideoParameters *p_Vid = currMB->p_Vid;
  
  int i; 
  imgpel **imgY = (pl) ? currSlice->dec_picture->imgUV[pl - 1] : currSlice->dec_picture->imgY;

  PixelPos pix_a[4];
  PixelPos pix_b, pix_d;

  int block_available_up;
  int block_available_left;
  int block_available_up_left;

  imgpel **mb_pred = currSlice->mb_pred[pl];    
  
  for (i=0;i<4;++i)
  {
    p_Vid->getNeighbour(currMB, ioff -1 , joff +i , p_Vid->mb_size[IS_LUMA], &pix_a[i]);
  }

  p_Vid->getNeighbour(currMB, ioff    , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_b);
  p_Vid->getNeighbour(currMB, ioff -1 , joff -1 , p_Vid->mb_size[IS_LUMA], &pix_d);

  if (p_Vid->active_pps->constrained_intra_pred_flag)
  {
    for (i=0, block_available_left=1; i<4;++i)
      block_available_left  &= pix_a[i].available ? currSlice->intra_block[pix_a[i].mb_addr]: 0;
    block_available_up       = pix_b.available ? currSlice->intra_block [pix_b.mb_addr] : 0;
    block_available_up_left  = pix_d.available ? currSlice->intra_block [pix_d.mb_addr] : 0;
  }
  else
  {
    block_available_left     = pix_a[0].available;
    block_available_up       = pix_b.available;
    block_available_up_left  = pix_d.available;
  }

  if ((!block_available_up)||(!block_available_left)||(!block_available_up_left))
    printf ("warning: Intra_4x4_Horizontal_Down prediction mode not allowed at mb %d\n", (int) currSlice->current_mb_nr);
  else
  {
    imgpel PredPixel[10];
    imgpel *pred_pel = &imgY[pix_b.pos_y][pix_b.pos_x];

    // form predictor pels
    imgpel p_a = *pred_pel++;
    imgpel p_b = *pred_pel++;
    imgpel p_c = *pred_pel++;

    imgpel p_i = imgY[pix_a[0].pos_y][pix_a[0].pos_x];
    imgpel p_j = imgY[pix_a[1].pos_y][pix_a[1].pos_x];
    imgpel p_k = imgY[pix_a[2].pos_y][pix_a[2].pos_x];
    imgpel p_l = imgY[pix_a[3].pos_y][pix_a[3].pos_x];

    imgpel p_x = imgY[pix_d.pos_y][pix_d.pos_x];

    PredPixel[0] = (imgpel) ((p_k + p_l + 1) >> 1);
    PredPixel[1] = (imgpel) ((p_j + 2*p_k + p_l + 2) >> 2);
    PredPixel[2] = (imgpel) ((p_j + p_k + 1) >> 1);
    PredPixel[3] = (imgpel) ((p_i + 2*p_j + p_k + 2) >> 2);
    PredPixel[4] = (imgpel) ((p_i + p_j + 1) >> 1);
    PredPixel[5] = (imgpel) ((p_x + 2*p_i + p_j + 2) >> 2);
    PredPixel[6] = (imgpel) ((p_x + p_i + 1) >> 1);
    PredPixel[7] = (imgpel) ((p_i + 2*p_x + p_a + 2) >> 2);
    PredPixel[8] = (imgpel) ((p_x + 2*p_a + p_b + 2) >> 2);
    PredPixel[9] = (imgpel) ((p_a + 2*p_b + p_c + 2) >> 2);

    memcpy(&mb_pred[joff++][ioff], &PredPixel[6], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[4], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff++][ioff], &PredPixel[2], 4 * sizeof(imgpel));
    memcpy(&mb_pred[joff  ][ioff], &PredPixel[0], 4 * sizeof(imgpel));
  }

  return DECODING_OK;
}

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 4x4 intra prediction blocks 
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
int intra4x4_pred_mbaff(Macroblock *currMB,    //!< current macroblock
                        ColorPlane pl,         //!< current image plane
                        int ioff,              //!< pixel offset X within MB
                        int joff,              //!< pixel offset Y within MB
                        int img_block_x,       //!< location of block X, multiples of 4
                        int img_block_y)       //!< location of block Y, multiples of 4
{
  VideoParameters *p_Vid = currMB->p_Vid;
  byte predmode = p_Vid->ipredmode[img_block_y][img_block_x];
  currMB->ipmode_DPCM = predmode; //For residual DPCM

  switch (predmode)
  {
  case DC_PRED:
    return (intra4x4_dc_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case VERT_PRED:
    return (intra4x4_vert_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case HOR_PRED:
    return (intra4x4_hor_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case DIAG_DOWN_RIGHT_PRED:
    return (intra4x4_diag_down_right_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case DIAG_DOWN_LEFT_PRED:
    return (intra4x4_diag_down_left_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case VERT_RIGHT_PRED:
    return (intra4x4_vert_right_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case VERT_LEFT_PRED:
    return (intra4x4_vert_left_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case HOR_UP_PRED:
    return (intra4x4_hor_up_pred_mbaff(currMB, pl, ioff, joff));
    break;
  case HOR_DOWN_PRED:  
    return (intra4x4_hor_down_pred_mbaff(currMB, pl, ioff, joff));
  default:
    printf("Error: illegal intra_4x4 prediction mode: %d\n", (int) predmode);
    return SEARCH_SYNC;
    break;
  }
}
