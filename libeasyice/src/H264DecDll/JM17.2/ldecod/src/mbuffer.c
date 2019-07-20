
/*!
 ***********************************************************************
 *  \file
 *      mbuffer.c
 *
 *  \brief
 *      Frame buffer functions
 *
 *  \author
 *      Main contributors (see contributors.h for copyright, address and affiliation details)
 *      - Karsten Sühring                 <suehring@hhi.de>
 *      - Alexis Tourapis                 <alexismt@ieee.org>
 *      - Jill Boyce                      <jill.boyce@thomson.net>
 *      - Saurav K Bandyopadhyay          <saurav@ieee.org>
 *      - Zhenyu Wu                       <Zhenyu.Wu@thomson.net
 *      - Purvin Pandit                   <Purvin.Pandit@thomson.net>
 *
 ***********************************************************************
 */

#include <limits.h>

#include "global.h"
#include "erc_api.h"
#include "header.h"
#include "image.h"
#include "mbuffer.h"
#include "memalloc.h"
#include "output.h"
#include "mbuffer_mvc.h"
#include "fast_memory.h"



static void insert_picture_in_dpb    (VideoParameters *p_Vid, FrameStore* fs, StorablePicture* p);
#if (MVC_EXTENSION_ENABLE)
static int  output_one_frame_from_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id);
static void get_smallest_poc         (DecodedPictureBuffer *p_Dpb, int *poc,int * pos, int curr_view_id);
static int  remove_unused_frame_from_dpb (DecodedPictureBuffer *p_Dpb, int curr_view_id);
#else
static void output_one_frame_from_dpb(DecodedPictureBuffer *p_Dpb);
static void get_smallest_poc         (DecodedPictureBuffer *p_Dpb, int *poc,int * pos);
static int  remove_unused_frame_from_dpb (DecodedPictureBuffer *p_Dpb);
#endif
static void gen_field_ref_ids        (VideoParameters *p_Vid, StorablePicture *p);
static int  is_used_for_reference    (FrameStore* fs);
static int  is_short_term_reference  (FrameStore* fs);
static int  is_long_term_reference   (FrameStore* fs);

#define MAX_LIST_SIZE 33

/*!
 ************************************************************************
 * \brief
 *    Print out list of pictures in DPB. Used for debug purposes.
 ************************************************************************
 */
void dump_dpb(DecodedPictureBuffer *p_Dpb)
{
#if DUMP_DPB
  unsigned i;

  for (i=0; i<p_Dpb->used_size;i++)
  {
    printf("(");
    printf("fn=%d  ", p_Dpb->fs[i]->frame_num);
    if (p_Dpb->fs[i]->is_used & 1)
    {
      if (p_Dpb->fs[i]->top_field)
        printf("T: poc=%d  ", p_Dpb->fs[i]->top_field->poc);
      else
        printf("T: poc=%d  ", p_Dpb->fs[i]->frame->top_poc);
    }
    if (p_Dpb->fs[i]->is_used & 2)
    {
      if (p_Dpb->fs[i]->bottom_field)
        printf("B: poc=%d  ", p_Dpb->fs[i]->bottom_field->poc);
      else
        printf("B: poc=%d  ", p_Dpb->fs[i]->frame->bottom_poc);
    }
    if (p_Dpb->fs[i]->is_used == 3)
      printf("F: poc=%d  ", p_Dpb->fs[i]->frame->poc);
    printf("G: poc=%d)  ", p_Dpb->fs[i]->poc);
    if (p_Dpb->fs[i]->is_reference) printf ("ref (%d) ", p_Dpb->fs[i]->is_reference);
    if (p_Dpb->fs[i]->is_long_term) printf ("lt_ref (%d) ", p_Dpb->fs[i]->is_reference);
    if (p_Dpb->fs[i]->is_output) printf ("out  ");
    if (p_Dpb->fs[i]->is_used == 3)
    {
      if (p_Dpb->fs[i]->frame->non_existing) printf ("ne  ");
    }
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs[i]->is_reference) 
      printf ("view_id (%d) ", p_Dpb->fs[i]->view_id);
#endif
    printf ("\n");
  }
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Returns the size of the dpb depending on level and picture size
 *
 *
 ************************************************************************
 */
int getDpbSize(seq_parameter_set_rbsp_t *active_sps)
{
  int pic_size = (active_sps->pic_width_in_mbs_minus1 + 1) * (active_sps->pic_height_in_map_units_minus1 + 1) * (active_sps->frame_mbs_only_flag?1:2) * 384;

  int size = 0;

  switch (active_sps->level_idc)
  {
  case 9:
    size = 152064;
    break;
  case 10:
    size = 152064;
    break;
  case 11:
    if (!IS_FREXT_PROFILE(active_sps->profile_idc) && (active_sps->constrained_set3_flag == 1))
      size = 152064;
    else
      size = 345600;
    break;
  case 12:
    size = 912384;
    break;
  case 13:
    size = 912384;
    break;
  case 20:
    size = 912384;
    break;
  case 21:
    size = 1824768;
    break;
  case 22:
    size = 3110400;
    break;
  case 30:
    size = 3110400;
    break;
  case 31:
    size = 6912000;
    break;
  case 32:
    size = 7864320;
    break;
  case 40:
    size = 12582912;
    break;
  case 41:
    size = 12582912;
    break;
  case 42:
    size = 13369344;
    break;
  case 50:
    size = 42393600;
    break;
  case 51:
    size = 70778880;
    break;
  default:
    error ("undefined level", 500);
    break;
  }

  size /= pic_size;
  size = imin( size, 16);

  if (active_sps->vui_parameters_present_flag && active_sps->vui_seq_parameters.bitstream_restriction_flag)
  {
    int size_vui;
    if ((int)active_sps->vui_seq_parameters.max_dec_frame_buffering > size)
    {
      error ("max_dec_frame_buffering larger than MaxDpbSize", 500);
    }
    size_vui = imax (1, active_sps->vui_seq_parameters.max_dec_frame_buffering);
#ifdef _DEBUG
    if(size_vui < size)
    {
      printf("Warning: max_dec_frame_buffering(%d) is less than DPB size(%d) calculated from Profile/Level.\n", size_vui, size);
    }
#endif
    size = size_vui;    
  }

  return size;
}

/*!
 ************************************************************************
 * \brief
 *    Check then number of frames marked "used for reference" and break
 *    if maximum is exceeded
 *
 ************************************************************************
 */
void check_num_ref(DecodedPictureBuffer *p_Dpb)
{
  if ((int)(p_Dpb->ltref_frames_in_buffer +  p_Dpb->ref_frames_in_buffer ) > (imax(1, p_Dpb->num_ref_frames)))
  {
    error ("Max. number of reference frames exceeded. Invalid stream.", 500);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Allocate memory for decoded picture buffer and initialize with sane values.
 *
 ************************************************************************
 */
void init_dpb(VideoParameters *p_Vid, DecodedPictureBuffer *p_Dpb)
{
  unsigned i; 
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  p_Dpb->p_Vid = p_Vid;

  if (p_Dpb->init_done)
  {
    free_dpb(p_Dpb);
  }

#if (MVC_EXTENSION_ENABLE)
  if(p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH)
    p_Dpb->size = GetMaxDecFrameBuffering(p_Vid) + 2;
  else
    p_Dpb->size = getDpbSize(active_sps);

  if(active_sps->profile_idc == MVC_HIGH || active_sps->profile_idc == STEREO_HIGH)
    p_Dpb->size = (p_Dpb->size<<1) + 2;
#else
  p_Dpb->size = getDpbSize(active_sps);
#endif

  
  p_Dpb->num_ref_frames = active_sps->num_ref_frames; 

#if (MVC_EXTENSION_ENABLE)
  if ((unsigned int)active_sps->max_dec_frame_buffering < active_sps->num_ref_frames)
#else
  if (p_Dpb->size < active_sps->num_ref_frames)
#endif
  {
    error ("DPB size at specified level is smaller than the specified number of reference frames. This is not allowed.\n", 1000);
  }

  p_Dpb->used_size = 0;
  p_Dpb->last_picture = NULL;

  p_Dpb->ref_frames_in_buffer = 0;
  p_Dpb->ltref_frames_in_buffer = 0;

  p_Dpb->fs = calloc(p_Dpb->size, sizeof (FrameStore*));
  if (NULL==p_Dpb->fs)
    no_mem_exit("init_dpb: p_Dpb->fs");

  p_Dpb->fs_ref = calloc(p_Dpb->size, sizeof (FrameStore*));
  if (NULL==p_Dpb->fs_ref)
    no_mem_exit("init_dpb: p_Dpb->fs_ref");

  p_Dpb->fs_ltref = calloc(p_Dpb->size, sizeof (FrameStore*));
  if (NULL==p_Dpb->fs_ltref)
    no_mem_exit("init_dpb: p_Dpb->fs_ltref");

  for (i = 0; i < p_Dpb->size; i++)
  {
    p_Dpb->fs[i]       = alloc_frame_store();
    p_Dpb->fs_ref[i]   = NULL;
    p_Dpb->fs_ltref[i] = NULL;
#if (MVC_EXTENSION_ENABLE)
    p_Dpb->fs[i]->view_id = MVC_INIT_VIEW_ID;
    p_Dpb->fs[i]->inter_view_flag[0] = p_Dpb->fs[i]->inter_view_flag[1] = 0;
    p_Dpb->fs[i]->anchor_pic_flag[0] = p_Dpb->fs[i]->anchor_pic_flag[1] = 0;
#endif
  }
  /*
  for (i = 0; i < 6; i++)
  {
  currSlice->listX[i] = calloc(MAX_LIST_SIZE, sizeof (StorablePicture*)); // +1 for reordering
  if (NULL==currSlice->listX[i])
  no_mem_exit("init_dpb: currSlice->listX[i]");
  }
  */
  /* allocate a dummy storable picture */
  p_Vid->no_reference_picture = alloc_storable_picture (p_Vid, FRAME, p_Vid->width, p_Vid->height, p_Vid->width_cr, p_Vid->height_cr);
  p_Vid->no_reference_picture->top_field    = p_Vid->no_reference_picture;
  p_Vid->no_reference_picture->bottom_field = p_Vid->no_reference_picture;
  p_Vid->no_reference_picture->frame        = p_Vid->no_reference_picture;

  p_Dpb->last_output_poc = INT_MIN;

#if (MVC_EXTENSION_ENABLE)
  p_Dpb->last_output_view_id = -1;
#endif

  p_Vid->last_has_mmco_5 = 0;

  p_Dpb->init_done = 1;

  // picture error concealment
  if(p_Vid->conceal_mode !=0)
    p_Vid->last_out_fs = alloc_frame_store();
}

void re_init_dpb(VideoParameters *p_Vid, DecodedPictureBuffer *p_Dpb)
{
  int i; 
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
  int iDpbSize;

#if (MVC_EXTENSION_ENABLE)
  if(p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH)
    iDpbSize = GetMaxDecFrameBuffering(p_Vid) + 2;
  else
    iDpbSize = getDpbSize(active_sps);

  if(active_sps->profile_idc == MVC_HIGH || active_sps->profile_idc == STEREO_HIGH)
    iDpbSize = (iDpbSize<<1) + 2;
#else
  iDpbSize = getDpbSize(active_sps);
#endif

  p_Dpb->num_ref_frames = active_sps->num_ref_frames;
  if( iDpbSize > (int)p_Dpb->size)
  {
#if (MVC_EXTENSION_ENABLE)
    if ((unsigned int)active_sps->max_dec_frame_buffering < active_sps->num_ref_frames)
#else
    if (p_Dpb->size < active_sps->num_ref_frames)
#endif
    {
      error ("DPB size at specified level is smaller than the specified number of reference frames. This is not allowed.\n", 1000);
    }

    p_Dpb->fs = realloc(p_Dpb->fs, iDpbSize*sizeof (FrameStore*));
    if (NULL==p_Dpb->fs)
      no_mem_exit("re_init_dpb: p_Dpb->fs");

    p_Dpb->fs_ref = realloc(p_Dpb->fs_ref, iDpbSize*sizeof (FrameStore*));
    if (NULL==p_Dpb->fs_ref)
      no_mem_exit("re_init_dpb: p_Dpb->fs_ref");

    p_Dpb->fs_ltref = realloc(p_Dpb->fs_ltref, iDpbSize*sizeof (FrameStore*));
    if (NULL==p_Dpb->fs_ltref)
      no_mem_exit("re_init_dpb: p_Dpb->fs_ltref");

    for (i = p_Dpb->size; i < iDpbSize; i++)
    {
      p_Dpb->fs[i]       = alloc_frame_store();
      p_Dpb->fs_ref[i]   = NULL;
      p_Dpb->fs_ltref[i] = NULL;
#if (MVC_EXTENSION_ENABLE)
      p_Dpb->fs[i]->view_id = MVC_INIT_VIEW_ID;
      p_Dpb->fs[i]->inter_view_flag[0] = p_Dpb->fs[i]->inter_view_flag[1] = 0;
      p_Dpb->fs[i]->anchor_pic_flag[0] = p_Dpb->fs[i]->anchor_pic_flag[1] = 0;
#endif
    }
    p_Dpb->size = iDpbSize;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Free memory for decoded picture buffer.
 ************************************************************************
 */
void free_dpb(DecodedPictureBuffer *p_Dpb)
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned i;
  if (p_Dpb->fs)
  {
    for (i=0; i<p_Dpb->size; i++)
    {
      free_frame_store(p_Dpb->fs[i]);
    }
    free (p_Dpb->fs);
    p_Dpb->fs=NULL;
  }
  if (p_Dpb->fs_ref)
  {
    free (p_Dpb->fs_ref);
  }
  if (p_Dpb->fs_ltref)
  {
    free (p_Dpb->fs_ltref);
  }
  p_Dpb->last_output_poc = INT_MIN;
#if (MVC_EXTENSION_ENABLE)
  p_Dpb->last_output_view_id = -1;
#endif

  p_Dpb->init_done = 0;

  // picture error concealment
  if(p_Vid->conceal_mode != 0)
      free_frame_store(p_Vid->last_out_fs);

  free_storable_picture(p_Vid->no_reference_picture);
}


/*!
 ************************************************************************
 * \brief
 *    Allocate memory for decoded picture buffer frame stores and initialize with sane values.
 *
 * \return
 *    the allocated FrameStore structure
 ************************************************************************
 */
FrameStore* alloc_frame_store(void)
{
  FrameStore *f;

  f = calloc (1, sizeof(FrameStore));
  if (NULL==f)
    no_mem_exit("alloc_frame_store: f");

  f->is_used      = 0;
  f->is_reference = 0;
  f->is_long_term = 0;
  f->is_orig_reference = 0;

  f->is_output = 0;

  f->frame        = NULL;;
  f->top_field    = NULL;
  f->bottom_field = NULL;

  return f;
}

void alloc_pic_motion(PicMotionParamsOld *motion, int size_y, int size_x)
{
  motion->mb_field = calloc (size_y * size_x, sizeof(byte));
  if (motion->mb_field == NULL)
    no_mem_exit("alloc_storable_picture: motion->mb_field");
}

/*!
 ************************************************************************
 * \brief
 *    Allocate memory for a stored picture.
 *
 * \param p_Vid
 *    VideoParameters
 * \param structure
 *    picture structure
 * \param size_x
 *    horizontal luma size
 * \param size_y
 *    vertical luma size
 * \param size_x_cr
 *    horizontal chroma size
 * \param size_y_cr
 *    vertical chroma size
 *
 * \return
 *    the allocated StorablePicture structure
 ************************************************************************
 */
StorablePicture* alloc_storable_picture(VideoParameters *p_Vid, PictureStructure structure, int size_x, int size_y, int size_x_cr, int size_y_cr)
{
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;  

  StorablePicture *s;
  int   nplane;

  //printf ("Allocating (%s) picture (x=%d, y=%d, x_cr=%d, y_cr=%d)\n", (type == FRAME)?"FRAME":(type == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", size_x, size_y, size_x_cr, size_y_cr);

  s = calloc (1, sizeof(StorablePicture));
  if (NULL==s)
    no_mem_exit("alloc_storable_picture: s");

  if (structure!=FRAME)
  {
    size_y    /= 2;
    size_y_cr /= 2;
  }

  s->PicSizeInMbs = (size_x*size_y)/256;
  s->imgUV = NULL;

  //get_mem2Dpel (&(s->imgY), size_y, size_x);
  get_mem2DpelWithPad (&(s->imgY), size_y, size_x, p_Vid->iLumaPadY, p_Vid->iLumaPadX);
  s->iLumaStride = size_x+2*p_Vid->iLumaPadX;
  s->iLumaExpandedHeight = size_y+2*p_Vid->iLumaPadY;

  if (active_sps->chroma_format_idc != YUV400)
    get_mem3DpelWithPad(&(s->imgUV), 2, size_y_cr, size_x_cr, p_Vid->iChromaPadY, p_Vid->iChromaPadX);  //get_mem3Dpel (&(s->imgUV), 2, size_y_cr, size_x_cr);
  s->iChromaStride =size_x_cr + 2*p_Vid->iChromaPadX;
  s->iChromaExpandedHeight = size_y_cr + 2*p_Vid->iChromaPadY;
  s->iLumaPadY = p_Vid->iLumaPadY;
  s->iLumaPadX = p_Vid->iLumaPadX;
  s->iChromaPadY = p_Vid->iChromaPadY;
  s->iChromaPadX = p_Vid->iChromaPadX;

  s->separate_colour_plane_flag = p_Vid->separate_colour_plane_flag;


  get_mem2Dshort (&(s->slice_id), size_y / MB_BLOCK_SIZE, size_x / MB_BLOCK_SIZE);

  get_mem2Dmp     ( &s->mv_info, size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);
  alloc_pic_motion( &s->motion , size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);

  if( (p_Vid->separate_colour_plane_flag != 0) )
  {
    for( nplane=0; nplane<MAX_PLANE; nplane++ )
    {
     get_mem2Dmp      (&s->JVmv_info[nplane], size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);
      alloc_pic_motion(&s->JVmotion[nplane] , size_y / BLOCK_SIZE, size_x / BLOCK_SIZE);
    }
  }

  s->pic_num=0;
  s->frame_num=0;
  s->long_term_frame_idx=0;
  s->long_term_pic_num=0;
  s->used_for_reference=0;
  s->is_long_term=0;
  s->non_existing=0;
  s->is_output = 0;
  s->max_slice_id = 0;
#if (MVC_EXTENSION_ENABLE)
  s->view_id = -1;
#endif

  s->structure=structure;

  s->size_x = size_x;
  s->size_y = size_y;
  s->size_x_cr = size_x_cr;
  s->size_y_cr = size_y_cr;
  s->size_x_m1 = size_x - 1;
  s->size_y_m1 = size_y - 1;
  s->size_x_cr_m1 = size_x_cr - 1;
  s->size_y_cr_m1 = size_y_cr - 1;

  s->top_field    = p_Vid->no_reference_picture;
  s->bottom_field = p_Vid->no_reference_picture;
  s->frame        = p_Vid->no_reference_picture;

  s->dec_ref_pic_marking_buffer = NULL;

  s->coded_frame  = 0;
  s->mb_aff_frame_flag  = 0;

  s->top_poc = s->bottom_poc = s->poc = 0;
  s->seiHasTone_mapping = 0;

  if(!p_Vid->active_sps->frame_mbs_only_flag && structure!=FRAME)
  {
   int i;
   for (i = 0; i < 2; i++)
   {
    s->listX[i] = calloc(MAX_LIST_SIZE, sizeof (StorablePicture*)); // +1 for reordering
    if (NULL==s->listX[i])
      no_mem_exit("alloc_storable_picture: s->listX[i]");
   }
  }

  return s;
}

/*!
 ************************************************************************
 * \brief
 *    Free frame store memory.
 *
 * \param p_Vid
 *    VideoParameters
 * \param f
 *    FrameStore to be freed
 *
 ************************************************************************
 */
void free_frame_store(FrameStore* f)
{
  if (f)
  {
    if (f->frame)
    {
      free_storable_picture(f->frame);
      f->frame=NULL;
    }
    if (f->top_field)
    {
      free_storable_picture(f->top_field);
      f->top_field=NULL;
    }
    if (f->bottom_field)
    {
      free_storable_picture(f->bottom_field);
      f->bottom_field=NULL;
    }
    free(f);
  }
}

void free_pic_motion(PicMotionParamsOld *motion)
{
  if (motion->mb_field)
  {
    free(motion->mb_field);
    motion->mb_field = NULL;
  }
}


/*!
 ************************************************************************
 * \brief
 *    Free picture memory.
 *
 * \param p
 *    Picture to be freed
 *
 ************************************************************************
 */
void free_storable_picture(StorablePicture* p)
{
  int nplane;
  if (p)
  {
    if (p->mv_info)
    {
      free_mem2Dmp(p->mv_info);
      p->mv_info = NULL;
    }
    free_pic_motion(&p->motion);

    if( (p->separate_colour_plane_flag != 0) )
    {
      for( nplane=0; nplane<MAX_PLANE; nplane++ )
      {
        if (p->JVmv_info[nplane])
        {
          free_mem2Dmp(p->JVmv_info[nplane]);
          p->JVmv_info[nplane] = NULL;
        }
        free_pic_motion(&p->JVmotion[nplane]);
      }
    }

    if (p->imgY)
    {
      free_mem2DpelWithPad(p->imgY, p->iLumaPadY, p->iLumaPadX);
      p->imgY=NULL;
    }

    if (p->imgUV)
    {
      free_mem3DpelWithPad(p->imgUV, p->iChromaPadY, p->iChromaPadX);
      p->imgUV=NULL;
    }

    if (p->slice_id)
    {
      free_mem2Dshort(p->slice_id);
      p->slice_id=NULL;
    }

    if (p->seiHasTone_mapping)
      free(p->tone_mapping_lut);

    {
      int i;
      for(i=0; i<2; i++)
      {
        if(p->listX[i])
        {
          free(p->listX[i]);
          p->listX[i] = NULL;
        }
      }
    }
    free(p);
    p = NULL;
  }
}

/*!
 ************************************************************************
 * \brief
 *    mark FrameStore unused for reference
 *
 ************************************************************************
 */
static void unmark_for_reference(FrameStore* fs)
{

  if (fs->is_used & 1)
  {
    if (fs->top_field)
    {
      fs->top_field->used_for_reference = 0;
    }
  }
  if (fs->is_used & 2)
  {
    if (fs->bottom_field)
    {
      fs->bottom_field->used_for_reference = 0;
    }
  }
  if (fs->is_used == 3)
  {
    if (fs->top_field && fs->bottom_field)
    {
      fs->top_field->used_for_reference = 0;
      fs->bottom_field->used_for_reference = 0;
    }
    fs->frame->used_for_reference = 0;
  }

  fs->is_reference = 0;

  if(fs->frame)
  {
    free_pic_motion(&fs->frame->motion);
  }

  if (fs->top_field)
  {
    free_pic_motion(&fs->top_field->motion);
  }

  if (fs->bottom_field)
  {
    free_pic_motion(&fs->bottom_field->motion);
  }
}


/*!
 ************************************************************************
 * \brief
 *    mark FrameStore unused for reference and reset long term flags
 *
 ************************************************************************
 */
static void unmark_for_long_term_reference(FrameStore* fs)
{

  if (fs->is_used & 1)
  {
    if (fs->top_field)
    {
      fs->top_field->used_for_reference = 0;
      fs->top_field->is_long_term = 0;
    }
  }
  if (fs->is_used & 2)
  {
    if (fs->bottom_field)
    {
      fs->bottom_field->used_for_reference = 0;
      fs->bottom_field->is_long_term = 0;
    }
  }
  if (fs->is_used == 3)
  {
    if (fs->top_field && fs->bottom_field)
    {
      fs->top_field->used_for_reference = 0;
      fs->top_field->is_long_term = 0;
      fs->bottom_field->used_for_reference = 0;
      fs->bottom_field->is_long_term = 0;
    }
    fs->frame->used_for_reference = 0;
    fs->frame->is_long_term = 0;
  }

  fs->is_reference = 0;
  fs->is_long_term = 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two stored pictures by picture number for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_pic_by_pic_num_desc( const void *arg1, const void *arg2 )
{
  int pic_num1 = (*(StorablePicture**)arg1)->pic_num;
  int pic_num2 = (*(StorablePicture**)arg2)->pic_num;

  if (pic_num1 < pic_num2)
    return 1;
  if (pic_num1 > pic_num2)
    return -1;
  else
    return 0;
}

/*!
 ************************************************************************
 * \brief
 *    compares two stored pictures by picture number for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_pic_by_lt_pic_num_asc( const void *arg1, const void *arg2 )
{
  int long_term_pic_num1 = (*(StorablePicture**)arg1)->long_term_pic_num;
  int long_term_pic_num2 = (*(StorablePicture**)arg2)->long_term_pic_num;

  if ( long_term_pic_num1 < long_term_pic_num2)
    return -1;
  if ( long_term_pic_num1 > long_term_pic_num2)
    return 1;
  else
    return 0;
}

/*!
 ************************************************************************
 * \brief
 *    compares two frame stores by pic_num for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_fs_by_frame_num_desc( const void *arg1, const void *arg2 )
{
  int frame_num_wrap1 = (*(FrameStore**)arg1)->frame_num_wrap;
  int frame_num_wrap2 = (*(FrameStore**)arg2)->frame_num_wrap;
  if ( frame_num_wrap1 < frame_num_wrap2)
    return 1;
  if ( frame_num_wrap1 > frame_num_wrap2)
    return -1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two frame stores by lt_pic_num for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_fs_by_lt_pic_idx_asc( const void *arg1, const void *arg2 )
{
  int long_term_frame_idx1 = (*(FrameStore**)arg1)->long_term_frame_idx;
  int long_term_frame_idx2 = (*(FrameStore**)arg2)->long_term_frame_idx;

  if ( long_term_frame_idx1 < long_term_frame_idx2)
    return -1;
  if ( long_term_frame_idx1 > long_term_frame_idx2)
    return 1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two stored pictures by poc for qsort in ascending order
 *
 ************************************************************************
 */
static inline int compare_pic_by_poc_asc( const void *arg1, const void *arg2 )
{
  int poc1 = (*(StorablePicture**)arg1)->poc;
  int poc2 = (*(StorablePicture**)arg2)->poc;

  if ( poc1 < poc2)
    return -1;  
  if ( poc1 > poc2)
    return 1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two stored pictures by poc for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_pic_by_poc_desc( const void *arg1, const void *arg2 )
{
  int poc1 = (*(StorablePicture**)arg1)->poc;
  int poc2 = (*(StorablePicture**)arg2)->poc;

  if (poc1 < poc2)
    return 1;
  if (poc1 > poc2)
    return -1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two frame stores by poc for qsort in ascending order
 *
 ************************************************************************
 */
static inline int compare_fs_by_poc_asc( const void *arg1, const void *arg2 )
{
  int poc1 = (*(FrameStore**)arg1)->poc;
  int poc2 = (*(FrameStore**)arg2)->poc;

  if (poc1 < poc2)
    return -1;
  if (poc1 > poc2)
    return 1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    compares two frame stores by poc for qsort in descending order
 *
 ************************************************************************
 */
static inline int compare_fs_by_poc_desc( const void *arg1, const void *arg2 )
{
  int poc1 = (*(FrameStore**)arg1)->poc;
  int poc2 = (*(FrameStore**)arg2)->poc;

  if (poc1 < poc2)
    return 1;
  if (poc1 > poc2)
    return -1;
  else
    return 0;
}


/*!
 ************************************************************************
 * \brief
 *    returns true, if picture is short term reference picture
 *
 ************************************************************************
 */
int is_short_ref(StorablePicture *s)
{
  return ((s->used_for_reference) && (!(s->is_long_term)));
}


/*!
 ************************************************************************
 * \brief
 *    returns true, if picture is long term reference picture
 *
 ************************************************************************
 */
int is_long_ref(StorablePicture *s)
{
  return ((s->used_for_reference) && (s->is_long_term));
}


/*!
 ************************************************************************
 * \brief
 *    Generates a alternating field list from a given FrameStore list
 *
 ************************************************************************
 */
void gen_pic_list_from_frame_list(PictureStructure currStructure, FrameStore **fs_list, int list_idx, StorablePicture **list, char *list_size, int long_term)
{
  int top_idx = 0;
  int bot_idx = 0;

  int (*is_ref)(StorablePicture *s);

  if (long_term)
    is_ref=is_long_ref;
  else
    is_ref=is_short_ref;

  if (currStructure == TOP_FIELD)
  {
    while ((top_idx<list_idx)||(bot_idx<list_idx))
    {
      for ( ; top_idx<list_idx; top_idx++)
      {
        if(fs_list[top_idx]->is_used & 1)
        {
          if(is_ref(fs_list[top_idx]->top_field))
          {
            // short term ref pic
            list[(short) *list_size] = fs_list[top_idx]->top_field;
            (*list_size)++;
            top_idx++;
            break;
          }
        }
      }
      for ( ; bot_idx<list_idx; bot_idx++)
      {
        if(fs_list[bot_idx]->is_used & 2)
        {
          if(is_ref(fs_list[bot_idx]->bottom_field))
          {
            // short term ref pic
            list[(short) *list_size] = fs_list[bot_idx]->bottom_field;
            (*list_size)++;
            bot_idx++;
            break;
          }
        }
      }
    }
  }
  if (currStructure == BOTTOM_FIELD)
  {
    while ((top_idx<list_idx)||(bot_idx<list_idx))
    {
      for ( ; bot_idx<list_idx; bot_idx++)
      {
        if(fs_list[bot_idx]->is_used & 2)
        {
          if(is_ref(fs_list[bot_idx]->bottom_field))
          {
            // short term ref pic
            list[(short) *list_size] = fs_list[bot_idx]->bottom_field;
            (*list_size)++;
            bot_idx++;
            break;
          }
        }
      }
      for ( ; top_idx<list_idx; top_idx++)
      {
        if(fs_list[top_idx]->is_used & 1)
        {
          if(is_ref(fs_list[top_idx]->top_field))
          {
            // short term ref pic
            list[(short) *list_size] = fs_list[top_idx]->top_field;
            (*list_size)++;
            top_idx++;
            break;
          }
        }
      }
    }
  }
}

#if (MVC_EXTENSION_ENABLE)
/*!
 ************************************************************************
 * \brief
 *    Generates a alternating field list from a given FrameStore inter-view list
 *
 ************************************************************************
 */
static void gen_pic_list_from_frame_interview_list(PictureStructure currStrcture, FrameStore **fs_list, int list_idx, StorablePicture **list, char *list_size)
{
  int i;

  if (currStrcture == TOP_FIELD)
  {
    for (i=0; i<list_idx; i++)
    {
      list[(int)(*list_size)] = fs_list[i]->top_field;
      (*list_size)++;
    }
  }
  if (currStrcture == BOTTOM_FIELD)
  {
    for (i=0; i<list_idx; i++)
    {
      list[(int)(*list_size)] = fs_list[i]->bottom_field;
      (*list_size)++;
    }
  }
}
#endif


void update_pic_num(Slice *currSlice)
{
  unsigned int i;
  VideoParameters *p_Vid = currSlice->p_Vid;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;
  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;

  int add_top = 0, add_bottom = 0;
  int MaxFrameNum = 1 << (active_sps->log2_max_frame_num_minus4 + 4);

  if (currSlice->structure == FRAME)
  {
    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ref[i]->is_used==3)
      {
        if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
        {
          if( p_Dpb->fs_ref[i]->frame_num > currSlice->frame_num )
          {
            p_Dpb->fs_ref[i]->frame_num_wrap = p_Dpb->fs_ref[i]->frame_num - MaxFrameNum;
          }
          else
          {
            p_Dpb->fs_ref[i]->frame_num_wrap = p_Dpb->fs_ref[i]->frame_num;
          }
          p_Dpb->fs_ref[i]->frame->pic_num = p_Dpb->fs_ref[i]->frame_num_wrap;
        }
      }
    }
    // update long_term_pic_num
    for (i = 0; i < p_Dpb->ltref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ltref[i]->is_used==3)
      {
        if (p_Dpb->fs_ltref[i]->frame->is_long_term)
        {
          p_Dpb->fs_ltref[i]->frame->long_term_pic_num = p_Dpb->fs_ltref[i]->frame->long_term_frame_idx;
        }
      }
    }
  }
  else
  {
    if (currSlice->structure == TOP_FIELD)
    {
      add_top    = 1;
      add_bottom = 0;
    }
    else
    {
      add_top    = 0;
      add_bottom = 1;
    }

    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ref[i]->is_reference)
      {
        if( p_Dpb->fs_ref[i]->frame_num > currSlice->frame_num )
        {
          p_Dpb->fs_ref[i]->frame_num_wrap = p_Dpb->fs_ref[i]->frame_num - MaxFrameNum;
        }
        else
        {
          p_Dpb->fs_ref[i]->frame_num_wrap = p_Dpb->fs_ref[i]->frame_num;
        }
        if (p_Dpb->fs_ref[i]->is_reference & 1)
        {
          p_Dpb->fs_ref[i]->top_field->pic_num = (2 * p_Dpb->fs_ref[i]->frame_num_wrap) + add_top;
        }
        if (p_Dpb->fs_ref[i]->is_reference & 2)
        {
          p_Dpb->fs_ref[i]->bottom_field->pic_num = (2 * p_Dpb->fs_ref[i]->frame_num_wrap) + add_bottom;
        }
      }
    }
    // update long_term_pic_num
    for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ltref[i]->is_long_term & 1)
      {
        p_Dpb->fs_ltref[i]->top_field->long_term_pic_num = 2 * p_Dpb->fs_ltref[i]->top_field->long_term_frame_idx + add_top;
      }
      if (p_Dpb->fs_ltref[i]->is_long_term & 2)
      {
        p_Dpb->fs_ltref[i]->bottom_field->long_term_pic_num = 2 * p_Dpb->fs_ltref[i]->bottom_field->long_term_frame_idx + add_bottom;
      }
    }
  }
}

/*!
************************************************************************
* \brief
*    Initialize reference lists depending on current slice type
*
************************************************************************
*/
void init_lists_i_slice(Slice *currSlice)
{
  //VideoParameters *p_Vid = currSlice->p_Vid;

#if (MVC_EXTENSION_ENABLE)
  currSlice->listinterviewidx0 = 0;
  currSlice->listinterviewidx1 = 0;
#endif

  currSlice->listXsize[0] = 0;
  currSlice->listXsize[1] = 0;
}

/*!
************************************************************************
* \brief
*    Initialize reference lists for a P Slice
*
************************************************************************
*/
void init_lists_p_slice(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

  unsigned int i;

  int list0idx = 0;
  int listltidx = 0;

  FrameStore **fs_list0;
  FrameStore **fs_listlt;

#if (MVC_EXTENSION_ENABLE)
  int currPOC = currSlice->ThisPOC;
  int curr_view_id = currSlice->view_id;
  int anchor_pic_flag = currSlice->anchor_pic_flag;

  currSlice->listinterviewidx0 = 0;
  currSlice->listinterviewidx1 = 0;
#endif

  if (currSlice->structure == FRAME)
  {
    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ref[i]->is_used==3)
      {
#if (MVC_EXTENSION_ENABLE)
        if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
        if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
        {
          currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
        }
      }
    }
    // order list 0 by PicNum
    qsort((void *)currSlice->listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_pic_num_desc);
    currSlice->listXsize[0] = (char) list0idx;
    //printf("listX[0] (PicNum): "); for (i=0; i<list0idx; i++){printf ("%d  ", currSlice->listX[0][i]->pic_num);} printf("\n");

    // long term handling
    for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
    {
      if (p_Dpb->fs_ltref[i]->is_used==3)
      {
#if (MVC_EXTENSION_ENABLE)
        if (p_Dpb->fs_ltref[i]->frame->is_long_term && (p_Dpb->fs_ltref[i]->frame->view_id == curr_view_id))
#else
        if (p_Dpb->fs_ltref[i]->frame->is_long_term)
#endif
        {
          currSlice->listX[0][list0idx++]=p_Dpb->fs_ltref[i]->frame;
        }
      }
    }
    qsort((void *)&currSlice->listX[0][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
    currSlice->listXsize[0] = (char) list0idx;
  }
  else
  {
    fs_list0 = calloc(p_Dpb->size, sizeof (FrameStore*));
    if (NULL==fs_list0)
      no_mem_exit("init_lists: fs_list0");
    fs_listlt = calloc(p_Dpb->size, sizeof (FrameStore*));
    if (NULL==fs_listlt)
      no_mem_exit("init_lists: fs_listlt");

    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
#if (MVC_EXTENSION_ENABLE)
      if (p_Dpb->fs_ref[i]->is_reference && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
      if (p_Dpb->fs_ref[i]->is_reference)
#endif
      {
        fs_list0[list0idx++] = p_Dpb->fs_ref[i];
      }
    }

    qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_frame_num_desc);

    //printf("fs_list0 (FrameNum): "); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list0[i]->frame_num_wrap);} printf("\n");

    currSlice->listXsize[0] = 0;
    gen_pic_list_from_frame_list(currSlice->structure, fs_list0, list0idx, currSlice->listX[0], &currSlice->listXsize[0], 0);

    //printf("listX[0] (PicNum): "); for (i=0; i < currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->pic_num);} printf("\n");

    // long term handling
    for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
    {
#if (MVC_EXTENSION_ENABLE)
      if (p_Dpb->fs_ltref[i]->view_id == curr_view_id)
#endif
        fs_listlt[listltidx++]=p_Dpb->fs_ltref[i];
    }

    qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

    gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[0], &currSlice->listXsize[0], 1);

    free(fs_list0);
    free(fs_listlt);
  }
  currSlice->listXsize[1] = 0;    


#if (MVC_EXTENSION_ENABLE)
  if (currSlice->svc_extension_flag==0)
  {    
    currSlice->fs_listinterview0 = calloc(p_Dpb->size, sizeof (FrameStore*));
    if (NULL==currSlice->fs_listinterview0)
      no_mem_exit("init_lists: fs_listinterview0");
    list0idx = currSlice->listXsize[0];
    if (currSlice->structure == FRAME)
    {	
      append_interview_list(p_Dpb, 0, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);

      for (i=0; i<(unsigned int)currSlice->listinterviewidx0; i++)
      {
      currSlice->listX[0][list0idx++] = currSlice->fs_listinterview0[i]->frame;
      }
      currSlice->listXsize[0] = (char) list0idx;
    }
    else
    {
      append_interview_list(p_Dpb, currSlice->structure, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
      gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview0, currSlice->listinterviewidx0, currSlice->listX[0], &currSlice->listXsize[0]);
    }
  }
#endif
  // set max size
  currSlice->listXsize[0] = (char) imin (currSlice->listXsize[0], currSlice->num_ref_idx_active[LIST_0]);
  currSlice->listXsize[1] = (char) imin (currSlice->listXsize[1], currSlice->num_ref_idx_active[LIST_1]);

  // set the unused list entries to NULL
  for (i=currSlice->listXsize[0]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[0][i] = p_Vid->no_reference_picture;
  }
  for (i=currSlice->listXsize[1]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[1][i] = p_Vid->no_reference_picture;
  }
/*#if PRINTREFLIST
#if (MVC_EXTENSION_ENABLE)
  // print out for debug purpose
  if((p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH) && currSlice->current_slice_nr==0)
  {
    if(currSlice->listXsize[0]>0)
    {
      printf("\n");
      printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
      for(i=0; i<(unsigned int)(currSlice->listXsize[0]); i++)	//ref list 0
      {
        printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[0][i]->poc, currSlice->listX[0][i]->pic_num, currSlice->listX[0][i]->view_id);
      }
    }
  }
#endif
#endif*/
}

/*!
 ************************************************************************
 * \brief
 *    Initialize reference lists for a B Slice
 *
 ************************************************************************
 */
void init_lists_b_slice(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

  unsigned int i;
  int j;

  int list0idx = 0;
  int list0idx_1 = 0;
  int listltidx = 0;

  FrameStore **fs_list0;
  FrameStore **fs_list1;
  FrameStore **fs_listlt;

#if (MVC_EXTENSION_ENABLE)
  int currPOC = currSlice->ThisPOC;
  int curr_view_id = currSlice->view_id;
  int anchor_pic_flag = currSlice->anchor_pic_flag;

  currSlice->listinterviewidx0 = 0;
  currSlice->listinterviewidx1 = 0;
#endif

  {
    // B-Slice
    if (currSlice->structure == FRAME)
    {
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
          {
            if (currSlice->framepoc >= p_Dpb->fs_ref[i]->frame->poc) //!KS use >= for error concealment
              //            if (p_Vid->framepoc > p_Dpb->fs_ref[i]->frame->poc)
            {
              currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
            }
          }
        }
      }
      qsort((void *)currSlice->listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_poc_desc);
      list0idx_1 = list0idx;
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
          {
            if (currSlice->framepoc < p_Dpb->fs_ref[i]->frame->poc)
            {
              currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
            }
          }
        }
      }
      qsort((void *)&currSlice->listX[0][list0idx_1], list0idx-list0idx_1, sizeof(StorablePicture*), compare_pic_by_poc_asc);

      for (j=0; j<list0idx_1; j++)
      {
        currSlice->listX[1][list0idx-list0idx_1+j]=currSlice->listX[0][j];
      }
      for (j=list0idx_1; j<list0idx; j++)
      {
        currSlice->listX[1][j-list0idx_1]=currSlice->listX[0][j];
      }

      currSlice->listXsize[0] = currSlice->listXsize[1] = (char) list0idx;

      //      printf("currSlice->listX[0] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->poc);} printf("\n");
      //      printf("currSlice->listX[1] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[1]; i++){printf ("%d  ", currSlice->listX[1][i]->poc);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ltref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if (p_Dpb->fs_ltref[i]->frame->is_long_term && (p_Dpb->fs_ltref[i]->frame->view_id == curr_view_id))
#else
          if (p_Dpb->fs_ltref[i]->frame->is_long_term)
#endif
          {
            currSlice->listX[0][list0idx]   = p_Dpb->fs_ltref[i]->frame;
            currSlice->listX[1][list0idx++] = p_Dpb->fs_ltref[i]->frame;
          }
        }
      }
      qsort((void *)&currSlice->listX[0][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
      qsort((void *)&currSlice->listX[1][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
      currSlice->listXsize[0] = currSlice->listXsize[1] = (char) list0idx;
    }
    else
    {
      fs_list0 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_list0)
        no_mem_exit("init_lists: fs_list0");
      fs_list1 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_list1)
        no_mem_exit("init_lists: fs_list1");
      fs_listlt = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_listlt)
        no_mem_exit("init_lists: fs_listlt");

      currSlice->listXsize[0] = 0;
      currSlice->listXsize[1] = 1;

      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used)
        {
#if (MVC_EXTENSION_ENABLE)
          if (currSlice->ThisPOC >= p_Dpb->fs_ref[i]->poc && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
          if (currSlice->ThisPOC >= p_Dpb->fs_ref[i]->poc)
#endif
          {
            fs_list0[list0idx++] = p_Dpb->fs_ref[i];
          }
        }
      }
      qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_poc_desc);
      list0idx_1 = list0idx;
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used)
        {
#if (MVC_EXTENSION_ENABLE)
          if (currSlice->ThisPOC < p_Dpb->fs_ref[i]->poc && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
          if (currSlice->ThisPOC < p_Dpb->fs_ref[i]->poc)
#endif
          {
            fs_list0[list0idx++] = p_Dpb->fs_ref[i];
          }
        }
      }
      qsort((void *)&fs_list0[list0idx_1], list0idx-list0idx_1, sizeof(FrameStore*), compare_fs_by_poc_asc);

      for (j=0; j<list0idx_1; j++)
      {
        fs_list1[list0idx-list0idx_1+j]=fs_list0[j];
      }
      for (j=list0idx_1; j<list0idx; j++)
      {
        fs_list1[j-list0idx_1]=fs_list0[j];
      }

      //      printf("fs_list0 currPoc=%d (Poc): ", currSlice->ThisPOC); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list0[i]->poc);} printf("\n");
      //      printf("fs_list1 currPoc=%d (Poc): ", currSlice->ThisPOC); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list1[i]->poc);} printf("\n");

      currSlice->listXsize[0] = 0;
      currSlice->listXsize[1] = 0;
      gen_pic_list_from_frame_list(currSlice->structure, fs_list0, list0idx, currSlice->listX[0], &currSlice->listXsize[0], 0);
      gen_pic_list_from_frame_list(currSlice->structure, fs_list1, list0idx, currSlice->listX[1], &currSlice->listXsize[1], 0);

      //      printf("currSlice->listX[0] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->poc);} printf("\n");
      //      printf("currSlice->listX[1] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[1]; i++){printf ("%d  ", currSlice->listX[1][i]->poc);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
#if (MVC_EXTENSION_ENABLE)
        if (p_Dpb->fs_ltref[i]->view_id == curr_view_id)
#endif
          fs_listlt[listltidx++]=p_Dpb->fs_ltref[i];
      }

      qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

      gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[0], &currSlice->listXsize[0], 1);
      gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[1], &currSlice->listXsize[1], 1);

      free(fs_list0);
      free(fs_list1);
      free(fs_listlt);
    }
  }

  if ((currSlice->listXsize[0] == currSlice->listXsize[1]) && (currSlice->listXsize[0] > 1))
  {
    // check if lists are identical, if yes swap first two elements of currSlice->listX[1]
    int diff=0;
    for (j = 0; j< currSlice->listXsize[0]; j++)
    {
      if (currSlice->listX[0][j] != currSlice->listX[1][j])
      {
        diff = 1;
        break;
      }
    }
    if (!diff)
    {
      StorablePicture *tmp_s = currSlice->listX[1][0];
      currSlice->listX[1][0]=currSlice->listX[1][1];
      currSlice->listX[1][1]=tmp_s;
    }
  }

#if (MVC_EXTENSION_ENABLE)
  if (currSlice->svc_extension_flag == 0)
  {
    // B-Slice
    currSlice->fs_listinterview0 = calloc(p_Dpb->size, sizeof (FrameStore*));
    if (NULL==currSlice->fs_listinterview0)
      no_mem_exit("init_lists: fs_listinterview0");
    currSlice->fs_listinterview1 = calloc(p_Dpb->size, sizeof (FrameStore*));
    if (NULL==currSlice->fs_listinterview1)
      no_mem_exit("init_lists: fs_listinterview1");
    list0idx = currSlice->listXsize[0];
    if (currSlice->structure == FRAME)
    {	
      append_interview_list(p_Dpb, 0, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
      append_interview_list(p_Dpb, 0, 1, currSlice->fs_listinterview1, &currSlice->listinterviewidx1, currPOC, curr_view_id, anchor_pic_flag);

      for (i=0; i<(unsigned int)currSlice->listinterviewidx0; i++)
      {
        currSlice->listX[0][list0idx++]=currSlice->fs_listinterview0[i]->frame;
      }
      currSlice->listXsize[0] = (char) list0idx;
      list0idx = currSlice->listXsize[1];
      for (i=0; i<(unsigned int)currSlice->listinterviewidx1; i++)
      {
        currSlice->listX[1][list0idx++]=currSlice->fs_listinterview1[i]->frame;
      }
      currSlice->listXsize[1] = (char) list0idx;
    }
    else
    {
      append_interview_list(p_Dpb, currSlice->structure, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
      gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview0, currSlice->listinterviewidx0, currSlice->listX[0], &currSlice->listXsize[0]);
      append_interview_list(p_Dpb, currSlice->structure, 1, currSlice->fs_listinterview1, &currSlice->listinterviewidx1, currPOC, curr_view_id, anchor_pic_flag);
      gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview1, currSlice->listinterviewidx1, currSlice->listX[1], &currSlice->listXsize[1]);	
    }    
  }
#endif

  // set max size
  currSlice->listXsize[0] = (char) imin (currSlice->listXsize[0], currSlice->num_ref_idx_active[LIST_0]);
  currSlice->listXsize[1] = (char) imin (currSlice->listXsize[1], currSlice->num_ref_idx_active[LIST_1]);

  // set the unused list entries to NULL
  for (i=currSlice->listXsize[0]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[0][i] = p_Vid->no_reference_picture;
  }
  for (i=currSlice->listXsize[1]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[1][i] = p_Vid->no_reference_picture;
  }
/*#if PRINTREFLIST
#if (MVC_EXTENSION_ENABLE)
  // print out for debug purpose
  if((p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH) && currSlice->current_slice_nr==0)
  {
    if((currSlice->listXsize[0]>0) || (currSlice->listXsize[1]>0))
      printf("\n");
    if(currSlice->listXsize[0]>0)
    {
      printf(" ** (CurViewID:%d) %s Ref Pic List 0 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
      for(i=0; i<(unsigned int)(currSlice->listXsize[0]); i++)	//ref list 0
      {
        printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[0][i]->poc, currSlice->listX[0][i]->pic_num, currSlice->listX[0][i]->view_id);
      }
    }
    if(currSlice->listXsize[1]>0)
    {
      printf(" ** (CurViewID:%d) %s Ref Pic List 1 ****\n", currSlice->view_id, currSlice->structure==FRAME ? "FRM":(currSlice->structure==TOP_FIELD ? "TOP":"BOT"));
      for(i=0; i<(unsigned int)(currSlice->listXsize[1]); i++)	//ref list 1
      {
        printf("   %2d -> POC: %4d PicNum: %4d ViewID: %d\n", i, currSlice->listX[1][i]->poc, currSlice->listX[1][i]->pic_num, currSlice->listX[1][i]->view_id);
      }
    }
  }
#endif
#endif*/
}

/*!
 ************************************************************************
 * \brief
 *    Initialize reference lists depending on current slice type
 *
 ************************************************************************
 */
void init_lists(Slice *currSlice)
{
  VideoParameters *p_Vid = currSlice->p_Vid;
  DecodedPictureBuffer *p_Dpb = currSlice->p_Dpb;

  unsigned int i;
  int j;

  int list0idx = 0;
  int list0idx_1 = 0;
  int listltidx = 0;

  FrameStore **fs_list0;
  FrameStore **fs_list1;
  FrameStore **fs_listlt;

#if (MVC_EXTENSION_ENABLE)
  int currPOC = currSlice->ThisPOC;
  int curr_view_id = currSlice->view_id;
  int anchor_pic_flag = currSlice->anchor_pic_flag;

  currSlice->listinterviewidx0 = 0;
	currSlice->listinterviewidx1 = 0;
#endif


  if ((currSlice->slice_type == I_SLICE)||(currSlice->slice_type == SI_SLICE))
  {
    currSlice->listXsize[0] = 0;
    currSlice->listXsize[1] = 0;
    return;
  }

  if ((currSlice->slice_type == P_SLICE)||(currSlice->slice_type == SP_SLICE))
  {
    if (currSlice->structure == FRAME)
    {
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
          {
            currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
          }
        }
      }
      // order list 0 by PicNum
      qsort((void *)currSlice->listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_pic_num_desc);
      currSlice->listXsize[0] = (char) list0idx;
      //printf("listX[0] (PicNum): "); for (i=0; i<list0idx; i++){printf ("%d  ", currSlice->listX[0][i]->pic_num);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ltref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if (p_Dpb->fs_ltref[i]->frame->is_long_term && (p_Dpb->fs_ltref[i]->frame->view_id == curr_view_id))
#else
          if (p_Dpb->fs_ltref[i]->frame->is_long_term)
#endif
          {
            currSlice->listX[0][list0idx++]=p_Dpb->fs_ltref[i]->frame;
          }
        }
      }
      qsort((void *)&currSlice->listX[0][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
      currSlice->listXsize[0] = (char) list0idx;
    }
    else
    {
      fs_list0 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_list0)
         no_mem_exit("init_lists: fs_list0");
      fs_listlt = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_listlt)
         no_mem_exit("init_lists: fs_listlt");

      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
#if (MVC_EXTENSION_ENABLE)
        if (p_Dpb->fs_ref[i]->is_reference && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
        if (p_Dpb->fs_ref[i]->is_reference)
#endif
        {
          fs_list0[list0idx++] = p_Dpb->fs_ref[i];
        }
      }

      qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_frame_num_desc);

      //printf("fs_list0 (FrameNum): "); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list0[i]->frame_num_wrap);} printf("\n");

      currSlice->listXsize[0] = 0;
      gen_pic_list_from_frame_list(currSlice->structure, fs_list0, list0idx, currSlice->listX[0], &currSlice->listXsize[0], 0);

      //printf("listX[0] (PicNum): "); for (i=0; i < currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->pic_num);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
#if (MVC_EXTENSION_ENABLE)
        if (p_Dpb->fs_ltref[i]->view_id == curr_view_id)
#endif
        fs_listlt[listltidx++]=p_Dpb->fs_ltref[i];
      }

      qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

      gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[0], &currSlice->listXsize[0], 1);

      free(fs_list0);
      free(fs_listlt);
    }
    currSlice->listXsize[1] = 0;
  }
  else
  {
    // B-Slice
    if (currSlice->structure == FRAME)
    {
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
          {
            if (currSlice->framepoc >= p_Dpb->fs_ref[i]->frame->poc) //!KS use >= for error concealment
//            if (p_Vid->framepoc > p_Dpb->fs_ref[i]->frame->poc)
            {
              currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
            }
          }
        }
      }
      qsort((void *)currSlice->listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_poc_desc);
      list0idx_1 = list0idx;
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term) && (p_Dpb->fs_ref[i]->frame->view_id == curr_view_id))
#else
          if ((p_Dpb->fs_ref[i]->frame->used_for_reference)&&(!p_Dpb->fs_ref[i]->frame->is_long_term))
#endif
          {
            if (currSlice->framepoc < p_Dpb->fs_ref[i]->frame->poc)
            {
              currSlice->listX[0][list0idx++] = p_Dpb->fs_ref[i]->frame;
            }
          }
        }
      }
      qsort((void *)&currSlice->listX[0][list0idx_1], list0idx-list0idx_1, sizeof(StorablePicture*), compare_pic_by_poc_asc);

      for (j=0; j<list0idx_1; j++)
      {
        currSlice->listX[1][list0idx-list0idx_1+j]=currSlice->listX[0][j];
      }
      for (j=list0idx_1; j<list0idx; j++)
      {
        currSlice->listX[1][j-list0idx_1]=currSlice->listX[0][j];
      }

      currSlice->listXsize[0] = currSlice->listXsize[1] = (char) list0idx;

//      printf("currSlice->listX[0] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->poc);} printf("\n");
//      printf("currSlice->listX[1] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[1]; i++){printf ("%d  ", currSlice->listX[1][i]->poc);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ltref[i]->is_used==3)
        {
#if (MVC_EXTENSION_ENABLE)
          if (p_Dpb->fs_ltref[i]->frame->is_long_term && (p_Dpb->fs_ltref[i]->frame->view_id == curr_view_id))
#else
          if (p_Dpb->fs_ltref[i]->frame->is_long_term)
#endif
          {
            currSlice->listX[0][list0idx]   = p_Dpb->fs_ltref[i]->frame;
            currSlice->listX[1][list0idx++] = p_Dpb->fs_ltref[i]->frame;
          }
        }
      }
      qsort((void *)&currSlice->listX[0][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
      qsort((void *)&currSlice->listX[1][(short) currSlice->listXsize[0]], list0idx - currSlice->listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
      currSlice->listXsize[0] = currSlice->listXsize[1] = (char) list0idx;
    }
    else
    {
      fs_list0 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_list0)
         no_mem_exit("init_lists: fs_list0");
      fs_list1 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_list1)
         no_mem_exit("init_lists: fs_list1");
      fs_listlt = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==fs_listlt)
         no_mem_exit("init_lists: fs_listlt");

      currSlice->listXsize[0] = 0;
      currSlice->listXsize[1] = 1;

      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used)
        {
#if (MVC_EXTENSION_ENABLE)
          if (currSlice->ThisPOC >= p_Dpb->fs_ref[i]->poc && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
          if (currSlice->ThisPOC >= p_Dpb->fs_ref[i]->poc)
#endif
          {
            fs_list0[list0idx++] = p_Dpb->fs_ref[i];
          }
        }
      }
      qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_poc_desc);
      list0idx_1 = list0idx;
      for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
      {
        if (p_Dpb->fs_ref[i]->is_used)
        {
#if (MVC_EXTENSION_ENABLE)
          if (currSlice->ThisPOC < p_Dpb->fs_ref[i]->poc && (p_Dpb->fs_ref[i]->view_id == curr_view_id))
#else
          if (currSlice->ThisPOC < p_Dpb->fs_ref[i]->poc)
#endif
          {
            fs_list0[list0idx++] = p_Dpb->fs_ref[i];
          }
        }
      }
      qsort((void *)&fs_list0[list0idx_1], list0idx-list0idx_1, sizeof(FrameStore*), compare_fs_by_poc_asc);

      for (j=0; j<list0idx_1; j++)
      {
        fs_list1[list0idx-list0idx_1+j]=fs_list0[j];
      }
      for (j=list0idx_1; j<list0idx; j++)
      {
        fs_list1[j-list0idx_1]=fs_list0[j];
      }

//      printf("fs_list0 currPoc=%d (Poc): ", currSlice->ThisPOC); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list0[i]->poc);} printf("\n");
//      printf("fs_list1 currPoc=%d (Poc): ", currSlice->ThisPOC); for (i=0; i<list0idx; i++){printf ("%d  ", fs_list1[i]->poc);} printf("\n");

      currSlice->listXsize[0] = 0;
      currSlice->listXsize[1] = 0;
      gen_pic_list_from_frame_list(currSlice->structure, fs_list0, list0idx, currSlice->listX[0], &currSlice->listXsize[0], 0);
      gen_pic_list_from_frame_list(currSlice->structure, fs_list1, list0idx, currSlice->listX[1], &currSlice->listXsize[1], 0);

//      printf("currSlice->listX[0] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[0]; i++){printf ("%d  ", currSlice->listX[0][i]->poc);} printf("\n");
//      printf("currSlice->listX[1] currPoc=%d (Poc): ", p_Vid->framepoc); for (i=0; i<currSlice->listXsize[1]; i++){printf ("%d  ", currSlice->listX[1][i]->poc);} printf("\n");

      // long term handling
      for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
      {
#if (MVC_EXTENSION_ENABLE)
        if (p_Dpb->fs_ltref[i]->view_id == curr_view_id)
#endif
        fs_listlt[listltidx++]=p_Dpb->fs_ltref[i];
      }

      qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

      gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[0], &currSlice->listXsize[0], 1);
      gen_pic_list_from_frame_list(currSlice->structure, fs_listlt, listltidx, currSlice->listX[1], &currSlice->listXsize[1], 1);

      free(fs_list0);
      free(fs_list1);
      free(fs_listlt);
    }
  }

  if ((currSlice->listXsize[0] == currSlice->listXsize[1]) && (currSlice->listXsize[0] > 1))
  {
    // check if lists are identical, if yes swap first two elements of currSlice->listX[1]
    int diff=0;
    for (j = 0; j< currSlice->listXsize[0]; j++)
    {
      if (currSlice->listX[0][j]!=currSlice->listX[1][j])
        diff=1;
    }
    if (!diff)
    {
      StorablePicture *tmp_s = currSlice->listX[1][0];
      currSlice->listX[1][0]=currSlice->listX[1][1];
      currSlice->listX[1][1]=tmp_s;
    }
  }
#if (MVC_EXTENSION_ENABLE)
  if (currSlice->svc_extension_flag==0)
  {
    if ((currSlice->slice_type == P_SLICE)||(currSlice->slice_type == SP_SLICE))
    {
      currSlice->fs_listinterview0 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==currSlice->fs_listinterview0)
        no_mem_exit("init_lists: fs_listinterview0");
      list0idx = currSlice->listXsize[0];
      if (currSlice->structure == FRAME)
      {	
        append_interview_list(p_Dpb, 0, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);

        for (i=0; i<(unsigned int)currSlice->listinterviewidx0; i++)
        {
          currSlice->listX[0][list0idx++]=currSlice->fs_listinterview0[i]->frame;
        }
        currSlice->listXsize[0] = (char) list0idx;
      }
      else
      {
        append_interview_list(p_Dpb, currSlice->structure, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
        gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview0, currSlice->listinterviewidx0, currSlice->listX[0], &currSlice->listXsize[0]);
      }
    }
    else
    {
      // B-Slice
      currSlice->fs_listinterview0 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==currSlice->fs_listinterview0)
        no_mem_exit("init_lists: fs_listinterview0");
      currSlice->fs_listinterview1 = calloc(p_Dpb->size, sizeof (FrameStore*));
      if (NULL==currSlice->fs_listinterview1)
        no_mem_exit("init_lists: fs_listinterview1");
      list0idx = currSlice->listXsize[0];
      if (currSlice->structure == FRAME)
      {	
        append_interview_list(p_Dpb, 0, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
        append_interview_list(p_Dpb, 0, 1, currSlice->fs_listinterview1, &currSlice->listinterviewidx1, currPOC, curr_view_id, anchor_pic_flag);

        for (i=0; i<(unsigned int)currSlice->listinterviewidx0; i++)
        {
          currSlice->listX[0][list0idx++]=currSlice->fs_listinterview0[i]->frame;
        }
        currSlice->listXsize[0] = (char) list0idx;
        list0idx = currSlice->listXsize[1];
        for (i=0; i<(unsigned int)currSlice->listinterviewidx1; i++)
        {
          currSlice->listX[1][list0idx++]=currSlice->fs_listinterview1[i]->frame;
        }
        currSlice->listXsize[1] = (char) list0idx;
      }
      else
      {
        append_interview_list(p_Dpb, currSlice->structure, 0, currSlice->fs_listinterview0, &currSlice->listinterviewidx0, currPOC, curr_view_id, anchor_pic_flag);
        gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview0, currSlice->listinterviewidx0, currSlice->listX[0], &currSlice->listXsize[0]);
        append_interview_list(p_Dpb, currSlice->structure, 1, currSlice->fs_listinterview1, &currSlice->listinterviewidx1, currPOC, curr_view_id, anchor_pic_flag);
        gen_pic_list_from_frame_interview_list(currSlice->structure, currSlice->fs_listinterview1, currSlice->listinterviewidx1, currSlice->listX[1], &currSlice->listXsize[1]);	
      }
    }
  }
#endif
  // set max size
  currSlice->listXsize[0] = (char) imin (currSlice->listXsize[0], currSlice->num_ref_idx_active[LIST_0]);
  currSlice->listXsize[1] = (char) imin (currSlice->listXsize[1], currSlice->num_ref_idx_active[LIST_1]);

  // set the unused list entries to NULL
  for (i=currSlice->listXsize[0]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[0][i] = p_Vid->no_reference_picture;
  }
  for (i=currSlice->listXsize[1]; i< (MAX_LIST_SIZE) ; i++)
  {
    currSlice->listX[1][i] = p_Vid->no_reference_picture;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Initialize listX[2..5] from lists 0 and 1
 *    listX[2]: list0 for current_field==top
 *    listX[3]: list1 for current_field==top
 *    listX[4]: list0 for current_field==bottom
 *    listX[5]: list1 for current_field==bottom
 *
 ************************************************************************
 */
void init_mbaff_lists(VideoParameters *p_Vid, Slice *currSlice)
{
  unsigned j;
  int i;

  for (i=2;i<6;i++)
  {
    for (j=0; j<MAX_LIST_SIZE; j++)
    {
      currSlice->listX[i][j] = p_Vid->no_reference_picture;
    }
    currSlice->listXsize[i]=0;
  }

  for (i=0; i<currSlice->listXsize[0]; i++)
  {
    currSlice->listX[2][2*i  ] = currSlice->listX[0][i]->top_field;
    currSlice->listX[2][2*i+1] = currSlice->listX[0][i]->bottom_field;
    currSlice->listX[4][2*i  ] = currSlice->listX[0][i]->bottom_field;
    currSlice->listX[4][2*i+1] = currSlice->listX[0][i]->top_field;
  }
  currSlice->listXsize[2]=currSlice->listXsize[4]=currSlice->listXsize[0] * 2;

  for (i=0; i<currSlice->listXsize[1]; i++)
  {
    currSlice->listX[3][2*i  ] = currSlice->listX[1][i]->top_field;
    currSlice->listX[3][2*i+1] = currSlice->listX[1][i]->bottom_field;
    currSlice->listX[5][2*i  ] = currSlice->listX[1][i]->bottom_field;
    currSlice->listX[5][2*i+1] = currSlice->listX[1][i]->top_field;
  }
  currSlice->listXsize[3]=currSlice->listXsize[5]=currSlice->listXsize[1] * 2;
}

 /*!
 ************************************************************************
 * \brief
 *    Returns short term pic with given picNum
 *
 ************************************************************************
 */
StorablePicture*  get_short_term_pic(DecodedPictureBuffer *p_Dpb, int picNum)
{
   VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned i;

  for (i = 0; i < p_Dpb->ref_frames_in_buffer; i++)
  {
    if (p_Vid->structure == FRAME)
    {
      if (p_Dpb->fs_ref[i]->is_reference == 3)
        if ((!p_Dpb->fs_ref[i]->frame->is_long_term)&&(p_Dpb->fs_ref[i]->frame->pic_num == picNum))
          return p_Dpb->fs_ref[i]->frame;
    }
    else
    {
      if (p_Dpb->fs_ref[i]->is_reference & 1)
        if ((!p_Dpb->fs_ref[i]->top_field->is_long_term)&&(p_Dpb->fs_ref[i]->top_field->pic_num == picNum))
          return p_Dpb->fs_ref[i]->top_field;
      if (p_Dpb->fs_ref[i]->is_reference & 2)
        if ((!p_Dpb->fs_ref[i]->bottom_field->is_long_term)&&(p_Dpb->fs_ref[i]->bottom_field->pic_num == picNum))
          return p_Dpb->fs_ref[i]->bottom_field;
    }
  }

  return p_Vid->no_reference_picture;
}

/*!
 ************************************************************************
 * \brief
 *    Returns long term pic with given LongtermPicNum
 *
 ************************************************************************
 */
StorablePicture*  get_long_term_pic(DecodedPictureBuffer *p_Dpb, int LongtermPicNum)
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned i;

  for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
  {
    if (p_Vid->structure==FRAME)
    {
      if (p_Dpb->fs_ltref[i]->is_reference == 3)
        if ((p_Dpb->fs_ltref[i]->frame->is_long_term)&&(p_Dpb->fs_ltref[i]->frame->long_term_pic_num == LongtermPicNum))
          return p_Dpb->fs_ltref[i]->frame;
    }
    else
    {
      if (p_Dpb->fs_ltref[i]->is_reference & 1)
        if ((p_Dpb->fs_ltref[i]->top_field->is_long_term)&&(p_Dpb->fs_ltref[i]->top_field->long_term_pic_num == LongtermPicNum))
          return p_Dpb->fs_ltref[i]->top_field;
      if (p_Dpb->fs_ltref[i]->is_reference & 2)
        if ((p_Dpb->fs_ltref[i]->bottom_field->is_long_term)&&(p_Dpb->fs_ltref[i]->bottom_field->long_term_pic_num == LongtermPicNum))
          return p_Dpb->fs_ltref[i]->bottom_field;
    }
  }
  return NULL;
}


#if (!MVC_EXTENSION_ENABLE)
/*!
 ************************************************************************
 * \brief
 *    Reordering process for short-term reference pictures
 *
 ************************************************************************
 */
static void reorder_short_term(Slice *currSlice, int cur_list, int num_ref_idx_lX_active_minus1, int picNumLX, int *refIdxLX)
{
  StorablePicture **RefPicListX = currSlice->listX[cur_list]; 
  int cIdx, nIdx;

  StorablePicture *picLX;

  picLX = get_short_term_pic(currSlice->p_Dpb, picNumLX);

  for( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
    RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

  RefPicListX[ (*refIdxLX)++ ] = picLX;

  nIdx = *refIdxLX;

  for( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
    if (RefPicListX[ cIdx ])
    {
      if( (RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->pic_num != picNumLX ))
        RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
    }
}

/*!
 ************************************************************************
 * \brief
 *    Reordering process for long-term reference pictures
 *
 ************************************************************************
 */
static void reorder_long_term(Slice *currSlice, StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int LongTermPicNum, int *refIdxLX)
{
  int cIdx, nIdx;

  StorablePicture *picLX;

  picLX = get_long_term_pic(currSlice->p_Dpb, LongTermPicNum);

  for( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
    RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

  RefPicListX[ (*refIdxLX)++ ] = picLX;

  nIdx = *refIdxLX;

  for( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
  {
    if (RefPicListX[ cIdx ])
    {
      if( (!RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->long_term_pic_num != LongTermPicNum ))
        RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
    }
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Reordering process for reference picture lists
 *
 ************************************************************************
 */
void reorder_ref_pic_list(Slice *currSlice, int cur_list)
{
  int *reordering_of_pic_nums_idc = currSlice->reordering_of_pic_nums_idc[cur_list];
  int *abs_diff_pic_num_minus1 = currSlice->abs_diff_pic_num_minus1[cur_list];
  int *long_term_pic_idx = currSlice->long_term_pic_idx[cur_list];
  int num_ref_idx_lX_active_minus1 = currSlice->num_ref_idx_active[cur_list] - 1;

  VideoParameters *p_Vid = currSlice->p_Vid;
  int i;

  int maxPicNum, currPicNum, picNumLXNoWrap, picNumLXPred, picNumLX;
  int refIdxLX = 0;

  if (p_Vid->structure==FRAME)
  {
    maxPicNum  = p_Vid->MaxFrameNum;
    currPicNum = currSlice->frame_num;
  }
  else
  {
    maxPicNum  = 2 * p_Vid->MaxFrameNum;
    currPicNum = 2 * currSlice->frame_num + 1;
  }

  picNumLXPred = currPicNum;

  for (i=0; reordering_of_pic_nums_idc[i]!=3; i++)
  {
    if (reordering_of_pic_nums_idc[i]>3)
      error ("Invalid remapping_of_pic_nums_idc command", 500);

    if (reordering_of_pic_nums_idc[i] < 2)
    {
      if (reordering_of_pic_nums_idc[i] == 0)
      {
        if( picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) < 0 )
          picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) + maxPicNum;
        else
          picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 );
      }
      else // (remapping_of_pic_nums_idc[i] == 1)
      {
        if( picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 )  >=  maxPicNum )
          picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 ) - maxPicNum;
        else
          picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 );
      }
      picNumLXPred = picNumLXNoWrap;

      if( picNumLXNoWrap > currPicNum )
        picNumLX = picNumLXNoWrap - maxPicNum;
      else
        picNumLX = picNumLXNoWrap;

#if (MVC_EXTENSION_ENABLE)
      reorder_short_term(currSlice, cur_list, num_ref_idx_lX_active_minus1, picNumLX, &refIdxLX, -1);
#else
      reorder_short_term(currSlice, cur_list, num_ref_idx_lX_active_minus1, picNumLX, &refIdxLX);
#endif
    }
    else //(remapping_of_pic_nums_idc[i] == 2)
    {
#if (MVC_EXTENSION_ENABLE)
      reorder_long_term(currSlice, currSlice->listX[cur_list], num_ref_idx_lX_active_minus1, long_term_pic_idx[i], &refIdxLX, -1);
#else
      reorder_long_term(currSlice, currSlice->listX[cur_list], num_ref_idx_lX_active_minus1, long_term_pic_idx[i], &refIdxLX);
#endif
    }

  }
  // that's a definition
  currSlice->listXsize[cur_list] = (char) (num_ref_idx_lX_active_minus1 + 1);
}


/*!
 ************************************************************************
 * \brief
 *    Update the list of frame stores that contain reference frames/fields
 *
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
void update_ref_list(DecodedPictureBuffer *p_Dpb, int curr_view_id)
{
  unsigned i, j;
  for (i=0, j=0; i<p_Dpb->used_size; i++)
  {
    if (is_short_term_reference(p_Dpb->fs[i]) && p_Dpb->fs[i]->view_id == curr_view_id)
    {
      p_Dpb->fs_ref[j++]=p_Dpb->fs[i];
    }
  }

  p_Dpb->ref_frames_in_buffer = j;

  while (j<p_Dpb->size)
  {
    p_Dpb->fs_ref[j++]=NULL;
  }
}
#else
void update_ref_list(DecodedPictureBuffer *p_Dpb)
{
  unsigned i, j;
  for (i=0, j=0; i<p_Dpb->used_size; i++)
  {
    if (is_short_term_reference(p_Dpb->fs[i]))
    {
      p_Dpb->fs_ref[j++]=p_Dpb->fs[i];
    }
  }

  p_Dpb->ref_frames_in_buffer = j;

  while (j<p_Dpb->size)
  {
    p_Dpb->fs_ref[j++]=NULL;
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Update the list of frame stores that contain long-term reference
 *    frames/fields
 *
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
void update_ltref_list(DecodedPictureBuffer *p_Dpb, int curr_view_id)
{
  unsigned i, j;
  for (i=0, j=0; i<p_Dpb->used_size; i++)
  {
    if (is_long_term_reference(p_Dpb->fs[i]) && p_Dpb->fs[i]->view_id == curr_view_id)
    {
      p_Dpb->fs_ltref[j++] = p_Dpb->fs[i];
    }
  }

  p_Dpb->ltref_frames_in_buffer = j;

  while (j<p_Dpb->size)
  {
    p_Dpb->fs_ltref[j++]=NULL;
  }
}
#else
void update_ltref_list(DecodedPictureBuffer *p_Dpb)
{
  unsigned i, j;
  for (i=0, j=0; i<p_Dpb->used_size; i++)
  {
    if (is_long_term_reference(p_Dpb->fs[i]))
    {
      p_Dpb->fs_ltref[j++] = p_Dpb->fs[i];
    }
  }

  p_Dpb->ltref_frames_in_buffer = j;

  while (j<p_Dpb->size)
  {
    p_Dpb->fs_ltref[j++]=NULL;
  }
}
#endif

/*!
 ************************************************************************
 * \brief
 *    Perform Memory management for idr pictures
 *
 ************************************************************************
 */
//static void idr_memory_management(DecodedPictureBuffer *p_Dpb, StorablePicture* p)
//{
//  unsigned i;
//#if (MVC_EXTENSION_ENABLE)
//  VideoParameters *p_Vid = p_Dpb->p_Vid;
//  int size = 0;
//	int iVOIdx = GetVOIdx(p_Vid, p->view_id);
//  int svc_extension_flag = p_Vid->ppSliceList[0]->svc_extension_flag;
//#endif
//
//  assert (p->idr_flag);
//
//  if (p->no_output_of_prior_pics_flag)
//  {
//    // free all stored pictures
//    for (i=0; i<p_Dpb->used_size; i++)
//    {
//#if (MVC_EXTENSION_ENABLE)
//      if (svc_extension_flag == 0 || p_Dpb->fs[i]->view_id == p->view_id)
//      {
//        free_frame_store(p_Dpb->fs[i]);
//        p_Dpb->fs[i] = alloc_frame_store();
//        size++;
//      }
//#else
//      // reset all reference settings
//      free_frame_store(p_Dpb->fs[i]);
//      p_Dpb->fs[i] = alloc_frame_store();
//#endif
//    }
//    for (i = 0; i < p_Dpb->ref_frames_in_buffer; i++)
//    {
//#if (MVC_EXTENSION_ENABLE)
//      if (svc_extension_flag == 0 || p_Dpb->fs_ref[i]->view_id == p->view_id)
//#endif
//      p_Dpb->fs_ref[i]=NULL;
//    }
//    for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
//    {
//#if (MVC_EXTENSION_ENABLE)
//      if (svc_extension_flag == 0 || p_Dpb->fs_ltref[i]->view_id == p->view_id)
//#endif
//      p_Dpb->fs_ltref[i]=NULL;
//    }
//#if (MVC_EXTENSION_ENABLE)
//    p_Dpb->used_size -= size;
//#else
//    p_Dpb->used_size=0;
//#endif
//  }
//  else
//  {
//#if (MVC_EXTENSION_ENABLE)
//    
//    if(p_Vid->profile_idc == MVC_HIGH || p_Vid->profile_idc == STEREO_HIGH) //if (svc_extension_flag == 0)
//      flush_dpb(p_Dpb, -1);
//    else
//      flush_dpb(p_Dpb, p->view_id);
//#else
//    flush_dpb(p_Dpb);
//#endif
//  }
//  p_Dpb->last_picture = NULL;
//
//#if (MVC_EXTENSION_ENABLE)
//  update_ref_list(p_Dpb, p->view_id);
//	update_ltref_list(p_Dpb, p->view_id);
//	p_Dpb->last_output_poc = INT_MIN;
//	p_Dpb->last_output_view_id = -1;
//
//	if (p->long_term_reference_flag)
//	{
//		p_Dpb->max_long_term_pic_idx[iVOIdx] = 0;
//		p->is_long_term           = 1;
//		p->long_term_frame_idx    = 0;
//	}
//	else
//	{
//		p_Dpb->max_long_term_pic_idx[iVOIdx] = -1;
//		p->is_long_term           = 0;
//	}
//#else
//  update_ref_list(p_Dpb);
//  update_ltref_list(p_Dpb);
//  p_Dpb->last_output_poc = INT_MIN;
//
//  if (p->long_term_reference_flag)
//  {
//    p_Dpb->max_long_term_pic_idx = 0;
//    p->is_long_term           = 1;
//    p->long_term_frame_idx    = 0;
//  }
//  else
//  {
//    p_Dpb->max_long_term_pic_idx = -1;
//    p->is_long_term           = 0;
//  }
//#endif
//}

/*!
 ************************************************************************
 * \brief
 *    Perform Sliding window decoded reference picture marking process
 *
 ************************************************************************
 */
static void sliding_window_memory_management(DecodedPictureBuffer *p_Dpb, StorablePicture* p)
{
  unsigned i;

  assert (!p->idr_flag);
  // if this is a reference pic with sliding sliding window, unmark first ref frame
  if (p_Dpb->ref_frames_in_buffer==p_Dpb->num_ref_frames - p_Dpb->ltref_frames_in_buffer)
  {
#if (MVC_EXTENSION_ENABLE)
    if ( p_Dpb->p_Vid->profile_idc == MVC_HIGH || p_Dpb->p_Vid->profile_idc == STEREO_HIGH )
    {
      for (i=0; i<p_Dpb->used_size;i++)
      {
        if (p_Dpb->fs[i]->is_reference && (!(p_Dpb->fs[i]->is_long_term)) && p_Dpb->fs[i]->view_id == p->view_id)
        {
          unmark_for_reference(p_Dpb->fs[i]);
          update_ref_list(p_Dpb, p->view_id);
          break;
        }
      }
    }
    else
    {
      for (i=0; i<p_Dpb->used_size;i++)
      {
        if (p_Dpb->fs[i]->is_reference && (!(p_Dpb->fs[i]->is_long_term)))
        {
          unmark_for_reference(p_Dpb->fs[i]);
          update_ref_list(p_Dpb, p->view_id);
          break;
        }
      }
    }
#else
    for (i=0; i<p_Dpb->used_size;i++)
    {
      if (p_Dpb->fs[i]->is_reference && (!(p_Dpb->fs[i]->is_long_term)))
      {
        unmark_for_reference(p_Dpb->fs[i]);
        update_ref_list(p_Dpb);
        break;
      }
    }
#endif
  }

  p->is_long_term = 0;
}

/*!
 ************************************************************************
 * \brief
 *    Calculate picNumX
 ************************************************************************
 */
static int get_pic_num_x (StorablePicture *p, int difference_of_pic_nums_minus1)
{
  int currPicNum;

  if (p->structure == FRAME)
    currPicNum = p->frame_num;
  else
    currPicNum = 2 * p->frame_num + 1;

  return currPicNum - (difference_of_pic_nums_minus1 + 1);
}


/*!
 ************************************************************************
 * \brief
 *    Adaptive Memory Management: Mark short term picture unused
 ************************************************************************
 */
static void mm_unmark_short_term_for_reference(DecodedPictureBuffer *p_Dpb, StorablePicture *p, int difference_of_pic_nums_minus1)
{
  int picNumX;

  unsigned i;

  picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

  for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
  {
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs_ref[i]->view_id == p->view_id)
		{
#endif
    if (p->structure == FRAME)
    {
      if ((p_Dpb->fs_ref[i]->is_reference==3) && (p_Dpb->fs_ref[i]->is_long_term==0))
      {
        if (p_Dpb->fs_ref[i]->frame->pic_num == picNumX)
        {
          unmark_for_reference(p_Dpb->fs_ref[i]);
          return;
        }
      }
    }
    else
    {
      if ((p_Dpb->fs_ref[i]->is_reference & 1) && (!(p_Dpb->fs_ref[i]->is_long_term & 1)))
      {
        if (p_Dpb->fs_ref[i]->top_field->pic_num == picNumX)
        {
          p_Dpb->fs_ref[i]->top_field->used_for_reference = 0;
          p_Dpb->fs_ref[i]->is_reference &= 2;
          if (p_Dpb->fs_ref[i]->is_used == 3)
          {
            p_Dpb->fs_ref[i]->frame->used_for_reference = 0;
          }
          return;
        }
      }
      if ((p_Dpb->fs_ref[i]->is_reference & 2) && (!(p_Dpb->fs_ref[i]->is_long_term & 2)))
      {
        if (p_Dpb->fs_ref[i]->bottom_field->pic_num == picNumX)
        {
          p_Dpb->fs_ref[i]->bottom_field->used_for_reference = 0;
          p_Dpb->fs_ref[i]->is_reference &= 1;
          if (p_Dpb->fs_ref[i]->is_used == 3)
          {
            p_Dpb->fs_ref[i]->frame->used_for_reference = 0;
          }
          return;
        }
      }
    }
#if (MVC_EXTENSION_ENABLE)
    }
#endif
  }
}


/*!
 ************************************************************************
 * \brief
 *    Adaptive Memory Management: Mark long term picture unused
 ************************************************************************
 */
static void mm_unmark_long_term_for_reference(DecodedPictureBuffer *p_Dpb, StorablePicture *p, int long_term_pic_num)
{
  unsigned i;
  for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
  {
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs_ltref[i]->view_id == p->view_id)
		{
#endif
    if (p->structure == FRAME)
    {
      if ((p_Dpb->fs_ltref[i]->is_reference==3) && (p_Dpb->fs_ltref[i]->is_long_term==3))
      {
        if (p_Dpb->fs_ltref[i]->frame->long_term_pic_num == long_term_pic_num)
        {
          unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
        }
      }
    }
    else
    {
      if ((p_Dpb->fs_ltref[i]->is_reference & 1) && ((p_Dpb->fs_ltref[i]->is_long_term & 1)))
      {
        if (p_Dpb->fs_ltref[i]->top_field->long_term_pic_num == long_term_pic_num)
        {
          p_Dpb->fs_ltref[i]->top_field->used_for_reference = 0;
          p_Dpb->fs_ltref[i]->top_field->is_long_term = 0;
          p_Dpb->fs_ltref[i]->is_reference &= 2;
          p_Dpb->fs_ltref[i]->is_long_term &= 2;
          if (p_Dpb->fs_ltref[i]->is_used == 3)
          {
            p_Dpb->fs_ltref[i]->frame->used_for_reference = 0;
            p_Dpb->fs_ltref[i]->frame->is_long_term = 0;
          }
          return;
        }
      }
      if ((p_Dpb->fs_ltref[i]->is_reference & 2) && ((p_Dpb->fs_ltref[i]->is_long_term & 2)))
      {
        if (p_Dpb->fs_ltref[i]->bottom_field->long_term_pic_num == long_term_pic_num)
        {
          p_Dpb->fs_ltref[i]->bottom_field->used_for_reference = 0;
          p_Dpb->fs_ltref[i]->bottom_field->is_long_term = 0;
          p_Dpb->fs_ltref[i]->is_reference &= 1;
          p_Dpb->fs_ltref[i]->is_long_term &= 1;
          if (p_Dpb->fs_ltref[i]->is_used == 3)
          {
            p_Dpb->fs_ltref[i]->frame->used_for_reference = 0;
            p_Dpb->fs_ltref[i]->frame->is_long_term = 0;
          }
          return;
        }
      }
    }
#if (MVC_EXTENSION_ENABLE)
    }
#endif
  }
}


/*!
 ************************************************************************
 * \brief
 *    Mark a long-term reference frame or complementary field pair unused for referemce
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
static void unmark_long_term_frame_for_reference_by_frame_idx(DecodedPictureBuffer *p_Dpb, int long_term_frame_idx, int view_id)
#else
static void unmark_long_term_frame_for_reference_by_frame_idx(DecodedPictureBuffer *p_Dpb, int long_term_frame_idx)
#endif
{
  unsigned i;
  for(i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
  {
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs_ltref[i]->long_term_frame_idx == long_term_frame_idx && p_Dpb->fs_ltref[i]->view_id == view_id)
#else
    if (p_Dpb->fs_ltref[i]->long_term_frame_idx == long_term_frame_idx)
#endif
      unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Mark a long-term reference field unused for reference only if it's not
 *    the complementary field of the picture indicated by picNumX
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
static void unmark_long_term_field_for_reference_by_frame_idx(DecodedPictureBuffer *p_Dpb, PictureStructure structure, int long_term_frame_idx, int mark_current, unsigned curr_frame_num, int curr_pic_num, int curr_view_id)
#else
static void unmark_long_term_field_for_reference_by_frame_idx(DecodedPictureBuffer *p_Dpb, PictureStructure structure, int long_term_frame_idx, int mark_current, unsigned curr_frame_num, int curr_pic_num)
#endif
{
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  unsigned i;

  assert(structure!=FRAME);
  if (curr_pic_num<0)
    curr_pic_num += (2 * p_Vid->MaxFrameNum);

  for(i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
  {
#if (MVC_EXTENSION_ENABLE)
    if (p_Dpb->fs_ltref[i]->view_id == curr_view_id)
    {
#endif
    if (p_Dpb->fs_ltref[i]->long_term_frame_idx == long_term_frame_idx)
    {
      if (structure == TOP_FIELD)
      {
        if ((p_Dpb->fs_ltref[i]->is_long_term == 3))
        {
          unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
        }
        else
        {
          if ((p_Dpb->fs_ltref[i]->is_long_term == 1))
          {
            unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
          }
          else
          {
            if (mark_current)
            {
              if (p_Dpb->last_picture)
              {
                if ( ( p_Dpb->last_picture != p_Dpb->fs_ltref[i] )|| p_Dpb->last_picture->frame_num != curr_frame_num)
                  unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
              else
              {
                unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
            }
            else
            {
              if ((p_Dpb->fs_ltref[i]->frame_num) != (unsigned)(curr_pic_num >> 1))
              {
                unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
            }
          }
        }
      }
      if (structure == BOTTOM_FIELD)
      {
        if ((p_Dpb->fs_ltref[i]->is_long_term == 3))
        {
          unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
        }
        else
        {
          if ((p_Dpb->fs_ltref[i]->is_long_term == 2))
          {
            unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
          }
          else
          {
            if (mark_current)
            {
              if (p_Dpb->last_picture)
              {
                if ( ( p_Dpb->last_picture != p_Dpb->fs_ltref[i] )|| p_Dpb->last_picture->frame_num != curr_frame_num)
                  unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
              else
              {
                unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
            }
            else
            {
              if ((p_Dpb->fs_ltref[i]->frame_num) != (unsigned)(curr_pic_num >> 1))
              {
                unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
              }
            }
          }
        }
      }
    }
#if (MVC_EXTENSION_ENABLE)
    }
#endif
  }
}


/*!
 ************************************************************************
 * \brief
 *    mark a picture as long-term reference
 ************************************************************************
 */
static void mark_pic_long_term(DecodedPictureBuffer *p_Dpb, StorablePicture* p, int long_term_frame_idx, int picNumX)
{
  unsigned i;
  int add_top, add_bottom;

  if (p->structure == FRAME)
  {
    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
#if (MVC_EXTENSION_ENABLE)
      if (p_Dpb->fs_ref[i]->view_id == p->view_id)
      {
#endif
      if (p_Dpb->fs_ref[i]->is_reference == 3)
      {
        if ((!p_Dpb->fs_ref[i]->frame->is_long_term)&&(p_Dpb->fs_ref[i]->frame->pic_num == picNumX))
        {
          p_Dpb->fs_ref[i]->long_term_frame_idx = p_Dpb->fs_ref[i]->frame->long_term_frame_idx
                                             = long_term_frame_idx;
          p_Dpb->fs_ref[i]->frame->long_term_pic_num = long_term_frame_idx;
          p_Dpb->fs_ref[i]->frame->is_long_term = 1;

          if (p_Dpb->fs_ref[i]->top_field && p_Dpb->fs_ref[i]->bottom_field)
          {
            p_Dpb->fs_ref[i]->top_field->long_term_frame_idx = p_Dpb->fs_ref[i]->bottom_field->long_term_frame_idx
                                                          = long_term_frame_idx;
            p_Dpb->fs_ref[i]->top_field->long_term_pic_num = long_term_frame_idx;
            p_Dpb->fs_ref[i]->bottom_field->long_term_pic_num = long_term_frame_idx;

            p_Dpb->fs_ref[i]->top_field->is_long_term = p_Dpb->fs_ref[i]->bottom_field->is_long_term
                                                   = 1;

          }
          p_Dpb->fs_ref[i]->is_long_term = 3;
          return;
        }
      }
#if (MVC_EXTENSION_ENABLE)
      }
#endif
    }
    printf ("Warning: reference frame for long term marking not found\n");
  }
  else
  {
    if (p->structure == TOP_FIELD)
    {
      add_top    = 1;
      add_bottom = 0;
    }
    else
    {
      add_top    = 0;
      add_bottom = 1;
    }
    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
#if (MVC_EXTENSION_ENABLE)
      if (p_Dpb->fs_ref[i]->view_id == p->view_id)
      {
#endif
      if (p_Dpb->fs_ref[i]->is_reference & 1)
      {
        if ((!p_Dpb->fs_ref[i]->top_field->is_long_term)&&(p_Dpb->fs_ref[i]->top_field->pic_num == picNumX))
        {
          if ((p_Dpb->fs_ref[i]->is_long_term) && (p_Dpb->fs_ref[i]->long_term_frame_idx != long_term_frame_idx))
          {
              printf ("Warning: assigning long_term_frame_idx different from other field\n");
          }

          p_Dpb->fs_ref[i]->long_term_frame_idx = p_Dpb->fs_ref[i]->top_field->long_term_frame_idx
                                             = long_term_frame_idx;
          p_Dpb->fs_ref[i]->top_field->long_term_pic_num = 2 * long_term_frame_idx + add_top;
          p_Dpb->fs_ref[i]->top_field->is_long_term = 1;
          p_Dpb->fs_ref[i]->is_long_term |= 1;
          if (p_Dpb->fs_ref[i]->is_long_term == 3)
          {
            p_Dpb->fs_ref[i]->frame->is_long_term = 1;
            p_Dpb->fs_ref[i]->frame->long_term_frame_idx = p_Dpb->fs_ref[i]->frame->long_term_pic_num = long_term_frame_idx;
          }
          return;
        }
      }
      if (p_Dpb->fs_ref[i]->is_reference & 2)
      {
        if ((!p_Dpb->fs_ref[i]->bottom_field->is_long_term)&&(p_Dpb->fs_ref[i]->bottom_field->pic_num == picNumX))
        {
          if ((p_Dpb->fs_ref[i]->is_long_term) && (p_Dpb->fs_ref[i]->long_term_frame_idx != long_term_frame_idx))
          {
              printf ("Warning: assigning long_term_frame_idx different from other field\n");
          }

          p_Dpb->fs_ref[i]->long_term_frame_idx = p_Dpb->fs_ref[i]->bottom_field->long_term_frame_idx
                                             = long_term_frame_idx;
          p_Dpb->fs_ref[i]->bottom_field->long_term_pic_num = 2 * long_term_frame_idx + add_bottom;
          p_Dpb->fs_ref[i]->bottom_field->is_long_term = 1;
          p_Dpb->fs_ref[i]->is_long_term |= 2;
          if (p_Dpb->fs_ref[i]->is_long_term == 3)
          {
            p_Dpb->fs_ref[i]->frame->is_long_term = 1;
            p_Dpb->fs_ref[i]->frame->long_term_frame_idx = p_Dpb->fs_ref[i]->frame->long_term_pic_num = long_term_frame_idx;
          }
          return;
        }
      }
#if (MVC_EXTENSION_ENABLE)
      }
#endif
    }
    printf ("Warning: reference field for long term marking not found\n");
  }
}


/*!
 ************************************************************************
 * \brief
 *    Assign a long term frame index to a short term picture
 ************************************************************************
 */
static void mm_assign_long_term_frame_idx(DecodedPictureBuffer *p_Dpb, StorablePicture* p, int difference_of_pic_nums_minus1, int long_term_frame_idx)
{
  int picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

  // remove frames/fields with same long_term_frame_idx
  if (p->structure == FRAME)
  {
#if (MVC_EXTENSION_ENABLE)
    unmark_long_term_frame_for_reference_by_frame_idx(p_Dpb, long_term_frame_idx, p->view_id);
#else
    unmark_long_term_frame_for_reference_by_frame_idx(p_Dpb, long_term_frame_idx);
#endif
  }
  else
  {
    unsigned i;
    PictureStructure structure = FRAME;

    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
    {
#if (MVC_EXTENSION_ENABLE)
      if (p_Dpb->fs_ref[i]->view_id == p->view_id)
			{
#endif
      if (p_Dpb->fs_ref[i]->is_reference & 1)
      {
        if (p_Dpb->fs_ref[i]->top_field->pic_num == picNumX)
        {
          structure = TOP_FIELD;
          break;
        }
      }
      if (p_Dpb->fs_ref[i]->is_reference & 2)
      {
        if (p_Dpb->fs_ref[i]->bottom_field->pic_num == picNumX)
        {
          structure = BOTTOM_FIELD;
          break;
        }
      }
#if (MVC_EXTENSION_ENABLE)
      }
#endif
    }
    if (structure==FRAME)
    {
      error ("field for long term marking not found",200);
    }

#if (MVC_EXTENSION_ENABLE)
    unmark_long_term_field_for_reference_by_frame_idx(p_Dpb, structure, long_term_frame_idx, 0, 0, picNumX, p->view_id);
#else
    unmark_long_term_field_for_reference_by_frame_idx(p_Dpb, structure, long_term_frame_idx, 0, 0, picNumX);
#endif
  }

  mark_pic_long_term(p_Dpb, p, long_term_frame_idx, picNumX);
}

/*!
 ************************************************************************
 * \brief
 *    Set new max long_term_frame_idx
 ************************************************************************
 */
//#if (MVC_EXTENSION_ENABLE)
//void mm_update_max_long_term_frame_idx(DecodedPictureBuffer *p_Dpb, int max_long_term_frame_idx_plus1, int curr_view_id)
//#else
//void mm_update_max_long_term_frame_idx(DecodedPictureBuffer *p_Dpb, int max_long_term_frame_idx_plus1)
//#endif
//{
//  unsigned i;
//
//#if (MVC_EXTENSION_ENABLE)
//  int iVOIdx = GetVOIdx(p_Dpb->p_Vid, curr_view_id);
//
//  p_Dpb->max_long_term_pic_idx[iVOIdx] = max_long_term_frame_idx_plus1 - 1;
//#else
//  p_Dpb->max_long_term_pic_idx = max_long_term_frame_idx_plus1 - 1;
//#endif
//
//  // check for invalid frames
//  for (i=0; i<p_Dpb->ltref_frames_in_buffer; i++)
//  {
//#if (MVC_EXTENSION_ENABLE)
//    if (p_Dpb->fs_ltref[i]->long_term_frame_idx > p_Dpb->max_long_term_pic_idx[iVOIdx] && p_Dpb->fs_ltref[i]->view_id == curr_view_id)
//#else
//    if (p_Dpb->fs_ltref[i]->long_term_frame_idx > p_Dpb->max_long_term_pic_idx)
//#endif
//    {
//      unmark_for_long_term_reference(p_Dpb->fs_ltref[i]);
//    }
//  }
//}
//

/*!
 ************************************************************************
 * \brief
 *    Mark all long term reference pictures unused for reference
 ************************************************************************
 */
//#if (MVC_EXTENSION_ENABLE)
//static void mm_unmark_all_long_term_for_reference (DecodedPictureBuffer *p_Dpb, int curr_view_id)
//{
//  mm_update_max_long_term_frame_idx(p_Dpb, 0, curr_view_id);
//}
//#else
//static void mm_unmark_all_long_term_for_reference (DecodedPictureBuffer *p_Dpb)
//{
//  mm_update_max_long_term_frame_idx(p_Dpb, 0);
//}
//#endif

/*!
 ************************************************************************
 * \brief
 *    Mark all short term reference pictures unused for reference
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
static void mm_unmark_all_short_term_for_reference (DecodedPictureBuffer *p_Dpb, int curr_view_id)
{
  unsigned int i;
  for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
  {
    if (p_Dpb->fs_ref[i]->view_id == curr_view_id)
    {
      unmark_for_reference(p_Dpb->fs_ref[i]);
    }
  }
  update_ref_list(p_Dpb, curr_view_id);
}
#else
static void mm_unmark_all_short_term_for_reference (DecodedPictureBuffer *p_Dpb)
{
  unsigned int i;
  for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
  {
    unmark_for_reference(p_Dpb->fs_ref[i]);
  }
  update_ref_list(p_Dpb);
}
#endif


/*!
 ************************************************************************
 * \brief
 *    Mark the current picture used for long term reference
 ************************************************************************
 */
static void mm_mark_current_picture_long_term(DecodedPictureBuffer *p_Dpb, StorablePicture *p, int long_term_frame_idx)
{
  // remove long term pictures with same long_term_frame_idx
#if (MVC_EXTENSION_ENABLE)
  if (p->structure == FRAME)
  {
    unmark_long_term_frame_for_reference_by_frame_idx(p_Dpb, long_term_frame_idx, p->view_id);
  }
  else
  {
    unmark_long_term_field_for_reference_by_frame_idx(p_Dpb, p->structure, long_term_frame_idx, 1, p->pic_num, 0, p->view_id);
  }
#else
  if (p->structure == FRAME)
  {
    unmark_long_term_frame_for_reference_by_frame_idx(p_Dpb, long_term_frame_idx);
  }
  else
  {
    unmark_long_term_field_for_reference_by_frame_idx(p_Dpb, p->structure, long_term_frame_idx, 1, p->pic_num, 0);
  }
#endif

  p->is_long_term = 1;
  p->long_term_frame_idx = long_term_frame_idx;
}


/*!
 ************************************************************************
 * \brief
 *    Perform Adaptive memory control decoded reference picture marking process
 ************************************************************************
 */
//static void adaptive_memory_management(DecodedPictureBuffer *p_Dpb, StorablePicture* p)
//{
//  DecRefPicMarking_t *tmp_drpm;
//  VideoParameters *p_Vid = p_Dpb->p_Vid;
//
//  p_Vid->last_has_mmco_5 = 0;
//
//  assert (!p->idr_flag);
//  assert (p->adaptive_ref_pic_buffering_flag);
//
//  while (p->dec_ref_pic_marking_buffer)
//  {
//    tmp_drpm = p->dec_ref_pic_marking_buffer;
//    switch (tmp_drpm->memory_management_control_operation)
//    {
//      case 0:
//        if (tmp_drpm->Next != NULL)
//        {
//          error ("memory_management_control_operation = 0 not last operation in buffer", 500);
//        }
//        break;
//      case 1:
//        mm_unmark_short_term_for_reference(p_Dpb, p, tmp_drpm->difference_of_pic_nums_minus1);
//#if (MVC_EXTENSION_ENABLE)
//        update_ref_list(p_Dpb, p->view_id);
//#else
//        update_ref_list(p_Dpb);
//#endif
//        break;
//      case 2:
//        mm_unmark_long_term_for_reference(p_Dpb, p, tmp_drpm->long_term_pic_num);
//#if (MVC_EXTENSION_ENABLE)
//        update_ltref_list(p_Dpb, p->view_id);
//#else
//        update_ltref_list(p_Dpb);
//#endif
//        break;
//      case 3:
//        mm_assign_long_term_frame_idx(p_Dpb, p, tmp_drpm->difference_of_pic_nums_minus1, tmp_drpm->long_term_frame_idx);
//#if (MVC_EXTENSION_ENABLE)
//        update_ref_list(p_Dpb, p->view_id);
//        update_ltref_list(p_Dpb, p->view_id);
//#else
//        update_ref_list(p_Dpb);
//        update_ltref_list(p_Dpb);
//#endif
//        break;
//      case 4:
//#if (MVC_EXTENSION_ENABLE)
//        mm_update_max_long_term_frame_idx (p_Dpb, tmp_drpm->max_long_term_frame_idx_plus1, p->view_id);
//        update_ltref_list(p_Dpb, p->view_id);
//#else
//        mm_update_max_long_term_frame_idx (p_Dpb, tmp_drpm->max_long_term_frame_idx_plus1);
//        update_ltref_list(p_Dpb);
//#endif
//        break;
//      case 5:
//#if (MVC_EXTENSION_ENABLE)
//        mm_unmark_all_short_term_for_reference(p_Dpb, p->view_id);
//        mm_unmark_all_long_term_for_reference(p_Dpb, p->view_id);
//#else
//        mm_unmark_all_short_term_for_reference(p_Dpb);
//        mm_unmark_all_long_term_for_reference(p_Dpb);
//#endif
//       p_Vid->last_has_mmco_5 = 1;
//        break;
//      case 6:
//        mm_mark_current_picture_long_term(p_Dpb, p, tmp_drpm->long_term_frame_idx);
//        check_num_ref(p_Dpb);
//        break;
//      default:
//        error ("invalid memory_management_control_operation in buffer", 500);
//    }
//    p->dec_ref_pic_marking_buffer = tmp_drpm->Next;
//    free (tmp_drpm);
//  }
//  if ( p_Vid->last_has_mmco_5 )
//  {
//    p->pic_num = p->frame_num = 0;
//
//    switch (p->structure)
//    {
//    case TOP_FIELD:
//      {
//        //p->poc = p->top_poc = p_Vid->toppoc =0;
//        p->poc = p->top_poc = 0;
//        break;
//      }
//    case BOTTOM_FIELD:
//      {
//        //p->poc = p->bottom_poc = p_Vid->bottompoc = 0;
//        p->poc = p->bottom_poc = 0;
//        break;
//      }
//    case FRAME:
//      {
//        p->top_poc    -= p->poc;
//        p->bottom_poc -= p->poc;
//
//        //p_Vid->toppoc = p->top_poc;
//        //p_Vid->bottompoc = p->bottom_poc;
//
//        p->poc = imin (p->top_poc, p->bottom_poc);
//        //p_Vid->framepoc = p->poc;
//        break;
//      }
//    }
//    //currSlice->ThisPOC = p->poc;
//#if (MVC_EXTENSION_ENABLE)
//    flush_dpb(p_Dpb, p->view_id);
//#else
//    flush_dpb(p_Dpb);
//#endif
//  }
//}
//

/*!
 ************************************************************************
 * \brief
 *    Store a picture in DPB. This includes cheking for space in DPB and
 *    flushing frames.
 *    If we received a frame, we need to check for a new store, if we
 *    got a field, check if it's the second field of an already allocated
 *    store.
 *
 * \param p_Vid
 *    VideoParameters
 * \param p
 *    Picture to be stored
 *
 ************************************************************************
 */
//void store_picture_in_dpb(DecodedPictureBuffer *p_Dpb, StorablePicture* p)
//{
//  VideoParameters *p_Vid = p_Dpb->p_Vid;
//  unsigned i;
//  int poc, pos;
//  // picture error concealment
//  
//  // diagnostics
//  //printf ("Storing (%s) non-ref pic with frame_num #%d\n", (p->type == FRAME)?"FRAME":(p->type == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", p->pic_num);
//  // if frame, check for new store,
//  assert (p!=NULL);
//
//  p_Vid->last_has_mmco_5=0;
//  p_Vid->last_pic_bottom_field = (p->structure == BOTTOM_FIELD);
//
//  if (p->idr_flag)
//  {
//    idr_memory_management(p_Dpb, p);
//  // picture error concealment
//    memset(p_Vid->pocs_in_dpb, 0, sizeof(int)*100);
//  }
//  else
//  {
//    // adaptive memory management
//    if (p->used_for_reference && (p->adaptive_ref_pic_buffering_flag))
//      adaptive_memory_management(p_Dpb, p);
//  }
//
//  if ((p->structure==TOP_FIELD)||(p->structure==BOTTOM_FIELD))
//  {
//    // check for frame store with same pic_number
//#if (MVC_EXTENSION_ENABLE)
//    if (!(p_Dpb->last_picture && p_Dpb->last_picture->view_id == p->view_id))
//		{
//			for(i=0; i<p_Dpb->used_size; i++)
//			{
//				if (p_Dpb->fs[i]->view_id == p->view_id && ((p->structure==TOP_FIELD && p_Dpb->fs[i]->is_used == 2) || (p->structure==BOTTOM_FIELD && p_Dpb->fs[i]->is_used == 1)))
//					break;
//			}
//			if (i < p_Dpb->used_size)
//      {
//				p_Dpb->last_picture = p_Dpb->fs[i];
//      }
//		}
//
//    if (p_Dpb->last_picture && p_Dpb->last_picture->view_id == p->view_id)
//#else
//    if (p_Dpb->last_picture)
//#endif
//    {
//      if ((int)p_Dpb->last_picture->frame_num == p->pic_num)
//      {
//        if (((p->structure==TOP_FIELD)&&(p_Dpb->last_picture->is_used==2))||((p->structure==BOTTOM_FIELD)&&(p_Dpb->last_picture->is_used==1)))
//        {
//          if ((p->used_for_reference && (p_Dpb->last_picture->is_orig_reference!=0))||
//              (!p->used_for_reference && (p_Dpb->last_picture->is_orig_reference==0)))
//          {
//            insert_picture_in_dpb(p_Vid, p_Dpb->last_picture, p);
//#if (MVC_EXTENSION_ENABLE)
//            update_ref_list(p_Dpb, p->view_id);
//            update_ltref_list(p_Dpb, p->view_id);
//#else
//            update_ref_list(p_Dpb);
//            update_ltref_list(p_Dpb);
//#endif
//            dump_dpb(p_Dpb);
//            p_Dpb->last_picture = NULL;
//            return;
//          }
//        }
//      }
//    }
//  }
//
//  // this is a frame or a field which has no stored complementary field
//
//  // sliding window, if necessary
//  if ((!p->idr_flag)&&(p->used_for_reference && (!p->adaptive_ref_pic_buffering_flag)))
//  {
//    sliding_window_memory_management(p_Dpb, p);
//  }
//
//  // picture error concealment
//  if(p_Vid->conceal_mode != 0)
//    for(i=0;i<p_Dpb->size;i++)
//      if(p_Dpb->fs[i]->is_reference)
//        p_Dpb->fs[i]->concealment_reference = 1;
//
//  // first try to remove unused frames
//  if (p_Dpb->used_size==p_Dpb->size)
//  {
//    // picture error concealment
//    if (p_Vid->conceal_mode != 0)
//      conceal_non_ref_pics(p_Dpb, 2);
//
//#if (MVC_EXTENSION_ENABLE)
//    remove_unused_frame_from_dpb(p_Dpb, p->view_id);
//#else
//    remove_unused_frame_from_dpb(p_Dpb);
//#endif
//
//    if(p_Vid->conceal_mode != 0)
//      sliding_window_poc_management(p_Dpb, p);
//  }
//  
//  // then output frames until one can be removed
//  while (p_Dpb->used_size == p_Dpb->size)
//  {
//    // non-reference frames may be output directly
//    if (!p->used_for_reference)
//    {
//#if (MVC_EXTENSION_ENABLE)
//      get_smallest_poc(p_Dpb, &poc, &pos, p->view_id);
//#else
//      get_smallest_poc(p_Dpb, &poc, &pos);
//#endif
//      if ((-1==pos) || (p->poc < poc))
//      {
//        direct_output(p_Vid, p, p_Vid->p_out);
//        return;
//      }
//    }
//    // flush a frame
//#if (MVC_EXTENSION_ENABLE)
//    output_one_frame_from_dpb(p_Dpb, p->view_id);
//#else
//    output_one_frame_from_dpb(p_Dpb);
//#endif
//  }
//
//  // check for duplicate frame number in short term reference buffer
//  if ((p->used_for_reference)&&(!p->is_long_term))
//  {
//    for (i=0; i<p_Dpb->ref_frames_in_buffer; i++)
//    {
//#if (MVC_EXTENSION_ENABLE)
//      if (p_Dpb->fs_ref[i]->frame_num == p->frame_num && p_Dpb->fs_ref[i]->view_id == p->view_id)
//#else
//      if (p_Dpb->fs_ref[i]->frame_num == p->frame_num)
//#endif
//      {
//        error("duplicate frame_num in short-term reference picture buffer", 500);
//      }
//    }
//
//  }
//  // store at end of buffer
//  insert_picture_in_dpb(p_Vid, p_Dpb->fs[p_Dpb->used_size],p);
//
//  // picture error concealment
//  if (p->idr_flag)
//  {
//    p_Vid->earlier_missing_poc = 0;
//  }
//
//  if (p->structure != FRAME)
//  {
//    p_Dpb->last_picture = p_Dpb->fs[p_Dpb->used_size];
//  }
//  else
//  {
//    p_Dpb->last_picture = NULL;
//  }
//
//  p_Dpb->used_size++;
//
//  if(p_Vid->conceal_mode != 0)
//    p_Vid->pocs_in_dpb[p_Dpb->used_size-1] = p->poc;
//
//#if (MVC_EXTENSION_ENABLE)
//  update_ref_list(p_Dpb, p->view_id);
//  update_ltref_list(p_Dpb, p->view_id);
//#else
//  update_ref_list(p_Dpb);
//  update_ltref_list(p_Dpb);
//#endif
//
//  check_num_ref(p_Dpb);
//
//  dump_dpb(p_Dpb);
//}
//

/*!
 ************************************************************************
 * \brief
 *    Insert the picture into the DPB. A free DPB position is necessary
 *    for frames, .
 *
 * \param p_Vid
 *    VideoParameters
 * \param fs
 *    FrameStore into which the picture will be inserted
 * \param p
 *    StorablePicture to be inserted
 *
 ************************************************************************
 */
// static void insert_picture_in_dpb(VideoParameters *p_Vid, FrameStore* fs, StorablePicture* p)
// {
//   InputParameters *p_Inp = p_Vid->p_Inp;
// //  printf ("insert (%s) pic with frame_num #%d, poc %d\n", (p->structure == FRAME)?"FRAME":(p->structure == TOP_FIELD)?"TOP_FIELD":"BOTTOM_FIELD", p->pic_num, p->poc);
//   assert (p!=NULL);
//   assert (fs!=NULL);
//   switch (p->structure)
//   {
//   case FRAME:
//     fs->frame = p;
//     fs->is_used = 3;
//     if (p->used_for_reference)
//     {
//       fs->is_reference = 3;
//       fs->is_orig_reference = 3;
//       if (p->is_long_term)
//       {
//         fs->is_long_term = 3;
//         fs->long_term_frame_idx = p->long_term_frame_idx;
//       }
//     }
// #if (MVC_EXTENSION_ENABLE)
//     fs->view_id = p->view_id;
// 		fs->inter_view_flag[0] = fs->inter_view_flag[1] = p->inter_view_flag;
// 		fs->anchor_pic_flag[0] = fs->anchor_pic_flag[1] = p->anchor_pic_flag;
// #endif
//     // generate field views
//     dpb_split_field(p_Vid, fs);
//     break;
//   case TOP_FIELD:
//     fs->top_field = p;
//     fs->is_used |= 1;
// #if (MVC_EXTENSION_ENABLE)
//     fs->view_id = p->view_id;
// 		fs->inter_view_flag[0] = p->inter_view_flag;
// 		fs->anchor_pic_flag[0] = p->anchor_pic_flag;
// #endif
//     if (p->used_for_reference)
//     {
//       fs->is_reference |= 1;
//       fs->is_orig_reference |= 1;
//       if (p->is_long_term)
//       {
//         fs->is_long_term |= 1;
//         fs->long_term_frame_idx = p->long_term_frame_idx;
//       }
//     }
//     if (fs->is_used == 3)
//     {
//       // generate frame view
//       dpb_combine_field(p_Vid, fs);
//     }
//     else
//     {
//       fs->poc = p->poc;
//       gen_field_ref_ids(p_Vid, p);
//     }
//     break;
//   case BOTTOM_FIELD:
//     fs->bottom_field = p;
//     fs->is_used |= 2;
// #if (MVC_EXTENSION_ENABLE)
//     fs->view_id = p->view_id;
// 		fs->inter_view_flag[1] = p->inter_view_flag;
// 		fs->anchor_pic_flag[1] = p->anchor_pic_flag;
// #endif
//     if (p->used_for_reference)
//     {
//       fs->is_reference |= 2;
//       fs->is_orig_reference |= 2;
//       if (p->is_long_term)
//       {
//         fs->is_long_term |= 2;
//         fs->long_term_frame_idx = p->long_term_frame_idx;
//       }
//     }
//     if (fs->is_used == 3)
//     {
//       // generate frame view
//       dpb_combine_field(p_Vid, fs);
//     }
//     else
//     {
//       fs->poc = p->poc;
//       gen_field_ref_ids(p_Vid, p);
//     }
//     break;
//   }
//   fs->frame_num = p->pic_num;
//   fs->recovery_frame = p->recovery_frame;

//   fs->is_output = p->is_output;

//   if (fs->is_used==3)
//   {
//     calculate_frame_no(p_Vid, p);
//     if (-1 != p_Vid->p_ref && !p_Inp->silent)
//       find_snr(p_Vid, fs->frame, &p_Vid->p_ref);
//   }
// }

/*!
 ************************************************************************
 * \brief
 *    Check if one of the frames/fields in frame store is used for reference
 ************************************************************************
 */
static int is_used_for_reference(FrameStore* fs)
{
  if (fs->is_reference)
  {
    return 1;
  }

  if (fs->is_used == 3) // frame
  {
    if (fs->frame->used_for_reference)
    {
      return 1;
    }
  }

  if (fs->is_used & 1) // top field
  {
    if (fs->top_field)
    {
      if (fs->top_field->used_for_reference)
      {
        return 1;
      }
    }
  }

  if (fs->is_used & 2) // bottom field
  {
    if (fs->bottom_field)
    {
      if (fs->bottom_field->used_for_reference)
      {
        return 1;
      }
    }
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Check if one of the frames/fields in frame store is used for short-term reference
 ************************************************************************
 */
static int is_short_term_reference(FrameStore* fs)
{

  if (fs->is_used==3) // frame
  {
    if ((fs->frame->used_for_reference)&&(!fs->frame->is_long_term))
    {
      return 1;
    }
  }

  if (fs->is_used & 1) // top field
  {
    if (fs->top_field)
    {
      if ((fs->top_field->used_for_reference)&&(!fs->top_field->is_long_term))
      {
        return 1;
      }
    }
  }

  if (fs->is_used & 2) // bottom field
  {
    if (fs->bottom_field)
    {
      if ((fs->bottom_field->used_for_reference)&&(!fs->bottom_field->is_long_term))
      {
        return 1;
      }
    }
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Check if one of the frames/fields in frame store is used for short-term reference
 ************************************************************************
 */
static int is_long_term_reference(FrameStore* fs)
{

  if (fs->is_used==3) // frame
  {
    if ((fs->frame->used_for_reference)&&(fs->frame->is_long_term))
    {
      return 1;
    }
  }

  if (fs->is_used & 1) // top field
  {
    if (fs->top_field)
    {
      if ((fs->top_field->used_for_reference)&&(fs->top_field->is_long_term))
      {
        return 1;
      }
    }
  }

  if (fs->is_used & 2) // bottom field
  {
    if (fs->bottom_field)
    {
      if ((fs->bottom_field->used_for_reference)&&(fs->bottom_field->is_long_term))
      {
        return 1;
      }
    }
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    remove one frame from DPB
 ************************************************************************
 */
static void remove_frame_from_dpb(DecodedPictureBuffer *p_Dpb, int pos)
{
  FrameStore* fs = p_Dpb->fs[pos];
  FrameStore* tmp;
  unsigned i;

//  printf ("remove frame with frame_num #%d\n", fs->frame_num);
  switch (fs->is_used)
  {
  case 3:
    free_storable_picture(fs->frame);
    free_storable_picture(fs->top_field);
    free_storable_picture(fs->bottom_field);
    fs->frame=NULL;
    fs->top_field=NULL;
    fs->bottom_field=NULL;
    break;
  case 2:
    free_storable_picture(fs->bottom_field);
    fs->bottom_field=NULL;
    break;
  case 1:
    free_storable_picture(fs->top_field);
    fs->top_field=NULL;
    break;
  case 0:
    break;
  default:
    error("invalid frame store type",500);
  }
  fs->is_used = 0;
  fs->is_long_term = 0;
  fs->is_reference = 0;
  fs->is_orig_reference = 0;

  // move empty framestore to end of buffer
  tmp = p_Dpb->fs[pos];

  for (i=pos; i<p_Dpb->used_size-1;i++)
  {
    p_Dpb->fs[i] = p_Dpb->fs[i+1];
  }
  p_Dpb->fs[p_Dpb->used_size-1] = tmp;
  p_Dpb->used_size--;
}

/*!
 ************************************************************************
 * \brief
 *    find smallest POC in the DPB.
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
static void get_smallest_poc(DecodedPictureBuffer *p_Dpb, int *poc,int * pos, int curr_view_id)
#else
static void get_smallest_poc(DecodedPictureBuffer *p_Dpb, int *poc,int * pos)
#endif
{
  unsigned i;

  if (p_Dpb->used_size<1)
  {
    error("Cannot determine smallest POC, DPB empty.",150);
  }

  *pos=-1;
  *poc = INT_MAX;
  for (i = 0; i < p_Dpb->used_size; i++)
  {
#if (MVC_EXTENSION_ENABLE)
    if ((*poc > p_Dpb->fs[i]->poc)&&(!p_Dpb->fs[i]->is_output) && (p_Dpb->fs[i]->view_id == curr_view_id || curr_view_id == -1))
#else
    if ((*poc > p_Dpb->fs[i]->poc)&&(!p_Dpb->fs[i]->is_output))
#endif
    {
      *poc = p_Dpb->fs[i]->poc;
      *pos=i;
    }
  }
}

/*!
 ************************************************************************
 * \brief
 *    Remove a picture from DPB which is no longer needed.
 ************************************************************************
 */
#if (MVC_EXTENSION_ENABLE)
static int remove_unused_frame_from_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id)
{
  unsigned i;

  // check for frames that were already output and no longer used for reference
  for (i = 0; i < p_Dpb->used_size; i++)
  {
    if (p_Dpb->fs[i]->is_output && (!is_used_for_reference(p_Dpb->fs[i])) && (p_Dpb->fs[i]->view_id == curr_view_id || curr_view_id == -1))
    {
      remove_frame_from_dpb(p_Dpb, i);
      return 1;
    }
  }
  return 0;
}
#else
static int remove_unused_frame_from_dpb(DecodedPictureBuffer *p_Dpb)
{
  unsigned i;

  // check for frames that were already output and no longer used for reference
  for (i = 0; i < p_Dpb->used_size; i++)
  {
    if (p_Dpb->fs[i]->is_output && (!is_used_for_reference(p_Dpb->fs[i])))
    {
      remove_frame_from_dpb(p_Dpb, i);
      return 1;
    }
  }
  return 0;
}
#endif


/*!
 ************************************************************************
 * \brief
 *    Output one picture stored in the DPB.
 ************************************************************************
 */
// #if (MVC_EXTENSION_ENABLE)
// static int output_one_frame_from_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id)
// #else
// static void output_one_frame_from_dpb(DecodedPictureBuffer *p_Dpb)
// #endif
// {
//   VideoParameters *p_Vid = p_Dpb->p_Vid;
//   int poc, pos;
//   //diagnostics
//   if (p_Dpb->used_size<1)
//   {
//     error("Cannot output frame, DPB empty.",150);
//   }

//   // find smallest POC
// #if (MVC_EXTENSION_ENABLE)
//   get_smallest_poc(p_Dpb, &poc, &pos, curr_view_id);

//   if(pos==-1)
//   {
// 		if (curr_view_id == -1)
// 			error("no frames for output available", 150);
// 		else
// 			return 0;
//   }
// #else
//   get_smallest_poc(p_Dpb, &poc, &pos);

//   if(pos==-1)
//   {
//     error("no frames for output available", 150);
//   }
// #endif  

//   // call the output function
// //  printf ("output frame with frame_num #%d, poc %d (dpb. p_Dpb->size=%d, p_Dpb->used_size=%d)\n", p_Dpb->fs[pos]->frame_num, p_Dpb->fs[pos]->frame->poc, p_Dpb->size, p_Dpb->used_size);

//   // picture error concealment
//   if(p_Vid->conceal_mode != 0)
//   {
//     if(p_Dpb->last_output_poc == 0)
//     {
//       write_lost_ref_after_idr(p_Dpb, pos);
//     }
//     write_lost_non_ref_pic(p_Dpb, poc, p_Vid->p_out);
//   }

// // JVT-P072 ends

// //delete by elva
//   //write_stored_frame(p_Vid, p_Dpb->fs[pos], p_Vid->p_out);

//   // picture error concealment
// #if (MVC_EXTENSION_ENABLE)
//   if(p_Vid->conceal_mode == 0)
//     if (p_Dpb->last_output_poc >= poc && p_Dpb->fs[pos]->view_id == p_Dpb->last_output_view_id)
//     {
//       error ("output POC must be in ascending order", 150);
//     }
//   p_Dpb->last_output_poc = poc;
//   p_Dpb->last_output_view_id = p_Dpb->fs[pos]->view_id;
//   // free frame store and move empty store to end of buffer
//   if (!is_used_for_reference(p_Dpb->fs[pos]))
//   {
//     remove_frame_from_dpb(p_Dpb, pos);
//   }

//   return 1;
// #else
//   if(p_Vid->conceal_mode == 0)
//     if (p_Dpb->last_output_poc >= poc)
//     {
//       error ("output POC must be in ascending order", 150);
//     }
//   p_Dpb->last_output_poc = poc;
//   // free frame store and move empty store to end of buffer
//   if (!is_used_for_reference(p_Dpb->fs[pos]))
//   {
//     remove_frame_from_dpb(p_Dpb, pos);
//   }
// #endif
// }



/*!
 ************************************************************************
 * \brief
 *    All stored picture are output. Should be called to empty the buffer
 ************************************************************************
 */
//#if (MVC_EXTENSION_ENABLE)
//void flush_dpb(DecodedPictureBuffer *p_Dpb, int curr_view_id)
//#else
//void flush_dpb(DecodedPictureBuffer *p_Dpb)
//#endif
//{
//  VideoParameters *p_Vid = p_Dpb->p_Vid;
//  unsigned i;
//
//  // diagnostics
//  // printf("Flush remaining frames from the dpb. p_Dpb->size=%d, p_Dpb->used_size=%d\n",p_Dpb->size,p_Dpb->used_size);
//
////  if(p_Vid->conceal_mode == 0)
//  if (p_Vid->conceal_mode != 0)
//    conceal_non_ref_pics(p_Dpb, 0);
//
//#if (MVC_EXTENSION_ENABLE)
//  // mark all frames unused
//  for (i=0; i<p_Dpb->used_size; i++)
//  {
//    if (p_Dpb->fs[i]->view_id == curr_view_id || curr_view_id == -1)
//    {
//      unmark_for_reference (p_Dpb->fs[i]);
//    }
//  }
//
//  while (remove_unused_frame_from_dpb(p_Dpb, curr_view_id)) ;
//
//  // output frames in POC order
//  while (p_Dpb->used_size && output_one_frame_from_dpb(p_Dpb, curr_view_id)) ;
//#else
//  // mark all frames unused
//  for (i=0; i<p_Dpb->used_size; i++)
//  {
//    unmark_for_reference (p_Dpb->fs[i]);
//  }
//
//  while (remove_unused_frame_from_dpb(p_Dpb)) ;
//
//  // output frames in POC order
//  while (p_Dpb->used_size)
//  {
//    output_one_frame_from_dpb(p_Dpb);
//  }
//#endif
//
//  p_Dpb->last_output_poc = INT_MIN;
//}


static void gen_field_ref_ids(VideoParameters *p_Vid, StorablePicture *p)
{
  int i,j;
   //! Generate Frame parameters from field information.
  for (i = 0; i < (p->size_x >> 2); i++)
  {
    for (j = 0; j < (p->size_y >> 2); j++)
    {
      //p->mv_info[j][i].field_frame = 1;
    }
  }

  //copy the list;
  if(p->listX[LIST_0])
  {
    p->listXsize[LIST_0] =  p_Vid->ppSliceList[0]->listXsize[LIST_0];
    for(i=0; i<p->listXsize[LIST_0]; i++)
      p->listX[LIST_0][i] = p_Vid->ppSliceList[0]->listX[LIST_0][i];
  }
  if(p->listX[LIST_1])
  {
    p->listXsize[LIST_1] =  p_Vid->ppSliceList[0]->listXsize[LIST_1];
    for(i=0; i<p->listXsize[LIST_1]; i++)
      p->listX[LIST_1][i] = p_Vid->ppSliceList[0]->listX[LIST_1][i];
  }
  
}

/*!
 ************************************************************************
 * \brief
 *    Extract top field from a frame
 ************************************************************************
 */
//void dpb_split_field(VideoParameters *p_Vid, FrameStore *fs)
//{
//  int i, j, ii, jj, jj4;
//  int idiv,jdiv;
//  int currentmb;
//  int twosz16 = 2 * (fs->frame->size_x >> 4);
//  StorablePicture *fs_top, *fs_btm; 
//  StorablePicture *frame = fs->frame;
//
//  fs->poc = frame->poc;
//
//  if (!frame->frame_mbs_only_flag)
//  {
//    fs_top = fs->top_field    = alloc_storable_picture(p_Vid, TOP_FIELD,    frame->size_x, frame->size_y, frame->size_x_cr, frame->size_y_cr);
//    fs_btm = fs->bottom_field = alloc_storable_picture(p_Vid, BOTTOM_FIELD, frame->size_x, frame->size_y, frame->size_x_cr, frame->size_y_cr);
//
//    for (i = 0; i < (frame->size_y >> 1); i++)
//    {
//      memcpy(fs_top->imgY[i], frame->imgY[i*2], frame->size_x*sizeof(imgpel));
//    }
//
//    for (i = 0; i< (frame->size_y_cr>>1); i++)
//    {
//      memcpy(fs_top->imgUV[0][i], frame->imgUV[0][i*2], frame->size_x_cr*sizeof(imgpel));
//      memcpy(fs_top->imgUV[1][i], frame->imgUV[1][i*2], frame->size_x_cr*sizeof(imgpel));
//    }
//
//    for (i = 0; i < (frame->size_y>>1); i++)
//    {
//      memcpy(fs_btm->imgY[i], frame->imgY[i*2 + 1], frame->size_x*sizeof(imgpel));
//    }
//
//    for (i = 0; i < (frame->size_y_cr>>1); i++)
//    {
//      memcpy(fs_btm->imgUV[0][i], frame->imgUV[0][i*2 + 1], frame->size_x_cr*sizeof(imgpel));
//      memcpy(fs_btm->imgUV[1][i], frame->imgUV[1][i*2 + 1], frame->size_x_cr*sizeof(imgpel));
//    }
//
//    fs_top->poc = frame->top_poc;
//    fs_btm->poc = frame->bottom_poc;
//
//#if (MVC_EXTENSION_ENABLE)
//    fs_top->view_id = frame->view_id;
//    fs_btm->view_id = frame->view_id;
//#endif
//
//    fs_top->frame_poc =  frame->frame_poc;
//
//    fs_top->bottom_poc = fs_btm->bottom_poc =  frame->bottom_poc;
//    fs_top->top_poc    = fs_btm->top_poc    =  frame->top_poc;
//    fs_btm->frame_poc  = frame->frame_poc;
//
//    fs_top->used_for_reference = fs_btm->used_for_reference
//                                      = frame->used_for_reference;
//    fs_top->is_long_term = fs_btm->is_long_term
//                                = frame->is_long_term;
//    fs->long_term_frame_idx = fs_top->long_term_frame_idx
//                            = fs_btm->long_term_frame_idx
//                            = frame->long_term_frame_idx;
//
//    fs_top->coded_frame = fs_btm->coded_frame = 1;
//    fs_top->mb_aff_frame_flag = fs_btm->mb_aff_frame_flag
//                        = frame->mb_aff_frame_flag;
//
//    frame->top_field    = fs_top;
//    frame->bottom_field = fs_btm;
//    frame->frame         = frame;
//    fs_top->bottom_field = fs_btm;
//    fs_top->frame        = frame;
//    fs_top->top_field = fs_top;
//    fs_btm->top_field = fs_top;
//    fs_btm->frame     = frame;
//    fs_btm->bottom_field = fs_btm;
//
//#if (MVC_EXTENSION_ENABLE)
//    fs_top->view_id = fs_btm->view_id = fs->view_id;
//		fs_top->inter_view_flag = fs->inter_view_flag[0];
//		fs_btm->inter_view_flag = fs->inter_view_flag[1];
//#endif
//
//    fs_top->chroma_format_idc = fs_btm->chroma_format_idc = frame->chroma_format_idc;
//    fs_top->iCodingType = fs_btm->iCodingType = frame->iCodingType;
//    if(frame->used_for_reference)
//    {
//      pad_dec_picture(p_Vid, fs_top);
//      pad_dec_picture(p_Vid, fs_btm);
//    }
//  }
//  else
//  {
//    fs_top=NULL;
//    fs_btm=NULL;
//    frame->top_field=NULL;
//    frame->bottom_field=NULL;
//    frame->frame = frame;
//  }
//
//  if (!frame->frame_mbs_only_flag)
//  {
//    if (frame->mb_aff_frame_flag)
//    {
//      PicMotionParamsOld *frm_motion = &frame->motion;
//      for (j=0 ; j< (frame->size_y >> 3); j++)
//      {
//        jj = (j >> 2)*8 + (j & 0x03);
//        jj4 = jj + 4;
//        jdiv = (j >> 1);
//        for (i=0 ; i < (frame->size_x>>2); i++)
//        {
//          idiv = (i >> 2);
//
//          currentmb = twosz16*(jdiv >> 1)+ (idiv)*2 + (jdiv & 0x01);
//          // Assign field mvs attached to MB-Frame buffer to the proper buffer
//          if (frm_motion->mb_field[currentmb])
//          {
//            //fs_btm->mv_info[j][i].field_frame  = fs_top->mv_info[j][i].field_frame = 1;
//            //frame->mv_info[2*j][i].field_frame = frame->mv_info[2*j+1][i].field_frame = 1;
//
//            fs_btm->mv_info[j][i].mv[LIST_0] = frame->mv_info[jj4][i].mv[LIST_0];
//            fs_btm->mv_info[j][i].mv[LIST_1] = frame->mv_info[jj4][i].mv[LIST_1];
//            fs_btm->mv_info[j][i].ref_idx[LIST_0] = frame->mv_info[jj4][i].ref_idx[LIST_0];
//            if(fs_btm->mv_info[j][i].ref_idx[LIST_0] >=0)
//              fs_btm->mv_info[j][i].ref_pic[LIST_0] = p_Vid->ppSliceList[0]->listX[4][(short) fs_btm->mv_info[j][i].ref_idx[LIST_0]];
//            else
//              fs_btm->mv_info[j][i].ref_pic[LIST_0] = NULL;
//            fs_btm->mv_info[j][i].ref_idx[LIST_1] = frame->mv_info[jj4][i].ref_idx[LIST_1];
//            if(fs_btm->mv_info[j][i].ref_idx[LIST_1] >=0)
//              fs_btm->mv_info[j][i].ref_pic[LIST_1] = p_Vid->ppSliceList[0]->listX[5][(short) fs_btm->mv_info[j][i].ref_idx[LIST_1]];
//            else
//              fs_btm->mv_info[j][i].ref_pic[LIST_1] = NULL;
//          
//            fs_top->mv_info[j][i].mv[LIST_0] = frame->mv_info[jj][i].mv[LIST_0];
//            fs_top->mv_info[j][i].mv[LIST_1] = frame->mv_info[jj][i].mv[LIST_1];
//            fs_top->mv_info[j][i].ref_idx[LIST_0] = frame->mv_info[jj][i].ref_idx[LIST_0];
//            if(fs_top->mv_info[j][i].ref_idx[LIST_0] >=0)
//              fs_top->mv_info[j][i].ref_pic[LIST_0] = p_Vid->ppSliceList[0]->listX[2][(short) fs_top->mv_info[j][i].ref_idx[LIST_0]];
//            else
//              fs_top->mv_info[j][i].ref_pic[LIST_0] = NULL;
//            fs_top->mv_info[j][i].ref_idx[LIST_1] = frame->mv_info[jj][i].ref_idx[LIST_1];
//            if(fs_top->mv_info[j][i].ref_idx[LIST_1] >=0)
//              fs_top->mv_info[j][i].ref_pic[LIST_1] = p_Vid->ppSliceList[0]->listX[3][(short) fs_top->mv_info[j][i].ref_idx[LIST_1]];
//            else
//              fs_top->mv_info[j][i].ref_pic[LIST_1] = NULL;
//          }
//        }
//      }
//    }
//  
//      //! Generate field MVs from Frame MVs
//    for (j=0 ; j < (frame->size_y >> 3) ; j++)
//    {
//      jj = 2* RSD(j);
//      jdiv = (j >> 1);
//      for (i=0 ; i < (frame->size_x >> 2) ; i++)
//      {
//        ii = RSD(i);
//        idiv = (i >> 2);
//
//        currentmb = twosz16 * (jdiv >> 1)+ (idiv)*2 + (jdiv & 0x01);
//
//        if (!frame->mb_aff_frame_flag  || !frame->motion.mb_field[currentmb])
//        {
//          //frame->mv_info[2*j+1][i].field_frame = frame->mv_info[2*j][i].field_frame = 0;
//
//          //fs_top->mv_info[j][i].field_frame = fs_btm->mv_info[j][i].field_frame = 0;
//
//          fs_top->mv_info[j][i].mv[LIST_0] = fs_btm->mv_info[j][i].mv[LIST_0] = frame->mv_info[jj][ii].mv[LIST_0];
//          fs_top->mv_info[j][i].mv[LIST_1] = fs_btm->mv_info[j][i].mv[LIST_1] = frame->mv_info[jj][ii].mv[LIST_1];
//
//          // Scaling of references is done here since it will not affect spatial direct (2*0 =0)
//          if (frame->mv_info[jj][ii].ref_idx[LIST_0] == -1)
//            fs_top->mv_info[j][i].ref_idx[LIST_0] = fs_btm->mv_info[j][i].ref_idx[LIST_0] = - 1;
//          else
//          {
//            fs_top->mv_info[j][i].ref_idx[LIST_0] = fs_btm->mv_info[j][i].ref_idx[LIST_0] = frame->mv_info[jj][ii].ref_idx[LIST_0];
//            fs_top->mv_info[j][i].ref_pic[LIST_0] = fs_btm->mv_info[j][i].ref_pic[LIST_0] = p_Vid->ppSliceList[0]->listX[LIST_0][(short) frame->mv_info[jj][ii].ref_idx[LIST_0]];
//          }
//
//          if (frame->mv_info[jj][ii].ref_idx[LIST_1] == -1)
//            fs_top->mv_info[j][i].ref_idx[LIST_1] = fs_btm->mv_info[j][i].ref_idx[LIST_1] = - 1;
//          else
//          {
//            fs_top->mv_info[j][i].ref_idx[LIST_1] = fs_btm->mv_info[j][i].ref_idx[LIST_1] = frame->mv_info[jj][ii].ref_idx[LIST_1];
//            fs_top->mv_info[j][i].ref_pic[LIST_1] = fs_btm->mv_info[j][i].ref_pic[LIST_1] = p_Vid->ppSliceList[0]->listX[LIST_1][(short) frame->mv_info[jj][ii].ref_idx[LIST_1]];
//          }
//        }
//        else
//        {
//          //frame->mv_info[2*j+1][i].field_frame = frame->mv_info[2*j][i].field_frame = frame->motion.mb_field[currentmb];
//        }
//      }
//    }
//
//  }
//  else
//  {
//    /*
//    for (j = 0; j < frame->size_y >> 2; j++)
//      for (i = 0; i < frame->size_x >> 2; i++)
//        frame->mv_info[j][i].field_frame = 0;
//        */
//  }
//}
//

/*!
 ************************************************************************
 * \brief
 *    Generate a frame from top and bottom fields,
 *    YUV components and display information only
 ************************************************************************
 */
//void dpb_combine_field_yuv(VideoParameters *p_Vid, FrameStore *fs)
//{
//  int i, j;
//
//  if (!fs->frame)
//  {
//    fs->frame = alloc_storable_picture(p_Vid, FRAME, fs->top_field->size_x, fs->top_field->size_y*2, fs->top_field->size_x_cr, fs->top_field->size_y_cr*2);
//  }
//
//  for (i=0; i<fs->top_field->size_y; i++)
//  {
//    memcpy(fs->frame->imgY[i*2],     fs->top_field->imgY[i]   , fs->top_field->size_x * sizeof(imgpel));     // top field
//    memcpy(fs->frame->imgY[i*2 + 1], fs->bottom_field->imgY[i], fs->bottom_field->size_x * sizeof(imgpel)); // bottom field
//  }
//
//  for (j = 0; j < 2; j++)
//  {
//    for (i=0; i<fs->top_field->size_y_cr; i++)
//    {
//      memcpy(fs->frame->imgUV[j][i*2],     fs->top_field->imgUV[j][i],    fs->top_field->size_x_cr*sizeof(imgpel));
//      memcpy(fs->frame->imgUV[j][i*2 + 1], fs->bottom_field->imgUV[j][i], fs->bottom_field->size_x_cr*sizeof(imgpel));
//    }
//  }
//
//  fs->poc=fs->frame->poc =fs->frame->frame_poc = imin (fs->top_field->poc, fs->bottom_field->poc);
//
//  fs->bottom_field->frame_poc=fs->top_field->frame_poc=fs->frame->poc;
//
//  fs->bottom_field->top_poc=fs->frame->top_poc=fs->top_field->poc;
//  fs->top_field->bottom_poc=fs->frame->bottom_poc=fs->bottom_field->poc;
//
//  fs->frame->used_for_reference = (fs->top_field->used_for_reference && fs->bottom_field->used_for_reference );
//  fs->frame->is_long_term = (fs->top_field->is_long_term && fs->bottom_field->is_long_term );
//
//  if (fs->frame->is_long_term)
//    fs->frame->long_term_frame_idx = fs->long_term_frame_idx;
//
//  fs->frame->top_field    = fs->top_field;
//  fs->frame->bottom_field = fs->bottom_field;
//  fs->frame->frame = fs->frame;
//
//  fs->frame->coded_frame = 0;
//
//  fs->frame->chroma_format_idc = fs->top_field->chroma_format_idc;
//  fs->frame->frame_cropping_flag = fs->top_field->frame_cropping_flag;
//  if (fs->frame->frame_cropping_flag)
//  {
//    fs->frame->frame_cropping_rect_top_offset = fs->top_field->frame_cropping_rect_top_offset;
//    fs->frame->frame_cropping_rect_bottom_offset = fs->top_field->frame_cropping_rect_bottom_offset;
//    fs->frame->frame_cropping_rect_left_offset = fs->top_field->frame_cropping_rect_left_offset;
//    fs->frame->frame_cropping_rect_right_offset = fs->top_field->frame_cropping_rect_right_offset;
//  }
//
//  fs->top_field->frame = fs->bottom_field->frame = fs->frame;
//  fs->top_field->bottom_field = fs->bottom_field;
//  fs->top_field->top_field = fs->top_field;
//  fs->bottom_field->top_field = fs->top_field;
//  fs->bottom_field->bottom_field = fs->bottom_field;
//  if(fs->top_field->used_for_reference || fs->bottom_field->used_for_reference)
//  {
//    pad_dec_picture(p_Vid, fs->frame);
//  }
//
//}
//

/*!
 ************************************************************************
 * \brief
 *    Generate a frame from top and bottom fields
 ************************************************************************
 */
//void dpb_combine_field(VideoParameters *p_Vid, FrameStore *fs)
//{
//  int i,j, jj, jj4, k;
//
//  dpb_combine_field_yuv(p_Vid, fs);
//
//#if (MVC_EXTENSION_ENABLE)
//  fs->frame->view_id = fs->view_id;
//#endif
//  fs->frame->iCodingType = fs->top_field->iCodingType; //FIELD_CODING;
//   //! Use inference flag to remap mvs/references
//
//  //! Generate Frame parameters from field information.
//#if 1
//  for (j=0 ; j < (fs->top_field->size_y >> 2) ; j++)
//  {
//    jj = (j<<1);
//    jj4 = jj + 1;
//    for (i=0 ; i< (fs->top_field->size_x >> 2) ; i++)
//    {
//      //fs->frame->mv_info[jj][i].field_frame= fs->frame->mv_info[jj4][i].field_frame = 1;
//
//      fs->frame->mv_info[jj][i].mv[LIST_0] = fs->top_field->mv_info[j][i].mv[LIST_0];
//      fs->frame->mv_info[jj][i].mv[LIST_1] = fs->top_field->mv_info[j][i].mv[LIST_1];
//
//      fs->frame->mv_info[jj][i].ref_idx[LIST_0] = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//      fs->frame->mv_info[jj][i].ref_idx[LIST_1] = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//
//      /* bug: top field list doesnot exist.*/
//      if(fs->top_field->listXsize[LIST_0] >0)
//      {
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//        assert( k < fs->top_field->listXsize[LIST_0]);
//        fs->frame->mv_info[jj][i].ref_pic[LIST_0] = k>=0? fs->top_field->listX[LIST_0][k]: NULL;  
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//        assert( k < fs->top_field->listXsize[LIST_1]);
//        fs->frame->mv_info[jj][i].ref_pic[LIST_1] = k>=0? fs->top_field->listX[LIST_1][k]: NULL;
//      }
//      else
//      {
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_0], p_Vid->ppSliceList[0]->listXsize[LIST_0]));
//        fs->frame->mv_info[jj][i].ref_pic[LIST_0] = k>=0?p_Vid->ppSliceList[0]->listX[LIST_0][k]: NULL;  
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_1], p_Vid->ppSliceList[0]->listXsize[LIST_1]));
//        fs->frame->mv_info[jj][i].ref_pic[LIST_1] = k>=0?p_Vid->ppSliceList[0]->listX[LIST_1][k]: NULL;
//      }
//
//      //! association with id already known for fields.
//      fs->frame->mv_info[jj4][i].mv[LIST_0] = fs->bottom_field->mv_info[j][i].mv[LIST_0];
//      fs->frame->mv_info[jj4][i].mv[LIST_1] = fs->bottom_field->mv_info[j][i].mv[LIST_1];
//
//      fs->frame->mv_info[jj4][i].ref_idx[LIST_0]  = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//      fs->frame->mv_info[jj4][i].ref_idx[LIST_1]  = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//
//      if(fs->bottom_field->listXsize[LIST_0]>0)
//      {
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < fs->bottom_field->listXsize[LIST_0]);
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_0] = k>=0? fs->bottom_field->listX[LIST_0][k]: NULL;
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < fs->bottom_field->listXsize[LIST_1]);
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_1] = k>=0? fs->bottom_field->listX[LIST_1][k]: NULL;
//      }
//      else
//      {
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_0], p_Vid->ppSliceList[0]->listXsize[LIST_0]));
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_0] = k>=0? p_Vid->ppSliceList[0]->listX[LIST_0][k]: NULL;
//
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_1], p_Vid->ppSliceList[0]->listXsize[LIST_1]));
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_1] = k>=0? p_Vid->ppSliceList[0]->listX[LIST_1][k]: NULL;
//      }
//      //fs->top_field->mv_info[j][i].field_frame = 1;
//      //fs->bottom_field->mv_info[j][i].field_frame = 1;
//    }
//  }
//#else
//  for (j=0 ; j < (fs->top_field->size_y >> 2) ; j++)
//  {
//    jj = 8*(j >> 2) + (j & 0x03);
//    jj4 = jj + 4;
//    for (i=0 ; i< (fs->top_field->size_x >> 2) ; i++)
//    {
//      //fs->frame->mv_info[jj][i].field_frame= fs->frame->mv_info[jj4][i].field_frame = 1;
//
//      fs->frame->mv_info[jj][i].mv[LIST_0] = fs->top_field->mv_info[j][i].mv[LIST_0];
//      fs->frame->mv_info[jj][i].mv[LIST_1] = fs->top_field->mv_info[j][i].mv[LIST_1];
//
//      fs->frame->mv_info[jj][i].ref_idx[LIST_0] = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//      fs->frame->mv_info[jj][i].ref_idx[LIST_1] = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//
//      /* bug: top field list doesnot exist.*/
//      if(fs->top_field->listXsize[LIST_0] >0)
//      {
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//        assert( k < fs->top_field->listXsize[LIST_0]);
//        fs->frame->mv_info[jj][i].ref_pic[LIST_0] = k>=0? fs->top_field->listX[LIST_0][k]: NULL;  
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//        assert( k < fs->top_field->listXsize[LIST_1]);
//        fs->frame->mv_info[jj][i].ref_pic[LIST_1] = k>=0? fs->top_field->listX[LIST_1][k]: NULL;
//      }
//      else
//      {
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_0], p_Vid->ppSliceList[0]->listXsize[LIST_0]));
//        fs->frame->mv_info[jj][i].ref_pic[LIST_0] = k>=0?p_Vid->ppSliceList[0]->listX[LIST_0][k]: NULL;  
//        k = fs->top_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_1], p_Vid->ppSliceList[0]->listXsize[LIST_1]));
//        fs->frame->mv_info[jj][i].ref_pic[LIST_1] = k>=0?p_Vid->ppSliceList[0]->listX[LIST_1][k]: NULL;
//      }
//
//      //! association with id already known for fields.
//      fs->frame->mv_info[jj4][i].mv[LIST_0] = fs->bottom_field->mv_info[j][i].mv[LIST_0];
//      fs->frame->mv_info[jj4][i].mv[LIST_1] = fs->bottom_field->mv_info[j][i].mv[LIST_1];
//
//      fs->frame->mv_info[jj4][i].ref_idx[LIST_0]  = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//      fs->frame->mv_info[jj4][i].ref_idx[LIST_1]  = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//
//      if(fs->bottom_field->listXsize[LIST_0]>0)
//      {
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < fs->bottom_field->listXsize[LIST_0]);
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_0] = k>=0? fs->bottom_field->listX[LIST_0][k]: NULL;
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < fs->bottom_field->listXsize[LIST_1]);
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_1] = k>=0? fs->bottom_field->listX[LIST_1][k]: NULL;
//      }
//      else
//      {
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_0];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_0], p_Vid->ppSliceList[0]->listXsize[LIST_0]));
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_0] = k>=0? p_Vid->ppSliceList[0]->listX[LIST_0][k]: NULL;
//
//        k = fs->bottom_field->mv_info[j][i].ref_idx[LIST_1];
//        assert(k < imin(p_Vid->ppSliceList[0]->num_ref_idx_active[LIST_1], p_Vid->ppSliceList[0]->listXsize[LIST_1]));
//        fs->frame->mv_info[jj4][i].ref_pic[LIST_1] = k>=0? p_Vid->ppSliceList[0]->listX[LIST_1][k]: NULL;
//      }
//      //fs->top_field->mv_info[j][i].field_frame = 1;
//      //fs->bottom_field->mv_info[j][i].field_frame = 1;
//    }
//  }
//#endif
//}
//

/*!
 ************************************************************************
 * \brief
 *    Allocate memory for buffering of reference picture reordering commands
 ************************************************************************
 */
void alloc_ref_pic_list_reordering_buffer(Slice *currSlice)
{
  //VideoParameters *p_Vid = currSlice->p_Vid;
  int size = currSlice->num_ref_idx_active[LIST_0] + 1;

  if (currSlice->slice_type!=I_SLICE && currSlice->slice_type!=SI_SLICE)
  {
    if ((currSlice->reordering_of_pic_nums_idc[LIST_0] = calloc(size ,sizeof(int)))==NULL) no_mem_exit("alloc_ref_pic_list_reordering_buffer: reordering_of_pic_nums_idc_l0");
    if ((currSlice->abs_diff_pic_num_minus1[LIST_0] = calloc(size,sizeof(int)))==NULL) no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_pic_num_minus1_l0");
    if ((currSlice->long_term_pic_idx[LIST_0] = calloc(size,sizeof(int)))==NULL) no_mem_exit("alloc_ref_pic_list_reordering_buffer: long_term_pic_idx_l0");
#if (MVC_EXTENSION_ENABLE)
    if ((currSlice->abs_diff_view_idx_minus1[LIST_0] = calloc(size,sizeof(int)))==NULL) no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_view_idx_minus1_l0");
#endif
  }
  else
  {
    currSlice->reordering_of_pic_nums_idc[LIST_0] = NULL;
    currSlice->abs_diff_pic_num_minus1[LIST_0] = NULL;
    currSlice->long_term_pic_idx[LIST_0] = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->abs_diff_view_idx_minus1[LIST_0] = NULL;
#endif
  }

  size = currSlice->num_ref_idx_active[LIST_1]+1;

  if (currSlice->slice_type==B_SLICE)
  {
    if ((currSlice->reordering_of_pic_nums_idc[LIST_1] = calloc(size,sizeof(int)))==NULL) 
      no_mem_exit("alloc_ref_pic_list_reordering_buffer: reordering_of_pic_nums_idc_l1");
    if ((currSlice->abs_diff_pic_num_minus1[LIST_1] = calloc(size,sizeof(int)))==NULL) 
      no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_pic_num_minus1_l1");
    if ((currSlice->long_term_pic_idx[LIST_1] = calloc(size,sizeof(int)))==NULL) 
      no_mem_exit("alloc_ref_pic_list_reordering_buffer: long_term_pic_idx_l1");
#if (MVC_EXTENSION_ENABLE)
    if ((currSlice->abs_diff_view_idx_minus1[LIST_1] = calloc(size,sizeof(int)))==NULL) no_mem_exit("alloc_ref_pic_list_reordering_buffer: abs_diff_view_idx_minus1_l1");
#endif
  }
  else
  {
    currSlice->reordering_of_pic_nums_idc[LIST_1] = NULL;
    currSlice->abs_diff_pic_num_minus1[LIST_1] = NULL;
    currSlice->long_term_pic_idx[LIST_1] = NULL;
#if (MVC_EXTENSION_ENABLE)
    currSlice->abs_diff_view_idx_minus1[LIST_1] = NULL;
#endif
  }
}


/*!
 ************************************************************************
 * \brief
 *    Free memory for buffering of reference picture reordering commands
 ************************************************************************
 */
void free_ref_pic_list_reordering_buffer(Slice *currSlice)
{
  if (currSlice->reordering_of_pic_nums_idc[LIST_0])
    free(currSlice->reordering_of_pic_nums_idc[LIST_0]);
  if (currSlice->abs_diff_pic_num_minus1[LIST_0])
    free(currSlice->abs_diff_pic_num_minus1[LIST_0]);
  if (currSlice->long_term_pic_idx[LIST_0])
    free(currSlice->long_term_pic_idx[LIST_0]);

  currSlice->reordering_of_pic_nums_idc[LIST_0] = NULL;
  currSlice->abs_diff_pic_num_minus1[LIST_0] = NULL;
  currSlice->long_term_pic_idx[LIST_0] = NULL;

  if (currSlice->reordering_of_pic_nums_idc[LIST_1])
    free(currSlice->reordering_of_pic_nums_idc[LIST_1]);
  if (currSlice->abs_diff_pic_num_minus1[LIST_1])
    free(currSlice->abs_diff_pic_num_minus1[LIST_1]);
  if (currSlice->long_term_pic_idx[LIST_1])
    free(currSlice->long_term_pic_idx[LIST_1]);

  currSlice->reordering_of_pic_nums_idc[LIST_1] = NULL;
  currSlice->abs_diff_pic_num_minus1[LIST_1] = NULL;
  currSlice->long_term_pic_idx[LIST_1] = NULL;

#if (MVC_EXTENSION_ENABLE)
  if (currSlice->abs_diff_view_idx_minus1[LIST_0])
    free(currSlice->abs_diff_view_idx_minus1[LIST_0]);
  currSlice->abs_diff_view_idx_minus1[LIST_0] = NULL;
  if (currSlice->abs_diff_view_idx_minus1[LIST_1])
    free(currSlice->abs_diff_view_idx_minus1[LIST_1]);
  currSlice->abs_diff_view_idx_minus1[LIST_1] = NULL;
#endif
}

/*!
 ************************************************************************
 * \brief
 *      Tian Dong
 *          June 13, 2002, Modifed on July 30, 2003
 *
 *      If a gap in frame_num is found, try to fill the gap
 * \param p_Vid
 *    VideoParameters structure
 *
 ************************************************************************
 */
//void fill_frame_num_gap(VideoParameters *p_Vid, Slice *currSlice)
//{
//  seq_parameter_set_rbsp_t *active_sps = p_Vid->active_sps;
//  
//  int CurrFrameNum;
//  int UnusedShortTermFrameNum;
//  StorablePicture *picture = NULL;
//  int tmp1 = currSlice->delta_pic_order_cnt[0];
//  int tmp2 = currSlice->delta_pic_order_cnt[1];
//  currSlice->delta_pic_order_cnt[0] = currSlice->delta_pic_order_cnt[1] = 0;
//
//  printf("A gap in frame number is found, try to fill it.\n");
//
//  UnusedShortTermFrameNum = (p_Vid->pre_frame_num + 1) % p_Vid->MaxFrameNum;
//  CurrFrameNum = currSlice->frame_num; //p_Vid->frame_num;
//
//  while (CurrFrameNum != UnusedShortTermFrameNum)
//  {
//    picture = alloc_storable_picture (p_Vid, FRAME, p_Vid->width, p_Vid->height, p_Vid->width_cr, p_Vid->height_cr);
//    picture->coded_frame = 1;
//    picture->pic_num = UnusedShortTermFrameNum;
//    picture->frame_num = UnusedShortTermFrameNum;
//    picture->non_existing = 1;
//    picture->is_output = 1;
//    picture->used_for_reference = 1;
//    picture->adaptive_ref_pic_buffering_flag = 0;
//#if (MVC_EXTENSION_ENABLE)
//    picture->view_id = currSlice->view_id;
//#endif
//
//    currSlice->frame_num = UnusedShortTermFrameNum;
//    if (active_sps->pic_order_cnt_type!=0)
//    {
//      decode_poc(p_Vid, p_Vid->ppSliceList[0]);
//    }
//    picture->top_poc    = currSlice->toppoc;
//    picture->bottom_poc = currSlice->bottompoc;
//    picture->frame_poc  = currSlice->framepoc;
//    picture->poc        = currSlice->framepoc;
//
//    store_picture_in_dpb(currSlice->p_Dpb, picture);
//
//    picture=NULL;
//    p_Vid->pre_frame_num = UnusedShortTermFrameNum;
//    UnusedShortTermFrameNum = (UnusedShortTermFrameNum + 1) % p_Vid->MaxFrameNum;
//  }
//  currSlice->delta_pic_order_cnt[0] = tmp1;
//  currSlice->delta_pic_order_cnt[1] = tmp2;
//  currSlice->frame_num = CurrFrameNum;
//
//}
//

/*!
 ************************************************************************
 * \brief
 *    Compute co-located motion info
 *
 ************************************************************************
 */
void compute_colocated (Slice *currSlice, StorablePicture **listX[6])
{
  int i,j;

  VideoParameters *p_Vid = currSlice->p_Vid;

  if (currSlice->direct_spatial_mv_pred_flag == 0)
  {    
    for (j = 0; j < 2 + (currSlice->mb_aff_frame_flag * 4); j += 2)
    {
      for (i=0; i<currSlice->listXsize[j];i++)
      {
        int prescale, iTRb, iTRp;

        if (j==0)
        {
          iTRb = iClip3( -128, 127, p_Vid->dec_picture->poc - listX[LIST_0 + j][i]->poc );
        }
        else if (j == 2)
        {
          iTRb = iClip3( -128, 127, p_Vid->dec_picture->top_poc - listX[LIST_0 + j][i]->poc );
        }
        else
        {
          iTRb = iClip3( -128, 127, p_Vid->dec_picture->bottom_poc - listX[LIST_0 + j][i]->poc );
        }

        iTRp = iClip3( -128, 127,  listX[LIST_1 + j][0]->poc - listX[LIST_0 + j][i]->poc);

        if (iTRp!=0)
        {
          prescale = ( 16384 + iabs( iTRp / 2 ) ) / iTRp;
          currSlice->mvscale[j][i] = iClip3( -1024, 1023, ( iTRb * prescale + 32 ) >> 6 ) ;
        }
        else
        {
          currSlice->mvscale[j][i] = 9999;
        }
      }
    }
  }
}


#if (MVC_EXTENSION_ENABLE)
int GetMaxDecFrameBuffering(VideoParameters *p_Vid)
{
  int i, j, iMax, iMax_1 = 0, iMax_2 = 0;
  subset_seq_parameter_set_rbsp_t *curr_subset_sps;
  seq_parameter_set_rbsp_t *curr_sps;

  curr_subset_sps = p_Vid->SubsetSeqParSet;
  curr_sps = p_Vid->SeqParSet;
  for(i=0; i<MAXSPS; i++)
  {
    if(curr_subset_sps->Valid && curr_subset_sps->sps.seq_parameter_set_id < MAXSPS)
    {
      j = curr_subset_sps->sps.max_dec_frame_buffering;

      if (curr_subset_sps->sps.vui_parameters_present_flag && curr_subset_sps->sps.vui_seq_parameters.bitstream_restriction_flag)
      {
        if ((int)curr_subset_sps->sps.vui_seq_parameters.max_dec_frame_buffering > j)
        {
          error ("max_dec_frame_buffering larger than MaxDpbSize", 500);
        }
        j = imax (1, curr_subset_sps->sps.vui_seq_parameters.max_dec_frame_buffering);
      }

      if(j > iMax_2)
        iMax_2 = j;
    }
    
    if(curr_sps->Valid)
    {
      j = curr_sps->max_dec_frame_buffering;

      if (curr_sps->vui_parameters_present_flag && curr_sps->vui_seq_parameters.bitstream_restriction_flag)
      {
        if ((int)curr_sps->vui_seq_parameters.max_dec_frame_buffering > j)
        {
          error ("max_dec_frame_buffering larger than MaxDpbSize", 500);
        }
        j = imax (1, curr_sps->vui_seq_parameters.max_dec_frame_buffering);
      }

      if(j > iMax_1)
        iMax_1 = j;
    }
    curr_subset_sps++;
    curr_sps++;
  }  
      
  if (iMax_1 > 0 && iMax_2 > 0)
    iMax = iMax_1 + iMax_2;
  else
    iMax = (iMax_1 >0? iMax_1*2 : iMax_2*2);
  return iMax;
}

static int is_view_id_in_ref_view_list(int view_id, int *ref_view_id, int num_ref_views)
{
   int i;
   for(i=0; i<num_ref_views; i++)
  {
	   if(view_id == ref_view_id[i])
		   break;
  }

   return (num_ref_views && (i<num_ref_views));
}

void append_interview_list(DecodedPictureBuffer *p_Dpb, 
                           PictureStructure currPicStructure, //0: frame; 1:top field; 2: bottom field;
													 int list_idx, 
													 FrameStore **list,
													 int *listXsize, 
													 int currPOC, 
													 int curr_view_id, 
													 int anchor_pic_flag)
{
  unsigned int i;
  VideoParameters *p_Vid = p_Dpb->p_Vid;
  int iVOIdx = -1;//GetVOIdx(p_Vid, curr_view_id);
  int pic_avail;
  int poc = 0;
  int fld_idx;
  int num_ref_views, *ref_view_id;


  if(iVOIdx <0)
    printf("Error: iVOIdx: %d is not less than 0\n", iVOIdx);

  if(anchor_pic_flag)
  {
    num_ref_views = list_idx? p_Vid->active_subset_sps->num_anchor_refs_l1[iVOIdx] : p_Vid->active_subset_sps->num_anchor_refs_l0[iVOIdx];
    ref_view_id = list_idx? p_Vid->active_subset_sps->anchor_ref_l1[iVOIdx]:p_Vid->active_subset_sps->anchor_ref_l0[iVOIdx];
  }
  else
  {
    num_ref_views = list_idx? p_Vid->active_subset_sps->num_non_anchor_refs_l1[iVOIdx] : p_Vid->active_subset_sps->num_non_anchor_refs_l0[iVOIdx];
    ref_view_id = list_idx? p_Vid->active_subset_sps->non_anchor_ref_l1[iVOIdx]:p_Vid->active_subset_sps->non_anchor_ref_l0[iVOIdx];
  }

  //	if(num_ref_views <= 0)
  //		printf("Error: iNumOfRefViews: %d is not larger than 0\n", num_ref_views);

  if(currPicStructure == BOTTOM_FIELD)
    fld_idx = 1;
  else
    fld_idx = 0;

  for(i=0; i<p_Dpb->used_size; i++)
  {
    if(currPicStructure==FRAME)
    {
      pic_avail = (p_Dpb->fs[i]->is_used == 3);
      if (pic_avail)
        poc = p_Dpb->fs[i]->frame->poc;
    }
    else if(currPicStructure==TOP_FIELD)
    {
      pic_avail = p_Dpb->fs[i]->is_used & 1;
      if (pic_avail)
        poc = p_Dpb->fs[i]->top_field->poc;
    }
    else if(currPicStructure==BOTTOM_FIELD)
    {
      pic_avail = p_Dpb->fs[i]->is_used & 2;
      if (pic_avail)
        poc = p_Dpb->fs[i]->bottom_field->poc;
    }
    else
      pic_avail =0;

    if(pic_avail && p_Dpb->fs[i]->inter_view_flag[fld_idx])
    {
      if(poc == currPOC)
      {
        if(is_view_id_in_ref_view_list(p_Dpb->fs[i]->view_id, ref_view_id, num_ref_views))
        {
          //add one inter-view reference;
          list[*listXsize] = p_Dpb->fs[i];
          //next;
          (*listXsize)++;
        }
      }
    }
  }
}
#endif

