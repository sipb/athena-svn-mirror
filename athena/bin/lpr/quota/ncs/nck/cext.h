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
 * Useful abbreviations and C language extensions.  (MS/DOS)
 */

#ifndef CEXT_WRAP
#define CEXT_WRAP

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef void (*vproc)();
typedef short (*sproc)();
typedef unsigned short (*usproc)();
typedef long (*lproc)();
typedef char* (*cpproc)();

#define NULLPROC	((vproc) 0)
#define NULL_VPROC	((vproc) 0)
#define NULL_SPROC	((sproc) 0)
#define NULL_LPROC	((lproc) 0)
#define NULL_CPPROC	((cpproc) 0)

#endif /*!CEXT_WRAP*/
