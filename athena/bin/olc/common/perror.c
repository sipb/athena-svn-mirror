/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the slightly modified perror function
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/perror.c,v $
 *	$Id: perror.c,v 1.2 1991-02-24 11:27:33 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/time.h>           /* System time definitions. */
#ifdef m68k
#include <time.h>
#endif

char *wday[] = 
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
} ;

char *month[] = 
{
  "January", "February", "March", "April", "May", "June", 
  "July", "August", "September", "October", "November", "December",
};

extern struct tm *localtime();

/*
 * Function:	time_now() returns a formatted string with the current time.
 * Arguments:	time_buf:	Buffer to store formatted time.
 * Returns:	Nothing.
 * Notes:
 *	First, get the current time and format it into a readable form.  Then
 *	copy the hour and minutes into the front of the buffer, retaining only
 *	the time through the minutes.
 */
char *format_time(time_buf,time_info)
     char *time_buf;
     struct tm *time_info;
{
  int hour;

  hour = time_info->tm_hour;

  if(hour > 12)  hour -= 12;
  if(hour == 0)  hour = 12;	/* If it's the midnight hour... */

  (void) sprintf(time_buf, "%3.3s %s%d-%3.3s-%s%d %s%d:%s%d%s",
		 wday[time_info->tm_wday],
		 time_info->tm_mday > 9 ? "" : "0", 
		 time_info->tm_mday,
		 month[time_info->tm_mon], 
		 time_info->tm_year > 9 ? "" : "0",
		 time_info->tm_year,
		 hour > 9 ? "" : " ",
		 hour,
		 time_info->tm_min > 9 ? "" : "0", 
		 time_info->tm_min,
		 time_info->tm_hour > 11 ? "pm" : "am");
  return(time_buf);
}

void time_now(time_buf)
     char *time_buf;
{
  long current_time;     	/* Current time. */
  struct tm *time_info;

  (void) time(&current_time);
  time_info = localtime(&current_time);
  format_time(time_buf,time_info);
}


/*
 * Function:	perror() similar to that of the C library, except that
 *	a datestamp precedes the message printed.
 * Arguments:	msg:	Message to print.
 * Returns:	nothing
 *
 */

extern int sys_nerr;
extern char *sys_errlist[];
extern int errno;
static char time_buf[25];
 
#ifndef m68k
/* punt it all, the mac can't deal with multiply defined symbols */
#ifdef mips
int errno; /* declared in same file as perror in libc,
	      so we must declare it here to avoid
	      multiple defs & linker lossage */
#endif

void
#if __STDC__
perror(char *msg)
#else
perror(msg)
     char *msg;
#endif
{
	register int error_number;
	struct iovec iov[6];
	register struct iovec *v = iov;

	error_number = errno;

	time_now(time_buf);
	v->iov_base = time_buf;
	v->iov_len = strlen(time_buf);
	v++;

	v->iov_base = " ";
	v->iov_len = 1;
	v++;

	if (msg) {
		if (*msg) {
			v->iov_base = (char *) msg;
			v->iov_len = strlen(msg);
			v++;
			v->iov_base = ": ";
			v->iov_len = 2;
			v++;
		}
	}

	if (error_number < sys_nerr)
		v->iov_base = sys_errlist[error_number];
	else
		v->iov_base = "Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;

	v->iov_base = "\n";
	v->iov_len = 1;
	(void) lseek(2, 0L, L_XTND);
	(void) writev(2, iov, (v - iov) + 1);
}

#endif
