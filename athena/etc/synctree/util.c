/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>

#ifndef MAXBSIZE
#define MAXBSIZE 20480
#endif

extern int errno;
extern int verbosef;

int copyfile(from,to,mode)
     char *from;
     char *to;
     u_short mode;
{
    int from_fd, to_fd;
    char buffer[MAXBSIZE];
    int n;

    if (verbosef) printf("Copying to %s from %s, mode is %5.5o.\n", to, from, mode);

    if ((from_fd = open(from, O_RDONLY, 0)) <= 0)
	return -1;
  
    if ((to_fd = open(to, O_WRONLY|O_CREAT|O_EXCL, mode)) <= 0)
	{ int savederrno = errno;
	  (void) close(from_fd);
	  errno = savederrno;
	  return -1;
      }

    while (n = read(from_fd, buffer, MAXBSIZE))
	{ 
	    if (n < 0)
		{ int savederrno = errno;
		  (void) close(from_fd);
		  (void) close(to_fd);
		  (void) unlink(to);
		  errno = savederrno;
		  return -1;
	      }
	    if (write(to_fd, buffer, n) != n)
		{ int savederrno = errno;
		  (void) close(from_fd);
		  (void) close(to_fd);
		  (void) unlink(to);
		  errno = savederrno;
		  return -1;
	      }
	}
    (void) close(from_fd);
    if (close(to_fd) != 0)
	{ int savederrno = errno;
	  (void) unlink(to);
	  errno = savederrno;
	  return -1;
      }

    return 0;
}

int copylink(from,to,mode)
     char *from;
     char *to;
     u_short mode;
{
    int from_fd, to_fd;
    char buffer[MAXBSIZE];
    int n;

    if (verbosef)
	printf("Copying link to %s from %s, mode is %5.5o.\n", to, from, mode);

    if ((n = readlink(from,buffer,MAXBSIZE)) < 0)
	return -1;
  
    buffer[n] = '\0';

    if (symlink(buffer,to) < 0)
	return -1;
  
    return 0;
}

int recursive_rmdir(dirname)
     char *dirname;
{
    char path[MAXPATHLEN];
    DIR *dirp;
    struct direct *dp;
    struct stat statbuf;
    
    if (verbosef)
	printf("Recursively removing %s.\n", dirname);
    if (!(dirp = opendir(dirname)))
	return -1;
    while ((dp = readdir(dirp)) != NULL) {
	if (!strcmp(dp->d_name, ".") ||
	    !strcmp(dp->d_name, ".."))
	    continue;
	strcpy(path, dirname);
	strcat(path, "/");
	strcat(path, dp->d_name);
	if (lstat(path, &statbuf)) {
	    printf("recursive_rmdir: lstat error %d on file %s.\n",
		   errno, path);
	} else {
	    if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
		recursive_rmdir(path);
		rmdir(path);
	    } else
		if (unlink(path))
		    printf("recursive_rmdir: unlink error %d on file %s.\n",
			   errno, path);
	}
    }
    closedir(dirp);
    return(rmdir(dirname));
}
