/* global.c 
* 
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto 
* 
* This library is free software; you can redistribute it and/or 
* modify it under the terms of the GNU Library General Public 
* License as published by the Free Software Foundation; either 
* version 2 of the License, or (at your option) any later version. 
* 
* This library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
* Library General Public License for more details. 
* 
* You should have received a copy of the GNU Library General Public 
* License along with this library; if not, write to the 
* Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
* Boston, MA 02111-1307, USA. 
*/ 

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "global.h"

Boolean string_not_equals(const char *string_1, const char *string_2)
{
  return !string_equals(string_1, string_2);
}

Boolean string_equals(const char *string_1, const char *string_2)
{
  if ((string_1 == NULL) && (string_2 == NULL)) return TRUE;
  if ((string_1 == NULL) || (string_2 == NULL)) return FALSE;
  return (strcmp(string_1, string_2) == 0);
}

Boolean string_empty(const char *string)
{
  return string_equals(string, EMPTY_STRING);
}

Boolean string_starts_with(const char *string_1, const char *string_2)
{
  if ((string_1 != NULL) && (string_2 == NULL)) return FALSE;
  if ((string_1 == NULL) && (string_2 == NULL)) return TRUE;
  if (string_empty(string_1) || string_empty(string_2)) return FALSE;
  return (memcmp(string_1, string_2, strlen(string_2)) == 0);
}

Boolean string_ends_with(const char *string_1, const char *string_2)
{
  int length_1;
  int length_2;

  if ((string_1 != NULL) && (string_2 == NULL)) return FALSE;
  if ((string_1 == NULL) && (string_2 == NULL)) return TRUE;
  if (string_empty(string_1) || string_empty(string_2)) return FALSE;
  length_1 = strlen(string_1);
  length_2 = strlen(string_2);
  if (length_2 > length_1) return FALSE;
  return (memcmp(string_1 + (length_1 - length_2), string_2, length_2) == 0);
}

void string_trim(char *string_1)
{
/*  size_t char_size = sizeof(char); */
  char *temp_str_ptr;
  int size = 0;

  if (string_1 == NULL) return;

  temp_str_ptr = string_1;
  while (string_starts_with(temp_str_ptr, " "))
    temp_str_ptr++;
  if (temp_str_ptr != string_1)
  {
    size = strlen(temp_str_ptr);
    memmove(string_1, temp_str_ptr, size);
    string_1[size] = '\0';
  }

  while (string_ends_with(string_1, " ") || string_ends_with(string_1, "\n")|| string_ends_with(string_1, "\r"))
    string_1[strlen(string_1) - 1] = '\0';
}

void *checked_malloc(const size_t size)
{
  void *data;

  data = malloc(size);
  if (data == NULL)
    {
      fprintf(stderr, "\nOut of memory.\n");
      fflush(stderr);
      exit(EXIT_FAILURE);
    }

  return data;
}
