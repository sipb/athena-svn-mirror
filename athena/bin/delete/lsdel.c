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
     static char rcsid_lsdel_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/lsdel.c,v 1.1 1989-01-27 10:17:37 jik Exp $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <strings.h>
#include "lsdel.h"
#include "util.h"
#include "directories.h"
#include "pattern.h"

char *malloc(), *realloc();
int column_array();

int dirsonly, marktype, recursive;
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
     dirsonly = marktype = recursive = 0;
     while ((arg = getopt(argc, argv, "dr")) != -1) {
	  switch (arg) {
	  case 'd':
	       dirsonly++;
	       break;
	  case 'r':
	       recursive++;
	       break;
	  default:
	       usage();
	       exit(1);
	  }
     }
     if (optind == argc) {
	  fprintf(stderr, "%s: no files specified.\n", whoami);
	  usage();
	  exit(1);
     }
     exit(ls(&argv[optind], argc - optind));
}






usage()
{
     printf("Usage: %s [ options ] filename ...\n", whoami);
     printf("Options are:\n");
     printf("     -d     list directory names, not contents\n");
     printf("     -r     recursive\n");
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
	       process_files(found_files, num_found);
	  else {
	       fprintf(stderr, "%s: %s: no match\n",
		       whoami, args[num - 1]);
	       status |= ERROR_MASK;
	  }
     }
     if (total) {
	  list_files();
     }
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





list_files()
{
     filerec *current;
     char **strings;
     int num;
     char newname[MAXPATHLEN];
     
     strings = (char **) malloc(0);
     num = 0;
     
     current = get_root_tree();
     if (! current->specified)
	  current = next_specified_leaf(current);
     while (current) {
	  convert_to_user_name(get_leaf_path(current, newname), newname);
	  num += 1;
	  strings = (char **) realloc(strings, sizeof(char *) * num);
	  strings[num - 1] = malloc(strlen(newname) + 1);
	  strcpy(strings[num - 1], newname);
	  current = next_specified_leaf(current);
     }
     current = get_cwd_tree();
     if (! current->specified)
	  current = next_specified_leaf(current);
     while (current) {
	  convert_to_user_name(get_leaf_path(current, newname), newname);
	  num += 1;
	  strings = (char **) realloc(strings, sizeof(char *) * num);
	  strings[num - 1] = malloc(strlen(newname) + 1);
	  strcpy(strings[num - 1], newname);
	  current = next_specified_leaf(current);
     }
     column_array(strings, num, DEF_SCREEN_WIDTH, 0, 0, 2, 1, 0, 1,
		  stdout);
     return(0);
}
