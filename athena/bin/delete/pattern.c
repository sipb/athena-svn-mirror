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
     static char rcsid_pattern_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.c,v 1.4 1989-01-27 02:58:15 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>
#include "directories.h"
#include "pattern.h"
#include "util.h"
#include "undelete.h"

static char *add_char();

extern char *malloc(), *realloc();

extern char *whoami, *error_buf;

/*
 * parse_pattern returns an area of memory allocated by malloc when it
 * is successful.  Therefore, other procedures calling parse_pattern
 * should use free() to free the region of memory when they are done
 * with it.
 */
char *parse_pattern(file_pattern)
char *file_pattern;
{
     char *re_pattern, *cur_ptr, *re_ptr;
     int guess_length;
     
     guess_length = strlen(file_pattern) + 5;
     re_ptr = re_pattern = malloc(guess_length);
     if (! re_ptr) {
	  perror(sprintf(error_buf, "%s: parse_pattern", whoami));
	  exit(1);
     }
     
     for (cur_ptr = file_pattern, re_ptr = re_pattern; *cur_ptr != NULL;
	  cur_ptr++) {
	  if (*cur_ptr == '\\') {
	       if (! cur_ptr[1]) {
		    fprintf(stderr,
			    "%s: parse_pattern: incomplete expression\n",
			    whoami);
		    return((char *) NULL);
	       }
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '\\'))
		    return ((char *) NULL);
	       cur_ptr++;
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, *cur_ptr))
		    return ((char *) NULL);
	       continue;
	  }
	  else if (*cur_ptr == '*') {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '.'))
		    return ((char *) NULL);
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '*'))
		    return ((char *) NULL);
	       continue;
	  }
	  else if (*cur_ptr == '?') {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '.'))
		    return ((char *) NULL);
	       continue;
	  }
	  else {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, *cur_ptr))
		    return ((char *) NULL);
	  }
     }
     if (! add_char(&re_pattern, &re_ptr, &guess_length, '\0'))
	  return ((char *) NULL);
     return (re_pattern);
}






/*
 * add_char() takes two char **, a length which is the current amount
 * of space implemented for the string pointed to by the first *(char **),
 * and a character to add to the string.  It reallocs extra space if
 * necessary, adds the character, and messes with the pointers if necessary.
 */
static char *add_char(start, finish, length, chr)
char **start, **finish;
int *length;
char chr;
{
     if (*finish - *start == *length) {
	  *start = realloc(*start, *length + 5);
	  if (! *start) {
	       perror(sprintf(error_buf, "%s: add_char", whoami));
	       exit(1);
	  }
	  *finish = *start + *length - 1;
	  *length += 5;
     }
     **finish = chr;
     (*finish)++;
     return(*start);
}

	  




/*
 * add_arrays() takes pointers to two arrays of char **'s and their
 * lengths, merges the two into the first by realloc'ing the first and
 * then free's the second's memory usage.
 */  
add_arrays(array1, num1, array2, num2)
char ***array1, ***array2;
int *num1, *num2;
{
     int counter;
     
     *array1 = (char **) realloc(*array1, sizeof(char *) * (*num1 + *num2));
     if (! *array1) {
	  perror(sprintf(error_buf, "%s: add_arrays", whoami));
	  exit(1);
     }
     for (counter = *num1; counter < *num1 + *num2; counter++)
	  *(*array1 + counter) = *(*array2 + counter - *num1);
     free (*array2);
     *num1 += *num2;
     return(0);
}







char **add_str(strs, num, str)
char **strs;
int num;
char *str;
{
     strs = (char **) realloc(strs, sizeof(char *) * (num + 1));
     if (! strs) {
	  perror(sprintf(error_buf, "%s: add_str", whoami));
	  exit(1);
     }
     strs[num] = malloc(strlen(str) + 1);
     if (! strs[num]) {
	  perror(sprintf(error_buf, "%s: add_str", whoami));
	  exit(1);
     }
     strcpy(strs[num], str);
     return(strs);
}







char **find_deleted_matches(base, expression, num_found)
char *base, *expression;
int *num_found;
{
     struct direct *dp;
     DIR *dirp;
     char **found;
     int num;
     char **next;
     int num_next;
     char first[MAXNAMLEN], rest[MAXPATHLEN];
     char new[MAXPATHLEN];
     
     found = (char **) malloc(0);
     *num_found = num = 0;

     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */
     
     strcpy(first, reg_firstpart(expression, rest));
     re_comp(first);

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (re_exec(dp->d_name) && *rest) {
	       strcpy(new, append(base, dp->d_name));
	       next = find_deleted_matches(new, rest, &num_next);
	       add_arrays(&found, &num, &next, &num_next);
	  }
	  else if (is_deleted(dp->d_name)) if (re_exec(&dp->d_name[2])) {
	       if (*rest) {
		    strcpy(new, append(base, dp->d_name));
		    next = find_deleted_matches(new, rest, &num_next);
		    add_arrays(&found, &num, &next, &num_next);
	       }
	       else {
		    found = add_str(found, num, append(base, dp->d_name));
		    num++;
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;
     return(found);
}





char **find_matches(base, expression, num_found)
char *base, *expression;
int *num_found;
{
     struct direct *dp;
     DIR *dirp;
     char **found;
     int num;
     char **next;
     int num_next;
     char first[MAXNAMLEN], rest[MAXPATHLEN];
     char new[MAXPATHLEN];
     
     found = (char **) malloc(0);
     *num_found = num = 0;

     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */
     
     strcpy(first, reg_firstpart(expression, rest));

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  re_comp(first);
	  if (re_exec(dp->d_name)) {
	       if (*rest) {
		    strcpy(new, append(base, dp->d_name));
		    next = find_matches(new, rest, &num_next);
		    add_arrays(&found, &num, &next, &num_next);
	       }
	       else {
		    found = add_str(found, num, append(base, dp->d_name));
		    num++;
	       }
	  }
	  else if (is_deleted(dp->d_name)) if (re_exec(&dp->d_name[2])) {
	       if (*rest) {
		    strcpy(new, append(base, dp->d_name));
		    next = find_matches(new, rest, &num_next);
		    add_arrays(&found, &num, &next, &num_next);
	       }
	       else {
		    found = add_str(found, num, append(base, dp->d_name));
		    num++;
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;
     return(found);
}







char **find_recurses(base, num_found)
char *base;
int *num_found;
{
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     char **found, **new_found;
     int found_num, new_found_num;
     struct stat stat_buf;
     
     found = (char **) malloc(0);
     *num_found = found_num = 0;
     
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  strcpy(newname, append(base, dp->d_name));
	  found = add_str(found, found_num, newname);
	  found_num++;
	  if (lstat(newname, &stat_buf))
	       continue;
	  if (stat_buf.st_mode & S_IFDIR) {
	       new_found = find_recurses(newname, &new_found_num);
	       add_arrays(&found, &found_num, &new_found, &new_found_num);
	  }
     }
     closedir(dirp);
     *num_found = found_num;
     return(found);
}






char **find_deleted_recurses(base, num_found)
char *base;
int *num_found;
{
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     char **found, **new_found;
     int found_num, new_found_num;
     struct stat stat_buf;
     
     found = (char **) malloc(0);
     *num_found = found_num = 0;
     
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  strcpy(newname, append(base, dp->d_name));
	  
	  if (is_deleted(dp->d_name)) {
	       found = add_str(found, found_num, newname);
	       found_num++;
	  }
	  if (lstat(newname, &stat_buf)) {
	       perror("foobar");
	       continue;
	  }
	  if (stat_buf.st_mode & S_IFDIR) {
	       new_found = find_deleted_recurses(newname, &new_found_num);
	       add_arrays(&found, &found_num, &new_found, &new_found_num);
	  }
     }
     closedir(dirp);
     *num_found = found_num;
     return(found);
}






char **find_contents(base, num_found)
char *base;
int *num_found;
{
     DIR *dirp;
     struct direct *dp;
     char **found;
     int num;

     found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  found = add_str(found, num, append(base, dp->d_name));
	  num += 1;
     }
     closedir(dirp);
     *num_found = num;
     return(found);
}


     
char **find_deleted_contents(base, num_found)
char *base;
int *num_found;
{
     DIR *dirp;
     struct direct *dp;
     char **found;
     int num;

     found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_deleted(dp->d_name)) {
	       found = add_str(found, num, append(base, dp->d_name));
	       num += 1;
	  }
     }
     closedir(dirp);
     *num_found = num;
     return(found);
}




char **find_deleted_contents_recurs(base, num_found)
char *base;
int *num_found;
{
     DIR *dirp;
     struct direct *dp;
     char **found;
     int num;
     struct stat stat_buf;
     char newname[MAXPATHLEN];
     char **new_found;
     int new_found_num;
     
     found = (char **) malloc(0);
     *num_found = num = 0;
   
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     readdir(dirp); readdir(dirp); /* get rid of . and .. */

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_deleted(dp->d_name)) {
	       strcpy(newname, append(base, dp->d_name));
	       found = add_str(found, num, newname);
	       num += 1;
	       if (lstat(newname, &stat_buf))
		    continue;
	       if (stat_buf.st_mode & S_IFDIR) {
		    new_found = find_recurses(newname, &new_found_num);
		    add_arrays(&found, &num, &new_found, &new_found_num);
	       }
	  }
     }
     closedir(dirp);
     *num_found = num;
     return(found);
}
     
