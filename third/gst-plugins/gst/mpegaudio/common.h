/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
common.h
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@gauss.enet.dec.com          *
 *                                                                    *
 * VERSION 4.0                                                        *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 2/25/91  Doulas Wong,        start of version 1.0 records          *
 *          Davis Pan                                                 *
 * 5/10/91  W. Joseph Carter    Reorganized & renamed all ".h" files  *
 *                              into "common.h" and "encoder.h".      *
 *                              Ported to Macintosh and Unix.         *
 *                              Added additional type definitions for *
 *                              AIFF, double/SANE and "bitstream.c".  *
 *                              Added function prototypes for more    *
 *                              rigorous type checking.               *
 * 27jun91  dpwe (Aware)        Added "alloc_*" defs & prototypes     *
 *                              Defined new struct 'frame_params'.    *
 *                              Changed info.stereo to info.mode_ext  *
 *                              #define constants for mode types      *
 *                              Prototype arguments if PROTO_ARGS     *
 * 5/28/91  Earle Jennings      added MS_DOS definition               *
 *                              MsDos function prototype declarations *
 * 7/10/91  Earle Jennings      added FLOAT definition as double      *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 *                              Modified some function prototypes.    *
 *                              Changed BUFFER_SIZE back to 4096.     *
 * 7/27/92  Michael Li          (re-)Ported to MS-DOS                 *
 * 7/27/92  Masahiro Iwadare    Ported to Convex                      *
 * 8/07/92  mc@tv.tek.com                                             *
 * 8/10/92  Amit Gulati         Ported to the AIX Platform (RS6000)   *
 *                              AIFF string constants redefined       *
 * 8/27/93 Seymour Shlien,      Fixes in Unix and MSDOS ports,        *
 *         Daniel Lauzon, and                                         *
 *         Bill Truerniet                                             *
 **********************************************************************/

/***********************************************************************
*
*  Global Conditional Compile Switches
*
***********************************************************************/

#define      UNIX            /* Unix conditional compile switch */
/* #define      MACINTOSH  */     /* Macintosh conditional compile switch */
/* #define      MS_DOS     */     /* IBM PC conditional compile switch */
/* #define      MSC60      */   /* Compiled for MS_DOS with MSC v6.0 */
#define      AIX             /* AIX conditional compile switch    */
/* #define      CONVEX     */  /*   CONVEX conditional compile switch */

#if defined(MSC60) 
#ifndef MS_DOS
#define MS_DOS
#endif
#ifndef PROTO_ARGS
#define PROTO_ARGS
#endif
#endif

#ifdef  UNIX
#define         TABLES_PATH     "tables"  /* to find data files */
/* name of environment variable holding path of table files */
#define         MPEGTABENV      "MPEGTABLES"
#define         PATH_SEPARATOR  "/"        /* how to build paths */
#endif  /* UNIX */

#ifdef  MACINTOSH
/* #define      TABLES_PATH ":tables:" */ /* where to find data files */
#endif  /* MACINTOSH */

/* 
 * Don't define FAR to far unless you're willing to clean up the 
 * prototypes
 */
#define FAR /*far*/

#ifdef __STDC__
#ifndef PROTO_ARGS
#define PROTO_ARGS
#endif
#endif

#ifdef CONVEX
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

/* MS_DOS and VMS do not define TABLES_PATH, so OpenTableFile will default
   to finding the data files in the default directory */

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include        <stdio.h>
#include        <string.h>
#include        <math.h>

#ifdef  UNIX
#include        <unistd.h>
#endif  /* UNIX */

#ifdef  MACINTOSH
#include        <stdlib.h>
#include        <console.h>
#endif  /* MACINTOSH */

#ifdef  MS_DOS
#include        <stdlib.h>
#ifdef MSC60
#include        <memory.h>
#else
#include        <alloc.h>
#include        <mem.h>
#endif  /* MSC60 */
#endif  /* MS_DOS */

/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/

/* General Definitions */

#ifdef  MS_DOS
#define         FLOAT                   double
#else
#define         FLOAT                   float
#endif

#define         NULL_CHAR               '\0'

#define         MAX_U_32_NUM            0xFFFFFFFF
#define         PI                      3.14159265358979
#define         PI4                     PI/4
#define         PI64                    PI/64
#define         LN_TO_LOG10             0.2302585093

#define         VOL_REF_NUM             0
#define         MPEG_AUDIO_ID           1
#define         MAC_WINDOW_SIZE         24

#define         MONO                    1
#define         STEREO                  2
#define         BITS_IN_A_BYTE          8
#define         WORD                    16
#define         MAX_NAME_SIZE           81
#define         SBLIMIT                 32
#define         FFT_SIZE                1024
#define         HAN_SIZE                512
#define         SCALE_BLOCK             12
#define         SCALE_RANGE             64
#define         SCALE                   32768
#define         CRC16_POLYNOMIAL        0x8005

/* MPEG Header Definitions - Mode Values */

#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

/* "bit_stream.h" Definitions */

#define         MINIMUM         4    /* Minimum size of the buffer in bytes */
#define         MAX_LENGTH      32   /* Maximum length of word written or
                                        read from bit stream */
#define         READ_MODE       0
#define         WRITE_MODE      1
#define         ALIGNING        8
#define         BINARY          0
#define         ASCII           1
#define         BS_FORMAT       BINARY /* BINARY or ASCII = 2x bytes */
#define         BUFFER_SIZE     4096

/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/

/* Structure for Reading Layer II Allocation Tables from File */

typedef struct {
    unsigned int    steps;
    unsigned int    bits;
    unsigned int    group;
    unsigned int    quant;
} sb_alloc, *alloc_ptr;

typedef sb_alloc        al_table[SBLIMIT][16];

/* Header Information Structure */

typedef struct {
    int version;
    int lay;
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
} layer, *the_layer;

/* Parent Structure Interpreting some Frame Parameters in Header */

typedef struct {
    layer       *header;        /* raw header information */
    int         actual_mode;    /* when writing IS, may forget if 0 chs */
    al_table    *alloc;         /* bit allocation table read in */
    int         tab_num;        /* number of table as loaded */
    int         stereo;         /* 1 for mono, 2 for stereo */
    int         jsbound;        /* first band of joint stereo coding */
    int         sblimit;        /* total number of sub bands */
} frame_params;

/* Double and SANE Floating Point Type Definitions */

typedef struct  IEEE_DBL_struct {
    unsigned long   hi;
    unsigned long   lo;
} IEEE_DBL;

typedef struct  SANE_EXT_struct {
    unsigned long   l1;
    unsigned long   l2;
    unsigned short  s1;
} SANE_EXT;

/* "bit_stream.h" Type Definitions */

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/

extern char     *mpegaudio_mode_names[4];
extern char     *mpegaudio_layer_names[3];
extern double   mpegaudio_s_freq[4];
extern int      mpegaudio_bitrate[3][15];
extern double FAR mpegaudio_multiple[64];

/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "common.c" */

#ifdef  PROTO_ARGS
extern int            mpegaudio_read_bit_alloc(int, al_table*);
extern int            mpegaudio_pick_table(frame_params*);
extern int            mpegaudio_js_bound(int, int);
extern void           mpegaudio_hdr_to_frps(frame_params*);
extern int            mpegaudio_NumericQ(char*);
extern int            mpegaudio_BitrateIndex(int, int);
extern int            mpegaudio_SmpFrqIndex(long);
extern int            mpegaudio_memcheck(char*, int, int);
extern void           FAR *mpegaudio_mem_alloc(unsigned long, char*);
extern void           mpegaudio_mem_free(void**);
extern void           mpegaudio_double_to_extended(double*, char[10]);
extern void           mpegaudio_extended_to_double(char[10], double*);
extern void           mpegaudio_I_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
                        unsigned int*);
extern void           mpegaudio_II_CRC_calc(frame_params*, unsigned int[2][SBLIMIT],
                        unsigned int[2][SBLIMIT], unsigned int*);
extern void           mpegaudio_update_CRC(unsigned int, unsigned int, unsigned int*);
extern void           mpegaudio_read_absthr(FLOAT*, int);

#ifdef  MACINTOSH
extern void           mpegaudio_set_mac_file_attr(char[MAX_NAME_SIZE], short, OsType,
                        OsType);
#endif
#ifdef MS_DOS
extern char           *mpegaudio_new_ext(char *filename, char *extname); 
#endif

#else
extern int            mpegaudio_read_bit_alloc();
extern int            mpegaudio_pick_table();
extern int            mpegaudio_js_bound();
extern void           mpegaudio_hdr_to_frps();
extern int            mpegaudio_NumericQ();
extern int            mpegaudio_BitrateIndex();
extern int            mpegaudio_SmpFrqIndex();
extern int            mpegaudio_memcheck();
extern void           FAR *mpegaudio_mem_alloc();
extern void           mpegaudio_mem_free();
extern void           mpegaudio_double_to_extended();
extern void           mpegaudio_extended_to_double();
extern int            mpegaudio_seek_sync();
extern void           mpegaudio_I_CRC_calc();
extern void           mpegaudio_II_CRC_calc();
extern void           mpegaudio_update_CRC();
extern void           mpegaudio_read_absthr();

#ifdef MS_DOS
extern char           *mpegaudio_new_ext(); 
#endif
#endif
