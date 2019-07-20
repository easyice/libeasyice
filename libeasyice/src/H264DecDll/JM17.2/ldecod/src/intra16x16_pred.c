/*!
 *************************************************************************************
 * \file intra16x16_pred.c
 *
 * \brief
 *    Functions for intra 16x16 prediction
 *
 * \author
 *      Main contributors (see contributors.h for copyright, 
 *                         address and affiliation details)
 *      - Alexis Michael Tourapis  <alexismt@ieee.org>
 *
 *************************************************************************************
 */
#include "global.h"
#include "intra16x16_pred.h"
#include "mb_access.h"
#include "image.h"


extern int intrapred_16x16_mbaff (Macroblock *currMB, ColorPlane pl, int predmode);
extern int intrapred_16x16_normal(Macroblock *currMB, ColorPlane pl, int predmode);

/*!
 ***********************************************************************
 * \brief
 *    makes and returns 16x16 intra prediction blocks 
 *
 * \return
 *    DECODING_OK   decoding of intraprediction mode was successful            \n
 *    SEARCH_SYNC   search next sync element as errors while decoding occured
 ***********************************************************************
 */
int intrapred16x16(Macroblock *currMB,  //!< Current Macroblock
                   ColorPlane pl,       //!< Current colorplane (for 4:4:4)                         
                   int predmode)        //!< prediction mode
{
  if (currMB->p_Slice->mb_aff_frame_flag)
  {
    return intrapred_16x16_mbaff(currMB, pl, predmode);
  }
  else
  {
    return intrapred_16x16_normal(currMB, pl, predmode);
  }
}

