/*!
 ************************************************************************
 * \file params.h
 *
 * \brief
 *    Input parameters related definitions
 *
 * \author
 *
 ************************************************************************
 */

#ifndef _PARAMS_H_
#define _PARAMS_H_

#include "defines.h"
#include "types.h"
#include "vui_params.h"
#include "frame.h"
#include "io_video.h"

//! all input parameters
struct inp_par_enc
{
  int ProfileIDC;                       //!< value of syntax element profile_idc
  int LevelIDC;                         //!< value of syntax element level_idc
  int IntraProfile;                     //!< Enable Intra profiles
  
  int no_frames;                        //!< number of frames to be encoded
  int qp[NUM_SLICE_TYPES];              //!< QP values for all slice types
  int qp2frame;                         //!< frame in display order from which to apply the Change QP offsets
  int qp2off[NUM_SLICE_TYPES];          //!< Change QP offset values for all slice types
  int qpsp;                             //!< QPSP quantization value
  int frame_skip;                       //!< number of frames to skip in input sequence (e.g 2 takes frame 0,3,6,9...)
  int jumpd;                            /*!< number of frames to skip in input sequence including intermediate pictures 
                                             (e.g 2 takes frame 0,3,6,9...) */
  int DisableSubpelME;                  //!< Disable sub-pixel motion estimation
  int search_range;                     /*!< search range - integer pel search and 16x16 blocks.  The search window is
                                             generally around the predicted vector. Max vector is 2xmcrange.  */
  int num_ref_frames;                   //!< number of reference frames to be used
  int P_List0_refs;                     //!< number of reference picture in list 0 in P pictures
  int B_List0_refs;                     //!< number of reference picture in list 0 in B pictures
  int B_List1_refs;                     //!< number of reference picture in list 1 in B pictures
  int Log2MaxFNumMinus4;                //!< value of syntax element log2_max_frame_num
  int Log2MaxPOCLsbMinus4;              //!< value of syntax element log2_max_pic_order_cnt_lsb_minus4

  // Input/output sequence format related variables
  FrameFormat source;                   //!< source related information
  FrameFormat output;                   //!< output related information
  int is_interleaved;
  int src_resize;                       //!< Control if input sequence will be resized (currently only cropping is supported)
  int src_BitDepthRescale;              //!< Control if input sequence bitdepth should be adjusted
  int yuv_format;                       //!< YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4)
  int intra_upd;                        /*!< For error robustness. 0: no special action. 1: One GOB/frame is intra coded
                                             as regular 'update'. 2: One GOB every 2 frames is intra coded etc.
                                             In connection with this intra update, restrictions is put on motion vectors
                                             to prevent errors to propagate from the past                                */

  int slice_mode;                       //!< Indicate what algorithm to use for setting slices
  int slice_argument;                   //!< Argument to the specified slice algorithm
  int UseConstrainedIntraPred;          //!< 0: Inter MB pixels are allowed for intra prediction 1: Not allowed
  int  SetFirstAsLongTerm;              //!< Support for temporal considerations for CB plus encoding
  int  infile_header;                   //!< If input file has a header set this to the length of the header
  int  MultiSourceData;
  VideoDataFile   input_file2;          //!< Input video file2
  VideoDataFile   input_file3;          //!< Input video file3
#if (MVC_EXTENSION_ENABLE)
  int num_of_views;                     //!< number of views to encode (1=1view, 2=2views)
  int MVCInterViewReorder;              //!< Reorder References according to interview pictures
  int MVCFlipViews;                     //!< Reverse the order of the views in the bitstream (view 1 has VOIdx 0 and view 1 has VOIdx 0)
  int MVCInterViewForceB;               //!< Force B slices for enhancement layer
  int View1QPOffset;                    //!< QP offset during rate control for View 1
  int enable_inter_view_flag;           //!< Enables inter_view_flag (allows pictures that are to be used for inter-view only prediction)
#endif

  VideoDataFile   input_file1;          //!< Input video file1
  char outfile       [FILE_NAME_SIZE];  //!< H.264 compressed output bitstream
  char ReconFile     [FILE_NAME_SIZE];  //!< Reconstructed Pictures (view 0 for MVC profile)
  char ReconFile2    [FILE_NAME_SIZE];  //!< Reconstructed Pictures (view 1)

  char TraceFile     [FILE_NAME_SIZE];  //!< Trace Outputs
  char StatsFile     [FILE_NAME_SIZE];  //!< Stats File
  char QmatrixFile   [FILE_NAME_SIZE];  //!< Q matrix cfg file
  int  ProcessInput;                    //!< Filter Input Sequence
  int  EnableOpenGOP;                   //!< support for open gops.
  int  EnableIDRGOP;                    //!< support for IDR closed gops with no shared B coded pictures.
  int  grayscale;                       //!< encode in grayscale (Currently only works for 8 bit, YUV 420)

  int idr_period;                       //!< IDR picture period
  int intra_period;                     //!< intra picture period
  int intra_delay;                      //!< IDR picture delay
  int adaptive_idr_period;
  int adaptive_intra_period;            //!< reinitialize start of intra period

  int start_frame;                      //!< Encode sequence starting from Frame start_frame

  int enable_32_pulldown;

  int GenerateMultiplePPS;
  int GenerateSEIMessage;
  char SEIMessageText[INPUT_TEXT_SIZE];

  int ResendSPS;
  int ResendPPS;

  int SendAUD;                          //!< send Access Unit Delimiter NALU
  int skip_gl_stats; 

  // B pictures
  int NumberBFrames;                    //!< number of B frames that will be used
  int PReplaceBSlice;
  int qpBRSOffset;                      //!< QP for reference B slice coded pictures
  int direct_spatial_mv_pred_flag;      //!< Direct Mode type to be used (0: Temporal, 1: Spatial)
  int directInferenceFlag;              //!< Direct Mode Inference Flag

  int BiPredMotionEstimation;           //!< Use of Bipredictive motion estimation
  int BiPredSearch[4];                  //!< Bipredictive motion estimation for modes 16x16, 16x8, 8x16, and 8x8  
  int BiPredMERefinements;              //!< Max number of Iterations for Bi-predictive motion estimation
  int BiPredMESearchRange;              //!< Search range of Bi-predictive motion estimation
  int BiPredMESubPel;                   //!< Use of subpixel refinement for Bi-predictive motion estimation

  // SP/SI Pictures
  int sp_periodicity;                   //!< The periodicity of SP-pictures
  int sp_switch_period;                 //!< Switch period (in terms of switching SP/SI frames) between bitstream 1 and bitstream 2
  int si_frame_indicator;               //!< Flag indicating whether SI frames should be encoded rather than SP frames (0: not used, 1: used)
  int sp2_frame_indicator;              //!< Flag indicating whether switching SP frames should be encoded rather than SP frames (0: not used, 1: used)
  int sp_output_indicator;              //!< Flag indicating whether coefficients are output to allow future encoding of switchin SP frames (0: not used, 1: used)
  char sp_output_filename[FILE_NAME_SIZE];    //!<Filename where SP coefficients are output
  char sp2_input_filename1[FILE_NAME_SIZE];   //!<Filename of coefficients of the first bitstream when encoding SP frames to switch bitstreams
  char sp2_input_filename2[FILE_NAME_SIZE];   //!<Filenames of coefficients of the second bitstream when encoding SP frames to switch bitstreams

  // Weighted Prediction
  int WeightedPrediction;               //!< Weighted prediction for P frames (0: not used, 1: explicit)
  int WeightedBiprediction;             //!< Weighted prediction for B frames (0: not used, 1: explicit, 2: implicit)
  int WPMethod;                         //!< WP method (0: DC, 1: LMS)
  int WPIterMC;                         //!< Iterative WP method
  int WPMCPrecision;
  int WPMCPrecFullRef;
  int WPMCPrecBSlice;
  int EnhancedBWeightSupport;
  int ChromaWeightSupport;           //!< Weighted prediction support for chroma (0: disabled, 1: enabled)
  int UseWeightedReferenceME;        //!< Use Weighted Reference for ME.
  int RDPictureDecision;             //!< Perform RD optimal decision between various coded versions of same picture
  int RDPSliceBTest;                 //!< Tests B slice replacement for P.
  int RDPSliceITest;                 //!< Tests I slice replacement for P.
  int RDPictureMaxPassISlice;        //!< Max # of coding passes for I-slice
  int RDPictureMaxPassPSlice;        //!< Max # of coding passes for P-slice
  int RDPictureMaxPassBSlice;        //!< Max # of coding passes for B-slice
  int RDPictureDeblocking;           //!< Whether to choose between deblocked and non-deblocked picture
  int RDPictureDirectMode;           //!< Whether to check the other direct mode for B slices
  int RDPictureFrameQPPSlice;        //!< Whether to check additional frame level QP values for P slices
  int RDPictureFrameQPBSlice;        //!< Whether to check additional frame level QP values for B slices

  int SkipIntraInInterSlices;        //!< Skip intra type checking in inter slices if best_mode is skip/direct
  int BRefPictures;                  //!< B coded reference pictures replace P pictures (0: not used, 1: used)
  int HierarchicalCoding;
  int HierarchyLevelQPEnable;
  char ExplicitHierarchyFormat[INPUT_TEXT_SIZE]; //!< Explicit GOP format (HierarchicalCoding==3).
  // explicit sequence information parameters
  int  ExplicitSeqCoding;
  char ExplicitSeqFile[FILE_NAME_SIZE];
  int  LowDelay;                      //!< Apply HierarchicalCoding without delay (i.e., encode in the captured/display order)

  int  ReferenceReorder;              //!< Reordering based on Poc distances
  int  PocMemoryManagement;           //!< Memory management based on Poc distances for hierarchical coding

  int symbol_mode;                   //!< Specifies the mode the symbols are mapped on bits
  int of_mode;                       //!< Specifies the mode of the output file
  int partition_mode;                //!< Specifies the mode of data partitioning

  int InterSearch[2][8];

  int DisableIntra4x4;
  int DisableIntra16x16;
  int FastMDEnable; 
  int FastIntraMD; 
  int FastIntra4x4;
  int FastIntra16x16;
  int FastIntra8x8;
  int FastIntraChroma;

  int DisableIntraInInter;
  int IntraDisableInterOnly;
  int Intra4x4ParDisable;
  int Intra4x4DiagDisable;
  int Intra4x4DirDisable;
  int Intra16x16ParDisable;
  int Intra16x16PlaneDisable;
  int ChromaIntraDisable;

  int EnableIPCM;

  double FrameRate;

  int chroma_qp_index_offset;
  int full_search;

  int rdopt;
  int de;     //!< the algorithm to estimate the distortion in the decoder
  int I16rdo; 
  int subMBCodingState;
  int Distortion[TOTAL_DIST_TYPES];
  double VisualResWavPSNR;
  int SSIMOverlapSize;
  int DistortionYUVtoRGB;
  int CtxAdptLagrangeMult;    //!< context adaptive lagrangian multiplier
  int FastCrIntraDecision;
  int disthres;
  int nobskip;
  int BiasSkipRDO;
  int ForceTrueRateRDO;

#ifdef _LEAKYBUCKET_
  int  NumberLeakyBuckets;
  char LeakyBucketRateFile[FILE_NAME_SIZE];
  char LeakyBucketParamFile[FILE_NAME_SIZE];
#endif

  int PicInterlace;           //!< picture adaptive frame/field
  int MbInterlace;            //!< macroblock adaptive frame/field
  int IntraBottom;            //!< Force Intra Bottom at GOP periods.

  // Error resilient RDO parameters
  double LossRateA;              //!< assumed loss probablility of partition A (or full slice), in per cent, used for loss-aware R/D optimization
  double LossRateB;              //!< assumed loss probablility of partition B, in per cent, used for loss-aware R/D
  double LossRateC;              //!< assumed loss probablility of partition C, in per cent, used for loss-aware R/D
  int FirstFrameCorrect;      //!< the first frame is encoded under the assumption that it is always correctly received.
  int NoOfDecoders;
  int ErrorConcealment;       //!< Error concealment method used for loss-aware RDO (0: Copy Concealment)
  int RestrictRef;
  int NumFramesInELSubSeq;

  int RandomIntraMBRefresh;     //!< Number of pseudo-random intra-MBs per picture

  // Chroma interpolation and buffering
  int ChromaMCBuffer;
  Boolean ChromaMEEnable;
  int ChromaMEWeight;
  int MEErrorMetric[3];
  int ModeDecisionMetric;
  int SkipDeBlockNonRef;
  
  //  Deblocking Filter parameters
  int DFSendParameters;
  int DFDisableIdc[2][NUM_SLICE_TYPES];
  int DFAlpha     [2][NUM_SLICE_TYPES];
  int DFBeta      [2][NUM_SLICE_TYPES];

  int SparePictureOption;
  int SPDetectionThreshold;
  int SPPercentageThreshold;

  // FMO
  char SliceGroupConfigFileName[FILE_NAME_SIZE];    //!< Filename for config info fot type 0, 2, 6
  int num_slice_groups_minus1;           //!< "FmoNumSliceGroups" in encoder.cfg, same as FmoNumSliceGroups, which should be erased later
  int slice_group_map_type;

  unsigned int *top_left;                         //!< top_left and bottom_right store values indicating foregrounds
  unsigned int *bottom_right;
  byte *slice_group_id;                   //!< slice_group_id is for slice group type being 6
  int *run_length_minus1;                //!< run_length_minus1 is for slice group type being 0

  int slice_group_change_direction_flag;
  int slice_group_change_rate_minus1;
  int slice_group_change_cycle;

  int redundant_pic_flag;   //! encoding of redundant pictures
  int pic_order_cnt_type;   //! POC type

  int context_init_method;
  int model_number;
  int Transform8x8Mode;
  int ReportFrameStats;
  int DisplayEncParams;
  int Verbose;

  //! Rate Control parameters
  int RCEnable;
  int bit_rate;
  int SeinitialQP;
  unsigned int basicunit;
  int channel_type;
  int RCUpdateMode;
  double RCIoverPRatio;
  double RCBoverPRatio;
  double RCISliceBitRatio;
  double RCBSliceBitRatio[RC_MAX_TEMPORAL_LEVELS];
  int    RCMinQP[NUM_SLICE_TYPES];
  int    RCMaxQP[NUM_SLICE_TYPES];
  int    RCMaxQPChange;

  // Motion Estimation related parameters
  int    UseMVLimits;
  int    SetMVXLimit;
  int    SetMVYLimit;

  // Search Algorithm
  SearchType SearchMode;
  
  // UMHEX related parameters
  int UMHexDSR;
  int UMHexScale;

  // EPZS related parameters
  int EPZSPattern;
  int EPZSDual;
  int EPZSFixed;
  int EPZSTemporal;
  int EPZSSpatialMem;
  int EPZSBlockType;
  int EPZSMinThresScale;
  int EPZSMaxThresScale;
  int EPZSMedThresScale;
  int EPZSSubPelGrid;
  int EPZSSubPelME;
  int EPZSSubPelMEBiPred;
  int EPZSSubPelThresScale;

  // Lambda Params
  int UseExplicitLambdaParams;
  int UpdateLambdaChromaME;
  double LambdaWeight[6];
  double FixedLambda[6];

  char QOffsetMatrixFile[FILE_NAME_SIZE];        //!< Quantization Offset matrix cfg file
  int  OffsetMatrixPresentFlag;                  //!< Enable Explicit Quantization Offset Matrices

  int AdaptiveRounding;                          //!< Adaptive Rounding parameter based on JVT-N011
  int AdaptRoundingFixed;                        //!< Global rounding for all qps
  int AdaptRndPeriod;                            //!< Set period for adaptive rounding of JVT-N011 in MBs
  int AdaptRndChroma;
  int AdaptRndWFactor  [2][NUM_SLICE_TYPES];     //!< Weighting factors for luma component based on reference indicator and slice type
  int AdaptRndCrWFactor[2][NUM_SLICE_TYPES];     //!< Weighting factors for chroma components based on reference indicator and slice type

//////////////////////////////////////////////////////////////////////////
  // Fidelity Range Extensions
  int ScalingMatrixPresentFlag;
  int ScalingListPresentFlag[12];

  int cb_qp_index_offset;
  int cr_qp_index_offset;
  // Lossless Coding
  int LosslessCoding;

  // Fast Mode Decision
  int EarlySkipEnable;
  int SelectiveIntraEnable;
  int DisposableP;
  int DispPQPOffset;

  //Redundant picture
  int NumRedundantHierarchy;   //!< number of entries to allocate redundant pictures in a GOP
  int PrimaryGOPLength;        //!< GOP length of primary pictures
  int NumRefPrimary;           //!< number of reference frames for primary picture

  // tone mapping SEI message
  int ToneMappingSEIPresentFlag;
  char ToneMappingFile[FILE_NAME_SIZE];    //!< ToneMapping SEI message cfg file

  // prediction structure
  int PreferDispOrder;       //!< Prefer display order when building the prediction structure as opposed to coding order
  int PreferPowerOfTwo;      //!< Prefer prediction structures that have lengths expressed as powers of two
  int FrmStructBufferLength; //!< Number of frames that is populated every time populate_frm_struct is called

  int separate_colour_plane_flag;
  double WeightY;
  double WeightCb;
  double WeightCr;
  int UseRDOQuant;
  int RDOQ_DC;
  int RDOQ_CR;
  int RDOQ_DC_CR; 
  int RDOQ_QP_Num;
  int RDOQ_CP_Mode;
  int RDOQ_CP_MV;
  int RDOQ_Fast;

  int EnableVUISupport;
  // VUI parameters
  VUIParameters VUI;
  // end of VUI parameters

  int  MinIDRDistance;
  int stdRange;                         //!< 1 - standard range, 0 - full range
  int videoCode;                        //!< 1 - 709, 3 - 601:  See VideoCode in io_tiff.
};

#endif

