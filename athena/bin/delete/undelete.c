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
     static char rcsid_undelete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v 1.12 1989-01-27 02:58:46 jik Exp $";
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

#define ERROR_MASK 1
#define NO_DELETE_MASK 2

char *malloc(), *realloc();

int interactive, recursive, verbose, directoriesonly, noop, force;
int del_recursive = 0; /* this tells the pattern matcher that we do */
		       /* *not* want it to recurse deleted directories */
		       /* when recursive is set to false. */

char *whoami, *error_buf;




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
	  default:
	       usage();
	       exit(1);
	  }
     }
     if (optind == argc)
	  exit(interactive_mode());
     else while (optind < argc) {
	  status = status | undelete(argv[optind]);
	  optind++;
     }
     exit(status & ERROR_MASK);
}



interactive_mode()
{
     char buf[MAXPATHLEN];
     char *ptr;
     int status = 0;

     if (verbose) {
	  printf("Enter the files to be undeleted, one file per line.\n");
	  printf("Hit <RETURN> on a line by itself to exit.\n\n");
     }
     do {
	  printf("%s: ", whoami);
	  ptr = fgets(buf, MAXPATHLEN, stdin);
	  if (! ptr) {
	       printf("\n");
	       return(status);
	  }
	  ptr = index(buf, '\n');  /* fgets breakage */
	  if (ptr)
	       *ptr = '\0';
	  if (! *buf)
	       return(status);
	  status = status | undelete(buf);
     } while (*ptr);
     return(status);
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
     found_files = get_the_files(startdir, file_re, &num_found);
     free(file_re);
     if (num_found) {
	  process_files(found_files, num_found);
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
int num;
{
     int i;
     listrec *new_files;
     listrec *filelist;

     filelist = (listrec *) malloc(sizeof(listrec) * num);
     if (! filelist) {
	  perror(sprintf(error_buf, "%s: process_files\n", whoami));
	  exit(1);
     }
     for (i = 0; i < num; i++) {
	  filelist[i].real_name = malloc(strlen(files[i]) + 1);
	  strcpy(filelist[i].real_name, files[i]);
	  filelist[i].user_name = malloc(strlen(files[i]) + 1);
	  convert_to_user_name(files[i], filelist[i].user_name);
	  free(files[i]);
     }
     free(files);
     
     new_files = sort_files(filelist, num);
     new_files = unique(new_files, &num);
     if (initialize_tree()) {
	  exit(1);
     }
     for (i = 0; i < num; i++) {
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




char **get_the_files(base, reg_exp, num_found)
char *base, *reg_exp;
int *num_found;
{
     char **matches;
     int num_matches;
     char **found;
     int num;
     int i;
     
     found = (char **) malloc(0);
     num = 0;
     
     matches = find_matches(base, reg_exp, &num_matches);
     if (recursive) {
	  char **recurs_found;
	  int recurs_num;
	  
	  for (i = 0; i < num_matches; free(matches[i]), i++) {
	       if (is_deleted(lastpart(matches[i]))) {
		    found = add_str(found, num, matches[i]);
		    num++;
	       }
	       recurs_found = find_deleted_recurses(matches[i], &recurs_num);
	       add_arrays(&found, &num, &recurs_found, &recurs_num);
	  }
     }
     else {
	  struct stat stat_buf;
	  char **contents_found;
	  int num_contents;
	  
	  for (i = 0; i < num_matches; free(matches[i]), i++) {
	       if (is_deleted(lastpart(matches[i]))) {
		    found = add_str(found, num, matches[i]);
		    num++;
	       }
	       else if (! directoriesonly) {
		    if (lstat(matches[i], &stat_buf))
			 continue;
		    if (stat_buf.st_mode & S_IFDIR) {
			 contents_found = find_deleted_contents(matches[i],
								&num_contents);
			 add_arrays(&found, &num, &contents_found,
				    &num_contents);
		    }
	       }
	  }
	  
     }
     free(matches);
     *num_found = num;
     return(found);
}

			 
		    
	  
	  
