A [^ \n\r\t\\;]
AO [a-zA-Z]
W [ \t]+
S .*
%{
#define countlines() {char *s=yytext;while(*s!='\0')if(*(s++)=='\n')lineno++;}
#define getstring() yylval.s=(char *)malloc(yyleng+1);strcpy(yylval.s,yytext)
#define getshcmd() { int i=0; \
		     while ( yytext[i] == ' ' || yytext[i] == '\t') i++; \
		     i++; /* over the % or $ */ \
		     if (yytext[i] == ' ') i++; \
		     yylval.s=(char *)malloc(yyleng+1-i); \
		     strcpy(yylval.s,&yytext[i]); \
		   }
#define rtn(st) { extern int verbosef; if (verbosef>3) printf("LEX returning st\n") ; return st; }

#define useonly(nn) { int i; for(i=yyleng-1;i>=(nn);i--) unput(yytext[i]); }

static int parenlevel;
%}
%Start MapGE MapTypes MapDests ActGE ActTypes ActOpts WhenGE WhenTypes IfExp IfPExp SetVar Rule
%e 2000
%p 5000
%n 2000
%k 2000
%a 30000
%o 30000
%%
\\.			    { yylval.c = yytext[1]; rtn(ALPHANUM); }
<MapGE>\n		    { BEGIN 0; countlines(); rtn(GE_END); }
<MapGE>{W}+		    { BEGIN MapDests; rtn(GE_END); }
<MapGE>";".*		    { useonly(1); BEGIN MapTypes; rtn(GE_END); }
<MapGE>{A}		    { yylval.c = yytext[0]; rtn(ALPHANUM); }
<MapGE>.		    { yylval.c = yytext[0]; rtn(yytext[0]); }
<MapTypes>{AO}+		    { getstring(); BEGIN MapDests; rtn(FILETYPES); }
<MapTypes>.		    { rtn(yytext[0]); /* parser error */ }
<MapDests>({A}|"$"|":")+    { getstring(); rtn(MAPDEST); }
<ActGE>\n		    { BEGIN 0; countlines(); rtn(GE_END); }
<ActGE>{W}+		    { BEGIN ActOpts; rtn(GE_END); }
<ActGE>";".*		    { useonly(1); BEGIN ActTypes; rtn(GE_END); }
<ActGE>{A}		    { yylval.c=yytext[0]; rtn(ALPHANUM); }
<ActGE>.		    { yylval.c=yytext[0]; rtn(yytext[0]);}
<ActTypes>{AO}+	    	    { getstring(); BEGIN ActOpts; rtn(FILETYPES);}
<ActTypes>.		    { rtn(yytext[0]); /* error */ }
<ActOpts>"-"{AO}	    { yylval.c = yytext[1];  rtn(ACTIONOPT); }
<WhenGE>\n		    { BEGIN 0; countlines(); rtn(GE_END); }
<WhenGE>{W}+		    { BEGIN 0; rtn(GE_END); }
<WhenGE>";".*		    { useonly(1); BEGIN WhenTypes; rtn(GE_END); }
<WhenGE>{A}		    { yylval.c=yytext[0]; rtn(ALPHANUM); }
<WhenGE>.		    { yylval.c=yytext[0]; rtn(yytext[0]); }
<WhenTypes>{AO}+	    { getstring(); BEGIN 0; rtn(FILETYPES); }
<WhenTypes>.		    { rtn(yytext[0]); /* error */ }
<SetVar>{A}+	    	    { getstring(); BEGIN Rule; rtn(BOOLVAR); }
<IfExp>{A}+	  	    { getstring(); BEGIN Rule; rtn(BOOLVAR); }
<IfExp>"("		    { BEGIN IfPExp; parenlevel = 1; rtn('('); /*)*/ }
<IfPExp>{A}+                { getstring(); rtn(BOOLVAR); }
<IfPExp>"("		    { parenlevel++; rtn('('); /*)*/ }
<IfPExp>")"		    { if(--parenlevel==0) BEGIN Rule; /*(*/ rtn(')'); }
{W}			    ;
<Rule>map{W}		    { BEGIN MapGE;    rtn(MAP);	   }
<Rule>copy{W}	    	    { BEGIN ActGE;    rtn(COPY);   }
<Rule>local{W} 	            { BEGIN ActGE;    rtn(LOCAL);  }
<Rule>link{W}	    	    { BEGIN ActGE;    rtn(LINK);   }
<Rule>delete{W}	    	    { BEGIN ActGE;    rtn(DELETE);   }
<Rule>ignore{W}		    { BEGIN ActGE;    rtn(IGNORE); }
<Rule>chase{W}	            { BEGIN ActGE;    rtn(CHASE);  }
<Rule>when{W}	    	    { BEGIN WhenGE;   rtn(WHEN);   }
<Rule>set{W}		    { BEGIN SetVar;   rtn(SET);	   }
<Rule>unset{W}		    { BEGIN SetVar;   rtn(UNSET);  }
<Rule>if{W}		    { BEGIN IfExp;    rtn(IF);	   }
<Rule>else                  { BEGIN 0;        rtn(ELSE);   }
<Rule>begin		    { BEGIN 0;        rtn(BEGIN_); }
<Rule>\n                    { BEGIN 0; countlines();	   }
^{W}?map{W}		    { BEGIN MapGE;    rtn(MAP);	   }
^{W}?copy{W}	    	    { BEGIN ActGE;    rtn(COPY);   }
^{W}?local{W} 	            { BEGIN ActGE;    rtn(LOCAL);  }
^{W}?link{W}	    	    { BEGIN ActGE;    rtn(LINK);   }
^{W}?delete{W}	    	    { BEGIN ActGE;    rtn(DELETE);   }
^{W}?ignore{W}		    { BEGIN ActGE;    rtn(IGNORE); }
^{W}?chase{W}	            { BEGIN ActGE;    rtn(CHASE);  }
^{W}?when{W}	    	    { BEGIN WhenGE;   rtn(WHEN);   }
^{W}?set{W}		    { BEGIN SetVar;   rtn(SET);	   }
^{W}?unset{W}		    { BEGIN SetVar;   rtn(UNSET);  }
^{W}?if{W}		    { BEGIN IfExp;    rtn(IF);	   }
^{W}?else                   { BEGIN 0;        rtn(ELSE);   }
^{W}?begin		    { rtn(BEGIN_);                 }
^{W}?end		    { rtn(END);                    }
^{W}?"%".*\n	            { getshcmd(); countlines(); rtn(CSH_CMD);}
^{W}?"$".*\n	            { getshcmd(); countlines(); rtn(SH_CMD);}
";".*			    ;
.			    { yylval.c = yytext[0]; return yytext[0]; }
\n			    { countlines(); BEGIN 0; }
%%
yysetin(i)
     FILE *i;
{ yyin = i; }
