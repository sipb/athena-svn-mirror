%{
#include "bellcore-copyright.h"
#include "mit-copyright.h"
#include "track.h"
extern FILE *yyin, *yyout;
char linebuf[1024];
char wordbuf[256];
char *wordp;
int wordcnt = 0;
Entry* e = &entries[ 0];
%}
%token ARROW WHITESPACE COLON BACKSLASH NEWLINE BACKNEW BANG WORD GEXCEPT ENDOFFILE
%%
sublist	: opt_space header entrylist
	{
	    entnum = 0;	/* signifies to printmsg() that parse is complete. */
	}
	;
header	:
	{
	    clear_ent();
	    entrycnt = 1;
	    e = clear_ent();
	}
	| GEXCEPT opt_space COLON except COLON opt_space
	{
	    savestr( &e->fromfile, "GLOBAL_EXCEPTIONS");

	    /* cram compiled-in default exceptions onto lists:
	     */
	    while ( wordp = next_def_except()) {
		if ( file_pat( wordp))
		     add_list_elt( re_conv( wordp), DONT_TRACK, &e->patterns);
		else add_list_elt( wordp,	  DONT_TRACK, LIST( e->names));
	    }
	    /* XXX: global names should match both entry-children and
	     * should match occurences which are deep in an entry.
	     * it's ok to use linebuf for this,
	     * because we've processed linebuf's contents already.
	     */
	    for ( wordp = TEXT( e->names.table); wordp;
		  wordp = TEXT( NEXT( wordp))) {
		strcpy( linebuf, "*/");
		strcat( linebuf, wordp);
		add_list_elt( re_conv( linebuf), DONT_TRACK, &e->patterns);
	    }
	    if ( e->names.table) { /* lie, to make extra-roomy global table */
		 e->names.shift *= 8;
		 list2hashtable( &e->names);
	    }
	    entrycnt++;
	    e = clear_ent();
	}
        ;
entrylist :
	  |	entrylist entry opt_space
	;
entry: linkmark fromname COLON toname COLON cmpname COLON except COLON shellcmd
	{
	    if ( e->names.table) list2hashtable( &e->names);
	    entrycnt++;
	    e = clear_ent();
	}
	;
linkmark  :
	  | BANG opt_space
	{
	    sprintf( errmsg, "followlink isn't supported.\n");
	    do_gripe();
	    /* e->followlink = 1; */
	}
	;
fromname: WORD opt_space
	{
	    char *r = wordbuf;
	    while ( '/' == *r) r++;
	    savestr(&e->fromfile,r);
	    KEYCPY( e->sortkey, r);
	    e->keylen = strlen( r);
	    doreset();
	}
	;
toname  : opt_space
	{
	    char *defname;

	    defname = e->followlink ?
	     resolve( e->fromfile, fromroot) :
		      e->fromfile;

	    savestr( &e->tofile, defname);
	    doreset();
	}
	| opt_word
	{
	    char *r = wordbuf;
	    while ( '/' == *r) r++;
	    if ( ! e->followlink)
		    savestr(&e->tofile,r);
	    else	savestr(&e->tofile,
			     resolve( r, toroot));
	    doreset();
	}
	;
cmpname : opt_space
	{
	    char *defname, *root;

	    if ( writeflag) {
		    defname = e->fromfile;
		    root = fromroot;
	    }
	    else {
		    defname = e->tofile;
		    root = toroot;
	    }

	    if ( e->followlink)
		    defname = resolve( defname, root);

	    savestr( &e->cmpfile, defname);

	    doreset();
	}
	| opt_word
	{
	    char *r = wordbuf;
	    while ( '/' == *r) r++;
	    if ( ! e->followlink)
		    savestr(&e->cmpfile,r);
	    else	savestr(&e->cmpfile,
			     resolve( r, writeflag	? fromroot
						    : toroot));
	    doreset();
	}
	;
except  : opt_space
	| opt_space exlist opt_space
	{
	    doreset();
	}
	;
exlist: exceptword
	| exlist opt_space exceptword
	;
exceptword:   WORD
	{
	    wordp = wordbuf;		/* XXX lex returns longest match. */
	    wordcnt = DONT_TRACK;	/*     ->foo will thus come here. */
	    if ( ! strncmp( wordp, "->", 2)) {
		 wordp += 2;
		 wordcnt = FORCE_LINK;
	    }
	    if ( file_pat( wordp))
		 add_list_elt( re_conv( wordp), wordcnt, &e->patterns);
	    else add_list_elt( wordp,		wordcnt, LIST( e->names));
	}
	| ARROW opt_space WORD
	{
	    /* set force_links bit, add to e->names.
	     */
	    if ( file_pat( wordbuf))
		 add_list_elt( re_conv( wordbuf), FORCE_LINK, &e->patterns);
	    else add_list_elt( wordbuf,		  FORCE_LINK, LIST( e->names));
	}
	;
shellcmd: nullcmd NEWLINE
	{
	    savestr(&e->cmdbuf,"");
	    doreset();
	}
	| nullcmd shline NEWLINE
	{
	    savestr(&e->cmdbuf,linebuf);
	    doreset();
	}
	;
nullcmd:
	| nullcmd WHITESPACE
	| nullcmd BACKNEW
	;
shline:   black
	| shline black
	| shline WHITESPACE
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
%%

yyerror(s)
char *s;
{
	if ( parseflag) justshow();
	sprintf(errmsg,"parser error -- '%s'.\n   bad element was near: %s\n",
		s, wordbuf);
	do_panic();
}
#include "lex.yy.c"
