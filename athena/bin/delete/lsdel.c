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
     static char rcsid_lsdel_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v 1.2 1989-02-01 03:42:08 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <strings.h>
#include "col.h"
#include "util.h"
#include "directories.h"
#include "pattern.h"
#include "lsdel.h"

char *malloc(), *realloc();
extern int current_time;

int block_total = 0;
int dirsonly, recursive, timev, yield;
char *whoami, *error_buf;

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;

     whoami = lastpart(argv[0]);
     error_buf = malloc(strlen(whoami) + MAXPATHLEN + 3);
     if (! error_buf) {
	  perror(whoami);
	  exit(1);
     }
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
	  exit(ls(&cwd, 1));
     }
     exit(ls(&argv[optind], argc - optind));
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
     
     if (initialize_tree())
	  exit(1);
     
     for ( ; num; num--) {
	  if (*args[num - 1] == '/') {
	       start_dir = "/";
	       file_re = parse_pattern(args[num - 1] + 1);
	  }
	  else {
	       start_dir = "";
	       file_re = parse_pattern(args[num - 1]);
	  }
	  if (! file_re)
	       return(ERROR_MASK);

	  found_files = get_the_files(start_dir, file_re, &num_found);
	  free(file_re);
	  total += num_found;
	  if (num_found)
	       num_found = process_files(found_files, num_found);
	  if (! num_found) {
	       fprintf(stderr, "%s: %s: nothing to list\n",
		       whoami, (*args[num - 1] == '\0' ? "." : args[num - 1]));
	       status |= ERROR_MASK;
	  }
     }
     if (total) {
	  list_files();
     }
     if (yield)
	  printf("\nTotal space taken up by file%s: %dk\n",
		 (total == 1 ? "" : "s"), blk_to_k(block_total));
     return(status);
}




char **get_the_files(start_dir, file_re, number_found)
char *start_dir, *file_re;
int *number_found;
{
     char **matches;
     int num_matches;
     char **found;
     int num;
     int i;

     found = (char **) malloc(0);
     num = 0;

     matches = find_matches(start_dir, file_re, &num_matches);
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
	       if (dirsonly)
		    continue;
	       if (lstat(matches[i], &stat_buf))
		    continue;
	       if ((stat_buf.st_mode &S_IFMT) == S_IFDIR) {
		    contents_found = find_deleted_contents_recurs(matches[i],
							  &num_contents);
		    add_arrays(&found, &num, &contents_found, &num_contents);
		    
	       }
	  }
     }
     free(matches);
     *number_found = num;
     return(found);
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

     strings = (char **) malloc(sizeof(char *));
     num = 0;
     if (! strings) {
	  perror(sprintf(error_buf, "%s: list_files", whoami));
	  exit(1);
     }
     current = get_root_tree();
     strings = accumulate_names(current, strings, &num);
     current = get_cwd_tree();
     strings = accumulate_names(current, strings, &num);
     column_array(strings, num, DEF_SCR_WIDTH, 0, 0, 2, 1, 0, 1, stdout);
     for ( ; num; num--)
	  free(strings[num - 1]);
     free(strings);
     return(0);
}
