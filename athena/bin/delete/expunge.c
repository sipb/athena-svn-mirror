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
     static char rcsid_expunge_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/expunge.c,v 1.5 1989-03-08 09:58:52 jik Exp $";
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
#include "col.h"
#include "directories.h"
#include "util.h"
#include "pattern.h"
#include "expunge.h"

extern char *malloc(), *realloc();
extern int current_time;

char *whoami, *error_buf;

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
     timev = 0;
     yield = interactive = recursive = noop = verbose = listfiles = force = 0;
     while ((arg = getopt(argc, argv, "t:irfnvly")) != -1) {
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
	  dir = ".";
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
     timev = interactive = noop = verbose = force = 0;
     yield = listfiles = recursive = 1;
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
     printf("     -y     print yield of expunge\n");
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
	  if (num_found)
	       num_found = process_files(found_files, num_found);
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
			 fprintf(stderr, "%s: %s: not found\n",
				 whoami, files[num - 1]);
		    }
	       }
	       else {
		    fprintf(stderr, "%s: %s: no match\n", whoami,
			    files[num - 1]);
	       }
	  }
	  free(file_re);
     }
     if (total && listfiles) {
	  list_files();
	  if (! force) if (! top_level())
	       return(NO_DELETE_MASK);
     }
     current = get_root_tree();
     if (current)
	  status = status | expunge_specified(current);
     current = get_cwd_tree();
     if (current)
	  status = status | expunge_specified(current);

     if (yield) {
	  if (noop)
	       printf("Total that would be expunged: %dk\n",
		      blk_to_k(blocks_removed));
	  else
	       printf("Total expunged: %dk\n", blk_to_k(blocks_removed));
     }
     return(status);
}



expunge_specified(leaf)
filerec *leaf;
{
     int status = 0;

     if ((leaf->specified) && ((leaf->specs.st_mode & S_IFMT) == S_IFDIR))
	  status = do_directory_expunge(leaf);
     /* the "do_directory_expunge" really only asks the user if he */
     /* wants to expunge the directory, it doesn't do any deleting. */
     if (! status) {
	  if (leaf->dirs)
	       status |= expunge_specified(leaf->dirs);
	  if (leaf->files)
	       status |= expunge_specified(leaf->files);
     }
     if (leaf->specified)
	  status |= really_do_expunge(leaf);
     if (leaf->next)
	  status |= expunge_specified(leaf->next);
     free_leaf(leaf);
     return(status);
}


process_files(files, num)
char **files;
int num;
{
     int i;
     filerec *leaf;
     
     for (i = 0; i < num; i++) {
	  if (! (leaf = add_path_to_tree(files[i]))) {
	       fprintf(stderr, "%s: error adding path to filename tree\n",
		       whoami);
	       exit(1);
	  }

	  free(files[i]);
	  if (! timed_out(leaf, current_time, timev)) {
	       free_leaf(leaf);
	       num--;
	  }
     }
     free(files);
     return(num);
}









do_directory_expunge(file_ent)
filerec *file_ent;
{
     char buf[MAXPATHLEN];

     get_leaf_path(file_ent, buf);
     convert_to_user_name(buf, buf);
     
     if (interactive) {
	  printf("%s: Expunge directory %s? ", whoami, buf);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
     return(0);
}








really_do_expunge(file_ent)
filerec *file_ent;
{
     char real[MAXPATHLEN], user[MAXPATHLEN];
     int status;
     
     get_leaf_path(file_ent, real);
     convert_to_user_name(real, user);

     if (interactive) {
	  printf ("%s: Expunge %s (%dk)? ", whoami, user,
		  blk_to_k(file_ent->specs.st_blocks));
	  if (! yes())
	       return(NO_DELETE_MASK);
     }

     if (noop) {
	  blocks_removed += file_ent->specs.st_blocks;
	  printf("%s: %s (%dk) would be expunged (%dk total)\n", whoami, user,
		 blk_to_k(file_ent->specs.st_blocks),
		 blk_to_k(blocks_removed));
	  return(0);
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
	  return(0);
     }
     else {
	  if (! force)
	       fprintf(stderr, "%s: %s not expunged\n", whoami, user);
	  return(ERROR_MASK);
     }
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
     char **strings;
     int num;
     
     strings = (char **) malloc(sizeof(char *));
     num = 0;
     if (! strings) {
	  if (! force)
	       perror(sprintf(error_buf, "%s: list_files", whoami));
	  exit(1);
     }
     printf("The following deleted files are going to be expunged: \n\n");

     current = get_root_tree();
     strings = accumulate_names(current, strings, &num);
     current = get_cwd_tree();
     strings = accumulate_names(current, strings, &num);
     column_array(strings, num, DEF_SCR_WIDTH, 0, 0, 2, 1, 0, 1, stdout);
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
