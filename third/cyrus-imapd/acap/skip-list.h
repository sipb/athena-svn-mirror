/* skip-list.h -- skiplist API
 *
 * Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact  
 *	Office of Technology Transfer
 *	Carnegie Mellon University
 *	5000 Forbes Avenue
 *	Pittsburgh, PA  15213-3890
 *	(412) 268-4387, fax: (412) 268-7395
 *	tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: skip-list.h,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#ifndef SKIP_LIST_H
#define SKIP_LIST_H

/* this is a simple implementation of skiplists.  it takes void *
   pointers, and a int comp(void *, void *) comparison function. */

#define SKIP_ABSMAXLVL 15

typedef struct Skipnode {
    void *data;
    struct Skipnode *forward[1];
} skipnode;

typedef struct Skiplist skiplist;

/* ml = maximum level of each node
   p = probability of next higher level
   cf = comparator function for elements */
skiplist *skiplist_new(int ml, float p,  
		       int (*cf)(const void *, const void *));
void skiplist_free(skiplist *S);
void skiplist_freeeach(skiplist *S, void (*f)(const void *));

void *ssearch(skiplist *S, const void *k);

void sinsert(skiplist *S, void *k);
void sdelete(skiplist *S, void *k);

int sempty(skiplist *S);
int skiplist_items(skiplist *S);
void sforeach(skiplist *S, void (*f)(const void *));

void *sfirst(skiplist *S, skipnode **ptr);
void *snext(skipnode **ptr);

#endif
