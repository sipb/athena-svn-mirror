/* fontmap.c: read a file for additional font names.

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

#include <kpathsea/config.h>

#include <kpathsea/c-fopen.h>
#include <kpathsea/fontmap.h>
#include <kpathsea/line.h>
#include <kpathsea/pathsearch.h>
#include <kpathsea/str-list.h>


/* Fontname mapping.  We use a straightforward hash table.  */

#define MAP_SIZE 97


/* The hash function.  We go for simplicity here.  */

static unsigned
map_hash P1C(const_string, key)
{
  unsigned n = 0;
  
  /* There are very few font names which are anagrams of each other (I
     think), so no point in weighting the characters.  */
  while (*key != 0)
    n += *key++;
  
  n %= MAP_SIZE;
  
  return n;
}

/* Look up STR in MAP.  Return the corresponding `value' or NULL.  */

static string *
map_lookup_str P2C(map_type, map,  const_string, key)
{
  map_element_type *p;
  str_list_type ret;
  unsigned n = map_hash (key);
  ret = str_list_init ();
  
  /* Look at everything in this bucket.  */
  for (p = map[n]; p != NULL; p = p->next)
    if (STREQ (key, p->key))
      str_list_add (&ret, p->value);
  
  /* If we found anything, mark end of list with null.  */
  if (STR_LIST (ret))
    str_list_add (&ret, NULL);

  return STR_LIST (ret);
}


/* Look up KEY in MAP; if it's not found, remove any suffix from KEY and
   try again.  */

string *
map_lookup P2C(map_type, map,  const_string, key)
{
  string suffix = find_suffix (key);
  string *ret = map_lookup_str (map, key);
  
  if (!ret)
    {
      /* OK, the original KEY didn't work.  Let's check for the KEY without
         an extension -- perhaps they gave foobar.tfm, but the mapping only
         defines `foobar'.  */
      if (suffix)
        {
          string base_key = remove_suffix (key);
          
          ret = map_lookup_str (map, base_key);

          free (base_key);
        }
    }

  /* Append the original suffix, if we had one.  */
  if (ret && suffix)
    while (*ret)
      {
       *ret = extend_filename (*ret, suffix);
       ret++;
      }

  return ret;
}

/* Whether or not KEY is already in MAP, insert it and VALUE.  */

static void
map_insert P3C(map_type, map,  string, key,  string, value)
{
  unsigned n = map_hash (key);
  map_element_type *head = map[n];
  
  map[n] = XTALLOC (MAP_SIZE, map_element_type);
  map[n]->key = xstrdup (key);
  map[n]->value = xstrdup (value);
  map[n]->next = head;
}

/* Open and read the mapping file FILENAME, putting its entries into
   MAP. Comments begin with % and continue to the end of the line.  Each
   line of the file defines an entry: the first word is the real
   filename (e.g., `ptmr'), the second word is the alias (e.g.,
   `Times-Roman'), and any subsequent words are ignored.  .tfm is added
   if either the filename or the alias have no extension.  This is the
   same order as in Dvips' psfonts.map; unfortunately, we can't have TeX
   read that same file, since most of the real filenames start with an
   `r', because of the virtual fonts Dvips uses.  */

static void
map_file_parse P2C(map_type, map,  const_string, map_filename)
{
  extern FILE *xfopen ();	/* In xfopen.c.  */
  char *l;
  unsigned map_lineno = 0;
  FILE *f = xfopen (map_filename, FOPEN_R_MODE);
  
  while ((l = read_line (f)) != NULL)
    {
      string filename;
      string comment_loc = strchr (l, '%');
      
      map_lineno++;
      
      /* Ignore anything after a %.  */
      if (comment_loc)
        *comment_loc = 0;
      
      /* If we don't have any filename, that's ok, the line is blank.  */
      filename = strtok (l, " \t");
      if (filename)
        {
          string alias = strtok (NULL, " \t");
          
          /* But if we have a filename and no alias, something's wrong.  */
          if (alias == NULL || *alias == 0)
            fprintf (stderr, "%s:%u: Alias missing for filename `%s'.\n",
                     map_filename, map_lineno, filename);
          else
            {
              /* We've got everything.  Insert the new entry.  */
              map_insert (map, alias, filename);
            }
        }
      
      free (l);
    }
  
  xfclose (f, map_filename);
}

/* Can't use kpse_path_search, since we need to find all the
   texfonts.map's, not just the first.  Sigh.  */

#define MAP_NAME "texfonts.map"

map_type
map_create P1C(const_string, path)
{
  string *filenames = kpse_all_path_search (path, MAP_NAME);
  map_type map = (map_type) xcalloc (MAP_SIZE, sizeof (map_element_type *));
  
  while (*filenames)
    {
      map_file_parse (map, *filenames);
      filenames++;
    }

  return map;
}
