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
     static char rcsid_undelete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v 1.6 1989-01-26 00:09:16 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>
#include "directories.h"

#define DELETEPREFIX ".#"
#define DELETEREPREFIX "\\.#"
#define ERROR_MASK 1
#define NO_DELETE_MASK 2

typedef struct {
     char *user_name;
     char *real_name;
} listrec;


extern char *malloc(), *realloc();

char *whoami, *error_buf;
char *add_char(), *parse_pattern(), *append(),
     *strindex(), *strrindex(), *convert_to_user_name();
char **find_all_children(), **match_pattern();
listrec *unique(), *sort_files();

int interactive, recursive, verbose, directoriesonly, noop, force;

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     int status = 0;
     
     whoami = lastpart(argv[0]);
     interactive = recursive = verbose = directoriesonly = noop = force = 0;
     error_buf = malloc(MAXPATHLEN + strlen(whoami));
     if (! error_buf) {
	  perror(whoami);
	  exit(1);
     }
     while ((arg = getopt(argc, argv, "firvnR")) != -1) {
	  switch (arg) {
	  case 'f':
	       force++;
	       break;
	  case 'i':
	       interactive++;
	       break;
	  case 'r':
	       recursive++;
	       if (directoriesonly) {
		    fprintf(stderr, "%s: -r and -R and mutually exclusive.\n",
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
	  case 'R':
	       directoriesonly++;
	       if (recursive) {
		    fprintf(stderr, "%s: -r and -R are mutually exclusive.\n",
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
     exit(status | ERROR_MASK);
}



interactive_mode()
{
     return(0);
}



usage()
{
     fprintf(stderr, "Usage: %s [ options ] [filename ...]\n", whoami);
     fprintf(stderr, "Options are:\n");
     fprintf(stderr, "     -r     recursive\n");
     fprintf(stderr, "     -i     interactive\n");
     fprintf(stderr, "     -f     force\n");
     fprintf(stderr, "     -v     verbose\n");
     fprintf(stderr, "     -n     noop\n");
     fprintf(stderr, "     -R     directories only (i.e. no recursion)\n");
     fprintf(stderr, "     --     end options and start filenames\n");
     fprintf(stderr, "-r and -D are mutually exclusive\n");
}


undelete(file_exp)
char *file_exp;
{
     char *file_re;
     char **found_files;
     int num_found;
     char *startdir;
     int status = 0;
     filerec *current;
     
     if (*file_exp == '/') {
	  startdir = "/";
	  file_re = parse_pattern(file_exp + 1);
     }
     else {
	  startdir = "";
	  file_re = parse_pattern(file_exp);
     }
     if (! file_re)
	  return(ERROR_MASK);
     found_files = match_pattern(startdir, FtDirectory, file_re, &num_found);
     free(file_re);
     if (num_found) {
	  process_files(found_files, &num_found);
	  if (*file_exp == '/') 
	       current = get_root_tree();
	  else
	       current = get_cwd_tree();
	  current = next_specified_leaf(current);
	  if (current)
	       status = do_undelete(current);
     }
     else {
	  if (! force)
	       fprintf(stderr, "%s: %s not found\n", whoami, file_exp);
	  status = ERROR_MASK;
     }
     return(status);
}




process_files(files, num)
char **files;
int *num;
{
     int i;
     listrec *new_files;
     listrec *filelist;

     filelist = (listrec *) malloc(sizeof(listrec) * (*num));
     if (! filelist) {
	  perror(sprintf(error_buf, "%s: process_files\n", whoami));
	  exit(1);
     }
     for (i = 0; i < *num; i++) {
	  filelist[i].real_name = malloc(strlen(files[i]) + 1);
	  strcpy(filelist[i].real_name, files[i]);
	  filelist[i].user_name = malloc(strlen(files[i]) + 1);
	  convert_to_user_name(files[i], filelist[i].user_name);
	  free(files[i]);
     }
     free(files);
     
     new_files = sort_files(filelist, *num);
     new_files = unique(new_files, num);
     if (initialize_tree()) {
	  exit(1);
     }
     for (i = 0; i < *num; i++) {
	  if (!add_path_to_tree(new_files[i].real_name, FtUnknown)) {
	       fprintf(stderr, "%s: error adding path to filename tree\n",
		       whoami);
	       exit(1);
	  }
	  else {
	       free(new_files[i].real_name);
	       free(new_files[i].user_name);
	  }
     }
     free(new_files);
     return(0);
}

     


do_undelete(the_file)
filerec *the_file;
{
     int status;
     filerec *new_file;
     
     status = really_do_undelete(the_file);
     if (status && (the_file->ftype == FtDirectory)) {
	  new_file = next_specified_directory(the_file);
	  if (new_file)
	       status = status | do_undelete(new_file);
     }
     else {
	  new_file = next_specified_leaf(the_file);
	  if (new_file)
	       status = status | do_undelete(new_file);
     }
     free_leaf(the_file);
     return(status);
}







     
really_do_undelete(file_ent)
filerec *file_ent;
{
     struct stat stat_buf;
     char user_name[MAXPATHLEN], real_name[MAXPATHLEN];

     get_leaf_path(file_ent, real_name);
     convert_to_user_name(real_name, user_name);

     if (interactive) {
	  if (file_ent->ftype == FtDirectory)
	       printf("%s: Undelete directory %s? ", whoami, user_name);
	  else
	       printf("%s: Undelete %s? ", whoami, user_name);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
     if (! lstat(user_name, &stat_buf)) if (! force) {
	  printf("%s: An undeleted %s already exists.\n", whoami, user_name);
	  printf("Do you wish to continue with the undelete and overwrite that version? ");
	  if (! yes())
	       return(NO_DELETE_MASK);
	  unlink_completely(user_name);
     }
     if (noop) {
	  printf("%s: %s would be undeleted\n", whoami, user_name);
	  return(0);
     }

     if (! do_file_rename(real_name, user_name)) {
	  if (verbose)
	       printf("%s: %s undeleted\n", whoami, user_name);
	  return(0);
     }
     else {
	  if (! force)
	       fprintf(stderr, "%s: %s not undeleted\n", whoami, user_name);
	  return(ERROR_MASK);
     }
}




do_file_rename(real_name, user_name)
char *real_name, *user_name;
{
     char *ptr;
     
     char old_name[MAXPATHLEN], new_name[MAXPATHLEN];
     char buf[MAXPATHLEN];
     
     strcpy(old_name, real_name);
     strcpy(new_name, real_name);

     while (ptr = strrindex(new_name, ".#")) {
	  convert_to_user_name(ptr, ptr);
	  strcpy(ptr, firstpart(ptr, buf));
	  strcpy(&old_name[ptr - new_name],
		 firstpart(&old_name[ptr - new_name], buf));
	  if (rename(old_name, new_name)) {
	       return(ERROR_MASK);
	  }
	  if (ptr > new_name) {
	       *--ptr = '\0';
	       old_name[ptr - new_name] = '\0';
	  }
     }
     change_path(real_name, user_name);
     return(0);
}






char *convert_to_user_name(real_name, user_name)
char real_name[];
char user_name[];  /* RETURN */
{
     char *ptr, *q;
     
     strcpy(user_name, real_name);
     while (ptr = strrindex(user_name, ".#")) {
	  for (q = ptr; *(q + 2); q++)
	       *q = *(q + 2);
	  *q = '\0';
     }
     return (user_name);
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
char *add_char(start, finish, length, chr)
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
	  if ((! recursive) || (ftype != FtDirectory)) {
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

		    
	  
char **find_all_children(base, num)
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

	       



		    
     

filecmp(file1, file2)
listrec *file1, *file2;
{
     return(strcmp(file1->user_name, file2->user_name));
}

     
     
listrec *sort_files(data, num_data)
listrec *data;
int num_data;
{
     qsort(data, num_data, sizeof(listrec), filecmp);
     return(data);
}





listrec *unique(files, number)
listrec *files;
int *number;
{
     int i, last;
     int offset;
     
     for (last = 0, i = 1; i < *number; i++) {
	  if (! strcmp(files[last].user_name, files[i].user_name)) {
	       int better;

	       better = choose_better(files[last].real_name,
				      files[i].real_name);
	       if (better == 1) { /* the first one is better */
		    free (files[i].real_name);
		    free (files[i].user_name);
		    files[i].real_name = (char *) NULL;
	       }
	       else {
		    free (files[last].real_name);
		    free (files[last].user_name);
		    files[last].real_name = (char *) NULL;
		    last = i;
	       }
	  }
	  else
	       last = i;
     }
     
     for (offset = 0, i = 0; i + offset < *number; i++) {
	  if (! files[i].real_name)
	       offset++;
	  if (i + offset < *number)
	       files[i] = files[i + offset];
     }
     *number -= offset;
     files = (listrec *) realloc(files, sizeof(listrec) * *number);
     if (! files) {
	  perror(sprintf(error_buf, "%s: unique", whoami));
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



char *strrindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr;

     if (strlen(str))
	  ptr = &str[strlen(str) - 1];
     else
	  return((char *) NULL);
     while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     while (ptr != str) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr--;
	  while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     }
     if (! strncmp(ptr, sub_str, strlen(sub_str)))
	  return(str);
     else
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




     
is_dotfile(filename)
char *filename;
{
     return (! (strcmp(filename, ".") && strcmp(filename, "..")));
}



/*
 * NOTE: Append uses a static array, so its return value must be
 * copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if ((! *filename) || (! *filepath)) {
	  strcpy(buf, filename);
	  return(buf);
     }
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




yes() {
     char buf[BUFSIZ];

     fgets(buf, BUFSIZ, stdin);
     if (! index(buf, '\n')) do
	  fgets(buf + 1, BUFSIZ - 1, stdin);
     while (! index(buf + 1, '\n'));
     return(*buf == 'y');
}




unlink_completely(filename)
char *filename;
{
     char buf[MAXPATHLEN];
     struct stat stat_buf;
     DIR *dirp;
     struct direct *dp;
     int status = 0;
     
     if (lstat(filename, &stat_buf))
	  return(1);

     if (stat_buf.st_mode & S_IFDIR) {
	  dirp = opendir(filename);
	  if (! dirp)
	       return(1);
	  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	       if (is_dotfile(dp->d_name))
		    continue;
	       strcpy(buf, append(filename, dp->d_name));
	       if (! buf) {
		    status = 1;
		    continue;
	       }
	       status = status | unlink_completely(buf);
	  }
	  closedir(dirp);
     }
     else
	  return(unlink(filename) == -1);
     return(0);
}




int is_deleted(filename)
char *filename;
{
     return(! strncmp(filename, ".#", 2));
}
