/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/expunge.c,v $
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
     static char rcsid_expunge_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/expunge.c,v 1.3 1989-01-27 08:24:09 jik Exp $";
#endif

/*
 * Things that need to be fixed later:
 *
 * 1. The program should somehow store the sizes of deleted files and
 * report the total amount of space regained after an expunge or purge.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>
#include "directories.h"
#include "util.h"
#include "pattern.h"
#include "expunge.h"

extern char *malloc(), *realloc();

char *whoami, *error_buf;

int  time,		/* minimum mod time before undeletion */
     interactive,	/* query before each expunge */
     recursive,		/* expunge undeleted directories recursively */
     noop,		/* print what would be done instead of doing it */
     verbose,		/* print a line as each file is deleted */
     force,		/* do not ask for any confirmation */
     listfiles;		/* list files at toplevel */
int del_recursive = 1;	/* this tells the pattern matcher that we want */
			/* it always to recurse on deleted */
			/* directories, even when recursive is set to */
			/* false. */


int directoriesonly = 0;  /* we don't use this flag, but the */
			  /* pattern-matching routines expect it to */
			  /* exist, so we create it and set it to */
			  /* false, which will cause the */
			  /* pattern-matching routines to ignore it. */




main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     int status = 0;

     whoami = lastpart(argv[0]);
     error_buf = malloc(strlen(whoami) + MAXPATHLEN + 3);
     if (! error_buf) {
	  perror(whoami);
	  exit(1);
     }
     if (*whoami == 'p') { /* we're doing a purge */
	  exit (purge());
     }
     time = 0;
     interactive = recursive = noop = verbose = listfiles = force = 0;
     while ((arg = getopt(argc, argv, "t:irfnvl")) != -1) {
	  switch (arg) {
	  case 't':
	       time = atoi(optarg);
	       break;
	  case 'i':
	       interactive++;
	       break;
	  case 'r':
	       recursive++;
	       break;
	  case 'f':
	       force++;
	       break;
	  case 'n':
	       noop++;
	       break;
	  case 'v':
	       verbose++;
	       break;
	  case 'l':
	       listfiles++;
	       break;
	  default:
	       usage();
	       exit(1);
	  }
     }
     if (optind == argc) {
	  char *dir;
	  dir = "";
	  status = status | expunge(&dir, 1); /* current working directory */
     }
     else
	  status = status | expunge(&argv[optind], argc - optind);
     exit(status & ERROR_MASK);
}





purge()
{
     char *home[1];

     home[0] = malloc(MAXPATHLEN);
     if (! home[0]) {
	  perror(sprintf(error_buf, "%s: purge", whoami));
	  exit(1);
     }
     time = interactive = noop = verbose = force = 0;
     listfiles = recursive = 1;
     get_home(home[0]);
     if (! *home[0]) {
	  fprintf(stderr, "%s: purge: can't get home directory\n", whoami);
	  exit(1);
     }

     printf("Please be patient.... this may take a while.\n\n");
     
     return(expunge(home, 1));
}




usage()
{
     printf("Usage: %s [ options ] [ filename [ ... ]]\n", whoami);
     printf("Options are:\n");
     printf("     -r     recursive\n");
     printf("     -i     interactive\n");
     printf("     -f     force\n");
     printf("     -t n   n-day-or-older expunge\n");
     printf("     -n     noop\n");
     printf("     -v     verbose\n");
     printf("     -l     list files before expunging\n");
     printf("     --     end options and start filenames\n");
}





expunge(files, num)
char **files;
int num;
{
     char *file_re;
     char **found_files;
     int num_found;
     char *start_dir;
     int status = 0;
     int total = 0;
     filerec *current;
     
     if (initialize_tree())
	  exit(1);

     for ( ; num ; num--) {
	  if (*files[num - 1] == '/') {
	       start_dir = "/";
	       file_re = parse_pattern(files[num - 1] + 1);
	  }
	  else {
	       start_dir = "";
	       file_re = parse_pattern(files[num - 1]);
	  }
	  if (! file_re)
	       return(ERROR_MASK);
	  
	  found_files = get_the_files(start_dir, file_re, &num_found);
	  free(file_re);
	  total += num_found;
	  if (num_found)
	       process_files(found_files, num_found);
	  else if (! force) {
	       fprintf(stderr, "%s: %s cannot be expunged\n",
		       whoami, files[num - 1]);
	       status |= ERROR_MASK;
	  }
     }
     if (total && listfiles) {
	  list_files();
	  if (! force) if (! top_level())
	       return(NO_DELETE_MASK);
     }
     current = next_specified_leaf(get_root_tree());
     if (current)
	  status = status | do_expunge(current);
     current = next_specified_leaf(get_cwd_tree());
     if (current)
	  status = status | do_expunge(current);

     return(status);
}





process_files(files, num)
char **files;
int num;
{
     int i;

     for (i = 0; i < num; i++) {
	  if (! add_path_to_tree(files[i], FtUnknown)) {
	       fprintf(stderr, "%s: error adding path to filename tree\n",
		       whoami);
	       exit(1);
	  }
	  else
	       free(files[i]);
     }
     free(files);
     return(0);
}




do_expunge(file_ent)
filerec *file_ent;
{
     int status = 0;
     filerec *newent;

     while(file_ent) {
	  if (file_ent->ftype == FtDirectory) {
	       status |= do_directory_expunge(file_ent);
	       newent = next_specified_directory(file_ent);
	  }
	  else {
	       status |= really_do_expunge(file_ent);
	       newent = next_specified_leaf(file_ent);
	  }
	  free_leaf(file_ent);
	  file_ent = newent;
     }
     return(status);
}









do_directory_expunge(file_ent)
filerec *file_ent;
{
     filerec *new_ent;
     int status = 0;
     char buf[MAXPATHLEN];

     get_leaf_path(file_ent, buf);
     convert_to_user_name(buf, buf);
     
     if (! timed_out(file_ent))
	  return(NO_TIMEOUT_MASK);
     if (interactive) {
	  printf("%s: Expunge directory %s? ", whoami, buf);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
	  
     new_ent = first_specified_in_directory(file_ent);
     status |= do_expunge(new_ent);
     status |= really_do_expunge(file_ent);
     return(status);
}








really_do_expunge(file_ent)
filerec *file_ent;
{
     char real[MAXPATHLEN], user[MAXPATHLEN];
     int status;
     
     get_leaf_path(file_ent, real);
     convert_to_user_name(real, user);

     if (! timed_out(file_ent))
	  return(NO_TIMEOUT_MASK);
     
     if (interactive) {
	  printf ("%s: Expunge %s? ", whoami, user);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }

     if (noop) {
	  printf("%s: %s would be expunged\n", whoami, user);
	  return(0);
     }

     if (file_ent->ftype == FtDirectory)
	  status = rmdir(real);
     else
	  status = unlink(real);
     if (! status) {
	  if (verbose)
	       printf("%s: %s expunged\n", whoami, user);
	  return(0);
     }
     else {
	  if (! force)
	       fprintf(stderr, "%s: %s not expunged\n", whoami, user);
	  return(ERROR_MASK);
     }
}









timed_out(file_ent)
filerec *file_ent;
{
     char buf[MAXPATHLEN];
     struct stat stat_buf;
     struct timeval tm;
     long diff;

     if (time == 0) /* catch the common default case to save time */
	  return (1);
     
     get_leaf_path(file_ent, buf);
     if (lstat(buf, &stat_buf))
	  return(1);
     gettimeofday(&tm, (struct timezone *) NULL);
     diff = (tm.tv_sec - stat_buf.st_mtime) / 86400;
     if (diff >= time)
	  return(1);
     else
	  return(0);
}





top_level()
{
     if (interactive) {
printf("The above files, which have been marked for deletion, are about to be\n");
printf("expunged forever!  You will be asked for confirmation before each file is\n");
printf("deleted.  Do you wish to continue? ");
     }
     else {
printf("The above files, which have been marked for deletion, are about to be\n");
printf("expunged forever!  Make sure you don't need any of them before continuing.\n");
printf("Do you wish to continue? ");
     }
     return (yes());
}

list_files()
{
     filerec *current;
     char buf[MAXPATHLEN];
     
     printf("The following deleted files are going to be expunged: \n\n");

     current = get_root_tree();
     while (current = next_specified_leaf(current)) {
	  get_leaf_path(current, buf);
	  convert_to_user_name(buf, buf);
	  printf("     %s\n", buf);
     }
     current = get_cwd_tree();
     while (current = next_specified_leaf(current)) {
	  get_leaf_path(current, buf);
	  convert_to_user_name(buf, buf);
	  printf("     %s\n", buf);
     }
     printf("\n");
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
	       if (lstat(matches[i], &stat_buf))
		    continue;
	       if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
		    contents_found = find_deleted_contents_recurs(matches[i],
							       &num_contents);
		    add_arrays(&found, &num, &contents_found,
			       &num_contents);
	       }
	  }
     }
     free(matches);
     *num_found = num;
     return(found);
}
