/* tcom.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* Send a command to the tftp server */

#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	"srvr_cmds.h"

#define	LINSIZ	128
char	*lockfile = "/tftpd/lock";
char	*tftplog = "/tftpd/slog";
char	*showlog = "/exe/showlog";

main (argc, argv)
int	argc;
char	**argv;
{
	FILE	*cmdfd, *lockfd;
	char	line[LINSIZ];
	int	i, pid, child;

	if ((lockfd = fopen (lockfile, "r")) == NULL) {
		printf ("tftp daemon not alive");
		exit (1);
	}

	fscanf (lockfd, "%d", &pid);
	fclose (lockfd);
	
	if (argc > 1) {		/* run once? */
		
		if ((cmdfd = fopen (CMD_FILE, "w")) == NULL) {
			printf ("can't open command file\n");
			exit (1);
		}
		
		for (i = 1; i < argc; i++) {
			fputs (argv[i], cmdfd);
			putc (' ', cmdfd);
		}
	
		fclose (cmdfd);
	
		kill (pid, CMD_SIG);
	} else {			/* run interactively */
		if ((child = fork ()) == 0)	/* child */
			execl (showlog, showlog, tftplog, 0);
		printf ("(Type EOF to exit)\n");
		while (fgets (line, LINSIZ, stdin) != NULL) {
			char *ptr;
			
			if ((ptr = strchr(line, '\n')) != 0)
				*ptr = '\0';
			if ((cmdfd = fopen (CMD_FILE, "w")) == NULL) {
				printf ("can't open command file\n");
				exit (1);
			}
			fputs (line, cmdfd);
			fclose (cmdfd);
			kill (pid, CMD_SIG);
		}
		kill (child, SIGINT);	/* punt child */
	}
			
}
