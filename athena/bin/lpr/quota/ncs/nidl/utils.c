/********************************************************************/
/*                                                                  */
/*                       UTILS.C                                    */
/*                                                                  */
/*          IDC Miscellaneous utility routines                      */
/*                                                                  */
/********************************************************************/

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



#include <stdio.h>
#include "idl_base.h"
#include "utils.h"
#include "errors.h"

/*--------------------------------------------------------------------*/

/*  Function:   Tries to allocate a block of the specified
 *              size.   If successfull, the block is returned
 *              zeroed, otherwise an error is signalled.
 *
 *  inputs:     size - the number of bytes to allocate.
 *
 *  outputs:
 *
 *  functional value: a pointer to the
 *
 */

extern void exit __PROTOTYPE ((int));

#ifdef MSDOS
#include <malloc.h>
#else
extern char   *calloc __PROTOTYPE ((unsigned int, unsigned int));
#endif
    
char   *alloc (size)
unsigned int    size;
{
        char   *p;

#if defined(MSDOS) && defined(HEAPDEBUG)
            int heapstatus;

            heapstatus = _heapset(0x6969);
            if ((heapstatus != _HEAPOK) && (heapstatus != _HEAPEMPTY)) {
           fprintf(stderr, ":-( heap trashed, status %d\n", heapstatus);
                   abort();
            }
#endif
        p = (char *) calloc (1,size);
        if (!p) {
                error ("Out of memory\n");
                exit (-1);
        };

        return p;
}


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
void print_type_kind (fid, kind)
FILE *fid;
type_kind kind;
{
    switch (kind) {

        /* alphabetized list of type kind cases */
        case boolean_k:
            fprintf(fid, "boolean_k");
            break;
        case byte_k:
            fprintf(fid, "byte_k");
            break;
        case character_k:
            fprintf(fid, "character_k");
            break;
        case fixed_array_k:
            fprintf(fid, "fixed_array_k");
            break;
        case fixed_string_zero_k:
            fprintf(fid, "fixed_string_zero_k");
            break;
        case handle_k:
            fprintf(fid, "handle_k");
            break;
        case hyper_integer_k:
            fprintf(fid, "hyper_integer_k");
            break;
        case hyper_unsigned_k:
            fprintf(fid, "hyper_unsigned_k");
            break;
        case long_bitset_k:
            fprintf(fid, "long_bitset_k");
            break;
        case long_enumeration_k:
            fprintf(fid, "long_enumeration_k");
            break;
        case long_integer_k:
            fprintf(fid, "long_integer_k");
            break;
        case long_real_k:
            fprintf(fid, "long_real_k");
            break;
        case long_subrange_k:
            fprintf(fid, "long_subrange_k");
            break;
        case long_unsigned_k:
            fprintf(fid, "long_unsigned_k");
            break;
        case named_k:
            fprintf(fid, "named_k");
            break;
        case open_array_k:
            fprintf(fid, "open_array_k");
            break;
        case open_record_k:
            fprintf(fid, "open_record_k");
            break;
        case open_string_zero_k:
            fprintf(fid, "open_string_zero_k");
            break;
        case pointer_k:
            fprintf(fid, "pointer_k");
            break;
        case record_k:
            fprintf(fid, "record_k");
            break;
        case routine_ptr_k:
            fprintf(fid, "routine_ptr_k");
            break;
        case short_bitset_k:
            fprintf(fid, "short_bitset_k");
            break;
        case short_enumeration_k:
            fprintf(fid, "short_enumeration_k");
            break;
        case short_integer_k:
            fprintf(fid, "short_integer_k");
            break;
        case short_real_k:
            fprintf(fid, "short_real_k");
            break;
        case short_subrange_k:
            fprintf(fid, "short_subrange_k");
            break;
        case short_unsigned_k:
            fprintf(fid, "short_unsigned_k");
            break;
        case small_bitset_k:
            fprintf(fid, "small_bitset_k");
            break;
        case small_enumeration_k:
            fprintf(fid, "small_enumeration_k");
            break;
        case small_integer_k:
            fprintf(fid, "small_integer_k");
            break;
        case small_subrange_k:
            fprintf(fid, "small_subrange_k");
            break;
        case small_unsigned_k:
            fprintf(fid, "small_unsigned_k");
            break;
        case user_marshalled_k:
            fprintf(fid, "user_marshalled_k");
            break;
        case void_k:
            fprintf(fid, "void_k");
            break;
        default:
            fprintf(fid, "unknown");
        }
}



