# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin ={stdin}, *yyout ={stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
return ERROR_TABLE;
break;
case 2:
	return ERROR_TABLE;
break;
case 3:
return ERROR_CODE_ENTRY;
break;
case 4:
	return ERROR_CODE_ENTRY;
break;
case 5:
	return END;
break;
case 6:
	;
break;
case 7:
{ register char *p; yylval.dynstr = ds(yytext+1);
		  if (p=rindex(yylval.dynstr, '"')) *p='\0';
		  return QUOTED_STRING;
		}
break;
case 8:
{ yylval.dynstr = ds(yytext); return STRING; }
break;
case 9:
	;
break;
case 10:
	{ return (*yytext); }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] ={
0,

8,
0,

8,
0,

10,
0,

6,
10,
0,

6,
0,

10,
0,

10,
0,

8,
10,
0,

8,
10,
0,

7,
0,

9,
0,

8,
0,

4,
8,
0,

8,
0,

8,
0,

2,
8,
0,

5,
8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

8,
0,

3,
8,
0,

8,
0,

1,
8,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	1,3,	0,0,	
0,0,	6,10,	0,0,	7,12,	
0,0,	0,0,	1,4,	1,5,	
0,0,	6,10,	6,10,	7,12,	
7,13,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	1,6,	
1,7,	2,7,	6,11,	0,0,	
7,12,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,8,	0,0,	0,0,	
6,10,	0,0,	7,12,	0,0,	
0,0,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
0,0,	0,0,	1,9,	2,9,	
8,14,	22,23,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
8,14,	8,14,	8,14,	8,14,	
9,15,	16,19,	17,20,	20,21,	
21,22,	23,24,	24,26,	25,27,	
26,28,	27,29,	28,30,	9,16,	
29,31,	31,32,	0,0,	9,17,	
0,0,	9,18,	0,0,	0,0,	
0,0,	0,0,	23,25,	0,0,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+-1,	0,		yyvstop+1,
yycrank+-2,	yysvec+1,	yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+0,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+10,
yycrank+-4,	0,		yyvstop+12,
yycrank+-6,	0,		yyvstop+14,
yycrank+9,	0,		yyvstop+16,
yycrank+33,	yysvec+8,	yyvstop+19,
yycrank+0,	yysvec+6,	0,	
yycrank+0,	0,		yyvstop+22,
yycrank+0,	yysvec+7,	0,	
yycrank+0,	0,		yyvstop+24,
yycrank+0,	yysvec+8,	yyvstop+26,
yycrank+0,	yysvec+8,	yyvstop+28,
yycrank+33,	yysvec+8,	yyvstop+31,
yycrank+20,	yysvec+8,	yyvstop+33,
yycrank+0,	yysvec+8,	yyvstop+35,
yycrank+0,	yysvec+8,	yyvstop+38,
yycrank+24,	yysvec+8,	yyvstop+41,
yycrank+22,	yysvec+8,	yyvstop+43,
yycrank+10,	yysvec+8,	yyvstop+45,
yycrank+38,	yysvec+8,	yyvstop+47,
yycrank+27,	yysvec+8,	yyvstop+49,
yycrank+42,	yysvec+8,	yyvstop+51,
yycrank+40,	yysvec+8,	yyvstop+53,
yycrank+43,	yysvec+8,	yyvstop+55,
yycrank+41,	yysvec+8,	yyvstop+57,
yycrank+36,	yysvec+8,	yyvstop+59,
yycrank+0,	yysvec+8,	yyvstop+61,
yycrank+44,	yysvec+8,	yyvstop+64,
yycrank+0,	yysvec+8,	yyvstop+66,
0,	0,	0};
struct yywork *yytop = yycrank+154;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,01  ,01  ,01  ,01  ,'0' ,
01  ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	ncform	4.1	83/08/11	*/

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  && feof(yyin) )
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
