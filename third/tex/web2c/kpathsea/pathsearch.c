/* pathsearch.c: look up a filename in a path.

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

#include <kpathsea/config.h>

#include <kpathsea/absolute.h>
#include <kpathsea/expand.h>
#include <kpathsea/pathsearch.h>
#include <kpathsea/readable.h>
#include <kpathsea/str-list.h>


/* Concatenate each element in DIRS with NAME (assume each ends with a
   /, to save time).  If SEARCH_ALL is false, return the first readable
   regular file.  Else continue to search for more.  In any case, if
   none, return a list containing just NULL.

   We keep a single buffer for the potential filenames and reallocate
   only when necessary.  I'm not sure it's noticeably faster, but it
   does seem cleaner.  (We do waste a bit of space in the return
   value, though, since we don't shrink it to the final size returned.)  */

#define INIT_ALLOC 64  /* Doesn't much matter what this number is.  */

static str_list_type
dir_list_search P3C(str_llist_type *, dirs,  const_string, name,
                    boolean, search_all)
{
  str_llist_elt_type *elt;
  str_list_type ret;
  unsigned name_len = strlen (name);
  unsigned allocated = INIT_ALLOC;
  string potential = xmalloc (allocated);

  ret = str_list_init ();
  
  for (elt = *dirs; elt; elt = STR_LLIST_NEXT (*elt))
    {
      const_string dir = STR_LLIST (*elt);
      unsigned dir_len = strlen (dir);
      
      while (dir_len + name_len + 1 > allocated)
        {
          allocated += allocated;
          XRETALLOC (potential, allocated, char);
        }
      
      strcpy (potential, dir);
      strcat (potential, name);
      
      if (kpse_readable_file (potential))
        { 
          str_list_add (&ret, potential);
          
          /* Move this element towards the top of the list.  */
          str_llist_float (dirs, elt);
          
          /* If caller only wanted one file returned, no need to
             terminate the list with NULL; the caller knows to only look
             at the first element.  */
          if (!search_all)
            return ret;

          /* Start new filename.  */
          allocated = INIT_ALLOC;
          potential = xmalloc (allocated);
        }
    }
  
  /* If we get here, either we didn't find any files, or we were finding
     all the files.  But we're done with the last filename, anyway.  */
  free (potential);
  
  return ret;
}

/* This is called when NAME is absolute or explicitly relative; if it's
   readable, return (a list containing) it; otherwise, return NULL.  */

static str_list_type
absolute_search P1C(string, name)
{
  str_list_type ret_list;
  string found = kpse_readable_file (name);
  
  /* Old compilers can't initialize struct's.  */
  ret_list = str_list_init ();

  /* If NAME wasn't found, free the expansion.  */
  if (name != found)
    free (name);

  /* Add `found' to the return list even if it's null; that tells
     the caller we didn't find anything.  */
  str_list_add (&ret_list, found);
  
  return ret_list;
}


/* This is the hard case -- look for NAME in PATH.  If
   ALL is false, just return the first file found.  Otherwise,
   search all elements of PATH.  */

static str_list_type
path_search P3C(const_string, path,  string, name,  boolean, all)
{
  string elt;
  str_list_type ret_list;
  boolean done = false;
  ret_list = str_list_init ();

  for (elt = kpse_path_element (path); !done && elt != NULL;
       elt = kpse_path_element (NULL))
    {
      str_llist_type *dirs = kpse_element_dirs (elt);

      if (dirs && *dirs)
        {
          /* Search for `name' in `dirs'.  */
          str_list_type found;  /* old compilers can't init structs */
          found = dir_list_search (dirs, name, all);

          /* Did we find anything?  */
          if (STR_LIST (found))
            if (all)
              str_list_concat (&ret_list, found);
            else
              {
                str_list_add (&ret_list, STR_LIST_ELT (found, 0));
                done = true;
              }
        }
    }

  /* Free the expanded name we were passed.  It can't be in the return
     list, since the path directories got unconditionally prepended.  */
  free (name);
  
  return ret_list;
}      

/* Search PATH for ORIGINAL_NAME.  If ALL is false, or
   ORIGINAL_NAME is absolute_p, check
   ORIGINAL_NAME itself.  Otherwise, look at each element of PATH for
   the first readable ORIGINAL_NAME.
   
   Always return a list; if no files are found, the list will
   contain just NULL.  If ALL is true, the list will be
   terminated with NULL.  */

static string *
search P3C(const_string, path,  const_string, original_name, boolean, all)
{
  str_list_type ret_list;

  /* Make a leading ~ count as an absolute filename, and expand $FOO's.  */
  string name = kpse_expand (original_name);
  
  /* If the first name is absolute or explicitly relative, no need to
     consider PATH at all.  */
  boolean absolute_p = kpse_absolute_p (name);
  
  /* Find the file(s). */
  ret_list = absolute_p ? absolute_search (name)
                        : path_search (path, name, all);
  
  /* Append NULL terminator if we didn't find anything at all, or we're
     supposed to find ALL and the list doesn't end in NULL now.  */
  if (STR_LIST_LENGTH (ret_list) == 0
      || (all && STR_LIST_LAST_ELT (ret_list) != NULL))
    str_list_add (&ret_list, NULL);

  return STR_LIST (ret_list);
}

/* Search PATH for the first NAME.  */

string
kpse_path_search P2C(const_string, path,  const_string, name)
{
  string *ret_list = search (path, name, false);
  return *ret_list;
}


/* Search all elements of PATH for files named NAME.  */

string *
kpse_all_path_search P2C(const_string, path,  const_string, name)
{
  string *ret = search (path, name, true);
  return ret;
}

#ifdef TEST

void
test_path_search (const_string path, const_string file)
{
  string answer;
  string *answer_list;
  
  printf ("\nSearch %s for %s:\t", path, file);
  answer = kpse_path_search (path, file);
  puts (answer ? answer : "(null)");

  printf ("Search %s for all %s:\t", path, file);
  answer_list = kpse_all_path_search (path, file);
  putchar ('\n');
  while (*answer_list)
    {
      putchar ('\t');
      puts (*answer_list);
      answer_list++;
    }
}

#define TEXFONTS "/usr/local/lib/tex/fonts"

int
main ()
{
  /* All lists end with NULL.  */
  test_path_search (".", "nonexistent");
  test_path_search (".", "/nonexistent");
  test_path_search ("/k:.", "kpathsea.texi");
  test_path_search ("/k:.", "/etc/fstab");
  test_path_search (".:" TEXFONTS "//", "cmr10.tfm");
  test_path_search (".:" TEXFONTS "//", "logo10.tfm");
  test_path_search (TEXFONTS "//times:.::", "ptmr.vf");
  test_path_search (TEXFONTS "/adobe//:"
                    "/usr/local/src/TeX+MF/typefaces//", "plcr.pfa");
  
  test_path_search ("~karl", ".bashrc");
  test_path_search ("/k", "~karl/.bashrc");

  xputenv ("NONEXIST", "nonexistent");
  test_path_search (".", "$NONEXISTENT");
  xputenv ("KPATHSEA", "kpathsea");
  test_path_search ("/k:.", "$KPATHSEA.texi");  
  test_path_search ("/k:.", "${KPATHSEA}.texi");  
  test_path_search ("$KPATHSEA:.", "README");  
  test_path_search (".:$KPATHSEA", "README");  
  
  return 0;
}

#endif /* TEST */


/*
Local variables:
test-compile-command: "gcc -posix -g -I. -I.. -DTEST pathsearch.c kpathsea.a"
End:
*/
