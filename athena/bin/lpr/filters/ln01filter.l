%{
#include <sys/file.h>
#include <signal.h>
extern int sighandler();
struct sigvec vector =
{
	sighandler,
	(int) 0xFFFFFFFF,
	(int) 0
};
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
	for(i = 1; i <= 27; ++i) {
		sigvec(i, &vector, 0);
	}
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

int
sighandler(sig)
register int sig;
{
	yywrap();
	exit(sig);
}

/* Local Modes: */
/* Mode:C */
/* End: */
