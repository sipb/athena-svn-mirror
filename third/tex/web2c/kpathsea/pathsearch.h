/* pathsearch.h: generic path searching.

Copyright (C) 1993 Karl Berry.

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
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef KPATHSEA_PATHSEARCH_H
#define KPATHSEA_PATHSEARCH_H

#include <kpathsea/c-proto.h>
#include <kpathsea/str-llist.h>
#include <kpathsea/types.h>

/* If PATH is non-null, return its first element (as defined by
   IS_ENV_SEP).  If it's NULL, return the next element in the previous
   path, a la strtok.  Leading, trailing, or doubled colons result in
   the empty string.  When at the end of PATH, return NULL.  In any
   case, return a pointer to an area that may be overwritten on
   subsequent calls.  */
extern string kpse_path_element P1H(const_string path);


/* Given a path element ELT, return a pointer to a NULL-terminated list
   of the corresponding (existing) directory or directories, with
   trailing slashes, or NULL.  If ELT is the empty string, check the
   current working directory.  */
extern str_llist_type *kpse_element_dirs P1H(const_string elt);


/* Expand any leading ~ and variable references in NAME.  If the result
   is an absolute or explicitly relative filename, check whether it is a
   readable (regular) file.  Otherwise, look in each of the directories
   specified in PATH (also do tilde and variable expansion on elements
   in PATH).  In any case, return the complete filename if it
   was found, otherwise NULL.  */
extern string kpse_path_search P2H(const_string path, const_string name);

/* Like kpse_path_search, but return a list of all the filenames (or
   NULL), instead of taking the first.  */
extern string *kpse_all_path_search P2H(const_string path, const_string name);

#endif /* not KPATHSEA_PATHSEARCH_H */
