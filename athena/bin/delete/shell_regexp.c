/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/shell_regexp.c,v $
 * $Author: jik $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_shell_regexp_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/shell_regexp.c,v 1.1 1989-10-23 13:31:15 jik Exp $";
#endif

#include <com_err.h>
#include "shell_regexp.h"
#include "delete_errs.h"
#include "errors.h"

static int real_cmp();

/*
 * This is a simple pattern matcher that takes a pattern string and
 * another string (theoretically a filename) and checks if the second
 * string matches the pattern string using shell special characters
 * (i.e. it recognizes \, ?, *, [, ]).  It also special cases dot
 * files (i.e. * doesn't match files that start with periods, and
 * neither will ?*, and neither will [.]*).
 */

int reg_cmp(pattern, filename)
char *pattern, *filename;
{
     /* First, dot file special cases */
     if ((*filename == '.') && (*pattern != '.'))
	  return REGEXP_NO_MATCH;

     return real_cmp(pattern, filename);
}

static int real_cmp(pattern, filename)
char *pattern, *filename;
{
     if (*pattern == '\0') {
	  if (*filename == '\0')
	       return REGEXP_MATCH;
	  else
	       return REGEXP_NO_MATCH;
     }
     
     if (*pattern == '*') {
	  int retval;
	  char *ptr;
	  
	  if (*(pattern + 1) == '\0')
	       /* asterisk by itself matches anything */
	       return REGEXP_MATCH;
	  for (ptr = filename; *ptr; ptr++)
	       if ((retval = real_cmp(pattern + 1, ptr)) != REGEXP_NO_MATCH)
		    return retval;
	  return REGEXP_NO_MATCH;
     }

     if (*filename == '\0')
	  return REGEXP_NO_MATCH;
     
     if (*pattern == '?')
	  return real_cmp(pattern + 1, filename + 1);

     if (*pattern == '\\') {
	  if (*(pattern + 1) == '\0') {
	       set_error(REGEXP_MISSING_QUOTED_CHAR);
	       return -1;
	  }
	  if (*(pattern + 1) == *filename)
	       return real_cmp(pattern + 2, filename + 1);
	  else
	       return REGEXP_NO_MATCH;
     }

     if (*pattern == '[') {
	  char *ptr, *end_ptr;

	  for (end_ptr = pattern + 1; (*end_ptr != '\0') && (*end_ptr != ']');
	       end_ptr++) ;
	  if (*end_ptr == '\0') {
	       set_error(REGEXP_MISSING_BRACE);
	       return -1;
	  }
	  if (end_ptr == pattern + 1) {
	       set_error(REGEXP_EMPTY_BRACES);
	       return -1;
	  }
	  for (ptr = pattern + 1; ptr < end_ptr; ptr++) {
	       if ((*(ptr + 1) == '-') && (*(ptr + 2) != ']')) {
		    if ((*ptr <= *filename) && (*(ptr + 2) >= *filename))
			 return real_cmp(end_ptr + 1, filename + 1);
		    else {
			 ptr += 2;
			 continue;
		    }
	       }
	       if (*ptr == *filename)
		    return real_cmp(end_ptr + 1, filename + 1);
	  }

	  return REGEXP_NO_MATCH;
     }
		    
     if (*pattern == *filename)
	  return real_cmp(pattern + 1, filename + 1);
     else
	  return REGEXP_NO_MATCH;
}
