%{
#undef yywrap
int num_lines = 0;
%}

PC	[^\"]
AN	[A-Z_a-z0-9]
%%
\n		++num_lines;

error_table	return ERROR_TABLE;
et		return ERROR_TABLE;
error_code	return ERROR_CODE_ENTRY;
ec		return ERROR_CODE_ENTRY;
end		return END;

[\t\n ]		;

\"{PC}*\"	{ register char *p; yylval.dynstr = ds(yytext+1);
		  if (p=strrchr(yylval.dynstr, '"')) *p='\0';
		  return QUOTED_STRING;
		}

{AN}*	{ yylval.dynstr = ds(yytext); return STRING; }

#.*\n		;

.		{ return (*yytext); }
%%
#ifndef lint
static char rcsid_et_lex_lex_l[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/et/et_lex.lex.l,v 1.4 1997-11-03 13:03:21 ghudson Exp $";
#endif

int yywrap()
{
	return 1;
}
