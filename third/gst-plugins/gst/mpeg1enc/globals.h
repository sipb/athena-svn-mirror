/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/
/*
************************************************************
globals.h

This is where the global definitions are placed.

************************************************************
*/

#ifndef GLOBAL_DONE
#define GLOBAL_DONE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "config.h"
#include "mem.h"
#include "system.h"
#include "huffman.h"
#include "putbits.h"

#define BLOCKSIZE 64
#define BLOCKWIDTH 8
#define BLOCKHEIGHT 8

#define MAXIMUM_SOURCES 3
#define UMASK 0666  /* Octal */
#define BUFFERSIZE 256 

#define MAXIMUM_FGROUP 256

#define sropen mropen
#define srclose mrclose
#define swopen mwopen
#define swclose mwclose

#define sgetb mgetb
#define sgetv mgetv
#define sputv mputv

#define swtell mwtell
#define srtell mrtell

#define swseek mwseek
#define srseek mrseek

#define READ_IOB 1
#define WRITE_IOB 2

#define P_FORBIDDEN 0
#define P_INTRA 1
#define P_PREDICTED 2
#define P_INTERPOLATED 3
#define P_DCINTRA 4

#define M_DECODER 1

/* Image types */

#define IT_NTSC 0
#define IT_CIF 1
#define IT_QCIF 2

#define HUFFMAN_ESCAPE 0x1bff

/* Can be typedef'ed */
#define IMAGE struct Image_Definition
#define FRAME struct Frame_Definition
#define FSTORE struct FStore_Definition
#define STAT struct Statistics_Definition
#define RATE struct Rate_Control_Definition

#define MUTE 0
#define WHISPER 1
#define TALK 2
#define NOISY 3
#define SCREAM 4

/* Memory locations */

#define L_SQUANT 1
#define L_MQUANT 2
#define L_PTYPE 3
#define L_MTYPE 4
#define L_BD 5
#define L_FDBD 6
#define L_BDBD 7
#define L_IDBD 8
#define L_VAROR 9
#define L_FVAR 10
#define L_BVAR 11
#define L_IVAR 12
#define L_DVAR 13
#define L_RATE 14
#define L_BUFFERSIZE 15
#define L_BUFFERCONTENTS 16
#define L_QDFACT 17
#define L_QOFFS 18

#define ERROR_NONE 0
#define ERROR_BOUNDS 1            /*Input Values out of bounds */
#define ERROR_HUFFMAN_READ 2      /*Huffman Decoder finds bad code */
#define ERROR_HUFFMAN_ENCODE 3    /*Undefined value in encoder */
#define ERROR_MARKER 4            /*Error Found in Marker */
#define ERROR_INIT_FILE 5         /*Cannot initialize files */
#define ERROR_UNRECOVERABLE 6     /*No recovery mode specified */
#define ERROR_PREMATURE_EOF 7     /*End of file unexpected */
#define ERROR_MARKER_STRUCTURE 8  /*Bad Marker Structure */
#define ERROR_WRITE 9             /*Cannot write output */
#define ERROR_READ 10             /*Cannot write input */
#define ERROR_PARAMETER 11        /*System Parameter Error */
#define ERROR_MEMORY 12           /*Memory allocation failure */

/* encoding state */
#define NEW_DATA 1

typedef int iFunc();
typedef void vFunc();

#define GetFlag(value,flag) (((value) & (flag)) ? 1:0)

#define BEGIN(name) static char RoutineName[]= name
#define WHEREAMI() printf("F>%s:R>%s:L>%d: ",\
			  __FILE__,RoutineName,__LINE__)
/* InBounds is used to test whether a value is in or out of bounds. */
#define InBounds(var,lo,hi,str)\
{if (((var) < (lo)) || ((var) > (hi)))\
{WHEREAMI(); printf("%s in %d\n",(str),(var));ErrorValue=ERROR_BOUNDS;}}
#define BoundValue(var,lo,hi,str)\
{if((var)<(lo)){WHEREAMI();printf("Bounding %s to %d\n",str,lo);var=lo;}\
  else if((var)>(hi)){WHEREAMI();printf("Bounding %s to %d\n",str,hi);var=hi;}}

#define MakeStructure(named_st) ((named_st *) malloc(sizeof(named_st)))

IMAGE {
  char *StreamFileName;
  int PartialFrame;
  int MpegMode;
  int Height;
  int Width;
};

FRAME {
  int NumberComponents;
  char ComponentFilePrefix[MAXIMUM_SOURCES][200];
  char ComponentFileSuffix[MAXIMUM_SOURCES][200];
  char ComponentFileName[MAXIMUM_SOURCES][200];
  int PHeight[MAXIMUM_SOURCES];
  int PWidth[MAXIMUM_SOURCES];
  int Height[MAXIMUM_SOURCES];
  int Width[MAXIMUM_SOURCES];
  int hf[MAXIMUM_SOURCES];
  int vf[MAXIMUM_SOURCES];
};

FSTORE {
  int NumberComponents;
  IOBUF *Iob[MAXIMUM_SOURCES];
};

STAT {
  double mean;
  double mse;
  double mrsnr;
  double snr;
  double psnr;
  double entropy;
};

RATE {
  int position;
  int size;
  int baseq;
};


/* MPEG Marker information */
typedef struct sequence {
  /* Video sequence information */
  int HorizontalSize;               /* Horizontal dimensions */
  int VerticalSize;                 /* Vertical dimensions */
  int Aprat;                      /* Aspect ratio */
  int DropFrameFlag;                /* Whether frame will be dropped */
  int Prate;                      /* Picture rate (def 30fps) */
  int Brate;                /* Bit rate */
  int Bsize;                      /* Buffer size */
  int ConstrainedParameterFlag;     /* Default: unconstrained */
  int LoadIntraQuantizerMatrix;     /* Quantization load */
  int LoadNonIntraQuantizerMatrix;
} mpeg1encoder_Sequence;

typedef struct gop {
  /* Group of pictures layer */
  int TimeCode;                     /* SMPTE timestamp */
  int ClosedGOP;                    /* Back pred. needed of GOP */
  int BrokenLink;                   /* Whether editing has occurred. */
} mpeg1encoder_GoP;

typedef struct picture {
  /* Picture layer */
  int TemporalReference;            /* "frame" reference with base of GOP */
  int PType;                  /* Picture type */
  int BufferFullness;               /* Current fullness of buffer */
  int FullPelForward;               /* Forward motion vector on full pel */
  int ForwardIndex;                 /* Decoding table to be used */
  int FullPelBackward;              /* Backward motion vector on full pel */
  int BackwardIndex;                /* Decoding table to be used */
  int PictureExtra;                 /* Flag set if extra info present */
  int PictureExtraInfo;
} mpeg1encoder_Picture;

typedef struct slice {
  int SQuant;          /* Slice quantization */
  int MBperSlice;      /* Number of macroblocks per slice */
                       /* If zero, set automaticallly */
  int SliceExtra;      /* (Last) slice extra flag */
  int SliceExtraInfo;  /* (Last) slice extra information */
} mpeg1encoder_Slice;

typedef struct macroblock {
  int MType;           /* Macroblock type */
  int LastMType;  /* Last encoded MType */
  int SkipMode;   /* Whether we skip coding */
  int EncStartSlice;    /* If encoder has started a slice */
  int EncEndSlice;      /* If encoder has ended a slice */
  int EncPerfectSlice;  /* Set if you want first and  last block */
                               /* of a slice to be defined. Clear if you */
                               /* allow skipped macroblocks between frames */
  int UseTimeCode;/*If 1 forces frame number to be same as time code */

  int MQuant;          /* Macroblock quantization */
  int SVP;             /* Slice vertical position */
  int MVD1H;           /* Forward motion vector */
  int MVD1V;
  int MVD2H;           /* Backward motion vector */
  int MVD2V;
  int LastMVD1H;       /* Interpolative predictors */
  int LastMVD1V;
  int LastMVD2H;
  int LastMVD2V;
  int CBP;          /* Coded block pattern */

  int MBSRead;
  int MBAIncrement;
  int LastMBA;
  int CurrentMBA;
} mpeg1encoder_MacroBlock;

typedef struct vid_stream {
  /* System Definitions */
  int DynamicMVBound;       /* Dynamically calculate motion vector bounds */
  int XING;                 /* If set, then outputs XING compatible file?*/
  int ImageType;      /* Default type is NTSC, can be changed to CIF*/

  int MBWidth;              /* Number macroblocks widexhigh */
  int MBHeight;
  int HPos;                 /* Current macroblock position widexhigh */
  int VPos; 
  int CurrentMBS;           /* Current macroblock slice count */

  int BaseFrame;            /* Base frame for interpolative mode */
  int CurrentFrame;         /* Current frame in encoding */
  int StartFrame;           /* Start frame of encoding */
  int LastFrame;            /* Last frame of encoding */
  int GroupFirstFrame;      /* First frame number of current group */
  int FrameOffset;        /* Offset by TIMECODE */

  int FrameDistance;        /* Distance between interpol. frame and base */
  int FrameInterval;        /* Frame interval between pred/intra frames */
  int FrameGroup;           /* Number of frame intervals per group */
  int FrameIntervalCount;   /* Current frame interval count */

  IMAGE *CImage;              /* Current Image definition structure */
  FRAME *CFrame;              /* Current Frame definition structure */
  FSTORE *CFStore;            /* Current fram in use */
  FSTORE *CFSBase;            /* Base frame for interpolative prediction */
  FSTORE *CFSNext;            /* Next (pred, intra) for interpol. pred. */
  FSTORE *CFSMid;             /* Mid frame for interpolative pred. */
  FSTORE *CFSNew;             /* New frame generated */
  FSTORE *CFSUse;             /* Original frame */
  STAT *CStat;                /* Statistics package */

  int **FMX;                       /* Motion vector arrays for forward */
  int **BMX;                       /* and backward compensation */
  int **FMY;
  int **BMY;

  MEM **FFS;                       /* Memory blocks used for frame stores */

  mpeg1encoder_Sequence seq;
  mpeg1encoder_GoP gop;
  mpeg1encoder_Picture pic;
  mpeg1encoder_Slice slice;
  mpeg1encoder_MacroBlock mb;

  /* Not used, but sometimes defined for P*64 */
  int FrameSkip;            /* Frame skip value */

  /* Stuff for RateControl */
                                       /*Xing Added 9; added to 15 */
  int FileSizeBits;
  int Rate;             /* Rate of the system in bits/second */
  int BufferOffset;     /* Number of bits assumed for initial buffer. */
  int QDFact;
  int QOffs;
  int QUpdateFrequency;
  int QUse;
  int QSum;

  /* Some internal parameters for rate control */
  #define DEFAULT_QUANTIZATION 8
  int InitialQuant;
  int UseQuant;

  /* Parser stuff */
  double *Memory;

  /* Stuff for Motion Compensation */
  int SearchLimit;
  int MVTelescope;
  int MVPrediction;
  int MX;
  int MY;
  int NX;
  int NY;
  int MeVAR[1024];
  int MeVAROR[1024];
  int MeMWOR[1024];
  int MV;
  int MeX[1024];
  int MeY[1024];
  int MeVal[1024];
  int MeN;
  int Mask[64];
  int Nask[64];
  BLOCK nb,rb;
  int MinX,MaxX,BMinX,BMaxX;
  int MinY,MaxY,BMinY,BMaxY;
  int CircleLimit;
  int VAR;
  int VAROR;
  int MWOR;
  unsigned char LocalX[272];
  unsigned char LocalY[272];
  unsigned char LocalXY[289];
  unsigned char *lptr,*aptr,*dptr,*eptr;

  IOBUF *Iob;

  /* Fixed coding parameters */
  int inputbuf[10][64];
  int interbuf[10][64];
  int fmcbuf[10][64];
  int bmcbuf[10][64];
  int imcbuf[10][64];
  int outputbuf[10][64];
  int output[64];
  int LastDC[3];

  /* file io */
  FILE *swout;
  FILE *srin;
  int current_write_byte;
  int current_read_byte;
  int read_position;
  int write_position;

  /* Book-keeping stuff */
  int DCIntraFlag;                 /* Whether we use DC Intra or not*/
  int ErrorValue;                  /* Error value registered */
  int Loud;                     /* Loudness of debug */
  int Oracle;                      /* Oracle consults fed program */

  /* Statistics */
  int NumberNZ;                    /* Number transform nonzero */
  int NumberOvfl;                  /* Number overflows registered */
  int TrailerValue;
  int MotionVectorBits;       /* Number of bits for motion vectors */
  int MacroAttributeBits;     /* Number of bits for macroblocks */
  int CodedBlockBits;         /* Number of bits for coded block */
  int YCoefBits;                   /* Number of bits for Y coeff */
  int UCoefBits;                   /* Number of bits for U coeff */
  int VCoefBits;                   /* Number of bits for V coeff */
  int EOBBits;                /* Number of bits for End-of-block */
  int StuffCount;

  int MaxTypes;
  int MacroTypeFrequency[20];
  int YTypeFrequency[20];
  int UVTypeFrequency[20];

  int TotalBits,LastBits;            /* Total number of bits, last frame bits */

  Huffman_tables huff;
  int NumberBitsCoded;

  /* DCT Stuff */
  /* Functional and macro declarations */

  vFunc *UseDct;
  vFunc *UseIDct;
#define DefaultDct(vs, o ,i) (*((vs)->UseDct))(o,i)
#define DefaultIDct(vs, o ,i) (*((vs)->UseIDct))(o,i)


/* Buffer definitions */

  int BufferSize;       /* 320 kbits */
#define FrameRate(vs) PrateIndex[(vs)->seq.Prate]
#define BufferContents(vs) (mwtell(vs) + (vs)->BufferOffset -\
			  ((((vs)->VPos*(vs)->MBWidth)+(vs)->HPos)\
			   *(vs)->Rate*(vs)->FrameSkip\
			  /((vs)->MBHeight*(vs)->MBWidth*FrameRate(vs))))

  putbits_t pb;
  unsigned char **frame_buffer;
  unsigned long frame_buffer_size;
  int frame_num;
  int inited;
  
} mpeg1encoder_VidStream;

extern const int IntraQuantMType[];
extern const int PredQuantMType[];
extern const int InterQuantMType[];
extern const int DCQuantMType[];
extern const int *QuantPMType[];

extern const int IntraMFMType[];
extern const int PredMFMType[];
extern const int InterMFMType[];
extern const int DCMFMType[];
extern const int *MFPMType[];
	
extern const int IntraMBMType[];
extern const int PredMBMType[];
extern const int InterMBMType[];
extern const int DCMBMType[];
extern const int *MBPMType[];

extern const int IntraCBPMType[];
extern const int PredCBPMType[];
extern const int InterCBPMType[];
extern const int DCCBPMType[];
extern const int *CBPPMType[];
	
extern const int IntraIMType[];
extern const int PredIMType[];
extern const int InterIMType[];
extern const int DCIMType[];
extern const int *IPMType[];

extern int MPEGIntraQ[];
extern int MPEGNonIntraQ[];
#include "prototypes.h"

#endif
