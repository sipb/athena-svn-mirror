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
     static char rcsid_expunge_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/expunge.c,v 1.10 1989-10-23 13:37:44 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>
#include <com_err.h>
#include <errno.h>
#include "col.h"
#include "directories.h"
#include "util.h"
#include "pattern.h"
#include "expunge.h"
#include "shell_regexp.h"
#include "mit-copyright.h"
#include "delete_errs.h"
#include "errors.h"

extern char *malloc(), *realloc();
extern long current_time;
extern int errno;

char *whoami;

int  timev,		/* minimum mod time before undeletion */
     interactive,	/* query before each expunge */
     recursive,		/* expunge undeleted directories recursively */
     noop,		/* print what would be done instead of doing it */
     verbose,		/* print a line as each file is deleted */
     force,		/* do not ask for any confirmation */
     listfiles,		/* list files at toplevel */
     yield;		/* print yield of expunge at end */

int blocks_removed = 0;




main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;

     initialize_del_error_table();
     
     whoami = lastpart(argv[0]);
     if (*whoami == 'p') { /* we're doing a purge */
	  if (purge())
	       error("purge");
	  exit(error_occurred ? 1 : 0);
     }
     timev = 0;
     yield = interactive = recursive = noop = verbose = listfiles = force = 0;
     while ((arg = getopt(argc, argv, "t:irfnvly")) != EOF) {
	  switch (arg) {
	  case 't':
	       timev = atoi(optarg);
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
	  case 'y':
	       yield++;
	       break;
	  default:
	       usage();
	       exit(1);
	  }
     }
     if (optind == argc) {
	  char *dir;
	  dir = "."; /* current working directory */
	  if (expunge(&dir, 1))
	       error("expunging .");
     }
     else if (expunge(&argv[optind], argc - optind))
	  error("expunge");

     exit((error_occurred && (! force)) ? 1 : 0);
}





purge()
{
     char *home;
     int retval;
     
     home = malloc(MAXPATHLEN);
     if (! home) {
	  set_error(errno);
	  error("purge");
	  return error_code;
     }
     timev = interactive = noop = verbose = force = 0;
     yield = listfiles = recursive = 1;
     if (retval = get_home(home)) {
	  error("purge");
	  return retval;
     }

     printf("Please be patient.... this may take a while.\n\n");

     if (retval = expunge(&home, 1)) {
	  error("expunge");
	  return retval;
     }
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
     printf("     -y     print yield of expunge\n");
     printf("     --     end options and start filenames\n");
}





int expunge(files, num)
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
     int retval;
     
     if (initialize_tree())
	  exit(1);

     for ( ; num ; num--) {
	  if (*files[num - 1] == '/') {
	       start_dir = "/";
	       file_re = files[num - 1] + 1;
	  }
	  else {
	       start_dir = "";
	       file_re = files[num - 1];
	  }
	  
	  retval = get_the_files(start_dir, file_re, &num_found, &found_files);
	  if (retval) {
	       error(files[num - 1]);
	       return retval;
	  }
	       
	  if (num_found) {
	       num_found = process_files(found_files, num_found);
	       if (num_found < 0) {
		    error("process_files");
		    return error_code;
	       }
	  }
	  
	  total += num_found;
	  if (! num_found) if (! force) {
	       /*
		* There are three different situations here.  Eiter we
		* are dealing with an existing directory with no
	        * deleted files in it, or we are deleting with a
	        * non-existing deleted file with wildcards, or we are
	        * dealing with a non-existing deleted file without
	        * wildcards.  In the former case we print nothing, and
	        * in the latter cases we print either "no match" or
	        * "not found" respectively
		*/
	       if (no_wildcards(file_re)) {
		    if (! directory_exists(files[num - 1])) {
			 set_error(ENOENT);
			 error(files[num - 1]);
		    }
	       }
	       else {
		    set_error(ENOMATCH);
		    error(files[num - 1]);
	       }
	  }
     }
     if (total && listfiles) {
	  if (retval = list_files()) {
	       error("list_files");
	       return retval;
	  }
	  if (! force) if (! top_level()) {
	       set_status(EXPUNGE_NOT_EXPUNGED);
	       return error_code;
	  }
     }
     current = get_root_tree();
     if (current) {
	  if (retval = expunge_specified(current)) {
	       error("expunge_specified");
	       status = retval;
	  }
     }
     current = get_cwd_tree();
     if (current) {
	  if (retval = expunge_specified(current)) {
	       error("expunge_specified");
	       status = retval;
	  }
     }
     if (yield) {
	  if (noop)
	       printf("Total that would be expunged: %dk\n",
		      blk_to_k(blocks_removed));
	  else
	       printf("Total expunged: %dk\n", blk_to_k(blocks_removed));
     }
     return status;
}



expunge_specified(leaf)
filerec *leaf;
{
     int status = 0;
     int do_it = 1;
     int retval;
     
     if ((leaf->specified) && ((leaf->specs.st_mode & S_IFMT) == S_IFDIR)) {
	  char buf[MAXPATHLEN];

	  if (retval = get_leaf_path(leaf, buf)) {
	       error("get_leaf_path");
	       return retval;
	  }
	  convert_to_user_name(buf, buf);

	  if (interactive) {
	       printf("%s: Expunge directory %s? ", whoami, buf);
	       status = (! (do_it = yes()));
	  }
     }
     if (do_it) {
	  if (leaf->dirs) {
	       if (retval = expunge_specified(leaf->dirs)) {
		    error("expunge_specified");
		    status = retval;
	       }
	  }
	  if (leaf->files) {
	       if (retval = expunge_specified(leaf->files)) {
		    error("expunge_specified");
		    status = retval;
	       }
	  }
     }
     if (leaf->specified && (! status)) {
	  if (retval = really_do_expunge(leaf)) {
	       error("really_do_expunge");
	       status = retval;
	  }
     }
     if (leaf->next) {
	  if (retval = expunge_specified(leaf->next)) {
	       error("expunge_specified");
	       status = retval;
	  }
     }

     free_leaf(leaf);
     return status;
}


process_files(files, num)
char **files;
int num;
{
     int i, skipped = 0;
     filerec *leaf;
     
     for (i = 0; i < num; i++) {
	  if (add_path_to_tree(files[i], &leaf)) {
	       error("add_path_to_tree");
	       return -1;
	  }
	  free(files[i]);
	  if (! timed_out(leaf, current_time, timev)) {
	       free_leaf(leaf);
	       skipped++;
	  }
     }
     free((char *) files);
     return(num-skipped);
}









really_do_expunge(file_ent)
filerec *file_ent;
{
     char real[MAXPATHLEN], user[MAXPATHLEN];
     int status;
     int retval;
     
     if (retval = get_leaf_path(file_ent, real)) {
	  error("get_leaf_path");
	  return retval;
     }
     convert_to_user_name(real, user);

     if (interactive) {
	  printf ("%s: Expunge %s (%dk)? ", whoami, user,
		  blk_to_k(file_ent->specs.st_blocks));
	  if (! yes()) {
	       set_status(EXPUNGE_NOT_EXPUNGED);
	       return error_code;
	  }
     }

     if (noop) {
	  blocks_removed += file_ent->specs.st_blocks;
	  printf("%s: %s (%dk) would be expunged (%dk total)\n", whoami, user,
		 blk_to_k(file_ent->specs.st_blocks),
		 blk_to_k(blocks_removed));
	  return 0;
     }

     if ((file_ent->specs.st_mode & S_IFMT) == S_IFDIR)
	  status = rmdir(real);
     else
	  status = unlink(real);
     if (! status) {
	  blocks_removed += file_ent->specs.st_blocks;
	  if (verbose)
	       printf("%s: %s (%dk) expunged (%dk total)\n", whoami, user,
		      blk_to_k(file_ent->specs.st_blocks),
		      blk_to_k(blocks_removed));
	  return 0;
     }
     else {
	  set_error(errno);
	  error(real);
	  return error_code;
     }
}









top_level()
{
     if (interactive) {
printf("The above files, which have been marked for deletion, are about to be\n");
printf("expunged forever!  You will be asked for confirmation before each file is\n");
printf("deleted.  Do you wish to continue [return = no]? ");
     }
     else {
printf("The above files, which have been marked for deletion, are about to be\n");
printf("expunged forever!  Make sure you don't need any of them before continuing.\n");
printf("Do you wish to continue [return = no]? ");
     }
     return (yes());
}





list_files()
{
     filerec *current;
     char **strings;
     int num;
     int retval;
     
     strings = (char **) malloc(sizeof(char *));
     num = 0;
     if (! strings) {
	  set_error(errno);
	  error("malloc");
	  return error_code;
     }

     printf("The following deleted files are going to be expunged: \n\n");

     current = get_root_tree();
     if (retval = accumulate_names(current, &strings, &num)) {
	  error("accumulate_names");
	  return retval;
     }
     current = get_cwd_tree();
     if (retval = accumulate_names(current, &strings, &num)) {
	  error("accumulate_names");
	  return retval;
     }
     if (retval = column_array(strings, num, DEF_SCR_WIDTH, 0, 0, 2, 1, 0,
			       1, stdout)) {
	  error("column_array");
	  return retval;
     }
     
     printf("\n");
     return(0);
}
     




int get_the_files(base, reg_exp, num_found, found)
char *base, *reg_exp;
int *num_found;
char ***found;
{
     char **matches;
     int num_matches;
     int num;
     int i;
     int retval;
     
     *found = (char **) malloc(0);
     num = 0;
     
     if (retval = find_matches(base, reg_exp, &num_matches, &matches)) {
	  error("find_matches");
	  return retval;
     }
	       
     if (recursive) {
	  char **recurs_found;
	  int recurs_num;
	  
	  for (i = 0; i < num_matches; free(matches[i]), i++) {
	       if (is_deleted(lastpart(matches[i]))) {
		    if (retval = add_str(found, num, matches[i])) {
			 error("add_str");
			 return retval;
		    }
		    num++;
	       }
	       if (retval = find_deleted_recurses(matches[i], &recurs_num,
						  &recurs_found)) {
		    error("find_deleted_recurses");
		    return retval;
	       }
	       if (retval = add_arrays(found, &num, &recurs_found,
				       &recurs_num)) {
		    error("add_arrays");
		    return retval;
	       }
	  }
     }	
     else {
	  struct stat stat_buf;
	  char **contents_found;
	  int num_contents;
	  
	  for (i = 0; i < num_matches; free(matches[i]), i++) {
	       if (is_deleted(lastpart(matches[i]))) {
		    if (retval = add_str(found, num, matches[i])) {
			 error("add_str");
			 return retval;
		    }
		    num++;
	       }
	       if (lstat(matches[i], &stat_buf))
		    continue;
	       if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
		    if (retval =
			find_deleted_contents_recurs(matches[i], &num_contents,
						     &contents_found)) {
			 error("find_deleted_contents_recurs");
			 return retval;
		    }
		    if (retval = add_arrays(found, &num, &contents_found,
					    &num_contents)) {
			 error("add_arrays");
			 return retval;
		    }
	       }
	  }
     }
     free((char *) matches);
     *num_found = num;
     return 0;
}
