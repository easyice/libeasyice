
/*!
 ************************************************************************
 * \file  memalloc.c
 *
 * \brief
 *    Memory allocation and free helper functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *     - Alexis Michael Tourapis         <alexismt@ieee.org> 
 *     - Karsten Sühring                 <suehring@hhi.de> 
 *
 ************************************************************************
 */

#include "global.h"
#include "memalloc.h"

 /*!
 ************************************************************************
 * \brief
 *    Initialize 2-dimensional top and bottom field to point to the proper
 *    lines in frame
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int init_top_bot_planes(imgpel **imgFrame, int dim0, imgpel ***imgTopField, imgpel ***imgBotField)
{
  int i;

  if((*imgTopField   = (imgpel**) malloc((dim0>>1) * sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgTopField");

  if((*imgBotField   = (imgpel**) malloc((dim0>>1) * sizeof(imgpel*))) == NULL)
    no_mem_exit("init_top_bot_planes: imgBotField");

  for(i = 0; i < (dim0>>1); i++)
  {
    (*imgTopField)[i] =  imgFrame[2 * i    ];
    (*imgBotField)[i] =  imgFrame[2 * i + 1];
  }

  return dim0 * sizeof(imgpel*);
}

 /*!
 ************************************************************************
 * \brief
 *    free 2-dimensional top and bottom fields without freeing target memory
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
void free_top_bot_planes(imgpel **imgTopField, imgpel **imgBotField)
{
  free (imgTopField);
  free (imgBotField);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> DistortionData array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Ddist(DistortionData ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (DistortionData**)malloc(dim0 *       sizeof(DistortionData*))) == NULL)
    no_mem_exit("get_mem2Ddist: array2D");
  if((*(*array2D) = (DistortionData* )calloc(dim0 * dim1, sizeof(DistortionData ))) == NULL)
    no_mem_exit("get_mem2Ddist: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(DistortionData*) + dim1 * sizeof(DistortionData));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LambdaParams array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dlm(LambdaParams ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (LambdaParams**)malloc(dim0 *      sizeof(LambdaParams*))) == NULL)
    no_mem_exit("get_mem2Dlm: array2D");
  if((*(*array2D) = (LambdaParams* )calloc(dim0 * dim1,sizeof(LambdaParams ))) == NULL)
    no_mem_exit("get_mem2Dlm: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(LambdaParams*) + dim1 * sizeof(LambdaParams));
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Ddist()
 ************************************************************************
 */
void free_mem2Ddist(DistortionData **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else
		printf("free_mem2Ddist: trying to free unused memory.%d",100);
      //error ("free_mem2Ddist: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
	  printf("free_mem2Ddist: trying to free unused memory.%d",100);
    //error ("free_mem2Ddist: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dlm()
 ************************************************************************
 */
void free_mem2Dlm(LambdaParams **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else
		printf("free_mem2Dlm: trying to free unused memory.%d",100);
      //error ("free_mem2Dlm: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
	  printf("free_mem2Dlm: trying to free unused memory.%d",100);
    //error ("free_mem2Dlm: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> PicMotionParams array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dmp(PicMotionParams ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (PicMotionParams**)malloc(dim0 *      sizeof(PicMotionParams*))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");
  if((*(*array2D) = (PicMotionParams* )calloc(dim0 * dim1,sizeof(PicMotionParams ))) == NULL)
    no_mem_exit("get_mem2Dmp: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(PicMotionParams*) + dim1 * sizeof(PicMotionParams));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> PicMotionParams array3D[frames][dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dmp(PicMotionParams ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(PicMotionParams**);

  if(((*array3D) = (PicMotionParams***)malloc(dim0 * sizeof(PicMotionParams**))) == NULL)
    no_mem_exit("get_mem3Dmp: array3D");

  mem_size += get_mem2Dmp(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dmp()
 ************************************************************************
 */
void free_mem2Dmp(PicMotionParams **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dmp: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dmp: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dmp()
 ************************************************************************
 */
void free_mem3Dmp(PicMotionParams ***array3D)
{
  if (array3D)
  {
    free_mem2Dmp(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dmp: trying to free unused memory",100);
  }
}



/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LevelQuantParams array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dquant(LevelQuantParams ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (LevelQuantParams**) malloc(dim0 * sizeof(LevelQuantParams*))) == NULL)
    no_mem_exit("get_mem2Dquant: array2D");
  if((*(*array2D) = (LevelQuantParams* ) calloc(dim0 * dim1,sizeof(LevelQuantParams ))) == NULL)
    no_mem_exit("get_mem2Dquant: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(LevelQuantParams*) + dim1 * sizeof(LevelQuantParams));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> LevelQuantParams array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dquant(LevelQuantParams ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams**);

  if(((*array3D) = (LevelQuantParams***)malloc(dim0 * sizeof(LevelQuantParams**))) == NULL)
    no_mem_exit("get_mem3Dquant: array3D");

  mem_size += get_mem2Dquant(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> LevelQuantParams array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dquant(LevelQuantParams *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams***);

  if(((*array4D) = (LevelQuantParams****)malloc(dim0 * sizeof(LevelQuantParams***))) == NULL)
    no_mem_exit("get_mem4Dquant: array4D");

  mem_size += get_mem3Dquant(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> LevelQuantParams array3D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dquant(LevelQuantParams ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int i, mem_size = dim0 * sizeof(LevelQuantParams***);

  if(((*array5D) = (LevelQuantParams*****)malloc(dim0 * sizeof(LevelQuantParams****))) == NULL)
    no_mem_exit("get_mem5Dquant: array5D");

  mem_size += get_mem4Dquant(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dquant()
 ************************************************************************
 */
void free_mem2Dquant(LevelQuantParams **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dquant: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dquant: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dquant()
 ************************************************************************
 */
void free_mem3Dquant(LevelQuantParams ***array3D)
{
  if (array3D)
  {
    free_mem2Dquant(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dquant: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dquant()
 ************************************************************************
 */
void free_mem4Dquant(LevelQuantParams ****array4D)
{
  if (array4D)
  {
    free_mem3Dquant(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dquant: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dquant()
 ************************************************************************
 */
void free_mem5Dquant(LevelQuantParams *****array5D)
{
  if (array5D)
  {
    free_mem4Dquant(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dquant: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> StorablePicturePtr array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2D_spp(StorablePicturePtr ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (StorablePicturePtr**)malloc(dim0 *      sizeof(StorablePicturePtr*))) == NULL)
    no_mem_exit("get_mem2D_spp: array2D");
  if((*(*array2D) = (StorablePicturePtr* )calloc(dim0 * dim1,sizeof(StorablePicturePtr ))) == NULL)
    no_mem_exit("get_mem2D_spp: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(StorablePicturePtr*) + dim1 * sizeof(StorablePicturePtr));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> StorablePicturePtr array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3D_spp(StorablePicturePtr ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(StorablePicturePtr**);

  if(((*array3D) = (StorablePicturePtr***)malloc(dim0 * sizeof(StorablePicturePtr**))) == NULL)
    no_mem_exit("get_mem3D_spp: array3D");

  mem_size += get_mem2D_spp(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> MotionVector array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dmv(MotionVector ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (MotionVector**)malloc(dim0 *      sizeof(MotionVector*))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");
  if((*(*array2D) = (MotionVector* )calloc(dim0 * dim1,sizeof(MotionVector ))) == NULL)
    no_mem_exit("get_mem2Dmv: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(MotionVector*) + dim1 * sizeof(MotionVector));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> MotionVector array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dmv(MotionVector ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(MotionVector**);

  if(((*array3D) = (MotionVector***)malloc(dim0 * sizeof(MotionVector**))) == NULL)
    no_mem_exit("get_mem3Dmv: array3D");

  mem_size += get_mem2Dmv(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> MotionVector array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dmv(MotionVector *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int i, mem_size = dim0 * sizeof(MotionVector***);

  if(((*array4D) = (MotionVector****)malloc(dim0 * sizeof(MotionVector***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  mem_size += get_mem3Dmv(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> MotionVector array3D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dmv(MotionVector ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int i, mem_size = dim0 * sizeof(MotionVector***);

  if(((*array5D) = (MotionVector*****)malloc(dim0 * sizeof(MotionVector****))) == NULL)
    no_mem_exit("get_mem5Dpel: array5D");

  mem_size += get_mem4Dmv(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 6D memory array -> MotionVector array6D[dim0][dim1][dim2][dim3][dim4][dim5]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem6Dmv(MotionVector *******array6D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5)
{
  int i, mem_size = dim0 * sizeof(MotionVector*****);

  if(((*array6D) = (MotionVector******)malloc(dim0 * sizeof(MotionVector*****))) == NULL)
    no_mem_exit("get_mem5Dpel: array6D");

  mem_size += get_mem5Dmv(*array6D, dim0 * dim1, dim2, dim3, dim4, dim5);

  for(i = 1; i < dim0; i++)
    (*array6D)[i] = (*array6D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 7D memory array -> MotionVector array7D[dim0][dim1][dim2][dim3][dim4][dim5][dim6]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem7Dmv(MotionVector ********array7D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
  int i, mem_size = dim0 * sizeof(MotionVector******);

  if(((*array7D) = (MotionVector*******)malloc(dim0 * sizeof(MotionVector******))) == NULL)
    no_mem_exit("get_mem7Dmv: array7D");

  mem_size += get_mem6Dmv(*array7D, dim0 * dim1, dim2, dim3, dim4, dim5, dim6);

  for(i = 1; i < dim0; i++)
    (*array7D)[i] = (*array7D)[i - 1] + dim1;
  
  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2D_spp()
 ************************************************************************
 */
void free_mem2D_spp(StorablePicturePtr **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2D_spp: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2D_spp: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3D_spp()
 ************************************************************************
 */
void free_mem3D_spp(StorablePicturePtr ***array3D)
{
  if (array3D)
  {
    free_mem2D_spp(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3D_spp: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dmv()
 ************************************************************************
 */
void free_mem2Dmv(MotionVector **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dmv: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dmv: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dmv()
 ************************************************************************
 */
void free_mem3Dmv(MotionVector ***array3D)
{
  if (array3D)
  {
    free_mem2Dmv(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dmv()
 ************************************************************************
 */
void free_mem4Dmv(MotionVector ****array4D)
{
  if (array4D)
  {
    free_mem3Dmv(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dmv()
 ************************************************************************
 */
void free_mem5Dmv(MotionVector *****array5D)
{
  if (array5D)
  {
    free_mem4Dmv(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dmv: trying to free unused memory",100);
  }
}
/*!
 ************************************************************************
 * \brief
 *    free 6D memory array
 *    which was allocated with get_mem6Dmv()
 ************************************************************************
 */
void free_mem6Dmv(MotionVector ******array6D)
{
  if (array6D)
  {
    free_mem5Dmv(*array6D);
    free (array6D);
  }
  else
  {
    error ("free_mem6Dmv: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 7D memory array
 *    which was allocated with get_mem7Dmv()
 ************************************************************************
 */
void free_mem7Dmv(MotionVector *******array7D)
{
  if (array7D)
  {
    free_mem6Dmv(*array7D);
    free (array7D);
  }
  else
  {
    error ("free_mem7Dmv: trying to free unused memory",100);
  }
}



/*!
 ************************************************************************
 * \brief
 *    Allocate 1D memory array -> imgpel array1D[dim0
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem1Dpel(imgpel **array1D, int dim0)
{
  if((*array1D    = (imgpel*)calloc(dim0,       sizeof(imgpel))) == NULL)
    no_mem_exit("get_mem1Dpel: arra12D");

  return (sizeof(imgpel*) + dim0 * sizeof(imgpel));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> imgpel array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (imgpel**)malloc(dim0 * sizeof(imgpel*))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");
  if((*(*array2D) = (imgpel* )calloc(dim0 * dim1,sizeof(imgpel ))) == NULL)
    no_mem_exit("get_mem2Dpel: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(imgpel*) + dim1 * sizeof(imgpel));
}

int get_mem2DpelWithPad(imgpel ***array2D, int dim0, int dim1, int iPadY, int iPadX)
{
  int i;
  imgpel *curr = NULL;
  int iHeight, iWidth;
  
  iHeight = dim0+2*iPadY;
  iWidth = dim1+2*iPadX;
  if((*array2D    = (imgpel**)malloc(iHeight*sizeof(imgpel*))) == NULL)
    no_mem_exit("get_mem2DpelWithPad: array2D");
  if((*(*array2D) = (imgpel* )calloc(iHeight * iWidth, sizeof(imgpel ))) == NULL)
    no_mem_exit("get_mem2DpelWithPad: array2D");

  (*array2D)[0] += iPadX;
  curr = (*array2D)[0];
  for(i = 1 ; i < iHeight; i++)
  {
    curr += iWidth;
    (*array2D)[i] = curr;
  }
  (*array2D) = &((*array2D)[iPadY]);

  return iHeight * (sizeof(imgpel*) + iWidth * sizeof(imgpel));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> imgpel array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dpel(imgpel ****array3D, int dim0, int dim1, int dim2)
{
  int i, mem_size = dim0 * sizeof(imgpel**);

  if(((*array3D) = (imgpel***)malloc(dim0 * sizeof(imgpel**))) == NULL)
    no_mem_exit("get_mem3Dpel: array3D");

  mem_size += get_mem2Dpel(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + dim1;
  
  return mem_size;
}

int get_mem3DpelWithPad(imgpel ****array3D, int dim0, int dim1, int dim2, int iPadY, int iPadX)
{
  int i, mem_size = dim0 * sizeof(imgpel**);

  if(((*array3D) = (imgpel***)malloc(dim0*sizeof(imgpel**))) == NULL)
    no_mem_exit("get_mem3DpelWithPad: array3D");

  mem_size += get_mem2DpelWithPad(*array3D, dim0*dim1+2*(dim0-1)*iPadY, dim2, iPadY, iPadX);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] = (*array3D)[i - 1] + (dim1+2*iPadY);
  
  return mem_size;
}

int get_mem3DpelWithPadSeparately(imgpel ****array3D, int dim0, int dim1, int dim2, int iPadY, int iPadX)
{
  int i, mem_size = dim0 * sizeof(imgpel**);

  if(((*array3D) = (imgpel***)malloc(dim0*sizeof(imgpel**))) == NULL)
    no_mem_exit("get_mem3DpelWithPadSeparately: array3D");

  for(i = 0; i < dim0; i++)
    mem_size += get_mem2DpelWithPad((*array3D)+i, dim1, dim2, iPadY, iPadX);
  
  return mem_size;
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> imgpel array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dpel(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3)
{  
  int  i, mem_size = dim0 * sizeof(imgpel***);

  if(((*array4D) = (imgpel****)malloc(dim0 * sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4Dpel: array4D");

  mem_size += get_mem3Dpel(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}

int get_mem4DpelWithPad(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3, int iPadY, int iPadX)
{  
  int  i, mem_size = dim0 * sizeof(imgpel***);

  if(((*array4D) = (imgpel****)malloc(dim0 * sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4DpelWithPad: array4D");

  mem_size += get_mem3DpelWithPad(*array4D, dim0 * dim1, dim2, dim3, iPadY, iPadX);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}

int get_mem4DpelWithPadSeparately(imgpel *****array4D, int dim0, int dim1, int dim2, int dim3, int iPadY, int iPadX)
{  
  int  i, mem_size = dim0 * sizeof(imgpel***);

  if(((*array4D) = (imgpel****)malloc(dim0 * sizeof(imgpel***))) == NULL)
    no_mem_exit("get_mem4DpelWithPadSeparately: array4D");

  mem_size += get_mem3DpelWithPadSeparately(*array4D, dim0 * dim1, dim2, dim3, iPadY, iPadX);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] = (*array4D)[i - 1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> imgpel array5D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dpel(imgpel ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i, mem_size = dim0 * sizeof(imgpel****);

  if(((*array5D) = (imgpel*****)malloc(dim0 * sizeof(imgpel****))) == NULL)
    no_mem_exit("get_mem5Dpel: array5D");

  mem_size += get_mem4Dpel(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;

  return mem_size;
}

int get_mem5DpelWithPad(imgpel ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4, int iPadY, int iPadX)
{
  int  i, mem_size = dim0 * sizeof(imgpel****);

  if(((*array5D) = (imgpel*****)malloc(dim0 * sizeof(imgpel****))) == NULL)
    no_mem_exit("get_mem5DpelWithPad: array5D");

  mem_size += get_mem4DpelWithPad(*array5D, dim0 * dim1, dim2, dim3, dim4, iPadY, iPadX);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;

  return mem_size;
}

int get_mem5DpelWithPadSeparately(imgpel ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4, int iPadY, int iPadX)
{
  int  i, mem_size = dim0 * sizeof(imgpel****);

  if(((*array5D) = (imgpel*****)malloc(dim0 * sizeof(imgpel****))) == NULL)
    no_mem_exit("get_mem5DpelWithPadSeparately: array5D");

  mem_size += get_mem4DpelWithPadSeparately(*array5D, dim0 * dim1, dim2, dim3, dim4, iPadY, iPadX);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] = (*array5D)[i - 1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 1D memory array
 *    which was allocated with get_mem1Dpel()
 ************************************************************************
 */
void free_mem1Dpel(imgpel *array1D)
{
  if (array1D)
  {
    free (array1D);
  } 
  else
  {
    error ("free_mem1Dpel: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dpel()
 ************************************************************************
 */
void free_mem2Dpel(imgpel **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dpel: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dpel: trying to free unused memory",100);
  }
}

void free_mem2DpelWithPad(imgpel **array2D, int iPadY, int iPadX)
{
  if (array2D)
  {
    if (*array2D)
      free (array2D[-iPadY]-iPadX);
    else 
      error ("free_mem2DpelWithPad: trying to free unused memory",100);

    free (&array2D[-iPadY]);
  } 
  else
  {
    error ("free_mem2DpelWithPad: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dpel()
 ************************************************************************
 */
void free_mem3Dpel(imgpel ***array3D)
{
  if (array3D)
  {
    free_mem2Dpel(*array3D);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dpel: trying to free unused memory",100);
  }
}

void free_mem3DpelWithPad(imgpel ***array3D, int iPadY, int iPadX)
{
  if (array3D)
  {
    free_mem2DpelWithPad(*array3D, iPadY, iPadX);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dpel: trying to free unused memory",100);
  }
  
}

void free_mem3DpelWithPadSeparately(imgpel ***array3D, int iDim12, int iPadY, int iPadX)
{
  if (array3D)
  {
    int i;
    for(i=0; i<iDim12; i++)
      if(array3D[i])
      {
        free_mem2DpelWithPad(array3D[i], iPadY, iPadX);
        array3D[i] = NULL;
      }
    free (array3D);
  }
  else
  {
    error ("free_mem3DpelWithPadSeparately: trying to free unused memory",100);
  }
  
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dpel()
 ************************************************************************
 */
void free_mem4Dpel(imgpel ****array4D)
{
  if (array4D)
  {
    free_mem3Dpel(*array4D);
    free (array4D);
  }
  else
  {
    error ("free_mem4Dpel: trying to free unused memory",100);
  }
}

void free_mem4DpelWithPad(imgpel  ****array4D, int iPadY, int iPadX)
{
  if (array4D)
  {
    free_mem3DpelWithPad(*array4D, iPadY, iPadX);
    free (array4D);
  }
  else
  {
    error ("free_mem4DpelWithPad: trying to free unused memory",100);
  }
}

void free_mem4DpelWithPadSeparately(imgpel  ****array4D, int iFrames, int iPadY, int iPadX)
{
  if (array4D)
  {
    free_mem3DpelWithPadSeparately(*array4D, iFrames, iPadY, iPadX);
    free (array4D);
  }
  else
  {
    error ("free_mem4DpelWithPadSeparately: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D memory array
 *    which was allocated with get_mem5Dpel()
 ************************************************************************
 */
void free_mem5Dpel(imgpel *****array5D)
{
  if (array5D)
  {
    free_mem4Dpel(*array5D);
    free (array5D);
  }
  else
  {
    error ("free_mem5Dpel: trying to free unused memory",100);
  }
}

void free_mem5DpelWithPad(imgpel *****array5D, int iPadY, int iPadX)
{
  if (array5D)
  {
    free_mem4DpelWithPad(*array5D, iPadY, iPadX);
    free (array5D);
  }
  else
  {
    error ("free_mem5DpelWithPad: trying to free unused memory",100);
  }
}

void free_mem5DpelWithPadSeparately(imgpel *****array5D, int iFrames, int iPadY, int iPadX)
{
  if (array5D)
  {
    free_mem4DpelWithPadSeparately(*array5D, iFrames, iPadY, iPadX);
    free (array5D);
  }
  else
  {
    error ("free_mem5DpelWithPad: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Create 2D memory array -> byte array2D[dim0][dim1]
 *
 * \par Output:
 *    byte type array of size dim0 * dim1
 ************************************************************************
 */
byte** new_mem2D(int dim0, int dim1)
{
  int i;
  byte **array2D;

  if((  array2D  = (byte**)malloc(dim0 *      sizeof(byte*))) == NULL)
    no_mem_exit("get_mem2D: array2D");
  if((*(array2D) = (byte* )calloc(dim0 * dim1,sizeof(byte ))) == NULL)
    no_mem_exit("get_mem2D: array2D");

  for(i = 1; i < dim0; i++)
    array2D[i] = array2D[i-1] + dim1;

  return (array2D);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> unsigned char array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************/
int get_mem2D(byte ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (byte**)malloc(dim0 *      sizeof(byte*))) == NULL)
    no_mem_exit("get_mem2D: array2D");
  if((*(*array2D) = (byte* )calloc(dim0 * dim1,sizeof(byte ))) == NULL)
    no_mem_exit("get_mem2D: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(byte*) + dim1 * sizeof(byte));
}


/*!
 ************************************************************************
 * \brief
 *    Create 2D memory array -> int array2D[dim0][dim1]
 *
 * \par Output:
 *    int type array of size dim0 * dim1
 ************************************************************************
 */
int** new_mem2Dint(int dim0, int dim1)
{
  int i;
  int **array2D;

  if((array2D    = (int**)malloc(dim0 *        sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if((*(array2D) = (int* )calloc(dim0 * dim1, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  for(i = 1 ; i < dim0; i++)
    (array2D)[i] =  (array2D)[i-1] + dim1;

  return (array2D);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dint(int ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int**)malloc(dim0 *       sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if((*(*array2D) = (int* )calloc(dim0 * dim1, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  for(i = 1 ; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(int*) + dim1 * sizeof(int));
}

int get_mem2DintWithPad(int ***array2D, int dim0, int dim1, int iPadY, int iPadX)
{
  int i;
  int *curr = NULL;
  int iHeight, iWidth;
  
  iHeight = dim0+2*iPadY;
  iWidth = dim1+2*iPadX;
  if((*array2D    = (int**)malloc(iHeight*sizeof(int*))) == NULL)
    no_mem_exit("get_mem2DintWithPad: array2D");
  if((*(*array2D) = (int* )calloc(iHeight * iWidth, sizeof(int ))) == NULL)
    no_mem_exit("get_mem2DintWithPad: array2D");

  (*array2D)[0] += iPadX;
  curr = (*array2D)[0];
  for(i = 1 ; i < iHeight; i++)
  {
    curr += iWidth;
    (*array2D)[i] = curr;
  }
  (*array2D) = &((*array2D)[iPadY]);

  return iHeight * (sizeof(int*) + iWidth * sizeof(int));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int64 array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dint64(int64 ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (int64**)malloc(dim0 *      sizeof(int64*))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");
  if((*(*array2D) = (int64* )calloc(dim0 * dim1,sizeof(int64 ))) == NULL)
    no_mem_exit("get_mem2Dint64: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(int64*) + dim1 * sizeof(int64));
}

int get_mem2Ddistblk(distblk ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D    = (distblk**)malloc(dim0 *      sizeof(distblk*))) == NULL)
    no_mem_exit("get_mem2Ddistblk: array2D");
  if((*(*array2D) = (distblk* )calloc(dim0 * dim1,sizeof(distblk ))) == NULL)
    no_mem_exit("get_mem2Ddistblk: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(distblk*) + dim1 * sizeof(distblk));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> unsigned char array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3D(byte ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(byte**);

  if(((*array3D) = (byte***)malloc(dim0 * sizeof(byte**))) == NULL)
    no_mem_exit("get_mem3D: array3D");

  mem_size += get_mem2D(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> unsigned char array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4D(byte *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(byte***);

  if(((*array4D) = (byte****)malloc(dim0 * sizeof(byte***))) == NULL)
    no_mem_exit("get_mem4D: array4D");

  mem_size += get_mem3D(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint(int ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(int**);

  if(((*array3D) = (int***)malloc(dim0 * sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Dint: array3D");

  mem_size += get_mem2Dint(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int64 array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dint64(int64 ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(int64**);

  if(((*array3D) = (int64***)malloc(dim0 * sizeof(int64**))) == NULL)
    no_mem_exit("get_mem3Dint64: array3D");

  mem_size += get_mem2Dint64(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

int get_mem3Ddistblk(distblk ****array3D, int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(distblk**);

  if(((*array3D) = (distblk***)malloc(dim0 * sizeof(distblk**))) == NULL)
    no_mem_exit("get_mem3Ddistblk: array3D");

  mem_size += get_mem2Ddistblk(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory array -> int array4D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dint(int *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(int***);

  if(((*array4D) = (int****)malloc(dim0 * sizeof(int***))) == NULL)
    no_mem_exit("get_mem4Dint: array4D");

  mem_size += get_mem3Dint(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

int get_mem4Dint64(int64 *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(int64***);

  if(((*array4D) = (int64****)malloc(dim0 * sizeof(int64***))) == NULL)
    no_mem_exit("get_mem4Dint64: array4D");

  mem_size += get_mem3Dint64(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

int get_mem4Ddistblk(distblk *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(distblk***);

  if(((*array4D) = (distblk****)malloc(dim0 * sizeof(distblk***))) == NULL)
    no_mem_exit("get_mem4Ddistblk: array4D");

  mem_size += get_mem3Ddistblk(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> int array5D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dint(int ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i, mem_size = dim0 * sizeof(int****);

  if(((*array5D) = (int*****)malloc(dim0 * sizeof(int****))) == NULL)
    no_mem_exit("get_mem5Dint: array5D");

  mem_size += get_mem4Dint(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] =  (*array5D)[i-1] + dim1;

  return mem_size;
}


/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2D()
 ************************************************************************
 */
void free_mem2D(byte **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2D: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2D: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint()
 ************************************************************************
 */
void free_mem2Dint(int **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dint: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dint: trying to free unused memory",100);
  }
}

void free_mem2DintWithPad(int **array2D, int iPadY, int iPadX)
{
  if (array2D)
  {
    if (*array2D)
      free (array2D[-iPadY]-iPadX);
    else 
      error ("free_mem2DintWithPad: trying to free unused memory",100);

    free (&array2D[-iPadY]);
  } 
  else
  {
    error ("free_mem2DintWithPad: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D memory array
 *    which was allocated with get_mem2Dint64()
 ************************************************************************
 */
void free_mem2Dint64(int64 **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Dint64: trying to free unused memory",100);
    free (array2D);
  } 
  else
  {
    error ("free_mem2Dint64: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3D()
 ************************************************************************
 */
void free_mem3D(byte ***array3D)
{
  if (array3D)
  {
   free_mem2D(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3D: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem3D()
 ************************************************************************
 */
void free_mem4D(byte ****array4D)
{
  if (array4D)
  {
   free_mem3D(*array4D);
   free (array4D);
  } 
  else
  {
    error ("free_mem4D: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Dint(int ***array3D)
{
  if (array3D)
  {
   free_mem2Dint(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dint: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was allocated with get_mem3Dint64()
 ************************************************************************
 */
void free_mem3Dint64(int64 ***array3D)
{
  if (array3D)
  {
   free_mem2Dint64(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dint64: trying to free unused memory",100);
  }
}

void free_mem3Ddistblk(distblk ***array3D)
{
  if (array3D)
  {
   free_mem2Ddistblk(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Ddistblk: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D memory array
 *    which was allocated with get_mem4Dint()
 ************************************************************************
 */
void free_mem4Dint(int ****array4D)
{
  if (array4D)
  {
    free_mem3Dint( *array4D);
    free (array4D);
  } else
  {
    error ("free_mem4Dint: trying to free unused memory",100);
  }
}

void free_mem4Dint64(int64 ****array4D)
{
  if (array4D)
  {
    free_mem3Dint64( *array4D);
    free (array4D);
  } else
  {
    error ("free_mem4Dint64: trying to free unused memory",100);
  }
}

void free_mem4Ddistblk(distblk ****array4D)
{
  if (array4D)
  {
    free_mem3Ddistblk( *array4D);
    free (array4D);
  } else
  {
    error ("free_mem4Ddistblk: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D int memory array
 *    which was allocated with get_mem5Dint()
 ************************************************************************
 */
void free_mem5Dint(int *****array5D)
{
  if (array5D)
  {
    free_mem4Dint( *array5D);
    free (array5D);
  } else
  {
    error ("free_mem5Dint: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Exit program if memory allocation failed (using error())
 * \param where
 *    string indicating which memory allocation failed
 ************************************************************************
 */
void no_mem_exit(char *where)
{
   snprintf(errortext, ET_SIZE, "Could not allocate memory: %s",where);
  /* error (errortext, 100);*/
}


/*!
 ************************************************************************
 * \brief
 *    Create 2D memory array -> uint16 array2D[dim0][dim1]
 *
 * \par Output:
 *    uint16 type array of size dim0 * dim1
 ************************************************************************
 */
uint16** new_mem2Duint16(int dim0, int dim1)
{
  int i;
  uint16 **array2D;

  if(( array2D = (uint16**)malloc(dim0 *      sizeof(uint16*))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");
  if((*array2D = (uint16* )calloc(dim0 * dim1,sizeof(uint16 ))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");

  for(i = 1; i < dim0; i++)
    array2D[i] = array2D[i-1] + dim1;

  return (array2D);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D uint16 memory array -> uint16 array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Duint16(uint16 ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (uint16**)malloc(dim0 *      sizeof(uint16*))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");

  if((*(*array2D) = (uint16* )calloc(dim0 * dim1,sizeof(uint16 ))) == NULL)
    no_mem_exit("get_mem2Duint16: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(uint16*) + dim1 * sizeof(uint16));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory uint16 array -> uint16 array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Duint16(uint16 ****array3D,int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(uint16**);

  if(((*array3D) = (uint16***)malloc(dim0 * sizeof(uint16**))) == NULL)
    no_mem_exit("get_mem3Duint16: array3D");

  mem_size += get_mem2Duint16(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory uint16 array -> uint16 array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Duint16(uint16 *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(uint16***);

  if(((*array4D) = (uint16****)malloc(dim0 * sizeof(uint16***))) == NULL)
    no_mem_exit("get_mem4Duint16: array4D");

  mem_size += get_mem3Duint16(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D short memory array -> short array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dshort(short ***array2D, int dim0, int dim1)
{
  int i;

  if((  *array2D  = (short**)malloc(dim0 *      sizeof(short*))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");
  if((*(*array2D) = (short* )calloc(dim0 * dim1,sizeof(short ))) == NULL)
    no_mem_exit("get_mem2Dshort: array2D");

  for(i = 1; i < dim0; i++)
    (*array2D)[i] = (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(short*) + dim1 * sizeof(short));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory short array -> short array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dshort(short ****array3D,int dim0, int dim1, int dim2)
{
  int  i, mem_size = dim0 * sizeof(short**);

  if(((*array3D) = (short***)malloc(dim0 * sizeof(short**))) == NULL)
    no_mem_exit("get_mem3Dshort: array3D");

  mem_size += get_mem2Dshort(*array3D, dim0 * dim1, dim2);

  for(i = 1; i < dim0; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1;

  return mem_size;
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 4D memory short array -> short array3D[dim0][dim1][dim2][dim3]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem4Dshort(short *****array4D, int dim0, int dim1, int dim2, int dim3)
{
  int  i, mem_size = dim0 * sizeof(short***);

  if(((*array4D) = (short****)malloc(dim0 * sizeof(short***))) == NULL)
    no_mem_exit("get_mem4Dshort: array4D");

  mem_size += get_mem3Dshort(*array4D, dim0 * dim1, dim2, dim3);

  for(i = 1; i < dim0; i++)
    (*array4D)[i] =  (*array4D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 5D memory array -> short array5D[dim0][dim1][dim2][dim3][dim4]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem5Dshort(short ******array5D, int dim0, int dim1, int dim2, int dim3, int dim4)
{
  int  i, mem_size = dim0 * sizeof(short****);

  if(((*array5D) = (short*****)malloc(dim0 * sizeof(short****))) == NULL)
    no_mem_exit("get_mem5Dshort: array5D");

  mem_size += get_mem4Dshort(*array5D, dim0 * dim1, dim2, dim3, dim4);

  for(i = 1; i < dim0; i++)
    (*array5D)[i] =  (*array5D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 6D memory array -> short array6D[dim0][dim1][dim2][dim3][dim4][dim5]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem6Dshort(short *******array6D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5)
{
  int  i, mem_size = dim0 * sizeof(short*****);

  if(((*array6D) = (short******)malloc(dim0 * sizeof(short*****))) == NULL)
    no_mem_exit("get_mem6Dshort: array6D");

  mem_size += get_mem5Dshort(*array6D, dim0 * dim1, dim2, dim3, dim4, dim5);

  for(i = 1; i < dim0; i++)
    (*array6D)[i] =  (*array6D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 7D memory array -> short array7D[dim0][dim1][dim2][dim3][dim4][dim5][dim6]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem7Dshort(short ********array7D, int dim0, int dim1, int dim2, int dim3, int dim4, int dim5, int dim6)
{
  int  i, mem_size = dim0 * sizeof(short******);

  if(((*array7D) = (short*******)malloc(dim0 * sizeof(short******))) == NULL)
    no_mem_exit("get_mem7Dshort: array7D");

  mem_size += get_mem6Dshort(*array7D, dim0 * dim1, dim2, dim3, dim4, dim5, dim6);

  for(i = 1; i < dim0; i++)
    (*array7D)[i] =  (*array7D)[i-1] + dim1;

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D uint16 memory array
 *    which was allocated with get_mem2Duint16()
 ************************************************************************
 */
void free_mem2Duint16(uint16 **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else error ("free_mem2Duint16: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Duint16: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D uint16 memory array
 *    which was allocated with get_mem3Duint16()
 ************************************************************************
 */
void free_mem3Duint16(uint16 ***array3D)
{
  if (array3D)
  {
   free_mem2Duint16(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Duint16: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D uint16 memory array
 *    which was allocated with get_mem4Duint16()
 ************************************************************************
 */
void free_mem4Duint16(uint16 ****array4D)
{  
  if (array4D)
  {
    free_mem3Duint16( *array4D);
    free (array4D);
  } 
  else
  {
    error ("free_mem4Duint16: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 2D short memory array
 *    which was allocated with get_mem2Dshort()
 ************************************************************************
 */
void free_mem2Dshort(short **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else error ("free_mem2Dshort: trying to free unused memory",100);

    free (array2D);
  } 
  else
  {
    error ("free_mem2Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D short memory array
 *    which was allocated with get_mem3Dshort()
 ************************************************************************
 */
void free_mem3Dshort(short ***array3D)
{
  if (array3D)
  {
   free_mem2Dshort(*array3D);
   free (array3D);
  } 
  else
  {
    error ("free_mem3Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 4D short memory array
 *    which was allocated with get_mem4Dshort()
 ************************************************************************
 */
void free_mem4Dshort(short ****array4D)
{  
  if (array4D)
  {
    free_mem3Dshort( *array4D);
    free (array4D);
  } 
  else
  {
    error ("free_mem4Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 5D short memory array
 *    which was allocated with get_mem5Dshort()
 ************************************************************************
 */
void free_mem5Dshort(short *****array5D)
{
  if (array5D)
  {
    free_mem4Dshort( *array5D) ;
    free (array5D);
  }
  else
  {
    error ("free_mem5Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 6D short memory array
 *    which was allocated with get_mem6Dshort()
 ************************************************************************
 */
void free_mem6Dshort(short ******array6D)
{
  if (array6D)
  {
    free_mem5Dshort( *array6D);
    free (array6D);
  }
  else
  {
    error ("free_mem6Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 7D short memory array
 *    which was allocated with get_mem7Dshort()
 ************************************************************************
 */
void free_mem7Dshort(short *******array7D)
{
  if (array7D)
  {
    free_mem6Dshort( *array7D);
    free (array7D);
  }
  else
  {
    error ("free_mem7Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[dim0][dim1]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Ddouble(double ***array2D, int dim0, int dim1)
{
  int i;

  if((*array2D      = (double**)malloc(dim0 * sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");
  
  if(((*array2D)[0] = (double* )calloc(dim0 * dim1,sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Ddouble: array2D");

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(double*) + dim1 * sizeof(double));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 1D memory array -> double array1D[dim0]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem1Dodouble(double **array1D, int dim0, int offset)
{
  if((*array1D      = (double*)malloc(dim0 *       sizeof(double))) == NULL)
    no_mem_exit("get_mem1Dodouble: array2D");

  *array1D += offset;

  return dim0 * sizeof(double);
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> double array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dodouble(double ***array2D, int dim0, int dim1, int offset)
{
  int i;

  if((*array2D      = (double**)malloc(dim0 *       sizeof(double*))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");
  if(((*array2D)[0] = (double* )calloc(dim0 * dim1, sizeof(double ))) == NULL)
    no_mem_exit("get_mem2Dodouble: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(double*) + dim1 * sizeof(double));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory double array -> double array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Dodouble(double ****array3D, int dim0, int dim1, int dim2, int offset)
{
  int  i,j;

  if(((*array3D) = (double***)malloc(dim0 * sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  if(((*array3D)[0] = (double** )calloc(dim0 * dim1, sizeof(double*))) == NULL)
    no_mem_exit("get_mem3Dodouble: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1  ;

  for (i = 0; i < dim0; i++)
    for (j = -offset; j < dim1 - offset; j++)
      if(((*array3D)[i][j] = (double* )calloc(dim2, sizeof(double))) == NULL)
        no_mem_exit("get_mem3Dodouble: array3D");

  return dim0*( sizeof(double**) + dim1 * ( sizeof(double*) + dim2 * sizeof(double)));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_offset_mem2Dshort(short ***array2D, int dim0, int dim1, int offset_y, int offset_x)
{
  int i;

  if((*array2D      = (short**)malloc(dim0 * sizeof(short*))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");

  if(((*array2D)[0] = (short* )calloc(dim0 * dim1, sizeof(short))) == NULL)
    no_mem_exit("get_offset_mem2Dshort: array2D");
  (*array2D)[0] += offset_x + offset_y * dim1;

  for(i=-1 ; i > -offset_y - 1; i--)
  {
    (*array2D)[i] =  (*array2D)[i+1] - dim1;
  }

  for(i=1 ; i < dim1 - offset_y; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1;

  return dim0 * (sizeof(short*) + dim1 * sizeof(short));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory int array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem3Doint(int ****array3D, int dim0, int dim1, int dim2, int offset)
{
  int  i,j;

  if(((*array3D) = (int***)malloc(dim0 * sizeof(int**))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  if(((*array3D)[0] = (int** )calloc(dim0 * dim1, sizeof(int*))) == NULL)
    no_mem_exit("get_mem3Doint: array3D");

  (*array3D) [0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array3D)[i] =  (*array3D)[i-1] + dim1  ;

  for (i = 0; i < dim0; i++)
    for (j = -offset; j < dim1 - offset; j++)
      if(((*array3D)[i][j] = (int* )calloc(dim2, sizeof(int))) == NULL)
        no_mem_exit("get_mem3Doint: array3D");

  return dim0 * (sizeof(int**) + dim1 * (sizeof(int*) + dim2 * sizeof(int)));
}

/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> int array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Doint(int ***array2D, int dim0, int dim1, int offset)
{
  int i;

  if((*array2D      = (int**)malloc(dim0 * sizeof(int*))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");
  if(((*array2D)[0] = (int* )calloc(dim0 * dim1, sizeof(int))) == NULL)
    no_mem_exit("get_mem2Dint: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(int*) + dim1 * sizeof(int));
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 3D memory array -> int array3D[dim0][dim1][dim2]
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
// same change as in get_mem3Dint
int get_mem3Ddouble(double ****array3D, int dim0, int dim1, int dim2)
{
	int  j, mem_size = dim0 * sizeof(double**);

  double **array2D;

  if(((*array3D) = (double***)malloc(dim0 * sizeof(double**))) == NULL)
    no_mem_exit("get_mem3Ddouble: array3D");

  mem_size += get_mem2Ddouble(&array2D, dim0 * dim1, dim2);

  for(j = 0; j < dim0; j++)
  {    
    (*array3D)[j] = &array2D[j * dim1];
  }

  return mem_size;
}

/*!
 ************************************************************************
 * \brief
 *    free 2D double memory array
 *    which was allocated with get_mem2Ddouble()
 ************************************************************************
 */
void free_mem2Ddouble(double **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Ddouble: trying to free unused memory",100);

    free (array2D);

  }
  else
  {
    error ("free_mem2Ddouble: trying to free unused memory",100);
  }
}

/*!
************************************************************************
* \brief
*    free 1D double memory array (with offset)
*    which was allocated with get_mem1Ddouble()
************************************************************************
*/
void free_mem1Dodouble(double *array1D, int offset)
{
  if (array1D)
  {
    array1D -= offset;
    free (array1D);
  } 
  else
  {
    error ("free_mem1Dodouble: trying to free unused memory",100);
  }
}

/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Dodouble(double **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else error ("free_mem2Dodouble: trying to free unused memory",100);

    free (array2D);

  } else
  {
    error ("free_mem2Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Dodouble(double ***array3D, int dim0, int dim1, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < dim0; i++)
    {
      for (j = -offset; j < dim1 - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Dodouble: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Dodouble: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Dodouble: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array with offset
 ************************************************************************
 */
void free_mem3Doint(int ***array3D, int dim0, int dim1, int offset)
{
  int i, j;

  if (array3D)
  {
    for (i = 0; i < dim0; i++)
    {
      for (j = -offset; j < dim1 - offset; j++)
      {
        if (array3D[i][j])
          free(array3D[i][j]);
        else
          error ("free_mem3Doint: trying to free unused memory",100);
      }
    }
    array3D[0] -= offset;
    if (array3D[0])
      free(array3D[0]);
    else
      error ("free_mem3Doint: trying to free unused memory",100);
    free (array3D);
  }
  else
  {
    error ("free_mem3Doint: trying to free unused memory",100);
  }
}


/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_mem2Doint(int **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_mem2Doint: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_mem2Doint: trying to free unused memory",100);
  }
}

/*!
************************************************************************
* \brief
*    free 2D double memory array (with offset)
*    which was allocated with get_mem2Ddouble()
************************************************************************
*/
void free_offset_mem2Dshort(short **array2D, int dim1, int offset_y, int offset_x)
{
  if (array2D)
  {
    array2D[0] -= offset_x + offset_y * dim1;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_offset_mem2Dshort: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_offset_mem2Dshort: trying to free unused memory",100);
  }
}

/*!
 ************************************************************************
 * \brief
 *    free 3D memory array
 *    which was alocated with get_mem3Dint()
 ************************************************************************
 */
void free_mem3Ddouble(double ***array3D)
{
  if (array3D)
  {
    free_mem2Ddouble(*array3D);
    free (array3D);
  } 
  else
  {
    error ("free_mem3D: trying to free unused memory",100);
  }
}


/*!
 ************************************************************************
 * \brief
 *    Allocate 2D memory array -> LambdaParams array2D[dim0][dim1]
 *    Note that array is shifted towards offset allowing negative values
 *
 * \par Output:
 *    memory size in bytes
 ************************************************************************
 */
int get_mem2Dolm(LambdaParams ***array2D, int dim0, int dim1, int offset)
{
  int i;

  if((*array2D      = (LambdaParams**) malloc(dim0 * sizeof(LambdaParams*))) == NULL)
    no_mem_exit("get_mem2Dolm: array2D");
  if(((*array2D)[0] = (LambdaParams* ) calloc(dim0 * dim1, sizeof(LambdaParams))) == NULL)
    no_mem_exit("get_mem2Dolm: array2D");

  (*array2D)[0] += offset;

  for(i=1 ; i<dim0 ; i++)
    (*array2D)[i] =  (*array2D)[i-1] + dim1  ;

  return dim0 * (sizeof(LambdaParams*) + dim1 * sizeof(LambdaParams));
}


/*!
************************************************************************
* \brief
*    free 2D LambdaParams memory array (with offset)
*    which was allocated with get_mem2Dlm()
************************************************************************
*/
void free_mem2Dolm(LambdaParams **array2D, int offset)
{
  if (array2D)
  {
    array2D[0] -= offset;
    if (array2D[0])
      free (array2D[0]);
    else 
      error ("free_mem2Dolm: trying to free unused memory",100);

    free (array2D);

  } 
  else
  {
    error ("free_mem2Dolm: trying to free unused memory",100);
  }
}

void free_mem2Ddistblk(distblk **array2D)
{
  if (array2D)
  {
    if (*array2D)
      free (*array2D);
    else 
      error ("free_mem2Ddistblk: trying to free unused memory",100);
    free (array2D);
  } 
  else
  {
    error ("free_mem2Ddistblk: trying to free unused memory",100);
  }
}
