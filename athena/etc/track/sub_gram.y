%{
# include "track.h"
extern FILE *yyin, *yyout;
char linebuf[1024];
char wordbuf[256][256];
int wordcnt = 0;
%}
%token WHITESPACE COLON BACKSLASH NEWLINE BACKNEW BANG WORD GEXCEPT ENDOFFILE
%%
sublist	: header entrylist opt_space 
	;
header	: 
		{
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
entry	: linkmark fromname COLON toname COLON cmpname COLON except COLON shellcmd opt_newline
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
			char *r = wordbuf[0];
			while ( '/' == *r) r++;
			savestr(&entries[entrycnt].fromfile,r);
			KEYCPY( entries[ entrycnt].sortkey, r);
			entries[ entrycnt].keylen = strlen( r);
			doreset();
		}
	;
toname  : opt_space
		{
			char *defname;

			defname = entries[ entrycnt].followlink ?
			 resolve( entries[ entrycnt].fromfile) :
				  entries[ entrycnt].fromfile;

			savestr( &entries[ entrycnt].tofile, defname);
			doreset();
		}
	| opt_word
		{
			char *r = wordbuf[0];
			while ( '/' == *r) r++;
			if ( ! entries[ entrycnt].followlink)
				savestr(&entries[entrycnt].tofile,r);
			else	savestr(&entries[entrycnt].tofile,
					 resolve( r));
			doreset();
		}
	;
cmpname : opt_space
		{
			char * defname;

			defname = writeflag ?
				entries[ entrycnt].fromfile :
				entries[ entrycnt].tofile;

			if ( entries[entrycnt].followlink)
				defname = resolve( defname);

			savestr( &entries[ entrycnt].cmpfile, defname);

			doreset();
		}
	| opt_word
		{
			char *r = wordbuf[0];
			while ( '/' == *r) r++;
			if ( ! entries[ entrycnt].followlink)
				savestr(&entries[entrycnt].cmpfile,r);
			else	savestr(&entries[entrycnt].cmpfile,
					 resolve( r));
			doreset();
		}
	;
except  : opt_space
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
				while ( '/' == *r && r[1]) r++;
				if ( file_pat( r))
					r = re_conv( r);
				if ( duplicate( r, entrycnt)) continue;
				if ( i >= WORDMAX) {
					sprintf( errmsg,
						"exception-list overflow\n");
					do_panic();
				}
				savestr(&entries[entrycnt].exceptions[i++], r);
			}
			doreset();
		}
	;
wordlist: WORD
	| wordlist opt_space WORD ;
shellcmd: | WHITESPACE
		{
			savestr(&entries[entrycnt].cmdbuf,"");
			doreset();
		}
	| shline
		{
			savestr(&entries[entrycnt].cmdbuf,linebuf);
			doreset();
		}
	;
shline:   black
	| shline black
	| shline BACKNEW
	;
black:	  WORD
	| COLON
	| BANG
	| BACKSLASH
	;
opt_word: opt_space WORD opt_space
	;
opt_space:
	| opt_space opt_ele
	;
opt_ele : NEWLINE | WHITESPACE
	;
opt_newline: NEWLINE | NEWLINE ENDOFFILE | ENDOFFILE
	;
%%

yyerror(s)
char *s;
{
	fprintf(stderr,"parser error -- %s\n",s);
}
#include "lex.yy.c"
