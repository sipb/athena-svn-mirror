/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/delete.c,v $
 * $Author: probe $
 *
 * This program is a replacement for rm.  Instead of actually deleting
 * files, it marks them for deletion by prefixing them with a ".#"
 * prefix.
 *
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_delete_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/delete.c,v 1.29 1993-02-09 00:35:54 probe Exp $";
#endif

#include <sys/types.h>
#ifdef AFS_MOUNTPOINTS
#include <sys/time.h>
#endif
#include <stdio.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#ifdef SYSV
#include <string.h>
#define index strchr
#define rindex strrchr
#else
#include <strings.h>
#endif /* SYSV */
#include <sys/param.h>
#include <sys/file.h>
#include <errno.h>
#include "errors.h"
#include "delete_errs.h"
#include "util.h"
#include "delete.h"
#include "mit-copying.h"



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
 *    yes -- is it a dotfile?
 *           yes -- abort
 *           no -- continue
 *        -- is the filesonly option set?
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
 * 3. If the file is a file, remove it.
 * 4. If the file is a directory, open it and pass each of its members
 *    (excluding . files) to delete().
 */


int force, interactive, recursive, noop, verbose, filesonly, directoriesonly;
int emulate_rm, linked_to_rm, linked_to_rmdir;
extern int errno;
#ifdef AFS_MOUNTPOINTS
struct timeval tvp[2];
#endif

main(argc, argv)
int argc;
char *argv[];
{
     extern char *optarg;
     extern int optind;
     int arg;
     
     whoami = lastpart(argv[0]);

     initialize_del_error_table();

#ifdef AFS_MOUNTPOINTS
     gettimeofday(&tvp[0], (struct timezone *)0);
     tvp[1] = tvp[0];
#endif

     force = interactive = recursive = noop = verbose = filesonly =
	  directoriesonly = emulate_rm = linked_to_rm = linked_to_rmdir = 0;

     if (!strcmp(whoami, "rm"))
	  emulate_rm++, filesonly++, linked_to_rm++;
     if (!strcmp(whoami, "rmdir") || !strcmp(whoami, "rd"))
	  emulate_rm++, directoriesonly++, linked_to_rmdir++;
     
     while ((arg = getopt(argc, argv, "efirnvFD")) != -1) {
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
	  case 'e':
	       emulate_rm++;
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
	  default:
	       usage();
	       exit(1);
	  }
     }
     report_errors = ! (force || emulate_rm);
     
     if (optind == argc) {
	  if (! force) {
	       fprintf(stderr, "%s: no files specified.\n", whoami);
	       usage();
	  }
	  exit(force ? 0 : 1);
     }
     while (optind < argc) {
	  if (delete(argv[optind], 0))
	       error(argv[optind]);
	  optind++;
     }
     exit(((! force) && error_occurred) ? 1 : 0);
}



usage()
{
     printf("Usage: %s [ options ] filename ...\n", whoami);
     printf("Options are:\n");
     if (! linked_to_rmdir)
	  printf("     -r     recursive\n");
     printf("     -i     interactive\n");
     printf("     -f     force\n");
     printf("     -n     noop\n");
     printf("     -v     verbose\n");
     if (! (linked_to_rmdir || linked_to_rm)) {
	  printf("     -e     emulate rm/rmdir\n");
	  printf("     -F     files only\n");
	  printf("     -D     directories only\n");
     }
     printf("     --     end options and start filenames\n");
     if (! (linked_to_rmdir || linked_to_rm)) {
	  printf("-r and -D are mutually exclusive\n");
	  printf("-F and -D are mutually exclusive\n");
     }
}




delete(filename, recursed)
char *filename;
int recursed;
{
     struct stat stat_buf;
     int retval;
     
     /* can the file be lstat'd? */
     if (lstat(filename, &stat_buf) == -1) {
	  set_error(errno);
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s nonexistent\n", whoami, filename);
	  error(filename);
	  return error_code;
     }

     /* is the file a directory? */
     if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) {
	  /* is the file a dot file? */
	  if (is_dotfile(lastpart(filename))) {
	       set_error(DELETE_IS_DOTFILE);
	       if (emulate_rm && (! force))
		    fprintf(stderr, "%s: cannot remove `.' or `..'\n", whoami);
	       error(filename);
	       return error_code;
	  }

	  /* is the filesonly option set? */
	  if (filesonly) {
	       /* is the recursive option specified? */
	       if (recursive) {
		    if (retval = recursive_delete(filename, stat_buf,
						  recursed)) {
			 error(filename);
			 return retval;
		    }
	       }
	       else {
		    if (emulate_rm && (! force))
			 fprintf(stderr, "%s: %s directory\n", whoami,
				 filename);
		    set_error(DELETE_CANT_DEL_DIR);
		    error(filename);
		    return error_code;
	       }
	  }
	  else {
	       /* is the directory empty? */
	       if ((retval = empty_directory(filename)) < 0) {
		    error(filename);
		    if (! emulate_rm)
			 return error_code;
	       }

	       if (retval > 0) {
		    /* remove it */
		    if (retval = do_move(filename, stat_buf, 0)) {
			 error(filename);
			 return error_code;
		    }
	       }
	       else {
		    /* is the directoriesonly option set? */
		    if (directoriesonly) {
			 if (emulate_rm && (! force))
			      fprintf(stderr, "%s: %s: Directory not empty\n",
				      whoami, filename);
			 set_error(DELETE_DIR_NOT_EMPTY);
			 error(filename);
			 return error_code;
		    }
		    else {
			 /* is the recursive option specified? */
			 if (recursive) {
			      if (retval = recursive_delete(filename, stat_buf,
							    recursed)) {
				   error(filename);
				   return error_code;
			      }
			 }
			 else {
			      if (emulate_rm && (! force))
				   fprintf(stderr, "%s: %s not empty\n",
					   whoami, filename);
			      set_error(DELETE_DIR_NOT_EMPTY);
			      error(filename);
			      return error_code;
			 }
		    }
	       }
	  }
     }
     else {
	  /* is the directoriesonly option set? */
	  if (directoriesonly) {
	       if (emulate_rm && (! force))
		    fprintf(stderr, "%s: %s: Not a directory\n", whoami,
			    filename);
	       set_error(DELETE_CANT_DEL_FILE);
	       error(filename);
	       return error_code;
	  }
	  else {
	       if (retval = do_move(filename, stat_buf, 0)) {
		    error(filename);
		    return error_code;
	       }
	  }
     }
     return 0;
}
	  

		 
			 
	       
empty_directory(filename)
char *filename;
{
     DIR *dirp;
#ifdef POSIX
     struct dirent *dp;
#else
     struct direct *dp;
#endif

     dirp = Opendir(filename);
     if (! dirp) {
	  set_error(errno);
	  error(filename);
	  return -1;
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  if (is_deleted(dp->d_name))
	       continue;
	  else {
	       closedir(dirp);
	       return 0;
	  }
     }
     closedir(dirp);
     return 1;
}




recursive_delete(filename, stat_buf, recursed)
char *filename;
struct stat stat_buf;
int recursed;
{
     DIR *dirp;
#ifdef POSIX
     struct dirent *dp;
#else
     struct direct *dp;
#endif
     int status = 0;
     char newfile[MAXPATHLEN];
     int retval = 0;
     
     if (interactive && recursed) {
	  printf("%s: remove directory %s? ", whoami, filename);
	  if (! yes()) {
	       set_status(DELETE_NOT_DELETED);
	       return error_code;
	  }
     }
     dirp = Opendir(filename);
     if (! dirp) {
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s not changed\n", whoami, filename);
	  set_error(errno);
	  error(filename);
	  return error_code;
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  if (is_deleted(dp->d_name))
	       continue;
	  else {
	       (void) strcpy(newfile, append(filename, dp->d_name));
	       if (! *newfile) {
		    error(filename);
		    status = error_code;
	       }

	       retval = delete(newfile, 1);
	       if (retval) {
		    error(newfile);
		    status = retval;
	       }
	  }
     }
     closedir(dirp);

     if (status && (! emulate_rm)) {
	  set_warning(DELETE_DIR_NOT_EMPTY);
	  error(filename);
     }
     else
	  retval = do_move(filename, stat_buf, status);
     
     if (retval)
	  status = retval;

     return status;
}

					 




do_move(filename, stat_buf, subs_not_deleted)
char *filename;
struct stat stat_buf;
int subs_not_deleted; /* If the file in question is a directory, and  */
		      /* there is something underneath it that hasn't */
		      /* been removed, this will be set to true.      */
                      /* The program asks if the user wants to delete */
		      /* the directory, and if the user says yes,     */
		      /* checks the value of subs_not_deleted.  If    */
		      /* it's true, an error results.                 */
                      /* This is used only when emulating rm.         */
{
     char *last;
     char buf[MAXPATHLEN];
     char name[MAXNAMLEN];
     struct stat deleted_buf;

     (void) strncpy(buf, filename, MAXPATHLEN);
     last = lastpart(buf);
     if (strlen(last) > MAXNAMLEN) {
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s: filename too long\n", whoami,
		       filename);
	  set_error(ENAMETOOLONG);
	  error(filename);
	  return error_code;
     }
     (void) strcpy(name, last);
     if (strlen(buf) + 3 > MAXPATHLEN) {
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s: pathname too long\n", whoami,
		       filename);
	  set_error(ENAMETOOLONG);
	  error(filename);
	  return error_code;
     }
     *last = '\0';
     (void) strcat(buf, ".#");
     (void) strcat(buf, name);
     if (interactive) {
	  printf("%s: remove %s? ", whoami, filename);
	  if (! yes()) {
	       set_status(DELETE_NOT_DELETED);
	       return error_code;
	  }
     }
     else if ((! force)
#ifdef S_IFLNK
	      && ((stat_buf.st_mode & S_IFMT) != S_IFLNK)
#endif
	      && access(filename, W_OK)) {
	  if (emulate_rm)
	       printf("%s: override protection %o for %s? ", whoami,
		      stat_buf.st_mode & 0777, filename);
	  else
	       printf("%s: File %s not writeable.  Delete anyway? ", whoami,
		      filename);
	  if (! yes()) {
	       set_status(DELETE_NOT_DELETED);
	       return error_code;
	  }
     }
     if (emulate_rm && subs_not_deleted) {
	  if (! force)
	       fprintf(stderr, "%s: %s not removed\n", whoami, filename);
	  return 1;
     }
     if (noop) {
	  fprintf(stderr, "%s: %s would be removed\n", whoami, filename);
	  return 0;
     }
     if ((! lstat(buf, &deleted_buf)) && unlink_completely(buf)) {
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s not removed\n", whoami, filename);
	  error(filename);
	  return error_code;
     }
     if (rename(filename, buf)) {
	  if (emulate_rm && (! force))
	       fprintf(stderr, "%s: %s not removed\n", whoami, filename);
	  set_error(errno);
	  error(filename);
	  return error_code;
     }
     else {
	  if (verbose)
	       fprintf(stderr, "%s: %s removed\n", whoami, filename);
#ifdef AFS_MOUNTPOINTS
	  /*
	   * Normally, expunge uses the ctime to determine how long
	   * ago a file was deleted (since the ctime is normally
	   * updated when a file is renamed).  However, in AFS,
	   * renaming a file does not change the ctime, mtime OR
	   * atime, so we have to use utimes to force the change.
	   * This unfortunately causes the loss of the real mtime, but
	   * there's nothing we can do about that, if we want expunge
	   * to be able to do the right thing.
	   *
	   * Don't bother checking for errors, because we can't do
	   * anything about them anyway, and in any case, this isn't a
	   * *really* important operation.
	   */
	  utimes(buf, tvp);
#endif
	  return 0;
     }
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
     int status = 0;
     int retval;
     
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
	       if (! *buf) {
		    status = error_code;
		    error(filename);
		    continue;
	       }
	       retval = unlink_completely(buf);
	       if (retval) {
		    status = retval;
		    error(filename);
	       }
	  }
	  closedir(dirp);

	  if (status)
	       return status;

	  retval = rmdir(filename);
	  if (retval) {
	       set_error(errno);
	       error(filename);
	       return errno;
	  }
     }
     else {
	  retval = unlink(filename);
	  if (retval) {
	       set_error(errno);
	       error(filename);
	       return error_code;
	  }
	  else
	       return 0;
     }
     return 0;
}
