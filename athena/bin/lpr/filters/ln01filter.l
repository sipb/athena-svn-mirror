/* Mode: -*- C -*- */
%{
#ifdef _AUX_SOURCE
#include <sys/types.h>
#endif
#include <sys/file.h>
#include <signal.h>
#ifdef SOLARIS
/*
 * flock operations.
 */
#define LOCK_SH               1       /* shared lock */
#define LOCK_EX               2       /* exclusive lock */
#define LOCK_NB               4       /* don't block when locking */
#define LOCK_UN               8       /* unlock */
#endif
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
	fflush(stdout);
	flock(1, LOCK_UN);
	return(1);
}
main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	flock(1, LOCK_EX);
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
