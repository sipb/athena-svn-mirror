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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/c_io.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/c_io.c,v 1.2 1989-11-17 14:01:36 tjcoppet Exp $";
#endif

#include <olc/olc.h>

#include <sys/types.h>             /* System type declarations. */
#include <sys/socket.h>            /* Network socket defs. */
#include <sys/file.h>              /* File handling defs. */
#include <sys/stat.h>
#include <sys/time.h>              /* System time definitions. */
#include <netinet/in.h>
#include <errno.h>                 /* System error numbers. */
#include <netdb.h>
#include <signal.h>
#include <sgtty.h>              /* Terminal param. definitions. */
#include <setjmp.h>


extern char DaemonHost[];			
extern int errno;

static struct hostent *hp = (struct hostent *) NULL;      /* daemon host */
static struct servent *service = (struct servent *) NULL; /* service entry */
static jmp_buf env;

struct hostent *gethostbyname(); /* Get host entry of a host. */

#define	MIN(a,b)	((a)>(b)?(b):(a))

extern int select_timeout;

/*
 * Note: All functions that deal with I/O on sockets in this file use the
 *	functions "sread()" and "swrite()", which check to ensure that the
 *	socket is, in fact, connected to something.
 */


send_dbinfo(fd,dbinfo)
     int fd;
     DBINFO *dbinfo;
{
  DBINFO dbi;

  dbi = *dbinfo;
  dbi.max_ask    =  (int) htonl((u_long) dbinfo->max_ask);
  dbi.max_answer =  (int) htonl((u_long) dbinfo->max_answer);
  if (swrite(fd, &dbi, sizeof(DBINFO)) != sizeof(DBINFO))
    return(ERROR);

  return(SUCCESS);
}


read_dbinfo(fd,dbinfo)
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
     RESPONSE response;
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
     RESPONSE *response;
{
  return(read_int_from_fd(fd, response));
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
#ifdef	TEST
  printf("write_int_to_fd(%d, %d)\n", fd, response);
#endif	TEST
  
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
  char error[ERRSIZE];	/* Error message. */
  
#ifdef	TEST
  printf("read_text_into_file(%d, %s)\n", fd, filename);
#endif	TEST
	
  if (read_int_from_fd(fd, &nbytes) != SUCCESS) 
    {
      perror("read_text_into_file: unable to read nbytes from socket");
      return(ERROR);
    }
  
  if ((filedes = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) 
    {
      (void) sprintf(error, 
		     "read_text_into_file: Unable to open file %s",
		     filename);
      perror(error);
      return(ERROR);
    }
  
  if (nbytes > 0) 
    {
      while (nbytes) 
	{
	  int n;
	  if ((n=sread(fd, inbuf, MIN(BUFSIZ, nbytes))) == -1) 
	    {
	      perror("read_text_into_file: Error reading text");
#ifdef	TEST
	      fprintf(stderr, "n=%d\n", n);
#endif	TEST
	      (void) unlink(filename);
	      (void) close(filedes);
	      return(ERROR);
	    }
	  
	  if (swrite(filedes, inbuf, n) != n) 
	    {
	      perror("read_to_file: Error writing text");
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

#ifdef	TEST
  printf("write_file_to_fd: fd %d, fn %s\n", fd, filename);
#endif	TEST

  if (stat(filename, &statbuf) != 0) 
    {
      perror("write_file");
      nbytes = 0;
    }
  else
    nbytes = statbuf.st_size;

#ifdef	TEST
  printf("nbytes=%d\n", nbytes);
#endif	TEST

  if (nbytes <= 0) 
    {
      write_int_to_fd(fd, 0);
      return(ERROR);
    }
  
  if ((filedes = open(filename, O_RDONLY, 0)) < 0) 
    {
      perror("write_file: Unable to open file.");
      return(ERROR);
    }
  
  if (write_int_to_fd(fd, nbytes) != SUCCESS) 
    {
      perror("write_file: Error writing size.");
      close(filedes);
      return(ERROR);
    }
  
  while (nbytes) 
    {
      int n_read, n_wrote;
	
      n_read = sread(filedes, inbuf, MIN(BUFSIZ, nbytes));
      if (n_read == -1) 
	{
	  perror("write_file: Error reading text.");
	  (void) close(filedes);
	  return(ERROR);
	}

      if ((n_wrote=swrite(fd, inbuf, n_read)) != n_read) 
	{
	  perror("write_file: Error writing text");

#ifdef	TEST
	  fprintf(stderr, "n_read = %d, n_wrote = %d\n", n_read, n_wrote);
#else   TEST
	  n_wrote = n_wrote;
#endif	TEST
		
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

#ifdef	TEST
  printf("write_text_to_fd(%d, 0x%x)\n", fd, buf);
#endif	TEST
	
  nchars = strlen(buf);
  if (write_int_to_fd(fd, nchars) != SUCCESS) 
    {
      perror("write_text_to_fd: write_int_to_fd");
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
      perror("read_text: read_int");
      return((char *) NULL);
    }

  if ((rtff_buf = malloc((unsigned) (nchars + 1))) == (char *) NULL) 
    {
      perror("read_text: Can't allocate memory.");
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



#ifdef	TEST
#undef	BUFSIZ
#define	BUFSIZ 64
#endif	TEST



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

ERRCODE
write_chars_to_fd(fd, buf, nchars)
     int fd;
     char *buf;
     int nchars;
{

#ifdef	TEST
  printf("write_chars_to_fd(%d, 0x%x, %d)\n", fd, buf, nchars);
#endif	TEST

  while (nchars > BUFSIZ) 
    {
      if (swrite(fd, buf, BUFSIZ) != BUFSIZ) 
	{
	  perror("write_chars: Error writing text.");
	  return(ERROR);
	}
      buf += BUFSIZ;
      nchars -= BUFSIZ;
    }

  if (swrite(fd, buf, nchars) != nchars) 
    {
      perror("write_chars: Error writing text.");
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

ERRCODE
read_chars_from_fd(fd, buf, nchars)
     int fd;
     char *buf;
     int nchars;
{
  int n;

#ifdef	TEST
  printf("read_chars_from_fd(%d, 0x%x, %d)\n", fd, buf, nchars);
#endif	TEST
  
  while (nchars) 
    {
      if ((n = sread(fd, buf, MIN(BUFSIZ, nchars))) == -1) 
	{
	  perror("read_chars: Error reading text.");

#ifdef	TEST
	  fprintf(stderr, "sread=%d\n", n);
#endif	TEST
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

sread(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  struct timeval tval;	/* System time structure. */
  int read_fds;		/* File descriptors to check.*/
  register int n_read, s_val;

  if (nbytes <= 0)
    return(0);

  /*
   *  A necessary evil so that the daemon doesn't hang.
   */

  tval.tv_sec = select_timeout;
  tval.tv_usec = 0;
  
  read_fds = 1 << fd;
  if ((s_val = select(fd+1, &read_fds, 0, 0, &tval)) < 1) 
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

swrite(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  struct timeval tval;	        /* System time structure. */
  int write_fds;		/* File descriptors to check.*/
  register int n_wrote, s_val;

  if (nbytes <= 0)
    return(0);
	
  tval.tv_sec = select_timeout;
  tval.tv_usec = 0;
  
  write_fds = 1 << fd;
  if ((s_val = select(fd+1, 0, &write_fds, 0, &tval)) != 1) 
    {
      if (s_val == 0)
	errno = ETIMEDOUT;

#ifdef	TEST
      perror("swrite: select");
#endif	TEST
      return(-1);
    }
  
  n_wrote = write(fd, buf, nbytes);

#ifdef	TEST
  if (n_wrote != nbytes) 
    {
      fprintf(stderr, "%d wrote of %d\n", n_wrote, nbytes);
    }
#endif	TEST
  
  return(n_wrote);
}

