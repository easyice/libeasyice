
/*!
 ***********************************************************************
 *  \mainpage
 *     This is the H.264/AVC decoder reference software. For detailed documentation
 *     see the comments in each file.
 *
 *     The JM software web site is located at:
 *     http://iphome.hhi.de/suehring/tml
 *
 *     For bug reporting and known issues see:
 *     https://ipbt.hhi.de
 *
 *  \author
 *     The main contributors are listed in contributors.h
 *
 *  \version
 *     JM 17.2 (FRExt)
 *
 *  \note
 *     tags are used for document system "doxygen"
 *     available at http://www.doxygen.org
 */
/*!
 *  \file
 *     ldecod.c
 *  \brief
 *     H.264/AVC reference decoder project main()
 *  \author
 *     Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Inge Lille-Langøy       <inge.lille-langoy@telenor.com>
 *     - Rickard Sjoberg         <rickard.sjoberg@era.ericsson.se>
 *     - Stephan Wenger          <stewe@cs.tu-berlin.de>
 *     - Jani Lainema            <jani.lainema@nokia.com>
 *     - Sebastian Purreiter     <sebastian.purreiter@mch.siemens.de>
 *     - Byeong-Moon Jeon        <jeonbm@lge.com>
 *     - Gabi Blaettermann
 *     - Ye-Kui Wang             <wyk@ieee.org>
 *     - Valeri George           <george@hhi.de>
 *     - Karsten Suehring        <suehring@hhi.de>
 *
 ***********************************************************************
 */

#include "contributors.h"

//#include <sys/stat.h>

#include "global.h"
#include "annexb.h"
#include "image.h"
#include "memalloc.h"
#include "mc_prediction.h"
#include "mbuffer.h"
#include "leaky_bucket.h"
#include "fmo.h"
#include "output.h"
#include "cabac.h"
#include "parset.h"
#include "sei.h"
#include "erc_api.h"
#include "quant.h"
#include "block.h"
#include "nalu.h"
#include "img_io.h"
#include "loopfilter.h"

#include "h264decoder.h"

#define LOGFILE     "log.dec"
#define DATADECFILE "dataDec.txt"
#define TRACEFILE   "trace_dec.txt"

// Decoder definition. This should be the only global variable in the entire
// software. Global variables should be avoided.
//DecoderParams  *p_Dec;
char errortext[ET_SIZE];
//BlockPos *PicPos;

// Prototypes of static functions
static void Report      (VideoParameters *p_Vid);
static void init        (VideoParameters *p_Vid);
//static void free_slice  (Slice *currSlice);

void init_frext(VideoParameters *p_Vid);

/*!
 ************************************************************************
 * \brief
 *    Error handling procedure. Print error message to stderr and exit
 *    with supplied code.
 * \param text
 *    Error message
 * \param code
 *    Exit code
 ************************************************************************
 */
void error(char *text, int code)
{
  fprintf(stderr, "%s\n", text);
//#if (MVC_EXTENSION_ENABLE)
//  flush_dpb(p_Dec->p_Vid->p_Dpb, -1);
//#else
//  flush_dpb(p_Dec->p_Vid->p_Dpb);
//#endif
//  exit(code);
}

static void reset_dpb( VideoParameters *p_Vid, DecodedPictureBuffer *p_Dpb )
{
  p_Dpb->p_Vid = p_Vid;
  p_Dpb->init_done = 0;
}
/*!
 ***********************************************************************
 * \brief
 *    Allocate the Video Parameters structure
 * \par  Output:
 *    Video Parameters VideoParameters *p_Vid
 ***********************************************************************
 */
// static void alloc_video_params( VideoParameters **p_Vid)
// {
//   int i;
//   if ((*p_Vid   =  (VideoParameters *) calloc(1, sizeof(VideoParameters)))==NULL) 
//     no_mem_exit("alloc_video_params: p_Vid");

//   if (((*p_Vid)->old_slice = (OldSliceParams *) calloc(1, sizeof(OldSliceParams)))==NULL) 
//     no_mem_exit("alloc_video_params: p_Vid->old_slice");

//   if (((*p_Vid)->snr =  (SNRParameters *)calloc(1, sizeof(SNRParameters)))==NULL) 
//     no_mem_exit("alloc_video_params: p_Vid->snr");  

//   // Allocate legacy dpb
//   if (((*p_Vid)->p_Dpb_legacy =  (DecodedPictureBuffer*)calloc(1, sizeof(DecodedPictureBuffer)))==NULL) 
//     no_mem_exit("alloc_video_params: p_Vid->p_Dpb_legacy");  

//   // Now set appropriate pointer
//   (*p_Vid)->p_Dpb = (*p_Vid)->p_Dpb_legacy;
//   reset_dpb(*p_Vid, (*p_Vid)->p_Dpb);

//   // Allocate new dpb buffer
//   for (i = 0; i < 2; i++)
//   {
//     if (((*p_Vid)->p_Dpb_layer[i] =  (DecodedPictureBuffer*)calloc(1, sizeof(DecodedPictureBuffer)))==NULL) 
//       no_mem_exit("alloc_video_params: p_Vid->p_Dpb_layer[i]");  
//     reset_dpb(*p_Vid, (*p_Vid)->p_Dpb_layer[i]);
//   }
  
//   (*p_Vid)->global_init_done = 0;

// #if (ENABLE_OUTPUT_TONEMAPPING)  
//   if (((*p_Vid)->seiToneMapping =  (ToneMappingSEI*)calloc(1, sizeof(ToneMappingSEI)))==NULL) 
//     no_mem_exit("alloc_video_params: (*p_Vid)->seiToneMapping");  
// #endif

//   if(((*p_Vid)->ppSliceList = (Slice **) calloc(MAX_NUM_DECSLICES, sizeof(Slice *))) == NULL)
//   {
//     no_mem_exit("alloc_video_params: p_Vid->ppSliceList");
//   }
//   (*p_Vid)->iNumOfSlicesAllocated = MAX_NUM_DECSLICES;
//   //(*p_Vid)->currentSlice = NULL;
//   (*p_Vid)->pNextSlice = NULL;
//   (*p_Vid)->nalu = AllocNALU(MAX_CODED_FRAME_SIZE);
//   (*p_Vid)->pDecOuputPic = (DecodedPicList *)calloc(1, sizeof(DecodedPicList));
//   (*p_Vid)->pNextPPS = AllocPPS();
// }


/*!
 ***********************************************************************
 * \brief
 *    Allocate the Input structure
 * \par  Output:
 *    Input Parameters InputParameters *p_Vid
 ***********************************************************************
 */
static void alloc_params( InputParameters **p_Inp )
{
  if ((*p_Inp = (InputParameters *) calloc(1, sizeof(InputParameters)))==NULL) 
    no_mem_exit("alloc_params: p_Inp");
}

  /*!
 ***********************************************************************
 * \brief
 *    Allocate the Decoder Structure
 * \par  Output:
 *    Decoder Parameters
 ***********************************************************************
 */
//static int alloc_decoder( DecoderParams **p_Dec)
//{
//  if ((*p_Dec = (DecoderParams *) calloc(1, sizeof(DecoderParams)))==NULL) 
//  {
//    fprintf(stderr, "alloc_decoder: p_Dec\n");
//    return -1;
//  }
//
//  alloc_video_params(&((*p_Dec)->p_Vid));
//  alloc_params(&((*p_Dec)->p_Inp));
//  (*p_Dec)->p_Vid->p_Inp = (*p_Dec)->p_Inp;
//  (*p_Dec)->p_trace = NULL;
//  (*p_Dec)->bufferSize = 0;
//  (*p_Dec)->bitcounter = 0;
//  return 0;
//}

/*!
 ***********************************************************************
 * \brief
 *    Free the Image structure
 * \par  Input:
 *    Image Parameters VideoParameters *p_Vid
 ***********************************************************************
 */
// static void free_img( VideoParameters *p_Vid)
// {
//   int i;
//   //free_mem3Dint(p_Vid->fcf    ); 
//   if (p_Vid != NULL)
//   {
//     free_annex_b (p_Vid);
// #if (ENABLE_OUTPUT_TONEMAPPING)  
//     if (p_Vid->seiToneMapping != NULL)
//     {
//       free (p_Vid->seiToneMapping);
//       p_Vid->seiToneMapping = NULL;
//     }
// #endif

//     if (p_Vid->bitsfile != NULL)
//     {
//       free (p_Vid->bitsfile);
//       p_Vid->bitsfile = NULL;
//     }

//     if (p_Vid->p_Dpb_legacy != NULL)
//     {
//       free (p_Vid->p_Dpb_legacy);
//       p_Vid->p_Dpb_legacy = NULL;
//     }

//     // Free new dpb layers
//     for (i = 0; i < 2; i++)
//     {
//       if (p_Vid->p_Dpb_layer[i] != NULL)
//       {
//         free (p_Vid->p_Dpb_layer[i]);
//         p_Vid->p_Dpb_layer[i] = NULL;
//       }
//     }

//     p_Vid->p_Dpb = NULL;

//     if (p_Vid->snr != NULL)
//     {
//       free (p_Vid->snr);
//       p_Vid->snr = NULL;
//     }
//     if (p_Vid->old_slice != NULL)
//     {
//       free (p_Vid->old_slice);
//       p_Vid->old_slice = NULL;
//     }

//     if(p_Vid->pNextSlice)
//     {
//       free_slice(p_Vid->pNextSlice);
//       p_Vid->pNextSlice=NULL;
//     }
//     if(p_Vid->ppSliceList)
//     {
//       int i;
//       for(i=0; i<p_Vid->iNumOfSlicesAllocated; i++)
//         if(p_Vid->ppSliceList[i])
//           free_slice(p_Vid->ppSliceList[i]);
//       free(p_Vid->ppSliceList);
//     }
//     if(p_Vid->nalu)
//     {
//       FreeNALU(p_Vid->nalu);
//       p_Vid->nalu=NULL;
//     }
//     //free memory;
//     FreeDecPicList(p_Vid->pDecOuputPic);
//   if(p_Vid->pNextPPS)
//   {
//     FreePPS(p_Vid->pNextPPS);
//     p_Vid->pNextPPS = NULL;
//   }

//     free (p_Vid);
//     p_Vid = NULL;
//   }
// }

void FreeDecPicList(DecodedPicList *pDecPicList)
{
  while(pDecPicList)
  {
    DecodedPicList *pPicNext = pDecPicList->pNext;
    if(pDecPicList->pY)
    {
      free(pDecPicList->pY);
      pDecPicList->pY=NULL;
      pDecPicList->pU=NULL;
      pDecPicList->pV=NULL;
    }
    free(pDecPicList);
    pDecPicList = pPicNext;
  }
}

/*!
 ***********************************************************************
 * \brief
 *    Initilize some arrays
 ***********************************************************************
 */
// static void init(VideoParameters *p_Vid)  //!< video parameters
// {
//   //int i;
//   InputParameters *p_Inp = p_Vid->p_Inp;
//   p_Vid->oldFrameSizeInMbs = (unsigned int) -1;

//   p_Vid->imgY_ref  = NULL;
//   p_Vid->imgUV_ref = NULL;

//   p_Vid->recovery_point = 0;
//   p_Vid->recovery_point_found = 0;
//   p_Vid->recovery_poc = 0x7fffffff; /* set to a max value */

//   p_Vid->idr_psnr_number = p_Inp->ref_offset;
//   p_Vid->psnr_number=0;

//   p_Vid->number = 0;
//   p_Vid->type = I_SLICE;

//   //p_Vid->dec_ref_pic_marking_buffer = NULL;

//   p_Vid->g_nFrame = 0;
//   // B pictures
//   p_Vid->Bframe_ctr = p_Vid->snr->frame_ctr = 0;

//   // time for total decoding session
//   p_Vid->tot_time = 0;

//   p_Vid->dec_picture = NULL;
//   /*// reference flag initialization
//   for(i=0;i<17;++i)
//   {
//     p_Vid->ref_flag[i] = 1;
//   }*/

//   p_Vid->MbToSliceGroupMap = NULL;
//   p_Vid->MapUnitToSliceGroupMap = NULL;

//   p_Vid->LastAccessUnitExists  = 0;
//   p_Vid->NALUCount = 0;


//   p_Vid->out_buffer = NULL;
//   p_Vid->pending_output = NULL;
//   p_Vid->pending_output_state = FRAME;
//   p_Vid->recovery_flag = 0;


// #if (ENABLE_OUTPUT_TONEMAPPING)
//   init_tone_mapping_sei(p_Vid->seiToneMapping);
// #endif

// #if (MVC_EXTENSION_ENABLE)
//   p_Vid->last_pic_width_in_mbs_minus1 = 0;
//   p_Vid->last_pic_height_in_map_units_minus1 = 0;
//   p_Vid->last_max_dec_frame_buffering = 0;
// #endif

//   p_Vid->newframe = 0;
//   p_Vid->previous_frame_num = 0;

//   p_Vid->iLumaPadX = MCBUF_LUMA_PAD_X;
//   p_Vid->iLumaPadY = MCBUF_LUMA_PAD_Y;
//   p_Vid->iChromaPadX = MCBUF_CHROMA_PAD_X;
//   p_Vid->iChromaPadY = MCBUF_CHROMA_PAD_Y;

//   p_Vid->iPostProcess = 0;
//   p_Vid->bDeblockEnable = 0x3;
// }

/*!
 ***********************************************************************
 * \brief
 *    Initialize FREXT variables
 ***********************************************************************
 */
void init_frext(VideoParameters *p_Vid)  //!< video parameters
{
  //pel bitdepth init
  p_Vid->bitdepth_luma_qp_scale   = 6 * (p_Vid->bitdepth_luma - 8);

  if(p_Vid->bitdepth_luma > p_Vid->bitdepth_chroma || p_Vid->active_sps->chroma_format_idc == YUV400)
    p_Vid->pic_unit_bitsize_on_disk = (p_Vid->bitdepth_luma > 8)? 16:8;
  else
    p_Vid->pic_unit_bitsize_on_disk = (p_Vid->bitdepth_chroma > 8)? 16:8;
  p_Vid->dc_pred_value_comp[0]    = 1<<(p_Vid->bitdepth_luma - 1);
  p_Vid->max_pel_value_comp[0] = (1<<p_Vid->bitdepth_luma) - 1;
  p_Vid->mb_size[0][0] = p_Vid->mb_size[0][1] = MB_BLOCK_SIZE;

  if (p_Vid->active_sps->chroma_format_idc != YUV400)
  {
    //for chrominance part
    p_Vid->bitdepth_chroma_qp_scale = 6 * (p_Vid->bitdepth_chroma - 8);
    p_Vid->dc_pred_value_comp[1]    = (1 << (p_Vid->bitdepth_chroma - 1));
    p_Vid->dc_pred_value_comp[2]    = p_Vid->dc_pred_value_comp[1];
    p_Vid->max_pel_value_comp[1]    = (1 << p_Vid->bitdepth_chroma) - 1;
    p_Vid->max_pel_value_comp[2]    = (1 << p_Vid->bitdepth_chroma) - 1;
    p_Vid->num_blk8x8_uv = (1 << p_Vid->active_sps->chroma_format_idc) & (~(0x1));
    p_Vid->num_uv_blocks = (p_Vid->num_blk8x8_uv >> 1);
    p_Vid->num_cdc_coeff = (p_Vid->num_blk8x8_uv << 1);
    p_Vid->mb_size[1][0] = p_Vid->mb_size[2][0] = p_Vid->mb_cr_size_x  = (p_Vid->active_sps->chroma_format_idc==YUV420 || p_Vid->active_sps->chroma_format_idc==YUV422)?  8 : 16;
    p_Vid->mb_size[1][1] = p_Vid->mb_size[2][1] = p_Vid->mb_cr_size_y  = (p_Vid->active_sps->chroma_format_idc==YUV444 || p_Vid->active_sps->chroma_format_idc==YUV422)? 16 :  8;

    p_Vid->subpel_x    = p_Vid->mb_cr_size_x == 8 ? 7 : 3;
    p_Vid->subpel_y    = p_Vid->mb_cr_size_y == 8 ? 7 : 3;
    p_Vid->shiftpel_x  = p_Vid->mb_cr_size_x == 8 ? 3 : 2;
    p_Vid->shiftpel_y  = p_Vid->mb_cr_size_y == 8 ? 3 : 2;
    p_Vid->total_scale = p_Vid->shiftpel_x + p_Vid->shiftpel_y;
  }
  else
  {
    p_Vid->bitdepth_chroma_qp_scale = 0;
    p_Vid->max_pel_value_comp[1] = 0;
    p_Vid->max_pel_value_comp[2] = 0;
    p_Vid->num_blk8x8_uv = 0;
    p_Vid->num_uv_blocks = 0;
    p_Vid->num_cdc_coeff = 0;
    p_Vid->mb_size[1][0] = p_Vid->mb_size[2][0] = p_Vid->mb_cr_size_x  = 0;
    p_Vid->mb_size[1][1] = p_Vid->mb_size[2][1] = p_Vid->mb_cr_size_y  = 0;
    p_Vid->subpel_x      = 0;
    p_Vid->subpel_y      = 0;
    p_Vid->shiftpel_x    = 0;
    p_Vid->shiftpel_y    = 0;
    p_Vid->total_scale   = 0;
  }

  p_Vid->mb_size_blk[0][0] = p_Vid->mb_size_blk[0][1] = p_Vid->mb_size[0][0] >> 2;
  p_Vid->mb_size_blk[1][0] = p_Vid->mb_size_blk[2][0] = p_Vid->mb_size[1][0] >> 2;
  p_Vid->mb_size_blk[1][1] = p_Vid->mb_size_blk[2][1] = p_Vid->mb_size[1][1] >> 2;

  p_Vid->mb_size_shift[0][0] = p_Vid->mb_size_shift[0][1] = CeilLog2_sf (p_Vid->mb_size[0][0]);
  p_Vid->mb_size_shift[1][0] = p_Vid->mb_size_shift[2][0] = CeilLog2_sf (p_Vid->mb_size[1][0]);
  p_Vid->mb_size_shift[1][1] = p_Vid->mb_size_shift[2][1] = CeilLog2_sf (p_Vid->mb_size[1][1]);
}

/*!
 ************************************************************************
 * \brief
 *    Reports the gathered information to appropriate outputs
 *
 * \par Input:
 *    InputParameters *p_Inp,
 *    VideoParameters *p_Vid,
 *    struct snr_par *stat
 *
 * \par Output:
 *    None
 ************************************************************************
 */
// static void Report(VideoParameters *p_Vid)
// {
//   pic_parameter_set_rbsp_t *active_pps = p_Vid->active_pps;
//   InputParameters *p_Inp = p_Vid->p_Inp;
//   SNRParameters   *snr   = p_Vid->snr;
// #define OUTSTRING_SIZE 255
//   char string[OUTSTRING_SIZE];
//   FILE *p_log;
//   static const char yuv_formats[4][4]= { {"400"}, {"420"}, {"422"}, {"444"} };
// #ifndef WIN32
//   time_t  now;
//   struct tm *l_time;
// #else
//   char timebuf[128];
// #endif

//   // normalize time
//   p_Vid->tot_time  = timenorm(p_Vid->tot_time);

//   if (p_Inp->silent == FALSE)
//   {
//     fprintf(stdout,"-------------------- Average SNR all frames ------------------------------\n");
//     fprintf(stdout," SNR Y(dB)           : %5.2f\n",snr->snra[0]);
//     fprintf(stdout," SNR U(dB)           : %5.2f\n",snr->snra[1]);
//     fprintf(stdout," SNR V(dB)           : %5.2f\n",snr->snra[2]);
//     fprintf(stdout," Total decoding time : %.3f sec (%.3f fps)[%d frm/%" FORMAT_OFF_T " ms]\n",p_Vid->tot_time*0.001,(snr->frame_ctr ) * 1000.0 / p_Vid->tot_time, snr->frame_ctr, p_Vid->tot_time);
//     fprintf(stdout,"--------------------------------------------------------------------------\n");
//     fprintf(stdout," Exit JM %s decoder, ver %s ",JM, VERSION);
//     fprintf(stdout,"\n");
//   }
//   else
//   {
//     fprintf(stdout,"\n----------------------- Decoding Completed -------------------------------\n");
//     fprintf(stdout," Total decoding time : %.3f sec (%.3f fps)[%d frm/%" FORMAT_OFF_T "  ms]\n",p_Vid->tot_time*0.001, (snr->frame_ctr) * 1000.0 / p_Vid->tot_time, snr->frame_ctr, p_Vid->tot_time);
//     fprintf(stdout,"--------------------------------------------------------------------------\n");
//     fprintf(stdout," Exit JM %s decoder, ver %s ",JM, VERSION);
//     fprintf(stdout,"\n");
//   }

//   // write to log file
//   fprintf(stdout," Output status file                     : %s \n",LOGFILE);
//   snprintf(string, OUTSTRING_SIZE, "%s", LOGFILE);

//   if ((p_log=fopen(string,"r"))==0)                    // check if file exist
//   {
//     if ((p_log=fopen(string,"a"))==0)
//     {
//       snprintf(errortext, ET_SIZE, "Error open file %s for appending",string);
//       error(errortext, 500);
//     }
//     else                                              // Create header to new file
//     {
//       fprintf(p_log," -------------------------------------------------------------------------------------------------------------------\n");
//       fprintf(p_log,"|  Decoder statistics. This file is made first time, later runs are appended               |\n");
//       fprintf(p_log," ------------------------------------------------------------------------------------------------------------------- \n");
//       fprintf(p_log,"|   ver  | Date  | Time  |    Sequence        |#Img| Format  | YUV |Coding|SNRY 1|SNRU 1|SNRV 1|SNRY N|SNRU N|SNRV N|\n");
//       fprintf(p_log," -------------------------------------------------------------------------------------------------------------------\n");
//     }
//   }
//   else
//   {
//     fclose(p_log);
//     p_log=fopen(string,"a");                    // File exist,just open for appending
//   }

//   fprintf(p_log,"|%s/%-4s", VERSION, EXT_VERSION);

// #ifdef WIN32
//   _strdate( timebuf );
//   fprintf(p_log,"| %1.5s |",timebuf );

//   _strtime( timebuf);
//   fprintf(p_log," % 1.5s |",timebuf);
// #else
//   now = time ((time_t *) NULL); // Get the system time and put it into 'now' as 'calender time'
//   time (&now);
//   l_time = localtime (&now);
//   strftime (string, sizeof string, "%d-%b-%Y", l_time);
//   fprintf(p_log,"| %1.5s |",string );

//   strftime (string, sizeof string, "%H:%M:%S", l_time);
//   fprintf(p_log,"| %1.5s |",string );
// #endif

//   fprintf(p_log,"%20.20s|",p_Inp->infile);

//   fprintf(p_log,"%3d |",p_Vid->number);
//   fprintf(p_log,"%4dx%-4d|", p_Vid->width, p_Vid->height);
//   fprintf(p_log," %s |", &(yuv_formats[p_Vid->yuv_format][0]));

//   if (active_pps)
//   {
//     if (active_pps->entropy_coding_mode_flag == CAVLC)
//       fprintf(p_log," CAVLC|");
//     else
//       fprintf(p_log," CABAC|");
//   }

//   fprintf(p_log,"%6.3f|",snr->snr1[0]);
//   fprintf(p_log,"%6.3f|",snr->snr1[1]);
//   fprintf(p_log,"%6.3f|",snr->snr1[2]);
//   fprintf(p_log,"%6.3f|",snr->snra[0]);
//   fprintf(p_log,"%6.3f|",snr->snra[1]);
//   fprintf(p_log,"%6.3f|",snr->snra[2]);
//   fprintf(p_log,"\n");
//   fclose(p_log);

//   snprintf(string, OUTSTRING_SIZE,"%s", DATADECFILE);
//   p_log=fopen(string,"a");

//   if(p_Vid->Bframe_ctr != 0) // B picture used
//   {
//     fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5d "
//       "%2.2f %2.2f %2.2f %5d "
//       "%2.2f %2.2f %2.2f %5d %.3f\n",
//       p_Vid->number, 0, p_Vid->ppSliceList[0]->qp,
//       snr->snr1[0],
//       snr->snr1[1],
//       snr->snr1[2],
//       0,
//       0.0,
//       0.0,
//       0.0,
//       0,
//       snr->snra[0],
//       snr->snra[1],
//       snr->snra[2],
//       0,
//       (double)0.001*p_Vid->tot_time/(p_Vid->number + p_Vid->Bframe_ctr - 1));
//   }
//   else
//   {
//     fprintf(p_log, "%3d %2d %2d %2.2f %2.2f %2.2f %5d "
//       "%2.2f %2.2f %2.2f %5d "
//       "%2.2f %2.2f %2.2f %5d %.3f\n",
//       p_Vid->number, 0, p_Vid->ppSliceList[0]? p_Vid->ppSliceList[0]->qp: 0,
//       snr->snr1[0],
//       snr->snr1[1],
//       snr->snr1[2],
//       0,
//       0.0,
//       0.0,
//       0.0,
//       0,
//       snr->snra[0],
//       snr->snra[1],
//       snr->snra[2],
//       0,
//       p_Vid->number ? ((double)0.001*p_Vid->tot_time/p_Vid->number) : 0.0);
//   }
//   fclose(p_log);
// }

/*!
 ************************************************************************
 * \brief
 *    Allocates a stand-alone partition structure.  Structure should
 *    be freed by FreePartition();
 *    data structures
 *
 * \par Input:
 *    n: number of partitions in the array
 * \par return
 *    pointer to DataPartition Structure, zero-initialized
 ************************************************************************
 */

DataPartition *AllocPartition(int n)
{
  DataPartition *partArr, *dataPart;
  int i;

  partArr = (DataPartition *) calloc(n, sizeof(DataPartition));
  if (partArr == NULL)
  {
    snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Data Partition failed");
    error(errortext, 100);
  }

  for (i=0; i<n; ++i) // loop over all data partitions
  {
    dataPart = &(partArr[i]);
    dataPart->bitstream = (Bitstream *) calloc(1, sizeof(Bitstream));
    if (dataPart->bitstream == NULL)
    {
      snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for Bitstream failed");
      error(errortext, 100);
    }
    dataPart->bitstream->streamBuffer = (byte *) calloc(MAX_CODED_FRAME_SIZE, sizeof(byte));
    if (dataPart->bitstream->streamBuffer == NULL)
    {
      snprintf(errortext, ET_SIZE, "AllocPartition: Memory allocation for streamBuffer failed");
      error(errortext, 100);
    }
  }
  return partArr;
}




/*!
 ************************************************************************
 * \brief
 *    Frees a partition structure (array).
 *
 * \par Input:
 *    Partition to be freed, size of partition Array (Number of Partitions)
 *
 * \par return
 *    None
 *
 * \note
 *    n must be the same as for the corresponding call of AllocPartition
 ************************************************************************
 */
void FreePartition (DataPartition *dp, int n)
{
  int i;

  assert (dp != NULL);
  assert (dp->bitstream != NULL);
  assert (dp->bitstream->streamBuffer != NULL);
  for (i=0; i<n; ++i)
  {
    free (dp[i].bitstream->streamBuffer);
    free (dp[i].bitstream);
  }
  free (dp);
}


/*!
 ************************************************************************
 * \brief
 *    Allocates the slice structure along with its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters InputParameters *p_Inp,  VideoParameters *p_Vid
 ************************************************************************
 */
Slice *malloc_slice(InputParameters *p_Inp, VideoParameters *p_Vid)
{
  int i, j, memory_size = 0;
  Slice *currSlice;

  currSlice = (Slice *) calloc(1, sizeof(Slice));
  if ( currSlice  == NULL)
  {
    /*snprintf(errortext, ET_SIZE, "Memory allocation for Slice datastruct in NAL-mode %d failed", p_Inp->FileFormat);
    error(errortext,100);*/
  }

  // create all context models
  currSlice->mot_ctx = create_contexts_MotionInfo();
  currSlice->tex_ctx = create_contexts_TextureInfo();

  currSlice->max_part_nr = 3;  //! assume data partitioning (worst case) for the following mallocs()
  currSlice->partArr = AllocPartition(currSlice->max_part_nr);

  memory_size += get_mem3Dint(&(currSlice->wp_weight), 2, MAX_REFERENCE_PICTURES, 3);
  memory_size += get_mem3Dint(&(currSlice->wp_offset), 6, MAX_REFERENCE_PICTURES, 3);
  memory_size += get_mem4Dint(&(currSlice->wbp_weight), 6, MAX_REFERENCE_PICTURES, MAX_REFERENCE_PICTURES, 3);

  memory_size += get_mem3Dpel(&(currSlice->mb_pred), MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  memory_size += get_mem3Dpel(&(currSlice->mb_rec ), MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  memory_size += get_mem3Dint(&(currSlice->mb_rres), MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  memory_size += get_mem3Dint(&(currSlice->cof    ), MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);
  //  memory_size += get_mem3Dint(&(currSlice->fcf    ), MAX_PLANE, MB_BLOCK_SIZE, MB_BLOCK_SIZE);

  allocate_pred_mem(currSlice);

#if (MVC_EXTENSION_ENABLE)
  currSlice->view_id = MVC_INIT_VIEW_ID;
  currSlice->inter_view_flag = 0;
  currSlice->anchor_pic_flag = 0;
#endif
  // reference flag initialization
  for(i=0;i<17;++i)
  {
    currSlice->ref_flag[i] = 1;
  }
  for (i = 0; i < 6; i++)
  {
    currSlice->listX[i] = calloc(MAX_LIST_SIZE, sizeof (StorablePicture*)); // +1 for reordering
    if (NULL==currSlice->listX[i])
      no_mem_exit("malloc_slice: currSlice->listX[i]");
  }
  for (j = 0; j < 6; j++)
  {
    for (i = 0; i < MAX_LIST_SIZE; i++)
    {
      currSlice->listX[j][i] = NULL;
    }
    currSlice->listXsize[j]=0;
  }

  return currSlice;
}


/*!
 ************************************************************************
 * \brief
 *    Memory frees of the Slice structure and of its dependent
 *    data structures
 *
 * \par Input:
 *    Input Parameters Slice *currSlice
 ************************************************************************
 */
void free_slice(Slice *currSlice)
{
  int i;
  free_pred_mem(currSlice);

  free_mem3Dint(currSlice->cof    );
  free_mem3Dint(currSlice->mb_rres);
  free_mem3Dpel(currSlice->mb_rec );
  free_mem3Dpel(currSlice->mb_pred);


  free_mem3Dint(currSlice->wp_weight );
  free_mem3Dint(currSlice->wp_offset );
  free_mem4Dint(currSlice->wbp_weight);

  FreePartition (currSlice->partArr, 3);

  //if (1)
  {
    // delete all context models
    delete_contexts_MotionInfo(currSlice->mot_ctx);
    delete_contexts_TextureInfo(currSlice->tex_ctx);
  }

  for (i=0; i<6; i++)
  {
    if (currSlice->listX[i])
    {
      free (currSlice->listX[i]);
      currSlice->listX[i] = NULL;
    }
  }
  while (currSlice->dec_ref_pic_marking_buffer)
  {
    DecRefPicMarking_t *tmp_drpm=currSlice->dec_ref_pic_marking_buffer;
    currSlice->dec_ref_pic_marking_buffer=tmp_drpm->Next;
    free (tmp_drpm);
  }

  free(currSlice);
  currSlice = NULL;
}

/*!
 ************************************************************************
 * \brief
 *    Dynamic memory allocation of frame size related global buffers
 *    buffers are defined in global.h, allocated memory must be freed in
 *    void free_global_buffers()
 *
 *  \par Input:
 *    Input Parameters VideoParameters *p_Vid
 *
 *  \par Output:
 *     Number of allocated bytes
 ***********************************************************************
 */
//int init_global_buffers(VideoParameters *p_Vid)
//{
//  int memory_size=0;
//  int i;
//
//  if (p_Vid->global_init_done)
//  {
//    free_global_buffers(p_Vid);
//  }
//
//  // allocate memory for reference frame in find_snr
//  memory_size += get_mem2Dpel(&p_Vid->imgY_ref, p_Vid->height, p_Vid->width);
//
//  if (p_Vid->active_sps->chroma_format_idc != YUV400)
//    memory_size += get_mem3Dpel(&p_Vid->imgUV_ref, 2, p_Vid->height_cr, p_Vid->width_cr);
//  else
//    p_Vid->imgUV_ref=NULL;
//
//  // allocate memory in structure p_Vid
//  if( (p_Vid->separate_colour_plane_flag != 0) )
//  {
//    for( i=0; i<MAX_PLANE; ++i )
//    {
//      if(((p_Vid->mb_data_JV[i]) = (Macroblock *) calloc(p_Vid->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
//        no_mem_exit("init_global_buffers: p_Vid->mb_data");
//    }
//    p_Vid->mb_data = NULL;
//  }
//  else
//  {
//    if(((p_Vid->mb_data) = (Macroblock *) calloc(p_Vid->FrameSizeInMbs, sizeof(Macroblock))) == NULL)
//      no_mem_exit("init_global_buffers: p_Vid->mb_data");
//  }
//  if( (p_Vid->separate_colour_plane_flag != 0) )
//  {
//    for( i=0; i<MAX_PLANE; ++i )
//    {
//      if(((p_Vid->intra_block_JV[i]) = (char*) calloc(p_Vid->FrameSizeInMbs, sizeof(char))) == NULL)
//        no_mem_exit("init_global_buffers: p_Vid->intra_block_JV");
//    }
//    p_Vid->intra_block = NULL;
//  }
//  else
//  {
//    if(((p_Vid->intra_block) = (char*) calloc(p_Vid->FrameSizeInMbs, sizeof(char))) == NULL)
//      no_mem_exit("init_global_buffers: p_Vid->intra_block");
//  }
//
//
//  //memory_size += get_mem2Dint(&PicPos,p_Vid->FrameSizeInMbs + 1,2);  //! Helper array to access macroblock positions. We add 1 to also consider last MB.
//  if(((PicPos) = (BlockPos*) calloc(p_Vid->FrameSizeInMbs + 1, sizeof(BlockPos))) == NULL)
//    no_mem_exit("init_global_buffers: PicPos");
//
//
//  for (i = 0; i < (int) p_Vid->FrameSizeInMbs + 1;++i)
//  {
//    PicPos[i].x = (short) (i % p_Vid->PicWidthInMbs);
//    PicPos[i].y = (short) (i / p_Vid->PicWidthInMbs);
//  }
//
//  if( (p_Vid->separate_colour_plane_flag != 0) )
//  {
//    for( i=0; i<MAX_PLANE; ++i )
//    {
//      get_mem2D(&(p_Vid->ipredmode_JV[i]), 4*p_Vid->FrameHeightInMbs, 4*p_Vid->PicWidthInMbs);
//    }
//    p_Vid->ipredmode = NULL;
//  }
//  else
//   memory_size += get_mem2D(&(p_Vid->ipredmode), 4*p_Vid->FrameHeightInMbs, 4*p_Vid->PicWidthInMbs);
//
//  // CAVLC mem
//  memory_size += get_mem4D(&(p_Vid->nz_coeff), p_Vid->FrameSizeInMbs, 3, BLOCK_SIZE, BLOCK_SIZE);
//  if( (p_Vid->separate_colour_plane_flag != 0) )
//  {
//    for( i=0; i<MAX_PLANE; ++i )
//    {
//      get_mem2Dint(&(p_Vid->siblock_JV[i]), p_Vid->FrameHeightInMbs, p_Vid->PicWidthInMbs);
//      if(p_Vid->siblock_JV[i]== NULL)
//        no_mem_exit("init_global_buffers: p_Vid->siblock_JV");
//    }
//    p_Vid->siblock = NULL;
//  }
//  else
//  {
//    memory_size += get_mem2Dint(&(p_Vid->siblock), p_Vid->FrameHeightInMbs, p_Vid->PicWidthInMbs);
//  }
//  init_qp_process(p_Vid);
//
//  p_Vid->global_init_done = 1;
//
//  p_Vid->oldFrameSizeInMbs = p_Vid->FrameSizeInMbs;
//  return (memory_size);
//}

/*!
 ************************************************************************
 * \brief
 *    Free allocated memory of frame size related global buffers
 *    buffers are defined in global.h, allocated memory is allocated in
 *    int init_global_buffers()
 *
 * \par Input:
 *    Input Parameters VideoParameters *p_Vid
 *
 * \par Output:
 *    none
 *
 ************************************************************************
 */
//void free_global_buffers(VideoParameters *p_Vid)
//{  
//  free_mem2Dpel (p_Vid->imgY_ref);
//
//  if (p_Vid->imgUV_ref)
//    free_mem3Dpel (p_Vid->imgUV_ref);
//
//  // CAVLC free mem
//  free_mem4D(p_Vid->nz_coeff);
//
//  // free mem, allocated for structure p_Vid
//  if( (p_Vid->separate_colour_plane_flag != 0) )
//  {
//    int i;
//    for(i=0; i<MAX_PLANE; i++)
//    {
//      free(p_Vid->mb_data_JV[i]);
//      p_Vid->mb_data_JV[i] = NULL;
//      free_mem2Dint(p_Vid->siblock_JV[i]);
//      p_Vid->siblock_JV[i] = NULL;
//      free_mem2D(p_Vid->ipredmode_JV[i]);
//      p_Vid->ipredmode_JV[i] = NULL;
//      free (p_Vid->intra_block_JV[i]);
//      p_Vid->intra_block_JV[i] = NULL;
//    }   
//  }
//  else
//  {
//    if (p_Vid->mb_data != NULL)
//    {
//      free(p_Vid->mb_data);
//      p_Vid->mb_data = NULL;
//    }
//    if(p_Vid->siblock)
//    {
//      free_mem2Dint(p_Vid->siblock);
//      p_Vid->siblock = NULL;
//    }
//    if(p_Vid->ipredmode)
//    {
//      free_mem2D(p_Vid->ipredmode);
//      p_Vid->ipredmode = NULL;
//    }
//    if(p_Vid->intra_block)
//    {
//      free (p_Vid->intra_block);
//      p_Vid->intra_block = NULL;
//    }
//  }
//  if(PicPos)
//  {
//    free(PicPos);
//    PicPos=NULL;
//  }
//
//  free_qp_matrices(p_Vid);
//
//  p_Vid->global_init_done = 0;
//}

void report_stats_on_error(void)
{
  //free_encoder_memory(p_Vid);
  exit (-1);
}

void ClearDecPicList(VideoParameters *p_Vid)
{
  DecodedPicList *pPic = p_Vid->pDecOuputPic, *pPrior = NULL;
  //find the head first;
  while(pPic && !pPic->bValid)
  {
    pPrior = pPic;
    pPic = pPic->pNext;
  }

  if(pPic && (pPic != p_Vid->pDecOuputPic))
  {
    //move all nodes before pPic to the end;
    DecodedPicList *pPicTail = pPic;
    while(pPicTail->pNext)
      pPicTail = pPicTail->pNext;

    pPicTail->pNext = p_Vid->pDecOuputPic;
    p_Vid->pDecOuputPic = pPic;
    pPrior->pNext = NULL;
  }
}

DecodedPicList *GetOneAvailDecPicFromList(DecodedPicList *pDecPicList, int b3D)
{
  DecodedPicList *pPic = pDecPicList, *pPrior = NULL;
  if(b3D)
  {
   while(pPic && (pPic->bValid==3))
   {
    pPrior = pPic;
    pPic = pPic->pNext;
   }
  }
  else
  {
   while(pPic && (pPic->bValid))
   {
    pPrior = pPic;
    pPic = pPic->pNext;
   }
  }

  if(!pPic)
  {
    pPic = (DecodedPicList *)calloc(1, sizeof(*pPic));
    pPrior->pNext = pPic;
  }

  return pPic;
}
/************************************
Interface: OpenDecoder
Return: 
       0: NOERROR;
       <0: ERROR;
************************************/
//int OpenDecoder(InputParameters *p_Inp)
//{
//#if (MVC_EXTENSION_ENABLE)
//  int i;
//#endif
//  int iRet;
//  DecoderParams *pDecoder;
//
//  iRet = alloc_decoder(&p_Dec);
//  if(iRet)
//  {
//    return (iRet|DEC_ERRMASK);
//  }
//  pDecoder = p_Dec;
//  //Configure (pDecoder->p_Vid, pDecoder->p_Inp, argc, argv);
//  memcpy(pDecoder->p_Inp, p_Inp, sizeof(InputParameters));
//  pDecoder->p_Vid->conceal_mode = pDecoder->p_Inp->conceal_mode;
//  pDecoder->p_Vid->ref_poc_gap = pDecoder->p_Inp->ref_poc_gap;
//  pDecoder->p_Vid->poc_gap = pDecoder->p_Inp->poc_gap;
//#if TRACE
//  if ((pDecoder->p_trace = fopen(TRACEFILE,"w"))==0)             // append new statistic at the end
//  {
//    snprintf(errortext, ET_SIZE, "Error open file %s!",TRACEFILE);
//    //error(errortext,500);
//    return -1;
//  }
//#endif
//
//#if (!MVC_EXTENSION_ENABLE)
//  if ((pDecoder->p_Vid->p_out = open(pDecoder->p_Inp->outfile, OPENFLAGS_WRITE, OPEN_PERMISSIONS))==-1)
//  {
//    snprintf(errortext, ET_SIZE, "Error open file %s ",p_Inp->outfile);
//    error(errortext,500);
//  }
//#endif
//
//  if(strlen(pDecoder->p_Inp->reffile)>0 && strcmp(pDecoder->p_Inp->reffile, "\"\""))
//  {
//   if ((pDecoder->p_Vid->p_ref = open(pDecoder->p_Inp->reffile, OPENFLAGS_READ))==-1)
//   {
//    fprintf(stdout," Input reference file                   : %s does not exist \n",pDecoder->p_Inp->reffile);
//    fprintf(stdout,"                                          SNR values are not available\n");
//   }
//  }
//  else
//    pDecoder->p_Vid->p_ref = -1;
//
//  initBitsFile(pDecoder->p_Vid, pDecoder->p_Inp->FileFormat);
//  pDecoder->p_Vid->bitsfile->OpenBitsFile(pDecoder->p_Vid, pDecoder->p_Inp->infile);
//  
//  // Allocate Slice data struct
//  //pDecoder->p_Vid->currentSlice = NULL; //malloc_slice(pDecoder->p_Inp, pDecoder->p_Vid);
//  
//  init_old_slice(pDecoder->p_Vid->old_slice);
//
//  init(pDecoder->p_Vid);
// 
//  init_out_buffer(pDecoder->p_Vid);
//
//
//#if (MVC_EXTENSION_ENABLE)
//  pDecoder->p_Vid->p_out = -1;
//  //multiview output file initialization
//  for(i=0;i<MAX_VIEW_NUM;i++)
//  {
//    pDecoder->p_Vid->p_out_mvc[i] = -1;
//  }
//  pDecoder->p_Vid->active_sps = NULL;
//  pDecoder->p_Vid->active_subset_sps = NULL;
//  init_subset_sps_list(pDecoder->p_Vid->SubsetSeqParSet, MAXSPS);
//#endif
//
//  return DEC_OPEN_NOERR;
//}

/************************************
Interface: DecodeOneFrame
Return: 
       0: NOERROR;
       1: Finished decoding;
       others: Error Code;
************************************/
//int DecodeOneFrame(DecodedPicList **ppDecPicList)
//{
//  int iRet;
//  DecoderParams *pDecoder = p_Dec;
//  ClearDecPicList(pDecoder->p_Vid);
//  iRet = decode_one_frame(pDecoder);
//  if(iRet == SOP)
//  {
//    iRet = DEC_SUCCEED;
//  }
//  else if(iRet == EOS)
//  {
//    iRet = DEC_EOS;
//  }
//  else
//  {
//    iRet |= DEC_ERRMASK;
//  }
//
//  *ppDecPicList = pDecoder->p_Vid->pDecOuputPic;
//  return iRet;
//}

/*int FinitDecoder(DecodedPicList **ppDecPicList)
{
  DecoderParams *pDecoder = p_Dec;
  if(!pDecoder)
    return DEC_GEN_NOERR;
  ClearDecPicList(pDecoder->p_Vid);
#if (MVC_EXTENSION_ENABLE)
  flush_dpb(pDecoder->p_Vid->p_Dpb, -1);
#else
  flush_dpb(pDecoder->p_Vid->p_Dpb);
#endif
#if (PAIR_FIELDS_IN_OUTPUT)
  flush_pending_output(pDecoder->p_Vid, pDecoder->p_Vid->p_out);
#endif
  ResetAnnexB(pDecoder->p_Vid->annex_b); 
  pDecoder->p_Vid->newframe = 0;
  pDecoder->p_Vid->previous_frame_num = 0;
  *ppDecPicList = pDecoder->p_Vid->pDecOuputPic;
  return DEC_GEN_NOERR;
}*/

//int CloseDecoder()
//{
//#if (MVC_EXTENSION_ENABLE)
//  int i;
//#endif
//
//  DecoderParams *pDecoder = p_Dec;
//  if(!pDecoder)
//    return DEC_CLOSE_NOERR;
//  
//  Report(pDecoder->p_Vid);
//  FmoFinit(pDecoder->p_Vid);
//  free_global_buffers(pDecoder->p_Vid);
//
//  pDecoder->p_Vid->bitsfile->CloseBitsFile(pDecoder->p_Vid);
//
//#if (MVC_EXTENSION_ENABLE)
//  for(i=0;i<MAX_VIEW_NUM;i++)
//  {
//    if (pDecoder->p_Vid->p_out_mvc[i] != -1)
//    {
//      close(pDecoder->p_Vid->p_out_mvc[i]);
//    }
//  }
//#else
//  close(pDecoder->p_Vid->p_out);
//#endif
//
//  if (pDecoder->p_Vid->p_ref != -1)
//    close(pDecoder->p_Vid->p_ref);
//
//#if TRACE
//  fclose(pDecoder->p_trace);
//#endif
//
//  ercClose(pDecoder->p_Vid, pDecoder->p_Vid->erc_errorVar);
//
//  CleanUpPPS(pDecoder->p_Vid);
//#if (MVC_EXTENSION_ENABLE)
//  for(i=0; i<MAXSPS; i++)
//  {
//    reset_subset_sps(pDecoder->p_Vid->SubsetSeqParSet+i);
//  }
//#endif
//  free_dpb(pDecoder->p_Vid->p_Dpb);
//  uninit_out_buffer(pDecoder->p_Vid);
//
//  free (pDecoder->p_Inp);
//  free_img (pDecoder->p_Vid);
//  free(pDecoder);
//
//  return DEC_CLOSE_NOERR;
//}
