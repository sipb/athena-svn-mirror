/*
 * ==========================================================================
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ==========================================================================
 *
 * F L O A T
 *
 * Floating point conversion routines.
 *     Currently implemented:  
 *        VAX  <---> IEEE 
 *        Cray <---> IEEE
 */

/*
 * VAX Floating point
 * ------------------
 *
 * VAX has four floating types: F, D, G, and H:
 *
 *     F is 32 bits with a 1-bit sign, an 8-bit exponent field (excess
 *     128), and a 23-bit mantissa representing a fraction in the range
 *     0.5 (inclusive) to 1.0 (exclusive).  (The redundant leading "1"
 *     bit is omitted.)
 *
 *     D is 64 bits with a 1 bit sign, an 8-bit exponent, and a 56-bit
 *     mantissa.
 *
 *     G is 64 bits with a 1 bit sign, an 11-bit exponent (excess 1024),
 *     and a 52-bit mantissa.
 *
 *     H is 128 bits. ...
 * 
 *
 * IEEE Floating point
 * -------------------
 *
 * IEEE has two floating types: single and double:
 *
 *     Single is 32 bits with a 1-bit sign, an 8-bit exponent field 
 *     (excess 127 = 16#7F), and a 23-bit mantissa representing a fraction 
 *     in the range 1.0 (inclusive) to 2.0 (exclusive).  (The redundant,
 *     leading "1" bit is omitted.)
 *
 *                              -38         +38
 *                  range   = 10     ..   10      
 *
 *                  MaxReal = 3.402823E+38
 *
 *     Double is 64 bits with a 1-bit sign, an 11-bit exponent field 
 *     (excess 1023 = 16#3FF) and a 52-bit mantissa.
 *
 *                                -308        +308
 *                  range     = 10     ..   10        
 *
 *                  MaxReal = 1.797693134862316E+308
 *
 * Note that (unlike all other allowed NDR floating point representations)
 * IEEE floating point numbers are generated on machines made by different
 * manufacturers (and with different architectures).  In particular, some
 * of those machines are little-endian (e.g. 80x86) and some are big-endian
 * (e.g. M680x0).  NDR defines that to interpret bytes in a stream tagged
 * as using IEEE floating point numbers, the integer representation tag
 * is used to determine the order of the bytes of the IEEE floating point
 * number.  This means that the bytes are in reverse order if sent from
 * machine that uses little-endian integers as compared to one that uses
 * big-endian integers.
 * 
 *
 * CRAY Floating Point
 * -------------------
 * 
 *     Cray-1, and X-MP's have one hardware floating-point type:  single.  
 *     (95-bit double-precision routines are available in software.) 
 *     
 *     Single is 64 bits with a 1-bit sign, a 15-bit exponent field
 *     (biased by 16384 = 16#4000), and a 48-bit mantissa representing
 *     a fraction in the range of 0.0 (inclusive) to 1.0 (exclusive).
 *
 *                              -2466       +2466
 *                  range   = 10     ..   10
 *                  
 *                  MaxReal = 1.2650140831707E+2466  !! not correct !!
 *
 *     A zero value or an underflow result is not biased and is represented 
 *     as 64 bits of zero.
 *
 *     An overflow result is represented by an exponent >= 24576 (=16#6000).
 *
 * Conversions
 * -----------
 *
 * NDR defines two floating types: single and double.  NDR defines a set
 * of possible representations of these types:  IEEE, VAX, Cray, IBM.  The
 * correspondences between the NDR types and the representations is shown
 * below:
 *
 *       NDR          IEEE     VAX      CRAY     IBM
 *                 +--------+--------+--------+--------+
 *      Single     | single |   F    | single |        |
 *                 +--------+--------+--------+--------+
 *      Double     | double |   G    | single |        |
 *                 +--------+--------+--------+--------+
 *
 * Note that the original VAX architecture defined only F and D, not G or H.
 * Most VAX compilers seem by default to interpret "double precision float"
 * as meaning D format; however, some compilers have an option telling
 * them to interpret it as G.  G seems to be the more "modern" and "popular"
 * flavor of double so that is why NDR uses it; also G corresponds better to
 * IEEE double.
 *
 * NOTE WELL:  The conversion routines as they currently exist are sure to
 * have bugs in the boundary cases.  Note also that the routines that
 * convert to VAX format are likely to work only on a VAX.
 *
 * The following diagrams show how the various representations appear in
 * an NDR byte stream:

    VAX F Floating
    ==============

        |<---- 8 bits ---->|   byte offset      field size
    
        +--+---------------+
        |X2|   FRAC1       |        0              1, 7
        +--+---------------+
        |SN|     X1        |        1              1, 7
        +--+---------------+
        |     FRAC3        |        2              8 
        +------------------+                       
        |     FRAC2        |        3              8
        +------------------+
    
        In terms of the VAX architecture handbook:
        
                X1    = 8:14
                X2    = 7:7
                SN    = 15:15
                FRAC1 = 0:6  
                FRAC2 = 24:31
                FRAC3 = 16:23 
        
        FRACn are the segments of the mantissa.  In increasing order of
        significance they are:  FRAC3, FRAC2, FRAC1.  Xn are the segments
        of the exponent.  In increasing order of significance they are:  X2,
        X1.  SN is the sign bit.


    VAX D Floating (for illustration only)
    ==============

        |<---- 8 bits ---->|   byte offset      field size
    
        +--+---------------+
        |X2|   FRAC1       |        0              1, 7
        +--+---------------+
        |SN|     X1        |        1              1, 7
        +--+---------------+
        |     FRAC3        |        2              8 
        +------------------+                       
        |     FRAC2        |        3              8
        +------------------+
        |     FRAC5        |        4              8
        +------------------+                       
        |     FRAC4        |        5              8
        +------------------+                       
        |     FRAC7        |        6              8
        +------------------+                       
        |     FRAC6        |        7              8
        +------------------+


    VAX G Floating
    ==============

        |<---- 8 bits ---->|    byte offset     field size
    
        +--------+---------+
        |   X2   | FRAC1   |        0              4, 4 
        +--+-----+---------+
        |SN|     X1        |        1              1, 7
        +--+---------------+
        |     FRAC3        |        2              8
        +------------------+        
        |     FRAC2        |        3              8
        +------------------+
        |     FRAC5        |        4              8
        +------------------+                       
        |     FRAC4        |        5              8
        +------------------+                       
        |     FRAC7        |        6              8
        +------------------+                       
        |     FRAC6        |        7              8
        +------------------+
    


    IEEE single
    ===========

            big-endian                         little-endian
                                            
        |<---- 8 bits ---->|                |<---- 8 bits ---->|
                                            
        +--+---------------+                +------------------+
        |SN|    X1         |    0           |       F3         |    0
        +--+---------------+                +------------------+
        |X2|      F1       |    1           |       F2         |    1
        +--+---------------+                +--+---------------+
        |       F2         |    2           |X2|      F1       |    2
        +------------------+                +--+---------------+
        |       F3         |    3           |SN|    X1         |    3
        +------------------+                +--+---------------+
                                            
    IEEE double
    ===========

            big-endian                         little-endian

        |<---- 8 bits ---->|                |<---- 8 bits ---->|
                                            
        +--+---------------+                +------------------+
        |SN|    X1         |    0           |       F7         |    0
        +--+-----+---------+                +------------------+
        |   X2   |   F1    |    1           |       F6         |    1
        +--------+---------+                +------------------+
        |       F2         |    2           |       F5         |    2
        +------------------+                +------------------+
        |       F3         |    3           |       F4         |    3
        +------------------+                +------------------+
        |       F4         |    4           |       F3         |    4
        +------------------+                +------------------+
        |       F5         |    5           |       F2         |    5
        +------------------+                +--+---------------+
        |       F6         |    6           |SN|    X1         |    6
        +------------------+                +--+-----+---------+
        |       F7         |    7           |   X2   |   F1    |    7
        +------------------+                +--------+---------+


    Cray single
    ===========                      
                                     
                                     
        |<---- 8 bits ---->|         
                                     
        +--+---------------+         
        |SN|    X1         |    0    
        +--+-----+---------+         
        |       X2         |    1    
        +--------+---------+         
        |       F1         |    2    
        +------------------+         
        |       F2         |    3    
        +------------------+         
        |       F3         |    4    
        +------------------+         
        |       F4         |    5    
        +------------------+         
        |       F5         |    6            
        +------------------+       
        |       F6         |    7    
        +------------------+         

 */

#include "sysdep.h"
#include "std.h"

#ifdef DSEE
#include "$(rpc.idl).h"
#else
#include "rpc.h"
#endif

#define internal static

#define IEEE_SNAN_SP  0xFFBFFFFF    /* Signalling Not-A-Number */
#define IEEE_SNAN_DP  0xFFF7FFFF    /* Signalling Not-A-Number */
#define VAX_ROP       0x00000000    /* Needs to be fixed */
#ifdef cray
#define CRAY_INDEF    0xFFFFFFFFFFFFFFFF    /* Indefinite value */
#endif

#define IEEE_INF_SP   0x7F800000    /* Infinity */
#define IEEE_INF_DP   0x7FF00000    /* Infinity */
#ifdef cray
#define CRAY_INF      0x6000000000000000    /* Infinity */
#endif

#define IEEE_FLOAT (FLOAT_REP == rpc_$drep_float_ieee)
#define VAX_FLOAT  (FLOAT_REP == rpc_$drep_float_vax)
#define CRAY_FLOAT (FLOAT_REP == rpc_$drep_float_cray)
#define IBM_FLOAT  (FLOAT_REP == rpc_$drep_float_ibm)

#define SWAB_64(dst, src) ( \
    (dst)[0] = (src)[7], \
    (dst)[1] = (src)[6], \
    (dst)[2] = (src)[5], \
    (dst)[3] = (src)[4], \
    (dst)[4] = (src)[3], \
    (dst)[5] = (src)[2], \
    (dst)[6] = (src)[1], \
    (dst)[7] = (src)[0]  \
)

#ifndef cray

#define SWAB_32(dst, src) ( \
    (dst)[0] = (src)[3], \
    (dst)[1] = (src)[2], \
    (dst)[2] = (src)[1], \
    (dst)[3] = (src)[0]  \
)

#define VAXG_BL_SWAP(dst, src) ( \
    (dst)[0] = (src)[1], \
    (dst)[1] = (src)[0], \
    (dst)[2] = (src)[3], \
    (dst)[3] = (src)[2], \
    (dst)[4] = (src)[5], \
    (dst)[5] = (src)[4], \
    (dst)[6] = (src)[7], \
    (dst)[7] = (src)[6]  \
)

#define VAXF_BL_SWAP(dst, src) ( \
    (dst)[0] = (src)[1], \
    (dst)[1] = (src)[0], \
    (dst)[2] = (src)[3], \
    (dst)[3] = (src)[2]  \
)

#else

#define SWAB_32(dst, src) ( \
    (dst)[4] = (src)[7], \
    (dst)[5] = (src)[6], \
    (dst)[6] = (src)[5], \
    (dst)[7] = (src)[4]  \
)

/* #define VAXG_BL_SWAP(dst, src) ... */

/* #define VAXF_BL_SWAP(dst, src) ... */

#endif

#define COPY_64(dst, src) ( \
    *(rpc_$long_float_p_t) (dst) = *(rpc_$long_float_p_t) (src) \
)

#ifndef cray
#define COPY_32(dst, src) ( \
    *(rpc_$short_float_p_t) (dst) = *(rpc_$short_float_p_t) (src) \
)
#endif


/***************************************************************************************************/

#if IEEE_FLOAT

/*
 * VAX -> IEEE conversions
 */

internal int vaxf_to_ieee32(vax_flt, ieee_flt)
    u_char *vax_flt;
    u_long *ieee_flt;
{
    u_short sign = 
        vax_flt[1] >> 7;
    u_short vax_exponent = 
        (vax_flt[0] >> 7) | ((vax_flt[1] & 0x7f) << 1);     /* VAX-biased */
    u_long vax_mantissa = 
        vax_flt[2] | (vax_flt[3] << 8) | ((vax_flt[0] & 0x7f) << 16);
                                                             
    if (vax_exponent == 0)  
        if (sign == 0) 
            *ieee_flt = 0;
        else
            *ieee_flt = IEEE_SNAN_SP;
    else  
        *ieee_flt = (sign << 31) | 
                                                /* VAX-unbias, div 2, IEEE-bias */
                    ((((vax_exponent - 128) - 1) + 127) << 23) |
                    vax_mantissa;
}

internal int vaxg_to_ieee64(vax_flt, ieee_flt)
    u_char *vax_flt;
    u_long *ieee_flt;   
{
    u_short sign = 
        vax_flt[1] >> 7;
    u_short vax_exponent =                                  /* VAX-biased */
        (vax_flt[0] >> 4) | ((vax_flt[1] & 0x7f) << 4);     
    u_long vax_mantissa_msbs =                              /* 20 bits */
        vax_flt[2] | (vax_flt[3] << 8) | ((vax_flt[0] & 0x0f) << 16);  
    u_long vax_mantissa_lsbs =                              /* 32 bits */
        vax_flt[6] | (vax_flt[7] << 8) | (vax_flt[4] << 16) | (vax_flt[5] << 24);  
                                                           
    if (vax_exponent == 0)  
        if (sign == 0) 
            ieee_flt[0] = ieee_flt[1] = 0;
        else
            ieee_flt[0] = ieee_flt[1] = IEEE_SNAN_DP;
    else {
        ieee_flt[0] = (sign << 31) | 
                        ((((vax_exponent - 1024) - 1) + 1023) << 20) |  /* VAX-unbias, div 2, IEEE-bias */
                        vax_mantissa_msbs;
        ieee_flt[1] = vax_mantissa_lsbs;
    }
}

/*
 * CRAY -> IEEE conversions
 */

/*
 * Note 32 bit NDR floats for a cray are represented as IEEE 32 bit values.
 */

internal int cray64_to_ieee64(cray_flt, ieee_flt)
    u_long *ieee_flt;   /* 32 bit entities */
    u_long *cray_flt;
{
    u_long sign = 
         cray_flt[0] & 0x80000000;
    long cray_exponent = 
        ((cray_flt[0] & 0x7fff0000) >> 16) - 16384;  /* 15 bits - CRAY-unbiased */
    u_long cray_mantissa_msb =                       /* 16 bits */
         cray_flt[0] & 0x0000ffff;
    u_long cray_mantissa_lsb =                       /* 32 bits */
         cray_flt[1];

    if (cray_exponent == 0 && cray_mantissa_msb == 0 && cray_mantissa_lsb == 0) { 
        ieee_flt[0] = sign;                     /* cray signed zero */
        ieee_flt[1] = 0;
    } else if (cray_exponent-1 >= 1024) {       /* cray exceeded ieee range */
        ieee_flt[0] = sign | IEEE_INF_DP;       /* overflow => signed infinity */
        ieee_flt[1] = 0;
    } else if (cray_exponent-1 < -1023) {       /* cray exceeded ieee range */
        ieee_flt[0] = sign;                     /* underflow => signed 0 */
        ieee_flt[1] = 0;
    } else {
        ieee_flt[0] =                            /* CRAY-unbiased, div 2, IEEE-bias */
            sign | (((cray_exponent - 1) + 1023) << 20)
                                                 /* room for 20 mantissa bits */
            | ((cray_mantissa_msb & 0x00007fff) << 5)   /* cray msb via exponent */
            | ((cray_mantissa_lsb & 0xf8000000) >> 27); /* remaining 5 */
        ieee_flt[1] =                            /* room for 32 more mantissa bits */
              ((cray_mantissa_lsb & 0x07ffffff) << 5);  /* 5 LSBs left over */
    }
}

#endif

/***************************************************************************************************/

#if VAX_FLOAT

/*
 * IEEE -> VAX conversions
 */

internal int ieee32_to_vaxf(ieee_flt, vax_flt)
    u_char *ieee_flt;
    u_short *vax_flt;
{
    u_short sign = 
        ieee_flt[0] >> 7;
    u_short ieee_exponent = 
        (ieee_flt[1] >> 7) | ((ieee_flt[0] & 0x7f) << 1);   /* IEEE-biased */
    u_long ieee_mantissa = 
        ieee_flt[3] | (ieee_flt[2] << 8) | ((ieee_flt[1] & 0x7f) << 16);
                                                             
    if (ieee_exponent == 0)  
        *(long *) vax_flt = 0;
    else if (ieee_exponent == 0xff)
        *(long *) vax_flt = VAX_ROP;
    else {
        vax_flt[0] = (sign << 15) | 
                                                /* IEEE-unbias, mul 2, VAX-bias */
                        ((((ieee_exponent - 127) + 1) + 128) << 7) |  
                        (ieee_mantissa >> 16);
        vax_flt[1] = ieee_mantissa & 0xffff;
    }
}

internal int ieee64_to_vaxg(ieee_flt, vax_flt)
    u_char *ieee_flt;
    u_short *vax_flt;
{
    u_short sign = 
        ieee_flt[0] >> 7;
    u_short ieee_exponent =                                 /* IEEE-biased */
        (ieee_flt[1] >> 4) | ((ieee_flt[0] & 0x7f) << 4);   
    u_long ieee_mantissa_msb =                              /* 20 bits */
        ieee_flt[3] | (ieee_flt[2] << 8) | ((ieee_flt[1] & 0x0f) << 16);      
    u_long ieee_mantissa_lsb =                              /* 32 bits */
        ieee_flt[7] | (ieee_flt[6] << 8) | (ieee_flt[5] << 16) | (ieee_flt[4] << 24);
                                                             
    if (ieee_exponent == 0)
        vax_flt[0] = vax_flt[1] = vax_flt[2] = vax_flt[3] = 0;
    else if (ieee_exponent == 0xff)
        vax_flt[0] = vax_flt[1] = vax_flt[2] = vax_flt[3] = VAX_ROP;    /* Not quite */
    else {
        vax_flt[0] = (sign << 15) | 
                                                /* IEEE-unbias, mul 2, VAX-bias */
                        ((((ieee_exponent - 1023) + 1) + 1024) << 4) |  
                        (ieee_mantissa_msb >> 16);
        vax_flt[1] = ieee_mantissa_msb & 0xffff;
        vax_flt[2] = ieee_mantissa_lsb >> 16;
        vax_flt[3] = ieee_mantissa_lsb & 0xffff;   
    }
}

#endif

/***************************************************************************************************/

#if CRAY_FLOAT

/*
 * IEEE -> CRAY conversions
 */


/*
 * Note 32 bit NDR floats for a cray are repersented as IEEE 32 bit values.
 */

/*
 * This routine is not "internal" because cray idl_base.h macros need to invoke
 * it directly.
 */
int rpc_$cray64_to_ieee32(cray_flt, ieee_flt)
    u_long *ieee_flt;   /* 64 bit entities */
    u_long *cray_flt;
{
    u_long sign = 
         *cray_flt &  0x8000000000000000;
    long cray_exponent = 
        ((*cray_flt & 0x7fff000000000000) >> 48) - 16384;  /* 15 bits - CRAY-unbiased */
    u_long cray_mantissa =                       /* 48 bits */
         *cray_flt &  0x0000ffffffffffff;

    if (cray_exponent == 0 && cray_mantissa == 0)       /* cray signed zero */
        *ieee_flt = sign >> 32;
    else if (cray_exponent-1 >= 128)                    /* cray exceeded ieee range */
        *ieee_flt = IEEE_INF_SP;      /* overflow => signed infinity */
    else if (cray_exponent-1 < -127)                    /* cray exceeded ieee range */
        *ieee_flt = sign >> 32;       /* underflow => signed 0 */
    else {
        u_long buf;
        buf =                              /* CRAY-unbiased, div 2, IEEE-bias */
            sign | (((cray_exponent - 1) + 127) << (23+32))
                                                 /* room for 23 mantissa bits */
            | ((cray_mantissa & 0x00007fffff000000) << 8);   /* cray msb via exponent */
        *ieee_flt = (buf >> 32) & 0x00000000ffffffff;
    }
}

/*
 * This routine is not "internal" because cray idl_base.h macros need to invoke
 * it directly.
 */
int rpc_$ieee32_to_cray64(ieee_flt, cray_flt)
    u_long *ieee_flt;   /* 64 bit entities */
    u_long *cray_flt;
{
    u_long sign = 
         *ieee_flt & 0x0000000080000000;
    u_long ieee_exponent =                 /* 8 bits - IEEE-biased */
        (*ieee_flt & 0x000000007f800000) >> 23;
    u_long ieee_mantissa =                 /* 23 bits */
         *ieee_flt & 0x00000000007fffff;
                                                             
    if (ieee_exponent == 0 && ieee_mantissa == 0)        /* ieee signed 0 */
        *cray_flt = sign << 32;
    else if (ieee_exponent == 255 && ieee_mantissa == 0) /* ieee signed infinity */
        *cray_flt = sign << 32 | CRAY_INF;
    else if (ieee_exponent == 255 && ieee_mantissa != 0) /* ieee NAN */
        *cray_flt = CRAY_INDEF;
    else
        *cray_flt =                             /* IEEE-unbias, mul 2, CRAY-bias */
            ((sign | ((((ieee_exponent - 127) + 1) + 16384) << 16)) << 32)
            | 0x0000800000000000                    /* insert explicit most-sig-bit */
            | ((ieee_mantissa & 0x007fffff) << (8+16));  /* room for 44 bits; only have 23 */
}

internal int ieee64_to_cray64(ieee_flt, cray_flt)
    u_long *ieee_flt;   /* 64 bit entities */
    u_long *cray_flt;
{
    u_long sign = 
         *ieee_flt & 0x8000000000000000;
    u_long ieee_exponent =                 /* 11 bits - IEEE-biased */
        (*ieee_flt & 0x7ff0000000000000) >> (20+32);
    u_long ieee_mantissa =                 /* 52 bits */
         *ieee_flt & 0x000fffffffffffff;
                                                             
    if (ieee_exponent == 0 && ieee_mantissa == 0)         /* ieee signed zero */
        *cray_flt = sign;
    else if (ieee_exponent == 2047 && ieee_mantissa == 0) /* ieee signed infinity */
        *cray_flt = sign | CRAY_INF;
    else if (ieee_exponent == 2047 && ieee_mantissa != 0) /* ieee NAN */
        *cray_flt = CRAY_INDEF;
    else
        *cray_flt =               /* IEEE-unbias, mul 2, CRAY-bias */
            sign | ((((ieee_exponent - 1023) + 1) + 16384) << (16+32))
            | 0x0000800000000000         /* insert explicit most-sig-bit */
            | ((ieee_mantissa & 0x000fffffffffffe0) >> 5); /* room for 47 bits */
                                                           /* no room for the 5 LSBs */
}

#endif

/***************************************************************************************************/

/*
 * C V T _ F L O A T
 *
 * Internal common routine for "rpc_$cvt_long/short_float".
 */

internal void cvt_float(sdrep, ddrep, srcp, dstp, longf)
    char *srcp;
    char *dstp;
    rpc_$drep_t sdrep;
    rpc_$drep_t ddrep;
    boolean longf;
{
    char buff[8];

    switch (ddrep.float_rep) {

#if IEEE_FLOAT
        case rpc_$drep_float_ieee:
            switch (sdrep.float_rep) {
                case rpc_$drep_float_ieee:
                    if (ddrep.int_rep == sdrep.int_rep)
                        if (longf) COPY_64(dstp, srcp); else COPY_32(dstp, srcp);
                    else
                        if (longf) SWAB_64(dstp, srcp); else SWAB_32(dstp, srcp);
                    break;
    
                case rpc_$drep_float_vax:
                    if (sdrep.int_rep != rpc_$drep_int_little_endian) {
                        if (longf) VAXG_BL_SWAP(buff, srcp); else VAXF_BL_SWAP(buff, srcp);
                        srcp = buff;
                    }
                    if (longf) vaxg_to_ieee64(srcp, dstp); else vaxf_to_ieee32(srcp, dstp);
                    break;
    
                case rpc_$drep_float_cray:
                    /* cray short floats will be in IEEE 32 Bit format */
                    if (sdrep.int_rep != ddrep.int_rep) {
                        if (longf) SWAB_64(buff, srcp); else SWAB_32(buff, srcp);
                        srcp = buff;
                    }
                    if (longf) cray64_to_ieee64(srcp, dstp); else COPY_32(dstp, srcp);
                    break;

                case rpc_$drep_float_ibm:
                    abort();

                default:
                    abort();
            }
            break;
#endif

#if VAX_FLOAT
        case rpc_$drep_float_vax:
            switch (sdrep.float_rep) {
                case rpc_$drep_float_ieee: 
                    if (sdrep.int_rep == rpc_$drep_int_little_endian) {
                        if (longf) SWAB_64(buff, srcp); else SWAB_32(buff, srcp);
                        srcp = buff;
                    }
                    if (longf) ieee64_to_vaxg(srcp, dstp); else ieee32_to_vaxf(srcp, dstp);
                    break; 

                case rpc_$drep_float_vax:
                    if (ddrep.int_rep == sdrep.int_rep)
                        if (longf) COPY_64(dstp, srcp); else COPY_32(dstp, srcp);
                    else
                        if (longf) VAXG_BL_SWAP(dstp, srcp); else VAXF_BL_SWAP(dstp, srcp);
                    break;

                case rpc_$drep_float_cray:
                    abort();

                case rpc_$drep_float_ibm:
                    abort();

                default:
                    abort();
            }
            break;
#endif

#if CRAY_FLOAT
        case rpc_$drep_float_cray:
            switch (sdrep.float_rep) {
                case rpc_$drep_float_ieee: 
                    if (sdrep.int_rep != ddrep.int_rep) {
                        if (longf) SWAB_64(buff, srcp); else SWAB_32(buff, srcp);
                        srcp = buff;
                    }
                    if (longf) ieee64_to_cray64(srcp, dstp); else rpc_$ieee32_to_cray64(srcp, dstp);
                    break; 

                case rpc_$drep_float_vax:
                    abort();

                case rpc_$drep_float_cray:
                    /* cray short floats will be in IEEE 32 Bit format */
                    if (sdrep.int_rep != ddrep.int_rep) {
                        if (longf) SWAB_64(buff, srcp); else SWAB_32(buff, srcp);
                        srcp = buff;
                    }
                    if (longf) COPY_64(dstp, srcp); else rpc_$ieee32_to_cray64(srcp, dstp);
                    break;

                case rpc_$drep_float_ibm:
                    abort();

                default:
                    abort();
            }
            break;
#endif

#ifdef IBM_FLOAT
        case rpc_$drep_float_ibm:
            abort();
#endif

        default:
            abort();
    }
}


/*
 * R P C _ $ C V T _ S H O R T _ F L O A T
 *
 * Convert between short (32 bit) floating point representations.
 */

void rpc_$cvt_short_float(source_drep, dst_drep, srcp, dstp)
    rpc_$drep_t source_drep;
    rpc_$drep_t dst_drep;
    rpc_$short_float_p_t srcp;
    rpc_$short_float_p_t dstp;
{
    cvt_float(source_drep, dst_drep, srcp, dstp, false);
}


/*
 * R P C _ $ C V T _ L O N G _ F L O A T
 *
 * Convert between long (64 bit) floating point representations.
 */

void rpc_$cvt_long_float(source_drep, dst_drep, srcp, dstp)
    rpc_$drep_t source_drep;
    rpc_$drep_t dst_drep;
    rpc_$long_float_p_t srcp;
    rpc_$long_float_p_t dstp;
{
    cvt_float(source_drep, dst_drep, srcp, dstp, true);
}
