/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: io.c,v 1.13 1999-03-06 16:49:14 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#define SELECT_TIMEOUT	10     /* timeout after 10 seconds */

/*
 * Function:	sread() is just like read() except that it checks to make
 *	 sure that the file descriptor is ready for reading.
 * Arguments:	fd:	File descriptor to read.
 *		buf:	Ptr. to buffer to be used.
 *		nbytes:	Number of bytes to read.
 * Returns:	Number of bytes actually read, or -1 if an error occurs.
 * Notes:
 *	Use select(2) to see if the file descriptor is ready for reading.
 *	If it times out, return ERROR. Otherwise, read the given number
 *	of bytes and return what read(2) returns.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "system.h"

int
sread(fd, buf, nbytes)
     int fd;
     void *buf;
     int nbytes;
{
  struct timeval tval;	/* System time structure. */
  fd_set read_fds;	/* File descriptors to check.*/
  int n_read, s_val;

  if (nbytes <= 0)
    return(0);

  /*
   *  A necessary evil so that the daemon doesn't hang.
   */

  tval.tv_sec = SELECT_TIMEOUT;
  tval.tv_usec = 0;
  
  FD_ZERO(&read_fds);
  FD_SET(fd,&read_fds);
  s_val = select(fd+1, &read_fds, NULL, NULL, &tval);
  if (s_val < 1) 
    {
      if (s_val == 0)
	errno = ETIMEDOUT;

      perror("sread: select");
      return(-1);
    }

  n_read = read(fd, buf, nbytes);
  return(n_read);
}



/*
 * Function:	swrite() is just like write() except that it checks to make
 *	sure that the file descriptor is ready for writeing.
 * Arguments:	fd:	File descriptor to write.
 *		buf:	Ptr. to buffer to be used.
 *		nbytes:	Number of bytes to write.
 * Returns:	Number of bytes actually written, or -1 if an error occurs.
 * Notes:
 *	Use select(2) to see if the file descriptor is ready for writing.
 *	If it times out, return ERROR. Otherwise, write the given number
 *	of bytes and return what write(2) returns.
 */

int
swrite(fd, buf, nbytes)
     int fd;
     void *buf;
     int nbytes;
{
  struct timeval tval;	        /* System time structure. */
  fd_set write_fds;		/* File descriptors to check.*/
  register int n_wrote, s_val;

  if (nbytes <= 0)
    return(0);
	
  tval.tv_sec = SELECT_TIMEOUT;
  tval.tv_usec = 0;
  

  FD_ZERO(&write_fds);
  FD_SET(fd,&write_fds);
  s_val = select(fd+1, NULL, &write_fds, NULL, &tval);
  if (s_val != 1)
    {
      if (s_val == 0)
	errno = ETIMEDOUT;
      perror("swrite: select");
      return(-1);
    }
  
  n_wrote = write(fd, buf, nbytes);

  if (n_wrote != nbytes) 
    fprintf(stderr, "swrite: %d wrote of %d\n", n_wrote, nbytes);
  return(n_wrote);
}