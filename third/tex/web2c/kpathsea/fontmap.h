/* fontmap.h: declarations for reading a file to define additional font names.

Copyright (C) 1993 Free Software Foundation, Inc.

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

#ifndef FONTMAP_H
#define FONTMAP_H

#include <kpathsea/c-proto.h>
#include <kpathsea/types.h>


typedef struct map_element_struct
{
  string key;
  string value;
  struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type;


/* Parse the file `texfonts.map' in each of the directories in PATH and
   return the resulting structure.  Entries in earlier files override
   later files.  */

extern map_type map_create P1H(const_string path);


/* Look up KEY in MAP, and return a null-terminated list of all matching
   entries.  If none, return NULL.  */
extern string *map_lookup P2H(map_type map, const_string key);

#endif /* not FONTMAP_H */
