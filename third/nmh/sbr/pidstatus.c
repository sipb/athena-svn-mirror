
/*
 * pidstatus.c -- report child's status
 *
 * $Id: pidstatus.c,v 1.1.1.1 1999-02-07 18:14:09 danw Exp $
 */

#include <h/mh.h>

/*
 * auto-generated header
 */
#include <sigmsg.h>

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifndef WTERMSIG
# define WTERMSIG(s) ((int)((s) & 0x7F))
#endif

#ifndef WCOREDUMP
# define WCOREDUMP(s) ((s) & 0x80)
#endif

int
pidstatus (int status, FILE *fp, char *cp)
{
    int signum;

/*
 * I have no idea what this is for (rc)
 * so I'm commenting it out for right now.
 *
 *  if ((status & 0xff00) == 0xff00)
 *      return status;
 */

    /* If child process returned normally */
    if (WIFEXITED(status)) {
	if ((signum = WEXITSTATUS(status))) {
	    if (cp)
		fprintf (fp, "%s: ", cp);
	    fprintf (fp, "exit %d\n", signum);
	}
    } else if (WIFSIGNALED(status)) {
	/* If child process terminated due to receipt of a signal */
	signum = WTERMSIG(status);
	if (signum != SIGINT) {
	    if (cp)
		fprintf (fp, "%s: ", cp);
	    fprintf (fp, "signal %d", signum);
	    if (signum >= 0 && signum < sizeof(sigmsg) && sigmsg[signum] != NULL)
		fprintf (fp, " (%s%s)\n", sigmsg[signum],
			 WCOREDUMP(status) ? ", core dumped" : "");
	    else
		fprintf (fp, "%s\n", WCOREDUMP(status) ? " (core dumped)" : "");
	}
    }

    return status;
}