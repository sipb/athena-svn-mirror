/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.c,v $
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
     static char rcsid_pattern_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.c,v 1.11 1989-10-23 13:33:06 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>
#include <com_err.h>
#include "directories.h"
#include "pattern.h"
#include "util.h"
#include "undelete.h"
#include "shell_regexp.h"
#include "mit-copyright.h"
#include "delete_errs.h"
#include "errors.h"

extern char *malloc(), *realloc();
extern int errno;

extern char *whoami;



/*
 * add_arrays() takes pointers to two arrays of char **'s and their
 * lengths, merges the two into the first by realloc'ing the first and
 * then free's the second's memory usage.
 */  
int add_arrays(array1, num1, array2, num2)
char ***array1, ***array2;
int *num1, *num2;
{
     int counter;
     
     *array1 = (char **) realloc((char *) *array1, (unsigned)
				 (sizeof(char *) * (*num1 + *num2)));
     if (! *array1) {
	  set_error(errno);
	  error("realloc");
	  return error_code;
     }
     for (counter = *num1; counter < *num1 + *num2; counter++)
	  *(*array1 + counter) = *(*array2 + counter - *num1);
     free ((char *) *array2);
     *num1 += *num2;
     return 0;
}






/*
 * Add a string to a list of strings.
 */
int add_str(strs, num, str)
char ***strs;
int num;
char *str;
{
     char **ary;

     ary = *strs = (char **) realloc((char *) *strs, (unsigned)
				     (sizeof(char *) * (num + 1)));
     if (! strs) {
	  set_error(errno);
	  error("realloc");
	  return error_code;
     }
     ary[num] = malloc((unsigned) (strlen(str) + 1));
     if (! ary[num]) {
	  set_error(errno);
	  error("malloc");
	  return error_code;
     }
     (void) strcpy(ary[num], str);
     return 0;
}






int find_deleted_matches(base, expression, num_found, found)
char *base, *expression;
int *num_found;
char ***found;
{
     struct direct *dp;
     DIR *dirp;
     int num;
     char **next;
     int num_next;
     char first[MAXNAMLEN], rest[MAXPATHLEN];
     char new[MAXPATHLEN];
     int retval;
     
#ifdef DEBUG
     printf("Looking for %s in %s\n", expression, base);
#endif
     *found = (char **) malloc(0);
     if (! *found) {
	  set_error(errno);
	  error("malloc");
	  return error_code;
     }
     num = 0;

     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }

     (void) strcpy(first, firstpart(expression, rest));
     
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if ((retval = reg_cmp(first, dp->d_name)) < 0) {
	       error("reg_cmp");
	       return error_code;
	  }
          if ((retval == REGEXP_MATCH) && *rest) {
	       (void) strcpy(new, append(base, dp->d_name));
	       if (retval = find_deleted_matches(new, rest, &num_next,
						 &next)) {
		    error("find_deleted_matches");
		    closedir(dirp);
		    return retval;
	       }
	       if (retval = add_arrays(found, &num, &next, &num_next)) {
		    error("add_arrays");
		    closedir(dirp);
		    return retval;
	       }
	  }
	  else if (is_deleted(dp->d_name)) {
	       if ((retval = reg_cmp(first, &dp->d_name[2])) < 0) {
		    error("reg_cmp");
		    closedir(dirp);
		    return retval;
	       }
	       if (retval == REGEXP_MATCH) {
		    if (*rest) {
			 (void) strcpy(new, append(base, dp->d_name));
			 if (retval = find_deleted_matches(new, rest,
							   &num_next, &next)) {
			      error("find_deleted_matches");
			      closedir(dirp);
			      return retval;
			 }
			 if (retval = add_arrays(found, &num, &next,
						 &num_next)) {
			      error("add_arrays");
			      closedir(dirp);
			      return retval;
			 }
		    }
		    else {
			 char *tmp;

			 tmp = append(base, dp->d_name);
			 if (! *tmp) {
			      error("append");
			      return error_code;
			 }
			 if (retval = add_str(found, num, tmp)) {
			      error("add_str");
			      closedir(dirp);
			      return retval;
			 }
			 num++;
		    }
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;
     return 0;
}





int find_matches(base, expression, num_found, found)
char *base, *expression;
int *num_found;
char ***found;
{
     struct direct *dp;
     DIR *dirp;
     int num;
     char **next;
     int num_next;
     char first[MAXNAMLEN], rest[MAXPATHLEN];
     char new[MAXPATHLEN];
     int retval;
     
#ifdef DEBUG
     printf("Looking for %s in %s\n", expression, base);
#endif
     *found = (char **) malloc(0);
     num = 0;

     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }

     (void) strcpy(first, firstpart(expression, rest));
     
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  retval = reg_cmp(first, dp->d_name);
	  if (retval < 0) {
	       error("reg_cmp");
	       closedir(dirp);
	       return retval;
	  }
          if (retval == REGEXP_MATCH) {
	       if (*rest) {
		    char *tmp;

		    tmp = append(base, dp->d_name);
		    if (! *tmp) {
			 error("append");
			 return error_code;
		    }
		    (void) strcpy(new, tmp);
		    if (retval = find_matches(new, rest, &num_next, &next)) {
			 error("find_matches");
			 closedir(dirp);
			 return retval;
		    }
		    if (retval = add_arrays(found, &num, &next, &num_next)) {
			 error("add_arrays");
			 closedir(dirp);
			 return retval;
		    }
	       }
	       else {
		    char *tmp;

		    tmp = append(base, dp->d_name);
		    if (! *tmp) {
			 error("append");
			 return error_code;
		    }
		    
		    if (retval = add_str(found, num, tmp)) {
			 error("add_str");
			 closedir(dirp);
			 return retval;
		    }
		    num++;
	       }
	  }
	  else if (is_deleted(dp->d_name)) {
	       if ((retval = reg_cmp(first, &dp->d_name[2])) < 0) {
		    error("reg_cmp");
		    closedir(dirp);
		    return retval;
	       }
	       if (retval == REGEXP_MATCH) {
		    if (*rest) {
			 (void) strcpy(new, append(base, dp->d_name));
			 if (! *new) {
			      error("append");
			      return error_code;
			 }
			 if (retval = find_matches(new, rest, &num_next,
						   &next)) {
			      error("find_matches");
			      closedir(dirp);
			      return retval;
			 }
			 if (retval = add_arrays(found, &num, &next,
						 &num_next)) {
			      error("add_arrays");
			      closedir(dirp);
			      return retval;
			 }
		    }
		    else {
			 char *tmp;

			 tmp = append(base, dp->d_name);
			 if (! *tmp) {
			      error("append");
			      return error_code;
			 }
			 if (retval = add_str(found, num, tmp)) {
			      error("add_str");
			      closedir(dirp);
			      return 0;
			 }
			 num++;
		    }
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;

     return 0;
}







int find_recurses(base, num_found, found)
char *base;
int *num_found;
char ***found;
{
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     char **new_found;
     int found_num, new_found_num;
     struct stat stat_buf;
     int retval;
     
#ifdef DEBUG
     printf("Looking for subs of %s\n", base);
#endif
     *found = (char **) malloc(0);
     *num_found = found_num = 0;
     
     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  (void) strcpy(newname, append(base, dp->d_name));
	  if (! *newname) {
	       error("append");
	       return error_code;
	  }
	  if (retval = add_str(found, found_num, newname)) {
	       error("add_str");
	       closedir(dirp);
	       return retval;
	  }
	  found_num++;
	  if (lstat(newname, &stat_buf)) {
	       set_error(errno);
	       error(newname);
	       continue;
	  }
	  if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
	       if (retval = find_recurses(newname, &new_found_num,
					  &new_found)) {
		    error(newname);
		    closedir(dirp);
		    return retval;
	       }
	       if (retval = add_arrays(found, &found_num, &new_found,
				       &new_found_num)) {
		    error("add_arrays");
		    closedir(dirp);
		    return retval;
	       }
	  }
     }
     closedir(dirp);
     *num_found = found_num;
     return 0;
}






int find_deleted_recurses(base, num_found, found)
char *base;
int *num_found;
char ***found;
{
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     char **new_found;
     int found_num, new_found_num;
     struct stat stat_buf;
     int retval;
     
#ifdef DEBUG
     printf("Looking for deleted recurses of %s\n", base);
#endif
     *found = (char **) malloc(0);
     *num_found = found_num = 0;
     
     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;

	  (void) strcpy(newname, append(base, dp->d_name));

	  if (! *newname) {
	       error("append");
	       return error_code;
	  }
	  
	  if (is_deleted(dp->d_name)) {
	       if (retval = add_str(found, found_num, newname)) {
		    error("add_str");
		    closedir(dirp);
		    return retval;
	       }
	       found_num++;
	  }
	  if (lstat(newname, &stat_buf)) {
	       set_error(errno);
	       error(newname);
	       continue;
	  }
	  if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
	       if (retval = find_deleted_recurses(newname, &new_found_num,
						  &new_found)) {
		    error(newname);
		    closedir(dirp);
		    return retval;
	       }
	       if (retval = add_arrays(found, &found_num, &new_found,
				       &new_found_num)) {
		    error("add_arrays");
		    closedir(dirp);
		    return retval;
	       }
	  }
     }
     closedir(dirp);
     *num_found = found_num;
     return 0;
}






int find_contents(base, num_found, found)
char *base;
int *num_found;
char ***found;
{
     DIR *dirp;
     struct direct *dp;
     int num;
     int retval;
     char *tmp;
     
#ifdef DEBUG
     printf("Looking for contents of %s\n", base);
#endif
     *found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;

	  tmp = append(base, dp->d_name);
	  if (! *tmp) {
	       error("append");
	       return error_code;
	  }
	  if (retval = add_str(found, num, tmp)) {
	       error("add_str");
	       closedir(dirp);
	       return retval;
	  }
	  num += 1;
     }
     closedir(dirp);
     *num_found = num;
     return 0;
}


     
int find_deleted_contents(base, num_found, found)
char *base;
int *num_found;
char ***found;
{
     DIR *dirp;
     struct direct *dp;
     int num;
     int retval;
     
#ifdef DEBUG
     printf("Looking for deleted contents of %s\n", base);
#endif
     *found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  if (is_deleted(dp->d_name)) {
	       char *tmp;

	       tmp = append(base, dp->d_name);
	       if (! *tmp) {
		    error("append");
		    return error_code;
	       }
	       if (retval = add_str(found, num, tmp)) {
		    error("add_str");
		    closedir(dirp);
		    return retval;
	       }
	       num += 1;
	  }
     }
     closedir(dirp);
     *num_found = num;
     return 0;
}




int find_deleted_contents_recurs(base, num_found, found)
char *base;
int *num_found;
char ***found;
{
     DIR *dirp;
     struct direct *dp;
     int num;
     struct stat stat_buf;
     char newname[MAXPATHLEN];
     char **new_found;
     int new_found_num;
     int retval;
     
#ifdef DEBUG
     printf("Looking for recursive deleted contents of %s\n", base);
#endif
     *found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp) {
	  set_error(errno);
	  error(base);
	  return error_code;
     }
     
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  if (is_deleted(dp->d_name)) {
	       (void) strcpy(newname, append(base, dp->d_name));
	       if (! *newname) {
		    error("append");
		    return error_code;
	       }
	       if (retval = add_str(found, num, newname)) {
		    error("add_str");
		    closedir(dirp);
		    return retval;
	       }
	       num += 1;
	       if (lstat(newname, &stat_buf)) {
		    set_error(errno);
		    error(newname);
		    continue;
	       }
	       if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
		    if (retval = find_recurses(newname, &new_found_num,
					       &new_found)) {
			 error(newname);
			 closedir(dirp);
			 return retval;
		    }
		    if (retval = add_arrays(found, &num, &new_found,
					    &new_found_num)) {
			 error("add_arrays");
			 closedir(dirp);
			 return retval;
		    }
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;
     return 0;
}
     


/*
 * returns true if the filename has no globbing wildcards in it.  That
 * means no non-quoted question marks, asterisks, or open square
 * braces.  Assumes a null-terminated string, and a valid globbing
 * expression.
 */
int no_wildcards(name)
char *name;
{
     do {
	  switch (*name) {
	  case '\\':
	       name++;
	       break;
	  case '?':
	       return(0);
	  case '*':
	       return(0);
	  case '[':
	       return(0);
	  }
     } while (*++name);
     return(1);
}
