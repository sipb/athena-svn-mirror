/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v $
 * $Author: jik $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_undelete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/undelete.c,v 1.25 1991-06-25 16:15:14 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <sys/param.h>
#ifdef SYSV
#include <string.h>
#define index strchr
#define rindex strrchr
#else
#include <strings.h>
#endif /* SYSV */
#include <com_err.h>
#include <errno.h>
#include "delete_errs.h"
#include "pattern.h"
#include "util.h"
#include "directories.h"
#include "undelete.h"
#include "shell_regexp.h"
#include "mit-copying.h"
#include "errors.h"

extern char *realloc();
extern int errno;

int interactive, recursive, verbose, directoriesonly, noop, force;


main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     int retval;
     
     initialize_del_error_table();
     
     whoami = lastpart(argv[0]);
     interactive = recursive = verbose = directoriesonly = noop = force = 0;

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
	       break;
	  default:
	       usage();
	       exit(1);
	  }
     }

     report_errors = ! force;
     
     if (optind == argc) {
	  if (interactive_mode())
	       error("interactive_mode");
     }
     else while (optind < argc) {
	  retval = undelete(argv[optind]);
	  if (retval)
	       error(argv[optind]);
	  optind++;
     }
     exit(((! force) && error_occurred) ? 1 : 0);
}



interactive_mode()
{
     char buf[MAXPATHLEN];
     char *ptr;
     int status = 0;
     int retval;
     
     if (verbose) {
	  printf("Enter the files to be undeleted, one file per line.\n");
	  printf("Hit <RETURN> on a line by itself to exit.\n\n");
     }
     do {
	  printf("%s: ", whoami);
	  ptr = fgets(buf, MAXPATHLEN, stdin);
	  if (! ptr) {
	       printf("\n");
	       return status;
	  }
	  ptr = index(buf, '\n');  /* fgets breakage */
	  if (ptr)
	       *ptr = '\0';
	  if (! *buf)
	       return status;
	  retval = undelete(buf);
	  if (retval) {
	       error(buf);
	       status = retval;
	  }
     } while (*buf);
     return status;
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

undelete(name)
char *name;
{
     char **found_files;
     int num_found;
     int status = 0;
     filerec *current;
     int retval;
     
     if (retval =  get_the_files(name, &num_found, &found_files)) {
	  error(name);
	  return retval;
     }
     
     if (num_found) {
	  if (retval = process_files(found_files, num_found)) {
	       error(name);
	       return retval;
	  }
	  if (*name == '/')
	       current = get_root_tree();
	  else
	       current = get_cwd_tree();

	  status = recurs_and_undelete(current);
	  if (status) {
	       error(name);
	       return status;
	  }
     }
     else {
	  if (no_wildcards(name)) {
	       set_error(ENOENT)
	  }
	  else
	       set_error(ENOMATCH);
	  error(name);
	  return error_code;
     }

     return status;
}





int recurs_and_undelete(leaf)
filerec *leaf;
{
     int status = 0;
     int retval;
     
     if (leaf->specified) {
	  retval = do_undelete(leaf);
	  if (retval) {
	       error("do_undelete");
	       status = retval;
	  }
     }

     if (! status) { /* recurse only if if top-level undelete */
		     /* succeeded or was not requested        */
	  if (leaf->dirs) {
	       retval = recurs_and_undelete(leaf->dirs);
	       if (retval) {
		    error("recurs_and_undelete");
		    status = retval;
	       }
	  }

	  if (leaf->files) {
	       retval = recurs_and_undelete(leaf->files);
	       if (retval) {
		    error("recurs_and_undelete");
		    status = retval;
	       }
	  }
     }

     if (leaf->next) {
	  retval = recurs_and_undelete(leaf->next);
	  if (retval) {
	       error("recurs_and_undelete");
	       status = retval;
	  }
     }

     free_leaf(leaf);

     return status;
}






int process_files(files, num)
char **files;
int num;
{
     int i;
     listrec *filelist;
     struct filrec *not_needed;
     int retval;
     
     filelist = (listrec *) Malloc((unsigned) (sizeof(listrec) * num));
#ifdef MALLOC_0_RETURNS_NULL
     if ((! filelist) && num)
#else
     if (! filelist)
#endif
     {
	  set_error(errno);
	  error("process_files");
	  return error_code;
     }
     
     for (i = 0; i < num; i++) {
	  filelist[i].real_name = Malloc((unsigned) (strlen(files[i]) + 1));
	  if (! filelist[i].real_name) {
	       set_error(errno);
	       error("process_files");
	       return error_code;
	  }
	  (void) strcpy(filelist[i].real_name, files[i]);
	  filelist[i].user_name = Malloc((unsigned) (strlen(files[i]) + 1));
	  if (! filelist[i].user_name) {
	       set_error(errno);
	       error("process_files");
	       return error_code;
	  }
 	  (void) convert_to_user_name(files[i], filelist[i].user_name);
	  free(files[i]);
     }
     free((char *) files);
	  
     if (retval = sort_files(filelist, num)) {
	  error("sort_files");
	  return retval;
     }
     if (retval = unique(&filelist, &num)) {
	  error("unique");
	  return retval;
     }
     if (retval = initialize_tree()) {
	  error("initialize_tree");
	  return retval;
     }
	  
     for (i = 0; i < num; i++) {
	  if (retval = add_path_to_tree(filelist[i].real_name, &not_needed)) {
	       error("add_path_to_tree");
	       return retval;
	  }
	  else {
	       free(filelist[i].real_name);
	       free(filelist[i].user_name);
	  }
     }
     free((char *) filelist);
     return 0;
}

     





     
do_undelete(file_ent)
filerec *file_ent;
{
     struct stat stat_buf;
     char user_name[MAXPATHLEN], real_name[MAXPATHLEN];
     int retval;
     
     if (retval = get_leaf_path(file_ent, real_name)) {
	  if (! force)
	       fprintf(stderr, "%s: %s: %s\n", whoami, "get_leaf_path",
		       error_message(retval));
	  return retval;
     }
     
     (void) convert_to_user_name(real_name, user_name);

     if (interactive) {
	  if ((file_ent->specs.st_mode & S_IFMT) == S_IFDIR)
	       printf("%s: Undelete directory %s? ", whoami, user_name);
	  else
	       printf("%s: Undelete %s? ", whoami, user_name);
	  if (! yes()) {
	       set_status(UNDEL_NOT_UNDELETED);
	       return error_code;
	  }
     }
     if (! lstat(user_name, &stat_buf)) if (! force) {
	  printf("%s: An undeleted %s already exists.\n", whoami, user_name);
	  printf("Do you wish to continue with the undelete and overwrite that version? ");
	  if (! yes()) {
	       set_status(UNDEL_NOT_UNDELETED);
	       return error_code;
	  }
	  if (! noop) if (retval = unlink_completely(user_name)) {
	       error(user_name);
	       return retval;
	  }
     }
     if (noop) {
	  printf("%s: %s would be undeleted\n", whoami, user_name);
	  return 0;
     }

     if (retval = do_file_rename(real_name, user_name)) {
	  error("do_file_rename");
	  return retval;
     }
     else {
	  if (verbose)
	       printf("%s: %s undeleted\n", whoami, user_name);
	  return 0;
     }
}




do_file_rename(real_name, user_name)
char *real_name, *user_name;
{
     char *ptr;
     int retval;
     char error_buf[MAXPATHLEN+MAXPATHLEN+14];
     char old_name[MAXPATHLEN], new_name[MAXPATHLEN];
     char buf[MAXPATHLEN];

     (void) strcpy(old_name, real_name);
     (void) strcpy(new_name, real_name);

     while (ptr = strrindex(new_name, ".#")) {
	  (void) convert_to_user_name(ptr, ptr);
	  (void) strcpy(ptr, firstpart(ptr, buf));
	  (void) strcpy(&old_name[ptr - new_name],
			firstpart(&old_name[ptr - new_name], buf));
	  if (rename(old_name, new_name)) {
	       set_error(errno);
	       (void) sprintf(error_buf, "renaming %s to %s",
			      old_name, new_name);
	       error(error_buf);
	       return error_code;
	  }
	  if (ptr > new_name) {
	       *--ptr = '\0';
	       old_name[ptr - new_name] = '\0';
	  }
     }
     if (retval = change_path(real_name, user_name)) {
	  error("change_path");
	  return retval;
     }
     
     return 0;
}






filecmp(file1, file2)
listrec *file1, *file2;
{
     return(strcmp(file1->user_name, file2->user_name));
}

     
     
int sort_files(data, num_data)
listrec *data;
int num_data;
{
     qsort((char *) data, num_data, sizeof(listrec), filecmp);

     return 0;
}





int unique(the_files, number)
listrec **the_files;
int *number;
{
     int i, last;
     int offset;
     listrec *files;

     files = *the_files;
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
     files = (listrec *) realloc((char *) files,
				 (unsigned) (sizeof(listrec) * *number));
#ifdef MALLOC_0_RETURNS_NULL
     if ((! files) && *number)
#else
     if (! files)
#endif
     {
	  set_error(errno);
	  error("realloc");
	  return errno;
     }

     *the_files = files;
     return 0;
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
#ifdef POSIX
     struct dirent *dp;
#else
     struct direct *dp;
#endif
     int retval;
     int status = 0;
     
     if (lstat(filename, &stat_buf)) {
	  set_error(errno);
	  error(filename);
	  return error_code;
     }

     if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
	  dirp = Opendir(filename);
	  if (! dirp) {
	       set_error(errno);
	       error(filename);
	       return error_code;
	  }
	       
	  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	       if (is_dotfile(dp->d_name))
		    continue;
	       (void) strcpy(buf, append(filename, dp->d_name));
	       if (retval = unlink_completely(buf)) {
		    error(buf);
		    status = retval;
	       }
	  }
	  closedir(dirp);
	  if (retval = rmdir(filename)) {
	       set_error(errno);
	       error(filename);
	       return error_code;
	  }
     }
     else if (retval = unlink(filename)) {
	  set_error(errno);
	  error(filename);
	  return error_code;
     }

     return status;
}




int get_the_files(name, num_found, found)
char *name;
int *num_found;
char ***found;
{
     int retval;
     int options;
     
     options = FIND_DELETED;
     if (recursive)
	  options |= RECURS_DELETED;
     if (! directoriesonly)
	  options |= FIND_CONTENTS;

     retval = find_matches(name, num_found, found, options);
     if (retval) {
	  error("find_matches");
	  return retval;
     }

     return 0;
}
