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
mpeg.c

This is the file that has the "main" routine and all of the associated
high-level routines.  Some of these routines are kludged from the
jpeg.c files and have somewhat more extensibility towards multiple
image components and sampling rates than required.

This file was written before the established structure given in the
MPEG coded bitstream syntax.  Much of the inherent structure of the
finalized syntax is not exploited.

************************************************************
*/

/*LABEL mpeg.c */

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "stream.h"
#include "qtables.h"

#define TEMPORAL_MODULO 1024

/*PUBLIC*/

static void mpeg1encoder_init(mpeg1encoder_VidStream *vid_stream);
static void set_defaults(mpeg1encoder_VidStream *vid_stream); 
static void MpegEncodeSequenceFirst(mpeg1encoder_VidStream *vid_stream);
static void MpegEncodeSequenceNext(mpeg1encoder_VidStream *vid_stream);
static void MpegEncodeSequenceLast(mpeg1encoder_VidStream *vid_stream);
static void MpegEncodeDSequence(mpeg1encoder_VidStream *vid_stream);
static void ExecuteQuantization(mpeg1encoder_VidStream *vid_stream, int *Qptr);
static void CleanStatistics(mpeg1encoder_VidStream *vid_stream);
static void CollectStatistics(mpeg1encoder_VidStream *vid_stream);

static void MpegEncodeSlice(mpeg1encoder_VidStream *vid_stream, int Count);
static void MpegEncodeMDU(mpeg1encoder_VidStream *vid_stream);
static void MpegFindMType(mpeg1encoder_VidStream *vid_stream);
static void MpegCompressMType(mpeg1encoder_VidStream *vid_stream);
static void MpegWriteMType(mpeg1encoder_VidStream *vid_stream);

static void MpegEncodeDFrame(mpeg1encoder_VidStream *vid_stream);
static void MpegDecodeDFrame(mpeg1encoder_VidStream *vid_stream);
static void MpegDecodeSaveMDU(mpeg1encoder_VidStream *vid_stream);
static void MpegDecompressMDU(mpeg1encoder_VidStream *vid_stream);

/*PRIVATE*/

/* The following component indices point to given blocks in the CBP */

const int BlockJ[] = {0,0,0,0,1,2};    /* Index positions for which color component*/
const int BlockV[] = {0,0,1,1,0,0};    /* Vertical and horizontal indices */
const int BlockH[] = {0,1,0,1,0,0};

const char *DefaultSuffix[]={".Y",".U",".V"};  /* Suffixes for generic files */

const int PrateIndex[] = {0,24,24,25,30,30,50,60,60,30,30,30,30,30,30,30};

#define SwapFS(fs1,fs2) {FSTORE *ftemp;ftemp = fs1;fs1 = fs2;fs2 = ftemp;}

/* Type Definitions */

/* We define the following variables by layers, to avoid compatibility
problems with compilers unable to do automatic aggregate
initialization.

The MType arrays are indexed on macroblock type.
The PMType arrays are indexed on picture type and macroblock type. */

/* Quantization used */
const int IntraQuantMType[] = {0,1};
const int PredQuantMType[] = {0,0,0,0,1,1,1};
const int InterQuantMType[] = {0,0,0,0,0,0,0,1,1,1,1};
const int DCQuantMType[] = {0};
const int *QuantPMType[] = {
  (int *) 0,
  IntraQuantMType,
  PredQuantMType,
  InterQuantMType,
  DCQuantMType};

/* Motion forward vector used */
const int IntraMFMType[] = {0,0};
const int PredMFMType[] = {1,0,1,0,1,0,0};
const int InterMFMType[] = {1,1,0,0,1,1,0,1,1,0,0};
const int DCMFMType[] = {0};
const int *MFPMType[] = {
  (int *) 0,
  IntraMFMType,
  PredMFMType,
  InterMFMType,
  DCMFMType};

/* Motion backward vector used */
const int IntraMBMType[] = {0,0};
const int PredMBMType[] = {0,0,0,0,0,0,0};
const int InterMBMType[] = {1,1,1,1,0,0,0,1,0,1,0};
const int DCMBMType[] = {0};
const int *MBPMType[] = {
  (int *) 0,
  IntraMBMType,
  PredMBMType,
  InterMBMType,
  DCMBMType};

/* CBP used in coding */
const int IntraCBPMType[] = {0,0};
const int PredCBPMType[] = {1,1,0,0,1,1,0};
const int InterCBPMType[] = {0,1,0,1,0,1,0,1,1,1,0};
const int DCCBPMType[] = {0};
const int *CBPPMType[] = {
  (int *) 0,
  IntraCBPMType,
  PredCBPMType,
  InterCBPMType,
  DCCBPMType};

/* Intra coded macroblock */
const int IntraIMType[] = {1,1};
const int PredIMType[] = {0,0,0,1,0,0,1};
const int InterIMType[] = {0,0,0,0,0,0,1,0,0,0,1};
const int DCIMType[] = {1};
const int *IPMType[] = {
  (int *) 0,
  IntraIMType,
  PredIMType,
  InterIMType,
  DCIMType};

/*START*/

mpeg1encoder_VidStream *mpeg1encoder_new_encoder() {
  mpeg1encoder_VidStream *new;

  new = malloc(sizeof(mpeg1encoder_VidStream));

  set_defaults(new);
  return new;
}

int mpeg1encoder_new_picture(mpeg1encoder_VidStream *vid_stream, unsigned char *data, unsigned long size, int state)
{
  FILE *out;
  char filename[256];
  if (!vid_stream->inited) mpeg1encoder_init(vid_stream);


  sprintf(filename, "/opt2/test/out.%d.Y", vid_stream->frame_num);
  out = fopen(filename,"w");
  fwrite(data,sizeof(unsigned char),vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize,out);
  fclose(out);
  sprintf(filename, "/opt2/test/out.%d.V", vid_stream->frame_num);
  out = fopen(filename,"w");
  fwrite(data+vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize,
                   sizeof(unsigned char),(vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize)>>2, out);
  fclose(out);
  sprintf(filename, "/opt2/test/out.%d.U", vid_stream->frame_num);
  out = fopen(filename,"w");
  fwrite(data+vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize+
                   ((vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize)>>2),
                   sizeof(unsigned char),(vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize)>>2, out);
  fclose(out);

  if (vid_stream->frame_num == 0) {
    /* convertRGBtoYUV(vid_stream, inbuf, vid_stream->frame_buffer[0]); */
    memcpy(vid_stream->frame_buffer[0], data, MIN(vid_stream->frame_buffer_size, size));
    putbits_new_empty_buffer(&vid_stream->pb, 1000000);
    printf("encoding %d\n", vid_stream->frame_num);
    MpegEncodeSequenceFirst(vid_stream);
    vid_stream->frame_num++;
    return NEW_DATA;
  }
  else {
    /* convertRGBtoYUV(vid_stream, inbuf, vid_stream->frame_buffer[(vid_stream->framenum-1)%vid_stream->M]); */
    printf("%d %d %d\n", vid_stream->frame_num, vid_stream->FrameInterval, ((vid_stream->frame_num-1)%vid_stream->FrameInterval)+1);
    memcpy(vid_stream->frame_buffer[((vid_stream->frame_num-1)%vid_stream->FrameInterval)+1], data, MIN(vid_stream->frame_buffer_size, size));

    if (vid_stream->frame_num%vid_stream->FrameInterval == 0) {
      putbits_new_empty_buffer(&vid_stream->pb, 1000000);

      MpegEncodeSequenceNext(vid_stream);
      memcpy(vid_stream->frame_buffer[0], vid_stream->frame_buffer[vid_stream->FrameInterval], vid_stream->frame_buffer_size);
      vid_stream->frame_num++;
      return NEW_DATA;
    }
    vid_stream->frame_num++;
  }
  return 0;
}

static void mpeg1encoder_init(mpeg1encoder_VidStream *vid_stream) 
{
  int i;	

  MakeImage(vid_stream);   /* Initialize storage */
  MakeFrame(vid_stream);
  inithuff(vid_stream);    /* Put Huffman tables on */

  vid_stream->slice.MBperSlice=1620;
  strcpy(vid_stream->CFrame->ComponentFilePrefix[0],"/opt2/test/out.");
  vid_stream->CImage->StreamFileName = "/opt2/test/test.mpg";
  vid_stream->BaseFrame = 0;
  vid_stream->frame_num = 0;
  vid_stream->StartFrame=vid_stream->BaseFrame;
  vid_stream->LastFrame = 2800;
  CreateFrameSizes(vid_stream);

  vid_stream->frame_buffer = (unsigned char **)malloc((vid_stream->FrameInterval+1)*sizeof(unsigned char *));

  vid_stream->frame_buffer_size = vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize+
                                 (vid_stream->seq.HorizontalSize*vid_stream->seq.VerticalSize>>1);

  for (i=0; i<vid_stream->FrameInterval+1; i++) {
    vid_stream->frame_buffer[i] = (unsigned char *)malloc(vid_stream->frame_buffer_size);
  }
  
  vid_stream->inited = 1;
}

static void set_defaults(mpeg1encoder_VidStream *vid_stream) {

  vid_stream->seq.HorizontalSize=0;               /* Horizontal dimensions */
  vid_stream->seq.VerticalSize=0;                 /* Vertical dimensions */
  vid_stream->seq.Aprat = 1;                      /* Aspect ratio */
  vid_stream->seq.DropFrameFlag=0;                /* Whether frame will be dropped */
  vid_stream->seq.Prate = 3;                      /* Picture rate (def 25fps) */
  vid_stream->seq.Brate = 0x3ffff;                /* Bit rate */
  vid_stream->seq.Bsize = 0;                      /* Buffer size */
  vid_stream->seq.ConstrainedParameterFlag=0;     /* Default: unconstrained */
  vid_stream->seq.LoadIntraQuantizerMatrix=0;     /* Quantization load */
  vid_stream->seq.LoadNonIntraQuantizerMatrix=0;
  vid_stream->gop.TimeCode= -1;                     /* SMPTE timestamp */
  vid_stream->gop.ClosedGOP=0;                    /* Back pred. needed of GOP */
  vid_stream->gop.BrokenLink=0;                   /* Whether editing has occurred. */

  vid_stream->pic.TemporalReference=0;            /* "frame" reference with base of GOP */
  vid_stream->pic.PType=P_INTRA;                  /* Picture type */
  vid_stream->pic.BufferFullness=0;               /* Current fullness of buffer */
  vid_stream->pic.FullPelForward=0;               /* Forward motion vector on full pel */
  vid_stream->pic.ForwardIndex=0;                 /* Decoding table to be used */
  vid_stream->pic.FullPelBackward=0;              /* Backward motion vector on full pel */
  vid_stream->pic.BackwardIndex=0;                /* Decoding table to be used */
  vid_stream->pic.PictureExtra=0;                 /* Flag set if extra info present */
  vid_stream->pic.PictureExtraInfo=0;
  vid_stream->slice.SQuant=1;          /* Slice quantization */
  vid_stream->slice.MBperSlice=0;      /* Number of macroblocks per slice */
                       /* If zero, set automaticallly */
  vid_stream->slice.SliceExtra=0;      /* (Last) slice extra flag */
  vid_stream->slice.SliceExtraInfo=0;  /* (Last) slice extra information */

  vid_stream->mb.MType=0;           /* Macroblock type */
  vid_stream->mb.EncPerfectSlice=1;  /* Set if you want first and  last block */
                               /* of a slice to be defined. Clear if you */
                               /* allow skipped macroblocks between frames */
  vid_stream->mb.UseTimeCode=0;/*If 1 forces frame number to be same as time code */

  vid_stream->mb.MQuant=1;          /* Macroblock quantization */
  vid_stream->mb.SVP=0;             /* Slice vertical position */
  vid_stream->mb.MVD1H=0;           /* Forward motion vector */
  vid_stream->mb.MVD1V=0;
  vid_stream->mb.MVD2H=0;           /* Backward motion vector */
  vid_stream->mb.MVD2V=0;
  vid_stream->mb.LastMVD1H=0;       /* Interpolative predictors */
  vid_stream->mb.LastMVD1V=0;
  vid_stream->mb.LastMVD2H=0;
  vid_stream->mb.LastMVD2V=0;
  vid_stream->mb.CBP=0x3f;          /* Coded block pattern */

  vid_stream->mb.MBSRead=0;
  vid_stream->mb.MBAIncrement=0;
  vid_stream->mb.LastMBA=0;
  vid_stream->mb.CurrentMBA=0;

  vid_stream->DynamicMVBound=1;       /* Dynamically calculate motion vector bounds */
  vid_stream->XING=0;                 /* If set, then outputs XING compatible file?*/
  vid_stream->ImageType=IT_NTSC;      /* Default type is NTSC, can be changed to CIF*/

  vid_stream->MBWidth=0;              /* Number macroblocks widexhigh */
  vid_stream->MBHeight=0;
  vid_stream->HPos=0;                 /* Current macroblock position widexhigh */
  vid_stream->VPos=0; 
  vid_stream->CurrentMBS=0;           /* Current macroblock slice count */

  vid_stream->BaseFrame=0;            /* Base frame for interpolative mode */
  vid_stream->CurrentFrame=0;         /* Current frame in encoding */
  vid_stream->StartFrame=0;           /* Start frame of encoding */
  vid_stream->LastFrame=0;            /* Last frame of encoding */
  vid_stream->GroupFirstFrame=0;      /* First frame number of current group */
  vid_stream->FrameOffset= -1;        /* Offset by TIMECODE */

  vid_stream->FrameDistance=1;        /* Distance between interpol. frame and base */
  vid_stream->FrameInterval=3;        /* Frame interval between pred/intra frames */
  vid_stream->FrameGroup=5;           /* Number of frame intervals per group */
  vid_stream->FrameIntervalCount=0;   /* Current frame interval count */

  vid_stream->CImage=NULL;              /* Current Image definition structure */
  vid_stream->CFrame=NULL;              /* Current Frame definition structure */
  vid_stream->CFStore=NULL;            /* Current fram in use */
  vid_stream->CFSBase=NULL;            /* Base frame for interpolative prediction */
  vid_stream->CFSNext=NULL;            /* Next (pred, intra) for interpol. pred. */
  vid_stream->CFSMid=NULL;             /* Mid frame for interpolative pred. */
  vid_stream->CFSNew=NULL;             /* New frame generated */
  vid_stream->CFSUse=NULL;             /* Original frame */
  vid_stream->CStat=NULL;                /* Statistics package */

  /* Not used, but sometimes defined for P*64 */
  vid_stream->FrameSkip=1;            /* Frame skip value */

  /* Stuff for RateControl */
  vid_stream->FileSizeBits=0;
  vid_stream->Rate=1150000;             /* Rate of the system in bits/second */
  vid_stream->BufferOffset=0;     /* Number of bits assumed for initial buffer. */
  vid_stream->QDFact=1;
  vid_stream->QOffs=1;
  vid_stream->QUpdateFrequency=11;
  vid_stream->QUse=0;
  vid_stream->QSum=0;

  /* Some internal parameters for rate control */
  vid_stream->InitialQuant=0;

  vid_stream->SearchLimit=128;                 /* Whether we use DC Intra or not*/
  vid_stream->MVPrediction = 0;  /* Sets some complicated prediction */
  vid_stream->MVTelescope=1;   
  vid_stream->MeN=0;


  /* Book-keeping stuff */
  vid_stream->DCIntraFlag=0;                 /* Whether we use DC Intra or not*/
  vid_stream->ErrorValue=0;                  /* Error value registered */
  vid_stream->Loud=2;                     /* Loudness of debug */
  vid_stream->Oracle=0;                      /* Oracle consults fed program */

  /* Statistics */

  vid_stream->NumberNZ=0;                    /* Number transform nonzero */
  vid_stream->NumberOvfl=0;                  /* Number overflows registered */
  vid_stream->TrailerValue=0;    
  vid_stream->YCoefBits=0;                   /* Number of bits for Y coeff */
  vid_stream->UCoefBits=0;                   /* Number of bits for U coeff */
  vid_stream->VCoefBits=0;                   /* Number of bits for V coeff */

  vid_stream->UseDct = ReferenceDct;
  vid_stream->UseIDct = ReferenceIDct;


/* Buffer definitions */
  vid_stream->BufferSize = 20*(16*1024);       /* 320 kbits */
  vid_stream->inited = 0;
}


/*BFUNC

main() is the first routine called by program activation. It parses
the input command line and sets parameters accordingly.

EFUNC*/

#if 0
int main(argc,argv)
     int argc;
     char **argv;
{
  BEGIN("main");
  int i,p,s;

  MakeImage();   /* Initialize storage */
  MakeFrame();
  inithuff();    /* Put Huffman tables on */
  if (argc==1)
    {
      Help();
      exit(-1);
    }
  for(s=0,p=0,i=1;i<argc;i++)
    {
      if (!strcmp("-NTSC",argv[i]))
	ImageType = IT_NTSC;
      else if (!strcmp("-CIF",argv[i]))
	ImageType = IT_CIF;
      else if (!strcmp("-QCIF",argv[i]))
	ImageType = IT_QCIF;
      else if (!strcmp("-PF",argv[i]))
	CImage->PartialFrame=1;
      else if (!strcmp("-NPS",argv[i]))
	EncPerfectSlice=0;
      else if (!strcmp("-MBPS",argv[i]))
	MBperSlice=atoi(argv[++i]);
      else if (!strcmp("-UTC",argv[i]))
	UseTimeCode=1;
      else if (!strcmp("-XING",argv[i]))
	{
	  XING=1;
	  HorizontalSize=160;
	  VerticalSize=120;
	}
      else if (!strcmp("-DMVB",argv[i]))
	DynamicMVBound=1;
      else if (!strcmp("-MVNT",argv[i]))
	MVTelescope=0;
      else if (*(argv[i]) == '-')
 	{
	  switch(*(++argv[i]))
 	    {
	    case '4':
	      DCIntraFlag=1;
	      break;
	    case 'a':
	      BaseFrame = atoi(argv[++i]);
	      StartFrame=BaseFrame;
	      break;
	    case 'b':
	      LastFrame = atoi(argv[++i]);
	      break;
	    case 'c':
	      MVPrediction=1;
	      break;
 	    case 'd': 
	      CImage->MpegMode |= M_DECODER;
 	      break; 
	    case 'f':
	      FrameInterval = atoi(argv[++i]);
	      break;
	    case 'g':
	      FrameGroup = atoi(argv[++i]);
	      break;
	    case 'h':
	      HorizontalSize = atoi(argv[++i]);
	      break;
	    case 'i':
	      SearchLimit = atoi(argv[++i]);
	      /* BoundValue(SearchLimit,1,15,"SearchLimit"); */
	      if (SearchLimit<1) SearchLimit=1;
	      DynamicMVBound=1;  /* Calculate the bounds appropriately */
	      break;
/* NOT USED
	    case 'k':
	      FrameSkip = atoi(argv[++i]);
	      break;
*/
	    case 'l':
	      Loud = atoi(argv[++i]);
	      break;
	    case 'o':
	      Oracle=1;
	      break;
	    case 'p':
	      Prate = atoi(argv[++i]);
	      break;
	    case 'q':
	      InitialQuant=atoi(argv[++i]);
	      BoundValue(InitialQuant,2,31,"InitialQuant");
	      break;
	    case 'r':
	      Rate = (atoi(argv[++i]));
	      break;
	    case 's':
	      CImage->StreamFileName = argv[++i];
	      break;
	    case 'v':
	      VerticalSize = atoi(argv[++i]);
	      break;
	    case 'x':
	      vid_stream->FileSizeBits = (atoi(argv[++i]));
	      break;
	    case 'y':
	      UseDct = ReferenceDct;
	      UseIDct = ReferenceIDct;
	      break;
	    case 'z':
	      strcpy(CFrame->ComponentFileSuffix[s++],argv[++i]);
	      break;
	    default:
	      WHEREAMI();
	      printf("Illegal Option %c\n",*argv[i]);
	      exit(ERROR_BOUNDS);
	      break;
	    }
	}
      else
	{
	  strcpy(CFrame->ComponentFilePrefix[p++],argv[i]);
	}
    }
  if (!CImage->StreamFileName)
    {
      if (!(CImage->StreamFileName =
	    (char *) calloc(strlen(CFrame->ComponentFilePrefix[0])+6,
			    sizeof(char))))
	{
	  WHEREAMI();
	  printf("Cannot allocate string for StreamFileName.\n");
	  exit(ERROR_MEMORY);
	}
      sprintf(CImage->StreamFileName,
	      "%s.mpg",CFrame->ComponentFilePrefix[0]);
    }

  if (vid_stream->XING)
    {
      CImage->PartialFrame=1;
      Prate = 9;
      ConstrainedParameterFlag=1;
      vid_stream->slice.MBperSlice= -1;      /* Ensure automatic setting of MBperslice */
      FrameInterval=1;     /* in following encoder... */
      FrameGroup=1;
    }

  if (Oracle)
    {
      initparser();
      parser();
    }
  if(!(GetFlag(CImage->MpegMode,M_DECODER)))
    {
      if ((!vid_stream->seq.HorizontalSize)||(!vid_stream->seq.VerticalSize)) /* Unspecified hor, ver */
	SetCCITT();
      CreateFrameSizes(vid_stream);
      if (vid_stream->BaseFrame>vid_stream->LastFrame)
	{
	  WHEREAMI();
	  printf("Need positive number of frames.\n");
	  exit(ERROR_BOUNDS);
	}
      MpegEncodeSequence(vid_stream);
    }
  else
    {
      MpegDecodeSequence();
    }
  exit(vid_stream->ErrorValue);
}
#endif

/*BFUNC


MpegEncodeSequence() encodes the sequence defined by the CImage and
CFrame structures, startframe and lastframe.

EFUNC*/

static void MpegEncodeSequenceFirst(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MpegEncodeSequence"); */

  if (vid_stream->DCIntraFlag)     /* DC Intraframes are handled by a faster engine */
    {
      MpegEncodeDSequence(vid_stream);
      return;
    }
  MakeFGroup(vid_stream);        /* Make our group structure */
  MakeStat(vid_stream);          /* Make the statistics structure */
  MakeFS(vid_stream,READ_IOB);    /* Make our frame stores */
  vid_stream->CFSUse=vid_stream->CFStore;
  MakeFS(vid_stream,WRITE_IOB);
  vid_stream->CFSBase=vid_stream->CFStore;
  MakeFS(vid_stream,WRITE_IOB);
  vid_stream->CFSNext=vid_stream->CFStore;
  MakeFS(vid_stream,WRITE_IOB);
  vid_stream->CFSMid=vid_stream->CFStore;
  swopen(vid_stream,vid_stream->CImage->StreamFileName);  /* Open file */
  if (vid_stream->Loud > MUTE)
    {
      PrintImage(vid_stream);
      PrintFrame(vid_stream);
    }
  if (vid_stream->FileSizeBits)  /* Rate is determined by bits/second. */
    vid_stream->Rate=(vid_stream->FileSizeBits*FrameRate(vid_stream))/(vid_stream->FrameSkip*(vid_stream->LastFrame-vid_stream->StartFrame+1));
  if (vid_stream->Rate)
    {
      if (vid_stream->Rate/4 > vid_stream->BufferSize)  vid_stream->BufferSize = vid_stream->Rate/4;
      vid_stream->QDFact = (vid_stream->Rate/230);
      vid_stream->QOffs = 1;
      if (!vid_stream->InitialQuant)
	{
	  vid_stream->InitialQuant = 10000000/vid_stream->Rate;
	  if (vid_stream->InitialQuant>31)
	    vid_stream->InitialQuant=31;
	  else if (vid_stream->InitialQuant<2)
	    vid_stream->InitialQuant=2;

	  printf("Rate: %d  Buffersize: %d  QDFact: %d  QOffs: %d\n",
		 vid_stream->Rate,vid_stream->BufferSize,vid_stream->QDFact,vid_stream->QOffs);
	  printf("Starting Quantization: %d\n",vid_stream->InitialQuant);
	}
    }
  else if (!vid_stream->InitialQuant)
    vid_stream->InitialQuant=DEFAULT_QUANTIZATION;
  vid_stream->UseQuant=vid_stream->slice.SQuant=vid_stream->mb.MQuant=vid_stream->InitialQuant;
  vid_stream->BufferOffset=0;
  vid_stream->TotalBits=0;
  vid_stream->NumberOvfl=0;
  printf("START>SEQUENCE\n");

/*
  HorizontalSize = CImage->Width; 
  VerticalSize = CImage->Height;
*/

  WriteVSHeader(vid_stream);    /* Write out the header file */
  vid_stream->GroupFirstFrame=vid_stream->CurrentFrame=vid_stream->BaseFrame;
  vid_stream->gop.TimeCode = Integer2TimeCode(vid_stream, vid_stream->GroupFirstFrame);
  if (vid_stream->XING) vid_stream->gop.ClosedGOP=0; 
  else vid_stream->gop.ClosedGOP=1;        /* Closed GOP is 1 at start of encoding. */
  WriteGOPHeader(vid_stream);   /* Set up first frame */
  vid_stream->gop.ClosedGOP=0;        /* Reset closed GOP */
  vid_stream->pic.PType=P_INTRA;
  MakeFileNames(vid_stream);
  /* VerifyFiles(vid_stream); */
  vid_stream->CFStore=vid_stream->CFSUse; SwapFS(vid_stream->CFSBase,vid_stream->CFSNext); vid_stream->CFSNew=vid_stream->CFSNext;
  ReadFS(vid_stream);
  vid_stream->pic.TemporalReference = 0;
  MpegEncodeIPBDFrame(vid_stream);
  vid_stream->FrameIntervalCount= 0;
}

static void MpegEncodeSequenceNext(mpeg1encoder_VidStream *vid_stream) 
{
  int i;

  /* for(vid_stream->FrameIntervalCount=0;vid_stream->BaseFrame<vid_stream->LastFrame;vid_stream->FrameIntervalCount++) */
  /*  {  */
      if (vid_stream->BaseFrame+vid_stream->FrameInterval > vid_stream->LastFrame)
	vid_stream->FrameInterval = vid_stream->LastFrame-vid_stream->BaseFrame;
      LoadFGroup(vid_stream, vid_stream->BaseFrame);                  /* We do motion compensation */
      InterpolativeBME(vid_stream);
      vid_stream->CurrentFrame=vid_stream->BaseFrame+vid_stream->FrameInterval;   /* Load in next base */
      if (!((vid_stream->FrameIntervalCount+1)%vid_stream->FrameGroup))
	{                              /* Load an Intra Frame */
	  vid_stream->GroupFirstFrame=vid_stream->BaseFrame+1;  /* Base of group */
	  vid_stream->pic.TemporalReference = (vid_stream->CurrentFrame-vid_stream->GroupFirstFrame)%TEMPORAL_MODULO;
	  vid_stream->gop.TimeCode = Integer2TimeCode(vid_stream, vid_stream->GroupFirstFrame);
	  if (!vid_stream->XING)
	    WriteGOPHeader(vid_stream);        /* Write off a group of pictures */
	  vid_stream->pic.PType=P_INTRA;
	  MakeFileNames(vid_stream);
	  /* VerifyFiles(vid_stream); */
	  vid_stream->CFStore=vid_stream->CFSUse; SwapFS(vid_stream->CFSBase,vid_stream->CFSNext); vid_stream->CFSNew=vid_stream->CFSNext;
	  ReadFS(vid_stream);
	  MpegEncodeIPBDFrame(vid_stream);
	}
      else
	{	                /* Load Next Predicted Frame */
	  vid_stream->pic.TemporalReference = (vid_stream->CurrentFrame-vid_stream->GroupFirstFrame)%TEMPORAL_MODULO;
	  vid_stream->FrameDistance = vid_stream->FrameInterval;
	  vid_stream->pic.PType=P_PREDICTED;
	  MakeFileNames(vid_stream);
	  /* VerifyFiles(vid_stream); */
	  vid_stream->CFStore=vid_stream->CFSUse; SwapFS(vid_stream->CFSBase,vid_stream->CFSNext); vid_stream->CFSNew=vid_stream->CFSNext;
	  ReadFS(vid_stream);
	  MpegEncodeIPBDFrame(vid_stream);
	}
      for(i=1;i<vid_stream->FrameInterval;i++)      /* Load Interpolated Frames */
	{
	  vid_stream->CurrentFrame=vid_stream->BaseFrame+i;
	  vid_stream->FrameDistance = i;
	  vid_stream->pic.TemporalReference = (vid_stream->CurrentFrame-vid_stream->GroupFirstFrame)%TEMPORAL_MODULO;
	  vid_stream->pic.PType=P_INTERPOLATED;
	  MakeFileNames(vid_stream);
	  /* VerifyFiles(vid_stream); */
	  vid_stream->CFStore=vid_stream->CFSUse; vid_stream->CFSNew=vid_stream->CFSMid;
	  ReadFS(vid_stream);
	  MpegEncodeIPBDFrame(vid_stream);
	}
      vid_stream->BaseFrame+=vid_stream->FrameInterval;         /* Shift base frame to next interval */

  vid_stream->FrameIntervalCount++;
}

static void MpegEncodeSequenceLast(mpeg1encoder_VidStream *vid_stream) 
{
  WriteVEHeader(vid_stream);              /* Write out the end header... */
  swclose(vid_stream);

  printf("END>SEQUENCE\n");
  printf("Number of buffer overflows: %d\n",vid_stream->NumberOvfl);
}

/*BFUNC

MpegEncodeDSequence() encodes the DC intraframe sequence defined by
the CImage and CFrame structures, startframe and lastframe.

EFUNC*/

static void MpegEncodeDSequence(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MpegEncodeDSequence"); */

  MakeFGroup(vid_stream);        /* Make our group structure */
  MakeStat(vid_stream);          /* Make the statistics structure */
  MakeFS(vid_stream,READ_IOB);    /* Make our frame stores */
  vid_stream->CFSUse=vid_stream->CFStore;
  MakeFS(vid_stream,WRITE_IOB);
  vid_stream->CFSNext=vid_stream->CFStore;

  swopen(vid_stream,vid_stream->CImage->StreamFileName);  /* Open file */
  if (vid_stream->Loud > MUTE)
    {
      PrintImage(vid_stream);
      PrintFrame(vid_stream);
    }
  if (!vid_stream->InitialQuant)                      /* Actually useless */
    vid_stream->InitialQuant=DEFAULT_QUANTIZATION;
  vid_stream->UseQuant=vid_stream->slice.SQuant=vid_stream->mb.MQuant=vid_stream->InitialQuant;
  vid_stream->BufferOffset=0;
  vid_stream->TotalBits=0;
  vid_stream->NumberOvfl=0;
  printf("START>SEQUENCE\n");
  vid_stream->seq.HorizontalSize = vid_stream->CImage->Width;  /* Set up dimensions */
  vid_stream->seq.VerticalSize = vid_stream->CImage->Height;
  vid_stream->gop.TimeCode = Integer2TimeCode(vid_stream, vid_stream->StartFrame);
  WriteVSHeader(vid_stream);    /* Write out the header file */
  vid_stream->gop.ClosedGOP=1;        /* Closed GOP is 1 at start of encoding. */
  WriteGOPHeader(vid_stream);   /* Set up first frame */
  vid_stream->gop.ClosedGOP=0;        /* Reset closed GOP */
  vid_stream->pic.TemporalReference = 0;
  vid_stream->pic.PType=P_DCINTRA;

  for(vid_stream->CurrentFrame=vid_stream->StartFrame;vid_stream->CurrentFrame<=vid_stream->LastFrame;vid_stream->CurrentFrame++)
    {
      MakeFileNames(vid_stream);
      /* VerifyFiles(vid_stream); */
      vid_stream->CFStore=vid_stream->CFSUse; vid_stream->CFSNew=vid_stream->CFSNext;
      ReadFS(vid_stream);
      MpegEncodeIPBDFrame(vid_stream);
      vid_stream->pic.TemporalReference++;
    }
  WriteVEHeader(vid_stream);              /* Write out the end header... */
  swclose(vid_stream);
/*
  SaveMem("XX",CFS->fs[0]->mem);
  SaveMem("YY",OFS->fs[0]->mem);
*/
  printf("END>SEQUENCE\n");
  printf("Number of buffer overflows: %d\n",vid_stream->NumberOvfl);
}

/*BFUNC

ExecuteQuantization() references the program in memory to define the
quantization required for coding the next block.

EFUNC*/

static void ExecuteQuantization(mpeg1encoder_VidStream *vid_stream, int *Qptr)
{
  /* BEGIN("ExecuteQuantization"); */
  int CurrentSize;

  CurrentSize = BufferContents(vid_stream);
  *Qptr = (CurrentSize/vid_stream->QDFact) + vid_stream->QOffs;
  if ((vid_stream->pic.PType==P_INTRA)||(vid_stream->pic.PType==P_PREDICTED))
    *Qptr = *Qptr/2;
  if (*Qptr<1) *Qptr=1;
  if (*Qptr>31) *Qptr=31;
  if (vid_stream->Oracle)  /* If oracle, then consult oracle */
    {
      vid_stream->Memory[L_SQUANT] = (double) vid_stream->slice.SQuant;
      vid_stream->Memory[L_MQUANT] = (double) vid_stream->mb.MQuant;
      vid_stream->Memory[L_PTYPE] = (double) vid_stream->pic.PType;
      vid_stream->Memory[L_MTYPE] = (double) vid_stream->mb.MType;
      vid_stream->Memory[L_RATE] = (double) vid_stream->Rate;
      vid_stream->Memory[L_BUFFERSIZE] = (double) vid_stream->BufferSize;
      vid_stream->Memory[L_BUFFERCONTENTS] = (double) CurrentSize;
      vid_stream->Memory[L_QDFACT] = (double) vid_stream->QDFact;
      vid_stream->Memory[L_QOFFS] = (double) vid_stream->QOffs;
      /* Execute(1); */
      vid_stream->slice.SQuant = (int)  vid_stream->Memory[L_SQUANT];  /* Possibly check Mquant */
      if (vid_stream->slice.SQuant<1) vid_stream->slice.SQuant=1;
      if (vid_stream->slice.SQuant>31) vid_stream->slice.SQuant=31;
    }
  printf("BufferContents: %d  New SQuant: %d\n",CurrentSize,*Qptr);
}

/*BFUNC

CleanStatistics() cleans/initializes the type statistics in memory.

EFUNC*/

static void CleanStatistics(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("CleanStatistics"); */
  int x;
  for(x=0;x<vid_stream->MaxTypes;x++)
    {
      vid_stream->MacroTypeFrequency[x]=0; /* Initialize Statistics */
      vid_stream->YTypeFrequency[x]=0;
      vid_stream->UVTypeFrequency[x]=0;
    }
  vid_stream->MotionVectorBits=vid_stream->MacroAttributeBits=0;
  vid_stream->YCoefBits=vid_stream->UCoefBits=vid_stream->VCoefBits=vid_stream->EOBBits=0;
  vid_stream->QUse=vid_stream->QSum=0;
  vid_stream->NumberNZ=0;
  vid_stream->StuffCount=0;
}

/*BFUNC

CollectStatistics() is used to assemble and calculate the relevant
encoding statistics.  It also prints these statistics out to the
screen.

EFUNC*/

static void CollectStatistics(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("CollectStatistics"); */
  int x;

  x = mwtell(vid_stream);                    /* Calculate the size in bits */
  vid_stream->LastBits = x - vid_stream->TotalBits;
  vid_stream->TotalBits = x;
  printf("Total No of Bits: %8d  Bits for Frame: %8d\n",
	 vid_stream->TotalBits,vid_stream->LastBits);
  if (vid_stream->Rate)
    {
      printf("Buffer Contents: %8ld  out of: %8d\n",
	     BufferContents(vid_stream),
	     vid_stream->BufferSize);
    }
  printf("MB Attribute Bits: %6d  MV Bits: %6d   EOB Bits: %6d\n",
	 vid_stream->MacroAttributeBits,vid_stream->MotionVectorBits,vid_stream->EOBBits);
  printf("Y Bits: %7d  U Bits: %7d  V Bits: %7d  Total Bits: %7d\n",
	 vid_stream->YCoefBits,vid_stream->UCoefBits,vid_stream->VCoefBits,(vid_stream->YCoefBits+vid_stream->UCoefBits+vid_stream->VCoefBits));
  printf("MV StepSize: %f  MV NumberNonZero: %f  MV NumberZero: %f\n",
	 (double) ((double) vid_stream->QSum)/((double)(vid_stream->QUse)),
	 (double) ((double) vid_stream->NumberNZ)/
	 ((double)(vid_stream->MBHeight*vid_stream->MBWidth*6)),
	 (double) ((double) (vid_stream->MBHeight*vid_stream->MBWidth*6*64)- vid_stream->NumberNZ)/
	 ((double)(vid_stream->MBHeight*vid_stream->MBWidth*6)));
  printf("Code MType: ");
  for(x=0;x<vid_stream->MaxTypes;x++) printf("%5d",x);
  printf("\n");
  printf("Macro Freq: ");
  for(x=0;x<vid_stream->MaxTypes;x++) printf("%5d",vid_stream->MacroTypeFrequency[x]);
  printf("\n");
  printf("Y     Freq: ");
  for(x=0;x<vid_stream->MaxTypes;x++) printf("%5d",vid_stream->YTypeFrequency[x]);
  printf("\n");
  printf("UV    Freq: ");
  for(x=0;x<vid_stream->MaxTypes;x++) printf("%5d",vid_stream->UVTypeFrequency[x]);
  printf("\n");
}


/*BFUNC

MVBoundIndex() calculates the index for the motion vectors given two motion
vector arrays, MVX, and MVY.  The return value is appropriate for
ForwardIndex of BackwardIndex

EFUNC*/

static int MVBoundIndex(mpeg1encoder_VidStream *vid_stream, int *MVX, int *MVY)
{
  BEGIN("MVBoundIndex");
  int i, mvpos=0, mvneg=0, mvtest, best;

  for(i=0;i<vid_stream->MBWidth*vid_stream->MBHeight;i++)
    {
      mvtest = *(MVX++);
      if (mvtest>mvpos) mvpos = mvtest;
      else if (mvtest<mvneg) mvneg = mvtest;
      mvtest = *(MVY++);
      if (mvtest>mvpos) mvpos = mvtest;
      else if (mvtest<mvneg) mvneg = mvtest;
    }

  best = (mvpos>>4);  /* Get positive bound, some (1<<4)*(1<<FI)-1 */
  mvneg = -mvneg;                      /* Get similar neg bound, */
  if (mvneg>0) mvneg = (mvneg-1)>>4;   /* some -(1<<4)*(1<<FI) */
  else mvneg = 0;
  
  if (mvneg > best) best = mvneg;

  /* Now we need to find the log of this value between 0 and n */

  if (best >= (1<<5))
    {
      WHEREAMI();
      printf("Warning: at least one motion vector out of range.\n");
    }
  for(i=4;i>=0;i--) if (best&(1<<i)) break;
  return(i+2);
}

/*BFUNC

MpegEncodeIPBDFrame(Intra,Predicted,)
     ) is used to encode a single Intra; is used to encode a single Intra, Predicted,
Bidirectionally predicted, DC Intra frame to the opened stream.

EFUNC*/

void MpegEncodeIPBDFrame(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegEncodeIPBDFrame");
  int i,length;

  if (vid_stream->Rate)                               /* Assume no decoder delay */
    vid_stream->pic.BufferFullness = (90000*((vid_stream->BufferSize - BufferContents(vid_stream))/400))/(vid_stream->Rate/400);

  switch(vid_stream->pic.PType)
    {
    case P_INTRA:
      printf("START>Intraframe: %d\n",vid_stream->CurrentFrame);
      vid_stream->MaxTypes=2;
      vid_stream->slice.SQuant=vid_stream->InitialQuant/2;  /* Preliminary rate control */
      break;
    case P_PREDICTED:
      printf("START>Predicted: %d\n",vid_stream->CurrentFrame);
      vid_stream->MaxTypes=7;
      vid_stream->slice.SQuant=vid_stream->InitialQuant/2;
      break;
    case P_INTERPOLATED:
      printf("START>Interpolated: %d\n",vid_stream->CurrentFrame);
      vid_stream->MaxTypes=11;
      vid_stream->slice.SQuant=vid_stream->InitialQuant;
      break;
    case P_DCINTRA:
      printf("START>DC Intraframe: %d\n",vid_stream->CurrentFrame);
      MpegEncodeDFrame(vid_stream);
      return;
      break;
    default:
      printf("Unknown PType: %d\n",vid_stream->pic.PType);
      break;
    }
  CleanStatistics(vid_stream);


  vid_stream->pic.ForwardIndex=vid_stream->pic.BackwardIndex=0;
  if (vid_stream->DynamicMVBound)
    {
      /* Calculate a larger motion vector */

      if ((vid_stream->pic.PType==P_PREDICTED)||(vid_stream->pic.PType==P_INTERPOLATED))
	vid_stream->pic.ForwardIndex = MVBoundIndex(vid_stream,vid_stream->FMX[vid_stream->FrameDistance],vid_stream->FMY[vid_stream->FrameDistance]);
      if ((vid_stream->pic.PType==P_INTERPOLATED))
	vid_stream->pic.BackwardIndex = MVBoundIndex(vid_stream,vid_stream->BMX[vid_stream->FrameDistance],vid_stream->BMY[vid_stream->FrameDistance]);
    }
  else
    {
      /* The following equations rely on a maximum bound from -16->15*fd */
      /* for telescopic motion estimation; if a larger motion vector */  
      /* is desired, it must be calculated as below. */

      if ((vid_stream->pic.PType==P_PREDICTED)||(vid_stream->pic.PType==P_INTERPOLATED))
	{
	  vid_stream->pic.ForwardIndex=vid_stream->FrameDistance-1;
	  if (vid_stream->pic.ForwardIndex >= (1<<5))
	    {
	      WHEREAMI();
	      printf("Warning: possible motion vectors out of range.\n");
	    }
	  for(i=4;i>=0;i--) if (vid_stream->pic.ForwardIndex&(1<<i)) break;
	  vid_stream->pic.ForwardIndex = i+2;
	}
      

      if ((vid_stream->pic.PType==P_INTERPOLATED))
	{
	  vid_stream->pic.BackwardIndex = vid_stream->FrameInterval-vid_stream->FrameDistance-1;
	  if (vid_stream->pic.BackwardIndex >= (1<<5))
	    {
	      WHEREAMI();
	      printf("Warning: possible motion vectors out of range.\n");
	    }
	  for(i=4;i>=0;i--) if (vid_stream->pic.BackwardIndex&(1<<i)) break;
	  vid_stream->pic.BackwardIndex = i+2;
	}
    }

  if ((vid_stream->pic.ForwardIndex>6)||(vid_stream->pic.BackwardIndex>6))
    {
      WHEREAMI();
      printf("Warning: possible motion vectors out of range.\n");
    }
  
  /*printf("ForwardIndex: %d;  BackwardIndex: %d\n",
	 ForwardIndex, BackwardIndex);*/

  WritePictureHeader(vid_stream);

  /* BEGIN CODING */

  if (!vid_stream->slice.MBperSlice) vid_stream->slice.MBperSlice=vid_stream->MBWidth;  /* Set macroblocks per slice */
  vid_stream->HPos=vid_stream->VPos=0;
  vid_stream->CurrentMBS=0;
  vid_stream->mb.LastMBA= -1;
  vid_stream->mb.CurrentMBA=0;
  while(vid_stream->VPos<vid_stream->MBHeight)
    {
      vid_stream->CurrentMBS++;
      length = vid_stream->MBWidth*vid_stream->MBHeight - (vid_stream->HPos + (vid_stream->VPos*vid_stream->MBWidth));
      if ((vid_stream->slice.MBperSlice<0)||(length<vid_stream->slice.MBperSlice)) vid_stream->slice.MBperSlice=length;
      MpegEncodeSlice(vid_stream, vid_stream->slice.MBperSlice);
    }
  vid_stream->HPos=vid_stream->VPos=0;
  if (vid_stream->Rate) vid_stream->BufferOffset -= (vid_stream->Rate*vid_stream->FrameSkip/FrameRate(vid_stream));
  CollectStatistics(vid_stream);
  Statistics(vid_stream,vid_stream->CFSUse,vid_stream->CFSNew);
}


static void MpegEncodeSlice(mpeg1encoder_VidStream *vid_stream, int Count)
{
  BEGIN("MpegEncodeSlice");
  int i,x;
  
  vid_stream->mb.LastMVD1V=vid_stream->mb.LastMVD1H=vid_stream->mb.LastMVD2V=vid_stream->mb.LastMVD2H=0; /* Reset MC */
  if (vid_stream->Loud > MUTE) printf("VPos: %d %d %d\n",vid_stream->VPos, vid_stream->CurrentFrame, vid_stream->StartFrame);
  if (((vid_stream->Rate)&&(vid_stream->CurrentFrame!=vid_stream->StartFrame))) /* Change Quantization */
    ExecuteQuantization(vid_stream, &vid_stream->slice.SQuant);      /* only after buffer filled */
  vid_stream->UseQuant=vid_stream->slice.SQuant;                        /* Reset quantization to slice */
  vid_stream->mb.SVP = vid_stream->VPos+1;
  /* printf("write slice %d\n", vid_stream->UseQuant); */
  WriteMBSHeader(vid_stream);

  for(x=0;x<3;x++) vid_stream->LastDC[x]=128;   /* Reset DC pred., irrsp. LastMType */

  /* LastMBA = (VPos*MBWidth)+HPos-1; */

  vid_stream->mb.EncEndSlice=vid_stream->mb.EncStartSlice=0;
  for(i=0;i<Count;i++)
    {
      /* printf("write slice %d %d\n", i, BufferContents(vid_stream)); */
      if (!i) vid_stream->mb.EncStartSlice=1;        /* Special flags */
      if ((i==Count-1)&&vid_stream->mb.EncPerfectSlice) vid_stream->mb.EncEndSlice=1;

      if (vid_stream->VPos >= vid_stream->MBHeight)
	{
	  WHEREAMI();
	  printf("Slice-MDU Overflow.\n");
	}
      if (vid_stream->Rate)        /* Perform preliminary rate control */
	{
	  if ((vid_stream->HPos)&&!(vid_stream->HPos%vid_stream->QUpdateFrequency)&&
	      (vid_stream->CurrentFrame!=vid_stream->StartFrame))
	    { /* Begin incremental buffer control */
	      /* Disabled for macroblock quant - trickier... */
	      /* ExecuteQuantization(&MQuant); */
	      /* if (Oracle) MQuant = (int)  Memory[L_MQUANT]; */
	    }
	  if ((BufferContents(vid_stream)>vid_stream->BufferSize))
	    {
	      vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=0; /* Motion vectors 0 */
	      vid_stream->mb.MType=0;     /* No coefficient transmission */
	      vid_stream->NumberOvfl++;
	      WHEREAMI();
	      printf("Buffer Overflow!\n");
	    }
	}
      /* printf("Hpos:%d  Vpos:%d\n",HPos,VPos); */
      MpegEncodeMDU(vid_stream);
      vid_stream->HPos++;
      if (vid_stream->HPos >= vid_stream->MBWidth)
	{
	  vid_stream->HPos=0; vid_stream->VPos++;
	}
    }
}

/*BFUNC

MpegEncodeMDU() encodes the current MDU.  It finds the macroblock
type, attempts to compress the macroblock type, and then writes the
macroblock type out. The decoded MDU is then saved for predictive
encoding.

EFUNC*/

static void MpegEncodeMDU(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegEncodeMDU");
  
  MpegFindMType(vid_stream);
  MpegCompressMType(vid_stream);
  MpegWriteMType(vid_stream);
  MpegDecodeSaveMDU(vid_stream);
  
  vid_stream->QUse++;                /* Accumulate statistics */
  vid_stream->QSum+=vid_stream->UseQuant;
  vid_stream->mb.CurrentMBA++;
  if (vid_stream->mb.MType < vid_stream->MaxTypes)
    vid_stream->MacroTypeFrequency[vid_stream->mb.MType]++;
  else
    {
      WHEREAMI();
      printf("Illegal picture type: %d macroblock type: %d.\n",
	     vid_stream->pic.PType,vid_stream->mb.MType);
    }
  vid_stream->mb.LastMType=vid_stream->mb.MType;
}


/*BFUNC

MpegFindMType() makes an initial decision as to the macroblock type
used for MPEG encoding.

EFUNC*/

static void MpegFindMType(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegFindMType");
  int c,j,h,v,x;
  int *input;
  int *inter;
  int *fmc,*bmc,*imc;
  double xValue=0,fyValue=0,byValue=0,iyValue=0,xVAR=0,fyVAR=0,byVAR=0,iyVAR=0,orM=0,orVAR=0;
  
  switch(vid_stream->pic.PType)
    {
    case P_INTRA:
      vid_stream->mb.MType=0;
      for(c=0;c<6;c++)
	{
	  input = &vid_stream->inputbuf[c][0];
	  j = BlockJ[c];
	  v = BlockV[c];
	  h = BlockH[c];
	  InstallFSIob(vid_stream,vid_stream->CFSUse,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  ReadBlock(vid_stream,input);
	}
      break;
    case P_PREDICTED:
      /* Calculate decisions */
      vid_stream->mb.MVD1H = vid_stream->FMX[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      vid_stream->mb.MVD1V = vid_stream->FMY[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      /* printf("FMX: %d  FMY: %d\n",MVD1H,MVD1V); */
      xValue=fyValue=xVAR=fyVAR=orM=orVAR=0.0;
      for(c=0;c<6;c++)
	{
	  input = &vid_stream->inputbuf[c][0];
	  inter = &vid_stream->interbuf[c][0];
	  fmc = &vid_stream->fmcbuf[c][0];
	  j = BlockJ[c];
	  v = BlockV[c];
	  h = BlockH[c];
	  InstallFSIob(vid_stream,vid_stream->CFSUse,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  ReadBlock(vid_stream,input);
	  for(x=0;x<64;x++) 
	    fmc[x] = inter[x] = input[x];
	  InstallFSIob(vid_stream,vid_stream->CFSBase,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  vid_stream->MX = vid_stream->MY = 0;                        /* Calc interframe */
	  SubCompensate(vid_stream,inter,vid_stream->CFSBase->Iob[j]);
	  if (c < 4)                         /* Calc motion comp */
	    {vid_stream->MX = vid_stream->mb.MVD1H;  vid_stream->MY = vid_stream->mb.MVD1V;}
	  else
	    {vid_stream->MX = vid_stream->mb.MVD1H/2;  vid_stream->MY = vid_stream->mb.MVD1V/2;}
	  SubCompensate(vid_stream,fmc,vid_stream->CFSBase->Iob[j]);
	  if (c < 4)                  /* Base solely on luminance */
	    {
	      for(x=0;x<64;x++)
		{
		  orM = orM + input[x];      /* Original frame */
		  orVAR = orVAR + input[x]*input[x];
		  
		  if (inter[x]>0)                   /* Interframe */
		    xValue = xValue + inter[x];
		  else
		    xValue = xValue - inter[x];
		  xVAR = xVAR + inter[x]*inter[x];
		  
		  if (fmc[x]>0)                      /* Motion comp */
		    fyValue = fyValue + fmc[x]; 
		  else
		    fyValue = fyValue - fmc[x];
		  fyVAR = fyVAR + fmc[x]*fmc[x];
		}
	    }
	}
      xValue = xValue/256.0;
      fyValue = fyValue/256.0;
      xVAR = xVAR/256.0;
      fyVAR = fyVAR/256.0;
      orM = orM/256.0;
      orVAR = orVAR/256.0;
      orVAR = orVAR - (orM*orM);
      /*
	printf("interABS: %f   mcABS: %f\n",
	xValue,fyValue);
	printf("interVAR: %f   mcVAR: %f   orVAR: %f\n",
	xVAR,fyVAR,orVAR);
	*/
      /* Decide which coding option to take (P*64) */
      if (((xValue < 3.0) && (fyValue > (xValue*0.5))) ||
	  ((fyValue > (xValue/1.1))))
	{
	  vid_stream->mb.MType = 1;                    /* Inter mode */
	  if ((xVAR >64) && (xVAR > orVAR))
	    vid_stream->mb.MType = 3;                  /* If err too high, intra */
	}
      else
	{
	  vid_stream->mb.MType = 0;                    /* MC Mode */
	  if ((fyVAR > 64) && (fyVAR > orVAR))
	    vid_stream->mb.MType = 3;                 /* If err too high, intra */
	}
      break;
    case P_INTERPOLATED:
      /* Calculate decisions */
      vid_stream->mb.MVD1H = vid_stream->FMX[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      vid_stream->mb.MVD1V = vid_stream->FMY[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      vid_stream->mb.MVD2H = vid_stream->BMX[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      vid_stream->mb.MVD2V = vid_stream->BMY[vid_stream->FrameDistance][vid_stream->VPos*vid_stream->MBWidth + vid_stream->HPos];
      xValue=fyValue=byValue=iyValue=0.0;
      xVAR=fyVAR=byVAR=iyVAR=orM=orVAR=0.0;
      for(c=0;c<6;c++)
	{
	  input = &vid_stream->inputbuf[c][0];
	  inter = &vid_stream->interbuf[c][0];
	  fmc = &vid_stream->fmcbuf[c][0];
	  bmc = &vid_stream->bmcbuf[c][0];
	  imc = &vid_stream->imcbuf[c][0];
	  j = BlockJ[c];
	  v = BlockV[c];
	  h = BlockH[c];
	  InstallFSIob(vid_stream,vid_stream->CFSUse,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  ReadBlock(vid_stream,input);
	  for(x=0;x<64;x++) 
	    imc[x]=bmc[x]=fmc[x]=inter[x]=input[x];
	  InstallFSIob(vid_stream,vid_stream->CFSBase,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  InstallFSIob(vid_stream,vid_stream->CFSNext,j);
	  MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	  vid_stream->MX = vid_stream->MY = 0;
	  SubCompensate(vid_stream,inter,vid_stream->CFSBase->Iob[j]);
	  if (c < 4)
	    {
	      vid_stream->NX = vid_stream->mb.MVD2H;  vid_stream->NY = vid_stream->mb.MVD2V;
	      vid_stream->MX = vid_stream->mb.MVD1H;  vid_stream->MY = vid_stream->mb.MVD1V;
	    }
	  else
	    {
	      vid_stream->NX = vid_stream->mb.MVD2H/2;  vid_stream->NY = vid_stream->mb.MVD2V/2;
	      vid_stream->MX = vid_stream->mb.MVD1H/2;  vid_stream->MY = vid_stream->mb.MVD1V/2;
	    }
	  SuperSubCompensate(vid_stream,fmc,bmc,imc,
			     vid_stream->CFSBase->Iob[j],vid_stream->CFSNext->Iob[j]);
	  if (c < 4)
	    {
	      for(x=0;x<64;x++)
		{
		  orM = orM + input[x];
		  orVAR = orVAR + input[x]*input[x];
		  if (inter[x]>0)
		    xValue = xValue + inter[x];
		  else
		    xValue = xValue - inter[x];
		  xVAR = xVAR + inter[x]*inter[x];
		  if (fmc[x]>0)
		    fyValue = fyValue + fmc[x];
		  else
		    fyValue = fyValue - fmc[x];
		  fyVAR = fyVAR + fmc[x]*fmc[x];
		  if (bmc[x]>0)
		    byValue = byValue + bmc[x];
		  else
		    byValue = byValue - bmc[x];
		  byVAR = byVAR + bmc[x]*bmc[x];
		  if (imc[x]>0)
		    iyValue = iyValue + imc[x];
		  else
		    iyValue = iyValue - imc[x];
		  iyVAR = iyVAR + imc[x]*imc[x];
		}
	    }
	}
      xValue = xValue/256.0;
      fyValue = fyValue/256.0;
      byValue = byValue/256.0;
      iyValue = iyValue/256.0;
      xVAR = xVAR/256.0;
      fyVAR = fyVAR/256.0;
      byVAR = byVAR/256.0;
      iyVAR = iyVAR/256.0;
      orM = orM/256.0;
      orVAR = orVAR/256.0;
      orVAR = orVAR - (orM*orM);
      /*
	printf("interVAR:%f  orVAR:%f\n",
	xVAR,orVAR);
	printf("fmcVAR:%f  bmcVAR:%f  imcVAR:%f\n",
	fyVAR,byVAR,iyVAR);
	*/
      if (iyVAR<=fyVAR)
	{
	  if (iyVAR<=byVAR)
	    {
	      if ((iyVAR > 64) && (iyVAR > orVAR)) {vid_stream->mb.MType = 6;}
	      else {vid_stream->mb.MType=1;}
	    }
	  else 
	    {
	      if ((byVAR > 64) && (byVAR > orVAR)) {vid_stream->mb.MType = 6;}
	      else {vid_stream->mb.MType=3;}
	    }
	}
      else if (byVAR <= fyVAR)
	{
	  if ((byVAR > 64) && (byVAR > orVAR)) {vid_stream->mb.MType = 6;}
	  else {vid_stream->mb.MType=3;}
	}
      else
	{
	  if ((fyVAR > 64) && (fyVAR > orVAR)) {vid_stream->mb.MType = 6;}
	  else {vid_stream->mb.MType=5;}
	}
      /* printf("[%d,%d:%d]",HPos,VPos,MType);*/
      break;
    default:
      WHEREAMI();
      printf("Unknown type: %d\n",vid_stream->pic.PType);
      break;
    }
  if (vid_stream->Oracle)
    {
      vid_stream->Memory[L_SQUANT] = (double) vid_stream->slice.SQuant;
      vid_stream->Memory[L_MQUANT] = (double) vid_stream->mb.MQuant;
      vid_stream->Memory[L_PTYPE] = (double) vid_stream->pic.PType;
      vid_stream->Memory[L_MTYPE] = (double) vid_stream->mb.MType;
      vid_stream->Memory[L_BD] = (double) xValue;
      vid_stream->Memory[L_FDBD] = (double) fyValue;
      vid_stream->Memory[L_BDBD] = (double) byValue;
      vid_stream->Memory[L_IDBD] = (double) iyValue;
      vid_stream->Memory[L_VAROR] = (double) orVAR;
      vid_stream->Memory[L_FVAR] = (double) fyVAR;
      vid_stream->Memory[L_BVAR] = (double) byVAR; 
      vid_stream->Memory[L_IVAR] = (double) iyVAR;
      vid_stream->Memory[L_DVAR] = (double) xVAR;
      vid_stream->Memory[L_RATE] = (double) vid_stream->Rate;
      vid_stream->Memory[L_BUFFERSIZE] = (double) vid_stream->BufferSize;
      vid_stream->Memory[L_BUFFERCONTENTS] = (double) BufferContents(vid_stream);
      vid_stream->Memory[L_QDFACT] = (double) vid_stream->QDFact;
      vid_stream->Memory[L_QOFFS] = (double) vid_stream->QOffs;
      /* Execute(0); */
      vid_stream->mb.MType = (int) vid_stream->Memory[L_MTYPE];
    }
}


/*BFUNC

MpegWriteMType() writes a macroblock type out to the stream.  It
handles the predictive nature of motion vectors, etc.

EFUNC*/

static void MpegWriteMType(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegWriteMType");
  int c,j,x;
  int *input;
  
  
  /* We only erase motion vectors when required */
  if (vid_stream->pic.PType==P_PREDICTED)
    {
      if (!MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	vid_stream->mb.LastMVD1V=vid_stream->mb.LastMVD1H=vid_stream->mb.MVD1V=vid_stream->mb.MVD1H=0;  /* Erase forward mv */
    }
  else if (vid_stream->pic.PType==P_INTERPOLATED)
    {
      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{
	  vid_stream->mb.LastMVD1V=vid_stream->mb.LastMVD1H=vid_stream->mb.MVD1V=vid_stream->mb.MVD1H=0;  /* Erase forward mv */
	  vid_stream->mb.LastMVD2V=vid_stream->mb.LastMVD2H=vid_stream->mb.MVD2V=vid_stream->mb.MVD2H=0;  /* Erase backward mv */
	}
    }
  while(BufferContents(vid_stream)< 0)
    {
      WriteStuff(vid_stream);
      vid_stream->StuffCount++;
      if (vid_stream->Loud > TALK)
	{
          WHEREAMI();
          printf("Stuffing for underflow.\n");
	}
    }
  if (!vid_stream->mb.SkipMode)
    {
      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.CBP=0x3f;
      else if (!CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.CBP=0;

      if (vid_stream->mb.EncStartSlice)
	{
	  vid_stream->mb.MBAIncrement= vid_stream->HPos+1;
	  vid_stream->mb.EncStartSlice=0;
	}
      else
	vid_stream->mb.MBAIncrement = (vid_stream->mb.CurrentMBA-vid_stream->mb.LastMBA);
      vid_stream->mb.LastMBA = vid_stream->mb.CurrentMBA;

      /* printf("[MBAInc: %d; Sigma= %d]\n",MBAIncrement,CurrentMBA); */
      WriteMBHeader(vid_stream);
      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{
	  if (!(IPMType[vid_stream->pic.PType][vid_stream->mb.LastMType]))
	    for(x=0;x<3;x++) vid_stream->LastDC[x]=128;  /* Reset DC prediction */
	}
      for(c=0;c<6;c++)
	{
	  j = BlockJ[c];
	  input = &vid_stream->outputbuf[c][0];
	  if (vid_stream->mb.CBP & bit_set_mask[5-c])
	    {
	      if(j) {vid_stream->UVTypeFrequency[vid_stream->mb.MType]++;}
	      else {vid_stream->YTypeFrequency[vid_stream->mb.MType]++;}
	      vid_stream->CodedBlockBits=0;
	      if (CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType])
		CBPEncodeAC(vid_stream,0,input);
	      else
		{
		  if(j)
		    EncodeDC(vid_stream,*input-vid_stream->LastDC[j],vid_stream->huff.DCChromEHuff);
		  else
		    EncodeDC(vid_stream,*input-vid_stream->LastDC[j],vid_stream->huff.DCLumEHuff);
		  vid_stream->LastDC[j] = *input;
		  EncodeAC(vid_stream,1,input);
		}
	      if (vid_stream->Loud > TALK)
		{
		  printf("CMBS: %d  CMDU %d\n",
			 vid_stream->VPos,vid_stream->HPos);
		  PrintMatrix(input);
		}
	      if(!j){vid_stream->YCoefBits+=vid_stream->CodedBlockBits;}
	      else if(j==1){vid_stream->UCoefBits+=vid_stream->CodedBlockBits;}
	      else{vid_stream->VCoefBits+=vid_stream->CodedBlockBits;}
	    }
	}
    }
  else
    {
      vid_stream->mb.CBP=0;  /* Bypass any decoding */
      /* Added 8/18/93 */
      for(x=0;x<3;x++) vid_stream->LastDC[x]=128;  /* Reset DC prediction */
    }
}


/*BFUNC

MpegCompressMType() makes sure that the macroblock type is legal.  It
also handles skipping, zero CBP, and other MPEG-related macroblock
stuff.

EFUNC*/

static void MpegCompressMType(mpeg1encoder_VidStream *vid_stream)
{
  int c,x;
  int *input;
  
  /* Set quant */

  if (QuantPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      vid_stream->UseQuant=vid_stream->mb.MQuant;
      vid_stream->slice.SQuant=vid_stream->mb.MQuant;       /* Resets it for quantizations forward */
    }
  else {vid_stream->UseQuant=vid_stream->slice.SQuant;}

  vid_stream->mb.SkipMode=0;
  vid_stream->mb.CBP = 0x00;
  for(c=0;c<6;c++)
    {
      if ((MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])&&(MBPMType[vid_stream->pic.PType][vid_stream->mb.MType]))
	{input = &vid_stream->imcbuf[c][0];}
      else if (MBPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{input = &vid_stream->bmcbuf[c][0];}
      else if (MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{input = &vid_stream->fmcbuf[c][0];}
      else if (!IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{input = &vid_stream->interbuf[c][0];}
      else 
	{input = &vid_stream->inputbuf[c][0];}
      DefaultDct(vid_stream,input,vid_stream->output);
      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	MPEGIntraQuantize(vid_stream->output,MPEGIntraQ,vid_stream->UseQuant);
      else
	MPEGNonIntraQuantize(vid_stream->output,MPEGNonIntraQ,vid_stream->UseQuant);
      BoundQuantizeMatrix(vid_stream->output);
      input = &vid_stream->outputbuf[c][0];            /* Save to output buffer */
      ZigzagMatrix(vid_stream->output,input);
      if (CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{
	  for(x=0;x<64;x++)              /* Check if coding */
	    if (input[x]) {vid_stream->mb.CBP |= bit_set_mask[5-c];break;}
	}
    }
  if (vid_stream->pic.PType==P_PREDICTED)
    {
      if ((CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType]) && (!vid_stream->mb.CBP))
	{
	  if ((vid_stream->mb.MType==0)||(vid_stream->mb.MType==4))
	    vid_stream->mb.MType = 2;
	  else if ((vid_stream->mb.MType==1)||(vid_stream->mb.MType==5))
	    {
	      /* printf("Skipping.\n");*/
	      if ((!vid_stream->mb.EncEndSlice)&&(!vid_stream->mb.EncStartSlice))
		vid_stream->mb.SkipMode=1;
	      else
		{
		  vid_stream->mb.MVD1V=vid_stream->mb.MVD1H=0;
		  vid_stream->mb.MType=2;
		}
	    }
	}
    }
  else if (vid_stream->pic.PType==P_INTERPOLATED)
    {
      if (!vid_stream->mb.CBP)
	{
	  /* printf("No cbp\n"); */
	  if ((vid_stream->mb.MType==1)||(vid_stream->mb.MType==3)||(vid_stream->mb.MType==5))
	    {
	      if ((!IPMType[vid_stream->pic.PType][vid_stream->mb.LastMType])&&
		  (vid_stream->mb.LastMType==vid_stream->mb.MType))
		{/* Skipping enabled */
		  vid_stream->mb.SkipMode=1;
		  if ((MFPMType[vid_stream->pic.PType][vid_stream->mb.MType]) &&
		      ((vid_stream->mb.LastMVD1H != vid_stream->mb.MVD1H) ||
		       (vid_stream->mb.LastMVD1V != vid_stream->mb.MVD1V)))
		    vid_stream->mb.SkipMode=0;
		  if ((MBPMType[vid_stream->pic.PType][vid_stream->mb.MType]) &&
		      ((vid_stream->mb.LastMVD2H != vid_stream->mb.MVD2H)||
		       (vid_stream->mb.LastMVD2V != vid_stream->mb.MVD2V)))
		    vid_stream->mb.SkipMode=0;
		  if ((vid_stream->mb.EncStartSlice)||(vid_stream->mb.EncEndSlice)) vid_stream->mb.SkipMode=0;
		}
#ifdef FOO
#endif
	      if (!vid_stream->mb.SkipMode)
		vid_stream->mb.MType--;
	      else
		{
		  /*printf("Skip[%d:%d] T:%d  MV1[%d:%d] MV2[%d:%d].\n",
			 HPos,VPos,MType,MVD1H,MVD1V,MVD2H,MVD2V);*/
		}
	    }
	  else if (vid_stream->mb.MType==7)
	    {vid_stream->mb.MType=0;}
	  else if (vid_stream->mb.MType==8)
	    {vid_stream->mb.MType=4;}
	  else if (vid_stream->mb.MType==9)
	    {vid_stream->mb.MType=2;}
	}
    }
}

/*BFUNC

MpegEncodeDFrame() encodes just the DC Intraframe out to the currently
open stream. It avoids full DCT calculation.

EFUNC*/

static void MpegEncodeDFrame(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegEncodeDFrame");
  int c,i,j,h,v,x;
  int input[64];
  int dcval;

  if (vid_stream->pic.PType != P_DCINTRA)
    {
      WHEREAMI();
      printf("PType: %d not DCINTRA\n",vid_stream->pic.PType);
      return;
    }
  vid_stream->mb.MType=0;
  vid_stream->MaxTypes=1;
  vid_stream->mb.MBAIncrement=1;
  CleanStatistics(vid_stream);
  WritePictureHeader(vid_stream);

  if (vid_stream->slice.MBperSlice<=0) vid_stream->slice.MBperSlice=vid_stream->MBWidth;  /* Set macroblocks per slice */

  /* BEGIN CODING */
  for(vid_stream->mb.CurrentMBA=vid_stream->VPos=0;vid_stream->VPos<vid_stream->MBHeight;vid_stream->HPos=0,vid_stream->VPos++)
    {
      for(;vid_stream->HPos<vid_stream->MBWidth;vid_stream->HPos++)
	{
	  /* printf("VPos: %d\n",VPos);*/
	  if (vid_stream->Loud > MUTE) printf("Vertical Position (VPos): %d \n",vid_stream->VPos);

	  if (!(vid_stream->mb.CurrentMBA%vid_stream->slice.MBperSlice))
	    {
	      vid_stream->mb.SVP = vid_stream->VPos+1;
	      if (!vid_stream->XING) WriteMBSHeader(vid_stream);
	      for(x=0;x<3;x++) vid_stream->LastDC[x]=128; /* Reset DC pred. */
	      vid_stream->mb.MBAIncrement=vid_stream->HPos+1;
	      WriteMBHeader(vid_stream);
	    }
	  else
	    {
	      vid_stream->mb.MBAIncrement=1;
	      WriteMBHeader(vid_stream);
	    }
	  for(c=0;c<6;c++)
	    {
	      j = BlockJ[c];         /* Get addresses */
	      v = BlockV[c];
	      h = BlockH[c];
	      InstallFSIob(vid_stream,vid_stream->CFSUse,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      ReadBlock(vid_stream,input);
	      for(dcval=i=0;i<64;i++)
		dcval += input[i];
	      dcval = dcval/8;           /* Renormalize power */
	      if (dcval>0)               /* Quantize */
		dcval=(dcval + 4)/8;
	      else
		dcval=(dcval - 4)/8;

	      if(j) {vid_stream->UVTypeFrequency[vid_stream->mb.MType]++;}
	      else {vid_stream->YTypeFrequency[vid_stream->mb.MType]++;}
	      vid_stream->CodedBlockBits=0;
	      if(j)
		EncodeDC(vid_stream,dcval-vid_stream->LastDC[j],vid_stream->huff.DCChromEHuff);
	      else
		EncodeDC(vid_stream,dcval-vid_stream->LastDC[j],vid_stream->huff.DCLumEHuff);
	      vid_stream->LastDC[j] = dcval;

	      if(!j){vid_stream->YCoefBits+=vid_stream->CodedBlockBits;}
	      else if(j==1){vid_stream->UCoefBits+=vid_stream->CodedBlockBits;}
	      else{vid_stream->VCoefBits+=vid_stream->CodedBlockBits;}
		
	      /* Decode everything, just in case  */
	      /* dcval = dcval *8/8  is just identity */

	      for(x=0;x<64;x++) input[x]=dcval;
	      InstallFSIob(vid_stream,vid_stream->CFSNew,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      BoundIntegerMatrix(input);
	      WriteBlock(vid_stream,input);
	    }
	  vid_stream->QUse++;
	  vid_stream->QSum+=vid_stream->slice.SQuant;
	  vid_stream->mb.CurrentMBA++;
	  if (vid_stream->mb.MType < vid_stream->MaxTypes)
	    vid_stream->MacroTypeFrequency[vid_stream->mb.MType]++;
	  else
	    {
	      WHEREAMI();
	      printf("Illegal DCINTRA macroblock type: %d.\n",vid_stream->mb.MType);
	    }
	}
    }
  vid_stream->HPos=vid_stream->VPos=0;
  if (vid_stream->Rate) vid_stream->BufferOffset -= (vid_stream->Rate*vid_stream->FrameSkip/FrameRate(vid_stream));
  CollectStatistics(vid_stream);
  Statistics(vid_stream,vid_stream->CFSUse,vid_stream->CFSNew);
}

/*BFUNC

MpegDecodeSequence() decodes the sequence defined in the CImage and
CFrame structures; the stream is opened from this routine.

EFUNC*/

void MpegDecodeSequence(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegDecodeSequence");
  int fnum;
  int Setup;
  int Active;
  int BaseBegin=0,BaseEnd=0;
  int CurrentOffset,Diff; /* Stuff for calculating next frame */
  int FirstFrame;
  
  sropen(vid_stream,vid_stream->CImage->StreamFileName);
  FirstFrame=1;
  Active=Setup=0;
  vid_stream->CurrentFrame=vid_stream->BaseFrame-2; vid_stream->pic.PType=P_INTERPOLATED;
  if (ReadHeaderHeader(vid_stream))     /* Look for next header */
    {
      srclose(vid_stream);
      WHEREAMI();
      printf("Header anticipated.\n");
      exit(vid_stream->ErrorValue);
    }
  ReadHeaderTrailer(vid_stream);
  while(1)
    {
      if (vid_stream->mb.MBSRead == -4)        /* Video sequence start */
	{
	  if (ReadVSHeader(vid_stream))   /* Attempt to read the header file */
	    {
	      srclose(vid_stream);
	      WHEREAMI();
	      printf("Invalid VS sequence.\n");
	      exit(-1);
	    }
	  printf("START>SEQUENCE\n");
	  if (vid_stream->Rate)
	    printf("Transmission rate (bps): %d\n",vid_stream->Rate);
	  vid_stream->ImageType=IT_NTSC;
	  CreateFrameSizes(vid_stream);           /* Hor size and vert size important*/
	  if (vid_stream->Loud > MUTE)
	    {
	      PrintImage(vid_stream);
	      PrintFrame(vid_stream);
	    }
	  MakeFS(vid_stream,WRITE_IOB);
	  vid_stream->CFSBase=vid_stream->CFStore;
	  MakeFS(vid_stream,WRITE_IOB);
	  vid_stream->CFSNext=vid_stream->CFStore;
	  MakeFS(vid_stream,WRITE_IOB);
	  vid_stream->CFSMid=vid_stream->CFStore;
	  vid_stream->GroupFirstFrame=BaseBegin=BaseEnd=vid_stream->StartFrame;
	  MakeFGroup(vid_stream);
	  Setup=1;
	  if (ReadHeaderHeader(vid_stream))  /* nonzero on error or eof */
	    break; /* Could be end of file */
	  ReadHeaderTrailer(vid_stream);
	  continue;
	}
      else if (vid_stream->mb.MBSRead < 0)  /* Start of new marker */
	{
	  if (!Setup)
	    {
	      WHEREAMI();
	      printf("No first sequence code in stream!\n");
	      exit(-1);
	    }
	  if (Active)
	    {
	      printf("END>Frame: %d\n",vid_stream->CurrentFrame);
	      MakeFileNames(vid_stream);
	      WriteFS(vid_stream);                        /* Store pictures */
	    }
	  if (vid_stream->mb.MBSRead == -2)  /* Start of group of frames */
	    {
	      ReadGOPHeader(vid_stream);         /* Next, read the header again */


	      if (vid_stream->pic.PType==P_INTERPOLATED)  /* Interp means additional frame*/
		vid_stream->GroupFirstFrame=vid_stream->CurrentFrame+2;
	      else
		vid_stream->GroupFirstFrame=vid_stream->CurrentFrame+1;

	      if (vid_stream->FrameOffset<0)
		vid_stream->FrameOffset = vid_stream->GroupFirstFrame - TimeCode2Integer(vid_stream,vid_stream->gop.TimeCode);
	      else
		{
		  fnum=TimeCode2Integer(vid_stream,vid_stream->gop.TimeCode)+vid_stream->FrameOffset;
		  if (fnum!=vid_stream->GroupFirstFrame)
		    {
		      WHEREAMI();
		      printf("Time codes do not match. Frame: %d  Found: %d\n",
			     vid_stream->GroupFirstFrame,fnum);
		      if (vid_stream->mb.UseTimeCode) vid_stream->GroupFirstFrame=fnum;
		    }
		}
	      printf("GOP>FirstFrame: %d\n",vid_stream->GroupFirstFrame); 
	      Active=0;
	    }
	  else if (vid_stream->mb.MBSRead== -1)
	    {
	      ReadPictureHeader(vid_stream);
	      if (!Active)     /* Start of picture frame */
		{
		  vid_stream->CurrentFrame=vid_stream->GroupFirstFrame;
		  Active=1;
		}
	      /* Calculate next picture location */
	      CurrentOffset=(vid_stream->CurrentFrame-vid_stream->GroupFirstFrame)%TEMPORAL_MODULO;
	      Diff = (vid_stream->pic.TemporalReference-CurrentOffset+TEMPORAL_MODULO)%
		TEMPORAL_MODULO;     /* Get positive modulo difference */
	      if (Diff < (TEMPORAL_MODULO >> 1))
		vid_stream->CurrentFrame += Diff;
	      else
		vid_stream->CurrentFrame -= (TEMPORAL_MODULO - Diff);
	      printf("START>Frame: %d\n",vid_stream->CurrentFrame);
	    }
	  else if (vid_stream->mb.MBSRead == -3)      /* End of pictures */
	    {
	      printf("END>SEQUENCE\n");
	      break;
	    }
	  if (ReadHeaderHeader(vid_stream)) /* nonzero on error or eof */
	    {
	      WHEREAMI();
	      printf("Bad header after picture start.\n");
	      exit(-1);
	    }
	  ReadHeaderTrailer(vid_stream);
	  continue;
	}
      else
	{
	  if (vid_stream->Rate)
	    {
	      if (FirstFrame)
		{
		  FirstFrame=0;
		  vid_stream->BufferOffset = (vid_stream->pic.BufferFullness/90)*(vid_stream->Rate/1000) - mrtell(vid_stream);
		  printf("First decoder buffer bits = %d\n",vid_stream->BufferOffset);
		}
	      else
		{
		  printf("Actual decoder buffer bits: %ld; On stream: %d\n",
			 (vid_stream->BufferOffset - mrtell(vid_stream)),
			 (vid_stream->pic.BufferFullness/90)*(vid_stream->Rate/1000));
		}
	    }
	  switch (vid_stream->pic.PType)
	    {
	    case P_PREDICTED:
	    case P_DCINTRA:
	    case P_INTRA:
	      if (vid_stream->gop.ClosedGOP&1) vid_stream->gop.ClosedGOP++;   /* Inc by 1, to preserve */
	      else vid_stream->gop.ClosedGOP=0;               /* else zero out */
	      if (vid_stream->gop.BrokenLink&1) vid_stream->gop.BrokenLink++; /* Inc by 1, to preserve */
	      else vid_stream->gop.BrokenLink=0;              /* else zero out */

	      SwapFS(vid_stream->CFSNext,vid_stream->CFSBase);  /* Load into Next */
	      vid_stream->CFStore=vid_stream->CFSNext;
	      BaseBegin = BaseEnd;
	      BaseEnd = vid_stream->CurrentFrame;
	      vid_stream->FrameDistance=BaseEnd-BaseBegin;
	      if (vid_stream->pic.PType==P_INTRA)
		printf("Intraframe Decode: %d\n",vid_stream->CurrentFrame);
	      else if (vid_stream->pic.PType==P_PREDICTED)
		printf("Predicted Decode: %d\n",vid_stream->CurrentFrame);
	      else 
		printf("DC Intraframe: %d\n",vid_stream->CurrentFrame);
	      MpegDecodeIPBDFrame(vid_stream);
	      break;
	    case P_INTERPOLATED:
	      if (vid_stream->gop.ClosedGOP)
		{
		  WHEREAMI();
		  printf("Closed GOP frame %d has pictures in it.\n",
			 vid_stream->CurrentFrame);
		}
	      else if (vid_stream->gop.BrokenLink)
		{
		  WHEREAMI();
		  printf("Broken link frame %d may be invalid.\n",
			 vid_stream->CurrentFrame);
		}
	      vid_stream->CFStore=vid_stream->CFSMid;
	      vid_stream->FrameDistance = vid_stream->CurrentFrame-BaseBegin;
	      printf("Interpolated Decode: %d  Base Relative: %d\n",
		     vid_stream->CurrentFrame,vid_stream->FrameDistance);
	      MpegDecodeIPBDFrame(vid_stream);
	      break;
	    default:
	      WHEREAMI();
	      printf("Bad Picture Type: %d\n",vid_stream->pic.PType);
	      break;
	    }
	  if (vid_stream->Rate) vid_stream->BufferOffset += (vid_stream->Rate*vid_stream->FrameSkip/FrameRate(vid_stream));
	}
    }
  srclose(vid_stream);
}

/*BFUNC

MpegDecodeIPBDFrame() is used to decode a generic frame. Note that the
DC Intraframe decoding is farmed off to a specialized routine for
speed.

EFUNC*/

void MpegDecodeIPBDFrame(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegDecodeIPBDFrame");
  int x;
  int OldType,OldMVD1V,OldMVD1H,OldMVD2V,OldMVD2H,OldCBP;
  int NextType,NextMVD1V,NextMVD1H,NextMVD2V,NextMVD2H,NextCBP;
  int NextMBA=0,NextVPos=0;
  int StartSlice,LastPass=0;

  if (vid_stream->pic.PType==P_DCINTRA)
    {
      MpegDecodeDFrame(vid_stream);
      return;
    }
  vid_stream->CurrentMBS=0;
  vid_stream->mb.CurrentMBA=vid_stream->mb.LastMBA= -1;
  vid_stream->HPos=vid_stream->mb.LastMType= -1; /* Reset MDU, Type pred */
  vid_stream->VPos=0;
  while(1)
    {
      if (vid_stream->mb.MBSRead >= 0)                  /* Read off a slice */
	{
	  ReadMBSHeader(vid_stream);
	  vid_stream->CurrentMBS++;
	  NextVPos = vid_stream->mb.SVP-1;
	}
      else
	{
	  NextMBA = vid_stream->MBHeight*vid_stream->MBWidth-1;
	  if (vid_stream->mb.CurrentMBA >= NextMBA) break;
	  LastPass=1;
	  NextMBA++;
	}
      vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=vid_stream->mb.MVD2H=vid_stream->mb.MVD2V=0;
      /* printf("VPos: %d\n",VPos); */
      vid_stream->UseQuant=vid_stream->slice.SQuant;                 /* Reset quantization to slice */
      if (vid_stream->Loud>MUTE)
	printf("Vertical Position: %d  out of: %d.\n",vid_stream->VPos,vid_stream->MBHeight);
      if (vid_stream->VPos > vid_stream->MBHeight)
	{
	  WHEREAMI();
	  printf("VPos: %d  MBHeight: %d. Buffer Overflow\n",vid_stream->VPos,vid_stream->MBHeight);
	  return;
	}
      for(x=0;x<3;x++) vid_stream->LastDC[x]=128;   /* Reset DC prediction */
      StartSlice=1;
      while(1)     /* Handle all coding for that slice */
	{          /* Save data with previous state */
	  OldType=vid_stream->mb.MType; OldCBP = vid_stream->mb.CBP;
	  OldMVD1V=vid_stream->mb.MVD1V; OldMVD1H=vid_stream->mb.MVD1H;
	  OldMVD2V=vid_stream->mb.MVD2V; OldMVD2H=vid_stream->mb.MVD2H;
	  if (StartSlice) vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=vid_stream->mb.MVD2H=vid_stream->mb.MVD2V=0;
	  if (!LastPass)
	    {
	      if (ReadMBHeader(vid_stream)) break;
	      if (StartSlice)
		{
		  StartSlice=0;
		  NextMBA = NextVPos*vid_stream->MBWidth+vid_stream->mb.MBAIncrement-1;
		}
	      else
		NextMBA = vid_stream->mb.LastMBA + vid_stream->mb.MBAIncrement;
	    }
	  else
	    {
	      printf("Entering Last Pass: %d of %d\n",
		     vid_stream->mb.CurrentMBA,NextMBA);
	      if (LastPass++>1) break;
	    }
	  /* Save data with previous state */
	  NextType=vid_stream->mb.MType; NextCBP = vid_stream->mb.CBP;
	  NextMVD1V=vid_stream->mb.MVD1V; NextMVD1H=vid_stream->mb.MVD1H;
	  NextMVD2V=vid_stream->mb.MVD2V; NextMVD2H=vid_stream->mb.MVD2H;

	  while(vid_stream->mb.CurrentMBA < NextMBA)
	    {
	      vid_stream->mb.CurrentMBA++;
	      if (++vid_stream->HPos >= vid_stream->MBWidth)
		{vid_stream->HPos=0; vid_stream->VPos++;}
	      /* printf("Loop:HPos: %d VPos: %d\n",HPos,VPos);*/
	      if (vid_stream->mb.CurrentMBA < NextMBA)
		{
		  /* printf("Skipping Macroblock.\n"); */
		  /* Added 8/18/93 */
		  for(x=0;x<3;x++) vid_stream->LastDC[x]=128;  /* Reset DC prediction */
		  switch(vid_stream->pic.PType)  /* Skipped Macroblocks */
		    {
		    case P_INTRA:
		      WHEREAMI();
		      printf("Bad skipped macroblock.\n");
		      vid_stream->mb.MType=OldType; vid_stream->mb.CBP=0;
		      break;
		    case P_PREDICTED:
		      if (QuantPMType[vid_stream->pic.PType][OldType])
			vid_stream->mb.MType=5;
		      else vid_stream->mb.MType=1;
		      vid_stream->mb.CBP=0; vid_stream->mb.MVD1V=0;vid_stream->mb.MVD1H=0;
		      break;
		    case P_INTERPOLATED:
		      /*printf("Skip[%d:%d] T:%d  MV1[%d:%d] MV2[%d:%d].\n",
			     HPos,VPos,OldType,
			     OldMVD1H,OldMVD1V,OldMVD2H,OldMVD2V); */
		      vid_stream->mb.MType=OldType; vid_stream->mb.CBP=0;  /* No blocks coded */
		      vid_stream->mb.MVD1V = OldMVD1V; vid_stream->mb.MVD1H = OldMVD1H; /* Reset MV */
		      vid_stream->mb.MVD2V = OldMVD2V; vid_stream->mb.MVD2H = OldMVD2H;
		      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
			{
			  WHEREAMI();
			  printf("Interpolated skipped INTRA macroblock\n");
			}
		      break;
		    }
		}
	      else if (LastPass) break;
	      else  /* Reload with new MV, new macro type */
		{ 
		  vid_stream->mb.MType=NextType;vid_stream->mb.CBP=NextCBP;
		  vid_stream->mb.MVD1V=NextMVD1V;vid_stream->mb.MVD1H=NextMVD1H;
		  vid_stream->mb.MVD2V=NextMVD2V;vid_stream->mb.MVD2H=NextMVD2H;
		}
	      /* printf("[%d:%d] FM[%d:%d] BM[%d:%d]\n",
		 HPos,VPos,MVD1H,MVD1V,MVD2H,MVD2V); */

	      MpegDecompressMDU(vid_stream);
	      MpegDecodeSaveMDU(vid_stream);
	    }
	  vid_stream->mb.LastMType = vid_stream->mb.MType;
	  vid_stream->mb.LastMBA = vid_stream->mb.CurrentMBA;
	}
      if (vid_stream->mb.MBSRead<0) break;
      else ReadHeaderTrailer(vid_stream);
    }    
}

/*BFUNC

MpegDecompressMDU() is used to decompress the raw data from the
stream. Motion compensation occurs later.

EFUNC*/

static void MpegDecompressMDU(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MpegDecompressMDU"); */
  int c,j,x;
  int *input;

  if (vid_stream->Loud > MUTE)
    {
      printf("CMBS: %d CMDU: %d  LastDC: %d\n",
	     vid_stream->VPos, vid_stream->HPos, vid_stream->LastDC[0]);
    }
  if (vid_stream->pic.PType==P_PREDICTED)
    {
      if (!MFPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=0;
    }
  else if (vid_stream->pic.PType==P_INTERPOLATED)
    {
      if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType]) vid_stream->mb.MVD1H=vid_stream->mb.MVD1V=vid_stream->mb.MVD2H=vid_stream->mb.MVD2V=0;
    }
  if (QuantPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      vid_stream->UseQuant=vid_stream->mb.MQuant; /* Macroblock overrides */
      vid_stream->slice.SQuant=vid_stream->mb.MQuant;   /* Overrides for future */
    }
  else vid_stream->UseQuant=vid_stream->slice.SQuant;
  if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
    {
      if ((vid_stream->mb.LastMType<0)||!(IPMType[vid_stream->pic.PType][vid_stream->mb.LastMType]))
	for(x=0;x<3;x++) vid_stream->LastDC[x]=128;  /* Reset DC prediction */
    }                                    /* if last one wasn't Intra */
  for(c=0;c<6;c++)
    {
      j=BlockJ[c];
      input = &vid_stream->outputbuf[c][0];

      if (vid_stream->mb.CBP & bit_set_mask[5-c])
	{
	  if (CBPPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	    CBPDecodeAC(vid_stream,0,input);
	  else
	    {
	      if (j)
		*input = DecodeDC(vid_stream,vid_stream->huff.DCChromDHuff) + vid_stream->LastDC[j];
	      else
		*input = DecodeDC(vid_stream,vid_stream->huff.DCLumDHuff) + vid_stream->LastDC[j];
	      vid_stream->LastDC[j] = *input;
	      DecodeAC(vid_stream,1,input);
	    }
	  if (vid_stream->Loud > TALK)
	    {
	      printf("Cooked Input\n");
	      PrintMatrix(input);
	    }
	}
      else for(x=0;x<64;x++) input[x]=0;
    }
}


/*BFUNC

MpegDecodeSaveMDU() is used to decode and save the results into a
frame store after motion compensation.

EFUNC*/

static void MpegDecodeSaveMDU(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MpegDecodeSaveMDU"); */
  int c,j,h,v;
  int *input;

  /* printf("HPos:%d  VPos:%d\n",HPos,VPos); */
  
  for(c=0;c<6;c++)
    {
      v=BlockV[c]; h=BlockH[c]; j=BlockJ[c];
      input = &vid_stream->outputbuf[c][0];

      if (vid_stream->mb.CBP & bit_set_mask[5-c])
	{
	  IZigzagMatrix(input,vid_stream->output);
	  if (IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	    MPEGIntraIQuantize(vid_stream->output,MPEGIntraQ,vid_stream->UseQuant);
	  else
	    MPEGNonIntraIQuantize(vid_stream->output,MPEGNonIntraQ,vid_stream->UseQuant);
	  BoundIQuantizeMatrix(vid_stream->output);
	  DefaultIDct(vid_stream, vid_stream->output,input);
	}
      if (!IPMType[vid_stream->pic.PType][vid_stream->mb.MType])
	{
	  /*
	    printf("MVD1H: %d MVD1V: %d\n",MVD1H,MVD1V);
	    printf("MVD2H: %d MVD2V: %d\n",MVD2H,MVD2V);
	    */
	  
	  if ((MFPMType[vid_stream->pic.PType][vid_stream->mb.MType])&& /* Do both */
	      (MBPMType[vid_stream->pic.PType][vid_stream->mb.MType]))
	    {
	      InstallFSIob(vid_stream,vid_stream->CFSBase,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      InstallFSIob(vid_stream,vid_stream->CFSNext,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      if (c < 4)
		{
		  vid_stream->MX = vid_stream->mb.MVD1H;  vid_stream->MY = vid_stream->mb.MVD1V;
		  vid_stream->NX = vid_stream->mb.MVD2H;  vid_stream->NY = vid_stream->mb.MVD2V;
		}
	      else 
		{
		  vid_stream->MX = vid_stream->mb.MVD1H/2;  vid_stream->MY = vid_stream->mb.MVD1V/2;
		  vid_stream->NX = vid_stream->mb.MVD2H/2;  vid_stream->NY = vid_stream->mb.MVD2V/2;
		}
	      Add2Compensate(vid_stream,input,
			     vid_stream->CFSBase->Iob[j],
			     vid_stream->CFSNext->Iob[j]);
	    }
	  else if (MBPMType[vid_stream->pic.PType][vid_stream->mb.MType]) /* Do backward */
	    {
	      InstallFSIob(vid_stream,vid_stream->CFSNext,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      if (c < 4) {vid_stream->MX = vid_stream->mb.MVD2H;  vid_stream->MY = vid_stream->mb.MVD2V;}
	      else {vid_stream->MX = vid_stream->mb.MVD2H/2;  vid_stream->MY = vid_stream->mb.MVD2V/2;}
	      AddCompensate(vid_stream,input,vid_stream->CFSNext->Iob[j]);
	    }
	  else                            /* Defaults to forward */
	    {
	      InstallFSIob(vid_stream,vid_stream->CFSBase,j);
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      if (c < 4) {vid_stream->MX = vid_stream->mb.MVD1H;  vid_stream->MY = vid_stream->mb.MVD1V;}
	      else {vid_stream->MX = vid_stream->mb.MVD1H/2;  vid_stream->MY = vid_stream->mb.MVD1V/2;}
	      AddCompensate(vid_stream,input,vid_stream->CFSBase->Iob[j]);
	    }
	}
      if(!(GetFlag(vid_stream->CImage->MpegMode,M_DECODER)))
	InstallFSIob(vid_stream,vid_stream->CFSNew,j);
      else
	InstallIob(vid_stream,j);  /* Should be correct */
      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
      BoundIntegerMatrix(input);
      WriteBlock(vid_stream,input);
    }
}

/*BFUNC

MpegDecodeDFrame() decodes a single DC Intraframe off of the stream.
This function is typically called only from MpegDecodeIPBDFrame().

EFUNC*/

static void MpegDecodeDFrame(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MpegDecodeDFrame");
  int c,j,v,h,x;
  int input[64];
  int StartSlice;
  int dcval;

  vid_stream->CurrentMBS=0;
  vid_stream->mb.CurrentMBA= -1;
  vid_stream->HPos=vid_stream->VPos=0;
  while(vid_stream->mb.MBSRead >= 0)
    {
      ReadMBSHeader(vid_stream);
      vid_stream->CurrentMBS++;
      vid_stream->VPos = vid_stream->mb.SVP-1;
      /* printf("VPos: %d\n",VPos); */
      if (vid_stream->Loud>MUTE)
	printf("Vertical Position: %d  MBHeight: %d\n",vid_stream->VPos,vid_stream->MBHeight);
      if (vid_stream->VPos > vid_stream->MBHeight)
	{
	  WHEREAMI();
	  printf("VPos: %d  MBHeight: %d. Buffer Overflow\n",vid_stream->VPos,vid_stream->MBHeight);
	  return;
	}
      StartSlice=1;
      for(x=0;x<3;x++) vid_stream->LastDC[x]=128;   /* Reset DC prediction */
      while(1)     /* Handle all coding */
	{          /* Save data with previous state */
	  if (ReadMBHeader(vid_stream)) break;
	  if (StartSlice)
	    {
	      if ((vid_stream->HPos+1)!=vid_stream->mb.MBAIncrement)
		{
		  WHEREAMI();
		  printf("Start-slice MBA: %d != MBAIncr: %d\n",
			 vid_stream->HPos+1,vid_stream->mb.MBAIncrement);
		}
	      vid_stream->HPos=vid_stream->mb.MBAIncrement-1;
	      StartSlice=0;
	    }
	  else if (vid_stream->mb.MBAIncrement != 1)
	    {
	      WHEREAMI();
	      printf("Nonconsecutive MBA increments in DCINTRA frame\n");
	    }
	  vid_stream->mb.CurrentMBA++;
          /* Save data with previous state */

	  if (vid_stream->Loud > MUTE)
	    {
	      printf("CMBS: %d CMDU: %d  LastDC: %d\n",
		     vid_stream->VPos, vid_stream->HPos, vid_stream->LastDC[0]);
	    }
	  for(c=0;c<6;c++)
	    {
	      v=BlockV[c]; h=BlockH[c]; j=BlockJ[c];
	      
	      if (j)
		dcval = DecodeDC(vid_stream,vid_stream->huff.DCChromDHuff) + vid_stream->LastDC[j];
	      else
		dcval = DecodeDC(vid_stream,vid_stream->huff.DCLumDHuff) + vid_stream->LastDC[j];
	      vid_stream->LastDC[j] = dcval;
	      for(x=0;x<64;x++) input[x]=dcval;
	      InstallIob(vid_stream,j);  /* Should be correct */
	      MoveTo(vid_stream,vid_stream->HPos,vid_stream->VPos,h,v);
	      BoundIntegerMatrix(input);
	      WriteBlock(vid_stream,input);
	    }
	  if (++vid_stream->HPos >= vid_stream->MBWidth)  {vid_stream->HPos=0; vid_stream->VPos++;}
	  /* printf("HPos: %d VPos: %d\n",HPos,VPos); */
	}
      ReadHeaderTrailer(vid_stream);
    }
}

/*BFUNC

PrintImage() prints the image structure to stdout.

EFUNC*/

void PrintImage(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("PrintImage"); */
  IMAGE *CImage = vid_stream->CImage;

  printf("*** Image ID: %p ***\n",CImage);
  if (CImage)
    {
      if (CImage->StreamFileName)
	{
	  printf("StreamFileName %s\n",CImage->StreamFileName);
	}
      printf("InternalMode: %d   Height: %d   Width: %d\n",
	     CImage->MpegMode,CImage->Height,CImage->Width);
    }
}

/*BFUNC

PrintFrame() prints the frame structure to stdout.

EFUNC*/

void PrintFrame(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("PrintFrame"); */
  int i;
  FRAME *CFrame = vid_stream->CFrame;

  printf("*** Frame ID: %p ***\n",CFrame);
  if (CFrame)
    {
      printf("NumberComponents %d\n",
	     CFrame->NumberComponents);
      for(i=0;i<CFrame->NumberComponents;i++)
	{
	  printf("Component: FilePrefix: %s FileSuffix: %s\n",
		 ((*CFrame->ComponentFilePrefix[i]) ?
		  CFrame->ComponentFilePrefix[i] : "Null"),
		 ((*CFrame->ComponentFileSuffix[i]) ?
		  CFrame->ComponentFileSuffix[i] : "Null"));
	  printf("Height: %d  Width: %d\n",
		 CFrame->Height[i],CFrame->Width[i]);
	  printf("HorizontalFrequency: %d  VerticalFrequency: %d\n",
		 CFrame->hf[i],CFrame->vf[i]);
	}
    }
}

/*BFUNC

MakeImage() makes an image structure and installs it as the current
image.

EFUNC*/

void MakeImage(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MakeImage");

  if (!(vid_stream->CImage = MakeStructure(IMAGE)))
    {
      WHEREAMI();
      printf("Cannot make an image structure.\n");
    }
  vid_stream->CImage->StreamFileName = NULL;
  vid_stream->CImage->PartialFrame=0;
  vid_stream->CImage->MpegMode = 0;
  vid_stream->CImage->Height = 0;
  vid_stream->CImage->Width = 0;
}

/*BFUNC

MakeFrame() makes a frame structure and installs it as the current
frame structure.

EFUNC*/

void MakeFrame(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("MakeFrame");
  int i;

  if (!(vid_stream->CFrame = MakeStructure(FRAME)))
    {
      WHEREAMI();
      printf("Cannot make an frame structure.\n");
    }
  vid_stream->CFrame->NumberComponents = 3;
  for(i=0;i<MAXIMUM_SOURCES;i++)
    {
      vid_stream->CFrame->PHeight[i] = 0;
      vid_stream->CFrame->PWidth[i] = 0;
      vid_stream->CFrame->Height[i] = 0;
      vid_stream->CFrame->Width[i] = 0;
      vid_stream->CFrame->hf[i] = 1;
      vid_stream->CFrame->vf[i] = 1;
      *vid_stream->CFrame->ComponentFileName[i]='\0';
      *vid_stream->CFrame->ComponentFilePrefix[i]='\0';
      *vid_stream->CFrame->ComponentFileSuffix[i]='\0';
    }
}

/*BFUNC

MakeFGroup() creates a memory structure for the frame group.

EFUNC*/

void MakeFGroup(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MakeFGroup"); */
  int i;

  vid_stream->FFS = (MEM **) calloc(vid_stream->FrameInterval+1,sizeof(MEM *));
  for(i=0;i<=vid_stream->FrameInterval;i++)
    {
      vid_stream->FFS[i]= MakeMem(vid_stream->CFrame->Width[0],vid_stream->CFrame->Height[0]);
      SetMem(128,vid_stream->FFS[i]); 
    }
  initme(vid_stream);                                    /* doesn't */
}

/*BFUNC

LoadFGroup() loads in the memory structure of the current frame group.

EFUNC*/

void LoadFGroup(mpeg1encoder_VidStream *vid_stream, int index)
{
  /* BEGIN("LoadFGroup"); */
  int i;
  static char TheFileName[100];

  for(i=0;i<=vid_stream->FrameInterval;i++)
    {
      sprintf(TheFileName,"%s%d%s",
	      vid_stream->CFrame->ComponentFilePrefix[0],
	      index+i,
	      vid_stream->CFrame->ComponentFileSuffix[0]);
      if (vid_stream->CImage->PartialFrame)
	vid_stream->FFS[i] =  LoadPartialMem(TheFileName,
				 vid_stream->CFrame->PWidth[0],
				 vid_stream->CFrame->PHeight[0],
				 vid_stream->CFrame->Width[0],
				 vid_stream->CFrame->Height[0],
				 vid_stream->FFS[i]);
      else {
	      /*
	vid_stream->FFS[i] =  LoadMem(TheFileName,
			  vid_stream->CFrame->Width[0],
			  vid_stream->CFrame->Height[0],
			  vid_stream->FFS[i]);
			  */
        printf("Loading file: %s %d\n",TheFileName, i);
	memcpy(vid_stream->FFS[i]->data, vid_stream->frame_buffer[i], 
			  vid_stream->CFrame->Width[0]*vid_stream->CFrame->Height[0]);
      }
    }
}

/*BFUNC

MakeFstore() makes and installs the frame stores for the motion
estimation and compensation.

EFUNC*/

void MakeFStore(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MakeFStore"); */
  int i;

  vid_stream->CFStore = (FSTORE *) malloc(sizeof(FSTORE));
  vid_stream->CFStore->NumberComponents = 0;
  for(i=0;i<MAXIMUM_SOURCES;i++)
    {
      vid_stream->CFStore->Iob[i] = NULL;
    }
}

/*BFUNC

MakeStat() makes the statistics structure to hold all of the current
statistics. (CStat).

EFUNC*/


void MakeStat(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MakeStat"); */
  vid_stream->CStat = MakeStructure(STAT);
}

/*BFUNC

SetCCITT(QCIF,)
     ) just sets the width and height parameters for the QCIF; just sets the width and height parameters for the QCIF,
CIF, NTSC-CIF frame sizes.

EFUNC*/

void SetCCITT(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("SetCCITT");

  switch(vid_stream->ImageType)
    {
    case IT_NTSC:
      vid_stream->seq.HorizontalSize=352;
      vid_stream->seq.VerticalSize=240;
      break;
    case IT_CIF:
      vid_stream->seq.HorizontalSize=352;
      vid_stream->seq.VerticalSize=288;
      break;
    case IT_QCIF:
      vid_stream->seq.HorizontalSize=176;
      vid_stream->seq.VerticalSize=144;
      break;
    default:
      WHEREAMI();
      printf("Unknown ImageType: %d\n",vid_stream->ImageType);
      exit(ERROR_BOUNDS);
      break;
    }
}


/*BFUNC

CreateFrameSizes() is used to initialize all of the frame sizes to fit
that of the input image sequence.

EFUNC*/

void CreateFrameSizes(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("CreateFrameSizes");
  int i,maxh,maxv;
  FRAME *CFrame = vid_stream->CFrame;
  IMAGE *CImage = vid_stream->CImage;

  CFrame->NumberComponents = 3;
  CFrame->hf[0] = 2;                 /* Default numbers */
  CFrame->vf[0] = 2;                 /* DO NOT CHANGE */
  CFrame->hf[1] = 1;
  CFrame->vf[1] = 1;
  CFrame->hf[2] = 1;
  CFrame->vf[2] = 1;
  if (*CFrame->ComponentFilePrefix[0]=='\0')
    {
      WHEREAMI();
      printf("A file prefix should be specified.\n");
      exit(ERROR_BOUNDS);
    }
  for(i=0;i<CFrame->NumberComponents;i++)
    {
      if (*CFrame->ComponentFilePrefix[i]=='\0')
	{
	  strcpy(CFrame->ComponentFilePrefix[i],
		 CFrame->ComponentFilePrefix[0]);
	}
      if (*CFrame->ComponentFileSuffix[i]=='\0')
	{
	  strcpy(CFrame->ComponentFileSuffix[i],
		 DefaultSuffix[i]);
	}
    }
  vid_stream->MBWidth = (vid_stream->seq.HorizontalSize+15)/16;
  vid_stream->MBHeight = (vid_stream->seq.VerticalSize+15)/16;
  CImage->Width = vid_stream->MBWidth*16;
  CImage->Height = vid_stream->MBHeight*16;
  printf("Image Dimensions: %dx%d   MPEG Block Dimensions: %dx%d\n",
	 vid_stream->seq.HorizontalSize,vid_stream->seq.VerticalSize,CImage->Width,CImage->Height);
  maxh = CFrame->hf[0];                   /* Look for maximum vf, hf */
  maxv = CFrame->vf[0];                   /* Actually already known */
  for(i=1;i<CFrame->NumberComponents;i++)
    {
      if (CFrame->hf[i]>maxh)
	maxh = CFrame->hf[i];
      if (CFrame->vf[i]>maxv)
	maxv = CFrame->vf[i];
    }

  if (CImage->PartialFrame)
    {
      for(i=0;i<CFrame->NumberComponents;i++)
	{
	  CFrame->Width[i]=CImage->Width*CFrame->hf[i]/maxh;
	  CFrame->Height[i]=CImage->Height*CFrame->vf[i]/maxv;
	  CFrame->PWidth[i]=vid_stream->seq.HorizontalSize*CFrame->hf[i]/maxh;
	  CFrame->PHeight[i]=vid_stream->seq.VerticalSize*CFrame->vf[i]/maxv;
	}
    }
  else
    {
      for(i=0;i<CFrame->NumberComponents;i++)
	{
	  CFrame->PWidth[i]=CFrame->Width[i]=
	    CImage->Width*CFrame->hf[i]/maxh;
	  CFrame->PHeight[i]=CFrame->Height[i]=
	    CImage->Height*CFrame->vf[i]/maxv;
	}
    }
}

/*BFUNC

Help() prints out help information about the MPEG program.

EFUNC*/

#if 0
void Help()
{
  BEGIN("Help");

  printf("mpeg [-d] [-NTSC] [-CIF] [-QCIF] [-PF] [-NPS] [-MBPS mbps] [-UTC]\n");
  printf("     [-XING] [-DMVB] [-MVNT]\n");
  printf("     [-a StartNumber] [-b EndNumber]\n");
  printf("     [-h HorizontalSize] [-v VerticalSize]\n");
  printf("     [-f FrameInterval] [-g GroupInterval]\n");
  printf("     [-4] [-c] [-i MCSearchLimit] [-o] [-p PictureRate]\n");
  printf("     [-q Quantization] [-r Target Rate]\n");
  printf("     [-s StreamFile]  [-x Target Filesize] [-y]\n");
  printf("     [-z ComponentFileSuffix i]\n");
  printf("     ComponentFilePrefix1 [ComponentFilePrefix2 ComponentFilePrefix3]\n");
  printf("-NTSC (352x240)  -CIF (352x288) -QCIF (176x144) base filesizes.\n");
  printf("-PF is partial frame encoding/decoding...\n");
  printf("    is useful for files horizontalxvertical sizes not multiple of 16\n");
  printf("    otherwise files are assumed to be multiples of 16.\n");
  printf("-NPS is not-perfect slice (first/end) blocks not same...\n");
  printf("-MBPS mbps: is macroblocks per slice.\n");
  printf("-UTC Use time code - forces frames to equal time code value\n");
  printf("-XING default 160x120 partial frame, XING encoding\n");
  printf("-DMVB dynamic motion vector bounding - useful for frames\n");
  printf("      with limited motion vector movement. Not required, however.\n");
  printf("-MVNT Disables motion vector telescoping.  Useful only for very\n");
  printf("      large search windows.\n");
  printf("-d enables the decoder\n");
  printf("-a is the start filename index. [inclusive] Defaults to 0.\n");
  printf("-b is the end filename index. [inclusive] Defaults to 0.\n");
  printf(" overiding -NTSC -CIF -QCIF\n");
  printf("Dimension parameters:\n");
  printf("  -h gives horizontal size of active picture.\n");
  printf("  -v gives vertical size of active picture.\n");
  printf("  -f gives frame interval - distance between intra/pred frame (def 3).\n");
  printf("  -g gives group interval - frame intervals per group (def 2).\n");
  printf("\n");
  printf("-4 used to specify if DC intraframe mode is used for encoding.\n");
  printf("-c is used to give motion vector prediction. (default: off).\n");
  printf("-i gives the MC search area: can be very large, e.g. 128 (default 15).\n");
  printf("-o enables the interpreter.\n");
  printf("-p gives the picture rate (see coding standard; default 30hz).\n");
  printf("-q denotes Quantization, between 1 and 31.\n");
  printf("-r gives the target rate in bits per second.\n");
  printf("-s denotes StreamFile, which defaults to ComponentFilePrefix1.mpg\n");
  printf("-x gives the target filesize in bits. (overrides -r option.)\n");
  printf("-y enables Reference DCT.\n");
  printf("-z gives the ComponentFileSuffixes (repeatable).\n");
}
#endif

/*BFUNC

MakeFileNames() creates the filenames for the component files
from the appropriate prefix and suffix commands.

EFUNC*/

void MakeFileNames(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("MakeFileNames"); */
  int i;

  for(i=0;i<3;i++)
    {
      sprintf(vid_stream->CFrame->ComponentFileName[i],"%s%d%s",
	      vid_stream->CFrame->ComponentFilePrefix[i],
	      vid_stream->CurrentFrame,
	      vid_stream->CFrame->ComponentFileSuffix[i]);
    }
}

/*BFUNC

VerifyFiles() checks to see if the component files are present and
of the correct length.

EFUNC*/

void VerifyFiles(mpeg1encoder_VidStream *vid_stream)
{
  BEGIN("VerifyFiles");
  int i,FileSize;
  FILE *test;  
  
  for(i=0;i<vid_stream->CFrame->NumberComponents;i++)
    {
      if ((test = fopen(vid_stream->CFrame->ComponentFileName[i],"r")) == NULL)
	{
	  WHEREAMI();
	  printf("Cannot Open FileName %s\n",
		 vid_stream->CFrame->ComponentFileName[i]);
	  exit(ERROR_BOUNDS);
	}
      fseek(test,0,2);
      FileSize = ftell(test);
      rewind(test);
      if (vid_stream->CFrame->PHeight[i] == 0)
	{
	  if (vid_stream->CFrame->PWidth[i] == 0)
	    {
	      WHEREAMI();
	      printf("Bad File Specification for file %s\n",
		     vid_stream->CFrame->ComponentFileName[i]);
	    }
	  else
	    {
	      vid_stream->CFrame->PHeight[i] = FileSize / vid_stream->CFrame->PWidth[i];
	      printf("Autosizing Height to %d\n",
		      vid_stream->CFrame->PHeight[i]);
	    }
	}
      if (FileSize != vid_stream->CFrame->PWidth[i] * vid_stream->CFrame->PHeight[i]) 
	{
	  WHEREAMI();
	  printf("Inaccurate File Sizes: Estimated %d: %s: %d \n",
		 vid_stream->CFrame->PWidth[i] * vid_stream->CFrame->PHeight[i],
		 vid_stream->CFrame->ComponentFileName[i],
		 FileSize);
	  exit(ERROR_BOUNDS);
	}
      fclose(test);
    }
}

/*BFUNC

Integer2TimeCode() is used to convert a frame number into a SMPTE
25bit time code as specified by the standard.

EFUNC*/

int Integer2TimeCode(mpeg1encoder_VidStream *vid_stream, int fnum)
{
  BEGIN("Integer2TimeCode");
  int code,temp;
  int TCH,TCM,TCS,TCP;
  mpeg1encoder_Sequence *seq = &vid_stream->seq;

  if (seq->DropFrameFlag && seq->Prate == 0x1)
    {
      TCH = (fnum/107890)%24;    /* Compensate for irregular sampling */
      fnum = fnum%107890;
      if (fnum < 17980)          /* First 10 minutes treated specially */
	{
	  TCM = fnum/1798;       /* Find minutes */
	  fnum = fnum%1798;
	  if (!fnum) TCS=TCP=0;
	  else
	    {
	      TCS = (fnum+2)/30; /* Note that frame 1 and 2 in min. insig. */
	      TCP = (fnum+2)%30;
	    }
	}
      else                       /* Other 10 minutes have larger first min. */
	{
	  fnum-=17980;           /* Remove offset from first 10 min. */
	  temp = 10 + (fnum/17982)*10;  /* temp is number of 10 minutes */
	  fnum = fnum % 17982;          /* fnum is pictures per 10 min. */
	  if (fnum < 1800)              /* Screen out first minute */
	    {
	      TCM = temp;
	      TCS = fnum/30;            /* No offset required */
	      TCP = fnum%30;
	    }
	  else                          /* Other minutes are simple... */
	    {
	      fnum -=1800;
	      TCM = (fnum/1798) + temp + 1;
	      fnum = fnum%1798;         /* fnum is pictures per minute */
	      if (!fnum) TCS=TCP=0;
	      else
		{
		  TCS = (fnum+2)/30;   /* We know each minute offset by 2 */
		  TCP = (fnum+2)%30;
		}
	    }
	}
    }
  else
    {
      if (seq->DropFrameFlag)
	{
	  WHEREAMI();
	  printf("DropFrameFlag only possible with 29.97 Hz sampling.\n");
	}
      TCP = fnum%PrateIndex[seq->Prate];     /* Nondropped frames are simpler */
      fnum = fnum/PrateIndex[seq->Prate];
      TCS = fnum%60;
      fnum = fnum/60;
      TCM = fnum%60;
      fnum = fnum/60;
      TCH = fnum%24;
    }
/*  printf("DFF: %d TCH: %d  TCM: %d  TCS: %d  TCP: %d\n",
	 DropFrameFlag,TCH,TCM,TCS,TCP); */

  code = ((((((((((seq->DropFrameFlag<<5)|TCH)<<6)|TCM)<<1)|1)<<6)|TCS)<<6)|TCP);
  return(code);
}

/*BFUNC

TimeCode2Integer() is used to convert the 25 bit SMPTE time code into
a general frame number based on 0hr 0min 0sec 0pic.

EFUNC*/

int TimeCode2Integer(mpeg1encoder_VidStream *vid_stream, int tc)
{
  BEGIN("TimeCode2Integer");
  int fnum;
  int TCH,TCM,TCS,TCP;
  mpeg1encoder_Sequence *seq = &vid_stream->seq;

  TCP = tc &0x3f;
  tc >>=6;
  TCS = tc &0x3f;
  tc >>=6;
  if (!(tc &0x1))
    {
      WHEREAMI();
      printf("Poorly chosen time code. Spare bit not set.\n");
    }
  tc >>=1;
  TCM = tc &0x3f;
  tc >>=6;
  TCH = tc &0x1f;
  tc >>=5;
  seq->DropFrameFlag = (tc&1);

/*  printf("DFF: %d TCH: %d  TCM: %d  TCS: %d  TCP: %d\n",
    DropFrameFlag,TCH,TCM,TCS,TCP); */

  if (seq->DropFrameFlag && seq->Prate == 0x1)
    {
      fnum = TCH*107890 + TCM*1798 + (TCM ? ((TCM-1)/10)*2 : 0) + TCS*30 + TCP;
      if ((!(TCM)||(TCM%10))&&((TCS)||(TCP)))  /* If not 10 multiple minute */
	fnum -= 2;                            /* Correct for 2 pel offset */
    }
  else  /* Simple  for non-dropped frames */
    {
      if (seq->DropFrameFlag)
	{
	  WHEREAMI();
	  printf("DropFrameFlag only possible with 29.97 Hz sampling.\n");
	}
      fnum = (((((TCH*60)+TCM)*60)+TCS)*PrateIndex[seq->Prate]+TCP);
    }
  return(fnum);
}

/*END*/

