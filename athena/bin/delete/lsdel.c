/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v $
 * $Author: jik $
 *
 * This program is a replacement for rm.  Instead of actually deleting
 * files, it marks them for deletion by prefixing them with a ".#"
 * prefix.
 *
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_lsdel_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v 1.7 1989-10-23 13:35:27 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <strings.h>
#include <errno.h>
#include <com_err.h>
#include "col.h"
#include "util.h"
#include "directories.h"
#include "pattern.h"
#include "lsdel.h"
#include "shell_regexp.h"
#include "mit-copyright.h"
#include "delete_errs.h"
#include "errors.h"

char *malloc(), *realloc();
extern int current_time;
extern int errno;

int block_total = 0;
int dirsonly, recursive, timev, yield;

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     
     whoami = lastpart(argv[0]);

     dirsonly = recursive = timev = yield = 0;
     while ((arg = getopt(argc, argv, "drt:y")) != -1) {
	  switch (arg) {
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
	  default:
	       usage();
	       exit(1);
	  }
     }
     if (optind == argc) {
	  char *cwd;

	  cwd = ".";

	  if (ls(&cwd, 1))
	       error("ls of .");
     }
     if (ls(&argv[optind], argc - optind))
	  error ("ls");

     exit (error_occurred ? 1 : 0);
}






usage()
{
     printf("Usage: %s [ options ] [ filename [ ...]]\n", whoami);
     printf("Options are:\n");
     printf("     -d     list directory names, not contents\n");
     printf("     -r     recursive\n");
     printf("     -t n   list n-day-or-older files only\n");
     printf("     -y     report total space taken up by files\n");
}




ls(args, num)
char **args;
int num;
{
     char *start_dir;
     char **found_files;
     int num_found, total = 0;
     char *file_re;
     int status = 0;
     int retval;

     initialize_del_error_table();
     
     if (retval = initialize_tree()) {
	  error("initialize_tree");
	  return retval;
     }
     
     for ( ; num; num--) {
	  if (*args[num - 1] == '/') {
	       start_dir = "/";
	       file_re = args[num - 1] + 1;
	  }
	  else {
	       start_dir = "";
	       file_re = args[num - 1];
	  }

	  if (retval = get_the_files(start_dir, file_re, &num_found,
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
	        * file_re is.  There are several possible conditions:
		* 1. file_re has no wildcards in it, which means that
		*    if we couldn't find it, that means it doesn't
		*    exist.  Print a not found error.
		* 2. file_re is an existing directory, with no deleted
		*    files in it.  Print nothing.
		* 3. file_re doesn't exist, and there are wildcards in
		*    it.  Print "no match".
		* None of these are considered error conditions, so we
		* don't set the error flag.
		*/
	       if (no_wildcards(file_re)) {
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
		 (total == 1 ? "" : "s"), blk_to_k(block_total));

     return status;
}




int get_the_files(start_dir, file_re, number_found, found)
char *start_dir, *file_re;
int *number_found;
char ***found;
{
     char **matches;
     int num_matches;
     int num;
     int i;
     int retval;
     int status = 0;
     
     *found = (char **) malloc(0);
     num = 0;

     if (retval = find_matches(start_dir, file_re, &num_matches, &matches)) {
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
	       if (dirsonly)
		    continue;
	       if (lstat(matches[i], &stat_buf)) {
		    set_error(errno);
		    error(matches[i]);
		    status = error_code;
		    continue;
	       }
	       if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
		    if (retval = find_deleted_contents_recurs(matches[i],
				      &num_contents, &contents_found)) {
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
     free(matches);
     *number_found = num;
     return status;
}






process_files(files, num)
char **files;
int num;
{
     int i;
     filerec *leaf;
     
     for (i = 0; i < num; i++) {
	  if (add_path_to_tree(files[i], &leaf)) {
	       error("add_path_to_tree");
	       return -1;
	  }
	  free(files[i]);
	  if (! timed_out(leaf, current_time, timev)) {
	       free_leaf(leaf);
	       num--;
	       continue;
	  }
	  block_total += leaf->specs.st_blocks;
     }
     free(files);
     return(num);
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
     
     if (retval = column_array(strings, num, DEF_SCR_WIDTH, 0, 0, 2, 1, 0,
			       1, stdout)) {
	  error("column_array");
	  return retval;
     }
     
     for ( ; num; num--)
	  free(strings[num - 1]);
     free(strings);
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
     if (! files) {
	  set_error(errno);
	  error("realloc");
	  return errno;
     }

     *the_files = files;
     return 0;
}
