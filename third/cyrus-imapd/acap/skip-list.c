/* skip-list.c -- generic skip list routines
 *
 * Copyright (c) 1998, 2000 Carnegie Mellon University.
 * All rights reserved.
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

/* $Id: skip-list.c,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include "skip-list.h"

struct Skiplist {
    int maxlevel;
    float prob;
    int curlevel;
    int items;
    int (*comp)(const void *, const void *);
    skipnode *header; /* phantom first element of skiplist */
#if 0
    skipnode *finger[SKIP_ABSMAXLVL]; /* finger for localization */
#endif
};

static int expensive_debug = 0;

skiplist *skiplist_new(int ml, float p, int (*cf)(const void *, const void *))
{
    skiplist *s = (skiplist *) malloc(sizeof(skiplist));
    int i;

    assert(s);

    if (ml > SKIP_ABSMAXLVL) 
	ml = SKIP_ABSMAXLVL;
    s->maxlevel = ml;
    s->prob = p;
    s->comp = cf;
    s->items = 0;

    s->header = (skipnode *) malloc(sizeof(skipnode) + 
				    ml * sizeof(skipnode *));

    assert(s->header);
    s->header->data = NULL;
    for (i = 0; i < ml; i++)
	s->header->forward[i] = NULL;

    s->curlevel = 0;

    return s;
}

static void invariant(skiplist *S)
{
    skipnode *x = S->header->forward[0];

    if (!x) {
	assert(S->items == 0);
	return;
    }
    if (!expensive_debug) return;
    while (x->forward[0]) {
	assert(S->comp(x->data, x->forward[0]->data) < 0);
	x = x->forward[0];
    }
}

void *ssearch(skiplist *S, const void *k)
{
    skipnode *x = S->header;
    int i;

    assert(S && k);

    /* invariant: x < k */
    for (i = S->curlevel; i >= 0; i--) {
	while (x->forward[i] && x->forward[i]->data &&
	       (S->comp(x->forward[i]->data, k) < 0)) {
	    x = x->forward[i];
	}
    }
    x = x->forward[0];
    if ((x != NULL) && (S->comp(x->data, k) == 0))
	return x->data;
    else
	return NULL;
}

int randLevel(skiplist *S)
{
    int lvl = 0;
    
    assert(S);
    while ((((float) rand() / (float) (RAND_MAX)) < S->prob) 
	   && (lvl < S->maxlevel))
	lvl++;
    return lvl;
}

void sinsert(skiplist *S, void *k)
{
    skipnode *update[SKIP_ABSMAXLVL];
    int newlvl = randLevel(S), i;
    skipnode *x = S->header, 
	*new = (skipnode *) malloc(sizeof(skipnode) +
				   newlvl * sizeof(skipnode *));

    assert(S && k && new && x);
    invariant(S);
    for (i = S->curlevel; i >= 0; i--) {
	while (x->forward[i] && S->comp(x->forward[i]->data, k) < 0)
	    x = x->forward[i];
	update[i] = x;
    }
    x = x->forward[0];
    if ((x != NULL) && (S->comp(x->data, k) == 0)) {
	x->data = k;
	free(new);
    } else {
	S->items++;
	if (newlvl > S->curlevel)
	    for (i = S->curlevel + 1; i <= newlvl; i++)
		update[i] = S->header;
	new->data = k;
	for (i = 0; i <= newlvl; i++) {
	    new->forward[i] = update[i]->forward[i];
	    update[i]->forward[i] = new;
	}
    }

    invariant(S);
}

void sdelete(skiplist *S, void *k)
{
    skipnode *update[SKIP_ABSMAXLVL];
    skipnode *x = S->header;
    int i;

    assert(S && k);
    invariant(S);
    for (i = S->curlevel; i >= 0; i--) {
	while (x->forward[i] && S->comp(x->forward[i]->data, k) < 0)
	    x = x->forward[i];
	update[i] = x;
    }
    x = x->forward[0];
    if ((x != NULL) && (S->comp(x->data, k) == 0)) {
	for (i = 0; i <= S->curlevel; i++) {
	    if (update[i]->forward[i] != x) break;
	    update[i]->forward[i] = x->forward[i];
	}
	free(x);
	S->items--;
	while ((S->curlevel >= 1) && (S->header->forward[S->curlevel] == NULL))
	    S->curlevel--;
    }

    invariant(S);
    /* if it isn't here, do nothing */
}

void skiplist_free(skiplist *S)
{
    skipnode *x = S->header, *y;

    assert(S);
    invariant(S);

    while (x != NULL) {
	y = x->forward[0];
	free(x);
	x = y;
    }
    free(S);
}

/* free the skiplist, calling 'f' on each data pointer */
void skiplist_freeeach(skiplist *S, void (*f)(const void *))
{
    skipnode *x, *y;

    assert(S);
    invariant(S);
    
    x = S->header->forward[0];
    free(S->header);
    while (x != NULL) {
	f(x->data);
	y = x->forward[0];
	free(x);
	x = y;
    }
    free(S);
}

int sempty(skiplist *S)
{
    assert(S);
    return (S->items == 0);
}

int skiplist_items(skiplist *S)
{
    assert(S);
    return S->items;
}

void sforeach(skiplist *S, void (*f)(const void *))
{
    skipnode *s;

    assert(S);
    s = S->header->forward[0];
    while (s != NULL) {
	f(s->data);
	s = s->forward[0];
    }
}

void *sfirst(skiplist *S, skipnode **ptr)
{
    assert(S && ptr);
    *ptr = S->header->forward[0];
    if (*ptr != NULL) {
	return (*ptr)->data;
    } else {
	return NULL;
    }
}

void *snext(skipnode **ptr)
{
    assert(ptr);
    if (*ptr != NULL) {
	*ptr = (*ptr)->forward[0];
    }
    if (*ptr != NULL) {
	return (*ptr)->data;
    } else {
	return NULL;
    }
}
