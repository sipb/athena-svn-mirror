/*  balanced_trees.h, /us/lib/ddslib/src, pato, 05/26/88
**      Symbol table interface
**
** ========================================================================== 
** Confidential and Proprietary.  Copyright 1988 by Apollo Computer Inc.,
** Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
** Copyright Laws Of The United States.
** 
** Apollo Computer Inc. reserves all rights, title and interest with respect
** to copying, modification or the distribution of such software programs
** and associated documentation, except those rights specifically granted
** by Apollo in a Product Software Program License, Source Code License
** or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
** Apollo and Licensee.  Without such license agreements, such software
** programs may not be used, copied, modified or distributed in source
** or object code form.  Further, the copyright notice must appear on the
** media, the supporting documentation and packaging as set forth in such
** agreements.  Such License Agreements do not grant any rights to use
** Apollo Computer's name or trademarks in advertising or publicity, with
** respect to the distribution of the software programs without the specific
** prior written permission of Apollo.  Trademark agreements may be obtained
** in a separate Trademark License Agreement.
** ========================================================================== 
**
*/

#ifdef DSEE
#   include "$(nbase.idl).h"
#else
#   include "nbase.h"
#endif

typedef struct sym_$datum {
    char    *dptr;
    int     dsize;
} sym_$datum_t;

typedef struct sym_$tree_root * sym_$tree_handle;

long sym_$insert_data (
#ifdef __STDC__
    long             replace_ok,
    sym_$tree_handle *tree,
    sym_$datum_t     *new_key,
    sym_$datum_t     *new_data
#endif
);

sym_$datum_t * sym_$fetch_data (
#ifdef __STDC__
    sym_$tree_handle tree,
    sym_$datum_t     *search_key
#endif
);

sym_$datum_t * sym_$fetch_next (
#ifdef __STDC__
    sym_$tree_handle tree,
    sym_$datum_t     *search_key,
    sym_$datum_t     **next_key
#endif
);

long sym_$delete_data (
#ifdef __STDC__
    sym_$tree_handle tree,
    sym_$datum_t     *search_key
#endif
);

void sym_$clear_database (
#ifdef __STDC__
    sym_$tree_handle *tree 
#endif
);
