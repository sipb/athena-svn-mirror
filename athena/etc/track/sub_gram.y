%{
# include "track.h"
extern FILE *yyin, *yyout;
char linebuf[1024];
char wordbuf[256][256];
int wordcnt = 0;
%}
%token WHITESPACE COLON BACKSLASH NEWLINE BACKNEW BANG WORD GEXCEPT
%%
sublist	: header entrylist opt_space 
	;
header	: 
		{
			entrycnt = 0;
			clear_ent();
			entrycnt = 1;
			clear_ent();
		}
	|   GEXCEPT opt_space COLON except COLON
		{
			savestr( &entries[ entrycnt].fromfile,
				"GLOBAL_EXCEPTIONS");
			entrycnt++;
			clear_ent();
		}
        ;
entrylist :
	  |	entrylist entry
	  ;
entry	: linkmark fromname COLON cmpname COLON toname COLON except COLON shellcmd
		{
			entrycnt++;
			clear_ent();
		}
	;
linkmark  :  opt_space
	  |  opt_space BANG opt_space
		{
			entries[entrycnt].followlink = 1;
		}
	  ;
fromname: WORD opt_space
		{
			savestr(&entries[entrycnt].fromfile,wordbuf[0]);
			KEYCPY( entries[ entrycnt].sortkey, wordbuf[0]);
			entries[ entrycnt].keylen = strlen( wordbuf[0]);
			doreset();
		}
	;
cmpname :	
		{
			if (entries[entrycnt].followlink)
			{
				savestr(&entries[entrycnt].cmpfile,
					resolve(entries[entrycnt].fromfile));
			}
			else
			{
				savestr(&entries[entrycnt].cmpfile,
						entries[entrycnt].fromfile);
			}
			doreset();
		}
	| opt_word
		{
			savestr(&entries[entrycnt].cmpfile,wordbuf[0]);
			doreset();
		}
	;
toname  :
		{
			if (entries[entrycnt].followlink)
			{
				savestr(&entries[entrycnt].tofile,
					resolve(entries[entrycnt].fromfile));
			}
			else
			{
				savestr(&entries[entrycnt].tofile,
						entries[entrycnt].cmpfile);
			}
			doreset();
		}
	| opt_word
		{
			savestr(&entries[entrycnt].tofile,wordbuf[0]);
			doreset();
		}
	;
except  :
		{
			doreset();
		}
	| opt_space wordlist opt_space
		{
			int i,j;
			char * r;
			for(i=0;entries[entrycnt].exceptions[i]!=(char*)0;i++)
			{
			}
			for (j=0;j<wordcnt;j++)
			{
				r = wordbuf[ j];
				if ( file_pat( r))
					r = re_conv( r);
				if ( duplicate( r, entrycnt)) continue;
				savestr(&entries[entrycnt].exceptions[i++], r);
			}
			qsort( (char *)&entries[ entrycnt].exceptions[ 0], i,
				sizeof( entries[	0].exceptions[ 0]),
				exceptcmp);
			doreset();
		}
	;
wordlist: WORD
	| wordlist opt_space WORD
	;
shellcmd: firstl shline NEWLINE
		{
			if (strlen(linebuf) > 1)
			{
				savestr(&entries[entrycnt].cmdbuf,linebuf);
			}
			else
			{
				savestr(&entries[entrycnt].cmdbuf,"");
			}
			doreset();
		} ;
shline	: 
	| shline WORD 
	| shline COLON
	| shline BANG
	| shline BACKSLASH
	| shline WHITESPACE
	;
firstl  : 
	| firstl shline BACKNEW 
	;
opt_word: opt_space WORD opt_space
	;
opt_space:
	| opt_space opt_ele
	;
opt_ele : NEWLINE
	| WHITESPACE
	;
	
%%

exceptcmp( p, q) char **p, **q; {
	return( strcmp( *p, *q));
}

yyerror(s)
char *s;
{
	fprintf(stderr,"parser error -- %s\n",s);
}
#include "lex.yy.c"
