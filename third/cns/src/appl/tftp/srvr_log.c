/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"


/* This file contains the routines that log error messages for the
 * server version of tftp.  It is in a separate file because these
 * routines differ in the server and user versions of the program.
 */

#include	<stdio.h>
#include	<sys/types.h>


cn_log (estring, ecode, arg)

/* Log the message estring on the standard error, preceeded by
 * process id and date/time.  The ecode argument is the tftp error code;
 * arg is an argument to printf for the estring.
 *
 * Arguments:
 */

char	*estring;			/* printf format error message */
int	ecode;				/* tftp error code */
caddr_t	arg;				/* general purpose argument */
{
	time_t	now;			/* current time */
	
	time (&now);
	fprintf (stderr, "\nPid %d %sCode = %d\n",
		getpid (), ctime (&now), ecode);
	fprintf (stderr, estring, arg);
	fflush (stderr);
}


cn_inform (msg, arg)

/* Log an informative message (as opposed to an error message).  This
 * is a separate routine because the user doesn't want to see these
 * messages.
 *
 * Arguments:
 */

char	*msg;				/* error message */
caddr_t	arg;				/* argument to error message */
{
	fprintf (stderr, msg, arg);
	fflush (stderr);
}
