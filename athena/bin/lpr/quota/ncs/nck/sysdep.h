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
 * S Y S D E P
 *
 * System-dependent definitions.
 */

/*
 * Define the scalar data representations used.  For possible values
 * see "rpc.idl".
 */

    /*
     * Motorola 680x0, Apollo PRISM, MIPS (not DEC/Ultrix), Ridge, Sun SPARC
     */

#if defined(m68000) || _ISP__A88K || (defined(mips) && !defined(ultrix)) || defined(ridge) || defined(sparc) || defined(ibm032) 
#define INT_REP     rpc_$drep_int_big_endian
#define FLOAT_REP   rpc_$drep_float_ieee
#define CHAR_REP    rpc_$drep_char_ascii
#endif

    /*
     * DEC DECstation (DEC/MIPS)
     */

#if defined(mips) && defined(ultrix)
#define INT_REP     rpc_$drep_int_little_endian
#define FLOAT_REP   rpc_$drep_float_ieee
#define CHAR_REP    rpc_$drep_char_ascii
#endif


    /*
     * DEC VAX
     */

#ifdef vax
#define INT_REP     rpc_$drep_int_little_endian
#define FLOAT_REP   rpc_$drep_float_vax
#define CHAR_REP    rpc_$drep_char_ascii
#endif

    /*
     * Intel 80x86 family
     */

#if defined(i8086) || defined(M_I86) || defined(i386)
#define INT_REP     rpc_$drep_int_little_endian
#define FLOAT_REP   rpc_$drep_float_ieee
#define CHAR_REP    rpc_$drep_char_ascii
#endif

    /*
     * Cray 1, XMP, YMP, Cray 2
     */

#if defined(cray) || defined(CRAY) || defined(CRAY1) || defined(CRAY2)
#define INT_REP     rpc_$drep_int_big_endian
#define FLOAT_REP   rpc_$drep_float_cray
#define CHAR_REP    rpc_$drep_char_ascii
#endif
