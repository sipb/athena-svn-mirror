/* Copyright 1998 by the Massachusetts Institute of Technology.
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

/* This file implements basic string array functions and structures
 * to be used by software using the athdir library.
 */

static char rcsid[] = "$Id: stringlist.c,v 1.1 1998-03-17 03:58:35 cfields Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stringlist.h"

#define CHUNKSIZE 10

/* sl_add_string
 *   Add a copy of "string" to "list," and to the front of "list" if
 *   "front" is false. If "list" is NULL, initialize it as well.
 *   Can be sl_free()d when no longer in use.
 */
int sl_add_string(string_list **list, char *string, int front)
{
  char *newstring;
  int index;

  /* To add nothing, do nothing. */
  if (string == NULL)
    return 0;

  if (list == NULL)
    return -1;

 /* Initialize the structure if it's NULL. */
  if (*list == NULL)
    {
      *list = malloc(sizeof(string_list));

      if (*list == NULL)
	return -1;

      (*list)->strings = malloc(CHUNKSIZE * sizeof(char *));
      if ((*list)->strings == NULL)
	{
	  free(*list);
	  *list = NULL;
	  return -1;
	}

      (*list)->alloced = CHUNKSIZE;
      (*list)->length = 0;
      (*list)->strings[0] = NULL;
    }

  /* Allocate more list space if the list is full. */
  if ((*list)->alloced == (*list)->length + 1) /* must be NULL terminated */
    {
      (*list)->strings = realloc((*list)->strings,
			      ((*list)->alloced + CHUNKSIZE) * sizeof(char *));
      if ((*list)->strings == NULL)
	{
	  free(*list);
	  return -1;
	}
      (*list)->alloced += CHUNKSIZE;
    }

  /* Make our own copy of the string. */
  newstring = malloc(strlen(string) + 1);
  if (newstring == NULL)
    return -1;
  strcpy(newstring, string);

  /* Add the string to the list. */
  if (front)
    {
      (*list)->length++;
      for (index = (*list)->length; index > 0; index--)
	(*list)->strings[index] = (*list)->strings[index - 1];
      (*list)->strings[0] = newstring;
    }
  else
    {
      (*list)->strings[(*list)->length] = newstring;
      (*list)->length++;
      (*list)->strings[(*list)->length] = NULL;
    }

  return 0;
}

/* sl_remove_string
 *   Remove all instances of "string" from "list."
 */
int sl_remove_string(string_list **list, char *string)
{
  char **ptr, **remove;

  if (list == NULL || *list == NULL)
    return 0;

  for (ptr = (*list)->strings; *ptr != NULL; ptr++)
    {
      if (!strcmp(*ptr, string))
	{
	  free(*ptr);
	  for (remove = ptr; *remove != NULL; remove++)
	    *remove = *(remove + 1);
	  (*list)->length--;
	}
    }

  return 0;
}

/* sl_contains_string
 *   Return 1 if "list" contains "string," 0 if not.
 */
int sl_contains_string(string_list *list, char *string)
{
  char **ptr;

  if (list == NULL)
    return 0;

  for (ptr = list->strings; *ptr != NULL; ptr++)
    {
      if (!strcmp(*ptr, string))
	return 1;
    }

  return 0;
}

/* sl_free
 *   Free "list."
 */
void sl_free(string_list **list)
{
  char **ptr;

  if (list == NULL)
    return;

  if (*list != NULL)
    {
      for (ptr = (*list)->strings; *ptr != NULL; ptr++)
	free(*ptr);
      free((*list)->strings);
      free(*list);
    }

  *list = NULL;
}

/* sl_dump
 *   Print "list" to stdout.
 */ 
void sl_dump(string_list *list)
{
  char **ptr;

  if (list)
    {
      for (ptr = list->strings; *ptr != NULL; ptr++)
	fprintf(stdout, "%s\n", *ptr);
    }
}

/* sl_grab_string
 *   Returns a string made by concatenating all of the strings in
 *   "list," with the "separator" character separating them. '\0'
 *   may be passed as the separator if none is desired. The string
 *   returned can be free()d when no longer in use.
 */
char *sl_grab_string(string_list *list, char separator)
{
  char *return_value, **ptr;
  char sepstr[2];
  int length = 0;

  sepstr[0] = separator;
  sepstr[1] = '\0';

  if (list != NULL && list->strings[0] != NULL)
    {
      /* Count up how much memory we need for the concatenation. */
      for (ptr = list->strings; *ptr != NULL; ptr++)
	length += strlen(*ptr) + 1;

      /* Initialize the string. */
      return_value = malloc(length + 1);
      if (return_value == NULL)
	return NULL;
      return_value[0] = '\0';

      /* Generate the string. */
      for (ptr = list->strings; *ptr != NULL; ptr++)
	{
	  strcat(return_value, *ptr);
	  strcat(return_value, sepstr);
	}

      /* Zap the final string separator. */
      return_value[strlen(return_value) - 1] = '\0';

      return return_value;
    }

  return NULL;
}

/* sl_grab_length
 *   Returns the number of elements in the string array.
 */
int sl_grab_length(string_list *list)
{
  if (list != NULL)
    return list->length;

  return 0;
}

/* sl_grab_string_array
 *   Returns a NULL terminated array of strings (or NULL if the list
 *   is empty). Do not touch or free anything returned by this function;
 *   it's just a pointer to data in the string_list.
 */
char **sl_grab_string_array(string_list *list)
{
  if (list != NULL && list->strings[0] != NULL)
    return list->strings;

  return NULL;
}

/* sl_parse_string
 *   Parse "string" using "sep" as a separator into separate strings, adding
 *   adding each substring to the end of "list." Nondestructive to string,
 *   doesn't point to string.
 */
int sl_parse_string(string_list **list, char *string, char sep)
{
  char *ptr, *sep_ptr;
  char *value;
  int length;

  if (string != NULL)
    {
      /* Allocate a buffer big enough for any substring of string. */
      value = malloc(strlen(string) + 1);
      if (value == NULL)
	return -1;

      ptr = string;
      while (*ptr != '\0')
	{
	  /* Figure out the length of this substring. */
	  sep_ptr = strchr(ptr, sep);
	  if (sep_ptr == NULL)
	    length = strlen(ptr);
	  else
	    length = sep_ptr - ptr;

	  /* Make a copy to be copied by sl_add_string. */
	  strncpy(value, ptr, length);
	  value[length] = '\0';

	  if (sl_add_string(list, value, 0))
	    {
	      free(value);
	      return -1;
	    }

	  ptr += length;
	  if (*ptr == sep)
	    ptr++;
	}

      free(value);
    }

  return 0;
}
