/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for communication between the user programs
 * and the daemon.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: c_io.c,v 1.29 1999-06-28 22:52:24 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: c_io.c,v 1.29 1999-06-28 22:52:24 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <stdlib.h>
#include <sys/types.h>		/* System type declarations. */
#include <sys/socket.h>		/* Network socket defs. */
#include <sys/file.h>		/* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>		/* System time definitions. */
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>		/* System error numbers. */
#include <netdb.h>
#include <signal.h>
#include <sgtty.h>              /* Terminal param. definitions. */
#include <setjmp.h>

#include <olc/olc.h>

static ERRCODE write_chars_to_fd (int, char *, int);
static ERRCODE read_chars_from_fd (int, char *, int);

extern int select_timeout;

/*
 * Note: All functions that deal with I/O on sockets in this file use the
 *	functions "sread()" and "swrite()", which check to ensure that the
 *	socket is, in fact, connected to something.
 */


int send_dbinfo(fd,dbinfo)
     int fd;
     DBINFO *dbinfo;
{
  DBINFO dbi;

  dbi = *dbinfo;
  dbi.max_ask    =  (int) htonl((u_long) dbinfo->max_ask);
  dbi.max_answer =  (int) htonl((u_long) dbinfo->max_answer);
  if (swrite(fd, (char *) &dbi, sizeof(DBINFO)) != sizeof(DBINFO))
    return(ERROR);

  return(SUCCESS);
}


int read_dbinfo(fd,dbinfo)
     int fd;
     DBINFO *dbinfo;
{
  DBINFO dbi;

  if (sread(fd, (char *) &dbi, sizeof(DBINFO)) != sizeof(DBINFO))
    return(ERROR);

  dbinfo->max_ask    =  (int) ntohl((u_long) dbi.max_ask);
  dbinfo->max_answer =  (int) ntohl((u_long) dbi.max_answer);
  strcpy(dbinfo->title1, dbi.title1);
  strcpy(dbinfo->title2, dbi.title2);

  return(SUCCESS);
}


/*
 * Function:	send_response() sends a response the a user program.
 * Arguments:	fd:		File descriptor to write to.
 *		response:	Response to send.
 * Returns:	SUCCESS if the send is successful, ERROR otherwise.
 * Notes:
 *	Write the response to the socket returning SUCCESS if the write
 *	succeeds, and ERROR if it does not.
 */

ERRCODE
send_response(fd, response)
     int fd;
     ERRCODE response;
{
  return(write_int_to_fd(fd, response));
}



/*
 * Function:	read_response() reads a response from the daemon.
 * Arguments:	fd:		File descriptor to read from.
 *		response:	Ptr. to buffer for response.
 * Returns:	SUCCESS if the read is successful, ERROR otherwise.
 * Notes:
 *	Read the response from the socket returning SUCCESS if the read
 *	succeeds, and ERROR if it does not.
 */

ERRCODE
read_response(fd, response)
     int fd;
     ERRCODE *response;
{
  return(read_int_from_fd(fd, (int *)response));
}



/*
 * Function:	write_int_to_fd() writes an integer to a file descriptor.
 * Arguments:	fd:		File descriptor to write to.
 *		response:	Response to send.
 * Returns:	SUCCESS if the send is successful, ERROR otherwise.
 * Notes:
 *	Write the response to the socket returning SUCCESS if the write
 *	succeeds, and ERROR if it does not.
 */

ERRCODE
write_int_to_fd(fd, response)
     int fd;
     int response;
{
  response = htonl((u_long) response);
  if (swrite(fd, (char *) &response, sizeof(int)) != sizeof(int))
    return(ERROR);
  else
    return(SUCCESS);
}



/*
 * Function:	read_int_from_fd() reads an integer from a file descriptor.
 * Arguments:	fd:		File descriptor to write to.
 *		response:	Ptr. to buffer for response.
 * Returns:	SUCCESS if the read is successful, ERROR otherwise.
 * Notes:
 *	Read the response from the socket returning SUCCESS if the read
 *	succeeds, and ERROR if it does not.
 */

ERRCODE
read_int_from_fd(fd, response)
     int fd;
     int *response;
{
  if (sread(fd, (char *) response, sizeof(int)) != sizeof(int))
    return(ERROR);
  else 
    {
      *response = ntohl((u_long) *response);
      return(SUCCESS);
    }
}



/*
 * Function:	read_text_into_file() reads text from a file descriptor into
 *			a file.
 * Arguments:	fd:		File descriptor to read from.
 *		filename:	Name of file to write to.
 * Returns:
 * Notes:
 *	First, read an integer from the file descriptor to see how many
 *	bytes are coming.  Then read chunks from the file descriptor,
 *	writing each chunk to the file after it is read.
 *	Finally, close the file and return.
 */

ERRCODE
read_text_into_file(fd, filename)
     int fd;
     char *filename;
{
  int nbytes;		/* Number of bytes to read. */
  char inbuf[BUFSIZ];	/* Ptr. to input buffer. */
  int filedes;		/* Output file descriptor. */
  char error[ERROR_SIZE];	/* Error message. */
  
  if (read_int_from_fd(fd, &nbytes) != SUCCESS) 
    {
      olc_perror("read_text_into_file: unable to read nbytes from socket");
      return(ERROR);
    }
  
  filedes = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (filedes < 0) 
    {
      (void) sprintf(error, "read_text_into_file: Unable to open file %s",
		     filename);
      olc_perror(error);
      return(ERROR);
    }
  
  if (nbytes > 0) 
    {
      while (nbytes) 
	{
	  int n = sread(fd, inbuf, MIN(BUFSIZ, nbytes));
	  if (n < 1)
	    {
	      (void) sprintf(error,
			     "read_text_into_file: Error reading text, n=%d",
			     n);
	      olc_perror(error);
	      (void) unlink(filename);
	      (void) close(filedes);
	      return(ERROR);
	    }
	  
	  if (swrite(filedes, inbuf, n) != n) 
	    {
	      (void) sprintf(error, 
		     "read_text_into_file: Error writing text to file %s",
		     filename);
	      olc_perror(error);
	      (void) unlink(filename);
	      (void) close(filedes);
	      return(ERROR);
	    }
	  nbytes -= n;
	}
    }
  close(filedes);
  return(SUCCESS);
}

ERRCODE
read_file_into_text(filename,bufp)
     char *filename;
     char **bufp;
{
  char *buf;
  int nbytes;
  int fd;
  struct stat statbuf;
  char error[ERROR_SIZE];

  buf = *bufp;
  if (stat(filename,&statbuf) != 0)
    {
      sprintf(error,"read_file_into_text: bad stat value on file %s",
	      filename);
      olc_perror(error);
      return(ERROR);
    }
  nbytes = statbuf.st_size;

  buf = malloc(nbytes + 1);
  if (buf == NULL)
    {
      olc_perror("read_file_into_text: Can't allocate memory.");
      return(ERROR);
    }
  
  fd = open(filename,O_RDONLY,0);
  if (fd < 0)
    {
      free(buf);
      sprintf(error,"read_file_into_text: error opening %s", filename);
      olc_perror(error);
      return(ERROR);
    }

  if (read(fd,buf,nbytes) != nbytes)
    {
      free(buf);
      sprintf(error,"read_file_into_text: error reading %s", filename);
      olc_perror(error);
      close(fd);
      return(ERROR);
    }

  if (close(fd) < 0)
    {
      sprintf(error,"read_file_into_text: error closing %s", filename);
      olc_perror(error);
    }
  
  buf[nbytes] = '\0';
  *bufp = buf;
  return(SUCCESS);
}


/*
 * Function:	write_file_to_fd() writes a file to a specified file descriptor
 * Arguments:	fd:		File descriptor to write to.
 *		filename:	Name of file to copy.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, find out how large the file is.	If the size appears to be
 *	zero or negative, send that it is zero bytes long and return ERROR.
 *	Otherwise, open the file, send how long it is, and read it in BUFSIZE
 *	chunks, writing	each chunk to the file descriptor after it is read.
 *	Finally, close the file and return.
 */

ERRCODE
write_file_to_fd(fd, filename)
     int fd;
     char *filename;
{
  int nbytes;		/* Number of bytes in file. */
  int filedes;		/* Input file descriptor. */
  struct stat statbuf;	/* File status structure. */
  char inbuf[BUFSIZ];	/* Input buffer. */
  char error[ERROR_SIZE];	/* Error message. */

  if (stat(filename, &statbuf) != 0) 
    {
      (void) sprintf(error, "write_file_to_fd: bad stat value on file %s",
		     filename);
      olc_perror(error);
      nbytes = 0;
    }
  else
    nbytes = statbuf.st_size;

  if (nbytes <= 0) 
    {
      write_int_to_fd(fd, 0);
      return(ERROR);
    }
  
  filedes = open(filename, O_RDONLY, 0);
  if (filedes < 0) 
    {
      (void) sprintf(error, "write_file_to_fd: Unable to open file %s",
		     filename);
      olc_perror(error);
      return(ERROR);
    }
  
  if (write_int_to_fd(fd, nbytes) != SUCCESS) 
    {
      olc_perror("write_file_to_fd: Error writing size.");
      close(filedes);
      return(ERROR);
    }
  
  while (nbytes) 
    {
      int n_read, n_wrote;
	
      n_read = sread(filedes, inbuf, MIN(BUFSIZ, nbytes));
      if (n_read < 1)
	{
	  (void) sprintf(error,
		 "write_file_to_fd: Error reading text from file %s, n=%d",
		 filename, n_read);
	  olc_perror(error);
	  (void) close(filedes);
	  return(ERROR);
	}

      n_wrote = swrite(fd, inbuf, n_read);
      if (n_wrote != n_read) 
	{
	  (void) sprintf(error,
		 "write_file_to_fd: Error writing text, n_read=%d, n_wrote=%d",
		 n_read, n_wrote);
	  olc_perror(error);

	  (void) close(filedes);
	  return(ERROR);
	}
      nbytes -= n_read;
    }
  close(filedes);
  return(SUCCESS);
}



/*
 * Function:	write_text_to_fd() writes a string to a file descriptor.
 * Arguments:	fd:	File descriptor to write to.
 *		buf:	Ptr. to string to write.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Send the length of the string, followed by the characters, which
 *	are sent using write_chars_to_fd().
 */

ERRCODE
write_text_to_fd(fd, buf)
     int fd;
     char *buf;
{
  int nchars;		/* Number of characters in string. */

  if (buf != NULL)
    nchars = strlen(buf);
  else
    nchars = 0;

  if (write_int_to_fd(fd, nchars) != SUCCESS) 
    {
      olc_perror("write_text_to_fd: write_int_to_fd");
      return(ERROR);
    }
  
  if (write_chars_to_fd(fd, buf, nchars) == SUCCESS)
    return(SUCCESS);
  else 
    return(ERROR);
}



/*
 * Function:	read_text_from_fd() reads a string from a file desriptor.
 * Arguments:	fd:	File descriptor to read from.
 * Returns:	A pointer to the text read, or NULL if an error occurs.
 * Notes:
 *	Find out how long the string is, allocate memory for it, 
 *	then read the characters using read_chars_from_fd().
 */


char *
read_text_from_fd(fd)
     int fd;
{
  int nchars;		/* Number of characters to read. */
  static char *rtff_buf; 

  if (rtff_buf != (char *) NULL)
    free(rtff_buf);
  rtff_buf = (char *) NULL;
  
  if (read_int_from_fd(fd, &nchars) != SUCCESS) 
    {
      olc_perror("read_text: read_int");
      return((char *) NULL);
    }

  rtff_buf = malloc(nchars + 1);
  if (rtff_buf == NULL)
    {
      olc_perror("read_text: Can't allocate memory.");
      return((char *) NULL);
    }

  if (read_chars_from_fd(fd, rtff_buf, nchars) == SUCCESS) 
    {
      rtff_buf[nchars] = '\0';
      return(rtff_buf);
    }
  else
    return((char *) NULL);
}	


/*
 * Function:	write_chars_to_fd() writes a stream of characters to a file
 *			descriptor.
 * Arguments:	fd:	File descriptor to write to.
 *		buf:	Ptr. to buffer of characters to write.
 *		nchars:	Number of characters to write.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Write BUFSIZ characters each time until almost all characters
 *	are written, then write the final set.
 */

static ERRCODE
write_chars_to_fd(fd, buf, nchars)
     int fd;
     char *buf;
     int nchars;
{

  while (nchars > BUFSIZ) 
    {
      if (swrite(fd, buf, BUFSIZ) != BUFSIZ) 
	{
	  olc_perror("write_chars: Error writing text.");
	  return(ERROR);
	}
      buf += BUFSIZ;
      nchars -= BUFSIZ;
    }

  if (swrite(fd, buf, nchars) != nchars) 
    {
      olc_perror("write_chars: Error writing text.");
      return(ERROR);
    }
  return(SUCCESS);
}



/*
 * Function:	read_chars_from_fd() reads characters from a file desriptor.
 * Arguments:	fd:	File descriptor to read from.
 *		buf:	Ptr to buffer to use.
 *		nchars:	Number of characters to read.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Read BUFSIZ characters each time until almost all characters are
 *	read, then read the final set.
 */

static ERRCODE
read_chars_from_fd(fd, buf, nchars)
     int fd;
     char *buf;
     int nchars;
{
  int n;
  char error[ERROR_SIZE];	/* Error message. */

  while (nchars) 
    {
      n = sread(fd, buf, MIN(BUFSIZ, nchars));
      if (n < 1) 
	{
	  (void) sprintf(error,
			 "read_chars_from_fd: Error reading text, n=%d", n);
	  olc_perror(error);

	  return(ERROR);
	}
      
      buf += n;
      nchars -= n;
    }
  return(SUCCESS);
}

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

int sread(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  struct timeval tval;	/* System time structure. */
  fd_set read_fds;		/* File descriptors to check.*/
  register int n_read, s_val, tot_read;
  int loops = 0;

  if (nbytes <= 0)
    return(0);

  /*
   *  A necessary evil so that the daemon doesn't hang.
   */

  tot_read = 0;
  loops = 0;
  n_read = 0;
  do {
    if (loops > 5) {
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("sread: retried 5 times, giving up; last error");
      close(fd);
      return(-1);
    }

#ifndef IO_BLOCKING
    tval.tv_sec = select_timeout;
    tval.tv_usec = 0;
    
    FD_ZERO(&read_fds);
    FD_SET(fd,&read_fds);
    s_val = select(fd+1, &read_fds, NULL, NULL, &tval);
    if (s_val < 1) 
      {
	if (errno == EINTR) {
	  loops++;
	  continue;
	}

	if (s_val == 0)
	  errno = ETIMEDOUT;

#ifdef IO_DUMP
	dump_current_core_image();
#endif /* IO_DUMP */
	olc_perror("sread: select");
	close(fd);
	return(-1);
      }
    
    if (! FD_ISSET(fd,&read_fds)) {
      /* This ought to be impossible if s_val >= 1 */
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("sread: select() return isn't consistent; last error");
      close(fd);
      return(-1);
    }
#endif /* not IO_BLOCKING */

    n_read = read(fd, (char *)(buf + tot_read), nbytes);
    if (n_read < 0) {
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("sread: read");
      close(fd);
      return(n_read);
    }
    if (n_read != 0)
      loops = 0;

    tot_read += n_read;
    nbytes = nbytes - n_read;
    loops++;
  } while (nbytes != 0);

  return(tot_read);
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

int swrite(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  struct timeval tval;	        /* System time structure. */
  fd_set write_fds;		/* File descriptors to check.*/
  register int n_wrote, s_val, tot_wrote;
  int loops = 0;

  if (nbytes <= 0)
    return(0);
	
  tot_wrote = 0;
  loops = 0;
  n_wrote = 0;
  do {
    if (loops > 5) {
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("swrite: retried 5 times, giving up; last error");
      close(fd);
      return(-1);
    }

#ifndef IO_BLOCKING
    tval.tv_sec = select_timeout;
    tval.tv_usec = 0;
    
    FD_ZERO(&write_fds);
    FD_SET(fd,&write_fds);
    s_val = select(fd+1, NULL, &write_fds, NULL, &tval);
    if (s_val != 1) 
      {
	if (errno == EINTR) {
	  loops++;
	  continue;
	}

	if (s_val == 0)
	  errno = ETIMEDOUT;
	
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
	olc_perror("swrite: select");
	close(fd);
	return(-1);
      }
    
    if (! FD_ISSET(fd,&write_fds)) {
      /* This ought to be impossible if s_val is >= 1 */
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("swrite: select() return isn't consistent; last error");
      close(fd);
      return(-1);
    }
#endif /* not IO_BLOCKING */

    n_wrote = write(fd, (char *)(buf + tot_wrote), nbytes);
    if (n_wrote < 0) {
#ifdef IO_DUMP
      dump_current_core_image();
#endif /* IO_DUMP */
      olc_perror("swrite: write");
      close(fd);
      return(n_wrote);
    }
    if (n_wrote != 0)
      loops = 0;

    tot_wrote += n_wrote;
    nbytes = nbytes - n_wrote;
    loops++;
  } while (nbytes != 0);

  return(tot_wrote);
}