/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v $
 *	$Author: treese $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v 1.1 1986-01-18 18:09:09 treese Exp $
 */

#ifndef lint
static char *rcsid_cref_utils_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v 1.1 1986-01-18 18:09:09 treese Exp $";
#endif	lint

/* This file is part of the CREF finder.  It contains miscellaneous useful
 *
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	$Log: not supported by cvs2svn $
 *
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/cref_utils.c,v 1.1 1986-01-18 18:09:09 treese Exp $";
#endif

#include <stdio.h>			/* Standard I/O definitions. */
#include <curses.h>			/* Curses package defs. */
#include <sys/param.h>			/* System parameters file. */
#include <grp.h>			/* System group defs. */

#include "cref.h"			/* Finder defs. */

/* Function:	err_abort() prints an error message and exits.
 * Arguments:	message:	Message to print.
 * Returns:	Doesn't return.
 * Notes:
 */

err_abort(message)
char *message;
{
	move(LINES, 0);
	echo();
	noraw();
	endwin();
	fprintf(stderr, "%s\n", message);
	exit(ERROR);
}

/* Function:	check_consultant() checks to see if the user is a
 *			consultant and sets the consultant flag.
 * Arguments:	None.
 * Returns:	TRUE if consultant, FALSE otherwise.
 * Notes:
 */

int
check_consultant()
{
	int group_ids[NGROUPS];			/* Array of group ID's. */
	struct group *consult_group;		/* Consult group entry. */
	int i;					/* Index variable. */

	if (getgroups(NGROUPS, group_ids) < 0)
		err_abort("cref: Unable to get group ID list.");
	if ( (consult_group = getgrnam(CONSULT_GROUP)) == NULL)
		{
		if ( (consult_group = getgrnam(ALT_GROUP)) == NULL)
			err_abort("cref: Unable to get group entry.");
		}
	for (i = 0; i < NGROUPS; i++)
		{
		if (group_ids[i] == consult_group->gr_gid)
			{
			set_consultant(TRUE);
			return(TRUE);
			}
		}
	set_consultant(FALSE);
	return(FALSE);
}
	
/* Function:	call_program() executes the named program by forking the
 *			main process.
 * Arguments:	program:	Name of the program to execute.
 *		argument:	Argument to be passed to the program.
 *	Note: Currently we support only a single argument, though it
 *	may be necessary to extend this later.
 * Returns:	An error code.
 * Notes:
 *	First, we fork a new process.  If the fork is unsuccessful, this
 *	fact is logged, and we return an error.  Otherwise, the child
 *	process (pid = 0) exec's the desired program, while the parent
 *	program waits for it to finish.
 */

ERRCODE
call_program(program, argument)
char *program;				/* Name of program to be called. */
char *argument;				/* Argument to be passed to program. */
{
	int pid;			/* Process id for forking. */
	char error[ERRSIZE];		/* Error message. */
	extern int errno;
	extern char *sys_errlist[];

	if ( (pid = fork() ) == -1)
		{
		sprintf(error, "Can't fork to execute %s\n", program);
	        message(1, error);
		return(ERROR);
		}
	else if ( pid == 0 )
		{
		execlp(program, program, argument, 0);
		sprintf(error,"Error execing %s: %s", program,
			sys_errlist[errno]);
		message(1, error);
		return(ERROR);
		}
	else	{
		wait(0);
		return(SUCCESS);
		}
}
