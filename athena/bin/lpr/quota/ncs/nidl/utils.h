
/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */

#include "ast.h"

/*
 *  exit status codes 
 */
#define pgm_$max_severity    15
#ifdef VMS
#  define pgm_$ok            1
#else
#  define pgm_$ok            0
#endif
#define pgm_$true            0
#define pgm_$false           1
#define pgm_$warning         2
#define pgm_$error           3
#define pgm_$output_invalid  4
#define pgm_$internal_fatal  5
#define pgm_$program_faulted 6

#ifndef __PROTOTYPE
#ifdef __STDC__
#define __PROTOTYPE(x) x
#else
#define __PROTOTYPE(x) ()
#endif
#endif

/*--------------------------------------------------------------------*/
/* a l l o c
 *
 *  Function:   Tries to allocate a block of the specified
 *              size.   If successfull, the block is returned
 *              zeroed, otherwise an error is signalled.
 *
 *  inputs:     size - the number of bytes to allocate.
 *
 *  outputs:
 *
 *  functional value: a pointer to the block.
 *
 */
char *alloc __PROTOTYPE((unsigned int size));

void free __PROTOTYPE((void *ptr));

/*--------------------------------------------------------------------*/
/*  p r i n t _ t y p e _ k i n d
 *
 *  prints the name of type_kind (see ast.h) 'kind' on the stream 'fid'.
 *
 *  inputs:
 *      FILE      *fid;
 *      type_kind  kind;
 *
 */

void print_type_kind __PROTOTYPE((FILE *fid, type_kind kind));
