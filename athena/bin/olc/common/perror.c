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
 *	$Id: perror.c,v 1.10 1999-03-06 16:48:17 ghudson Exp $
 */


#if !defined(lint) && !defined(SABER)
static char rcsid[] ="$Id: perror.c,v 1.10 1999-03-06 16:48:17 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include "config.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>           /* Time-related definitions. */

#include "common.h"

#ifndef HAVE_STRERROR
extern    const char *const sys_errlist[];
#define   strerror(x)   (sys_errlist[(x)])
#endif  /* don't HAVE_STRERROR */

char *wday[] = 
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
} ;

char *month[] = 
{
  "January", "February", "March", "April", "May", "June", 
  "July", "August", "September", "October", "November", "December",
};

/* Convert "struct tm" data into a formatted string.
 * Arguments:	time_buf:	Buffer to store formatted time (>= 22 chars!)
 *		time_info:	Pointer to a struct tm containing time data.
 * Returns:	Nothing.
 */
char *format_time(char *time_buf, struct tm *time_info)
{
  int hour;

  hour = time_info->tm_hour;

  if(hour > 12)  hour -= 12;
  if(hour == 0)  hour = 12;	/* If it's the midnight hour... */

  /* We're not very paranoid about illegal values of struct tm fields
   * here.  Numeric fields are expected to be in the 00..99 range, except
   * for tm_year which will be 100 in the year 2000.
   */
  sprintf(time_buf, "%3.3s %02u-%3.3s-%02u %2u:%02u%2.2s",
	  wday[time_info->tm_wday],
	  time_info->tm_mday,
	  month[time_info->tm_mon], 
	  ((unsigned)time_info->tm_year % 100),
	  hour,
	  time_info->tm_min,
	  time_info->tm_hour > 11 ? "pm" : "am");
  return(time_buf);
}

/* Convert the current time into a formatted string.
 * Arguments:	time_buf:	Buffer to store formatted time (>= 22 chars!)
 * Returns:	Nothing.
 */
void time_now(char *time_buf)
{
  long current_time;     	/* Current time. */
  struct tm *time_info;

  time(&current_time);
  time_info = localtime(&current_time);
  format_time(time_buf,time_info);
}


/* This is similar to perror(), except a datestamp precedes the message
 * printed.
 * Arguments:	msg:	Message to print.
 * Returns:	nothing
 */
static void olc_std_perror(const char *msg)
{
  char *errmsg;
  static char time_buf[25];

  errmsg = strerror(errno);
  time_now(time_buf);

  fseek(stderr, 0L, SEEK_END); /* this is, of course, sheer paranoia */
  fprintf(stderr, "%s %s%s%s\n",
	  time_buf,
	  (msg && *msg) ? msg  : "",
	  (msg && *msg) ? ": " : "",
	  errmsg ? errmsg : "[unknown error]");
}

/* "olc_perror" points to a function used to print out errors from
 * libcommon.  The function takes a const char* argument and returns void.
 * We use a variable so an alternate way of printing errors can be provided.
 */
olc_perror_type olc_perror = olc_std_perror;

/* set_olc_perror() is the advertised interface for changing the
 * error-printing function.
 * Arguments:	perr:	New error-printing function.  Must take const char*
 *			and return void.  NULL means print to stderr, with
 *			a datestamp.
 * Returns:	Previous error-printing function.
 * Note: Without the typedef, the function declaration would be
 *         void (*set_olc_perror(void (*perr)(const char*)))(const char*)
 *       [This is a warning for the unwary, not a suggestion. =)]
 */
olc_perror_type set_olc_perror(olc_perror_type perr)
{
  olc_perror_type prev = olc_perror;
  olc_perror = (perr ? perr : olc_std_perror);
  return ((prev == olc_std_perror) ? NULL : prev);
}
