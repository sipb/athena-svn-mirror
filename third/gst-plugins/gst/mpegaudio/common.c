/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
common.c
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@3d.enet.dec.com             *
 *                                                                    *
 * VERSION 4.0                                                        *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 2/25/91  Doulas Wong,        start of version 1.0 records          *
 *          Davis Pan                                                 *
 * 5/10/91  W. Joseph Carter    Created this file for all common      *
 *                              functions and global variables.       *
 *                              Ported to Macintosh and Unix.         *
 *                              Added Jean-Georges Fritsch's          *
 *                              "bitstream.c" package.                *
 *                              Added routines to handle AIFF PCM     *
 *                              sound files.                          *
 *                              Added "mem_alloc()" and "mem_free()"  *
 *                              routines for memory allocation        *
 *                              portability.                          *
 *                              Added routines to convert between     *
 *                              Apple SANE extended floating point    *
 *                              format and IEEE double precision      *
 *                              floating point format.  For AIFF.     *
 * 02jul91 dpwe (Aware Inc)     Moved allocation table input here;    *
 *                              Tables read from subdir TABLES_PATH.  *
 *                              Added some debug printout fns (Write*)*
 * 7/10/91 Earle Jennings       replacement of the one float by FLOAT *
 *                              port to MsDos from MacIntosh version  *
 * 8/ 5/91 Jean-Georges Fritsch fixed bug in open_bit_stream_r()      *
 *10/ 1/91 S.I. Sudharsanan,    Ported to IBM AIX platform.           *
 *         Don H. Lee,                                                *
 *         Peter W. Farrett                                           *
 *10/3/91  Don H. Lee           implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              I_CRC_calc, II_CRC_calc and           *
 *                              update_CRC. Additions and revisions   *
 *                              are marked with dhl for clarity       *
 *10/18/91 Jean-Georges Fritsch fixed bug in update_CRC(),            *
 *                              II_CRC_calc() and I_CRC_calc()        *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 3/20/92 Jean-Georges Fritsch  fixed bug in start-of-frame search   *
 * 6/15/92 Juan Pineda          added refill_buffer(bs) "n"           *
 *                              initialization                        *
 * 7/08/92 Susanne Ritscher     MS-DOS, MSC6.0 port fixes             *
 * 7/27/92 Mike Li               (re-)Port to MS-DOS                  *
 * 8/19/92 Soren H. Nielsen     Fixed bug in I_CRC_calc and in        *
 *                              II_CRC_calc.  Added function: new_ext *
 *                              for better MS-DOS compatability       *
 * 3/10/93 Kevin Peterson       changed aiff_read_headers to handle   *
 *                              chunks in any order.  now returns     *
 *                              position of sound data in file.       *
 * 3/31/93 Jens Spille          changed IFF_* string compares to use  *
 *                              strcmp()                              *
 * 5/30/93 Masahiro Iwadare	?? the previous modification does not *
 *				  work. recovered to the original. ?? *
 * 8/27/93 Seymour Shlien,      Fixes in Unix and MSDOS ports,        *
 *         Daniel Lauzon, and                                         *
 *         Bill Truerniet                                             *
 **********************************************************************/

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include        <stdlib.h>
#include        "common.h"

#ifdef  MACINTOSH

#include        <SANE.h>
#include        <pascal.h>

#endif

#include <ctype.h>

#define TRUE 	1
#define FALSE 	0

#define MIN(A, B) 	((A) < (B) ? (A) : (B))
#define MAX(A, B) 	((A) > (B) ? (A) : (B))

/***********************************************************************
*
*  Global Variable Definitions
*
***********************************************************************/

char *mpegaudio_mode_names[4] = { "stereo", "j-stereo", "dual-ch", "single-ch" };
char *mpegaudio_layer_names[3] = { "I", "II", "III" };

double  mpegaudio_s_freq[4] = {44.1, 48, 32, 0};

int     mpegaudio_bitrate[3][15] = {
          {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
          {0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
          {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
        };

double FAR mpegaudio_multiple[64] = {
2.00000000000000, 1.58740105196820, 1.25992104989487,
1.00000000000000, 0.79370052598410, 0.62996052494744, 0.50000000000000,
0.39685026299205, 0.31498026247372, 0.25000000000000, 0.19842513149602,
0.15749013123686, 0.12500000000000, 0.09921256574801, 0.07874506561843,
0.06250000000000, 0.04960628287401, 0.03937253280921, 0.03125000000000,
0.02480314143700, 0.01968626640461, 0.01562500000000, 0.01240157071850,
0.00984313320230, 0.00781250000000, 0.00620078535925, 0.00492156660115,
0.00390625000000, 0.00310039267963, 0.00246078330058, 0.00195312500000,
0.00155019633981, 0.00123039165029, 0.00097656250000, 0.00077509816991,
0.00061519582514, 0.00048828125000, 0.00038754908495, 0.00030759791257,
0.00024414062500, 0.00019377454248, 0.00015379895629, 0.00012207031250,
0.00009688727124, 0.00007689947814, 0.00006103515625, 0.00004844363562,
0.00003844973907, 0.00003051757813, 0.00002422181781, 0.00001922486954,
0.00001525878906, 0.00001211090890, 0.00000961243477, 0.00000762939453,
0.00000605545445, 0.00000480621738, 0.00000381469727, 0.00000302772723,
0.00000240310869, 0.00000190734863, 0.00000151386361, 0.00000120155435,
1E-20
};

/***********************************************************************
 *
 * Using the decoded info the appropriate possible quantization per
 * subband table is loaded
 *
 **********************************************************************/

int mpegaudio_pick_table(fr_ps)   /* choose table, load if necess, return # sb's */
frame_params *fr_ps;
{
        int table, lay, ws, bsp, br_per_ch, sfrq;
        int sblim = fr_ps->sblimit;     /* return current value if no load */

        lay = fr_ps->header->lay - 1;
        bsp = fr_ps->header->bitrate_index;
        br_per_ch = mpegaudio_bitrate[lay][bsp] / fr_ps->stereo;
        ws = fr_ps->header->sampling_frequency;
        sfrq = mpegaudio_s_freq[ws];
        /* decision rules refer to per-channel bitrates (kbits/sec/chan) */
        if ((sfrq == 48 && br_per_ch >= 56) ||
            (br_per_ch >= 56 && br_per_ch <= 80)) table = 0;
        else if (sfrq != 48 && br_per_ch >= 96) table = 1;
        else if (sfrq != 32 && br_per_ch <= 48) table = 2;
        else table = 3;
        if (fr_ps->tab_num != table) {
           if (fr_ps->tab_num >= 0)
              mpegaudio_mem_free((void **)&(fr_ps->alloc));
           fr_ps->alloc = (al_table FAR *) mpegaudio_mem_alloc(sizeof(al_table),
                                                         "alloc");
           sblim = mpegaudio_read_bit_alloc(fr_ps->tab_num = table, fr_ps->alloc);
        }
        return sblim;
}

int mpegaudio_js_bound(lay, m_ext)
int lay, m_ext;
{
static int jsb_table[3][4] =  { { 4, 8, 12, 16 }, { 4, 8, 12, 16},
                                { 0, 4, 8, 16} };  /* lay+m_e -> jsbound */

    if(lay<1 || lay >3 || m_ext<0 || m_ext>3) {
        fprintf(stderr, "js_bound bad layer/modext (%d/%d)\n", lay, m_ext);
        exit(1);
    }
    return(jsb_table[lay-1][m_ext]);
}

void mpegaudio_hdr_to_frps(fr_ps) /* interpret data in hdr str to fields in fr_ps */
frame_params *fr_ps;
{
layer *hdr = fr_ps->header;     /* (or pass in as arg?) */

    fr_ps->actual_mode = hdr->mode;
    fr_ps->stereo = (hdr->mode == MPG_MD_MONO) ? 1 : 2;
    if (hdr->lay == 2)          fr_ps->sblimit = mpegaudio_pick_table(fr_ps);
    else                        fr_ps->sblimit = SBLIMIT;
    if(hdr->mode == MPG_MD_JOINT_STEREO)
        fr_ps->jsbound = mpegaudio_js_bound(hdr->lay, hdr->mode_ext);
    else
        fr_ps->jsbound = fr_ps->sblimit;
    /* alloc, tab_num set in pick_table */
}

void WriteHdr(fr_ps, s)
frame_params *fr_ps;
FILE *s;
{
layer *info = fr_ps->header;

   fprintf(s, "HDR:  s=FFF, id=%X, l=%X, ep=%X, br=%X, sf=%X, pd=%X, ",
           info->version, info->lay, !info->error_protection,
           info->bitrate_index, info->sampling_frequency, info->padding);
   fprintf(s, "pr=%X, m=%X, js=%X, c=%X, o=%X, e=%X\n",
           info->extension, info->mode, info->mode_ext,
           info->copyright, info->original, info->emphasis);
   fprintf(s, "layer=%s, tot bitrate=%d, sfrq=%.1f, mode=%s, ",
           mpegaudio_layer_names[info->lay-1], mpegaudio_bitrate[info->lay-1][info->bitrate_index],
           mpegaudio_s_freq[info->sampling_frequency], mpegaudio_mode_names[info->mode]);
   fprintf(s, "sblim=%d, jsbd=%d, ch=%d\n",
           fr_ps->sblimit, fr_ps->jsbound, fr_ps->stereo);
   fflush(s);
}

void WriteBitAlloc(bit_alloc, f_p, s)
unsigned int bit_alloc[2][SBLIMIT];
frame_params *f_p;
FILE *s;
{
int i,j;
int st = f_p->stereo;
int sbl = f_p->sblimit;
int jsb = f_p->jsbound;

    fprintf(s, "BITA ");
    for(i=0; i<sbl; ++i) {
        if(i == jsb) fprintf(s,"-");
        for(j=0; j<st; ++j)
            fprintf(s, "%1x", bit_alloc[j][i]);
    }
    fprintf(s, "\n");   fflush(s);
}

void WriteScale(bit_alloc, scfsi, scalar, fr_ps, s)
unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT], scalar[2][3][SBLIMIT];
frame_params *fr_ps;
FILE *s;
{
int stereo  = fr_ps->stereo;
int sblimit = fr_ps->sblimit;
int lay     = fr_ps->header->lay;
int i,j,k;

        if(lay == 2) {
            fprintf(s, "SFSI ");
            for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
                if (bit_alloc[k][i])  fprintf(s,"%d",scfsi[k][i]);
            fprintf(s, "\nSCFs ");
            for (k=0;k<stereo;k++) {
                for (i=0;i<sblimit;i++)
                    if (bit_alloc[k][i])
                        switch (scfsi[k][i]) {
                          case 0: for (j=0;j<3;j++)
                                  fprintf(s,"%2d%c",scalar[k][j][i],
                                          (j==2)?';':'-');
                                  break;
                          case 1:
                          case 3: fprintf(s,"%2d-",scalar[k][0][i]);
                                  fprintf(s,"%2d;",scalar[k][2][i]);
                                  break;
                          case 2: fprintf(s,"%2d;",scalar[k][0][i]);
                        }
                fprintf(s, "\n");
            }
        }
        else{   /* lay == 1 */
            fprintf(s, "SCFs ");
            for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
                if (bit_alloc[k][i])  fprintf(s,"%2d;",scalar[k][0][i]);
            fprintf(s, "\n");
        }
}

void WriteSamples(ch, sample, bit_alloc, fr_ps, s)
int ch;
unsigned int FAR sample[SBLIMIT];
unsigned int bit_alloc[SBLIMIT];
frame_params *fr_ps;
FILE *s;
{
int i;
int stereo = fr_ps->stereo;
int sblimit = fr_ps->sblimit;

        fprintf(s, "SMPL ");
        for (i=0;i<sblimit;i++)
                if ( bit_alloc[i] != 0)
                    fprintf(s, "%d:", sample[i]);
        if(ch==(stereo-1) )     fprintf(s, "\n");
        else                    fprintf(s, "\t");
}

int NumericQ(s) /* see if a string lookd like a numeric argument */
char *s;
{
char    c;

    while( (c = *s++)!='\0' && isspace((int)c)) /* strip leading ws */
        ;
    if( c == '+' || c == '-' )
        c = *s++;               /* perhaps skip leading + or - */
    return isdigit((int)c);
}

int mpegaudio_BitrateIndex(layr, bRate)   /* convert bitrate in kbps to index */
int     layr;           /* 1 or 2 */
int     bRate;          /* legal rates from 32 to 448 */
{
int     index = 0;
int     found = 0;

    while(!found && index<15)   {
        if(mpegaudio_bitrate[layr-1][index] == bRate)
            found = 1;
        else
            ++index;
    }
    if(found)
        return(index);
    else {
        fprintf(stderr, "BitrateIndex: %d (layer %d) is not a legal bitrate\n",
                bRate, layr);
        return(-1);     /* Error! */
    }
}

int mpegaudio_SmpFrqIndex(sRate)  /* convert samp frq in Hz to index */
long sRate;             /* legal rates 32000, 44100, 48000 */
{
    if(sRate == 44100L)
        return(0);
    else if(sRate == 48000L)
        return(1);
    else if(sRate == 32000L)
        return(2);
    else {
        fprintf(stderr, "SmpFrqIndex: %ld is not a legal sample rate\n", sRate);
        return(-1);      /* Error! */
    }
}

/*******************************************************************************
*
*  Allocate number of bytes of memory equal to "block".
*
*******************************************************************************/

void  FAR *mpegaudio_mem_alloc(block, item)
unsigned long   block;
char            *item;
{

    void    *ptr;

#ifdef  MACINTOSH
    ptr = NewPtr(block);
#endif

#ifdef MSC60
    /*ptr = (void FAR *) _fmalloc((unsigned int)block);*/ /* far memory, 92-07-08 sr */
    ptr = (void FAR *) malloc((unsigned int)block); /* far memory, 93-08-24 ss */
#endif

#if ! defined (MACINTOSH) && ! defined (MSC60)
    ptr = (void FAR *) malloc(block);
#endif

    if (ptr != NULL){
#ifdef  MSC60
        _fmemset(ptr, 0, (unsigned int)block); /* far memory, 92-07-08 sr */
#else
        memset(ptr, 0, block);
#endif
    }
    else{
        printf("Unable to allocate %s\n", item);
        exit(0);
    }
    return(ptr);
}


/****************************************************************************
*
*  Free memory pointed to by "*ptr_addr".
*
*****************************************************************************/

void    mpegaudio_mem_free(ptr_addr)
void    **ptr_addr;
{

    if (*ptr_addr != NULL){
#ifdef  MACINTOSH
        DisposPtr(*ptr_addr);
#else
        free(*ptr_addr);
#endif
        *ptr_addr = NULL;
    }

}

/*******************************************************************************
*
*  Check block of memory all equal to a single byte, else return FALSE
*
*******************************************************************************/

int mpegaudio_memcheck(array, test, num)
char *array;
int test;       /* but only tested as a char (bottom 8 bits) */
int num;
{
 int i=0;

   while (array[i] == test && i<num) i++;
   if (i==num) return TRUE;
   else return FALSE;
}

/****************************************************************************
*
*  Routines to convert between the Apple SANE extended floating point format
*  and the IEEE double precision floating point format.  These routines are
*  called from within the Audio Interchange File Format (AIFF) routines.
*
*****************************************************************************/

/*
*** Apple's 80-bit SANE extended has the following format:

 1       15      1            63
+-+-------------+-+-----------------------------+
|s|       e     |i|            f                |
+-+-------------+-+-----------------------------+
  msb        lsb   msb                       lsb

The value v of the number is determined by these fields as follows:
If 0 <= e < 32767,              then v = (-1)^s * 2^(e-16383) * (i.f).
If e == 32767 and f == 0,       then v = (-1)^s * (infinity), regardless of i.
If e == 32767 and f != 0,       then v is a NaN, regardless of i.

*** IEEE Draft Standard 754 Double Precision has the following format:

MSB
+-+---------+-----------------------------+
|1| 11 Bits |           52 Bits           |
+-+---------+-----------------------------+
 ^     ^                ^
 |     |                |
 Sign  Exponent         Mantissa
*/

/*****************************************************************************
*
*  double_to_extended()
*
*  Purpose:     Convert from IEEE double precision format to SANE extended
*               format.
*
*  Passed:      Pointer to the double precision number and a pointer to what
*               will hold the Apple SANE extended format value.
*
*  Outputs:     The SANE extended format pointer will be filled with the
*               converted value.
*
*  Returned:    Nothing.
*
*****************************************************************************/

void    mpegaudio_double_to_extended(pd, ps)
double  *pd;
char    ps[10];
{

#ifdef  MACINTOSH

        x96tox80(pd, (extended *) ps);

#else

register unsigned long  top2bits;

register unsigned short *ps2;
register IEEE_DBL       *p_dbl;
register SANE_EXT       *p_ext;
 
   p_dbl = (IEEE_DBL *) pd;
   p_ext = (SANE_EXT *) ps;
   top2bits = p_dbl->hi & 0xc0000000;
   p_ext->l1 = ((p_dbl->hi >> 4) & 0x3ff0000) | top2bits;
   p_ext->l1 |= ((p_dbl->hi >> 5) & 0x7fff) | 0x8000;
   p_ext->l2 = (p_dbl->hi << 27) & 0xf8000000;
   p_ext->l2 |= ((p_dbl->lo >> 5) & 0x07ffffff);
   ps2 = (unsigned short *) & (p_dbl->lo);
   ps2++;
   p_ext->s1 = (*ps2 << 11) & 0xf800;

#endif

}

/*****************************************************************************
*
*  extended_to_double()
*
*  Purpose:     Convert from SANE extended format to IEEE double precision
*               format.
*
*  Passed:      Pointer to the Apple SANE extended format value and a pointer
*               to what will hold the the IEEE double precision number.
*
*  Outputs:     The IEEE double precision format pointer will be filled with
*               the converted value.
*
*  Returned:    Nothing.
*
*****************************************************************************/

void    mpegaudio_extended_to_double(ps, pd)
char    ps[10];
double  *pd;
{

#ifdef  MACINTOSH

   x80tox96((extended *) ps, pd);

#else

register unsigned long  top2bits;

register IEEE_DBL       *p_dbl;
register SANE_EXT       *p_ext;

   p_dbl = (IEEE_DBL *) pd;
   p_ext = (SANE_EXT *) ps;
   top2bits = p_ext->l1 & 0xc0000000;
   p_dbl->hi = ((p_ext->l1 << 4) & 0x3ff00000) | top2bits;
   p_dbl->hi |= (p_ext->l1 << 5) & 0xffff0;
   p_dbl->hi |= (p_ext->l2 >> 27) & 0x1f;
   p_dbl->lo = (p_ext->l2 << 5) & 0xffffffe0;
   p_dbl->lo |= (unsigned long) ((p_ext->s1 >> 11) & 0x1f);

#endif

}


/****  for debugging 
showchar(str)
char str[4];
{
int i;
for (i=0;i<4;i++) printf("%c",str[i]);
printf("\n");
}
****/

/*****************************************************************************
*
*  CRC error protection package
*
*****************************************************************************/

void mpegaudio_I_CRC_calc(fr_ps, bit_alloc, crc)
frame_params *fr_ps;
unsigned int bit_alloc[2][SBLIMIT];
unsigned int *crc;
{
        int i, k;
        layer *info = fr_ps->header;
        int stereo  = fr_ps->stereo;
        int jsbound = fr_ps->jsbound;

        *crc = 0xffff; /* changed from '0' 92-08-11 shn */
        mpegaudio_update_CRC(info->bitrate_index, 4, crc);
        mpegaudio_update_CRC(info->sampling_frequency, 2, crc);
        mpegaudio_update_CRC(info->padding, 1, crc);
        mpegaudio_update_CRC(info->extension, 1, crc);
        mpegaudio_update_CRC(info->mode, 2, crc);
        mpegaudio_update_CRC(info->mode_ext, 2, crc);
        mpegaudio_update_CRC(info->copyright, 1, crc);
        mpegaudio_update_CRC(info->original, 1, crc);
        mpegaudio_update_CRC(info->emphasis, 2, crc);

        for (i=0;i<SBLIMIT;i++)
                for (k=0;k<((i<jsbound)?stereo:1);k++)
                        mpegaudio_update_CRC(bit_alloc[k][i], 4, crc);
}

void mpegaudio_II_CRC_calc(fr_ps, bit_alloc, scfsi, crc)
frame_params *fr_ps;
unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT];
unsigned int *crc;
{
        int i, k;
        layer *info = fr_ps->header;
        int stereo  = fr_ps->stereo;
        int sblimit = fr_ps->sblimit;
        int jsbound = fr_ps->jsbound;
        al_table *alloc = fr_ps->alloc;

        *crc = 0xffff; /* changed from '0' 92-08-11 shn */
        mpegaudio_update_CRC(info->bitrate_index, 4, crc);
        mpegaudio_update_CRC(info->sampling_frequency, 2, crc);
        mpegaudio_update_CRC(info->padding, 1, crc);
        mpegaudio_update_CRC(info->extension, 1, crc);
        mpegaudio_update_CRC(info->mode, 2, crc);
        mpegaudio_update_CRC(info->mode_ext, 2, crc);
        mpegaudio_update_CRC(info->copyright, 1, crc);
        mpegaudio_update_CRC(info->original, 1, crc);
        mpegaudio_update_CRC(info->emphasis, 2, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<((i<jsbound)?stereo:1);k++)
                        mpegaudio_update_CRC(bit_alloc[k][i], (*alloc)[i][0].bits, crc);

        for (i=0;i<sblimit;i++)
                for (k=0;k<stereo;k++)
                        if (bit_alloc[k][i])
                                mpegaudio_update_CRC(scfsi[k][i], 2, crc);
}

void mpegaudio_update_CRC(data, length, crc)
unsigned int data, length, *crc;
{
        unsigned int  masking, carry;

        masking = 1 << length;

        while((masking >>= 1)){
                carry = *crc & 0x8000;
                *crc <<= 1;
                if (!carry ^ !(data & masking))
                        *crc ^= CRC16_POLYNOMIAL;
        }
        *crc &= 0xffff;
}

/*****************************************************************************
*
*  End of CRC error protection package
*
*****************************************************************************/

#ifdef  MACINTOSH
/*****************************************************************************
*
*  Set Macintosh file attributes.
*
*****************************************************************************/

void    mpegaudio_set_mac_file_attr(fileName, vRefNum, creator, fileType)
char    fileName[MAX_NAME_SIZE];
short   vRefNum;
OsType  creator;
OsType  fileType;
{

short   theFile;
char    pascal_fileName[MAX_NAME_SIZE];
FInfo   fndrInfo;

        CtoPstr(strcpy(pascal_fileName, fileName));

        FSOpen(pascal_fileName, vRefNum, &theFile);
        GetFInfo(pascal_fileName, vRefNum, &fndrInfo);
        fndrInfo.fdCreator = creator;
        fndrInfo.fdType = fileType;
        SetFInfo(pascal_fileName, vRefNum, &fndrInfo);
        FSClose(theFile);

}
#endif


#ifdef  MS_DOS
/* ------------------------------------------------------------------------
new_ext.
Puts a new extension name on a file name <filename>.
Removes the last extension name, if any.
92-08-19 shn
------------------------------------------------------------------------ */
char *mpegaudio_new_ext(char *filename, char *extname)
{
  int found, dotpos;
  char newname[80];

  /* First, strip the extension */
  dotpos=strlen(filename); found=0;
  do
  {
    switch (filename[dotpos])
    {
      case '.' : found=1; break;
      case '\\':                  /* used by MS-DOS */
      case '/' :                  /* used by UNIX */
      case ':' : found=-1; break; /* used by MS-DOS in drive designation */
      default  : dotpos--; if (dotpos<0) found=-1; break;
    }
  } while (found==0);
  if (found==-1) strcpy(newname,filename);
  if (found== 1) strncpy(newname,filename,dotpos); newname[dotpos]='\0';
  strcat(newname,extname);
  return(newname);
}
#endif
