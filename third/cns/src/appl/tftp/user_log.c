/* user_log.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* This file contains the routines to log errors in the user tftp program.
 * It is a separate file because these routines differ in the user and
 * server programs.
 */

#include	<stdio.h>
#include	<sys/types.h>


cn_log (estring, ecode, arg)

/* Log the message estring on the standard error.  The ecode argument is
 * the tftp error code, which is ignored; arg is an argument to printf
 * for the estring.
 *
 * Arguments:
 */

char	*estring;			/* printf format error message */
int	ecode;				/* tftp error code */
caddr_t	arg;				/* general purpose argument */
{
	fprintf (stderr, estring, arg);
	fflush (stderr);
}


cn_inform (msg, arg)

/* This routine is called to log an informative message.  Since the
 * user is presumably uninterested in such messages, the routine
 * does nothing.
 *
 * Arguments:
 */

char	*msg;				/* printf format msg */
caddr_t	arg;				/* argument */
{
}
