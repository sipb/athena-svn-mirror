#include "bellcore-copyright.h"
#include "mit-copyright.h"
%%

GLOBAL		{
			strncat(linebuf,yytext,yyleng);
			return(GEXCEPT);
		}
"->"		{
			strncat(linebuf,yytext,yyleng);
			return(ARROW);
		}
\\[\t ]*#[\t\40-\176]*\n	
		{
			strcat(linebuf,"\n");
			return(BACKNEW);
		}
#[\t\40-\176]*$	;
[ \t]+		{
			strncat(linebuf,yytext,yyleng);
			return(WHITESPACE);
		}
[\42\44-\71\73-\133\135-\176]+	{ /* everything except # : ! and \  */
			strncat(linebuf,yytext,yyleng);
			for ( wordp = yytext;
			     *wordp == '/' && wordp[1];
			      wordp++);
			strcpy(wordbuf, wordp);
			return(WORD);
		}
":"		{
			strncat(linebuf,yytext,yyleng);
			return(COLON);
		}
"!"		{
			strncat(linebuf,yytext,yyleng);
			return(BANG);
		}
"\\"		{
			strncat(linebuf,yytext,yyleng);
			return(BACKSLASH);
		}
"\n"		{
			strncat(linebuf,yytext,yyleng);
			return(NEWLINE);
		}
"\\\n"		{
			strcat(linebuf,"\n");
			return(BACKNEW);
		}
"\177"		{
			strcat(linebuf,"\n");
			return(NEWLINE);
		}
%%
#undef input
/* this version of input() is identical to yylex' original,
 * except for the expression "endchar^='\177'", which is
 * returned at end-of-file, and which is 0 in the original version.
 * this expression will return '\177' on the 1st detection of EOF,
 * and 0 (as usually happens at EOF) at the next invocation of input().
 * our version allows yylex() to tokenize EOF, and yyparse() to parse it,
 * so that an entry's command-field can optionally end without a newline.
 * track supports this possibility, because emacs doesn't automatically
 * terminate a file's last line with a newline.
 */
static unsigned char endchar = '\0';
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?\
(yylineno++,yytchar):yytchar)==EOF?(endchar^='\177'):yytchar)
