/* Mode: -*- C -*- */
%{
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

extern int sighandler();
char *reset="\033c";			/* ris	*/
char *land_length="\033[2550t";
char *landscape="\033[10m";
char *port_length="\033[3300t";		/* form length = 11 inches */
char *portrait="\033[11m";
char *beginunderline = "\033[4m";
char *endunderline = "\033[0m";
char *mode;
char *form_length;
%}
PRINTABLE    [0-9A-Za-z \t`\[\];'\\<,./-=~!@#$%^&*()_+{}:"|>?]
%%
\f			{
				printf("\f%s%s%s",reset,mode,form_length);
				fflush(stdout);
			}
(_\b{PRINTABLE})+	{
				register char *p = &yytext[0];
				printf(beginunderline);
				while(*p)
				{
					if((*p != '_') && (*p != '\b'))
					{
						putchar(*p);
					}
					++p;
				}
				printf(endunderline);
				fflush(stdout);
			}
\031\1			{ /* ^Y^A */
				/*
				 * lpd needs to use a different filter to
				 * print data so stop what we are doing and
				 * wait for lpd to restart us.
				 */
				fflush(stdout);
				kill(getpid(), SIGSTOP);
				printf("%s%s%s",reset,mode,form_length);
				fflush(stdout);
			}

%%

yywrap()
{
	struct flock lock;

	lock.l_type = F_UNLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	fflush(stdout);
	fcntl(STDOUT_FILENO, F_SETLK, &lock);
	return(1);
}
main(argc,argv)
int argc;
register char *argv[];
{
	struct flock lock;
	int i;

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
	fcntl(STDOUT_FILENO, F_SETLKW, &lock);
	if(!strcmp(argv[0], "ln01l")) {
		mode = landscape;
		form_length = land_length;
	}
	else {
		mode = portrait;
		form_length = port_length;
	}
	setbuf(stdout, NULL);
	printf("%s%s%s",reset,mode,form_length);
	fflush(stdout);
	while(1) {
			if(yylex() <= 0) break;
	}
}

/* Local Modes: */
/* Mode:C */
/* End: */
