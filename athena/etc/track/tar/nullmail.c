

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/tar/nullmail.c,v 1.1 1993-10-12 04:48:52 probe Exp $";
#endif

#include <stdio.h>
#include "bellcore-copyright.h"
#include "mit-copyright.h"

#define	LINELN	256

extern FILE *popen();
/*
**	program to swollow null message, but exec mail if necessary
*/
main(argc,argv)
int argc;
char **argv;
{
	int buf,i,tmpstat;
	char errline[LINELN],mailcmd[LINELN];
	static char errmsg[LINELN] = "";
	FILE *paper,*mailpipe;

	if ((buf = getc(stdin))!=  EOF)
	{
		if (argc < 2)
		{
			sprintf(errmsg,"not enough arguments");
			goto trouble;
		}
		strcpy(mailcmd,"/bin/mail");
		for (i=1;i<argc;i++)
		{
			strcat(mailcmd," ");
			strcat(mailcmd,argv[i]);
		}
		if ((mailpipe = popen(mailcmd, "w")) == NULL)
		{
			sprintf(errmsg,"can't execute mail command");
			goto trouble;
		}
		putc(buf,mailpipe);	/* output first char */
		/*
		**	keep reading and writing
		*/
		while((buf =getc(stdin)) != EOF )
		{
			putc(buf,mailpipe);
		}
		/*
		**	mail swollows an unfinished line - damn.
		**	stick an extra newline
		*/
		putc('\n',mailpipe);
		if ((tmpstat = pclose(mailpipe)) != 0)
		{
			sprintf(errmsg,"bad return code octal -- %o",tmpstat);
			goto trouble;
		}
		exit(0);

		/*
		**	shouldn't get here . . . complain like hell
		*/
trouble:;
		sprintf(errline,
			"\n\r\t\tWARNING %s FAILED -- %s\n\r\t\t\tmail from automatic software installation probably lost\n\r",
			argv[0],errmsg);
		if ((paper = fopen("/dev/console","w")) != NULL)
		{
			fprintf(paper,"%s\n",errline);
		}
		fprintf(stderr,"%s\n",errline);
		exit(1);
	}
	exit(0);
}
