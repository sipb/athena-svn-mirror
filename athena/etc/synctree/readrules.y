%{
/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include <stdio.h>
#include "synctree.h"

extern rule rules[];
extern unsigned int lastrule;
int lineno;
char *yyinfilename;
static char *srcpath;
static char *dstpath;
%}
%union{
  char *s;
  char c;
  char **svec;
  enum action_type action_type;
  unsigned int bits;
  bool_exp bool_exp;
}
%token               MAP
%token               COPY LOCAL LINK IGNORE CHASE DELETE
%token               SET UNSET IF ELSE BEGIN_ END
%token               WHEN
%token               GE_END
%token <c>           ALPHANUM ACTIONOPT '/' '{' '}' ',' '?' '*' '[' ']' '!'
%token <s>           MAPDEST BOOLVAR CSH_CMD SH_CMD FILETYPES
%type  <s>           pathge srcpathge dstpathge
%type  <s>           ge ge1 ge2 chars
%type  <svec>        sh_cmds csh_cmds mapdests mapdests1
%type  <bits>        filetypes aopts ifhack elsehack
%type  <action_type> action
%type  <bool_exp>    boolexp boolexp1 boolexp0
%%

%{
#include "lex.yy.c"
%}

rulefile:   rules |
rules:      rules rule | rule 
rule:       mrule | arule | wrule | srule | irule | brule
crules:     crules crule | crule
crule:      mrule | arule | wrule | irule | brule

mrule: MAP srcpathge filetypes mapdests  { newrule();
                                           lstrule.type = R_MAP;
                                           lstrule.u.u_map.globexp = $2;
                                           lstrule.u.u_map.file_types = $3;
                                           lstrule.u.u_map.dests = $4; }
     | CHASE srcpathge filetypes aopts   { newrule();
                                           lstrule.type = R_CHASE;
                                           lstrule.u.u_chase.globexp = $2;
                                           /* filetypes and aopts ignored */
                                         }
arule: action dstpathge filetypes aopts  { newrule();
                                           lstrule.type = R_ACTION;
                                           lstrule.u.u_action.type = $1;
                                           lstrule.u.u_action.globexp = $2;
                                           lstrule.u.u_action.file_types = $3;
                                           lstrule.u.u_action.options = $4; }
wrule: WHEN dstpathge filetypes csh_cmds { newrule();
                                           lstrule.type = R_WHEN;
                                           lstrule.u.u_when.type = WHEN_CSH;
                                           lstrule.u.u_when.globexp = $2;
                                           lstrule.u.u_when.file_types = $3;
                                           lstrule.u.u_when.cmds = $4; }
     | WHEN dstpathge filetypes sh_cmds  { newrule();
                                           lstrule.type = R_WHEN;
                                           lstrule.u.u_when.type = WHEN_SH;
                                           lstrule.u.u_when.globexp = $2;
                                           lstrule.u.u_when.file_types = $3;
                                           lstrule.u.u_when.cmds = $4; }
srule:  SET BOOLVAR                      { setvar($2); sfree($2); }
     |  UNSET BOOLVAR                    { unsetvar($2); sfree($2); }
irule:  IF ifhack boolexp crule          { newrule();
                                           lstrule.type = R_IF;
                                           lstrule.u.u_if.boolexp = $3;
                                           lstrule.u.u_if.first = $2 + 1;
                                         }
     |  IF ifhack boolexp crule elsehack crule
                                         { newrule();
					   lstrule.type = R_IF_ELSE;
					   lstrule.u.u_if.boolexp = $3;
					   lstrule.u.u_if.first = $5;
					   rules[$5].u.u_skip.first = $2 + 1;
					 }
elsehack: ELSE                           { newrule();
					   lstrule.type = R_SKIP;
					   $$ = lastrule;
					 }
ifhack:                                  { $$ = lastrule; }
brule:  BEGIN_ crules END                { /* nothing to be done here! */ }

filetypes:                        { $$ = TYPE_ALL; }
         |  FILETYPES             { char *s = $1; $$ = 0;
                                    while (*s != '\0') switch (*(s++)) {
#define xxx(c1,c2,type) case c1: case c2: $$ |= type; break
                                      xxx('d','D',TYPE_D);
                                      xxx('c','C',TYPE_C);
                                      xxx('b','B',TYPE_B);
                                      xxx('l','L',TYPE_L);
                                      xxx('s','S',TYPE_S);
				      xxx('r','R',TYPE_R);
                                      xxx('x','X',TYPE_X);
#undef xxx    
                                    default:
                                      /* *** give some sort of error message */
                                      break;
                                    }
                                    sfree($1);
                                  }
    
action:    COPY                   { $$ = ACTION_COPY; }
      |    LOCAL                  { $$ = ACTION_LOCAL; }
      |    LINK                   { $$ = ACTION_LINK; }
      |    DELETE		  { $$ = ACTION_DELETE; }
      |    IGNORE                 { $$ = ACTION_IGNORE; }
    
aopts:                            { $$ = 0; }
     |     aopts ACTIONOPT        { $$ = $1; set_option($$,$2); }
    
    
csh_cmds:  csh_cmds CSH_CMD       { $$ = svecappend($1,$2); }
        |  CSH_CMD                { $$ = s2svec($1); }
    
sh_cmds:   sh_cmds SH_CMD         { $$ = svecappend($1,$2); }
       |   SH_CMD                 { $$ = s2svec($1); }
    

mapdests:   mapdests1             { $$ = $1; }
        |                         { $$ = (char **) malloc(sizeof(char *));
				    $$[0] = 0; }

mapdests1:  mapdests1 MAPDEST     { $$ = svecappend($1,$2); }
         |  MAPDEST               { $$ = s2svec($1); }

srcpathge: pathge               { $$ = concat(append(scopy(srcpath),'/'),$1); }
dstpathge: pathge               { $$ = concat(append(scopy(dstpath),'/'),$1); }

pathge:    ge GE_END              { $$ = $1; }

ge:       ALPHANUM ge         { $$ = concat(c2s($1),$2); }
  |       '?' ge 	      { $$ = concat(c2s($1),$2); }
  |       '*' ge 	      { $$ = concat(c2s($1),$2); }
  |       '{' ge1 '}' ge      { $$ = concat(c2s($1),concat(append($2,$3),$4));}
  |       '[' chars ']' ge    { $$ = concat(c2s($1),concat(append($2,$3),$4));}
  |       ALPHANUM            { $$ = c2s($1); }
  |       '?'		      { $$ = c2s($1); }
  |       '*'		      { $$ = c2s($1); }
  |       '{' ge1 '}'	      { $$ = append(concat(c2s($1),$2),$3); }
  |       '[' chars ']'	      { $$ = append(concat(c2s($1),$2),$3); }

ge1:       ge2 ',' ge1        { $$ = concat(append($1,$2),$3); }
   |       ge2                { $$ = $1; }

ge2:       ge                 { $$ = $1; }
   |                          { $$ = scopy(""); }
      
chars:     chars ALPHANUM     { $$ = append($1,$2); }
     |     ALPHANUM           { $$ = c2s($1); }
      
boolexp0:  '!' BOOLVAR           { $$ = bool_not(bool_var($2)); }
        |  BOOLVAR               { $$ = bool_var($1); }
	|  '!' '(' boolexp ')'   { $$ = bool_not($3); }
        |  '(' boolexp ')'       { $$ = $2; }

boolexp1:  boolexp1 '&' boolexp0 { $$ = bool_and($1,$3); }
	|  boolexp0		 { $$ = $1; }

boolexp:   boolexp  '|' boolexp1 { $$ = bool_or($1,$3); }
       |   boolexp1		 { $$ = $1; }

%%

/*
 * String manipulation routines 
 */

char *c2s(c)
     char c;
{ char *r;
  r = (char *) malloc(10);
  r[0] = c;
  r[1] = '\0';
  return r;
}

char *scopy(s)
     char *s;
{ char *r;
  r = (char *) malloc(strlen(s)+1);
  strcpy(r,s);
  return r;
}

char *append(s,c)
     char *s;
     char c;
{ int len = strlen(s);
  s = (char *) realloc(s,len+2);
  s[len] = c;
  s[len+1] = '\0';
  return s;
}

char *concat(s,s2)
     char *s;
     char *s2;
{ int len = strlen(s);
  int len2 = strlen(s2);
  s = (char *) realloc(s,len+len2+1);
  strcat(s,s2);
  free(s2);
  return s;
}

void sfree(s)
     char *s;
{
  free(s);
}

/*
 * Svec manipulation routines 
 */

char **s2svec(s)
     char *s;
{ char **r;
  r = (char **) malloc(2*sizeof(char *));
  r[0] = s;
  r[1] = 0;
  return r;
}

char **svecappend(v,s)
     char **v;
     char *s;
{ int len = 0;
  while (v[len] != 0) len++;
  v = (char **) realloc(v,(len+2)*sizeof(char *));
  v[len] = s;
  v[len+1] = 0;
  return v;
}

void svecfree(v)
     char **v;
{ int i;
  for (i=0; v[i] != 0; i++)
    free(v[i]);
  free(v);
}

/*
 *  Readrules
 */

int readrules(rfile,src,dst)
     char *rfile;
     char *src;
     char *dst;
{ FILE *rulefile;
  int status;
  extern int verbosef, nflag;

#ifdef YYDEBUG
  extern int yydebug;
  yydebug = (verbosef>3)? 1: 0;
#endif YYDEBUG  

  srcpath = src; dstpath = dst;

  if ((rulefile = fopen(rfile,"r")) == NULL) return 0;
  if (verbosef || nflag) printf("Parsing %s.\n",rfile);
  yysetin(rulefile);
  lineno = 0;
  yyinfilename = rfile;
  status = yyparse();
  fclose(rulefile);
  return status;
}

yyerror(s)
        char *s;
{
        printf("%s: %s near line %d\n", yyinfilename, s, lineno);
}
