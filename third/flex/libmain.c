/* libmain - flex run-time support library "main" function */

/* $Header: /afs/dev.mit.edu/source/repository/third/flex/libmain.c,v 1.1.1.1 2001-04-10 17:05:05 ghudson Exp $ */

extern int yylex();

int main( argc, argv )
int argc;
char *argv[];
	{
	while ( yylex() != 0 )
		;

	return 0;
	}
