/* $Id: array.h,v 1.1 1999-04-23 01:10:11 rbasch Exp $ */

/* Copyright 1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#define BLOCKSIZE 10

typedef struct _Array {
  int size;
  int allocated;
  void **elements;
  int (*compar)(const void *, const void *);
} Array;

Array *array_new(void);
void array_add(Array *, const void *);
void *array_read(const Array *, int);
void array_sort(Array *, int (*)(const void *, const void *));
void *array_search(const Array *, const void *key);
int array_size(const Array *);
void array_free(Array *);
