/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/delete.c,v $
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
     static char rcsid_delete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/delete.c,v 1.4 1989-01-23 09:22:10 jik Exp $";
#endif

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <strings.h>
#include <sys/param.h>
#include <sys/file.h>

#define ERROR_MASK 1
#define NO_DELETE_MASK 2

/*
 * ALGORITHM:
 *
 * 1. Parse command-line arguments and set flags.
 * 2. Call the function delete() for each filename command-line argument.
 *
 * delete():
 *
 * 1. Can the file be lstat'd?
 *    no -- abort
 *    yes -- continue
 * 2. Is the file a directory?
 *    yes -- is the filesonly option set?
 *           yes -- is the recursive option specified?
 *                  yes -- continue
 *                  no -- abort
 *           no -- is the directory empty?
 *                  yes -- remove it
 *                  no -- is the directoriesonly option set?
 * 			  yes -- abort
 * 			  no -- continue
 * 		       -- is the recursive option specified?
 * 			  yes -- continue
 * 			  no -- abort
 *    no -- is the directoriesonly option set?
 * 	    yes -- abort
 * 	    no -- continue
 *       -- is the file a dot file?
 *          yes -- abort
 *          no -- continue
 * 3. If the file is a file, remove it.
 * 4. If the file is a directory, open it and pass each of its members
 *    (excluding . files) to delete().
 */


int force, interactive, recursive, noop, verbose, filesonly, directoriesonly;
char *whoami;
char *lastpart(), *malloc();

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     int status = 0;
     
     whoami = lastpart(argv[0]);

     force = interactive = recursive = noop = verbose = filesonly =
	  directoriesonly = 0;
     while ((arg = getopt(argc, argv, "firnvFD")) != -1) {
	  switch (arg) {
	  case 'r':
	       recursive++;
	       if (directoriesonly) {
		    fprintf(stderr, "%s: -r and -D are mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       break;
	  case 'f':
	       force++;
	       break;
	  case 'i':
	       interactive++;
	       break;
	  case 'n':
	       noop++;
	       break;
	  case 'v':
	       verbose++;
	       break;
	  case 'F':
	       filesonly++;
	       if (directoriesonly) {
		    fprintf(stderr, "%s: -F and -D are mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       break;
	  case 'D':
	       directoriesonly++;
	       if (recursive) {
		    fprintf(stderr, "%s: -r and -D are mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       if (filesonly) {
		    fprintf(stderr, "%s: -F and -D are mutually exclusive.\n",
			    whoami);
		    usage();
		    exit(1);
	       }
	       break;
	  }
     }
     if (optind == argc) {
	  fprintf(stderr, "%s: no files specified.\n", whoami);
	  usage();
	  exit(1);
     }
     while (optind < argc) {
	  status = status | delete(argv[optind], 0);
	  optind++;
     }
     exit(status | ERROR_MASK);
}




usage()
{
     printf("Usage: %s [ options ] filename ...\n", whoami);
     printf("Options are:\n");
     printf("     -r     recursive\n");
     printf("     -i     interactive\n");
     printf("     -f     force\n");
     printf("     -n     noop\n");
     printf("     -v     verbose\n");
     printf("     -F     files only\n");
     printf("     -D     directories only\n");
     printf("-r and -D are mutually exclusive\n");
     printf("-F and -D are mutually exclusive\n");
}






delete(filename, recursed)
char *filename;
int recursed;
{
     struct stat stat_buf;

     /* can the file be lstat'd? */
     if (lstat(filename, &stat_buf) == -1) {
	  if (! force)
	       fprintf(stderr, "%s: %s nonexistent\n", whoami, filename);
	  return(ERROR_MASK);
     }
     
     /* is the file a directory? */
     if (stat_buf.st_mode & S_IFDIR) {
	  /* is the filesonly option set? */
	  if (filesonly) {
	       /* is the recursive option specified? */
	       if (recursive) {
		    return(recursive_delete(filename, stat_buf, recursed));
	       }
	       else {
		    if (! force)
			 fprintf(stderr, "%s: %s directory\n", whoami,
				 filename);
		    return(ERROR_MASK);
	       }
	  }
	  else {
	       /* is the directory empty? */
	       if (empty_directory(filename)) {
		    /* remove it */
		    return(do_move(filename, stat_buf, 0));
	       }
	       else {
		    /* is the directoriesonly option set? */
		    if (directoriesonly) {
			 if (! force)
			      fprintf(stderr, "%s: %s: Directory not empty\n",
				     whoami, filename);
			 return(ERROR_MASK);
		    }
		    else {
			 /* is the recursive option specified? */
			 if (recursive) {
			      return(recursive_delete(filename, stat_buf,
						      recursed));
			 }
			 else {
			      if (! force)
				   fprintf(stderr, "%s: %s directory\n",
					   whoami, filename);
			      return(ERROR_MASK);
			 }
		    }
	       }
	  }
     }
     else {
	  /* is the directoriesonly option set? */
	  if (directoriesonly) {
	       if (! force)
		    fprintf(stderr, "%s: %s: Not a directory\n", whoami,
			    filename);
	       return(ERROR_MASK);
	  }
	  else {
	       /* is the file a dot file? */
	       if (is_dotfile(filename)) {
		    if (! force)
			 fprintf(stderr, "%s: cannot remove `.' or `..'\n",
				 whoami);
		    return(ERROR_MASK);
	       }
	       else
		    return(do_move(filename, stat_buf, 0));
	  }
     }
     return(0);
}

		 
			 
	       
char *append(filepath, filename, print_errors)
char *filepath, *filename;
int print_errors;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if (buf[strlen(buf) - 1] == '/')
	  buf[strlen(buf) - 1] = '\0';
     if (strlen(buf) + strlen(filename) + 2 > MAXPATHLEN) {
	  if (print_errors)
	       fprintf(stderr, "%s: %s/%s: pathname too long\n", whoami,
		       filepath, filename);
	  *buf = '\0';
	  return(buf);
     }
     strcat(buf, "/");
     strcat(buf, filename);
     return(buf);
}
    

	  


empty_directory(filename)
char *filename;
{
     DIR *dirp;
     struct direct *dp;

     dirp = opendir(filename);
     if (! dirp) {
	  return(0);
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  else {
	       closedir(dirp);
	       return(0);
	  }
     }
     closedir(dirp);
     return(1);
}




recursive_delete(filename, stat_buf, recursed)
char *filename;
struct stat stat_buf;
int recursed;
{
     DIR *dirp;
     struct direct *dp;
     int status = 0;
     char newfile[MAXPATHLEN];
     
     if (interactive && recursed) {
	  printf("%s: remove directory %s? ", whoami, filename);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
     dirp = opendir(filename);
     if (! dirp) {
	  if (! force)
	       fprintf(stderr, "%s: %s not changed\n", whoami, filename);
	  return(ERROR_MASK);
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  else {
	       strcpy(newfile, append(filename, dp->d_name, !force));
	       if (*newfile)
		    status = status | delete(newfile, 1);
	       else
		    status = ERROR_MASK;
	  }
     }
     closedir(dirp);
     status = status | do_move(filename, stat_buf, status | NO_DELETE_MASK);
     return(status);
}

					 



yes() {
     char buf[BUFSIZ];

     fgets(buf, BUFSIZ, stdin);
     if (! index(buf, '\n')) do
	  fgets(buf + 1, BUFSIZ - 1, stdin);
     while (! index(buf + 1, '\n'));
     return(*buf == 'y');
}




	  
char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}




is_dotfile(filename)
char *filename;
{
     return (! (strcmp(filename, ".") && strcmp(filename, "..")));
}





do_move(filename, stat_buf, no_delete_mask)
char *filename;
struct stat stat_buf;
int no_delete_mask;
{
     char *last;
     char buf[MAXPATHLEN];
     char name[MAXNAMLEN];
     struct stat deleted_buf;

     strncpy(buf, filename, MAXPATHLEN);
     last = lastpart(buf);
     if (strlen(last) > MAXNAMLEN) {
	  if (! force)
	       fprintf(stderr, "%s: %s: filename too long\n", whoami,
		       filename);
	  return(ERROR_MASK);
     }
     strcpy(name, last);
     if (strlen(buf) + 3 > MAXPATHLEN) {
	  if (! force)
	       fprintf(stderr, "%s: %s: pathname too long\n", whoami,
		       filename);
	  return(ERROR_MASK);
     }
     *last = '\0';
     strcat(buf, ".#");
     strcat(buf, name);
     if (interactive) {
	  printf("%s: remove %s? ", whoami, filename);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
     else if ((! force) && access(filename, W_OK)) {
	  printf("%s: override protection %o for %s? ", whoami,
		 stat_buf.st_mode & 0777, filename);
	  if (! yes())
	       return(NO_DELETE_MASK);
     }
     if (no_delete_mask) {
	  if ((! force) || noop)
	       fprintf(stderr, "%s: %s not removed\n", whoami, filename);
	  return(ERROR_MASK);
     }
     if (noop) {
	  fprintf(stderr, "%s: %s would be removed\n", whoami, filename);
	  return(0);
     }
     if (verbose)
	  fprintf(stderr, "%s: %s removed\n", whoami, filename);
     
     if (! lstat(buf, &deleted_buf))
	  unlink_completely(buf);
     if (rename(filename, buf)) {
	  if (! force)
	       fprintf(stderr, "%s: %s not removed\n", whoami, filename);
	  return(ERROR_MASK);
     }
     else
	  return(0);
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
	       strcpy(buf, append(filename, dp->d_name, 0));
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

	  
	       
	  
     
	  
