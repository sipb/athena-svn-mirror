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

static const char rcsid[] = "$Id: array.c,v 1.1 2000-06-19 03:53:32 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include "array.h"

extern char *progname;

Array *array_new()
{
  Array *array;

  array = malloc(sizeof(Array));
  if (array == NULL)
    {
      fprintf(stderr, "%s: out of memory\n", progname);
      exit(1);
    }

  array->elements = malloc(BLOCKSIZE * sizeof(void *));
  if (array->elements == NULL)
    {
      fprintf(stderr, "%s: out of memory\n", progname);
      exit(1);
    }

  array->allocated = BLOCKSIZE;
  array->size = 0;
  array->compar = NULL;

  return array;
}

void array_add(Array *array, const void *element)
{
  if (array->size == array->allocated)
    {
      array->allocated += BLOCKSIZE;
      array->elements = realloc(array->elements,
				sizeof (void *) * array->allocated);
      if (array->elements == NULL)
	{
	  fprintf(stderr, "%s: out of memory\n", progname);
	  exit(1);
	}
    }

  array->elements[array->size] = (void *) element;
  array->size++;
}

void *array_read(const Array *array, int index)
{
  if (index < array->size)
    return array->elements[index];

  return NULL;
}

/* Set the sort comparison function for an array, and sort it. */
void array_sort(Array *array, int (*compar)(const void *, const void *))
{
  qsort(array->elements, array->size, sizeof(void *), compar);
  array->compar = compar;
  return;
}

/* Search a sorted array for the given key, returning a pointer to
 * the matching element.  Returns NULL if not found, or if the
 * array was not previously sorted via ArraySort().  It is the
 * caller's responsibility to ensure that the array is sorted.
 */
void *array_search(const Array *array, const void *key)
{
  if (array->compar == NULL)
    return NULL;

  return bsearch(&key, array->elements, array->size, sizeof(void *),
		 array->compar);
}

int array_size(const Array *array)
{
  return array->size;
}

void array_free(Array *array)
{
  free(array->elements);
  free(array);
}
