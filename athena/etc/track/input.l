%%

GLOBAL		{
			char * patt;
			while ( patt = next_def_except())
				strcpy( wordbuf[ wordcnt++], patt); /* XXX */
			strcat(linebuf,yytext,yyleng);
			return(GEXCEPT);
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
			strcpy(wordbuf[wordcnt++],yytext);
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
%%
