/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v $
 * $Author: jik $
 *
 * This program is a replacement for rm.  Instead of actually deleting
 * files, it marks them for deletion by prefixing them with a ".#"
 * prefix.
 *
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_lsdel_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v 1.19 1991-06-04 22:08:30 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#ifdef POSIX
#include <dirent.h>
#define direct dirent
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
#ifdef _AUX_SOURCE
extern char *strcmp();
#endif
#include <errno.h>
#include <com_err.h>
#include "col.h"
#include "util.h"
#include "directories.h"
#include "pattern.h"
#include "lsdel.h"
#include "shell_regexp.h"
#include "mit-copying.h"
#include "delete_errs.h"
#include "errors.h"

extern char *realloc();
extern time_t current_time;
extern int errno;

int space_total = 0;
int dirsonly, recursive, yield, f_links, f_mounts, singlecolumn;
time_t timev;

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     
     whoami = lastpart(argv[0]);

     dirsonly = recursive = timev = yield = f_links = f_mounts = 0;
     singlecolumn = -1;
     while ((arg = getopt(argc, argv, "1Cdrt:ysm")) != -1) {
	  switch (arg) {
	  case '1':
	       if ((singlecolumn != -1) && (! singlecolumn)) {
		    fprintf(stderr,
			    "%s: -1 and -C options are mutually exclusive\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       singlecolumn = 1;
	       break;
	  case 'C':
	       if ((singlecolumn != -1) && singlecolumn) {
		    fprintf(stderr,
			    "%s: -1 and -C options are mutually exclusive\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       singlecolumn = 0;
	       break;
	  case 'd':
	       dirsonly++;
	       break;
	  case 'r':
	       recursive++;
	       break;
	  case 't':
	       timev = atoi(optarg);
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
     if (singlecolumn == -1)
	  singlecolumn = ! isatty(1);

     if (optind == argc) {
	  char *cwd;

	  cwd = ".";

	  if (ls(&cwd, 1))
	       error("ls of .");
     }
     else if (ls(&argv[optind], argc - optind))
	  error ("ls");

     exit (error_occurred ? 1 : 0);
}






usage()
{
     fprintf(stderr, "Usage: %s [ options ] [ filename [ ...]]\n", whoami);
     fprintf(stderr, "Options are:\n");
     fprintf(stderr, "     -d     list directory names, not contents\n");
     fprintf(stderr, "     -r     recursive\n");
     fprintf(stderr, "     -t n   list n-day-or-older files only\n");
     fprintf(stderr, "     -y     report total space taken up by files\n");
     fprintf(stderr, "     -s     follow symbolic links to directories\n");
     fprintf(stderr, "     -m     follow mount points\n");
     fprintf(stderr, "     -1     force single-column output\n");
     fprintf(stderr, "     -C     force multi-column output (default when output is to a terminal)\n");
     fprintf(stderr, "-1 and -C are mutually exclusive\n");
}




ls(args, num)
char **args;
int num;
{
     char **found_files;
     int num_found = 0, total = 0;
     int status = 0;
     int retval;

     initialize_del_error_table();
     
     if (retval = initialize_tree()) {
	  error("initialize_tree");
	  return retval;
     }
     
     for ( ; num; num--) {
	  if (retval = get_the_files(args[num - 1], &num_found,
				     &found_files)) {
	       error(args[num - 1]);
	       status = retval;
	       continue;
	  }

	  if (num_found) {
	       num_found = process_files(found_files, num_found);
	       if (num_found < 0) {
		    error("process_files");
		    status = error_code;
		    continue;
	       }
	       total += num_found;
	  }
	  else {
	       /* What we do at this point depends on exactly what the
	        * filename is.  There are several possible conditions:
		* 1. The filename has no wildcards in it, which means that
		*    if we couldn't find it, that means it doesn't
		*    exist.  Print a not found error.
		* 2. Filename is an existing directory, with no deleted
		*    files in it.  Print nothing.
		* 3. Filename doesn't exist, and there are wildcards in
		*    it.  Print "no match".
		* None of these are considered error conditions, so we
		* don't set the error flag.
		*/
	       if (no_wildcards(args[num - 1])) {
		    if (! directory_exists(args[num - 1])) {
			 set_error(ENOENT);
			 error(args[num - 1]);
			 status = error_code;
			 continue;
		    }
	       }
	       else {
		    set_error(ENOMATCH);
		    error(args[num - 1]);
		    status = error_code;
		    continue;
	       }
	  }
     }
     if (total) {
	  if (list_files()) {
	       error("list_files");
	       return error_code;
	  }
     }
     if (yield)
	  printf("\nTotal space taken up by file%s: %dk\n",
		 (total == 1 ? "" : "s"), space_to_k(space_total));

     return status;
}




int get_the_files(name, number_found, found)
char *name;
int *number_found;
char ***found;
{
     int retval;
     int options;
     
     options = FIND_DELETED | FIND_CONTENTS;
     if (recursive)
	  options |= RECURS_FIND_DELETED;
     if (! dirsonly)
	  options |= RECURS_DELETED;
     if (f_mounts)
	  options |= FOLLW_MOUNTPOINTS;
     if (f_links)
	  options |= FOLLW_LINKS;
     
     retval = find_matches(name, number_found, found, options);
     if (retval) {
	  error("find_matches");
	  return retval;
     }

     return 0;
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
	       continue;
	  }
	  space_total += specs_to_space(leaf->specs);
     }
     free((char *) files);
     return(num-skipped);
}




static int alphacmp(str1, str2)
char **str1, **str2;
{
     return(strcmp(*str1, *str2));
}

list_files()
{
     filerec *current;
     char **strings;
     int num;
     int retval;
     
     strings = (char **) Malloc((unsigned) sizeof(char *));
     num = 0;
     if (! strings) {
	  set_error(errno);
	  error("Malloc");
	  return error_code;
     }
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

     if (retval = sort_files(strings, num)) {
	  error("sort_files");
	  return retval;
     }
     
     if (retval = unique(&strings, &num)) {
	  error("unique");
	  return retval;
     }

     qsort((char *) strings, num, sizeof(char *), alphacmp);

     if (retval = column_array(strings, num, DEF_SCR_WIDTH, 0, singlecolumn,
			       2, 1, 0, 1, stdout)) {
	  error("column_array");
	  return retval;
     }
     
     for ( ; num; num--)
	  free(strings[num - 1]);
     free((char *) strings);
     return 0;
}


int sort_files(data, num_data)
char **data;
int num_data;
{
     qsort((char *) data, num_data, sizeof(char *), strcmp);

     return 0;
}


int unique(the_files, number)
char ***the_files;
int *number;
{
     int i, last;
     int offset;
     char **files;

     files = *the_files;
     for (last = 0, i = 1; i < *number; i++) {
	  if (! strcmp(files[last], files[i])) {
	       free (files[i]);
	       free (files[i]);
	       files[i] = (char *) NULL;
	  }
	  else
	       last = i;
     }
     
     for (offset = 0, i = 0; i + offset < *number; i++) {
	  if (! files[i])
	       offset++;
	  if (i + offset < *number)
	       files[i] = files[i + offset];
     }
     *number -= offset;
     files = (char **) realloc((char *) files,
			       (unsigned) (sizeof(char *) * *number));
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
