/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/filters/ln01filter.l,v $ */
/* $Author: builder $ */
/* $Locker:  $ */
/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/filters/ln01filter.l,v 1.1 1985-04-28 16:49:52 builder Exp $ */
%{
#include <sys/file.h>
#include <signal.h>
#undef YYLMAX
#define YYLMAX 1000
extern int sighandler();
struct sigvec vector =
{
	sighandler,
	(int) 0xFFFFFFFF,
	(int) 0,
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
char *lockfile = "/usr/lib/lpdfilters/ln.lock";
%}
PRINTABLE    [0-9A-Za-z \t`\[\];'\\<,./-=~!@#$%^&*()_+{}:"|>?]
%%
\f			printf("\f%s%s%s",reset,mode,form_length);
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
			}
%{
/*
{PRINTABLE}+/(\b)+_+	{
				register char *p;
				register int i;
				register char c;
				if(yyleng > 200)
				{
					ECHO;
					break;
				}
				p = &yytext[yyleng];
				while(c = input(), c == '\b')
				{
					if(p > yytext)
					{
						--p;
					}
				}
				unput(c);
				for(i = 0; &yytext[i] < p; ++i)
				{
					putchar(yytext[i]);
				}
				printf("%s", beginunderline);
				while(c = input(), c == '_')
				{
					if(p > &yytext[yyleng - 1])
					{
						break;
					}
					putchar(*p++);
				}
				printf("%s", endunderline);
				unput(c);
			}
_+/(\b)+{PRINTABLE}+	{
				register int i = yyleng;
				register int j;
				register char c;
				if(i > 200)
				{
					ECHO;
					break;
				}
				while(c = input(), c == '\b')
				{
					if(i > 0)
					{
						--i;
					}
				}
				unput(c);
				for(j = 0; j < i; ++j)
				{
					putchar('_');
				}
				printf("%s", beginunderline);
				while(c = input(), !((c == '\n') || (c == '\f')))
				{
					if(i < yyleng)
					{
						putchar(c);
						++i;
					}
					else
					{
						break;
					}
				}
				unput(c);
				printf("%s", endunderline);
			}
*/
%}

%%

yywrap()
{
	printf("\f%s",reset);
	flock(1, LOCK_UN);

	return(1);
}
main(argc,argv)
int argc;
register char *argv[];
{
	register int i;
	for(i = 1; i <= 27; ++i)
	{
		sigvec(i, &vector, 0);
	}
	while(flock(1, LOCK_EX) < 0)
	{
		sleep(2);
	}
	if(atoi(&argv[1][2]) > 80)
	{
		mode = landscape;
		form_length = land_length;
	}
	else
	{
		mode = portrait;
		form_length = port_length;
	}
	setbuf(stdout, NULL);
	printf("%s%s%s",reset,mode,form_length);
	while(yylex() > 0);
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
