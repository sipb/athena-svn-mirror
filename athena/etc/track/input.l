%%

GLOBAL[\t\40-\176]*$	
		{
			char *gptr;
			gptr = yytext;
			skipspace(&gptr);	
			skipword(&gptr);
			skipspace(&gptr);
			strcpy(linebuf,gptr);
			return(GEXCEPT);
		}
\\[\t ]*;[\t\40-\176]*\n	
		{
			strcat(linebuf,"\n");
			return(BACKNEW);
		}
\;[\t\40-\176]*$	;
[ \t]+		{
			strncat(linebuf,yytext,yyleng);
			return(WHITESPACE);
		}
[\42-\71\74-\133\135-\176]+	{ /* everything except ! : ; and \  */
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
