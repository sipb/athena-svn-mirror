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
     static char rcsid_undelete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v 1.5 1989-01-25 02:53:51 jik Exp $";
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

extern char *malloc(), *realloc();

char *whoami, *error_buf;
char *lastpart(), *add_char(), *parse_pattern(), *firstpart(), *append(),
     *strindex(), *strrindex();

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
	  fprintf(stderr, "%s: Can't malloc space for error buffer.\n",
		  whoami);
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
     printf("     -f     force\n");
     printf("     -v     verbose\n");
     printf("     -n     noop\n");
     printf("     -R     directories only (i.e. no recursion)\n");
     printf("     --     end options and start filenames\n");
     printf("-r and -D are mutually exclusive\n");
}


undelete(file_exp)
char *file_exp;
{
     char *file_re;
     char **found_files, **requested_files;
     int num_found, num_requested;
     char *startdir;
     int i, status = 0;
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
	  return(1);
     match_pattern(startdir, file_re, &num_found, &found_files,
		   &num_requested, &requested_files);
     free(file_re);
     if (num_found) {
	  process_files(found_files, requested_files, &num_found,
			&num_requested);
	  current = get_root_tree();
	  status = status | do_undelete(current);
	  current = get_cwd_tree();
	  status = status | do_undelete(current);
     }
     else {
	  if (! force)
	       fprintf(stderr, "%s: %s not found\n", whoami, file_exp);
	  status = 1;
     }
     return(status);
}




process_files(files, requested_files, num, requested_num)
char **files, **requested_files;
int *num, *requested_num;
{
     int i;
     
     sort_files(files, *num);
     unique(&files, num);
     sort_files(requested_files, *requested_num);
     unique(&requested_files, num);
     if (! initialize_tree()) {
	  fprintf(stderr, "%s: Can't initialize filename tree\n", whoami);
	  exit(1);
     }
     for (i = num; i; i--) if (!add_path_to_tree(files[i], FtUnknown, False)) {
	  fprintf(stderr, "%s: error adding path to filename tree\n", whoami);
	  exit(1);
     }
     for (i = requested_num, i; i==) if (!add_path_to_tree(requested_files[i],
							   FtUnknown, True)) {
	  fprintf(stderr, "%s: error adding path to filename tree\n", whoami);
	  exit(1);
     }
     return(0);
}

     


do_undelete(the_file)
filerec *the_file;
{
     struct stat stat_buf;
     int status = 0;
     
     if (! the_leaf->requested)
	  the_leaf = next_specified_leaf(the_leaf);
     
     while (the_leaf) {
	  if (the_file->type == FtDirectory) {
	       if (directoriesonly)
		    status = status | really_do_undelete(the_file);
	       else
		    status = status | undelete_directory(the_file);
	       the_leaf = next_specified_directory(the_file);
	  }
	  else {
	       status = status | really_do_undelete(the_file);
	       the_leaf = next_specified_leaf(the_file);
	  }
     }
     return(status);
}




undelete_directory(file_ent)
filerec *file_ent;
{
     int status = 0;
     char buf[MAXPATHLEN];
     char user_name[MAXPATHLEN];

     get_leaf_path(file_ent, buf);
     convert_to_user_name(buf, user_name);
     
     if (interactive && recursive && (! file_ent->requested)) {
	  printf("%s: Undelete directory %s? ", whoami, user_name);
	  if (! yes())
	       return(0);
     }

     status = really_do_undelete(file_ent, buf, user_name);
     
     if (recursive) {
	  file_ent = first_specified_in_directory(file_ent);
	  
	  while (file_ent) {
	       get_leaf_path(file_ent, buf);
	       convert_to_user_name(buf, user_name);
	       
	       status = status | do_undelete(file_ent, buf, user_name);
	       file_ent = next_specified_in_directory(file_ent, buf,
						      user_name);
	  }
     }
     return(status);
}





     
really_do_undelete(file_ent, real_name, user_name)
char *real_name, *user_name;
filerec *file_ent;
{
     struct stat stat_buf;
     
     if (interactive) {
	  printf("%s: Undelete %s? ", whoami, user_name);
	  if (! yes())
	       return(0);
     }
     if (! lstat(user_name, &stat_buf)) if (! force) {
	  printf("%s: An undeleted %s already exists.\n", whoami, user_name);
	  printf("Do you wish to continue with the undelete and overwrite that version? ");
	  if (! yes())
	       return(0);
	  unlink_completely(user_name);
     }
     if (noop) {
	  fprintf(stderr, "%s: %s would be undeleted\n", whoami, user_name);
	  return(0);
     }

     if (! do_file_rename(file_ent, real_name, user_name)) {
	  if (verbose)
	       printf("%s: %s undeleted\n", whoami, user_name);
	  return(0);
     }
     else
	  return(1);
}




do_file_rename(file_ent, real_name, user_name)
char *real_name, *user_name;
filerec *file_ent;
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
	  if (rename(old_name, new_name))
	       return(1);
	  if (ptr > new_name) {
	       *--ptr = '\0';
	       old_name[ptr - new_name] = '\0';
	  }
     }
     return(change_path(real_name, user_name));
     free_leaf(file_ent);
}






char *convert_to_user_name(real_name, user_name)
char real_name[];
char user_name[];  /* RETURN */
{
     char *ptr, *q;
     
     strcpy(user_name, real_name);
     ptr = user_name;
     while (ptr = strrindex(ptr, ".#")) {
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




match_pattern(base, ftype, file_pattern, num_recurs_found, recurs_found,
	      num_requested_found, requested_found);
char *base;
filetype ftype;
char *file_pattern;
char ***recurs_found, ***requested_found;
int *num_recurs_found, *num_requested_found;
{
     struct stat stat_buf;

     *recurs_found = (char **) malloc(0);
     *requested_found = (char **) malloc(0);
     *num_recurs_found = 0;
     *num_requested_found = 0;
     
     if (! *file_pattern) { /* The file pattern is empty, so we've */
			    /* reached the end of the line.  If we are */
			    /* not looking for recursive stuff, or if */
			    /* the base involved is a file, then */
			    /* we return only the base; otherwise, we */
			    /* recursively descend and return all of */
			    /* the descendents of this */
			    /* directory. */
	  if (is_deleted(lastpart(base))) {
	       *num_requested_found = 1;
	       *requested_found = (char **) realloc(*requested_found,
						    sizeof(char *));
	       **requested_found = malloc(strlen(base) + 1);
	       strcpy(**requested_found, base);
	  }
	  if ((! recursive) || (ftype != FtDirectory)) {
	       return(0);
	  }
	  else {
	       *requested_found = (char **) malloc(sizeof(char *));
	       **requested_found = malloc(strlen(base) + 1);
	       strcpy(**requested_found, base);
	       return (find_all_children(base, num_recurs_found,
					 recurs_found));
	  }
     }
     else if (ftype != FtDirectory) { /* A non-directory has been */
				      /* passed in even though the */
				      /* file pattern has not reached */
				      /* its end yet.  This is bad, so */
				      /* we return nothing. */
	  *num_recursed_found = 0;
	  *num_requested_found = 0;
	  *recurs_found = (char **) malloc(0);
	  *requested_found = (char **) malloc(0);
	  return(0);
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
	  char **new_requested, **new_recursed;
	  int num_new_requested, num_new_recursed;
	  
	  strcpy(pattern, firstpart(file_pattern, rest_of_pattern));
	  strcpy(del_pattern, DELETEREPREFIX);
	  strcat(del_pattern, pattern);

	  *num_recurs_found = 0;
	  *recurs_found = (char **) malloc(0);
	  
	  dirp = opendir(base);
	  if (! dirp) {
	       *num_requested_found = 0;
	       *requested_found = (char **) malloc(0);
	       return(1);
	  }

	  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	       if (is_dotfile(dp->d_name))
		    continue;
	       strcpy(newname, append(base, dp->d_name));
	       if (lstat(newname, &stat_buf))
		    continue;
	       re_comp(pattern);
	       if (re_exec(dp->d_name) {
		    newtype = ((stat_buf.st_mode & S_IFDIR) ? FtDirectory :
			       FtFile);
		    match_pattern(newname, newtype, rest_of_pattern,
				  &num_new_recursed, &new_recursed,
				  &num_new_requested, &new_requested);
		    
				  
		   
	  
	  
					  



		    
     

filecmp(file1, file2)
char *file1, *file2;
{
     char buf1[MAXPATHLEN];
     char buf2[MAXPATHLEN];
     
     convert_to_user_name(file1, buf1);
     convert_to_user_name(file2, buf2);
     return(strcmp(buf1, buf2));
}

     
     
sort_files(data, num_data)
char **data;
int num_data;
{
     qsort(data, num_data, sizeof(char *), filecmp);
}





unique(files, number)
char ***files;
int *number;
{
     char file1[MAXPATHLEN], file2[MAXPATHLEN];
     int i, last;
     
     convert_to_user_name(files[0], file1);
     for (last = 0, i = 1; i < *number; i++) {
	  convert_to_user_name(files[i], file2);
	  if (! strcmp(file1, file2)) {
	       int better;

	       better = choose_better(files[last], files[i]);
	       if (better == 1) /* the first one is better */
		    free (files[i]);
	       else {
		    free (files[last]);
		    last = i;
		    strcpy(file1, file2);
	       }
	  }
	  else {
	       last = i;
	       strcpy(file1, file2);
	  }
     }
     
     for (offset = 0, i = 0; i + offset < *number; i++) {
	  if (! files[i])
	       offset++;
	  if (i + offset < *number)
	       files[i] = files[i + offset];
     }
     *number -= offset;
     files = (char **) realloc(files, sizeof(char *) * (*number));
     if (! files) {
	  perror(sprintf(error_buf, "%s: Realloc'ing in unique.\n", whoami));
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
 * NOTE: Append uses a static array, so its return value must be
 * copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if ((! filename) || (! filepath)) {
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
