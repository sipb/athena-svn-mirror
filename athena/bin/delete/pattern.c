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
     static char rcsid_pattern_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/pattern.c,v 1.2 1989-01-26 10:59:50 jik Exp $";
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
static char **find_all_children();
static char **find_dir_contents();

extern char *malloc(), *realloc();

extern char *whoami, *error_buf;
extern int recursive, directoriesonly;

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
     cur_ptr = file_pattern;
     
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




char **match_pattern(base, ftype, file_pattern, num_found)
char *base;
filetype ftype;
char *file_pattern;
int *num_found;
{
     char **found;
     
     found = (char **) malloc(0);
     *num_found = 0;
     
     if (! *file_pattern) { /* The file pattern is empty, so we've */
			    /* reached the end of the line.  If we are */
			    /* not looking for recursive stuff, or if */
			    /* the base involved is a file, then */
			    /* we return only the base; otherwise, we */
			    /* recursively descend and return all of */
			    /* the descendents of this */
			    /* directory. */
	  if (is_deleted(lastpart(base))) {
	       *num_found = 1;
	       found = (char **) realloc(found, sizeof(char *));
	       if (! found) {
		    perror(sprintf(error_buf, "%s: match_pattern", whoami));
		    exit(1);
	       }
	       *found = malloc(strlen(base) + 1);
	       if (! *found) {
		    perror(sprintf(error_buf, "%s: match_pattern", whoami));
		    exit(1);
	       }
	       strcpy(*found, base);
	  }
	  if ((ftype == FtDirectory) && (! recursive) &&
	      (! directoriesonly)) {
	       int num_dir_found;
	       char **dir_found;
	       dir_found = find_dir_contents(base, &num_dir_found);
	       add_arrays(&found, num_found, &dir_found, &num_dir_found);
	  }
	  else if ((! recursive) || (ftype != FtDirectory) ||
		   directoriesonly) {
	       return(found);
	  }
	  else {
	       int num_recurs_found;
	       char **recurs_found;
	       recurs_found = find_all_children(base, &num_recurs_found);
	       add_arrays(&found, num_found, &recurs_found,
			  &num_recurs_found);
	       return(found);
	  }
     }
     else if (ftype != FtDirectory) { /* A non-directory has been */
				      /* passed in even though the */
				      /* file pattern has not reached */
				      /* its end yet.  This is bad, so */
				      /* we return nothing. */
	  return(found);
     }
     else { /* we have a file pattern to work with, so we take the */
	    /* first part of it and match it against all the files in */
	    /* the directory.  For each one found, call match_pattern */
	    /* on it and concatenate the result onto what we've */
	    /* already got. */
	  DIR *dirp;
	  struct direct *dp;
	  struct stat stat_buf;
	  char pattern[MAXNAMLEN], del_pattern[MAXNAMLEN];
	  char rest_of_pattern[MAXPATHLEN];
	  char newname[MAXPATHLEN];
	  filetype newtype;
	  char **new_found;
	  int num_new_found;
	  Boolean match;
	  
	  strcpy(pattern, firstpart(file_pattern, rest_of_pattern));
	  strcpy(del_pattern, DELETEREPREFIX);
	  strcat(del_pattern, pattern);

	  dirp = opendir(base);
	  if (! dirp) {
	       return(found);
	  }

	  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	       if (is_dotfile(dp->d_name))
		    continue;
	       strcpy(newname, append(base, dp->d_name));
	       if (*rest_of_pattern || recursive) {
		    if (stat(newname, &stat_buf))
			 continue;
	       }
	       else {
		    if (lstat(newname, &stat_buf))
			 continue;
	       }
	       re_comp(pattern);
	       match = re_exec(dp->d_name);
	       if (! match) {
		    re_comp(del_pattern);
		    match = re_exec(dp->d_name);
	       }
	       if (match) {
		    newtype = ((stat_buf.st_mode & S_IFDIR) ? FtDirectory :
			       FtFile);
		    new_found = match_pattern(newname, newtype,
					      rest_of_pattern, &num_new_found);
		    add_arrays(&found, num_found, &new_found, &num_new_found);
	       }
	  }
	  closedir(dirp);
     }
     return(found);
}

		    
	  
static char **find_all_children(base, num)
char *base;
int *num;
{
     int new_num;
     char **found, **new_found;
     struct stat stat_buf;
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     
     *num = 0;
     found = (char **) malloc(0);
     
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  strcpy(newname, append(base, dp->d_name));
	  if (is_deleted(dp->d_name)) {
	       *num += 1;
	       found = (char **) realloc(found, sizeof(char *) * (*num));
	       if (! found) {
		    perror(sprintf(error_buf, "%s: find_all_children",
				   whoami));
		    exit(1);
	       }
	       found[*num - 1] = (char *) malloc(strlen(newname) + 1);
	       if (! found[*num - 1]) {
		    perror(sprintf(error_buf, "%s: find_all_children",
				   whoami));
		    exit(1);
	       }
	       strcpy(found[*num - 1], newname);
	  }
	  if (lstat(newname, &stat_buf))
	       continue;
	  if (stat_buf.st_mode & S_IFDIR) {
	       new_found = find_all_children(newname, &new_num);
	       add_arrays(&found, num, &new_found, &new_num);
	  }
     }
     closedir(dirp);
     return(found);
}

	       


static char **find_dir_contents(base, num)
char *base;
int *num;
{
     char **found;
     DIR *dirp;
     struct direct *dp;
     char newname[MAXPATHLEN];
     
     *num = 0;
     found = (char **) malloc(0);
     
     dirp = opendir(base);
     if (! dirp)
	  return(found);

     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  if (! is_deleted(dp->d_name))
	       continue;
	  strcpy(newname, append(base, dp->d_name));
	  *num += 1;
	  found = (char **) realloc(found, sizeof(char *) * (*num));
	  if (! found) {
	       perror(sprintf(error_buf, "%s: find_dir_contents",
			      whoami));
	       exit(1);	
	  }
	  found[*num - 1] = (char *) malloc(strlen(newname) + 1);
	  if (! found[*num - 1]) {
	       perror(sprintf(error_buf, "%s: find_dir_contents",
			      whoami));
	       exit(1);
	  }
	  strcpy(found[*num - 1], newname);
     }
     closedir(dirp);
     return(found);
}
