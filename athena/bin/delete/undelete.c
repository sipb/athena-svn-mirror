/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v $
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
     static char rcsid_undelete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v 1.2 1989-01-23 11:04:41 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>

#define DELETEPREFIX ".#"
#define DELETEREPREFIX "\\.#"

typedef struct {
     char *realname;		/* The real filesystem name, with */
				/* deletion characters */
     char *username;		/* The name the user enters to */
				/* undelete it */
} filerec;

static filerec cwd = {"./", "./"};
static filerec root = {"/", "/"};

extern char *malloc(), *realloc();

char *whoami, *error_buf;
char *lastpart(), *add_char(), *parse_pattern(), *firstpart(), *append();
filerec *match_pattern(), *unique();

int interactive, recursive, verbose, directoriesonly, noop;

/* ARGSUSED */
main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     int status;
     
     whoami = lastpart(argv[0]);
     interactive = recursive = verbose = directoriesonly = noop = 0;
     error_buf = malloc(MAXPATHLEN + strlen(whoami));
     if (! error_buf) {
	  fprintf(stderr, "%s: Can't malloc space for error buffer.\n",
		  whoami);
	  exit(1);
     }
     while ((arg = getopt(argc, argv, "irvnD")) != -1) {
	  switch (arg) {
	  case 'i':
	       interactive++;
	       break;
	  case 'r':
	       recursive++;
	       if (directoriesonly) {
		    fprintf(stderr, "%s: -r and -D and mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       break;
	  case 'v':
	       verbose++;
	       break;
	  case 'n':
	       noop++;
	       break;
	  case 'D':
	       directoriesonly++;
	       if (recursive) {
		    fprintf(stderr, "%s: -r and -D are mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	  }
     }
     if (optind == argc)
	  exit(interactive_mode());
     else while (optind < argc) {
	  status = status | undelete(argv[optind]);
	  optind++;
     }
     exit(status);
}



interactive_mode()
{
     return(0);
}



usage()
{
     printf("Usage: %s [ options ] [filename ...]\n", whoami);
     printf("Options are:\n");
     printf("     -r     recursive\n");
     printf("     -i     interactive\n");
     printf("     -v     verbose\n");
     printf("     -n     noop\n");
     printf("     -D     directories only\n");
     printf("     --     end options and start filenames\n");
     printf("-r and -D are mutually exclusive\n");
}


undelete(file_exp)
char *file_exp;
{
     char *file_re;
     filerec *found_files;
     int num_found;
     filerec startdir;

     if (*file_exp == '/') {
	  startdir = root;
	  file_re = parse_pattern(file_exp + 1);
     }
     else {
	  startdir = cwd;
	  file_re = parse_pattern(file_exp);
     }
     if (! file_re)
	  return(1);
     found_files = match_pattern(startdir, file_re, &num_found);
     free(file_re);
     sort_filerecs(found_files, num_found);
     found_files = unique(found_files, &num_found);
}

     

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
     cur_ptr = file_pattern;
     
     if (! re_pattern) {
	  perror(sprintf(error_buf, "%s: parse_pattern", whoami));
	  return ((char *) NULL);
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
char *add_char(start, finish, length, chr)
char **start, **finish;
int *length;
char chr;
{
     if (*finish - *start == *length) {
	  *start = realloc(*start, *length + 5);
	  if (! *start)
	       return ((char *) NULL);
	  *finish = *start + *length - 1;
	  *length += 5;
     }
     **finish = chr;
     (*finish)++;
     return(*start);
}

	  

/*
 * add_arrays() takes two arrays of filerec's and their lengths,
 * merges the two into the first by realloc'ing the first and then
 * free's the second's memory usage.
 */  
filerec *add_arrays(array1, num1, array2, num2)
filerec *array1, *array2;
int *num1, *num2;
{
     int counter;
     
     array1 = (filerec *) realloc(array1, sizeof(filerec) * (*num1 + *num2));
     if (! array1) {
	  fprintf(stderr, "%s: Error in realloc in add_arrays!\n", whoami);
	  exit(1);
     }
     for (counter = *num1; counter < *num1 + *num2; counter++)
	  array1[counter] = array2[counter - *num1];
     free(array2);
     *num1 += *num2;
     return(array1);
}




/*
 * match_pattern() takes a filerec base directory to start up in, a
 * file pattern string which has already been processed with
 * parse_pattern, and an (int *) argument into which will be placed
 * the number of files found when match_pattern returns.  It searches
 * down the directory tree for files matching the given pattern, puts
 * them into an array of filerec's and returns the array.
 *
 * This function is recursive.
 *
 * The return array of filerecs can (and should) be freed using free()
 * when it is no longer needed.
 */
filerec *match_pattern(base, file_pattern, num_returned)
filerec base;
char *file_pattern;
int *num_returned;
{
     char search_string[MAXNAMLEN], deleted_search_string[MAXNAMLEN];
     char rest_of_string[MAXPATHLEN];
     filerec *rec_list;
     int num_rec_list;
     DIR *dirp;
     struct direct *dp;
     struct stat stat_buf;
     char realname[MAXPATHLEN], username[MAXPATHLEN];
     
     rec_list = (filerec *) malloc(0); /* so it can be realloc'd later */
     num_rec_list = 0;

     strcpy(search_string, firstpart(file_pattern, rest_of_string));
     strcpy(deleted_search_string, DELETEREPREFIX);
     strcat(deleted_search_string, search_string);
     
     dirp = opendir(base.realname);
     if (! dirp) {
	  *num_returned = num_rec_list;
	  return(rec_list);
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  strcpy(realname, append(base.realname, dp->d_name));
	  printf("checking %s, rest_of_string is %s\n", dp->d_name,
		 rest_of_string);
	  if (lstat(realname, &stat_buf))
	       continue;
	  if (stat_buf.st_mode & S_IFDIR) {
	       re_comp(deleted_search_string);
	       if (re_exec(dp->d_name)) {
		    filerec *retval;
		    int retnum;
		    filerec newbase;
		    
		    strcpy(username, append(base.username, &dp->d_name[2]));
		    newbase.realname = realname;
		    newbase.username = username;
		    retval = match_pattern(newbase, rest_of_string,
					   &retnum);
		    if (retnum != 0) {
			 rec_list = add_arrays(rec_list, &num_rec_list,
					       retval, &retnum);
		    }
	       }
	       else {
		    re_comp(search_string);
		    if (re_exec(dp->d_name)) {
			 filerec *retval;
			 int retnum;
			 filerec newbase;

			 strcpy(username, append(base.username, dp->d_name));
			 newbase.realname = realname;
			 newbase.username = username;
			 retval = match_pattern(newbase, rest_of_string,
						&retnum);
			 if (retnum != 0) {
			      rec_list = add_arrays(rec_list, &num_rec_list,
						    retval, &retnum);
			 }
		    }	
	       }
	  }
	  else {
	       if (*rest_of_string)
		    continue;
	       re_comp(deleted_search_string);
	       if (re_exec(dp->d_name)) {
		    strcpy(username, append(base.username, &dp->d_name[2]));
		    num_rec_list += 1;
		    rec_list = (filerec *) realloc(rec_list, sizeof(filerec) *
						   num_rec_list);
		    
		    rec_list[num_rec_list - 1].realname =
			 malloc(strlen(realname) + 1);
		    strcpy(rec_list[num_rec_list - 1].realname, realname);
		    rec_list[num_rec_list - 1].username =
			 malloc(strlen(username) + 1);
		    strcpy(rec_list[num_rec_list - 1].username, username);
	       }
	  }
     }
     *num_returned = num_rec_list;
     return(rec_list);
}

		    
     


filereccmp(rec1, rec2)
filerec *rec1, *rec2;
{
     return(strcmp(rec1->username, rec2->username));
}


sort_filerecs(data, num_data)
filerec *data;
int num_data;
{
     qsort(data, num_data, sizeof(filerec), filereccmp);
}





filerec *unique(files, number)
filerec *files;
int *number;
{
     int i, offset, last;

     for (last = 0, i = 1; i < *number; i++) {
	  if (! strcmp(files[last].username, files[i].username)) {
	       int better;
	       filerec *garbage;
	       
	       better = choose_better(files[last].realname, files[i].realname);
	       if (better == 1) /* the first one is better */
		    garbage = &files[i];
	       else {
		    garbage = &files[last];
		    last = i;
	       }
	       free(garbage->realname);
	       free(garbage->username);
	  }
	  else
	       last = i;
     }
     for (offset = 0, i = 0; i + offset < *number; i++) {
	  if (! files[i].realname) {
	       offset++;
	  }
	  if (i + offset < *number)
	       files[i] = files[i + offset];
     }
     *number -= offset;
     realloc(files, sizeof(filerec) * *number);
     if (! files) {
	  perror(sprintf("%s: Realloc'ing in unique.\n", whoami));
	  exit(1);
     }
     return(files);
}




char *strindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr = str;
     while (ptr = index(ptr, *sub_str)) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr++;
     }
     return ((char *) NULL);
}

     
choose_better(str1, str2)
char *str1, *str2;
{
     char *pos1, *pos2;
     
     pos1 = strindex(str1, ".#");
     pos2 = strindex(str2, ".#");
     while (pos1 && pos2) {
	  if (pos1 - str1 < pos2 - str2)
	       return(2);
	  else if (pos2 - str2 < pos1 - str1)
	       return(1);
	  pos1 = strindex(pos1 + 1, ".#");
	  pos2 = strindex(pos2 + 1, ".#");
     }
     if (! pos1)
	  return(1);
     else
	  return(2);
}




     
char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}





char *firstpart(filename, rest)
char *filename;
char *rest; /* RETURN */
{
     char *part;
     static char buf[MAXPATHLEN];

     strcpy(buf, filename);
     part = index(buf, '/');
     if (! part) {
	  *rest = '\0';
	  return(buf);
     }
     strcpy(rest, part + 1);
     *part = '\0';
     return(buf);
}



is_dotfile(filename)
char *filename;
{
     return (! (strcmp(filename, ".") && strcmp(filename, "..")));
}



/*
 * NOTE: The two string arguments to append must NOT be strings of
 * length zero.  Furthermore, append uses a static array, so its
 * return value must be copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if (buf[strlen(buf) - 1] == '/')
	  buf[strlen(buf) - 1] = '\0';
     if (strlen(buf) + strlen(filename) + 2 > MAXPATHLEN) {
 	  *buf = '\0';
	  return(buf);
     }
     strcat(buf, "/");
     strcat(buf, filename);
     return(buf);
}
