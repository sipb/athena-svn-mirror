/* srvr_cmds.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* This file contains the command processing for the tftp server.
 * Commands are given to the tftp server by writing them into a
 * command file and sending a special interrupt signal to the server.
 * The presently implemented commands do such things as changing the
 * log file, turning packet tracing on and off, and so forth.
 */


#include	<stdio.h>
#include	<sys/types.h>
#include	<netinet/in.h>
#include	<sys/times.h>
#include	"tftp.h"
#include	"conn.h"
#include	"srvr_cmds.h"

#define	LINSIZ		128		/* max. command line size */


/* Table of supported commands */

#define	C_INTRACE	0		/* input packet tracing */
#define	C_OUTTRACE	1		/* output packet tracing */
#define	C_TRACE		2		/* all tracing */
#define	C_EXIT		3		/* exit */
#define	C_TIMES		4		/* get times */
#define	C_HELP		5		/* get help */
#define	C_UPTIME	6		/* get daemon up time */

static	struct	cmd_tab	{
	char	*cm_name;		/* the command string */
	int	cm_tag;			/* the command tag */
} cmd_tab[] = {
	{ "input_trace", C_INTRACE },
	{ "output_trace", C_OUTTRACE },
	{ "trace", C_TRACE },
	{ "exit", C_EXIT },
	{ "times", C_TIMES },
	{ "help", C_HELP },
	{ "uptime", C_UPTIME },
	{ "", 0 },			/* last entry */
};
char	cmd_intrpt;			/* command interrupt flag */
extern	char	*lockfile;
extern	char	*parse_cmd();
extern	struct	cmd_tab	*find_cmd();
extern	int	cmdint();


init_cmd ()

/* Set up the command processing.  Open the command file, set up to
 * catch the command signal, etc.
 */
{
	signal (CMD_SIG, cmdint);	/* catch command interrupt */
	return (TRUE);
}


do_cmd (cn)

/* This routine is called when a command present signal is received.  It
 * reads the commands from the command file and performs them.
 *
 * Arguments:
 */

struct	conn	*cn;			/* current connection */
{
	char	line[LINSIZ];		/* for command line */
	char	cmdname[LINSIZ];	/* command from line */
	char	*arg;			/* argument to command */
	FILE	*cmdfd;			/* command file descriptor */
	register struct	cmd_tab	*cmd;	/* the command struct */
	struct	tms	tbuf;		/* for system times */
	extern	time_t	start;		/* daemon start time */
	extern	char	*trim();
	
	if ((cmdfd = fopen (CMD_FILE, "r")) == NULL) {
		cn_log ("Unable to open command file", 0, 0);
		return (FALSE);
	}
	
	fseek (cmdfd, 0L, 0L);		/* to beginning */
	
	while (fgets (line, LINSIZ, cmdfd) != NULL) {
		cn_inform ("\nServer command: %s\n", line);		
		arg = parse_cmd (line, cmdname);
		arg = trim (arg);	/* trim leading white space */
		if ((cmd = find_cmd (cmdname, cmd_tab)) == NULL) /* error */
			continue;
		switch (cmd->cm_tag) {	/* found one; do it */
case C_INTRACE:				/* input packet tracing */
			if (prefix ("on", arg) >= 0)
				cn->intrace = TRUE;
			else
				cn->intrace = FALSE;
			break;
case C_OUTTRACE:			/* output packet tracing */
			if (prefix ("on", arg) >= 0)
				cn->outtrace = TRUE;
			else
				cn->outtrace = FALSE;
			break;
case C_TRACE:				/* both input and output tracing */
			if (prefix ("on", arg) >= 0)
				cn->intrace = cn->outtrace = TRUE;
			else
				cn->intrace = cn->outtrace = FALSE;
			break;
case C_EXIT:				/* exit */
			cn_close (cn);
			unlink (lockfile);
			exit (0);
case C_TIMES:				/* get process times */
			times (&tbuf);
			fprintf(stderr, "\nParent user time %ld\n",
				(long) tbuf.tms_utime);
			fprintf(stderr, "Parent system time %ld\n",
				(long) tbuf.tms_stime);
			fprintf(stderr, "Child user time %ld\n",
				(long) tbuf.tms_cutime);
			fprintf(stderr, "Child system time %ld\n",
				(long) tbuf.tms_cstime);
			fflush (stderr);
			break;
case C_HELP:				/* display help */
			show_help ();
			break;
case C_UPTIME:				/* display daemon up time */
			fprintf(stderr, "\nTFTP Daemon up since %s", ctime (&start));
			fflush (stderr);
			break;
		}
	}
	fclose (cmdfd);
}


show_help ()

/* Show the help stuff into the log.
 */
{
	fprintf(stderr,"\nCommands are:\n");
	fprintf(stderr,"help			print this cruft\n");
	fprintf(stderr,"input_trace on|off	turn input packet tracing on or off\n");
	fprintf(stderr,"output_trace on|off	turn output packet tracing on or off\n");
	fprintf(stderr,"trace on|off		turn all packet tracing on or off\n");
	fprintf(stderr,"times			display process times\n");
	fprintf(stderr,"uptime			display daemon start time\n");
	fprintf(stderr,"exit			force TFTP server to exit\n");
	fflush (stderr);
}


char *parse_cmd (line, cmd)

/* Parse off the command from the specified line.  The command is everything
 * up to the first space, tab, or newline, or the end-of-string.  The
 * command is copied into the buffer cmd, null-terminated.
 * Returns a pointer to the first character after the command, or the
 * null at the end of the string.
 */

register char	*line;
register char	*cmd;
{
	while (*line != ' ' && *line != '\t' && *line != '\n' && *line != '\0')
		*cmd++ = *line++;
	*cmd = '\0';
	return (line);
}
 

struct cmd_tab *find_cmd (cmd, tab)

/* Find the specified command in the specified command table.  Only enough
 * of the command to unambiguously identify it needs to be supplied.  If
 * the command is not found in the table or is ambiguous, an appropriate
 * error message is printed and 0 is returned.  Otherwise the index
 * of the command in the table is returned.
 */

register char	*cmd;
register struct	cmd_tab	*tab;
{
	register struct	cmd_tab	*found = 0;
	
	for (; *(tab->cm_name) != 0; tab++)
		if (prefix (cmd, tab->cm_name) >= 0)
			if (found != 0) {
				cn_inform ("Ambiguous command\n");
				return (0);
			} else
				found = tab;
	if (found == 0) {
		cn_inform ("Unknown command\n");
		return (0);
	}
	return (found);
}
				

prefix (s1, s2)

/* Returns > 0 iff string s1 is a prefix of string s2; ie if
 * strlen(s1) <= strlen(s2) and s1 == the first strlen(s1) characters
 * of s2.  Returns == 0 iff s1 == s2.  Otherwise returns < 0.
 */

register char	*s1, *s2;
{
	while (*s1 != '\0')
		if (*s1++ != *s2 || *s2++ == '\0')
			return (-1);
	return (*s2 == 0 ? 0 : 1);
}


char	*trim (str)

/* Trim the leading linear white space characters from the specified string.
 * Return a pointer to the first non-LWSP character in the string, or to
 * the NULL at the end of the string if none.
 *
 * Arguments:
 */

register char	*str;			/* string */
{
	while (*str != 0)
		if (*str != ' ' && *str != '\t' && *str != '\n')
			break;
		else
			str++;
	return (str);
}


cmdint (signo)

/* Signal handling routine for the command present signal.  Just sets the
 * command interrupt occurred flag and returns.
 *
 * Arguments:
 */

int	signo;
{
	cmd_intrpt = TRUE;
	signal (signo, cmdint);
}
