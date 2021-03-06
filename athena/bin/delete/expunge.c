/*
 * $Id: expunge.c,v 1.25 1999-01-22 23:08:59 ghudson Exp $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/param.h>
#include <string.h>
#include <com_err.h>
#include <errno.h>
#include <unistd.h>
#include "col.h"
#include "directories.h"
#include "pattern.h"
#include "expunge.h"
#include "shell_regexp.h"
#include "mit-copying.h"
#include "delete_errs.h"
#include "errors.h"
#include "util.h"

static void usage(void);
static int purge(void), expunge(char **files, int num);
static int process_files(char **files, int num), list_files(void);
static int top_level(void), expunge_specified(filerec *leaf);
static int really_do_expunge(filerec *file_ent);

extern time_t current_time;

extern char *whoami;

time_t timev; 		/* minimum mod time before undeletion */

int  interactive,	/* query before each expunge */
     recursive,		/* expunge undeleted directories recursively */
     noop,		/* print what would be done instead of doing it */
     verbose,		/* print a line as each file is deleted */
     force,		/* do not ask for any confirmation */
     listfiles,		/* list files at toplevel */
     yield,		/* print yield of expunge at end */
     f_links,		/* follow symbolic links */
     f_mounts;		/* follow mount points */

int space_removed = 0;




int main(int argc, char *argv[])
{
     extern char *optarg;
     extern int optind;
     int arg;

#if defined(__APPLE__) && defined(__MACH__)
     add_error_table(&et_del_error_table);
#else
     initialize_del_error_table();
#endif
     
     whoami = lastpart(argv[0]);
     if (*whoami == 'p') { /* we're doing a purge */
	  if (argc > 1) {
	       set_error(PURGE_TOO_MANY_ARGS);
	       error("");
	       exit(1);
	  }
	  if (purge())
	       error("purge");
	  exit(error_occurred ? 1 : 0);
     }
     timev = 0;
     yield = interactive = recursive = noop = verbose = listfiles = force = 0;
     while ((arg = getopt(argc, argv, "t:irfnvlysm")) != EOF) {
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
	  case 's':
	       f_links++;
	       break;
	  case 'm':
	       f_mounts++;
	       break;
	  default:
	       usage();
	       exit(1);
	  }
     }
     report_errors = ! force;
     
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





static int purge()
{
     char *home;
     int retval;
     
     home = Malloc((unsigned) MAXPATHLEN);
     if (! home) {
	  set_error(errno);
	  error("purge");
	  return error_code;
     }
     timev = interactive = noop = verbose = force = 0;
     yield = listfiles = recursive = 1;
     if ((retval = get_home(home))) {
	  error("purge");
	  return retval;
     }

     printf("Please be patient.... this may take a while.\n\n");

     if ((retval = expunge(&home, 1))) {
	  error("expunge");
	  return retval;
     }
     return 0;
}




static void usage()
{
     fprintf(stderr, "Usage: %s [ options ] [ filename [ ... ]]\n", whoami);
     fprintf(stderr, "Options are:\n");
     fprintf(stderr, "     -r     recursive\n");
     fprintf(stderr, "     -i     interactive\n");
     fprintf(stderr, "     -f     force\n");
     fprintf(stderr, "     -t n   n-day-or-older expunge\n");
     fprintf(stderr, "     -n     noop\n");
     fprintf(stderr, "     -v     verbose\n");
     fprintf(stderr, "     -l     list files before expunging\n");
     fprintf(stderr, "     -s     follow symbolic links to directories\n");
     fprintf(stderr, "     -m     follow mount points\n");
     fprintf(stderr, "     -y     print yield of expunge\n");
     fprintf(stderr, "     --     end options and start filenames\n");
}





static int expunge(char **files, int num)
{
     char **found_files;
     int num_found;
     int status = 0;
     int total = 0;
     filerec *current;
     int retval;
     
     if (initialize_tree())
	  exit(1);

     for ( ; num ; num--) {
	  retval = get_the_files(files[num - 1], &num_found, &found_files);
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
	       if (no_wildcards(files[num - 1])) {
		    if (! directory_exists(files[num - 1])) {
			 set_error(ENOENT);
			 error(files[num - 1]);
		    }
	       }
	       else {
		    set_error(DELETE_ENOMATCH);
		    error(files[num - 1]);
	       }
	  }
     }
     if (total && listfiles) {
       if ((retval = list_files())) {
	 error("list_files");
	 return retval;
       }
       if (! (force || top_level())) {
	 set_status(EXPUNGE_NOT_EXPUNGED);
	 return error_code;
       }
     }
     current = get_root_tree();
     if (current) {
       if ((retval = expunge_specified(current))) {
	 error("expunge_specified");
	 status = retval;
       }
     }
     current = get_cwd_tree();
     if (current) {
       if ((retval = expunge_specified(current))) {
	 error("expunge_specified");
	 status = retval;
       }
     }
     if (yield) {
       char *friendly = space_to_friendly(space_removed);
       if (noop)
	 printf("Total that would be expunged: %s\n", friendly);
       else
	 printf("Total expunged: %s\n", friendly);
       free(friendly);
     }
     return status;
}



static int expunge_specified(filerec *leaf)
{
     int status = 0;
     int do_it = 1;
     int retval;
     
     if ((leaf->specified) && ((leaf->specs.st_mode & S_IFMT) == S_IFDIR)) {
	  /*
	   * This is static so that we don't create a copy of it for
	   * every recursive invocation of expunge_specified.
	   */
	  static char buf[MAXPATHLEN];

	  if ((retval = get_leaf_path(leaf, buf))) {
	       error("get_leaf_path");
	       return retval;
	  }
	  (void) convert_to_user_name(buf, buf);

	  if (interactive) {
	       printf("%s: Expunge directory %s? ", whoami, buf);
	       status = (! (do_it = yes()));
	  }
     }
     if (do_it) {
	  if (leaf->dirs) {
	    if ((retval = expunge_specified(leaf->dirs))) {
	      error("expunge_specified");
	      status = retval;
	    }
	  }
	  if (leaf->files) {
	    if ((retval = expunge_specified(leaf->files))) {
	      error("expunge_specified");
	      status = retval;
	    }
	  }
     }
     if (leaf->specified && (! status)) {
       if ((retval = really_do_expunge(leaf))) {
	 error("really_do_expunge");
	 status = retval;
       }
     }
     if (leaf->next) {
       if ((retval = expunge_specified(leaf->next))) {
	 error("expunge_specified");
	 status = retval;
       }
     }

     free_leaf(leaf);
     return status;
}


static int process_files(char **files, int num)
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









static int really_do_expunge(filerec *file_ent)
{
     char real[MAXPATHLEN], user[MAXPATHLEN];
     int status;
     int retval;
     
     if ((retval = get_leaf_path(file_ent, real))) {
	  error("get_leaf_path");
	  return retval;
     }
     (void) convert_to_user_name(real, user);

     if (interactive) {
       char *friendly = specs_to_friendly(file_ent->specs);
       printf ("%s: Expunge %s (%s)? ", whoami, user, friendly);
       free(friendly);
	  if (! yes()) {
	       set_status(EXPUNGE_NOT_EXPUNGED);
	       return error_code;
	  }
     }

     if (noop) {
	  space_removed += specs_to_space(file_ent->specs);
	  char *friendly = space_to_friendly(space_removed);
	  char *friendly2 = specs_to_friendly(file_ent->specs);
	  printf("%s: %s (%s) would be expunged (%s total)\n", whoami, user,
		 friendly2, friendly);
	  free(friendly);
	  free(friendly2);
	  return 0;
     }

     if ((file_ent->specs.st_mode & S_IFMT) == S_IFDIR)
	  status = rmdir(real);
     else
	  status = unlink(real);
     if (! status) {
	  space_removed += specs_to_space(file_ent->specs);
	  if (verbose) {
	    char *friendly = space_to_friendly(space_removed);
	    char *friendly2 = specs_to_friendly(file_ent->specs);
	    printf("%s: %s (%s) expunged (%s total)\n", whoami, user,
		   friendly2, friendly);
	    free(friendly);
	    free(friendly2);
	  }
	  return 0;
     }
     else {
	  set_error(errno);
	  error(real);
	  return error_code;
     }
}









static int top_level()
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





static int list_files()
{
     filerec *current;
     char **strings;
     int num;
     int retval;
     
     strings = (char **) Malloc(sizeof(char *));
     num = 0;
     if (! strings) {
	  set_error(errno);
	  error("Malloc");
	  return error_code;
     }

     printf("The following deleted files are going to be expunged: \n\n");

     current = get_root_tree();
     if ((retval = accumulate_names(current, &strings, &num))) {
	  error("accumulate_names");
	  return retval;
     }
     current = get_cwd_tree();
     if ((retval = accumulate_names(current, &strings, &num))) {
	  error("accumulate_names");
	  return retval;
     }
     if ((retval = column_array(strings, num, DEF_SCR_WIDTH, 0, 0, 2, 1, 0,
				1, stdout))) {
	  error("column_array");
	  return retval;
     }
     
     printf("\n");
     return(0);
}
     




int get_the_files(name, num_found, found)
char *name;
int *num_found;
char ***found;
{
     int retval;
     int options;
     
     options = FIND_DELETED | FIND_CONTENTS | RECURS_DELETED;
     if (recursive)
	  options |= RECURS_FIND_DELETED;
     if (f_mounts)
	  options |= FOLLW_MOUNTPOINTS;
     if (f_links)
	  options |= FOLLW_LINKS;
     
     retval = find_matches(name, num_found, found, options);
     if (retval) {
	  error("find_matches");
	  return retval;
     }

     return 0;
}
