/* GNU gettext - internationalization aids
   Copyright (C) 1995, 1996, 1998, 2000, 2001 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _STR_LIST_H
#define _STR_LIST_H 1

/* Get size_t and NULL.  */
#include <stddef.h>

/* Get bool.  */
#include <stdbool.h>

/* Type describing list of immutable strings,
   implemented using a dynamic array.  */
typedef struct string_list_ty string_list_ty;
struct string_list_ty
{
  const char **item;
  size_t nitems;
  size_t nitems_max;
};

/* Initialize an empty list of strings.  */
extern void string_list_init PARAMS ((string_list_ty *slp));

/* Return a fresh, empty list of strings.  */
extern string_list_ty *string_list_alloc PARAMS ((void));

/* Append a single string to the end of a list of strings.  */
extern void string_list_append PARAMS ((string_list_ty *slp, const char *s));

/* Append a single string to the end of a list of strings, unless it is
   already contained in the list.  */
extern void string_list_append_unique PARAMS ((string_list_ty *slp,
					       const char *s));

/* Destroy a list of strings.  */
extern void string_list_destroy PARAMS ((string_list_ty *slp));

/* Free a list of strings.  */
extern void string_list_free PARAMS ((string_list_ty *slp));

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list.  */
extern char *string_list_concat PARAMS ((const string_list_ty *slp));

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, and destroy the list.  */
extern char *string_list_concat_destroy PARAMS ((string_list_ty *slp));

/* Return a freshly allocated string obtained by concatenating all the
   strings in the list, separated by spaces.  */
extern char *string_list_join PARAMS ((const string_list_ty *slp));

/* Return 1 if s is contained in the list of strings, 0 otherwise.  */
extern bool string_list_member PARAMS ((const string_list_ty *slp,
					const char *s));

#endif /* _STR_LIST_H */
