#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: strings.c,v 1.1.1.1 2001-02-19 07:11:41 ghudson Exp $";
#endif
/*----------------------------------------------------------------------

            T H E    P I N E    M A I L   S Y S T E M

   Laurence Lundblade and Mike Seibel
   Networks and Distributed Computing
   Computing and Communications
   University of Washington
   Administration Builiding, AG-44
   Seattle, Washington, 98195, USA
   Internet: lgl@CAC.Washington.EDU
             mikes@CAC.Washington.EDU

   Please address all bugs and comments to "pine-bugs@cac.washington.edu"


   Pine and Pico are registered trademarks of the University of Washington.
   No commercial use of these trademarks may be made without prior written
   permission of the University of Washington.

   Pine, Pico, and Pilot software and its included text are Copyright
   1989-2001 by the University of Washington.

   The full text of our legal notices is contained in the file called
   CPYRIGHT, included with this distribution.


   Pine is in part based on The Elm Mail System:
    ***********************************************************************
    *  The Elm Mail System  -  Revision: 2.13                             *
    *                                                                     *
    * 			Copyright (c) 1986, 1987 Dave Taylor              *
    * 			Copyright (c) 1988, 1989 USENET Community Trust   *
    ***********************************************************************
 

  ----------------------------------------------------------------------*/

/*======================================================================
    strings.c
    Misc extra and useful string functions
      - rplstr         replace a substring with another string
      - sqzspaces      Squeeze out the extra blanks in a string
      - sqznewlines    Squeeze out \n and \r.
      - removing_trailing_white_space 
      - short_str      Replace part of string with ... for display
      - removing_leading_white_space 
		       Remove leading or trailing white space
      - removing_double_quotes 
		       Remove surrounding double quotes
      - strclean       
                       both of above plus convert to lower case
      - skip_white_space
		       return pointer to first non-white-space char
      - skip_to_white_space
		       return pointer to first white-space char
      - srchstr        Search a string for first occurrence of a sub string
      - srchrstr       Search a string for last occurrence of a sub string
      - strindex       Replacement for strchr/index
      - strrindex      Replacement for strrchr/rindex
      - sstrcpy        Copy one string onto another, advancing dest'n pointer
      - istrncpy       Copy n chars between bufs, making ctrl chars harmless
      - month_abbrev   Return three letter abbreviations for months
      - month_num      Calculate month number from month/year string
      - cannon_date    Formalize format of a some what formatted date
      - pretty_command Return nice string describing character
      - repeat_char    Returns a string n chars long
      - comatose       Format number with nice commas
      - fold           Inserts newlines for folding at whitespace.
      - byte_string    Format number of bytes with Kb, Mb, Gb or bytes
      - enth-string    Format number i.e. 1: 1st, 983: 983rd....
      - string_to_cstring  Convert string to C-style constant string with \'s
      - cstring_to_hexstring  Convert cstring to hex string
      - cstring_to_string  Convert C-style string to string
      - add_backslash_escapes    Escape / and \ with \
      - remove_backslash_escapes Undo the \ escaping, and stop string at /.

 ====*/

#include "headers.h"

typedef struct role_args {
    char    *ourcharset;
    int      multi;
    char   **cset;
} ROLE_ARGS_T;

char       *add_escapes PROTO((char *, char *, int, char *, char *));
char       *add_pat_escapes PROTO((char *));
void        char_to_octal_triple PROTO((int, char *));
int         read_octal PROTO((char **));
char       *copy_quoted_string_asis PROTO((char *));
void        open_any_patterns PROTO((long));
void        sub_open_any_patterns PROTO((long));
int         sub_any_patterns PROTO((long, PAT_STATE *));
void        sub_close_patterns PROTO((long));
int         sub_write_patterns PROTO((long));
PAT_S      *first_any_pattern PROTO((PAT_STATE *));
PAT_S      *last_any_pattern PROTO((PAT_STATE *));
PAT_S      *prev_any_pattern PROTO((PAT_STATE *));
PAT_S      *next_any_pattern PROTO((PAT_STATE *));
int         write_pattern_file PROTO((char **, PAT_LINE_S *));
int         write_pattern_lit PROTO((char **, PAT_LINE_S *));
int         write_pattern_inherit PROTO((char **, PAT_LINE_S *));
char       *data_for_patline PROTO((PAT_S *));
PAT_LINE_S *parse_pat_lit PROTO((char *));
PAT_LINE_S *parse_pat_inherit PROTO((void));
PAT_S      *parse_pat PROTO((char *));
void        free_patline PROTO((PAT_LINE_S **));
void        free_patgrp PROTO((PATGRP_S **));
ARBHDR_S   *parse_arbhdr PROTO((char *));
void        free_arbhdr PROTO((ARBHDR_S **));
PAT_S      *copy_pat PROTO((PAT_S *));
PATGRP_S   *copy_patgrp PROTO((PATGRP_S *));
PATTERN_S  *copy_pattern PROTO((PATTERN_S *));
void        set_up_search_pgm PROTO((char *, PATTERN_S *, SEARCHPGM *,
				     ROLE_ARGS_T *));
void        add_type_to_pgm PROTO((char *, PATTERN_S *, SEARCHPGM *,
				   ROLE_ARGS_T *));
void        set_srch PROTO((char *, char *, SEARCHPGM *, ROLE_ARGS_T *));
void        set_srch_hdr PROTO((char *, char *, SEARCHPGM *, ROLE_ARGS_T *));
int	    non_eh PROTO((char *));
void        add_eh PROTO((char **, char **, char *, int *));
void        set_extra_hdrs PROTO((char *));
int         is_ascii_string PROTO((char *));
int	    rfc2369_parse PROTO((char *, RFC2369_S *));



/*
 * Useful def's to help with HEX string conversions
 */
#define	XDIGIT2C(C)	((C) - (isdigit((unsigned char) (C)) \
			  ? '0' : (isupper((unsigned char)(C))? '7' : 'W')))
#define	X2C(S)		((XDIGIT2C(*(S)) << 4) | XDIGIT2C(*((S)+1)))
#define	C2XPAIR(C, S)	{ \
			    *(S)++ = HEX_CHAR1(C); \
			    *(S)++ = HEX_CHAR2(C); \
			}





/*----------------------------------------------------------------------
       Replace n characters in one string with another given string

   args: os -- the output string
         dl -- the number of character to delete from start of os
         is -- The string to insert
  
 Result: returns pointer in originl string to end of string just inserted
         First 
  ---*/
char *
rplstr(os,dl,is)
char *os,*is;
int dl;
{   
    register char *x1,*x2,*x3;
    int           diff;

    if(os == NULL)
        return(NULL);
       
    for(x1 = os; *x1; x1++);
    if(dl > x1 - os)
        dl = x1 - os;
        
    x2 = is;      
    if(is != NULL){
        while(*x2++);
        x2--;
    }

    if((diff = (x2 - is) - dl) < 0){
        x3 = os; /* String shrinks */
        if(is != NULL)
            for(x2 = is; *x2; *x3++ = *x2++); /* copy new string in */
        for(x2 = x3 - diff; *x2; *x3++ = *x2++); /* shift for delete */
        *x3 = *x2;
    } else {                
        /* String grows */
        for(x3 = x1 + diff; x3 >= os + (x2 - is); *x3-- = *x1--); /* shift*/
        for(x1 = os, x2 = is; *x2 ; *x1++ = *x2++);
        while(*x3) x3++;                 
    }
    return(x3);
}



/*----------------------------------------------------------------------
     Squeeze out blanks 
  ----------------------------------------------------------------------*/
void
sqzspaces(string)
     char *string;
{
    char *p = string;

    while(*string = *p++)		   /* while something to copy       */
      if(!isspace((unsigned char)*string)) /* only really copy if non-blank */
	string++;
}



/*----------------------------------------------------------------------
     Squeeze out CR's and LF's 
  ----------------------------------------------------------------------*/
void
sqznewlines(string)
    char *string;
{
    char *p = string;

    while(*string = *p++)		      /* while something to copy  */
      if(*string != '\r' && *string != '\n')  /* only copy if non-newline */
	string++;
}



/*----------------------------------------------------------------------  
       Remove leading white space from a string in place
  
  Args: string -- string to remove space from
  ----*/
void
removing_leading_white_space(string)
     char *string;
{
    register char *p;

    if(!string)
      return;

    for(p = string; *p; p++)		/* find the first non-blank  */
      if(!isspace((unsigned char) *p)){
	  while(*string++ = *p++)	/* copy back from there... */
	    ;

	  return;
      }
}



/*----------------------------------------------------------------------  
       Remove trailing white space from a string in place
  
  Args: string -- string to remove space from
  ----*/
void
removing_trailing_white_space(string)
    char *string;
{
    char *p = NULL;

    if(!string)
      return;

    for(; *string; string++)		/* remember start of whitespace */
      p = (!isspace((unsigned char)*string)) ? NULL : (!p) ? string : p;

    if(p)				/* if whitespace, blast it */
      *p = '\0';
}


void
removing_leading_and_trailing_white_space(string)
     char *string;
{
    register char *p, *q = NULL;

    if(!string)
      return;

    for(p = string; *p; p++)		/* find the first non-blank  */
      if(!isspace((unsigned char)*p)){
	  while(*string = *p++){	/* copy back from there... */
	      q = (!isspace((unsigned char)*string)) ? NULL : (!q) ? string : q;
	      string++;
	  }

	  if(q)
	    *q = '\0';
	    
	  return;
      }

    if(*string != '\0')
      *string = '\0';
}


/*----------------------------------------------------------------------  
       Remove one set of double quotes surrounding string in place
       Returns 1 if quotes were removed
  
  Args: string -- string to remove quotes from
  ----*/
int
removing_double_quotes(string)
     char *string;
{
    register char *p;
    int ret = 0;

    if(string && string[0] == '"' && string[1] != '\0'){
	p = string + strlen(string) - 1;
	if(*p == '"'){
	    ret++;
	    *p = '\0';
	    for(p = string; *p; p++) 
	      *p = *(p+1);
	}
    }

    return(ret);
}



/*----------------------------------------------------------------------  
  return a pointer to first non-whitespace char in string
  
  Args: string -- string to scan
  ----*/
char *
skip_white_space(string)
     char *string;
{
    while(*string && isspace((unsigned char) *string))
      string++;

    return(string);
}



/*----------------------------------------------------------------------  
  return a pointer to first whitespace char in string
  
  Args: string -- string to scan
  ----*/
char *
skip_to_white_space(string)
     char *string;
{
    while(*string && !isspace((unsigned char) *string))
      string++;

    return(string);
}



/*----------------------------------------------------------------------  
       Remove quotes from a string in place
  
  Args: string -- string to remove quotes from
  Rreturns: string passed us, but with quotes gone
  ----*/
char *
removing_quotes(string)
    char *string;
{
    register char *p, *q;

    if(*(p = q = string) == '\"'){
	do
	  if(*q == '\"' || *q == '\\')
	    q++;
	while(*p++ = *q++);
    }

    return(string);
}



/*---------------------------------------------------
     Remove leading whitespace, trailing whitespace and convert 
     to lowercase

   Args: s, -- The string to clean

 Result: the cleaned string
  ----*/
char *
strclean(string)
     char *string;
{
    char *s = string, *sc = NULL, *p = NULL;

    for(; *s; s++){				/* single pass */
	if(!isspace((unsigned char)*s)){
	    p = NULL;				/* not start of blanks   */
	    if(!sc)				/* first non-blank? */
	      sc = string;			/* start copying */
	}
	else if(!p)				/* it's OK if sc == NULL */
	  p = sc;				/* start of blanks? */

	if(sc)					/* if copying, copy */
	  *sc++ = isupper((unsigned char)(*s))
			  ? (unsigned char)tolower((unsigned char)(*s))
			  : (unsigned char)(*s);
    }

    if(p)					/* if ending blanks  */
      *p = '\0';				/* tie off beginning */
    else if(!sc)				/* never saw a non-blank */
      *string = '\0';				/* so tie whole thing off */

    return(string);
}


/*
 * Returns a pointer to a short version of the string.
 * If src is not longer than len, pointer points to src.
 * If longer than len, a version which is len long is made in
 * buf and the pointer points there.
 *
 * Args  src -- The string to be shortened
 *       buf -- A place to put the short version, length should be >= len+1
 *       len -- Desired length of shortened string
 *     where -- Where should the dots be in the shortened string. Can be
 *              FrontDots, MidDots, EndDots.
 *
 *     FrontDots           ...stuvwxyz
 *     EndDots             abcdefgh...
 *     MidDots             abcd...wxyz
 */
char *
short_str(src, buf, len, where)
    char     *src;
    char     *buf;
    int       len;
    WhereDots where;
{
    char *ans;
    int   alen, first, second;

    if(len <= 0)
      ans = "";
    else if((alen = strlen(src)) <= len)
      ans = src;
    else{
	ans = buf;
	if(len < 5){
	    strncpy(buf, "....", len);
	    buf[len] = '\0';
	}
	else{
	    /*
	     * first == length of preellipsis text
	     * second == length of postellipsis text
	     */
	    if(where == FrontDots){
		first = 0;
		second = len - 3;
	    }
	    else if(where == MidDots){
		first = (len - 3)/2;
		second = len - 3 - first;
	    }
	    else if(where == EndDots){
		first = len - 3;
		second = 0;
	    }

	    if(first)
	      strncpy(buf, src, first);

	    strcpy(buf+first, "...");
	    if(second)
	      strncpy(buf+first+3, src+alen-second, second);

	    buf[len] = '\0';
	}
    }
    
    return(ans);
}



/*----------------------------------------------------------------------
        Search one string for another

   Args:  is -- The string to search in, the larger string
          ss -- The string to search for, the smaller string

   Search for first occurrence of ss in the is, and return a pointer
   into the string is when it is found. The search is case indepedent.
  ----*/
char *	    
srchstr(is, ss)
    char *is, *ss;
{
    register char *p, *q;

    if(ss && is)
      for(; *is; is++)
	for(p = ss, q = is; ; p++, q++){
	    if(!*p)
	      return(is);			/* winner! */
	    else if(!*q)
	      return(NULL);			/* len(ss) > len(is)! */
	    else if(*p != *q && !CMPNOCASE(*p, *q))
	      break;
	}

    return(NULL);
}



/*----------------------------------------------------------------------
        Search one string for another, from right

   Args:  is -- The string to search in, the larger string
          ss -- The string to search for, the smaller string

   Search for last occurrence of ss in the is, and return a pointer
   into the string is when it is found. The search is case indepedent.
  ----*/

char *	    
srchrstr(is, ss)
register char *is, *ss;
{                    
    register char *sx, *sy;
    char          *ss_store, *rv;
    char          *begin_is;
    char           temp[251];
    
    if(is == NULL || ss == NULL)
      return(NULL);

    if(strlen(ss) > sizeof(temp) - 2)
      ss_store = (char *)fs_get(strlen(ss) + 1);
    else
      ss_store = temp;

    for(sx = ss, sy = ss_store; *sx != '\0' ; sx++, sy++)
      *sy = isupper((unsigned char)(*sx))
		      ? (unsigned char)tolower((unsigned char)(*sx))
		      : (unsigned char)(*sx);
    *sy = *sx;

    begin_is = is;
    is = is + strlen(is) - strlen(ss_store);
    rv = NULL;
    while(is >= begin_is){
        for(sx = is, sy = ss_store;
	    ((*sx == *sy)
	      || ((isupper((unsigned char)(*sx))
		     ? (unsigned char)tolower((unsigned char)(*sx))
		     : (unsigned char)(*sx)) == (unsigned char)(*sy))) && *sy;
	    sx++, sy++)
	   ;

        if(!*sy){
            rv = is;
            break;
        }

        is--;
    }

    if(ss_store != temp)
      fs_give((void **)&ss_store);

    return(rv);
}



/*----------------------------------------------------------------------
    A replacement for strchr or index ...

    Returns a pointer to the first occurrence of the character
    'ch' in the specified string or NULL if it doesn't occur

 ....so we don't have to worry if it's there or not. We bring our own.
If we really care about efficiency and think the local one is more
efficient the local one can be used, but most of the things that take
a long time are in the c-client and not in pine.
 ----*/
char *
strindex(buffer, ch)
    char *buffer;
    int ch;
{
    do
      if(*buffer == ch)
	return(buffer);
    while (*buffer++ != '\0');

    return(NULL);
}


/* Returns a pointer to the last occurrence of the character
 * 'ch' in the specified string or NULL if it doesn't occur
 */
char *
strrindex(buffer, ch)
    char *buffer;
    int   ch;
{
    char *address = NULL;

    do
      if(*buffer == ch)
	address = buffer;
    while (*buffer++ != '\0');
    return(address);
}



/*----------------------------------------------------------------------
  copy the source string onto the destination string returning with
  the destination string pointer at the end of the destination text

  motivation for this is to avoid twice passing over a string that's
  being appended to twice (i.e., strcpy(t, x); t += strlen(t))
 ----*/
void
sstrcpy(d, s)
    char **d;
    char *s;
{
    while((**d = *s++) != '\0')
      (*d)++;
}


void
sstrncpy(d, s, n)
    char **d;
    char *s;
    int n;
{
    while(n-- > 0 && (**d = *s++) != '\0')
      (*d)++;
}


/*----------------------------------------------------------------------
  copy at most n chars of the source string onto the destination string
  returning pointer to start of destination and converting any undisplayable
  characters to harmless character equivalents.
 ----*/
char *
istrncpy(d, s, n)
    char *d, *s;
    int n;
{
    char *rv = d;

    do
      if(F_OFF(F_PASS_CONTROL_CHARS, ps_global) && *s && CAN_DISPLAY(*s))
	if(n-- > 0){
	    *d++ = '^';

	    if(n-- > 0)
	      *d++ = *s++ + '@';
	}
    while(n-- > 0 && (*d++ = *s++));

    return(rv);
}



char *xdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL};

char *
month_abbrev(month_num)
     int month_num;
{
    static char *xmonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
    if(month_num < 1 || month_num > 12)
      return("xxx");
    return(xmonths[month_num - 1]);
}

char *
month_name(month_num)
     int month_num;
{
    static char *months[] = {"January", "February", "March", "April",
		"May", "June", "July", "August", "September", "October",
		"November", "December", NULL};
    if(month_num < 1 || month_num > 12)
      return("");
    return(months[month_num - 1]);
}

char *
week_abbrev(week_day)
     int week_day;
{
    if(week_day < 0 || week_day > 6)
      return("???");
    return(xdays[week_day]);
}


/*----------------------------------------------------------------------
      Return month number of month named in string
  
   Args: s -- string with 3 letter month abbreviation of form mmm-yyyy
 
 Result: Returns month number with January, year 1900, 2000... being 0;
         -1 if no month/year is matched
 ----*/
int
month_num(s)
     char *s;
{
    int month, year;
    int i;

    for(i = 0; i < 12; i++){
        if(struncmp(month_abbrev(i+1), s, 3) == 0)
          break;
    }
    if(i == 12)
      return(-1);

    year = atoi(s + 4);
    if(year == 0)
      return(-1);

    month = year * 12 + i;
    return(month);
}


/*
 * Structure containing all knowledge of symbolic time zones.
 * To add support for a given time zone, add it here, but make sure
 * the zone name is in upper case.
 */
static struct {
    char  *zone;
    short  len,
    	   hour_offset,
	   min_offset;
} known_zones[] = {
    {"PST", 3, -8, 0},			/* Pacific Standard */
    {"PDT", 3, -7, 0},			/* Pacific Daylight */
    {"MST", 3, -7, 0},			/* Mountain Standard */
    {"MDT", 3, -6, 0},			/* Mountain Daylight */
    {"CST", 3, -6, 0},			/* Central Standard */
    {"CDT", 3, -5, 0},			/* Central Daylight */
    {"EST", 3, -5, 0},			/* Eastern Standard */
    {"EDT", 3, -4, 0},			/* Eastern Daylight */
    {"JST", 3,  9, 0},			/* Japan Standard */
    {"GMT", 3,  0, 0},			/* Universal Time */
    {"UT",  2,  0, 0},			/* Universal Time */
#ifdef	IST_MEANS_ISREAL
    {"IST", 3,  2, 0},			/* Israel Standard */
#else
#ifdef	IST_MEANS_INDIA
    {"IST", 3,  5, 30},			/* India Standard */
#endif
#endif
    {NULL, 0, 0},
};

/*----------------------------------------------------------------------
  Parse date in or near RFC-822 format into the date structure

Args: given_date -- The input string to parse
      d          -- Pointer to a struct date to place the result in
 
Returns nothing

The following date fomrats are accepted:
  WKDAY DD MM YY HH:MM:SS ZZ
  DD MM YY HH:MM:SS ZZ
  WKDAY DD MM HH:MM:SS YY ZZ
  DD MM HH:MM:SS YY ZZ
  DD MM WKDAY HH:MM:SS YY ZZ
  DD MM WKDAY YY MM HH:MM:SS ZZ

All leading, intervening and trailing spaces tabs and commas are ignored.
The prefered formats are the first or second ones.  If a field is unparsable
it's value is left as -1. 

  ----*/
void
parse_date(given_date, d)
     char        *given_date;
     struct date *d;
{
    char *p, **i, *q, n;
    int   month;

    d->sec   = -1;
    d->minute= -1;
    d->hour  = -1;
    d->day   = -1;
    d->month = -1;
    d->year  = -1;
    d->wkday = -1;
    d->hours_off_gmt = -1;
    d->min_off_gmt   = -1;

    if(given_date == NULL)
      return;

    p = given_date;
    while(*p && isspace((unsigned char)*p))
      p++;

    /* Start with month, weekday or day ? */
    for(i = xdays; *i != NULL; i++) 
      if(struncmp(p, *i, 3) == 0) /* Match first 3 letters */
        break;
    if(*i != NULL) {
        /* Started with week day */
        d->wkday = i - xdays;
        while(*p && !isspace((unsigned char)*p) && *p != ',')
          p++;
        while(*p && (isspace((unsigned char)*p) || *p == ','))
          p++;
    }
    if(isdigit((unsigned char)*p)) {
        d->day = atoi(p);
        while(*p && isdigit((unsigned char)*p))
          p++;
        while(*p && (*p == '-' || *p == ',' || isspace((unsigned char)*p)))
          p++;
    }
    for(month = 1; month <= 12; month++)
      if(struncmp(p, month_abbrev(month), 3) == 0)
        break;
    if(month < 13) {
        d->month = month;

    } 
    /* Move over month, (or whatever is there) */
    while(*p && !isspace((unsigned char)*p) && *p != ',' && *p != '-')
       p++;
    while(*p && (isspace((unsigned char)*p) || *p == ',' || *p == '-'))
       p++;

    /* Check again for day */
    if(isdigit((unsigned char)*p) && d->day == -1) {
        d->day = atoi(p);
        while(*p && isdigit((unsigned char)*p))
          p++;
        while(*p && (*p == '-' || *p == ',' || isspace((unsigned char)*p)))
          p++;
    }

    /*-- Check for time --*/
    for(q = p; *q && isdigit((unsigned char)*q); q++);
    if(*q == ':') {
        /* It's the time (out of place) */
        d->hour = atoi(p);
        while(*p && *p != ':' && !isspace((unsigned char)*p))
          p++;
        if(*p == ':') {
            p++;
            d->minute = atoi(p);
            while(*p && *p != ':' && !isspace((unsigned char)*p))
              p++;
            if(*p == ':') {
                d->sec = atoi(p);
                while(*p && !isspace((unsigned char)*p))
                  p++;
            }
        }
        while(*p && isspace((unsigned char)*p))
          p++;
    }
    

    /* Get the year 0-49 is 2000-2049; 50-100 is 1950-1999 and
                                           101-9999 is 101-9999 */
    if(isdigit((unsigned char)*p)) {
        d->year = atoi(p);
        if(d->year < 50)   
          d->year += 2000;
        else if(d->year < 100)
          d->year += 1900;
        while(*p && isdigit((unsigned char)*p))
          p++;
        while(*p && (*p == '-' || *p == ',' || isspace((unsigned char)*p)))
          p++;
    } else {
        /* Something wierd, skip it and try to resynch */
        while(*p && !isspace((unsigned char)*p) && *p != ',' && *p != '-')
          p++;
        while(*p && (isspace((unsigned char)*p) || *p == ',' || *p == '-'))
          p++;
    }

    /*-- Now get hours minutes, seconds and ignore tenths --*/
    for(q = p; *q && isdigit((unsigned char)*q); q++);
    if(*q == ':' && d->hour == -1) {
        d->hour = atoi(p);
        while(*p && *p != ':' && !isspace((unsigned char)*p))
          p++;
        if(*p == ':') {
            p++;
            d->minute = atoi(p);
            while(*p && *p != ':' && !isspace((unsigned char)*p))
              p++;
            if(*p == ':') {
                p++;
                d->sec = atoi(p);
                while(*p && !isspace((unsigned char)*p))
                  p++;
            }
        }
    }
    while(*p && isspace((unsigned char)*p))
      p++;


    /*-- The time zone --*/
    d->hours_off_gmt = 0;
    d->min_off_gmt = 0;
    if(*p) {
        if((*p == '+' || *p == '-')
	   && isdigit((unsigned char)p[1])
	   && isdigit((unsigned char)p[2])
	   && isdigit((unsigned char)p[3])
	   && isdigit((unsigned char)p[4])
	   && !isdigit((unsigned char)p[5])) {
            char tmp[3];
            d->min_off_gmt = d->hours_off_gmt = (*p == '+' ? 1 : -1);
            p++;
            tmp[0] = *p++;
            tmp[1] = *p++;
            tmp[2] = '\0';
            d->hours_off_gmt *= atoi(tmp);
            tmp[0] = *p++;
            tmp[1] = *p++;
            tmp[2] = '\0';
            d->min_off_gmt *= atoi(tmp);
        } else {
	    for(n = 0; known_zones[n].zone; n++)
	      if(struncmp(p, known_zones[n].zone, known_zones[n].len) == 0){
		  d->hours_off_gmt = (int) known_zones[n].hour_offset;
		  d->min_off_gmt   = (int) known_zones[n].min_offset;
		  break;
	      }
        }
    }
    dprint(9, (debugfile,
	 "Parse date: \"%s\" to..  hours_off_gmt:%d  min_off_gmt:%d\n",
               given_date, d->hours_off_gmt, d->min_off_gmt));
    dprint(9, (debugfile,
	       "Parse date: wkday:%d  month:%d  year:%d  day:%d  hour:%d  min:%d  sec:%d\n",
            d->wkday, d->month, d->year, d->day, d->hour, d->minute, d->sec));
}



/*----------------------------------------------------------------------
     Map some of the special characters into sensible strings for human
   consumption.
  ----*/
char *
pretty_command(c)
     int c;
{
    static char  buf[10];
    char	*s;

    switch(c){
      case '\033'    : s = "ESC";		break;
      case '\177'    : s = "DEL";		break;
      case ctrl('I') : s = "TAB";		break;
      case ctrl('J') : s = "LINEFEED";		break;
      case ctrl('M') : s = "RETURN";		break;
      case ctrl('Q') : s = "XON";		break;
      case ctrl('S') : s = "XOFF";		break;
      case KEY_UP    : s = "Up Arrow";		break;
      case KEY_DOWN  : s = "Down Arrow";	break;
      case KEY_RIGHT : s = "Right Arrow";	break;
      case KEY_LEFT  : s = "Left Arrow";	break;
      case KEY_PGUP  : s = "Prev Page";		break;
      case KEY_PGDN  : s = "Next Page";		break;
      case KEY_HOME  : s = "Home";		break;
      case KEY_END   : s = "End";		break;
      case KEY_DEL   : s = "Delete";		break; /* Not necessary DEL! */
      case PF1	     :
      case PF2	     :
      case PF3	     :
      case PF4	     :
      case PF5	     :
      case PF6	     :
      case PF7	     :
      case PF8	     :
      case PF9	     :
      case PF10	     :
      case PF11	     :
      case PF12	     :
        sprintf(s = buf, "F%d", c - PF1 + 1);
	break;

      default:
	if(c < ' ')
	  sprintf(s = buf, "^%c", c + 'A' - 1);
	else
	  sprintf(s = buf, "%c", c);

	break;
    }

    return(s);
}
        
    

/*----------------------------------------------------------------------
     Create a little string of blanks of the specified length.
   Max n is 511.
  ----*/
char *
repeat_char(n, c)
     int  n;
     int  c;
{
    static char bb[512];
    if(n > sizeof(bb))
       n = sizeof(bb) - 1;
    bb[n--] = '\0';
    while(n >= 0)
      bb[n--] = c;
    return(bb);
}


/*----------------------------------------------------------------------
        Turn a number into a string with comma's

   Args: number -- The long to be turned into a string. 

  Result: pointer to static string representing number with commas
  ---*/
char *
comatose(number) 
    long number;
{
#ifdef	DOS
    static char buf[3][16];
    static int whichbuf = 0;
    char *b;
    short i;

    if(!number)
	return("0");

    whichbuf = (whichbuf + 1) % 3;

    if(number < 0x7FFFFFFFL){		/* largest DOS signed long */
        buf[whichbuf][15] = '\0';
        b = &buf[whichbuf][14];
        i = 2;
	while(number){
 	    *b-- = (number%10) + '0';
	    if((number /= 10) && i-- == 0 ){
		*b-- = ',';
		i = 2;
	    }
	}
    }
    else
      return("Number too big!");		/* just fits! */

    return(++b);
#else
    long        i, x, done_one;
    static char buf[3][50];
    static int whichbuf = 0;
    char       *b;

    whichbuf = (whichbuf + 1) % 3;
    dprint(9, (debugfile, "comatose(%ld) returns:", number));
    if(number == 0){
        strcpy(buf[whichbuf], "0");
        return(buf[whichbuf]);
    }
    
    done_one = 0;
    b = buf[whichbuf];
    for(i = 1000000000; i >= 1; i /= 1000) {
	x = number / i;
	number = number % i;
	if(x != 0 || done_one) {
	    if(b != buf[whichbuf])
	      *b++ = ',';
	    sprintf(b, done_one ? "%03ld" : "%d", x);
	    b += strlen(b);
	    done_one = 1;
	}
    }
    *b = '\0';

    dprint(9, (debugfile, "\"%s\"\n", buf[whichbuf]));

    return(buf[whichbuf]);
#endif	/* DOS */
}



/*----------------------------------------------------------------------
   Format number as amount of bytes, appending Kb, Mb, Gb, bytes

  Args: bytes -- number of bytes to format

 Returns pointer to static string. The numbers are divided to produce a 
nice string with precision of about 2-4 digits
    ----*/
char *
byte_string(bytes)
     long bytes;
{
    char       *a, aa[5];
    char       *abbrevs = "GMK";
    long        i, ones, tenths;
    static char string[10];

    ones   = 0L;
    tenths = 0L;

    if(bytes == 0L){
        strcpy(string, "0 bytes");
    } else {
        for(a = abbrevs, i = 1000000000; i >= 1; i /= 1000, a++) {
            if(bytes > i) {
                ones = bytes/i;
                if(ones < 10L && i > 10L)
                  tenths = (bytes - (ones * i)) / (i / 10L);
                break;
            }
        }
    
        aa[0] = *a;  aa[1] = '\0'; 
    
        if(tenths == 0)
          sprintf(string, "%ld%s%s", ones, aa, *a ? "B" : "bytes");
        else
          sprintf(string, "%ld.%ld%s%s", ones, tenths, aa, *a ? "B" : "bytes");
    }

    return(string);
}



/*----------------------------------------------------------------------
    Print a string corresponding to the number given:
      1st, 2nd, 3rd, 105th, 92342nd....
 ----*/

char *
enth_string(i)
     int i;
{
    static char enth[10];

    switch (i % 10) {
        
      case 1:
        if( (i % 100 ) == 11)
          sprintf(enth,"%dth", i);
        else
          sprintf(enth,"%dst", i);
        break;

      case 2:
        if ((i % 100) == 12)
          sprintf(enth, "%dth",i);
        else
          sprintf(enth, "%dnd",i);
        break;

      case 3:
        if(( i % 100) == 13)
          sprintf(enth, "%dth",i);
        else
          sprintf(enth, "%drd",i);
        break;

      default:
        sprintf(enth,"%dth",i);
        break;
    }
    return(enth);
}


/*
 * Inserts newlines for folding at whitespace.
 *
 * Args          src -- The source text.
 *             width -- Approximately where the fold should happen.
 *          maxwidth -- Maximum width we want to fold at.
 *                cr -- End of line is \r\n instead of just \n.
 *       preserve_ws -- Preserve whitespace when folding. This is for vcard
 *                       folding where CRLF SPACE is removed when unfolding, so
 *                       we need to leave the space in. With rfc822 unfolding
 *                       only the CRLF is removed when unfolding.
 *      first_indent -- String to use as indent on first line.
 *            indent -- String to use as indent for subsequent folded lines.
 *
 * Returns   An allocated string which caller should free.
 */
char *
fold(src, width, maxwidth, cr, preserve_ws, first_indent, indent)
    char *src;
    int   width,
	  maxwidth,
	  cr,
	  preserve_ws;
    char *first_indent,
	 *indent;
{
    char *next_piece, *res, *p;
    int   i, len, nb, starting_point, shorter, longer, winner, eol;
    int   indent1 = 0, indent2 = 0;
    char  save_char;

    if(indent)
      indent2 = strlen(indent);

    if(first_indent)
      indent1 = strlen(first_indent);

    nb = indent1;
    len = indent1;
    next_piece = src;
    eol = cr ? 2 : 1;
    if(!src || !*src)
      nb += eol;

    /*
     * We can't tell how much space is going to be needed without actually
     * passing through the data to see.
     */
    while(next_piece && *next_piece){
	if(next_piece != src && indent2){
	    len += indent2;
	    nb += indent2;
	}

	if(strlen(next_piece) + len <= width){
	    nb += (strlen(next_piece) + eol);
	    break;
	}
	else{ /* fold it */
	    starting_point = width - len;	/* space left on this line */
	    /* find a good folding spot */
	    winner = -1;
	    for(i = 0;
		winner == -1
		  && (starting_point - i > len + 8
		      || starting_point + i < maxwidth - width);
		i++){

		if((shorter=starting_point-i) > len + 8
		   && isspace((unsigned char)next_piece[shorter]))
		  winner = shorter;
		else if((longer=starting_point+i) < maxwidth - width
			&& (!next_piece[longer]
		            || isspace((unsigned char)next_piece[longer])))
		  winner = longer;
	    }

	    if(winner == -1) /* if no good folding spot, fold at width */
	      winner = starting_point;
	    
	    nb += (winner + eol);
	    next_piece += winner;
	    if(!preserve_ws && isspace((unsigned char)next_piece[0]))
	      next_piece++;
	}

	len = 0;
    }

    res = (char *)fs_get((nb+1) * sizeof(char));
    p = res;
    sstrcpy(&p, first_indent);
    len = indent1;
    next_piece = src;

    while(next_piece && *next_piece){
	if(next_piece != src && indent2){
	    sstrcpy(&p, indent);
	    len += indent2;
	}

	if(strlen(next_piece) + len <= width){
	    sstrcpy(&p, next_piece);
	    if(cr)
	      *p++ = '\r';

	    *p++ = '\n';
	    break;
	}
	else{ /* fold it */
	    starting_point = width - len;	/* space left on this line */
	    /* find a good folding spot */
	    winner = -1;
	    for(i = 0;
		winner == -1
		  && (starting_point - i > len + 8
		      || starting_point + i < maxwidth - width);
		i++){

		if((shorter=starting_point-i) > len + 8
		   && isspace((unsigned char)next_piece[shorter]))
		  winner = shorter;
		else if((longer=starting_point+i) < maxwidth - width
			&& (!next_piece[longer]
		            || isspace((unsigned char)next_piece[longer])))
		  winner = longer;
	    }

	    if(winner == -1) /* if no good folding spot, fold at width */
	      winner = starting_point;
	    
	    save_char = next_piece[winner];
	    next_piece[winner] = '\0';
	    sstrcpy(&p, next_piece);
	    if(cr)
	      *p++ = '\r';

	    *p++ = '\n';
	    next_piece[winner] = save_char;
	    next_piece += winner;
	    if(!preserve_ws && isspace((unsigned char)next_piece[0]))
	      next_piece++;
	}

	len = 0;
    }

    if(!src || !*src){
	if(cr)
	  *p++ = '\r';

	*p++ = '\n';
    }

    *p = '\0';

    return(res);
}


/*
 * strsquish - fancifies a string into the given buffer if it's too
 *	       long to fit in the given width
 */
char *
strsquish(buf, s, width)
    char *buf, *s;
    int   width;
{
    int i, offset;

    if(width > 0){
	if((i = strlen(s)) <= width)
	  return(s);

	if(width > 14){
	    strncpy(buf, s, offset = ((width / 2) - 2));
	    strcpy(buf + offset, "...");
	    offset += 3;
	}
	else if(width > 3){
	    strcpy(buf, "...");
	    offset = 3;
	}
	else
	  offset = 0;

	strcpy(buf + offset, s + (i - width) + offset);
	return(buf);
    }
    else
      return("");
}


char *
long2string(l)
     long l;
{
    static char string[20];
    sprintf(string, "%ld", l);
    return(string);
}

char *
int2string(i)
     int i;
{
    static char string[20];
    sprintf(string, "%d", i);
    return(string);
}


/*
 * strtoval - convert the given string to a positive integer.
 */
char *
strtoval(s, val, minmum, maxmum, otherok, errbuf, varname)
    char *s;
    int  *val;
    int   minmum, maxmum, otherok;
    char *errbuf, *varname;
{
    int   i = 0, neg = 1;
    char *p = s, *errstr = NULL;

    removing_leading_and_trailing_white_space(p);
    for(; *p; p++)
      if(isdigit((unsigned char)*p)){
	  i = (i * 10) + (*p - '0');
      }
      else if(*p == '-' && i == 0){
	  neg = -1;
      }
      else{
	  sprintf(errstr = errbuf,
		  "Non-numeric value ('%c' in \"%.8s\") in %s. Using \"%d\"",
		  *p, s, varname, *val);
	  return(errbuf);
      }

    i *= neg;

    /* range describes acceptable values */
    if(maxmum > minmum && (i < minmum || i > maxmum) && i != otherok)
      sprintf(errstr = errbuf,
	      "%s of %d not supported (M%s %d). Using \"%d\"",
	      varname, i, (i > maxmum) ? "ax" : "in",
	      (i > maxmum) ? maxmum : minmum, *val);
    /* range describes unacceptable values */
    else if(minmum > maxmum && !(i < maxmum || i > minmum))
      sprintf(errstr = errbuf, "%s of %d not supported. Using \"%d\"",
	      varname, i, *val);
    else
      *val = i;

    return(errstr);
}


/*
 *  Function to parse the given string into two space-delimited fields
 *  Quotes may be used to surround labels or values with spaces in them.
 *  Backslash negates the special meaning of a quote.
 *  Unescaping of backslashes only happens if the pair member is quoted,
 *    this provides for backwards compatibility.
 *
 * Args -- string -- the source string
 *          label -- the first half of the string, a return value
 *          value -- the last half of the string, a return value
 *        firstws -- if set, the halves are delimited by the first unquoted
 *                    whitespace, else by the last unquoted whitespace
 *   strip_internal_label_quotes -- unescaped quotes in the middle of the label
 *                                   are removed. This is useful for vars
 *                                   like display-filters and url-viewers
 *                                   which may require quoting of an arg
 *                                   inside of a _TOKEN_.
 */
void
get_pair(string, label, value, firstws, strip_internal_label_quotes)
    char *string, **label, **value;
    int   firstws;
    int   strip_internal_label_quotes;
{
    char *p, *q, *tmp, *token = NULL;
    int	  quoted = 0;

    *label = *value = NULL;

    /*
     * This for loop just finds the beginning of the value. If firstws
     * is set, then it begins after the first whitespace. Otherwise, it begins
     * after the last whitespace. Quoted whitespace doesn't count as
     * whitespace. If there is no unquoted whitespace, then there is no
     * label, there's just a value.
     */
    for(p = string; p && *p;){
	if(*p == '"')				/* quoted label? */
	  quoted = (quoted) ? 0 : 1;

	if(*p == '\\' && *(p+1) == '"')		/* escaped quote? */
	  p++;					/* skip it... */

	if(isspace((unsigned char)*p) && !quoted){	/* if space,  */
	    while(*++p && isspace((unsigned char)*p))	/* move past it */
	      ;

	    if(!firstws || !token)
	      token = p;			/* remember start of text */
	}
	else
	  p++;
    }

    if(token){					/* copy label */
	*label = p = (char *)fs_get(((token - string) + 1) * sizeof(char));

	/* make a copy of the string */
	tmp = (char *)fs_get(((token - string) + 1) * sizeof(char));
	strncpy(tmp, string, token - string);
	tmp[token-string] = '\0';

	removing_leading_and_trailing_white_space(tmp);
	quoted = removing_double_quotes(tmp);
	
	for(q = tmp; *q; q++){
	    if(quoted && *q == '\\' && (*(q+1) == '"' || *(q+1) == '\\'))
	      *p++ = *++q;
	    else if(!(strip_internal_label_quotes && *q == '"'))
	      *p++ = *q;
	}

	*p = '\0';				/* tie off label */
	fs_give((void **)&tmp);
	if(*label == '\0')
	  fs_give((void **)label);
    }
    else
      token = string;

    if(token){					/* copy value */
	*value = p = (char *)fs_get((strlen(token) + 1) * sizeof(char));

	tmp = cpystr(token);
	removing_leading_and_trailing_white_space(tmp);
	quoted = removing_double_quotes(tmp);

	for(q = tmp; *q ; q++){
	    if(quoted && *q == '\\' && (*(q+1) == '"' || *(q+1) == '\\'))
	      *p++ = *++q;
	    else
	      *p++ = *q;
	}

	*p = '\0';				/* tie off value */
	fs_give((void **)&tmp);
    }
}


/*
 *  This is sort of the inverse of get_pair.
 *
 * Args --  label -- the first half of the string
 *          value -- the last half of the string
 *
 * Returns -- an allocated string which is "label" SPACE "value"
 *
 *  Label and value are quoted separately. If quoting is needed (they contain
 *  whitespace) then backslash escaping is done inside the quotes for
 *  " and for \. If quoting is not needed, no escaping is done.
 */
char *
put_pair(label, value)
    char *label, *value;
{
    char *result, *lab = label, *val = value;

    if(label && *label)
      lab = quote_if_needed(label);

    if(value && *value)
      val = quote_if_needed(value);

    result = (char *)fs_get((strlen(lab) + strlen(val) +1 +1) * sizeof(char));
    
    sprintf(result, "%s%s%s",
	    lab ? lab : "",
	    (lab && val) ? " " : "",
	    val ? val : "");

    if(lab && lab != label)
      fs_give((void **)&lab);
    if(val && val != value)
      fs_give((void **)&val);

    return(result);
}


/*
 * This is for put_pair type uses. It returns either an allocated
 * string which is the quoted src string or it returns a pointer to
 * the src string if no quoting is needed.
 */
char *
quote_if_needed(src)
    char *src;
{
    char *result = src, *qsrc = NULL;

    if(src && *src){
	/* need quoting? */
	if(strpbrk(src, " \t") != NULL)
	  qsrc = add_escapes(src, "\\\"", '\\', "", "");

	if(qsrc && !*qsrc)
	  fs_give((void **)&qsrc);

	if(qsrc){
	    result = (char *)fs_get((strlen(qsrc)+2+1) * sizeof(char));
	    sprintf(result, "\"%s\"", qsrc);
	    fs_give((void **)&qsrc);
	}
    }

    return(result);
}


/*
 * Convert a 1, 2, or 3-digit octal string into an 8-bit character.
 * Only the first three characters of s will be used, and it is ok not
 * to null-terminate it.
 */
int
read_octal(s)
    char **s;
{
    register int i, j;

    i = 0;
    for(j = 0; j < 3 && **s >= '0' && **s < '8' ; (*s)++, j++)
      i = (i * 8) + (int)(unsigned char)**s - '0';

    return(i);
}


/*
 * Convert two consecutive HEX digits to an integer.  First two
 * chars pointed to by "s" MUST already be tested for hexness.
 */
int
read_hex(s)
    char *s;
{
    return(X2C(s));
}


/*
 * Given a character c, put the 3-digit ascii octal value of that char
 * in the 2nd argument, which must be at least 3 in length.
 */
void
char_to_octal_triple(c, octal)
    int   c;
    char *octal;
{
    c &= 0xff;

    octal[2] = (c % 8) + '0';
    c /= 8;
    octal[1] = (c % 8) + '0';
    c /= 8;
    octal[0] = c + '0';
}


/*
 * Convert in memory string s to a C-style string, with backslash escapes
 * like they're used in C character constants.
 *
 * Returns allocated C string version of s.
 */
char *
string_to_cstring(s)
    char *s;
{
    char *b, *p;
    int   n, i;

    if(!s)
      return(cpystr(""));

    n = 20;
    b = (char *)fs_get((n+1) * sizeof(char));
    p  = b;
    *p = '\0';
    i  = 0;

    while(*s){
	if(i + 4 > n){
	    /*
	     * The output string may overflow the output buffer.
	     * Make more room.
	     */
	    n += 20;
	    fs_resize((void **)&b, (n+1) * sizeof(char));
	    p = &b[i];
	}
	else{
	    switch(*s){
	      case '\n':
		*p++ = '\\';
		*p++ = 'n';
		i += 2;
		break;

	      case '\r':
		*p++ = '\\';
		*p++ = 'r';
		i += 2;
		break;

	      case '\t':
		*p++ = '\\';
		*p++ = 't';
		i += 2;
		break;

	      case '\b':
		*p++ = '\\';
		*p++ = 'b';
		i += 2;
		break;

	      case '\f':
		*p++ = '\\';
		*p++ = 'f';
		i += 2;
		break;

	      case '\\':
		*p++ = '\\';
		*p++ = '\\';
		i += 2;
		break;

	      default:
		if(*s >= SPACE && *s <= '~'){
		    *p++ = *s;
		    i++;
		}
		else{  /* use octal output */
		    *p++ = '\\';
		    char_to_octal_triple(*s, p);
		    p += 3;
		    i += 4;
		}

		break;
	    }

	    s++;
	}
    }

    *p = '\0';
    return(b);
}


/*
 * Convert C-style string, with backslash escapes, into a hex string, two
 * hex digits per character.
 *
 * Returns allocated hexstring version of s.
 */
char *
cstring_to_hexstring(s)
    char *s;
{
    char *b, *p;
    int   n, i, c;

    if(!s)
      return(cpystr(""));

    n = 20;
    b = (char *)fs_get((n+1) * sizeof(char));
    p  = b;
    *p = '\0';
    i  = 0;

    while(*s){
	if(i + 2 > n){
	    /*
	     * The output string may overflow the output buffer.
	     * Make more room.
	     */
	    n += 20;
	    fs_resize((void **)&b, (n+1) * sizeof(char));
	    p = &b[i];
	}
	else{
	    if(*s == '\\'){
		s++;
		switch(*s){
		  case 'n':
		    c = '\n';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 'r':
		    c = '\r';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 't':
		    c = '\t';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 'v':
		    c = '\v';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 'b':
		    c = '\b';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 'f':
		    c = '\f';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 'a':
		    c = '\007';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case '\\':
		    c = '\\';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case '?':
		    c = '?';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case '\'':
		    c = '\'';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case '\"':
		    c = '\"';
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  case 0: /* reached end of s too early */
		    c = 0;
		    C2XPAIR(c, p);
		    i += 2;
		    s++;
		    break;

		  /* hex number */
		  case 'x':
		    s++;
		    if(isxpair(s)){
			c = X2C(s);
			s += 2;
		    }
		    else if(isxdigit((unsigned char)*s)){
			c = XDIGIT2C(*s);
			s++;
		    }
		    else
		      c = 0;

		    C2XPAIR(c, p);
		    i += 2;

		    break;

		  /* octal number */
		  default:
		    c = read_octal(&s);
		    C2XPAIR(c, p);
		    i += 2;

		    break;
		}
	    }
	    else{
		C2XPAIR(*s, p);
		i += 2;
		s++;
	    }
	}
    }

    *p = '\0';
    return(b);
}


/*
 * Convert C-style string, with backslash escapes, into a regular string.
 * Result goes in dst, which should be as big as src.
 *
 */
void
cstring_to_string(src, dst)
    char *src;
    char *dst;
{
    char *p;
    int   c;

    dst[0] = '\0';
    if(!src)
      return;

    p  = dst;

    while(*src){
	if(*src == '\\'){
	    src++;
	    switch(*src){
	      case 'n':
		*p++ = '\n';
		src++;
		break;

	      case 'r':
		*p++ = '\r';
		src++;
		break;

	      case 't':
		*p++ = '\t';
		src++;
		break;

	      case 'v':
		*p++ = '\v';
		src++;
		break;

	      case 'b':
		*p++ = '\b';
		src++;
		break;

	      case 'f':
		*p++ = '\f';
		src++;
		break;

	      case 'a':
		*p++ = '\007';
		src++;
		break;

	      case '\\':
		*p++ = '\\';
		src++;
		break;

	      case '?':
		*p++ = '?';
		src++;
		break;

	      case '\'':
		*p++ = '\'';
		src++;
		break;

	      case '\"':
		*p++ = '\"';
		src++;
		break;

	      case 0: /* reached end of s too early */
		src++;
		break;

	      /* hex number */
	      case 'x':
		src++;
		if(isxpair(src)){
		    c = X2C(src);
		    src += 2;
		}
		else if(isxdigit((unsigned char)*src)){
		    c = XDIGIT2C(*src);
		    src++;
		}
		else
		  c = 0;

		*p++ = c;

		break;

	      /* octal number */
	      default:
		c = read_octal(&src);
		*p++ = c;
		break;
	    }
	}
	else
	  *p++ = *src++;
    }

    *p = '\0';
}


/*
 * Quotes /'s and \'s with \
 *
 * Args: src -- The source string.
 *
 * Returns: A string with backslash quoting added. Any / in the string is
 *          replaced with \/ and any \ is replaced with \\, and any
 *          " is replaced with \".
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
add_backslash_escapes(src)
    char *src;
{
    return(add_escapes(src, "/\\\"", '\\', "", ""));
}


/*
 * Does pattern quoting. Takes the string that the user sees and converts
 * it to the config file string.
 *
 * Args: src -- The source string.
 *
 * The last arg to add_escapes causes \, and \\ to be replaced with hex
 * versions of comma and backslash. That's so we can embed commas in
 * list variables without having them act as separators. If the user wants
 * a literal comma, they type backslash comma.
 * If /, \, or " appear (other than the special cases in previous sentence)
 * they are backslash-escaped like \/, \\, or \".
 *
 * Returns: An allocated string with proper necessary added.
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
add_pat_escapes(src)
    char *src;
{
    return(add_escapes(src, "/\\\"", '\\', "", ",\\"));
}


/*
 * This takes envelope data and adds the backslash escapes that the user
 * would have been responsible for adding if editing manually.
 * It just escapes commas and backslashes.
 *
 * Caller must free result.
 */
char *
add_roletake_escapes(src)
    char *src;
{
    return(add_escapes(src, ",\\", '\\', "", ""));
}

/*
 * This function is similar to add_roletake_escapes, but it only escapes
 * commas.  This is for Specific Folders and Action folders in a role,
 * whereas, currently, all other fields for roles require that both 
 * commas and backslashes be escaped.
 */
char *
add_folder_escapes(src)
     char *src;
{
    return(add_escapes(src, ",", '\\', "", ""));
}

/*
 * Quote values for viewer-hdr-colors. We quote backslash, comma, and slash.
 *
 * Args: src -- The source string.
 *
 * Returns: A string with backslash quoting added.
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
add_viewerhdr_escapes(src)
    char *src;
{
    return(add_escapes(src, "/\\", '\\', ",", ""));
}


/*
 * This adds the quoting for vcard backslash quoting.
 * That is, commas are backslashed, backslashes are backslashed, 
 * semicolons are backslashed, and CRLFs are \n'd.
 * This is thought to be correct for draft-ietf-asid-mime-vcard-06.txt, Apr 98.
 */
char *
vcard_escape(src)
    char *src;
{
    char *p, *q;

    q = add_escapes(src, ";,\\", '\\', "", "");
    if(q){
	/* now do CRLF -> \n in place */
	for(p = q; *p != '\0'; p++)
	  if(*p == '\r' && *(p+1) == '\n'){
	      *p++ = '\\';
	      *p = 'n';
	  }
    }

    return(q);
}


/*
 * This undoes the vcard backslash quoting.
 *
 * In particular, it turns \n into newline, \, into ',', \\ into \, \; -> ;.
 * In fact, \<anything_else> is also turned into <anything_else>. The ID
 * isn't clear on this.
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
vcard_unescape(src)
    char *src;
{
    char *ans = NULL, *p;
    int done = 0;

    if(src){
	p = ans = (char *)fs_get(strlen(src) + 1);

	while(!done){
	    switch(*src){
	      case '\\':
		src++;
		if(*src == 'n' || *src == 'N'){
		    *p++ = '\n';
		    src++;
		}
		else if(*src)
		  *p++ = *src++;

		break;
	    
	      case '\0':
		done++;
		break;

	      default:
		*p++ = *src++;
		break;
	    }
	}

	*p = '\0';
    }

    return(ans);
}


/*
 * Turn folded lines into long lines in place.
 *
 * CRLF whitespace sequences are removed, the space is not preserved.
 */
void
vcard_unfold(string)
    char *string;
{
    char *p = string;

    while(*string)		      /* while something to copy  */
      if(*string == '\r' &&
         *(string+1) == '\n' &&
	 (*(string+2) == SPACE || *(string+2) == TAB))
	string += 3;
      else
	*p++ = *string++;
    
    *p = '\0';
}


/*
 * Quote specified chars with escape char.
 *
 * Args:          src -- The source string.
 *  quote_these_chars -- Array of chars to quote
 *       quoting_char -- The quoting char to be used (e.g., \)
 *    hex_these_chars -- Array of chars to hex escape
 *    hex_these_quoted_chars -- Array of chars to hex escape if they are
 *                              already quoted with quoting_char (that is,
 *                              turn \, into hex comma)
 *
 * Returns: An allocated copy of string with quoting added.
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
add_escapes(src, quote_these_chars, quoting_char, hex_these_chars,
	    hex_these_quoted_chars)
    char *src;
    char *quote_these_chars;
    int   quoting_char;
    char *hex_these_chars;
    char *hex_these_quoted_chars;
{
    char *ans = NULL;

    if(!quote_these_chars)
      panic("bad arg to add_escapes");

    if(src){
	char *q, *p, *qchar;

	p = q = (char *)fs_get(2*strlen(src) + 1);

	while(*src){
	    if(*src == quoting_char)
	      for(qchar = hex_these_quoted_chars; *qchar != '\0'; qchar++)
		if(*(src+1) == *qchar)
		  break;

	    if(*src == quoting_char && *qchar){
		src++;	/* skip quoting_char */
		*p++ = '\\';
		*p++ = 'x';
		C2XPAIR(*src, p);
		src++;	/* skip quoted char */
	    }
	    else{
		for(qchar = quote_these_chars; *qchar != '\0'; qchar++)
		  if(*src == *qchar)
		    break;

		if(*qchar){		/* *src is a char to be quoted */
		    *p++ = quoting_char;
		    *p++ = *src++;
		}
		else{
		    for(qchar = hex_these_chars; *qchar != '\0'; qchar++)
		      if(*src == *qchar)
			break;

		    if(*qchar){		/* *src is a char to be escaped */
			*p++ = '\\';
			*p++ = 'x';
			C2XPAIR(*src, p);
			src++;
		    }
		    else			/* a regular char */
		      *p++ = *src++;
		}
	    }

	}

	*p = '\0';

	ans = cpystr(q);
	fs_give((void **)&q);
    }

    return(ans);
}


/*
 * Undoes backslash quoting of source string.
 *
 * Args: src -- The source string.
 *
 * Returns: A string with backslash quoting removed or NULL. The string starts
 *          at src and goes until the end of src or until a / is reached. The
 *          / is not included in the string. /'s may be quoted by preceding
 *          them with a backslash (\) and \'s may also be quoted by
 *          preceding them with a \. In fact, \ quotes any character.
 *          Not quite, \nnn is octal escape, \xXX is hex escape.
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
remove_backslash_escapes(src)
    char *src;
{
    char *ans = NULL, *q, *p;
    int done = 0;

    if(src){
	p = q = (char *)fs_get(strlen(src) + 1);

	while(!done){
	    switch(*src){
	      case '\\':
		src++;
		if(*src){
		    if(isdigit((unsigned char)*src))
		      *p++ = (char)read_octal(&src);
		    else if((*src == 'x' || *src == 'X') &&
			    *(src+1) && *(src+2) && isxpair(src+1)){
			*p++ = (char)read_hex(src+1);
			src += 3;
		    }
		    else
		      *p++ = *src++;
		}

		break;
	    
	      case '\0':
	      case '/':
		done++;
		break;

	      default:
		*p++ = *src++;
		break;
	    }
	}

	*p = '\0';

	ans = cpystr(q);
	fs_give((void **)&q);
    }

    return(ans);
}


/*
 * Undoes the escape quoting done by add_pat_escapes.
 *
 * Args: src -- The source string.
 *
 * Returns: A string with backslash quoting removed or NULL. The string starts
 *          at src and goes until the end of src or until a / is reached. The
 *          / is not included in the string. /'s may be quoted by preceding
 *          them with a backslash (\) and \'s may also be quoted by
 *          preceding them with a \. In fact, \ quotes any character.
 *          Not quite, \nnn is octal escape, \xXX is hex escape.
 *          Hex escapes are undone but left with a backslash in front.
 *
 *   The caller is responsible for freeing the memory allocated for the answer.
 */
char *
remove_pat_escapes(src)
    char *src;
{
    char *ans = NULL, *q, *p;
    int done = 0;

    if(src){
	p = q = (char *)fs_get(strlen(src) + 1);

	while(!done){
	    switch(*src){
	      case '\\':
		src++;
		if(*src){
		    if(isdigit((unsigned char)*src)){	/* octal escape */
			*p++ = '\\';
			*p++ = (char)read_octal(&src);
		    }
		    else if((*src == 'x' || *src == 'X') &&
			    *(src+1) && *(src+2) && isxpair(src+1)){
			*p++ = '\\';
			*p++ = (char)read_hex(src+1);
			src += 3;
		    }
		    else
		      *p++ = *src++;
		}

		break;
	    
	      case '\0':
	      case '/':
		done++;
		break;

	      default:
		*p++ = *src++;
		break;
	    }
	}

	*p = '\0';

	ans = cpystr(q);
	fs_give((void **)&q);
    }

    return(ans);
}


/*
 * Copy a string enclosed in "" without fixing \" or \\. Skip past \"
 * but copy it as is, removing only the enclosing quotes.
 */
char *
copy_quoted_string_asis(src)
    char *src;
{
    char *q, *p;
    int   done = 0, quotes = 0;

    if(src){
	p = q = (char *)fs_get(strlen(src) + 1);

	while(!done){
	    switch(*src){
	      case QUOTE:
		if(++quotes == 2)
		  done++;
		else
		  src++;

		break;

	      case BSLASH:	/* don't count \" as a quote, just copy */
		if(*(src+1) == QUOTE){
		    if(quotes == 1){
			*p++ = *src;
			*p++ = *(src+1);
		    }

		    src += 2;
		}
		else{
		    if(quotes == 1)
		      *p++ = *src;
		    
		    src++;
		}

		break;
	    
	      case '\0':
		fs_give((void **)&q);
		return(NULL);

	      default:
		if(quotes == 1)
		  *p++ = *src;
		
		src++;

		break;
	    }
	}

	*p = '\0';
    }

    return(q);
}


/*
 * Returns non-zero if dir is a prefix of path.
 *         zero     if dir is not a prefix of path, or if dir is empty.
 */
int
in_dir(dir, path)
    char *dir;
    char *path;
{
    return(*dir ? !strncmp(dir, path, strlen(dir)) : 0);
}


/*
 * isxpair -- return true if the first two chars in string are
 *	      hexidecimal characters
 */
int
isxpair(s)
    char *s;
{
    return(isxdigit((unsigned char) *s) && isxdigit((unsigned char) *(s+1)));
}





/*
 *  * * * * * *  something to help managing lists of strings   * * * * * * * *
 */


STRLIST_S *
new_strlist()
{
    STRLIST_S *sp = (STRLIST_S *) fs_get(sizeof(STRLIST_S));
    memset(sp, 0, sizeof(STRLIST_S));
    return(sp);
}



void
free_strlist(strp)
    STRLIST_S **strp;
{
    if(*strp){
	if((*strp)->next)
	  free_strlist(&(*strp)->next);

	if((*strp)->name)
	  fs_give((void **) &(*strp)->name);

	fs_give((void **) strp);
    }
}



/*
 *  * * * * * * * *      RFC 1522 support routines      * * * * * * * *
 *
 *   RFC 1522 support is *very* loosely based on code contributed
 *   by Lars-Erik Johansson <lej@cdg.chalmers.se>.  Thanks to Lars-Erik,
 *   and appologies for taking such liberties with his code.
 */


#define	RFC1522_INIT	"=?"
#define	RFC1522_INIT_L	2
#define RFC1522_TERM	"?="
#define	RFC1522_TERM_L	2
#define	RFC1522_DLIM	"?"
#define	RFC1522_DLIM_L	1
#define	RFC1522_MAXW	75
#define	ESPECIALS	"()<>@,;:\"/[]?.="
#define	RFC1522_OVERHEAD(S)	(RFC1522_INIT_L + RFC1522_TERM_L +	\
				 (2 * RFC1522_DLIM_L) + strlen(S) + 1);
#define	RFC1522_ENC_CHAR(C)	(((C) & 0x80) || !rfc1522_valtok(C)	\
				 || (C) == '_' )


int	       rfc1522_token PROTO((char *, int (*) PROTO((int)), char *,
				    char **));
int	       rfc1522_valtok PROTO((int));
int	       rfc1522_valenc PROTO((int));
int	       rfc1522_valid PROTO((char *, char **, char **, char **,
				    char **));
char	      *rfc1522_8bit PROTO((void *, int));
char	      *rfc1522_binary PROTO((void *, int));
unsigned char *rfc1522_encoded_word PROTO((unsigned char *, int, char *));


/*
 * rfc1522_decode - decode the given source string ala RFC 2047 (nee 1522),
 *		    IF NECESSARY, into the given destination buffer.
 *		    Don't bother copying if it turns out decoding
 *		    isn't necessary.
 *
 * Returns: pointer to either the destination buffer containing the
 *	    decoded text, or a pointer to the source buffer if there was
 *	    no reason to decode it.
 */
unsigned char *
rfc1522_decode(d, len, s, charset)
    unsigned char  *d;
    size_t          len;	/* length of d */
    char	   *s;
    char	  **charset;
{
    unsigned char *rv = NULL, *p;
    char	  *start = s, *sw, *cset, *enc, *txt, *ew, **q, *lang;
    unsigned long  l;
    int		   i;

    *d = '\0';					/* init destination */
    if(charset)
      *charset = NULL;

    while(s && (sw = strstr(s, RFC1522_INIT))){
	/* validate the rest of the encoded-word */
	if(rfc1522_valid(sw, &cset, &enc, &txt, &ew)){
	    if(!rv)
	      rv = d;				/* remember start of dest */

	    /* copy everything between s and sw to destination */
	    for(i = 0; &s[i] < sw; i++)
	      if(!isspace((unsigned char)s[i])){ /* if some non-whitespace */
		  while(s < sw && d-rv<len-1)
		    *d++ = (unsigned char) *s++;

		  break;
	      }

	    enc[-1] = txt[-1] = ew[0] = '\0';	/* tie off token strings */

	    if(lang = strchr(cset, '*'))
	      *lang++ = '\0';

	    /* Insert text explaining charset if we don't know what it is */
	    if((!ps_global->VAR_CHAR_SET
		|| strucmp((char *) cset, ps_global->VAR_CHAR_SET))
	       && strucmp((char *) cset, "US-ASCII")){
		dprint(5, (debugfile, "RFC1522_decode: charset mismatch: %s\n",
			   cset));
		if(charset){
		    if(!*charset)		/* only write first charset */
		      *charset = cpystr(cset);
		}
		else{
		    if(d-rv<len-1)
		      *d++ = '[';

		    sstrncpy((char **) &d, cset, len-1-(d-rv));
		    if(d-rv<len-1)
		      *d++ = ']';
		    if(d-rv<len-1)
		      *d++ = SPACE;
		}
	    }

	    /* based on encoding, write the encoded text to output buffer */
	    switch(*enc){
	      case 'Q' :			/* 'Q' encoding */
	      case 'q' :
		/* special hocus-pocus to deal with '_' exception, too bad */
		for(l = 0L, i = 0; txt[l]; l++)
		  if(txt[l] == '_')
		    i++;

		if(i){
		    q = (char **) fs_get((i + 1) * sizeof(char *));
		    for(l = 0L, i = 0; txt[l]; l++)
		      if(txt[l] == '_'){
			  q[i++] = &txt[l];
			  txt[l] = SPACE;
		      }

		    q[i] = NULL;
		}
		else
		  q = NULL;

		if(p = rfc822_qprint((unsigned char *)txt, strlen(txt), &l)){
		    strncpy((char *) d, (char *) p, len-1-(d-rv));
		    d[len-1-(d-rv)] = '\0';
		    fs_give((void **)&p);	/* free encoded buf */
		    d += l;			/* advance dest ptr to EOL */
		    if(d-rv > len-1)
		      d = rv+len-1;
		}
		else{
		    if(q)
		      fs_give((void **) &q);

		    goto bogus;
		}

		if(q){				/* restore underscores */
		    for(i = 0; q[i]; i++)
		      *(q[i]) = '_';

		    fs_give((void **)&q);
		}

		break;

	      case 'B' :			/* 'B' encoding */
	      case 'b' :
		if(p = rfc822_base64((unsigned char *) txt, strlen(txt), &l)){
		    strncpy((char *) d, (char *) p, len-1-(d-rv));
		    d[len-1-(d-rv)] = '\0';
		    fs_give((void **)&p);	/* free encoded buf */
		    d += l;			/* advance dest ptr to EOL */
		    if(d-rv > len-1)
		      d = rv+len-1;
		}
		else
		  goto bogus;

		break;

	      default:
		sstrncpy((char **) &d, txt, len-1-(d-rv));
		dprint(1, (debugfile, "RFC1522_decode: Unknown ENCODING: %s\n",
			   enc));
		break;
	    }

	    /* restore trompled source string */
	    enc[-1] = txt[-1] = '?';
	    ew[0]   = RFC1522_TERM[0];

	    /* advance s to start of text after encoded-word */
	    s = ew + RFC1522_TERM_L;

	    if(lang)
	      lang[-1] = '*';
	}
	else{

	    /*
	     * Found intro, but bogus data followed, treat it as normal text.
	     */

	    /* if already copying to destn, copy it */
	    if(rv){
		strncpy((char *) d, s,
			(int) min((l = (sw - s) + RFC1522_INIT_L),
			len-1-(d-rv)));
		d += l;				/* advance d, tie off text */
		if(d-rv > len-1)
		  d = rv+len-1;
		*d = '\0';
		s += l;				/* advance s beyond intro */
	    }
	    else
	      s += ((sw - s) + RFC1522_INIT_L);
	}
    }

    if(rv && *s)				/* copy remaining text */
      strncat((char *)rv, s, len-1-strlen((char *)rv));

/* BUG: MUST do code page mapping under DOS after decoding */

    return(rv ? rv : (unsigned char *) start);

  bogus:
    dprint(1, (debugfile, "RFC1522_decode: BOGUS INPUT: -->%s<--\n", start));
    return((unsigned char *) start);
}


/*
 * rfc1522_token - scan the given source line up to the end_str making
 *		   sure all subsequent chars are "valid" leaving endp
 *		   a the start of the end_str.
 * Returns: TRUE if we got a valid token, FALSE otherwise
 */
int
rfc1522_token(s, valid, end_str, endp)
    char  *s;
    int	 (*valid) PROTO((int));
    char  *end_str;
    char **endp;
{
    while(*s){
	if((char) *s == *end_str		/* test for matching end_str */
	   && ((end_str[1])
	        ? !strncmp((char *)s + 1, end_str + 1, strlen(end_str + 1))
	        : 1)){
	    *endp = s;
	    return(TRUE);
	}

	if(!(*valid)(*s++))			/* test for valid char */
	  break;
    }

    return(FALSE);
}


/*
 * rfc1522_valtok - test for valid character in the RFC 1522 encoded
 *		    word's charset and encoding fields.
 */
int
rfc1522_valtok(c)
    int c;
{
    return(!(c == SPACE || iscntrl(c & 0x7f) || strindex(ESPECIALS, c)));
}


/*
 * rfc1522_valenc - test for valid character in the RFC 1522 encoded
 *		    word's encoded-text field.
 */
int
rfc1522_valenc(c)
    int c;
{
    return(!(c == '?' || c == SPACE) && isprint((unsigned char)c));
}


/*
 * rfc1522_valid - validate the given string as to it's rfc1522-ness
 */
int
rfc1522_valid(s, charset, enc, txt, endp)
    char  *s;
    char **charset;
    char **enc;
    char **txt;
    char **endp;
{
    char *c, *e, *t, *p;
    int   rv;

    rv = rfc1522_token(c = s+RFC1522_INIT_L, rfc1522_valtok, RFC1522_DLIM, &e)
	   && rfc1522_token(++e, rfc1522_valtok, RFC1522_DLIM, &t)
	   && rfc1522_token(++t, rfc1522_valenc, RFC1522_TERM, &p)
	   && p - s <= RFC1522_MAXW;

    if(charset)
      *charset = c;

    if(enc)
      *enc = e;

    if(txt)
      *txt = t;

    if(endp)
      *endp = p;

    return(rv);
}


/*
 * rfc1522_encode - encode the given source string ala RFC 1522,
 *		    IF NECESSARY, into the given destination buffer.
 *		    Don't bother copying if it turns out encoding
 *		    isn't necessary.
 *
 * Returns: pointer to either the destination buffer containing the
 *	    encoded text, or a pointer to the source buffer if we didn't
 *          have to encode anything.
 */
char *
rfc1522_encode(d, len, s, charset)
    char	  *d;
    size_t         len;		/* length of d */
    unsigned char *s;
    char	  *charset;
{
    unsigned char *p, *q;
    int		   n;

    if(!s)
      return((char *) s);

    if(!charset)
      charset = UNKNOWN_CHARSET;

    /* look for a reason to encode */
    for(p = s, n = 0; *p; p++)
      if((*p) & 0x80){
	  n++;
      }
      else if(*p == RFC1522_INIT[0]
	      && !strncmp((char *) p, RFC1522_INIT, RFC1522_INIT_L)){
	  if(rfc1522_valid((char *) p, NULL, NULL, NULL, (char **) &q))
	    p = q + RFC1522_TERM_L - 1;		/* advance past encoded gunk */
      }
      else if(*p == ESCAPE && match_escapes((char *)(p+1))){
	  n++;
      }

    if(n){					/* found, encoding to do */
	char *rv  = d, *t,
	      enc = (n > (2 * (p - s)) / 3) ? 'B' : 'Q';

	while(*s){
	    if(d-rv < len-1-(RFC1522_INIT_L+2*RFC1522_DLIM_L+1)){
		sstrcpy(&d, RFC1522_INIT);	/* insert intro header, */
		sstrcpy(&d, charset);		/* character set tag, */
		sstrcpy(&d, RFC1522_DLIM);	/* and encoding flavor */
		*d++ = enc;
		sstrcpy(&d, RFC1522_DLIM);
	    }

	    /*
	     * feed lines to encoder such that they're guaranteed
	     * less than RFC1522_MAXW.
	     */
	    p = rfc1522_encoded_word(s, enc, charset);
	    if(enc == 'B')			/* insert encoded data */
	      sstrncpy(&d, t = rfc1522_binary(s, p - s), len-1-(d-rv));
	    else				/* 'Q' encoding */
	      sstrncpy(&d, t = rfc1522_8bit(s, p - s), len-1-(d-rv));

	    sstrncpy(&d, RFC1522_TERM, len-1-(d-rv));	/* insert terminator */
	    fs_give((void **) &t);
	    if(*p)				/* more src string follows */
	      sstrncpy(&d, "\015\012 ", len-1-(d-rv));	/* insert cont. line */

	    s = p;				/* advance s */
	}

	rv[len-1] = '\0';
	return(rv);
    }
    else
      return((char *) s);			/* no work for us here */
}



/*
 * rfc1522_encoded_word -- cut given string into max length encoded word
 *
 * Return: pointer into 's' such that the encoded 's' is no greater
 *	   than RFC1522_MAXW
 *
 *  NOTE: this line break code is NOT cognizant of any SI/SO
 *  charset requirements nor similar strategies using escape
 *  codes.  Hopefully this will matter little and such
 *  representation strategies don't also include 8bit chars.
 */
unsigned char *
rfc1522_encoded_word(s, enc, charset)
    unsigned char *s;
    int		   enc;
    char	  *charset;
{
    int goal = RFC1522_MAXW - RFC1522_OVERHEAD(charset);

    if(enc == 'B')			/* base64 encode */
      for(goal = ((goal / 4) * 3) - 2; goal && *s; goal--, s++)
	;
    else				/* special 'Q' encoding */
      for(; goal && *s; s++)
	if((goal -= RFC1522_ENC_CHAR(*s) ? 3 : 1) < 0)
	  break;

    return(s);
}



/*
 * rfc1522_8bit -- apply RFC 1522 'Q' encoding to the given 8bit buffer
 *
 * Return: alloc'd buffer containing encoded string
 */
char *
rfc1522_8bit(src, slen)
    void *src;
    int   slen;
{
    char *ret = (char *) fs_get ((size_t) (3*slen + 2));
    char *d = ret;
    unsigned char c;
    unsigned char *s = (unsigned char *) src;

    while (slen--) {				/* for each character */
	if (((c = *s++) == '\015') && (*s == '\012') && slen) {
	    *d++ = '\015';			/* true line break */
	    *d++ = *s++;
	    slen--;
	}
	else if(c == SPACE){			/* special encoding case */
	    *d++ = '_';
	}
	else if(RFC1522_ENC_CHAR(c)){
	    *d++ = '=';				/* quote character */
	    C2XPAIR(c, d);
	}
	else
	  *d++ = (char) c;			/* ordinary character */
    }

    *d = '\0';					/* tie off destination */
    return(ret);
}


/*
 * rfc1522_binary -- apply RFC 1522 'B' encoding to the given 8bit buffer
 *
 * Return: alloc'd buffer containing encoded string
 */
char *
rfc1522_binary (src, srcl)
    void *src;
    int   srcl;
{
    static char *v =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char *s = (unsigned char *) src;
    char *ret, *d;

    d = ret = (char *) fs_get ((size_t) ((((srcl + 2) / 3) * 4) + 1));
    for (; srcl; s += 3) {	/* process tuplets */
				/* byte 1: high 6 bits (1) */
	*d++ = v[s[0] >> 2];
				/* byte 2: low 2 bits (1), high 4 bits (2) */
	*d++ = v[((s[0] << 4) + (--srcl ? (s[1] >> 4) : 0)) & 0x3f];
				/* byte 3: low 4 bits (2), high 2 bits (3) */
	*d++ = srcl ? v[((s[1] << 2) + (--srcl ? (s[2] >> 6) :0)) & 0x3f] :'=';
				/* byte 4: low 6 bits (3) */
	*d++ = srcl ? v[s[2] & 0x3f] : '=';
	if(srcl)
	  srcl--;		/* count third character if processed */
    }

    *d = '\0';			/* tie off string */
    return(ret);		/* return the resulting string */
}



/*
 *  * * * * * * * *      RFC 1738 support routines      * * * * * * * *
 */


/*
 * Various helpful definitions
 */
#define	RFC1738_SAFE	"$-_.+"			/* "safe" */
#define	RFC1738_EXTRA	"!*'(),"		/* "extra" */
#define	RFC1738_RSVP	";/?:@&="		/* "reserved" */
#define	RFC1738_NEWS	"-.+_"			/* valid for "news:" URL */
#define	RFC1738_FUDGE	"#{}|\\^~[]"		/* Unsafe, but popular */
#define	RFC1738_ESC(S)	(*(S) == '%' && isxpair((S) + 1))


int   rfc1738uchar PROTO((char *));
int   rfc1738xchar PROTO((char *));
char *rfc1738_scheme_part PROTO((char *));



/*
 * rfc1738_scan -- Scan the given line for possible URLs as defined
 *		   in RFC1738
 */
char *
rfc1738_scan(line, len)
    char *line;
    int  *len;
{
    char *colon, *start, *end;
    int   n;

    /* process each : in the line */
    for(; colon = strindex(line, ':'); line = end){
	end = colon + 1;
	if(colon == line)		/* zero length scheme? */
	  continue;

	/*
	 * Valid URL (ala RFC1738 BNF)?  First, first look to the
	 * left to make sure there are valid "scheme" chars...
	 */
	start = colon - 1;
	while(1)
	  if(!(isdigit((unsigned char) *start)
	       || isalpha((unsigned char) *start)
	       || strchr("+-.", *start))){
	      start++;			/* advance over bogus char */
	      break;
	  }
	  else if(start > line)
	    start--;
	  else
	    break;

	/*
	 * Make sure everyhing up to the colon is a known scheme...
	 */
	if(start && (n = colon - start) && !isdigit((unsigned char) *start)
	   && (((n == 3
		 && (*start == 'F' || *start == 'f')
		 && !struncmp(start+1, "tp", 2))
		|| (n == 4
		    && (((*start == 'H' || *start == 'h') 
			 && !struncmp(start + 1, "ttp", 3))
			|| ((*start == 'N' || *start == 'n')
			    && !struncmp(start + 1, "ews", 3))
			|| ((*start == 'N' || *start == 'n')
			    && !struncmp(start + 1, "ntp", 3))
			|| ((*start == 'W' || *start == 'w')
			    && !struncmp(start + 1, "ais", 3))
#ifdef	ENABLE_LDAP
			|| ((*start == 'L' || *start == 'l')
			    && !struncmp(start + 1, "dap", 3))
#endif
			|| ((*start == 'I' || *start == 'i')
			    && !struncmp(start + 1, "map", 3))
			|| ((*start == 'F' || *start == 'f')
			    && !struncmp(start + 1, "ile", 3))))
		|| (n == 5
		    && (*start == 'H' || *start == 'h')
		    && !struncmp(start+1, "ttps", 4))
		|| (n == 6
		    && (((*start == 'G' || *start == 'g')
			 && !struncmp(start+1, "opher", 5))
			|| ((*start == 'M' || *start == 'm')
			    && !struncmp(start + 1, "ailto", 5))
			|| ((*start == 'T' || *start == 't')
			    && !struncmp(start + 1, "elnet", 5))))
		|| (n == 8
		    && (*start == 'P' || *start == 'p')
		    && !struncmp(start + 1, "rospero", 7)))
	       || url_external_specific_handler(start, n))){
		/*
		 * Second, make sure that everything to the right of the
		 * colon is valid for a "schemepart"...
		 */

	    if((end = rfc1738_scheme_part(colon + 1)) - colon > 1){
		int i, j;

		/* make sure something useful follows colon */
		for(i = 0, j = end - colon; i < j; i++)
		  if(!strchr(RFC1738_RSVP, colon[i]))
		    break;

		if(i != j){
		    *len = end - start;

		    /*
		     * Special case handling for comma.
		     * See the problem is comma's valid, but if it's the
		     * last character in the url, it's likely intended
		     * as a delimiter in the text rather part of the URL.
		     * In most cases any way, that's why we have the
		     * exception.
		     */
		    if(*(end - 1) == ','
		       || (*(end - 1) == '.' && (!*end  || *end == ' ')))
		      (*len)--;

		    if(*len - (colon - start) > 0)
		      return(start);
		}
	    }
	}
    }

    return(NULL);
}


/*
 * rfc1738_scheme_part - make sure what's to the right of the
 *			 colon is valid
 *
 * NOTE: we have a problem matching closing parens when users 
 *       bracket the url in parens.  So, lets try terminating our
 *	 match on any closing paren that doesn't have a coresponding
 *       open-paren.
 */
char *
rfc1738_scheme_part(s)
    char *s;
{
    int n, paren = 0, bracket = 0;

    while(1)
      switch(*s){
	default :
	  if(n = rfc1738xchar(s)){
	      s += n;
	      break;
	  }

	case '\0' :
	  return(s);

	case '[' :
	  bracket++;
	  s++;
	  break;

	case ']' :
	  if(bracket--){
	      s++;
	      break;
	  }

	  return(s);

	case '(' :
	  paren++;
	  s++;
	  break;

	case ')' :
	  if(paren--){
	      s++;
	      break;
	  }

	  return(s);
      }
}



/*
 * rfc1738_str - convert rfc1738 escaped octets in place
 */
char *
rfc1738_str(s)
    char *s;
{
    register char *p = s, *q = s;

    while(1)
      switch(*q = *p++){
	case '%' :
	  if(isxpair(p)){
	      *q = X2C(p);
	      p += 2;
	  }

	default :
	  q++;
	  break;

	case '\0':
	  return(s);
      }
}


/*
 * rfc1738uchar - returns TRUE if the given char fits RFC 1738 "uchar" BNF
 */
int
rfc1738uchar(s)
    char *s;
{
    return((RFC1738_ESC(s))		/* "escape" */
	     ? 2
	     : (isalnum((unsigned char) *s)	/* alphanumeric */
		|| strchr(RFC1738_SAFE, *s)	/* other special stuff */
		|| strchr(RFC1738_EXTRA, *s)));
}


/*
 * rfc1738xchar - returns TRUE if the given char fits RFC 1738 "xchar" BNF
 */
int
rfc1738xchar(s)
    char *s;
{
    int n;

    return((n = rfc1738uchar(s))
	    ? n
	    : (strchr(RFC1738_RSVP, *s) != NULL
	       || strchr(RFC1738_FUDGE, *s)));
}


/*
 * rfc1738_num - return long value of a string of digits, possibly escaped
 */
long
rfc1738_num(s)
    char **s;
{
    register char *p = *s;
    long n = 0L;

    for(; *p; p++)
      if(*p == '%' && isxpair(p+1)){
	  int c = X2C(p+1);
	  if(isdigit((unsigned char) c)){
	      n = (c - '0') + (n * 10);
	      p += 2;
	  }
	  else
	    break;
      }
      else if(isdigit((unsigned char) *p))
	n = (*p - '0') + (n * 10);
      else
	break;

    *s = p;
    return(n);
}


int
rfc1738_group(s)
    char *s;
{
    return(isalnum((unsigned char) *s)
	   || RFC1738_ESC(s)
	   || strchr(RFC1738_NEWS, *s));
}


/*
 * Encode (hexify) a mailto url.
 *
 * Args  s -- src url
 *
 * Returns  An allocated string which is suitably encoded.
 *          Result should be freed by caller.
 *
 * Since we don't know here which characters are reserved characters (? and &)
 * for use in delimiting the pieces of the url and which are just those
 * characters contained in the data that should be encoded, we always encode
 * them. That's because we know we don't use those as reserved characters.
 * If you do use those as reserved characters you have to encode each part
 * separately.
 */
char *
rfc1738_encode_mailto(s)
    char *s;
{
    char *p, *d, *ret = NULL;

    if(s){
	/* Worst case, encode every character */
	ret = d = (char *)fs_get((3*strlen(s) + 1) * sizeof(char));
	while(*s){
	    if(isalnum((unsigned char)*s)
	       || strchr(RFC1738_SAFE, *s)
	       || strchr(RFC1738_EXTRA, *s))
	      *d++ = *s++;
	    else{
		*d++ = '%';
		C2XPAIR(*s, d);
		s++;
	    }
	}

	*d = '\0';
    }

    return(ret);
}


/*
 *  * * * * * * * *      RFC 1808 support routines      * * * * * * * *
 */


int
rfc1808_tokens(url, scheme, net_loc, path, parms, query, frag)
    char  *url;
    char **scheme, **net_loc, **path, **parms, **query, **frag;
{
    char *p, *q, *start, *tmp = cpystr(url);

    start = tmp;
    if(p = strchr(start, '#')){		/* fragment spec? */
	*p++ = '\0';
	if(*p)
	  *frag = cpystr(p);
    }

    if((p = strchr(start, ':')) && p != start){ /* scheme part? */
	for(q = start; q < p; q++)
	  if(!(isdigit((unsigned char) *q)
	       || isalpha((unsigned char) *q)
	       || strchr("+-.", *q)))
	    break;

	if(p == q){
	    *p++ = '\0';
	    *scheme = cpystr(start);
	    start = p;
	}
    }

    if(*start == '/' && *(start+1) == '/'){ /* net_loc */
	if(p = strchr(start+2, '/'))
	  *p++ = '\0';

	*net_loc = cpystr(start+2);
	if(p)
	  start = p;
	else *start = '\0';		/* End of parse */
    }

    if(p = strchr(start, '?')){
	*p++ = '\0';
	*query = cpystr(p);
    }

    if(p = strchr(start, ';')){
	*p++ = '\0';
	*parms = cpystr(p);
    }

    if(*start)
      *path = cpystr(start);

    fs_give((void **) &tmp);

    return(1);
}



/*
 * web_host_scan -- Scan the given line for possible web host names
 *
 * NOTE: scan below is limited to DNS names ala RFC1034
 */
char *
web_host_scan(line, len)
    char *line;
    int  *len;
{
    char *end, last = '\0';
    int   n;

    for(; *line; last = *line++)
      if((*line == 'w' || *line == 'W')
	 && (!last || !(isalnum((unsigned char) last)
			|| last == '.' || last == '-'))
	 && (((*(line + 1) == 'w' || *(line + 1) == 'W')	/* "www" */
	      && (*(line + 2) == 'w' || *(line + 2) == 'W'))
	     || ((*(line + 1) == 'e' || *(line + 1) == 'E')	/* "web." */
		 && (*(line + 2) == 'b' || *(line + 2) == 'B')
		 && *(line + 3) == '.'))){
	  end = rfc1738_scheme_part(line + 3);
	  if((*len = end - line) > ((*(line+3) == '.') ? 4 : 3)){
	      /* Dread comma exception, see note in rfc1738_scan */
	      if(strchr(",:", *(line + (*len) - 1))
		 || (*(line + (*len) - 1) == '.'
		     && (!*(line + (*len)) || *(line + (*len)) == ' ')))
		(*len)--;

	      return(line);
	  }
	  else
	    line += 3;
      }

    return(NULL);
}


/*
 * mail_addr_scan -- Scan the given line for possible RFC822 addr-spec's
 *
 * NOTE: Well, OK, not strictly addr-specs since there's alot of junk
 *	 we're tying to sift thru and we'd like to minimize false-pos
 *	 matches.
 */
char *
mail_addr_scan(line, len)
    char *line;
    int  *len;
{
    char *amp, *start, *end;
    int   n;

    /* process each : in the line */
    for(; amp = strindex(line, '@'); line = end){
	end = amp + 1;
	/* zero length addr? */
	if(amp == line || !isalnum((unsigned char) *(start = amp - 1)))
	  continue;

	/*
	 * Valid address (ala RFC822 BNF)?  First, first look to the
	 * left to make sure there are valid "scheme" chars...
	 */
	while(1)
	  /* NOTE: we're not doing quoted-strings */
	  if(!(isalnum((unsigned char) *start) || strchr(".-_+", *start))){
	      /* advance over bogus char, and erase leading punctuation */
	      for(start++;
		  *start == '.' || *start == '-' || *start == '_';
		  start++)
		;

	      break;
	  }
	  else if(start > line)
	    start--;
	  else
	    break;

	/*
	 * Make sure everyhing up to the colon is a known scheme...
	 */
	if(start && (n = amp - start) > 0){
	    /*
	     * Second, make sure that everything to the right of
	     * amp is valid for a "domain"...
	     */
	    if(*(end = amp + 1) == '['){ /* domain literal */
		int dots = 3;

		for(++end; *end ; end++)
		  if(*end == ']'){
		      if(!dots){
			  *len = end - start + 1;
			  return(start);
		      }
		      else
			break;		/* bogus */
		  }
		  else if(*end == '.'){
		      if(--dots < 0)
			break;		/* bogus */
		  }
		  else if(!isdigit((unsigned char) *end))
		    break;		/* bogus */
	    }
	    else if(isalnum((unsigned char) *end)){ /* domain name? */
		for(++end; ; end++)
		  if(!(*end && (isalnum((unsigned char) *end)
				|| *end == '-'
				|| *end == '.'
				|| *end == '_'))){
		      /* can't end with dash, dot or underscore */
		      while(!isalnum((unsigned char) *(end - 1)))
			end--;

		      *len = end - start;
		      return(start);
		  }
	    }
	}
    }

    return(NULL);
}



/*
 *  * * * * * * * *      RFC 2231 support routines      * * * * * * * *
 */


/* Useful def's */
#define	RFC2231_MAX	64


char *
rfc2231_get_param(parms, name, charset, lang)
    PARAMETER *parms;
    char      *name, **charset, **lang;
{
    char *buf, *p;
    int	  decode = 0, name_len, i, n;

    name_len = strlen(name);
    for(; parms ; parms = parms->next)
      if(!struncmp(name, parms->attribute, name_len))
	if(parms->attribute[name_len] == '*'){
	    for(p = &parms->attribute[name_len + 1], n = 0; *(p+n); n++)
	      ;

	    decode = *(p + n - 1) == '*';

	    if(isdigit((unsigned char) *p)){
		char *pieces[RFC2231_MAX];
		int   count = 0, len;

		memset(pieces, 0, RFC2231_MAX * sizeof(char *));

		while(parms){
		    n = 0;
		    do
		      n = (n * 10) + (*p - '0');
		    while(isdigit(*++p));

		    if(n < RFC2231_MAX){
			pieces[n] = parms->value;
			if(n > count)
			  count = n;
		    }
		    else
		      return(NULL);		/* Too many segments! */

		    while(parms = parms->next)
		      if(!struncmp(name, parms->attribute, name_len)){
			  if(*(p = &parms->attribute[name_len]) == '*'
			      && isdigit((unsigned char) *++p))
			    break;
			  else
			    return(NULL);	/* missing segment no.! */
		      }
		}

		for(i = len = 0; i <= count; i++)
		  if(pieces[i])
		    len += strlen(pieces[i]);
		  else
		    return(NULL);		/* hole! */

		buf = (char *) fs_get((len + 1) * sizeof(char));

		for(i = len = 0; i <= count; i++){
		    if(n = *(p = pieces[i]) == '\"') /* quoted? */
		      p++;

		    while(*p && !(n && *p == '\"' && !*(p+1)))
		      buf[len++] = *p++;
		}

		buf[len] = '\0';
	    }
	    else
	      buf = cpystr(parms->value);

	    /* Do any RFC 2231 decoding? */
	    if(decode){
		n = 0;

		if(p = strchr(buf, '\'')){
		    n = (p - buf) + 1;
		    if(charset){
			*p = '\0';
			*charset = cpystr(buf);
			*p = '\'';
		    }

		    if(p = strchr(&buf[n], '\'')){
			n = (p - buf) + 1;
			if(lang){
			    *p = '\0';
			    *lang = cpystr(p);
			    *p = '\'';
			}
		    }
		}

		if(n){
		    /* Suck out the charset & lang while decoding hex */
		    p = &buf[n];
		    for(i = 0; buf[i] = *p; i++)
		      if(*p++ == '%' && isxpair(p)){
			  buf[i] = X2C(p);
			  p += 2;
		      }
		}
		else
		  fs_give((void **) &buf);	/* problems!?! */
	    }
	    
	    return(buf);
	}
	else
	  return(cpystr(parms->value ? parms->value : ""));

    return(NULL);
}


int
rfc2231_output(so, attrib, value, specials, charset)
    STORE_S *so;
    char    *attrib, *value, *specials, *charset;
{
    int  i, line = 0, encode = 0, quote = 0;

    /*
     * scan for hibit first since encoding clue has to
     * come on first line if any parms are broken up...
     */
    for(i = 0; value && value[i]; i++)
      if(value[i] & 0x80){
	  encode++;
	  break;
      }

    for(i = 0; ; i++){
	if(!(value && value[i]) || i > 80){	/* flush! */
	    if((line++ && !so_puts(so, ";\015\012        "))
	       || !so_puts(so, attrib))
		return(0);

	    if(value){
		if(((value[i] || line > 1) /* more lines or already lines */
		    && !(so_writec('*', so)
			 && so_puts(so, int2string(line - 1))))
		   || (encode && !so_writec('*', so))
		   || !so_writec('=', so)
		   || (quote && !so_writec('\"', so))
		   || ((line == 1 && encode)
		       && !(so_puts(so, ((ps_global->VAR_CHAR_SET
					  && strucmp(ps_global->VAR_CHAR_SET,
						     "us-ascii"))
					    ? ps_global->VAR_CHAR_SET
					    : UNKNOWN_CHARSET))
			     && so_puts(so, "''"))))
		  return(0);

		while(i--){
		    if(*value & 0x80){
			char tmp[3], *p;

			p = tmp;
			C2XPAIR(*value, p);
			*p = '\0';
			if(!(so_writec('%', so) && so_puts(so, tmp)))
			  return(0);
		    }
		    else if(((*value == '\\' || *value == '\"')
			     && !so_writec('\\', so))
			    || !so_writec(*value, so))
		      return(0);

		    value++;
		}

		if(quote && !so_writec('\"', so))
		  return(0);

		if(*value)			/* more? */
		  i = quote = 0;		/* reset! */
		else
		  return(1);			/* done! */
	    }
	    else
	      return(1);
	}

	if(!quote && strchr(specials, value[i]))
	  quote++;
    }
}


PARMLIST_S *
rfc2231_newparmlist(params)
    PARAMETER *params;
{
    PARMLIST_S *p = NULL;

    if(params){
	p = (PARMLIST_S *) fs_get(sizeof(PARMLIST_S));
	memset(p, 0, sizeof(PARMLIST_S));
	p->list = params;
    }

    return(p);
}


void
rfc2231_free_parmlist(p)
    PARMLIST_S **p;
{
    if(*p){
	if((*p)->value)
	  fs_give((void **) &(*p)->value);

	mail_free_body_parameter(&(*p)->seen);
	fs_give((void **) p);
    }
}


int
rfc2231_list_params(plist)
    PARMLIST_S *plist;
{
    PARAMETER *pp, **ppp;
    int	       i;
    char      *cp;

    if(plist->value)
      fs_give((void **) &plist->value);

    for(pp = plist->list; pp; pp = pp->next)
      /* get a name */
      for(i = 0; i < 32; i++)
	if(!(plist->attrib[i] = pp->attribute[i]) ||  pp->attribute[i] == '*'){
	    plist->attrib[i] = '\0';

	    for(ppp = &plist->seen;
		*ppp && strucmp((*ppp)->attribute, plist->attrib);
		ppp = &(*ppp)->next)
	      ;

	    if(!*ppp){
		plist->list = pp->next;
		*ppp = mail_newbody_parameter();	/* add to seen list */
		(*ppp)->attribute = cpystr(plist->attrib);
		plist->value = rfc2231_get_param(pp,plist->attrib,NULL,NULL);
		return(TRUE);
	    }

	    break;
	}

    return(FALSE);
}



/*
 * These are the global pattern handles which all of the pattern routines
 * use. Once we open one of these we usually leave it open until exiting
 * pine. The _any versions are only used if we are altering our configuration,
 * the _ne (NonEmpty) versions are used routinely. We open the patterns by
 * calling either nonempty_patterns (normal use) or any_patterns (config).
 *
 * There are five different pinerc variables which contain patterns. They are
 * patterns-filters, patterns-roles, patterns-scores, patterns-indexcolors,
 * and the old "patterns". The first four are the active patterns variables
 * but the old patterns variable is kept around so that we can convert old
 * patterns to new. The reason we split it into four separate variables is
 * so that each can independently be controlled by the main pinerc or by the
 * exception pinerc.
 *
 * Each of the five variables has its own handle and status variables below.
 * That means that they operate independently.
 *
 * Looking at just a single one of those variables, it has four possible
 * values. In normal use, we use the current_val of the variable to set
 * up the patterns. We do that by calling nonempty_patterns with the
 * appropriate rflags. When editing configurations, we have the other three
 * variables to deal with: pre_user_val, main_user_val, and post_user_val.
 * We only ever deal with one of those at a time, so we re-use the variables.
 * However, we do sometimes want to deal with one of those and at the same
 * time refer to the current current_val. For example, if we are editing
 * the pre, post, or main user_val for the filters variable, we still want
 * to check for new mail. If we find new mail we'll want to call
 * process_filter_patterns which uses the current_val for filter patterns.
 * That means we have to provide for the case where we are using current_val
 * at the same time as we're using one of the user_vals. That's why we have
 * both the _ne variables (NonEmpty) and the _any variables.
 *
 * In any_patterns (and first_pattern...) use_flags may only be set to
 * one value at a time, whereas rflags may be more than one value OR'd together.
 */
PAT_HANDLE	       **cur_pat_h;
static PAT_HANDLE	*pattern_h_roles_ne,  *pattern_h_roles_any,
			*pattern_h_scores_ne, *pattern_h_scores_any,
			*pattern_h_filts_ne,  *pattern_h_filts_any,
			*pattern_h_incol_ne,  *pattern_h_incol_any,
			*pattern_h_old_ne,    *pattern_h_old_any;

/*
 * These contain the PAT_OPEN_MASK open status and the PAT_USE_MASK use status.
 */
static long		*cur_pat_status;
static long	  	 pat_status_roles_ne,  pat_status_roles_any,
			 pat_status_scores_ne, pat_status_scores_any,
			 pat_status_filts_ne,  pat_status_filts_any,
			 pat_status_incol_ne,  pat_status_incol_any,
			 pat_status_old_ne,    pat_status_old_any;

#define SET_PATTYPE(rflags)						\
    set_pathandle(rflags);						\
    cur_pat_status =							\
      ((rflags) & PAT_USE_CURRENT)					\
	? (((rflags) & ROLE_DO_INCOLS) ? &pat_status_incol_ne :		\
	    ((rflags) & ROLE_DO_FILTER) ? &pat_status_filts_ne :	\
	     ((rflags) & ROLE_DO_SCORES) ? &pat_status_scores_ne :	\
	      ((rflags) & ROLE_DO_ROLES) ?  &pat_status_roles_ne :	\
					   &pat_status_old_ne)		\
	: (((rflags) & ROLE_DO_INCOLS) ? &pat_status_incol_any :	\
	    ((rflags) & ROLE_DO_FILTER) ? &pat_status_filts_any :	\
	     ((rflags) & ROLE_DO_SCORES) ? &pat_status_scores_any :	\
	      ((rflags) & ROLE_DO_ROLES) ?  &pat_status_roles_any :	\
					   &pat_status_old_any);
#define CANONICAL_RFLAGS(rflags)	\
    ((((rflags) & (ROLE_DO_ROLES | ROLE_REPLY | ROLE_FORWARD | ROLE_COMPOSE)) \
					? ROLE_DO_ROLES  : 0) |		   \
     (((rflags) & (ROLE_DO_INCOLS | ROLE_INCOL))			   \
					? ROLE_DO_INCOLS : 0) |		   \
     (((rflags) & (ROLE_DO_SCORES | ROLE_SCORE))			   \
					? ROLE_DO_SCORES : 0) |		   \
     (((rflags) & (ROLE_DO_FILTER))					   \
					? ROLE_DO_FILTER : 0) |		   \
     (((rflags) & (ROLE_OLD_PATS))					   \
					? ROLE_OLD_PATS  : 0))

#define SETPGMSTATUS(val,yes,no)	\
    switch(val){			\
      case PAT_STAT_YES:		\
	(yes) = 1;			\
	break;				\
      case PAT_STAT_NO:			\
	(no) = 1;			\
	break;				\
      case PAT_STAT_EITHER:		\
      default:				\
        break;				\
    }

#define SET_STATUS(srchin,srchfor,assignto)				\
    {char *qq, *pp;							\
     int   ii;								\
     NAMEVAL_S *vv;						\
     if((qq = srchstr(srchin, srchfor)) != NULL){			\
	if((pp = remove_pat_escapes(qq+strlen(srchfor))) != NULL){	\
	    for(ii = 0; vv = role_status_types(ii); ii++)		\
	      if(!strucmp(pp, vv->shortname)){				\
		  assignto = vv->value;					\
		  break;						\
	      }								\
									\
	    fs_give((void **)&pp);					\
	}								\
     }									\
    }


void
set_pathandle(rflags)
    long rflags;
{
    cur_pat_h = (rflags & PAT_USE_CURRENT)
		? ((rflags & ROLE_DO_INCOLS) ? &pattern_h_incol_ne :
		    (rflags & ROLE_DO_FILTER) ? &pattern_h_filts_ne :
		     (rflags & ROLE_DO_SCORES) ? &pattern_h_scores_ne :
		      (rflags & ROLE_DO_ROLES)  ? &pattern_h_roles_ne :
					           &pattern_h_old_ne)
	        : ((rflags & ROLE_DO_INCOLS) ? &pattern_h_incol_any :
		    (rflags & ROLE_DO_FILTER) ? &pattern_h_filts_any :
		     (rflags & ROLE_DO_SCORES) ? &pattern_h_scores_any :
		      (rflags & ROLE_DO_ROLES)  ? &pattern_h_roles_any :
					           &pattern_h_old_any);
}


/*
 * Rflags may be more than one pattern type OR'd together. It also contains
 * the "use" parameter.
 */
void
open_any_patterns(rflags)
    long rflags;
{
    long canon_rflags;

    dprint(7, (debugfile, "open_any_patterns(0x%x)\n", rflags));

    canon_rflags = CANONICAL_RFLAGS(rflags);

    if(canon_rflags & ROLE_DO_INCOLS)
      sub_open_any_patterns(ROLE_DO_INCOLS | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_FILTER)
      sub_open_any_patterns(ROLE_DO_FILTER | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_SCORES)
      sub_open_any_patterns(ROLE_DO_SCORES | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_ROLES)
      sub_open_any_patterns(ROLE_DO_ROLES | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_OLD_PATS)
      sub_open_any_patterns(ROLE_OLD_PATS | (rflags & PAT_USE_MASK));
}


/*
 * This should only be called with a single pattern type (plus use flags).
 * We assume that patterns of this type are closed before this is called.
 * This always succeeds unless we run out of memory, in which case fs_get
 * never returns.
 */
void
sub_open_any_patterns(rflags)
    long rflags;
{
    PAT_S      *pat;
    PAT_LINE_S *patline = NULL, *pl = NULL;
    char      **t = NULL;
    struct variable *var;

    SET_PATTYPE(rflags);

    *cur_pat_h = (PAT_HANDLE *)fs_get(sizeof(**cur_pat_h));
    memset((void *)*cur_pat_h, 0, sizeof(**cur_pat_h));

    if(rflags & ROLE_DO_ROLES)
      var = &ps_global->vars[V_PAT_ROLES];
    else if(rflags & ROLE_DO_FILTER)
      var = &ps_global->vars[V_PAT_FILTS];
    else if(rflags & ROLE_DO_SCORES)
      var = &ps_global->vars[V_PAT_SCORES];
    else if(rflags & ROLE_DO_INCOLS)
      var = &ps_global->vars[V_PAT_INCOLS];
    else if(rflags & ROLE_OLD_PATS)
      var = &ps_global->vars[V_PATTERNS];

    switch(rflags & PAT_USE_MASK){
      case PAT_USE_CURRENT:
	t = var->current_val.l;
	break;
      case PAT_USE_MAIN:
	t = var->main_user_val.l;
	break;
      case PAT_USE_POST:
	t = var->post_user_val.l;
	break;
    }

    if(t){
	for(; t[0] && t[0][0]; t++){
	    if(*t && !strncmp("LIT:", *t, 4))
	      patline = parse_pat_lit(*t + 4);
	    else if(*t && !strncmp("FILE:", *t, 5))
	      patline = parse_pat_file(*t + 5);
	    else if(rflags & (PAT_USE_MAIN | PAT_USE_POST) &&
		    patline == NULL && *t && !strcmp(INHERIT, *t))
	      patline = parse_pat_inherit();
	    else
	      patline = NULL;

	    if(patline){
		if(pl){
		    pl->next      = patline;
		    patline->prev = pl;
		    pl = pl->next;
		}
		else{
		    (*cur_pat_h)->patlinehead = patline;
		    pl = patline;
		}
	    }
	    else
	      q_status_message1(SM_ORDER, 0, 3,
				"Invalid patterns line \"%.40s\"", *t);
	}
    }

    *cur_pat_status = PAT_OPENED | (rflags & PAT_USE_MASK);
}


void
close_every_pattern()
{
    close_patterns(ROLE_DO_INCOLS | ROLE_DO_FILTER | ROLE_DO_SCORES |
		   ROLE_DO_ROLES | ROLE_OLD_PATS | PAT_USE_CURRENT);
    /*
     * Since there is only one set of variables for the other three uses
     * we can just close any one of them. There can only be one open at
     * a time.
     */
    close_patterns(ROLE_DO_INCOLS | ROLE_DO_FILTER | ROLE_DO_SCORES |
		   ROLE_DO_ROLES | ROLE_OLD_PATS | PAT_USE_MAIN);
}


/*
 * Can be called with more than one pattern type.
 */
void
close_patterns(rflags)
    long rflags;
{
    long canon_rflags;

    dprint(7, (debugfile, "close_patterns(0x%x)\n", rflags));

    canon_rflags = CANONICAL_RFLAGS(rflags);

    if(canon_rflags & ROLE_DO_INCOLS)
      sub_close_patterns(ROLE_DO_INCOLS | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_FILTER)
      sub_close_patterns(ROLE_DO_FILTER | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_SCORES)
      sub_close_patterns(ROLE_DO_SCORES | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_DO_ROLES)
      sub_close_patterns(ROLE_DO_ROLES | (rflags & PAT_USE_MASK));
    if(canon_rflags & ROLE_OLD_PATS)
      sub_close_patterns(ROLE_OLD_PATS | (rflags & PAT_USE_MASK));
}


/*
 * Can be called with only a single pattern type.
 */
void
sub_close_patterns(rflags)
    long rflags;
{
    SET_PATTYPE(rflags);

    if(*cur_pat_h != NULL){
	free_patline(&(*cur_pat_h)->patlinehead);
	fs_give((void **)cur_pat_h);
    }

    *cur_pat_status = PAT_CLOSED;

    scores_are_used(SCOREUSE_INVALID);
}


/*
 * Can be called with more than one pattern type.
 * Nonempty always uses PAT_USE_CURRENT (the current_val).
 */
int
nonempty_patterns(rflags, pstate)
    long       rflags;
    PAT_STATE *pstate;
{
    return(any_patterns((rflags & ROLE_MASK) | PAT_USE_CURRENT, pstate));
}


/*
 * Initializes pstate and parses and sets up appropriate pattern variables.
 * May be called with more than one pattern type OR'd together in rflags.
 * Pstate will keep track of that and next_pattern et. al. will increment
 * through all of those pattern types.
 */
int
any_patterns(rflags, pstate)
    long       rflags;
    PAT_STATE *pstate;
{
    int  ret = 0;
    long canon_rflags;

    dprint(7, (debugfile, "any_patterns(0x%x)\n", rflags));

    memset((void *)pstate, 0, sizeof(*pstate));
    pstate->rflags    = rflags;

    canon_rflags = CANONICAL_RFLAGS(pstate->rflags);

    if(canon_rflags & ROLE_DO_INCOLS)
      ret += sub_any_patterns(ROLE_DO_INCOLS, pstate);
    if(canon_rflags & ROLE_DO_FILTER)
      ret += sub_any_patterns(ROLE_DO_FILTER, pstate);
    if(canon_rflags & ROLE_DO_SCORES)
      ret += sub_any_patterns(ROLE_DO_SCORES, pstate);
    if(canon_rflags & ROLE_DO_ROLES)
      ret += sub_any_patterns(ROLE_DO_ROLES, pstate);
    if(canon_rflags & ROLE_OLD_PATS)
      ret += sub_any_patterns(ROLE_OLD_PATS, pstate);

    return(ret);
}


int
sub_any_patterns(rflags, pstate)
    long       rflags;
    PAT_STATE *pstate;
{
    SET_PATTYPE(rflags | (pstate->rflags & PAT_USE_MASK));

    if(*cur_pat_h &&
       (((pstate->rflags & PAT_USE_MASK) == PAT_USE_CURRENT &&
	 (*cur_pat_status & PAT_USE_MASK) != PAT_USE_CURRENT) ||
        ((pstate->rflags & PAT_USE_MASK) != PAT_USE_CURRENT &&
         ((*cur_pat_status & PAT_OPEN_MASK) != PAT_OPENED ||
	  (*cur_pat_status & PAT_USE_MASK) !=
	   (pstate->rflags & PAT_USE_MASK)))))
      close_patterns(rflags | (pstate->rflags & PAT_USE_MASK));
    
    /* open_any always succeeds */
    if(!*cur_pat_h && ((*cur_pat_status & PAT_OPEN_MASK) == PAT_CLOSED))
      open_any_patterns(rflags | (pstate->rflags & PAT_USE_MASK));
    
    if(!*cur_pat_h){		/* impossible */
	*cur_pat_status = PAT_CLOSED;
	return(0);
    }

    /*
     * Opening nonempty can fail. That just means there aren't any
     * patterns of that type.
     */
    if((pstate->rflags & PAT_USE_MASK) == PAT_USE_CURRENT &&
       !(*cur_pat_h)->patlinehead)
      *cur_pat_status = (PAT_OPEN_FAILED | PAT_USE_CURRENT);
       
    return(((*cur_pat_status & PAT_OPEN_MASK) == PAT_OPENED) ? 1 : 0);
}


PAT_LINE_S *
parse_pat_lit(litpat)
    char *litpat;
{
    PAT_LINE_S *patline;
    PAT_S      *pat;

    patline = (PAT_LINE_S *)fs_get(sizeof(*patline));
    memset((void *)patline, 0, sizeof(*patline));
    patline->type = Literal;


    if((pat = parse_pat(litpat)) != NULL){
	pat->patline   = patline;
	patline->first = pat;
	patline->last  = pat;
    }

    return(patline);
}


/*
 * This always returns a patline even if we can't read the file. The patline
 * returned will say readonly in the worst case and there will be no patterns.
 * If the file doesn't exist, this creates it if possible.
 */
PAT_LINE_S *
parse_pat_file(filename)
    char *filename;
{
#define BUF_SIZE 5000
    PAT_LINE_S *patline;
    PAT_S      *pat, *p;
    char        path[MAXPATH+1], buf[BUF_SIZE];
    char       *dir, *q;
    FILE       *fp;
    int         ok = 0, some_pats = 0;
    struct variable *vars = ps_global->vars;

    signature_path(filename, path, MAXPATH);

    if(VAR_OPER_DIR && !in_dir(VAR_OPER_DIR, path)){
	q_status_message1(SM_ORDER | SM_DING, 3, 4,
			  "Can't use Roles file outside of %s", VAR_OPER_DIR);
	return(NULL);
    }

    patline = (PAT_LINE_S *)fs_get(sizeof(*patline));
    memset((void *)patline, 0, sizeof(*patline));
    patline->type     = File;
    patline->filename = cpystr(filename);
    patline->filepath = cpystr(path);

    if(q = last_cmpnt(path)){
	int save;

	save = *--q;
	*q = '\0';
	dir  = cpystr(*path ? path : "/");
	*q = save;
    }
    else
      dir = cpystr(".");

#if	defined(DOS) || defined(OS2)
    /*
     * If the dir has become a drive letter and : (e.g. "c:")
     * then append a "\".  The library function access() in the
     * win 16 version of MSC seems to require this.
     */
    if(isalpha((unsigned char) *dir)
       && *(dir+1) == ':' && *(dir+2) == '\0'){
	*(dir+2) = '\\';
	*(dir+3) = '\0';
    }
#endif	/* DOS || OS2 */

    /*
     * Even if we can edit the file itself, we aren't going
     * to be able to change it unless we can also write in
     * the directory that contains it (because we write into a
     * temp file and then rename).
     */
    if(can_access(dir, EDIT_ACCESS) != 0)
      patline->readonly = 1;

    if(can_access(path, EDIT_ACCESS) == 0){
	if(patline->readonly)
	  q_status_message1(SM_ORDER, 0, 3,
			    "Pattern file directory (%s) is ReadOnly", dir);
    }
    else if(can_access(path, READ_ACCESS) == 0)
      patline->readonly = 1;

    if(can_access(path, ACCESS_EXISTS) == 0){
	if((fp = fopen(path, "r")) != NULL){
	    /* Check to see if this is a valid patterns file */
	    if(fp_file_size(fp) <= 0L)
	      ok++;
	    else{
		size_t len;

		len = strlen(PATTERN_MAGIC);
	        if(fread(buf, sizeof(char), len+3, fp) == len+3){
		    buf[len+3] = '\0';
		    buf[len] = '\0';
		    if(strcmp(buf, PATTERN_MAGIC) == 0){
			if(atoi(PATTERN_FILE_VERS) < atoi(buf + len + 1))
			  q_status_message1(SM_ORDER, 0, 4,
      "Pattern file \"%s\" is made by newer Pine, will try to use it anyway",
					    filename);

			ok++;
			some_pats++;
			/* toss rest of first line */
			(void)fgets(buf, BUF_SIZE, fp);
		    }
		}
	    }
		
	    if(!ok){
		patline->readonly = 1;
		q_status_message1(SM_ORDER | SM_DING, 3, 4,
				  "\"%s\" is not a Pattern file", path);
	    }

	    p = NULL;
	    while(some_pats && fgets(buf, BUF_SIZE, fp) != NULL){
		if((pat = parse_pat(buf)) != NULL){
		    pat->patline = patline;
		    if(!patline->first)
		      patline->first = pat;

		    patline->last  = pat;

		    if(p){
			p->next   = pat;
			pat->prev = p;
			p = p->next;
		    }
		    else
		      p = pat;
		}
	    }

	    (void)fclose(fp);
	}
	else{
	    patline->readonly = 1;
	    q_status_message2(SM_ORDER | SM_DING, 3, 4,
			      "Error \"%s\" reading pattern file \"%s\"",
			      error_description(errno), path);
	}
    }
    else{		/* doesn't exist yet, try to create it */
	if(patline->readonly)
	  q_status_message1(SM_ORDER, 0, 3,
			    "Pattern file directory (%s) is ReadOnly", dir);
	else{
	    /*
	     * We try to create it by making up an empty patline and calling
	     * write_pattern_file.
	     */
	    patline->dirty = 1;
	    if(write_pattern_file(NULL, patline) != 0){
		patline->readonly = 1;
		patline->dirty = 0;
		q_status_message1(SM_ORDER | SM_DING, 3, 4,
				  "Error creating pattern file \"%s\"",
				  path);
	    }
	}
    }

    if(dir)
      fs_give((void **)&dir);

    return(patline);
}


PAT_LINE_S *
parse_pat_inherit()
{
    PAT_LINE_S *patline;
    PAT_S      *pat;

    patline = (PAT_LINE_S *)fs_get(sizeof(*patline));
    memset((void *)patline, 0, sizeof(*patline));
    patline->type = Inherit;

    pat = (PAT_S *)fs_get(sizeof(*pat));
    memset((void *)pat, 0, sizeof(*pat));
    pat->inherit = 1;

    pat->patline = patline;
    patline->first = pat;
    patline->last  = pat;

    return(patline);
}


PAT_S *
parse_pat(str)
    char *str;
{
    PAT_S *pat = NULL;
    char  *p, *q, *astr, *pstr;
    int    i;
    NAMEVAL_S *v;
#define PTRN "pattern="
#define PTRNLEN 8
#define ACTN "action="
#define ACTNLEN 7

    if(str)
      removing_trailing_white_space(str);

    if(!str || !*str || *str == '#')
      return(pat);

    pat = (PAT_S *)fs_get(sizeof(*pat));
    memset((void *)pat, 0, sizeof(*pat));

    if((p = srchstr(str, PTRN)) != NULL){
	pat->patgrp = (PATGRP_S *)fs_get(sizeof(*pat->patgrp));
	memset((void *)pat->patgrp, 0, sizeof(*pat->patgrp));

	if((pstr = copy_quoted_string_asis(p+PTRNLEN)) != NULL){
	    /* get the nickname (we always force a nickname) */
	    if((q = srchstr(pstr, "/NICK=")) != NULL)
	      pat->patgrp->nick = remove_pat_escapes(q+6);
	    else
	      pat->patgrp->nick = cpystr("Alternate Role");

	    pat->patgrp->to      = parse_pattern("/TO=",     pstr, 1);
	    pat->patgrp->cc      = parse_pattern("/CC=",     pstr, 1);
	    pat->patgrp->recip   = parse_pattern("/RECIP=",  pstr, 1);
	    pat->patgrp->partic  = parse_pattern("/PARTIC=", pstr, 1);
	    pat->patgrp->from    = parse_pattern("/FROM=",   pstr, 1);
	    pat->patgrp->sender  = parse_pattern("/SENDER=", pstr, 1);
	    pat->patgrp->news    = parse_pattern("/NEWS=",   pstr, 1);
	    pat->patgrp->subj    = parse_pattern("/SUBJ=",   pstr, 1);
	    pat->patgrp->alltext = parse_pattern("/ALL=",    pstr, 1);

	    pat->patgrp->arbhdr = parse_arbhdr(pstr);

	    if((q = srchstr(pstr, "/SCOREI=")) != NULL){
		if((p = remove_pat_escapes(q+8)) != NULL){
		    int left, right;

		    if(parse_score_interval(p, &left, &right)){
			pat->patgrp->do_score  = 1;
			pat->patgrp->score_min = left;
			pat->patgrp->score_max = right;
		    }

		    fs_give((void **)&p);
		}
	    }

	    /* folder type */
	    pat->patgrp->fldr_type = FLDR_DEFL;
	    if((q = srchstr(pstr, "/FLDTYPE=")) != NULL){
		if((p = remove_pat_escapes(q+9)) != NULL){
		    for(i = 0; v = pat_fldr_types(i); i++)
		      if(!strucmp(p, v->shortname)){
			  pat->patgrp->fldr_type = v->value;
			  break;
		      }

		    fs_give((void **)&p);
		}
	    }

	    pat->patgrp->folder = parse_pattern("/FOLDER=", pstr, 1);

	    SET_STATUS(pstr,"/STATN=",pat->patgrp->stat_new);
	    SET_STATUS(pstr,"/STATI=",pat->patgrp->stat_imp);
	    SET_STATUS(pstr,"/STATA=",pat->patgrp->stat_ans);
	    SET_STATUS(pstr,"/STATD=",pat->patgrp->stat_del);

	    fs_give((void **)&pstr);
	}
    }

    if((p = srchstr(str, ACTN)) != NULL){
	pat->action = (ACTION_S *)fs_get(sizeof(*pat->action));
	memset((void *)pat->action, 0, sizeof(*pat->action));

	if((astr = copy_quoted_string_asis(p+ACTNLEN)) != NULL){
	    ACTION_S *action;

	    action = pat->action;
	    memset((void *)action, 0, sizeof(*action));

	    action->nick = cpystr((pat->patgrp->nick && pat->patgrp->nick[0])
				   ? pat->patgrp->nick : "Alternate Role");

	    if(srchstr(astr, "/ROLE=1"))
	      action->is_a_role = 1;

	    if(srchstr(astr, "/ISINCOL=1"))
	      action->is_a_incol = 1;

	    if(srchstr(astr, "/ISSCORE=1"))
	      action->is_a_score = 1;

	    /* get the score associated with this pattern */
	    if(action->is_a_score && (q = srchstr(astr, "/SCORE=")) != NULL){
		if((p = remove_pat_escapes(q+7)) != NULL){
		    i = atoi(p);
		    if(i >= SCORE_MIN && i <= SCORE_MAX)
		      action->scoreval = i;

		    fs_give((void **)&p);
		}
	    }

	    if(srchstr(astr, "/FILTER=1")){
		action->is_a_filter = 1;
		action->folder	    = parse_pattern("/FOLDER=", astr, 1);
		action->move_only_if_not_deleted =
		    (action->folder && srchstr(astr, "/NOTDEL=1")) ? 1 : 0;
	    }

	    if(action->is_a_role){
		/* reply type */
		action->repl_type = ROLE_REPL_DEFL;
		if((q = srchstr(astr, "/RTYPE=")) != NULL){
		    if((p = remove_pat_escapes(q+7)) != NULL){
			for(i = 0; v = role_repl_types(i); i++)
			  if(!strucmp(p, v->shortname)){
			      action->repl_type = v->value;
			      break;
			  }

			fs_give((void **)&p);
		    }
		}

		/* forward type */
		action->forw_type = ROLE_FORW_DEFL;
		if((q = srchstr(astr, "/FTYPE=")) != NULL){
		    if((p = remove_pat_escapes(q+7)) != NULL){
			for(i = 0; v = role_forw_types(i); i++)
			  if(!strucmp(p, v->shortname)){
			      action->forw_type = v->value;
			      break;
			  }

			fs_give((void **)&p);
		    }
		}

		/* compose type */
		action->comp_type = ROLE_COMP_DEFL;
		if((q = srchstr(astr, "/CTYPE=")) != NULL){
		    if((p = remove_pat_escapes(q+7)) != NULL){
			for(i = 0; v = role_comp_types(i); i++)
			  if(!strucmp(p, v->shortname)){
			      action->comp_type = v->value;
			      break;
			  }

			fs_give((void **)&p);
		    }
		}

		/* get the from */
		if((q = srchstr(astr, "/FROM=")) != NULL){
		    if((p = remove_pat_escapes(q+6)) != NULL){
			rfc822_parse_adrlist(&action->from, p,
					     ps_global->maildomain);
			fs_give((void **)&p);
		    }
		}

		/* get the reply-to */
		if((q = srchstr(astr, "/REPL=")) != NULL){
		    if((p = remove_pat_escapes(q+6)) != NULL){
			rfc822_parse_adrlist(&action->replyto, p,
					     ps_global->maildomain);
			fs_give((void **)&p);
		    }
		}

		/* get the fcc */
		if((q = srchstr(astr, "/FCC=")) != NULL)
		  action->fcc = remove_pat_escapes(q+5);

		/* get the literal sig */
		if((q = srchstr(astr, "/LSIG=")) != NULL)
		  action->litsig = remove_pat_escapes(q+6);

		/* get the sig file */
		if((q = srchstr(astr, "/SIG=")) != NULL)
		  action->sig = remove_pat_escapes(q+5);

		/* get the template file */
		if((q = srchstr(astr, "/TEMPLATE=")) != NULL)
		  action->template = remove_pat_escapes(q+10);

		/* get the custom headers */
		if((q = srchstr(astr, "/CSTM=")) != NULL){
		    if((p = remove_pat_escapes(q+6)) != NULL){
			char *list;
			int   commas = 0;

			/* count elements in list */
			for(q = p; q && *q; q++)
			  if(*q == ',')
			    commas++;

			action->cstm = parse_list(p, commas+1, NULL);
			fs_give((void **)&p);
		    }
		}

		/* get the inherit nick */
		if((q = srchstr(astr, "/INICK=")) != NULL)
		  action->inherit_nick = remove_pat_escapes(q+7);
	    }
	    else{
		action->repl_type = ROLE_NOTAROLE_DEFL;
		action->forw_type = ROLE_NOTAROLE_DEFL;
		action->comp_type = ROLE_NOTAROLE_DEFL;
	    }


	    /* get the index color */
	    if(action->is_a_incol && (q = srchstr(astr, "/INCOL=")) != NULL){
		if((p = remove_pat_escapes(q+7)) != NULL){
		    char *fg = NULL, *bg = NULL, *z;
		    /*
		     * Color should look like
		     * /FG=white/BG=red
		     */
		    if((z = srchstr(p, "/FG=")) != NULL)
		      fg = remove_pat_escapes(z+4);
		    if((z = srchstr(p, "/BG=")) != NULL)
		      bg = remove_pat_escapes(z+4);

		    if(fg && *fg && bg && *bg)
		      action->incol = new_color_pair(fg, bg);

		    if(fg)
		      fs_give((void **)&fg);
		    if(bg)
		      fs_give((void **)&bg);
		    fs_give((void **)&p);
		}
	    }

	    fs_give((void **)&astr);
	}
    }
    
    return(pat);
}


/*
 * Str looks like (min,max), left and right are return values.
 *
 * Parens are optional, whitespace is ignored.
 * If min is left out it is -INF. If max is left out it is INF.
 * If only one number and no comma number is min and max is INF.
 *
 * Returns 1 if ok, 0 if undefined.
 */
int
parse_score_interval(str, left, right)
    char *str;
    int  *left;
    int  *right;
{
    char *q;
    int   ret = 0;

    if(!str || !left || !right)
      return(ret);

    *left = *right = SCORE_UNDEF;
    q = str;

    /* skip to first number */
    while(isspace((unsigned char)*q) || *q == LPAREN)
      q++;
    
    /* min number */
    if(*q == COMMA || !struncmp(q, "-INF", 4))
      *left = - SCORE_INF;
    else if(*q == '-' || isdigit((unsigned char)*q))
      *left = atoi(q);
    /* else still UNDEF */

    if(*left != SCORE_UNDEF){
	/* skip to second number */
	while(*q && *q != COMMA && *q != RPAREN)
	  q++;
	if(*q == COMMA)
	  q++;
	while(isspace((unsigned char)*q))
	  q++;

	/* max number */
	if(*q == '\0' || *q == RPAREN || !struncmp(q, "INF", 3))
	  *right = SCORE_INF;
	else if(*q == '-' || isdigit((unsigned char)*q))
	  *right = atoi(q);
    }
    
    if(*left == SCORE_UNDEF || *right == SCORE_UNDEF)
      q_status_message1(SM_ORDER, 3, 5,
		    "Error: Score Interval \"%s\" (syntax is (min,max)",
			str);
    else if(*left > *right)
      q_status_message1(SM_ORDER, 3, 5,
			"Error: Score Interval \"%s\", min > max", str);
    else
      ret++;

    return(ret);
}


/*
 * Returns string that looks like "(left,right)".
 * Caller is responsible for freeing memory.
 */
char *
stringform_of_score_interval(left, right)
    int left, right;
{
    char *res = NULL;

    if(left != SCORE_UNDEF && right != SCORE_UNDEF && left <= right){
	char lbuf[20], rbuf[20];

	if(left == - SCORE_INF)
	  strcpy(lbuf, "-INF");
	else
	  sprintf(lbuf, "%d", left);

	if(right == SCORE_INF)
	  strcpy(rbuf, "INF");
	else
	  sprintf(rbuf, "%d", right);

	res = fs_get((strlen(lbuf) + strlen(rbuf) + 4) * sizeof(char));
	sprintf(res, "(%s,%s)", lbuf, rbuf);
    }
    
    return(res);
}


/*
 * Args -- flags  - SCOREUSE_INVALID  Mark scores_in_use invalid so that we'll
 *					recalculate if we want to use it again.
 *		  - SCOREUSE_GET      Return whether scores are being used or not.
 *
 * Returns -- 0 - Scores not being used at all.
 *	     >0 - Scores are used. The return value consists of flag values
 *		    OR'd together. Possible values are:
 * 
 *			SCOREUSE_INCOLS  - scores needed for index line colors
 *			SCOREUSE_ROLES   - scores needed for roles
 *			SCOREUSE_FILTERS - scores needed for filters
 */
int
scores_are_used(flags)
    int flags;
{
    static int  scores_in_use = -1;
    long        type1, type2;
    PAT_STATE   pstate1, pstate2;

    if(flags & SCOREUSE_INVALID) /* mark invalid so we recalculate next time */
      scores_in_use = -1;
    else if(scores_in_use == -1){

	/*
	 * Check the patterns to see if scores are potentially
	 * being used.
	 * The first_pattern() in the if checks whether there are any
	 * non-zero scorevals. The loop checks whether any patterns
	 * use those non-zero scorevals.
	 */
	type1 = ROLE_SCORE;
	type2 = (ROLE_REPLY | ROLE_FORWARD | ROLE_COMPOSE |
		 ROLE_INCOL | ROLE_DO_FILTER);
	if(nonempty_patterns(type1, &pstate1) && first_pattern(&pstate1) &&
	   nonempty_patterns(type2, &pstate2) && first_pattern(&pstate2)){
	    PAT_S *pat;

	    /*
	     * Careful. nonempty_patterns() may call close_pattern()
	     * which will set scores_in_use to -1! So we have to be
	     * sure to reset it after we call nonempty_patterns().
	     */
	    scores_in_use = 0;

	    for(pat = first_pattern(&pstate2);
		pat;
		pat = next_pattern(&pstate2))
	      if(pat->patgrp && pat->patgrp->do_score){
		  if(pat->action && pat->action->is_a_incol)
		    scores_in_use |= SCOREUSE_INCOLS;
		  if(pat->action && pat->action->is_a_role)
		    scores_in_use |= SCOREUSE_ROLES;
		  if(pat->action && pat->action->is_a_filter)
		    scores_in_use |= SCOREUSE_FILTERS;
	      }
	}
	else
	  scores_in_use = 0;
    }

    return((scores_in_use == -1) ? 0 : scores_in_use);
}


/*
 * Look for label in str and return a pointer to parsed string.
 * Converts from string from patterns file which looks like
 *       /NEWS=comp.mail.,comp.mail.pine/TO=...
 * This is the string that came from pattern="string" with the pattern=
 * and outer quotes removed.
 * This converts the string to a PATTERN_S list and returns
 * an allocated copy.
 */
PATTERN_S *
parse_pattern(label, str, hex_to_backslashed)
    char *label;
    char *str;
    int   hex_to_backslashed;
{
    char      *q, *labeled_str;
    PATTERN_S *head = NULL;

    if(!label || !str)
      return(NULL);

    if((q = srchstr(str, label)) != NULL){
	if((labeled_str = (hex_to_backslashed
		? remove_pat_escapes(q+strlen(label))
		: remove_backslash_escapes(q+strlen(label)))) != NULL){
	    head = string_to_pattern(labeled_str);
	    fs_give((void **)&labeled_str);
	}
    }

    return(head);
}


/*
 * Look for /ARB's in str and return a pointer to parsed ARBHDR_S.
 * Converts from string from patterns file which looks like
 *       /ARB<fieldname1>=pattern/.../ARB<fieldname2>=pattern...
 * This is the string that came from pattern="string" with the pattern=
 * and outer quotes removed.
 * This converts the string to a ARBHDR_S list and returns
 * an allocated copy.
 */
ARBHDR_S *
parse_arbhdr(str)
    char *str;
{
    char      *q, *qq, *s, *equals, *noesc;
    int        empty, skip;
    ARBHDR_S  *ahdr = NULL, *a, *aa;
    PATTERN_S *p = NULL;

    if(!str)
      return(NULL);

    aa = NULL;
    s = str;
    for(s = str;
	(q = (qq = srchstr(s, "/ARB")) ? qq : (srchstr(s, "/EARB")));
	s = q+1){
	empty = (q[1] == 'E') ? 1 : 0;
	skip = empty ? 5 : 4;
	if((noesc = remove_pat_escapes(q+skip)) != NULL){
	    if(*noesc != '=' && (equals = strindex(noesc, '=')) != NULL){
		a = (ARBHDR_S *)fs_get(sizeof(*a));
		memset((void *)a, 0, sizeof(*a));
		*equals = '\0';
		a->isemptyval = empty;
		a->field = cpystr(noesc);
		if(empty)
		  a->p     = string_to_pattern("");
		else if(*(equals+1) &&
			(p = string_to_pattern(equals+1)) != NULL)
		  a->p     = p;

		/* keep them in the same order */
		if(aa){
		    aa->next = a;
		    aa = aa->next;
		}
		else{
		    ahdr = a;
		    aa = ahdr;
		}
	    }

	    fs_give((void **)&noesc);
	}
    }

    return(ahdr);
}


/*
 * Converts a string to a PATTERN_S list and returns an
 * allocated copy. The source string looks like
 *        string1,string2,...
 * Commas and backslashes may be backslash-escaped in the original string
 * in order to include actual commas and backslashes in the pattern.
 * So \, is an actual comma and , is the separator character.
 * The string is the form edited by the user.
 */
PATTERN_S *
string_to_pattern(str)
    char *str;
{
    char      *q, *s, *workspace;
    PATTERN_S *p, *head = NULL, **nextp;

    if(!str)
      return(head);
    
    /*
     * We want an empty string to cause an empty substring in the pattern
     * instead of returning a NULL pattern. That can be used as a way to
     * match any header. For example, if all the patterns but the news
     * pattern were null and the news pattern was a substring of "" then
     * we use that to match any message with a newsgroups header.
     */
    if(!*str){
	head = (PATTERN_S *)fs_get(sizeof(*p));
	memset((void *)head, 0, sizeof(*head));
	head->substring = cpystr("");
    }
    else{
	nextp = &head;
	workspace = (char *)fs_get((strlen(str)+1) * sizeof(char));
	s = workspace;
	*s = '\0';
	q = str;
	do {
	    switch(*q){
	      case COMMA:
	      case '\0':
		*s = '\0';
		removing_leading_and_trailing_white_space(workspace);
		p = (PATTERN_S *)fs_get(sizeof(*p));
		memset((void *)p, 0, sizeof(*p));
		p->substring = cpystr(workspace);
		*nextp = p;
		nextp = &p->next;
		s = workspace;
		*s = '\0';
		break;
		
	      case BSLASH:
		if(*(q+1) == COMMA)
		  *s++ = *(++q);
		else
		  *s++ = *q;

		break;

	      default:
		*s++ = *q;
		break;
	    }
	} while(*q++);

	fs_give((void **)&workspace);
    }

    return(head);
}

    
/*
 * Converts a PATTERN_S list to a string.
 * The resulting string is allocated here and looks like
 *        string1,string2,...
 * Commas and backslashes in the original pattern
 * end up backslash-escaped in the string.
 * This string is what the user sees and edits.
 */
char *
pattern_to_string(pattern)
    PATTERN_S *pattern;
{
    PATTERN_S *p;
    char      *result = NULL, *q, *s;
    size_t     n;

    if(!pattern)
      return(result);

    /* how much space is needed? */
    n = 0;
    for(p = pattern; p; p = p->next){
	n += (p == pattern) ? 0 : 1;
	for(s = p->substring; s && *s; s++){
	    if(*s == COMMA)
	      n++;

	    n++;
	}
    }

    q = result = (char *)fs_get(++n);
    for(p = pattern; p; p = p->next){
	if(p != pattern)
	  *q++ = COMMA;

	for(s = p->substring; s && *s; s++){
	    if(*s == COMMA)
	      *q++ = '\\';

	    *q++ = *s;
	}
    }

    *q = '\0';

    return(result);
}


/*
 * Must be called with a pstate, we don't check for it.
 * It respects the cur_rflag_num in pstate. That is, it doesn't start over
 * at i=1, it starts at cur_rflag_num.
 */
PAT_S *
first_any_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_LINE_S *patline = NULL;
    int         i;
    long        local_rflag;

    /*
     * The rest of pstate should be set before coming here.
     * In particular, the rflags should be set by a call to nonempty_patterns
     * or any_patterns, and cur_rflag_num should be set.
     */
    pstate->patlinecurrent = NULL;
    pstate->patcurrent     = NULL;

    /*
     * The order of these is important. It is the same as the order
     * used for next_any_pattern and opposite of the order used by
     * last and prev. For next_any's benefit, we allow cur_rflag_num to
     * start us out past the first set.
     */
    for(i = pstate->cur_rflag_num; i <= 5; i++){

	local_rflag = 0L;

	switch(i){
	  case 1:
	    local_rflag = ROLE_DO_INCOLS & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 2:
	    local_rflag = ROLE_DO_FILTER & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 3:
	    local_rflag = ROLE_DO_SCORES & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 4:
	    local_rflag = ROLE_DO_ROLES & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 5:
	    local_rflag = ROLE_OLD_PATS & CANONICAL_RFLAGS(pstate->rflags);
	    break;
	}

	if(local_rflag){
	    SET_PATTYPE(local_rflag | (pstate->rflags & PAT_USE_MASK));

	    if(*cur_pat_h){
		/* Find first patline with a pat */
		for(patline = (*cur_pat_h)->patlinehead;
		    patline && !patline->first;
		    patline = patline->next)
		  ;
	    }

	    if(patline){
		pstate->cur_rflag_num  = i;
		pstate->patlinecurrent = patline;
		pstate->patcurrent     = patline->first;
	    }
	}

	if(pstate->patcurrent)
	  break;
    }

    return(pstate->patcurrent);
}


/*
 * Return first pattern of the specified types. These types were set by a
 * previous call to any_patterns or nonempty_patterns.
 *
 * Args --  pstate  pattern state. This is set here and passed back for
 *                  use by next_pattern. Must be non-null.
 *                  It must have been initialized previously by a call to
 *                  nonempty_patterns or any_patterns.
 */
PAT_S *
first_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_S           *pat;
    struct variable *vars = ps_global->vars;
    long             rflags;

    pstate->cur_rflag_num = 1;

    rflags = pstate->rflags;

    for(pat = first_any_pattern(pstate);
	pat && !((pat->action &&
		  ((rflags & ROLE_DO_ROLES && pat->action->is_a_role) ||
	           (rflags & ROLE_DO_INCOLS && pat->action->is_a_incol) ||
	           (rflags & ROLE_DO_SCORES && pat->action->is_a_score) ||
		   (rflags & ROLE_SCORE && pat->action->scoreval) ||
		   (rflags & ROLE_DO_FILTER && pat->action->is_a_filter) ||
	           (rflags & ROLE_REPLY &&
		    (pat->action->repl_type == ROLE_REPL_YES ||
		     pat->action->repl_type == ROLE_REPL_NOCONF)) ||
	           (rflags & ROLE_FORWARD &&
		    (pat->action->forw_type == ROLE_FORW_YES ||
		     pat->action->forw_type == ROLE_FORW_NOCONF)) ||
	           (rflags & ROLE_COMPOSE &&
		    (pat->action->comp_type == ROLE_COMP_YES ||
		     pat->action->comp_type == ROLE_COMP_NOCONF)) ||
		   (rflags & ROLE_INCOL && pat->action->incol &&
		    (!VAR_NORM_FORE_COLOR || !VAR_NORM_BACK_COLOR ||
		     !pat->action->incol->fg ||
		     !pat->action->incol->bg ||
		     strucmp(VAR_NORM_FORE_COLOR,pat->action->incol->fg) ||
		     strucmp(VAR_NORM_BACK_COLOR,pat->action->incol->bg))) ||
	           (rflags & ROLE_OLD_PATS)))
		||
		 pat->inherit);
	pat = next_any_pattern(pstate))
      ;
    
    return(pat);
}


/*
 * Just like first_any_pattern.
 */
PAT_S *
last_any_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_LINE_S *patline = NULL;
    int         i;
    long        local_rflag;

    /*
     * The rest of pstate should be set before coming here.
     * In particular, the rflags should be set by a call to nonempty_patterns
     * or any_patterns, and cur_rflag_num should be set.
     */
    pstate->patlinecurrent = NULL;
    pstate->patcurrent     = NULL;

    for(i = pstate->cur_rflag_num; i >= 1; i--){

	local_rflag = 0L;

	switch(i){
	  case 1:
	    local_rflag = ROLE_DO_INCOLS & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 2:
	    local_rflag = ROLE_DO_FILTER & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 3:
	    local_rflag = ROLE_DO_SCORES & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 4:
	    local_rflag = ROLE_DO_ROLES & CANONICAL_RFLAGS(pstate->rflags);
	    break;

	  case 5:
	    local_rflag = ROLE_OLD_PATS & CANONICAL_RFLAGS(pstate->rflags);
	    break;
	}

	if(local_rflag){
	    SET_PATTYPE(local_rflag | (pstate->rflags & PAT_USE_MASK));

	    pstate->patlinecurrent = NULL;
	    pstate->patcurrent     = NULL;

	    if(*cur_pat_h){
		/* Find last patline with a pat */
		for(patline = (*cur_pat_h)->patlinehead;
		    patline;
		    patline = patline->next)
		  if(patline->last)
		    pstate->patlinecurrent = patline;
		
		if(pstate->patlinecurrent)
		  pstate->patcurrent = pstate->patlinecurrent->last;
	    }

	    if(pstate->patcurrent)
	      pstate->cur_rflag_num = i;

	    if(pstate->patcurrent)
	      break;
	}
    }

    return(pstate->patcurrent);
}


/*
 * Return last pattern of the specified types. These types were set by a
 * previous call to any_patterns or nonempty_patterns.
 *
 * Args --  pstate  pattern state. This is set here and passed back for
 *                  use by prev_pattern. Must be non-null.
 *                  It must have been initialized previously by a call to
 *                  nonempty_patterns or any_patterns.
 */
PAT_S *
last_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_S           *pat;
    struct variable *vars = ps_global->vars;
    long             rflags;

    pstate->cur_rflag_num = 5;

    rflags = pstate->rflags;

    for(pat = last_any_pattern(pstate);
	pat && !((pat->action &&
		  ((rflags & ROLE_DO_ROLES && pat->action->is_a_role) ||
	           (rflags & ROLE_DO_INCOLS && pat->action->is_a_incol) ||
	           (rflags & ROLE_DO_SCORES && pat->action->is_a_score) ||
		   (rflags & ROLE_SCORE && pat->action->scoreval) ||
		   (rflags & ROLE_DO_FILTER && pat->action->is_a_filter) ||
	           (rflags & ROLE_REPLY &&
		    (pat->action->repl_type == ROLE_REPL_YES ||
		     pat->action->repl_type == ROLE_REPL_NOCONF)) ||
	           (rflags & ROLE_FORWARD &&
		    (pat->action->forw_type == ROLE_FORW_YES ||
		     pat->action->forw_type == ROLE_FORW_NOCONF)) ||
	           (rflags & ROLE_COMPOSE &&
		    (pat->action->comp_type == ROLE_COMP_YES ||
		     pat->action->comp_type == ROLE_COMP_NOCONF)) ||
		   (rflags & ROLE_INCOL && pat->action->incol &&
		    (!VAR_NORM_FORE_COLOR || !VAR_NORM_BACK_COLOR ||
		     !pat->action->incol->fg ||
		     !pat->action->incol->bg ||
		     strucmp(VAR_NORM_FORE_COLOR,pat->action->incol->fg) ||
		     strucmp(VAR_NORM_BACK_COLOR,pat->action->incol->bg))) ||
	           (rflags & ROLE_OLD_PATS)))
		||
		 pat->inherit);
	pat = prev_any_pattern(pstate))
      ;
    
    return(pat);
}

    
/*
 * This assumes that pstate is valid.
 */
PAT_S *
next_any_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_LINE_S *patline;

    if(pstate->patlinecurrent){
	if(pstate->patcurrent && pstate->patcurrent->next)
	  pstate->patcurrent = pstate->patcurrent->next;
	else{
	    /* Find next patline with a pat */
	    for(patline = pstate->patlinecurrent->next;
		patline && !patline->first;
		patline = patline->next)
	      ;
	    
	    if(patline){
		pstate->patlinecurrent = patline;
		pstate->patcurrent     = patline->first;
	    }
	    else{
		pstate->patlinecurrent = NULL;
		pstate->patcurrent     = NULL;
	    }
	}
    }

    /* we've reached the last, try the next rflag_num (the next pattern type) */
    if(!pstate->patcurrent){
	pstate->cur_rflag_num++;
	pstate->patcurrent = first_any_pattern(pstate);
    }

    return(pstate->patcurrent);
}


/*
 * Return next pattern of the specified types. These types were set by a
 * previous call to any_patterns or nonempty_patterns.
 *
 * Args -- pstate  pattern state. This is set by first_pattern or last_pattern.
 */
PAT_S *
next_pattern(pstate)
    PAT_STATE  *pstate;
{
    PAT_S           *pat;
    struct variable *vars = ps_global->vars;
    long             rflags;

    rflags = pstate->rflags;

    for(pat = next_any_pattern(pstate);
	pat && !((pat->action &&
		  ((rflags & ROLE_DO_ROLES && pat->action->is_a_role) ||
	           (rflags & ROLE_DO_INCOLS && pat->action->is_a_incol) ||
	           (rflags & ROLE_DO_SCORES && pat->action->is_a_score) ||
		   (rflags & ROLE_SCORE && pat->action->scoreval) ||
		   (rflags & ROLE_DO_FILTER && pat->action->is_a_filter) ||
	           (rflags & ROLE_REPLY &&
		    (pat->action->repl_type == ROLE_REPL_YES ||
		     pat->action->repl_type == ROLE_REPL_NOCONF)) ||
	           (rflags & ROLE_FORWARD &&
		    (pat->action->forw_type == ROLE_FORW_YES ||
		     pat->action->forw_type == ROLE_FORW_NOCONF)) ||
	           (rflags & ROLE_COMPOSE &&
		    (pat->action->comp_type == ROLE_COMP_YES ||
		     pat->action->comp_type == ROLE_COMP_NOCONF)) ||
		   (rflags & ROLE_INCOL && pat->action->incol &&
		    (!VAR_NORM_FORE_COLOR || !VAR_NORM_BACK_COLOR ||
		     !pat->action->incol->fg ||
		     !pat->action->incol->bg ||
		     strucmp(VAR_NORM_FORE_COLOR,pat->action->incol->fg) ||
		     strucmp(VAR_NORM_BACK_COLOR,pat->action->incol->bg))) ||
	           (rflags & ROLE_OLD_PATS)))
		||
		 pat->inherit);
	pat = next_any_pattern(pstate))
      ;
    
    return(pat);
}

    
/*
 * This assumes that pstate is valid.
 */
PAT_S *
prev_any_pattern(pstate)
    PAT_STATE *pstate;
{
    PAT_LINE_S *patline;

    if(pstate->patlinecurrent){
	if(pstate->patcurrent && pstate->patcurrent->prev)
	  pstate->patcurrent = pstate->patcurrent->prev;
	else{
	    /* Find prev patline with a pat */
	    for(patline = pstate->patlinecurrent->prev;
		patline && !patline->last;
		patline = patline->prev)
	      ;
	    
	    if(patline){
		pstate->patlinecurrent = patline;
		pstate->patcurrent     = patline->last;
	    }
	    else{
		pstate->patlinecurrent = NULL;
		pstate->patcurrent     = NULL;
	    }
	}
    }

    if(!pstate->patcurrent){
	pstate->cur_rflag_num--;
	pstate->patcurrent = last_any_pattern(pstate);
    }

    return(pstate->patcurrent);
}


/*
 * Return prev pattern of the specified types. These types were set by a
 * previous call to any_patterns or nonempty_patterns.
 *
 * Args -- pstate  pattern state. This is set by first_pattern or last_pattern.
 */
PAT_S *
prev_pattern(pstate)
    PAT_STATE  *pstate;
{
    PAT_S           *pat;
    struct variable *vars = ps_global->vars;
    long             rflags;

    rflags = pstate->rflags;

    for(pat = prev_any_pattern(pstate);
	pat && !((pat->action &&
		  ((rflags & ROLE_DO_ROLES && pat->action->is_a_role) ||
	           (rflags & ROLE_DO_INCOLS && pat->action->is_a_incol) ||
	           (rflags & ROLE_DO_SCORES && pat->action->is_a_score) ||
		   (rflags & ROLE_SCORE && pat->action->scoreval) ||
		   (rflags & ROLE_DO_FILTER && pat->action->is_a_filter) ||
	           (rflags & ROLE_REPLY &&
		    (pat->action->repl_type == ROLE_REPL_YES ||
		     pat->action->repl_type == ROLE_REPL_NOCONF)) ||
	           (rflags & ROLE_FORWARD &&
		    (pat->action->forw_type == ROLE_FORW_YES ||
		     pat->action->forw_type == ROLE_FORW_NOCONF)) ||
	           (rflags & ROLE_COMPOSE &&
		    (pat->action->comp_type == ROLE_COMP_YES ||
		     pat->action->comp_type == ROLE_COMP_NOCONF)) ||
		   (rflags & ROLE_INCOL && pat->action->incol &&
		    (!VAR_NORM_FORE_COLOR || !VAR_NORM_BACK_COLOR ||
		     !pat->action->incol->fg ||
		     !pat->action->incol->bg ||
		     strucmp(VAR_NORM_FORE_COLOR,pat->action->incol->fg) ||
		     strucmp(VAR_NORM_BACK_COLOR,pat->action->incol->bg))) ||
	           (rflags & ROLE_OLD_PATS)))
		||
		 pat->inherit);
	pat = prev_any_pattern(pstate))
      ;
    
    return(pat);
}

    
/*
 * Rflags may be more than one pattern type OR'd together.
 */
int
write_patterns(rflags)
    long rflags;
{
    int canon_rflags;
    int err = 0;

    dprint(7, (debugfile, "write_patterns(0x%x)\n", rflags));

    canon_rflags = CANONICAL_RFLAGS(rflags);

    if(canon_rflags & ROLE_DO_INCOLS)
      err += sub_write_patterns(ROLE_DO_INCOLS | (rflags & PAT_USE_MASK));
    if(!err && canon_rflags & ROLE_DO_FILTER)
      err += sub_write_patterns(ROLE_DO_FILTER | (rflags & PAT_USE_MASK));
    if(!err && canon_rflags & ROLE_DO_SCORES)
      err += sub_write_patterns(ROLE_DO_SCORES | (rflags & PAT_USE_MASK));
    if(!err && canon_rflags & ROLE_DO_ROLES)
      err += sub_write_patterns(ROLE_DO_ROLES | (rflags & PAT_USE_MASK));

    if(!err)
      write_pinerc(ps_global, (rflags & PAT_USE_MAIN) ? Main : Post);

    return(err);
}


int
sub_write_patterns(rflags)
    long rflags;
{
    int            err = 0, lineno = 0;
    char         **lvalue = NULL;
    PAT_LINE_S    *patline;

    SET_PATTYPE(rflags);

    if(!(*cur_pat_h)){
	q_status_message(SM_ORDER | SM_DING, 3, 4,
			  "Unknown error saving patterns");
	return(-1);
    }

    if((*cur_pat_h)->dirtypinerc){
	/* Count how many lines will be in patterns variable */
	for(patline = (*cur_pat_h)->patlinehead;
	    patline;
	    patline = patline->next)
	  lineno++;
    
	lvalue = (char **)fs_get((lineno+1)*sizeof(char *));
	memset(lvalue, 0, (lineno+1) * sizeof(char *));
    }

    for(patline = (*cur_pat_h)->patlinehead, lineno = 0;
	!err && patline;
	patline = patline->next, lineno++){
	if(patline->type == File)
	  err = write_pattern_file((*cur_pat_h)->dirtypinerc
				      ? &lvalue[lineno] : NULL, patline);
	else if(patline->type == Literal && (*cur_pat_h)->dirtypinerc)
	  err = write_pattern_lit(&lvalue[lineno], patline);
	else if(patline->type == Inherit)
	  err = write_pattern_inherit((*cur_pat_h)->dirtypinerc
				      ? &lvalue[lineno] : NULL, patline);
    }

    if((*cur_pat_h)->dirtypinerc){
	if(err)
	  free_list_array(&lvalue);
	else{
	    char ***alval;
	    struct variable *var;

	    if(rflags & ROLE_DO_ROLES)
	      var = &ps_global->vars[V_PAT_ROLES];
	    else if(rflags & ROLE_DO_FILTER)
	      var = &ps_global->vars[V_PAT_FILTS];
	    else if(rflags & ROLE_DO_SCORES)
	      var = &ps_global->vars[V_PAT_SCORES];
	    else if(rflags & ROLE_DO_INCOLS)
	      var = &ps_global->vars[V_PAT_INCOLS];

	    alval = ALVAL(var, (rflags & PAT_USE_MAIN) ? Main : Post);
	    if(*alval)
	      free_list_array(alval);
	    
	    *alval = lvalue;

	    set_current_val(var, TRUE, TRUE);
	}
    }

    if(!err)
      (*cur_pat_h)->dirtypinerc = 0;

    return(err);
}


/*
 * Write pattern lines into a file.
 *
 * Args  lvalue -- Pointer to char * to fill in variable value
 *      patline -- 
 *
 * Returns  0 -- all is ok, lvalue has been filled in, file has been written
 *       else -- error, lvalue untouched, file not written
 */
int
write_pattern_file(lvalue, patline)
    char      **lvalue;
    PAT_LINE_S *patline;
{
    char  *p, *tfile;
    int    fd = -1, err = 0;
    FILE  *fp_new;
    PAT_S *pat;

    dprint(7, (debugfile, "write_pattern_file(%s)\n", patline->filepath));

    if(lvalue){
	p = (char *)fs_get((strlen(patline->filename) + 6) * sizeof(char));
	strcat(strcpy(p, "FILE:"), patline->filename);
	*lvalue = p;
    }

    if(patline->readonly || !patline->dirty)	/* doesn't need writing */
      return(err);

    /* Get a tempfile to write the patterns into */
    if(((tfile = tempfile_in_same_dir(patline->filepath, ".pt", NULL)) == NULL)
       || ((fd = open(tfile, O_TRUNC|O_WRONLY|O_CREAT, 0600)) < 0)
       || ((fp_new = fdopen(fd, "w")) == NULL)){
	q_status_message1(SM_ORDER | SM_DING, 3, 4,
			  "Can't write in directory containing file \"%s\"",
			  patline->filepath);
	if(tfile){
	    (void)unlink(tfile);
	    fs_give((void **)&tfile);
	}
	
	if(fd >= 0)
	  close(fd);

	return(-1);
    }

    dprint(9, (debugfile, "write_pattern_file: writing into %s\n", tfile));
    
    if(fprintf(fp_new, "%s %s\n", PATTERN_MAGIC, PATTERN_FILE_VERS) == EOF)
      err--;

    for(pat = patline->first; !err && pat; pat = pat->next){
	if((p = data_for_patline(pat)) != NULL){
	    if(fprintf(fp_new, "%s\n", p) == EOF)
	      err--;
	    
	    fs_give((void **)&p);
	}
    }

    if(err || fclose(fp_new) == EOF){
	if(err)
	  (void)fclose(fp_new);

	err--;
	q_status_message2(SM_ORDER | SM_DING, 3, 4,
			  "I/O error: \"%s\": %s",
			  tfile, error_description(errno));
    }

    if(!err && rename_file(tfile, patline->filepath) < 0){
	err--;
	q_status_message3(SM_ORDER | SM_DING, 3, 4,
			  "Error renaming \"%s\" to \"%s\": %s",
			  tfile, patline->filepath, error_description(errno));
	dprint(2, (debugfile,
		   "write_pattern_file: Error renaming (%s,%s): %s\n",
		   tfile, patline->filepath, error_description(errno)));
    }

    if(tfile){
	(void)unlink(tfile);
	fs_give((void **)&tfile);
    }

    if(!err)
      patline->dirty = 0;

    return(err);
}


/*
 * Write literal pattern lines into lvalue (pinerc variable).
 *
 * Args  lvalue -- Pointer to char * to fill in variable value
 *      patline -- 
 *
 * Returns  0 -- all is ok, lvalue has been filled in, file has been written
 *       else -- error, lvalue untouched, file not written
 */
int
write_pattern_lit(lvalue, patline)
    char      **lvalue;
    PAT_LINE_S *patline;
{
    char  *p = NULL;
    int    err = 0;
    PAT_S *pat;

    pat = patline ? patline->first : NULL;
    
    if(pat && lvalue && (p = data_for_patline(pat)) != NULL){
	*lvalue = (char *)fs_get((strlen(p) + 5) * sizeof(char));
	strcat(strcpy(*lvalue, "LIT:"), p);
    }
    else{
	q_status_message(SM_ORDER | SM_DING, 3, 4,
			 "Unknown error saving pattern variable");
	err--;
    }
    
    if(p)
      fs_give((void **)&p);

    return(err);
}


int
write_pattern_inherit(lvalue, patline)
    char      **lvalue;
    PAT_LINE_S *patline;
{
    int    err = 0;

    if(patline && patline->type == Inherit && lvalue)
      *lvalue = cpystr(INHERIT);
    else
      err--;
    
    return(err);
}



char *
data_for_patline(pat)
    PAT_S *pat;
{
    char          *p, *q, *to_pat = NULL, *news_pat = NULL, *from_pat = NULL,
		  *sender_pat = NULL, *cc_pat = NULL, *subj_pat = NULL,
		  *arb_pat = NULL, *fldr_type_pat = NULL, *fldr_pat = NULL,
		  *alltext_pat = NULL, *scorei_pat = NULL, *recip_pat = NULL,
		  *partic_pat = NULL, *stat_new_val = NULL,
		  *stat_imp_val = NULL, *stat_del_val = NULL,
		  *stat_ans_val = NULL,
		  *from_act = NULL, *replyto_act = NULL, *fcc_act = NULL,
		  *sig_act = NULL, *nick = NULL, *templ_act = NULL,
		  *litsig_act = NULL, *cstm_act = NULL,
		  *repl_val = NULL, *forw_val = NULL, *comp_val = NULL,
		  *incol_act = NULL, *inherit_nick = NULL, *score_act = NULL,
		  *folder_act = NULL, *filt_ifnotdel = NULL;
    ACTION_S      *action = NULL;
    NAMEVAL_S     *f;

    if(pat->patgrp){
	if(pat->patgrp->nick)
	  if((nick = add_pat_escapes(pat->patgrp->nick)) && !*nick)
	    fs_give((void **) &nick);

	if(pat->patgrp->to){
	    p = pattern_to_string(pat->patgrp->to);
	    if(p){
		to_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->from){
	    p = pattern_to_string(pat->patgrp->from);
	    if(p){
		from_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->sender){
	    p = pattern_to_string(pat->patgrp->sender);
	    if(p){
		sender_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->cc){
	    p = pattern_to_string(pat->patgrp->cc);
	    if(p){
		cc_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->recip){
	    p = pattern_to_string(pat->patgrp->recip);
	    if(p){
		recip_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->partic){
	    p = pattern_to_string(pat->patgrp->partic);
	    if(p){
		partic_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->news){
	    p = pattern_to_string(pat->patgrp->news);
	    if(p){
		news_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->subj){
	    p = pattern_to_string(pat->patgrp->subj);
	    if(p){
		subj_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->alltext){
	    p = pattern_to_string(pat->patgrp->alltext);
	    if(p){
		alltext_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->arbhdr){
	    ARBHDR_S *a;
	    char     *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL;
	    int       len = 0;

	    /* This is brute force dumb, but who cares? */
	    for(a = pat->patgrp->arbhdr; a; a = a->next){
		if(a->field && a->field[0]){
		    p1 = pattern_to_string(a->p);
		    p1 = p1 ? p1 : cpystr("");
		    p2 = (char *)fs_get((strlen(a->field)+strlen(p1)+2) *
							    sizeof(char));
		    sprintf(p2, "%s=%s", a->field, p1);
		    p3 = add_pat_escapes(p2);
		    p4 = (char *)fs_get((strlen(p3)+6) * sizeof(char));
		    sprintf(p4, "/%sARB%s", a->isemptyval ? "E" : "", p3);
		    len += strlen(p4);

		    if(p1)
		      fs_give((void **)&p1);
		    if(p2)
		      fs_give((void **)&p2);
		    if(p3)
		      fs_give((void **)&p3);
		    if(p4)
		      fs_give((void **)&p4);
		}
	    }

	    p = arb_pat = (char *)fs_get((len + 1) * sizeof(char));

	    for(a = pat->patgrp->arbhdr; a; a = a->next){
		if(a->field && a->field[0]){
		    p1 = pattern_to_string(a->p);
		    p1 = p1 ? p1 : cpystr("");
		    p2 = (char *)fs_get((strlen(a->field)+strlen(p1)+2) *
							    sizeof(char));
		    sprintf(p2, "%s=%s", a->field, p1);
		    p3 = add_pat_escapes(p2);
		    p4 = (char *)fs_get((strlen(p3)+6) * sizeof(char));
		    sprintf(p4, "/%sARB%s", a->isemptyval ? "E" : "", p3);
		    sstrcpy(&p, p4);

		    if(p1)
		      fs_give((void **)&p1);
		    if(p2)
		      fs_give((void **)&p2);
		    if(p3)
		      fs_give((void **)&p3);
		    if(p4)
		      fs_give((void **)&p4);
		}
	    }
	}

	if(pat->patgrp->do_score){
	    p = stringform_of_score_interval(pat->patgrp->score_min,
					     pat->patgrp->score_max);
	    if(p){
		scorei_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if((f = pat_fldr_types(pat->patgrp->fldr_type)) != NULL)
	  fldr_type_pat = f->shortname;

	if(pat->patgrp->folder){
	    p = pattern_to_string(pat->patgrp->folder);
	    if(p){
		fldr_pat = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(pat->patgrp->stat_new != PAT_STAT_EITHER &&
	   (f = role_status_types(pat->patgrp->stat_new)) != NULL)
	  stat_new_val = f->shortname;

	if(pat->patgrp->stat_del != PAT_STAT_EITHER &&
	   (f = role_status_types(pat->patgrp->stat_del)) != NULL)
	  stat_del_val = f->shortname;

	if(pat->patgrp->stat_ans != PAT_STAT_EITHER &&
	   (f = role_status_types(pat->patgrp->stat_ans)) != NULL)
	  stat_ans_val = f->shortname;

	if(pat->patgrp->stat_imp != PAT_STAT_EITHER &&
	   (f = role_status_types(pat->patgrp->stat_imp)) != NULL)
	  stat_imp_val = f->shortname;
    }

    if(pat->action){
	action = pat->action;

	if(action->is_a_score && action->scoreval != 0 &&
	   action->scoreval >= SCORE_MIN && action->scoreval <= SCORE_MAX){
	    score_act = (char *)fs_get(5 * sizeof(char));
	    sprintf(score_act, "%d", pat->action->scoreval);
	}

	if(action->is_a_role){
	    if(action->inherit_nick)
	      inherit_nick = add_pat_escapes(action->inherit_nick);
	    if(action->fcc)
	      fcc_act = add_pat_escapes(action->fcc);
	    if(action->litsig)
	      litsig_act = add_pat_escapes(action->litsig);
	    if(action->sig)
	      sig_act = add_pat_escapes(action->sig);
	    if(action->template)
	      templ_act = add_pat_escapes(action->template);

	    if(action->cstm){
		size_t sz;
		char **l, *q;

		/* concatenate into string with commas first */
		sz = 0;
		for(l = action->cstm; l[0] && l[0][0]; l++)
		  sz += strlen(l[0]) + 1;

		if(sz){
		    char *p;
		    int   first_one = 1;

		    q = (char *)fs_get(sz);
		    memset(q, 0, sz);
		    p = q;
		    for(l = action->cstm; l[0] && l[0][0]; l++){
			if((!struncmp(l[0], "from", 4) &&
			   (l[0][4] == ':' || l[0][4] == '\0')) ||
			   (!struncmp(l[0], "reply-to", 8) &&
			   (l[0][8] == ':' || l[0][8] == '\0')))
			  continue;

			if(!first_one)
			  sstrcpy(&p, ",");

		        first_one = 0;
			sstrcpy(&p, l[0]);
		    }

		    cstm_act = add_pat_escapes(q);
		    fs_give((void **)&q);
		}
	    }

	    if((f = role_repl_types(action->repl_type)) != NULL)
	      repl_val = f->shortname;

	    if((f = role_forw_types(action->forw_type)) != NULL)
	      forw_val = f->shortname;
	    
	    if((f = role_comp_types(action->comp_type)) != NULL)
	      comp_val = f->shortname;
	}
	
	if(action->is_a_incol && action->incol){
	    char *ptr, buf[256], *p1, *p2;

	    ptr = buf;
	    memset(buf, 0, sizeof(buf));
	    sstrcpy(&ptr, "/FG=");
	    sstrcpy(&ptr, (p1=add_pat_escapes(action->incol->fg)));
	    sstrcpy(&ptr, "/BG=");
	    sstrcpy(&ptr, (p2=add_pat_escapes(action->incol->bg)));
	    /* the colors will be doubly escaped */
	    incol_act = add_pat_escapes(buf);
	    if(p1)
	      fs_give((void **)&p1);
	    if(p2)
	      fs_give((void **)&p2);
	}

	if(action->is_a_role && action->from){
	    char *bufp;

	    bufp = (char *)fs_get((size_t)est_size(action->from));
	    p = addr_string(action->from, bufp);
	    if(p){
		from_act = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(action->is_a_role && action->replyto){
	    char *bufp;

	    bufp = (char *)fs_get((size_t)est_size(action->replyto));
	    p = addr_string(action->replyto, bufp);
	    if(p){
		replyto_act = add_pat_escapes(p);
		fs_give((void **)&p);
	    }
	}

	if(action->is_a_filter && action->folder){
	    if(p = pattern_to_string(action->folder)){
		folder_act = add_pat_escapes(p);
		fs_give((void **) &p);

		if(action->move_only_if_not_deleted)
		  filt_ifnotdel = cpystr("/NOTDEL=1");
	    }
	}
    }

    p = (char *)fs_get((strlen(nick ? nick : "Alternate Role") +
			strlen(to_pat ? to_pat : "") +
			strlen(from_pat ? from_pat : "") +
			strlen(sender_pat ? sender_pat : "") +
			strlen(cc_pat ? cc_pat : "") +
			strlen(recip_pat ? recip_pat : "") +
			strlen(partic_pat ? partic_pat : "") +
			strlen(news_pat ? news_pat : "") +
			strlen(subj_pat ? subj_pat : "") +
			strlen(alltext_pat ? alltext_pat : "") +
			strlen(arb_pat ? arb_pat : "") +
			strlen(scorei_pat ? scorei_pat : "") +
			strlen(inherit_nick ? inherit_nick : "") +
			strlen(score_act ? score_act : "") +
			strlen(from_act ? from_act : "") +
			strlen(replyto_act ? replyto_act : "") +
			strlen(fcc_act ? fcc_act : "") +
			strlen(litsig_act ? litsig_act : "") +
			strlen(cstm_act ? cstm_act : "") +
			strlen(sig_act ? sig_act : "") +
			strlen(incol_act ? incol_act : "") +
			(folder_act ? (strlen(folder_act) + 8) : 0) +
			strlen(templ_act ? templ_act : "") + 300)*sizeof(char));

    q = p;
    sstrcpy(&q, "pattern=\"/NICK=");

    if(nick){
	sstrcpy(&q, nick);
	fs_give((void **) &nick);
    }
    else
      sstrcpy(&q, "Alternate Role");

    if(to_pat){
	sstrcpy(&q, "/TO=");
	sstrcpy(&q, to_pat);
	fs_give((void **) &to_pat);
    }

    if(from_pat){
	sstrcpy(&q, "/FROM=");
	sstrcpy(&q, from_pat);
	fs_give((void **) &from_pat);
    }

    if(sender_pat){
	sstrcpy(&q, "/SENDER=");
	sstrcpy(&q, sender_pat);
	fs_give((void **) &sender_pat);
    }

    if(cc_pat){
	sstrcpy(&q,"/CC=");
	sstrcpy(&q, cc_pat);
	fs_give((void **) &cc_pat);
    }

    if(recip_pat){
	sstrcpy(&q, "/RECIP=");
	sstrcpy(&q, recip_pat);
	fs_give((void **) &recip_pat);
    }

    if(partic_pat){
	sstrcpy(&q, "/PARTIC=");
	sstrcpy(&q, partic_pat);
	fs_give((void **) &partic_pat);
    }

    if(news_pat){
	sstrcpy(&q, "/NEWS=");
	sstrcpy(&q, news_pat);
	fs_give((void **) &news_pat);
    }

    if(subj_pat){
	sstrcpy(&q, "/SUBJ=");
	sstrcpy(&q, subj_pat);
	fs_give((void **)&subj_pat);
    }

    if(alltext_pat){
	sstrcpy(&q, "/ALL=");
	sstrcpy(&q, alltext_pat);
	fs_give((void **) &alltext_pat);
    }

    if(arb_pat){
	sstrcpy(&q, arb_pat);
	fs_give((void **)&arb_pat);
    }

    if(scorei_pat){
	sstrcpy(&q, "/SCOREI=");
	sstrcpy(&q, scorei_pat);
	fs_give((void **) &scorei_pat);
    }

    if(fldr_type_pat){
	sstrcpy(&q, "/FLDTYPE=");
	sstrcpy(&q, fldr_type_pat);
    }

    if(fldr_pat){
	sstrcpy(&q, "/FOLDER=");
	sstrcpy(&q, fldr_pat);
	fs_give((void **) &fldr_pat);
    }

    if(stat_new_val){
	sstrcpy(&q, "/STATN=");
	sstrcpy(&q, stat_new_val);
    }

    if(stat_del_val){
	sstrcpy(&q, "/STATD=");
	sstrcpy(&q, stat_del_val);
    }

    if(stat_imp_val){
	sstrcpy(&q, "/STATI=");
	sstrcpy(&q, stat_imp_val);
    }

    if(stat_ans_val){
	sstrcpy(&q, "/STATA=");
	sstrcpy(&q, stat_ans_val);
    }

    sstrcpy(&q, "\" action=\"");

    if(inherit_nick && *inherit_nick){
	sstrcpy(&q, "/INICK=");
	sstrcpy(&q, inherit_nick);
	fs_give((void **)&inherit_nick);
    }

    if(action){
	if(action->is_a_role)
	  sstrcpy(&q, "/ROLE=1");

	if(action->is_a_incol)
	  sstrcpy(&q, "/ISINCOL=1");

	if(action->is_a_score)
	  sstrcpy(&q, "/ISSCORE=1");

	if(action->is_a_filter)
	  sstrcpy(&q, "/FILTER=1");
    }

    if(score_act){
	sstrcpy(&q, "/SCORE=");
	sstrcpy(&q, score_act);
	fs_give((void **)&score_act);
    }

    if(from_act){
	sstrcpy(&q, "/FROM=");
	sstrcpy(&q, from_act);
      fs_give((void **) &from_act);
    }

    if(replyto_act){
	sstrcpy(&q, "/REPL=");
	sstrcpy(&q, replyto_act);
	fs_give((void **)&replyto_act);
    }

    if(fcc_act){
	sstrcpy(&q, "/FCC=");
	sstrcpy(&q, fcc_act);
	fs_give((void **)&fcc_act);
    }

    if(litsig_act){
	sstrcpy(&q, "/LSIG=");
	sstrcpy(&q, litsig_act);
	fs_give((void **)&litsig_act);
    }

    if(sig_act){
	sstrcpy(&q, "/SIG=");
	sstrcpy(&q, sig_act);
	fs_give((void **)&sig_act);
    }

    if(templ_act){
	sstrcpy(&q, "/TEMPLATE=");
	sstrcpy(&q, templ_act);
	fs_give((void **)&templ_act);
    }

    if(cstm_act){
	sstrcpy(&q, "/CSTM=");
	sstrcpy(&q, cstm_act);
	fs_give((void **)&cstm_act);
    }

    if(repl_val){
	sstrcpy(&q, "/RTYPE=");
	sstrcpy(&q, repl_val);
    }

    if(forw_val){
	sstrcpy(&q, "/FTYPE=");
	sstrcpy(&q, forw_val);
    }

    if(comp_val){
	sstrcpy(&q, "/CTYPE=");
	sstrcpy(&q, comp_val);
    }

    if(incol_act){
	sstrcpy(&q, "/INCOL=");
	sstrcpy(&q, incol_act);
	fs_give((void **)&incol_act);
    }

    if(folder_act){
	sstrcpy(&q, "/FOLDER=");
	sstrcpy(&q, folder_act);
	fs_give((void **) &folder_act);
    }

    if(filt_ifnotdel){
	sstrcpy(&q, filt_ifnotdel);
	fs_give((void **) &filt_ifnotdel);
    }

    *q++ = '\"';
    *q   = '\0';

    return(p);
}

    
/*
 * Returns 1 if any message in the searchset matches this pattern, else 0.
 * The "searched" bit will be set for each message which matches.
 */
int
match_pattern(patgrp, stream, searchset, section, get_score)
    PATGRP_S   *patgrp;
    MAILSTREAM *stream;
    SEARCHSET  *searchset;
    char       *section;
    int         (*get_score) PROTO((MAILSTREAM *, long));
{
    char         *charset = NULL;
    SEARCHPGM    *pgm;
    SEARCHSET    *s;
    MESSAGECACHE *mc;
    PATTERN_S    *p;
    long          i, msgno = 0L;
    long          flags = (SO_NOSERVER|SE_NOPREFETCH|SE_FREE);

    dprint(7, (debugfile, "match_pattern\n"));

    /*
     * Is the current folder the right type and possibly the right specific
     * folder for a match?
     */
    if(!(patgrp && match_pattern_folder(patgrp, stream)))
      return(0);

    /*
     * NULL searchset means that there is no message to compare against.
     * This is a match if the folder type matches above (that gets
     * us here). We choose to ignore the rest of the pattern and call it
     * a match. In any case, we don't want to call c-client with the
     * null searchset.
     */
    if(!searchset)
      return(1);

    pgm = match_pattern_srchpgm(patgrp, stream, &charset, searchset);

    if(patgrp->alltext
       && (!is_imap_stream(stream) || modern_imap_stream(stream)))
	/*
	 * Cache isn't going to work. Search on server.
	 * Except that is likely to not work on an old imap server because
	 * the OR criteria won't work and we are likely to have some ORs.
	 * So turn off the NOSERVER flag (and search on server if remote)
	 * unless the server is an old server. It doesn't matter if we
	 * turn if off if it's not an imap stream, but we do it anyway.
	 */
      flags &= ~SO_NOSERVER;

    if(section){
	int charset_unknown = 0;

	/*
	 * Mail_search_full only searches the top-level msg. We want to
	 * search an attached msg instead. First do the stuff
	 * that mail_search_full would have done before calling
	 * mail_search_msg, then call mail_search_msg with a section number.
	 * Mail_search_msg does take a section number even though
	 * mail_search_full doesn't.
	 */

	/*
	 * We'll only ever set section if the searchset is a single message.
	 */
	if(pgm->msgno->next == NULL && pgm->msgno->first == pgm->msgno->last)
	  msgno = pgm->msgno->first;

	for(i = 1L; i <= stream->nmsgs; i++)
	  mail_elt(stream, i)->searched = NIL;

	if(charset && *charset &&  /* convert if charset not ASCII or UTF-8 */
	  !(((charset[0] == 'U') || (charset[0] == 'u')) &&
	    ((((charset[1] == 'S') || (charset[1] == 's')) &&
	      (charset[2] == '-') &&
	      ((charset[3] == 'A') || (charset[3] == 'a')) &&
	      ((charset[4] == 'S') || (charset[4] == 's')) &&
	      ((charset[5] == 'C') || (charset[5] == 'c')) &&
	      ((charset[6] == 'I') || (charset[6] == 'i')) &&
	      ((charset[7] == 'I') || (charset[7] == 'i')) && !charset[8]) ||
	     (((charset[1] == 'T') || (charset[1] == 't')) &&
	      ((charset[2] == 'F') || (charset[2] == 'f')) &&
	      (charset[3] == '-') && (charset[4] == '8') && !charset[5])))){
	    if(utf8_text(NIL,charset,NIL,T))
	      utf8_searchpgm (pgm,charset);
	    else
	      charset_unknown++;
	}

	if(!charset_unknown && mail_search_msg(stream,msgno,section,pgm))
	  mail_elt(stream,msgno)->searched = T;

	if(flags & SE_FREE)
	  mail_free_searchpgm(&pgm);
    }
    else
      mail_search_full(stream, charset, pgm, flags);

    if(charset)
      fs_give((void **)&charset);

    /* check scores */
    if(get_score && scores_are_used(SCOREUSE_GET) && patgrp->do_score){
	char      *savebits;
	SEARCHSET *ss;

	/*
	 * Get_score may call build_header_line recursively (we may
	 * be in build_header_line now) so we have to preserve and
	 * restore the sequence bits.
	 */
	savebits = (char *)fs_get((stream->nmsgs+1) * sizeof(char));

	for(i = 1L; i <= stream->nmsgs; i++){
	    savebits[i] = (mc=mail_elt(stream, i))->sequence;
	    mc->sequence = 0;
	}

	/*
	 * Build a searchset which will get all the scores that we
	 * need but not more.
	 */
	for(s = searchset; s; s = s->next)
	  for(msgno = s->first; msgno <= s->last; msgno++)
	    if((mc=mail_elt(stream, msgno))->searched &&
	       get_msg_score(stream, msgno) == SCORE_UNDEF)
	      mc->sequence = 1;
	
	if((ss = build_searchset(stream)) != NULL){
	    calculate_some_scores(stream, ss);
	    mail_free_searchset(&ss);
	}

	/*
	 * Now check the scores versus the score intervals to see if
	 * any of the messages which have matched up to this point can
	 * be tossed because they don't match the score interval.
	 */
	for(s = searchset; s; s = s->next)
	  for(msgno = s->first; msgno <= s->last; msgno++)
	    if((mc = mail_elt(stream, msgno))->searched){
		int score;

		score = (*get_score)(stream, msgno);

		/*
		 * If the score is outside the interval, turn off the
		 * searched bit.
		 */
		if(score != SCORE_UNDEF &&
		   (score < patgrp->score_min || score > patgrp->score_max))
		  mc->searched = NIL;
	    }

	for(i = 1L; i <= stream->nmsgs; i++)
	  mail_elt(stream, i)->sequence = savebits[i];
    
	fs_give((void **)&savebits);
    }

    for(s = searchset; s; s = s->next)
      for(msgno = s->first; msgno <= s->last; msgno++)
        if(mail_elt(stream, msgno)->searched)
	  return(1);

    return(0);
}


int
match_pattern_folder(patgrp, stream)
   PATGRP_S   *patgrp;
   MAILSTREAM *stream;
{
    int	       is_news;
    
    return(stream
	   && ((patgrp->fldr_type == FLDR_ANY)
	       || ((is_news = IS_NEWS(stream))
		   && patgrp->fldr_type == FLDR_NEWS)
	       || (!is_news && patgrp->fldr_type == FLDR_EMAIL)
	       || (patgrp->fldr_type == FLDR_SPECIFIC
		   && match_pattern_folder_specific(patgrp->folder,stream,1))));
}


/* 
 * If FOR_PATTERN is set, this interprets simple names as nicknames in
 * the incoming collection, otherwise it treats simple names as being in
 * the primary collection.
 */
int
match_pattern_folder_specific(folders, stream, flags)
    PATTERN_S  *folders;
    MAILSTREAM *stream;
    int         flags;
{
    PATTERN_S *p;
    int        match = 0;

    if(!(stream && stream->mailbox && stream->mailbox[0]))
      return(0);

    /*
     * For each of the folders in the pattern, see if we get
     * a match. We're just looking for any match. If none match,
     * we return 0, otherwise we fall through and check the rest
     * of the pattern. The fact that the string is called "substring"
     * is not meaningful. We're just using the convenient pattern
     * structure to store a list of folder names. They aren't
     * substrings of names, they are the whole name.
     */
    for(p = folders; !match && p; p = p->next){
	if(p->substring
	   && (!strucmp(p->substring, ps_global->inbox_name)
	       || !strcmp(p->substring, ps_global->VAR_INBOX_PATH))){
	    if(stream == ps_global->inbox_stream)
	      match++;
	}
	else{
	    char      *patfolder, *fname;
	    char      *t, *streamfolder;
	    char       tmp1[MAILTMPLEN], tmp2[max(MAILTMPLEN,NETMAXMBX)];
	    CONTEXT_S *cntxt = NULL;

	    patfolder = p->substring;

	    if(flags & FOR_PATTERN){
		/*
		 * See if patfolder is a nickname in the incoming collection.
		 * If so, use its real name instead.
		 */
		if(patfolder[0] &&
		   (ps_global->context_list->use & CNTXT_INCMNG) &&
		   (fname = (folder_is_nick(patfolder,
					    FOLDERS(ps_global->context_list)))))
		  patfolder = fname;
	    }
	    else{
		/*
		 * If it's an absolute pathname, we treat is as a local file
		 * instead of interpreting it in the primary context.
		 */
		if(!is_absolute_path(patfolder)
		   && !(cntxt = default_save_context(ps_global->context_list)))
		  cntxt = ps_global->context_list;
		
		patfolder = context_apply(tmp1, cntxt, patfolder, sizeof(tmp1));
	    }

	    switch(patfolder[0]){
	      case '{':
		if(stream->mailbox[0] == '{' &&
		   same_stream(patfolder, stream) &&
		   (streamfolder = strindex(&stream->mailbox[1], '}')) &&
		   (t = strindex(&patfolder[1], '}')) &&
		   !strcmp(t+1, streamfolder+1))
		  match++;

		break;
	      
	      case '#':
	        if(!strcmp(patfolder, stream->mailbox))
		  match++;

		break;

	      default:
		t = (strlen(patfolder) < (MAILTMPLEN/2))
				? mailboxfile(tmp2, patfolder) : NULL;
		if(t && *t && !strcmp(t, stream->mailbox))
		  match++;

		break;
	    }
	}
    }

    return(match);
}


/*
 * generate a search program corresponding to the provided patgrp
 */
SEARCHPGM *
match_pattern_srchpgm(patgrp, stream, charsetp, searchset)
    PATGRP_S	*patgrp;
    MAILSTREAM	*stream;
    char       **charsetp;
    SEARCHSET	*searchset;
{
    SEARCHPGM	 *pgm;
    SEARCHSET	**sp;
    ROLE_ARGS_T	  rargs;

    rargs.multi = 0;
    rargs.cset = charsetp;
    rargs.ourcharset = (ps_global->VAR_CHAR_SET
			&& ps_global->VAR_CHAR_SET[0])
			 ? ps_global->VAR_CHAR_SET : NULL;

    pgm = mail_newsearchpgm();

    sp = &pgm->msgno;
    /* copy the searchset */
    while(searchset){
	SEARCHSET *s;

	s = mail_newsearchset();
	s->first = searchset->first;
	s->last  = searchset->last;
	searchset = searchset->next;
	*sp = s;
	sp = &s->next;
    }

    if(patgrp->subj)
      set_up_search_pgm("subject", patgrp->subj, pgm, &rargs);

    if(patgrp->cc)
      set_up_search_pgm("cc", patgrp->cc, pgm, &rargs);

    if(patgrp->from)
      set_up_search_pgm("from", patgrp->from, pgm, &rargs);

    if(patgrp->to)
      set_up_search_pgm("to", patgrp->to, pgm, &rargs);

    if(patgrp->sender)
      set_up_search_pgm("sender", patgrp->sender, pgm, &rargs);

    if(patgrp->news)
      set_up_search_pgm("newsgroups", patgrp->news, pgm, &rargs);

    /* To OR Cc */
    if(patgrp->recip){
	SEARCHOR *or, **or_ptr;

	/* find next unused or slot */
	for(or = pgm->or; or && or->next; or = or->next)
	  ;

	if(or)
	  or_ptr = &or->next;
	else
	  or_ptr = &pgm->or;

	*or_ptr = mail_newsearchor();
	set_up_search_pgm("to", patgrp->recip, (*or_ptr)->first, &rargs);
	set_up_search_pgm("cc", patgrp->recip, (*or_ptr)->second, &rargs);
    }

    /* To OR Cc OR From */
    if(patgrp->partic){
	SEARCHOR *or, **or_ptr;

	/* find next unused or slot */
	for(or = pgm->or; or && or->next; or = or->next)
	  ;

	if(or)
	  or_ptr = &or->next;
	else
	  or_ptr = &pgm->or;

	*or_ptr = mail_newsearchor();
	set_up_search_pgm("to", patgrp->partic, (*or_ptr)->first, &rargs);

	(*or_ptr)->second->or = mail_newsearchor();
	set_up_search_pgm("cc", patgrp->partic, (*or_ptr)->second->or->first,
			  &rargs);
	set_up_search_pgm("from", patgrp->partic, (*or_ptr)->second->or->second,
			  &rargs);
    }

    if(patgrp->arbhdr){
	ARBHDR_S *a;

	for(a = patgrp->arbhdr; a; a = a->next)
	  if(a->field && a->field[0] && a->p)
	    set_up_search_pgm(a->field, a->p, pgm, &rargs);
    }

    if(patgrp->alltext)
      set_up_search_pgm("alltext", patgrp->alltext, pgm, &rargs);
    
    SETPGMSTATUS(patgrp->stat_new,pgm->unseen,pgm->seen);
    SETPGMSTATUS(patgrp->stat_del,pgm->deleted,pgm->undeleted);
    SETPGMSTATUS(patgrp->stat_imp,pgm->flagged,pgm->unflagged);
    SETPGMSTATUS(patgrp->stat_ans,pgm->answered,pgm->unanswered);

    return(pgm);
}


void
set_up_search_pgm(field, pattern, pgm, rargs)
    char        *field;
    PATTERN_S   *pattern;
    SEARCHPGM   *pgm;
    ROLE_ARGS_T *rargs;
{
    SEARCHOR *or, **or_ptr;

    if(field && pattern && rargs && pgm){
	/*
	 * To is special because we want to use the ReSent-To header instead
	 * of the To header if it exists.  We set up something like:
	 *
	 * if((<resent-to exists> AND (resent-to matches pat1 or pat2...))
	 *                  OR
	 *    (<resent-to doesn't exist> AND (to matches pat1 or pat2...)))
	 */
	if(!strucmp(field, "to")){
	    ROLE_ARGS_T local_args;
	    char       *space = " ";

	    /* find next unused or slot */
	    for(or = pgm->or; or && or->next; or = or->next)
	      ;

	    if(or)
	      or_ptr = &or->next;
	    else
	      or_ptr = &pgm->or;

	    *or_ptr = mail_newsearchor();

	    local_args = *rargs;
	    local_args.cset = NULL;	/* just to save having to check */
	    /* check for resent-to exists */
	    set_srch("resent-to", space, (*or_ptr)->first, &local_args);

	    local_args.cset = rargs->cset;
	    add_type_to_pgm("resent-to", pattern, (*or_ptr)->first,
			    &local_args);

	    /* check for resent-to doesn't exist */
	    (*or_ptr)->second->not = mail_newsearchpgmlist();
	    local_args.cset = NULL;
	    set_srch("resent-to", space, (*or_ptr)->second->not->pgm,
		     &local_args);

	    /* now add the real To search to second */
	    local_args.cset = rargs->cset;
	    add_type_to_pgm(field, pattern, (*or_ptr)->second, &local_args);
	}
	else
	  add_type_to_pgm(field, pattern, pgm, rargs);
    }
}


void
add_type_to_pgm(field, pattern, pgm, rargs)
    char        *field;
    PATTERN_S   *pattern;
    SEARCHPGM   *pgm;
    ROLE_ARGS_T *rargs;
{
    PATTERN_S *p;
    SEARCHOR  *or, **or_ptr;

    if(field && pattern && rargs && pgm){
	for(p = pattern; p; p = p->next){
	    if(p->next){
		/*
		 * The list of or's (the or->next thing) is actually AND'd
		 * together. So we want to use a different member of that
		 * list for things we want to AND, like Subject A or B AND
		 * From C or D. On the other hand, for multiple items in
		 * one group which we really do want OR'd together, like
		 * Subject A or B or C we don't use the list, we use an
		 * OR tree (which is what this for loop is building).
		 */

		/* find next unused or slot */
		for(or = pgm->or; or && or->next; or = or->next)
		  ;

		if(or)
		  or_ptr = &or->next;
		else
		  or_ptr = &pgm->or;

		*or_ptr = mail_newsearchor();
		set_srch(field, p->substring ? p->substring : "",
			 (*or_ptr)->first, rargs);
		pgm = (*or_ptr)->second;
	    }
	    else{
		set_srch(field, p->substring ? p->substring : "", pgm, rargs);
	    }
	}
    }
}


void
set_srch(field, value, pgm, rargs)
    char        *field;
    char        *value;
    SEARCHPGM   *pgm;
    ROLE_ARGS_T *rargs;
{
    char        *decoded, *cs = NULL, *charset = NULL;
    STRINGLIST **list;

    if(!(field && value && rargs && pgm))
      return;

    if(!strucmp(field, "subject"))
      list = &pgm->subject;
    else if(!strucmp(field, "from"))
      list = &pgm->from;
    else if(!strucmp(field, "to"))
      list = &pgm->to;
    else if(!strucmp(field, "cc"))
      list = &pgm->cc;
    else if(!strucmp(field, "sender"))
      list = &pgm->sender;
    else if(!strucmp(field, "reply-to"))
      list = &pgm->reply_to;
    else if(!strucmp(field, "in-reply-to"))
      list = &pgm->in_reply_to;
    else if(!strucmp(field, "message-id"))
      list = &pgm->message_id;
    else if(!strucmp(field, "newsgroups"))
      list = &pgm->newsgroups;
    else if(!strucmp(field, "followup-to"))
      list = &pgm->followup_to;
    else if(!strucmp(field, "alltext"))
      list = &pgm->text;
    else{
	set_srch_hdr(field, value, pgm, rargs);
	return;
    }

    if(!list)
      return;

    *list = mail_newstringlist();
    decoded = (char *)rfc1522_decode((unsigned char *)tmp_20k_buf,
				     SIZEOF_20KBUF, value, &cs);

    (*list)->text.data = (unsigned char *)cpystr(decoded);
    (*list)->text.size = strlen(decoded);

    if(rargs->cset && !rargs->multi){
	if(decoded != value)
	  charset = (cs && cs[0]) ? cs : rargs->ourcharset;
	else if(!is_ascii_string(decoded))
	  charset = rargs->ourcharset;

	if(charset){
	    if(*rargs->cset){
		if(strucmp(*rargs->cset, charset) != 0){
		    rargs->multi = 1;
		    if(rargs->ourcharset &&
		       strucmp(rargs->ourcharset, *rargs->cset) != 0){
			fs_give((void **)rargs->cset);
			*rargs->cset = cpystr(rargs->ourcharset);
		    }
		}
	    }
	    else
	      *rargs->cset = cpystr(charset);
	}
    }

    if(cs)
      fs_give((void **)&cs);
}


void
set_srch_hdr(field, value, pgm, rargs)
    char        *field;
    char        *value;
    SEARCHPGM   *pgm;
    ROLE_ARGS_T *rargs;
{
    char *decoded, *cs = NULL, *charset = NULL;
    SEARCHHEADER  **hdr;

    if(!(field && value && rargs && pgm))
      return;

    hdr = &pgm->header;
    if(!hdr)
      return;

    decoded = (char *)rfc1522_decode((unsigned char *)tmp_20k_buf,
				     SIZEOF_20KBUF, value, &cs);
    while(*hdr && (*hdr)->next)
      *hdr = (*hdr)->next;
      
    if(*hdr)
      (*hdr)->next = mail_newsearchheader(field, decoded);
    else
      *hdr = mail_newsearchheader(field, decoded);

    if(rargs->cset && !rargs->multi){
	if(decoded != value)
	  charset = (cs && cs[0]) ? cs : rargs->ourcharset;
	else if(!is_ascii_string(decoded))
	  charset = rargs->ourcharset;

	if(charset){
	    if(*rargs->cset){
		if(strucmp(*rargs->cset, charset) != 0){
		    rargs->multi = 1;
		    if(rargs->ourcharset &&
		       strucmp(rargs->ourcharset, *rargs->cset) != 0){
			fs_give((void **)rargs->cset);
			*rargs->cset = cpystr(rargs->ourcharset);
		    }
		}
	    }
	    else
	      *rargs->cset = cpystr(charset);
	}
    }

    if(cs)
      fs_give((void **)&cs);
}


static char *extra_hdrs;

/*
 * Run through the patterns and note which headers we'll need to ask for
 * which aren't normally asked for and so won't be cached.
 */
void
calc_extra_hdrs()
{
    PAT_S    *pat = NULL;
    int       alloced_size;
    long      type = (ROLE_INCOL | ROLE_SCORE);
    ARBHDR_S *a;
    PAT_STATE pstate;
    char     *q, *p = NULL, *s, *hdrs[MLCMD_COUNT + 1], **pp;
#define INITIALSIZE 1000

    q = (char *)fs_get((INITIALSIZE+1) * sizeof(char));
    q[0] = '\0';
    alloced_size = INITIALSIZE;
    p = q;

    /*
     * *ALWAYS* make sure Resent-To is in the set of
     * extra headers getting fetched.
     *
     * This is because we *will* reference it when we're
     * building header lines and thus want it fetched with
     * the standard envelope data.  Worse, in the IMAP case
     * we're called back from c-client with the envelope data
     * so we can format and display the index lines as they
     * arrive, so we have to ensure the resent-to field
     * is in the cache so we don't reenter c-client
     * to look for it from the callback.  Yeouch.
     */
    add_eh(&q, &p, "resent-to", &alloced_size);
    add_eh(&q, &p, "resent-date", &alloced_size);
    add_eh(&q, &p, "resent-from", &alloced_size);
    add_eh(&q, &p, "resent-cc", &alloced_size);
    add_eh(&q, &p, "resent-subject", &alloced_size);

    /*
     * Sniff at viewer-hdrs too so we can include them
     * if there are any...
     */
    for(pp = ps_global->VAR_VIEW_HEADERS; pp && *pp; pp++)
      if(non_eh(*pp))
	add_eh(&q, &p, *pp, &alloced_size);

    /*
     * Be sure to ask for List management headers too
     * since we'll offer their use in the message view
     */
    for(pp = rfc2369_hdrs(hdrs); *pp; pp++)
      add_eh(&q, &p, *pp, &alloced_size);

    if(nonempty_patterns(type, &pstate))
      for(pat = first_pattern(&pstate);
	  pat;
	  pat = next_pattern(&pstate)){
	  /*
	   * This section wouldn't be necessary if sender was retreived
	   * from the envelope. But if not, we do need to add it.
	   */
	  if(pat->patgrp && pat->patgrp->sender)
	    add_eh(&q, &p, "sender", &alloced_size);

	  if(pat->patgrp && pat->patgrp->arbhdr)
	    for(a = pat->patgrp->arbhdr; a; a = a->next)
	      if(a->field && a->field[0] && a->p && non_eh(a->field))
		add_eh(&q, &p, a->field, &alloced_size);
      }
    
    set_extra_hdrs(q);
    if(q)
      fs_give((void **)&q);
}


int
non_eh(field)
    char *field;
{
    char **t;
    static char *existing[] = {"subject", "from", "to", "cc", "sender",
			       "reply-to", "in-reply-to", "message-id",
			       "path", "newsgroups", "followup-to",
			       "references", NULL};

    /*
     * If it is one of these, we should already have it
     * from the envelope or from the extra headers c-client
     * already adds to the list (hdrheader and hdrtrailer
     * in imap4r1.c, Aug 99, slh).
     */
    for(t = existing; *t; t++)
      if(!strucmp(field, *t))
	return(FALSE);

    return(TRUE);
}


/*
 * Add field to extra headers string if not already there.
 */
void
add_eh(start, ptr, field, asize)
    char **start;
    char **ptr;
    char  *field;
    int   *asize;
{
      char *s;

      /* already there? */
      for(s = *start; s = srchstr(s, field); s++)
	if(s[strlen(field)] == SPACE || s[strlen(field)] == '\0')
	  return;
    
      /* enough space for it? */
      while(strlen(field) + (*ptr - *start) + 1 > *asize){
	  (*asize) *= 2;
	  fs_resize((void **)start, (*asize)+1);
	  *ptr = *start + strlen(*start);
      }

      if(*ptr > *start)
	sstrcpy(ptr, " ");

      sstrcpy(ptr, field);
}


void
set_extra_hdrs(hdrs)
    char *hdrs;
{
    free_extra_hdrs();
    if(hdrs && *hdrs)
      extra_hdrs = cpystr(hdrs);
}


char *
get_extra_hdrs()
{
    return(extra_hdrs);
}


void
free_extra_hdrs()
{
    if(extra_hdrs)
      fs_give((void **)&extra_hdrs);
}


int
is_ascii_string(str)
    char *str;
{
    if(!str)
      return(0);
    
    while(*str && isascii(*str))
      str++;
    
    return(*str == '\0');
}


void
free_patline(patline)
    PAT_LINE_S **patline;
{
    if(patline && *patline){
	free_patline(&(*patline)->next);
	if((*patline)->filename)
	  fs_give((void **)&(*patline)->filename);
	if((*patline)->filepath)
	  fs_give((void **)&(*patline)->filepath);
	free_pat(&(*patline)->first);
	fs_give((void **)patline);
    }
}


void
free_pat(pat)
    PAT_S **pat;
{
    if(pat && *pat){
	free_pat(&(*pat)->next);
	free_patgrp(&(*pat)->patgrp);
	free_action(&(*pat)->action);
	fs_give((void **)pat);
    }
}


void
free_patgrp(patgrp)
    PATGRP_S **patgrp;
{
    if(patgrp && *patgrp){
	if((*patgrp)->nick)
	  fs_give((void **)&(*patgrp)->nick);
	free_pattern(&(*patgrp)->to);
	free_pattern(&(*patgrp)->cc);
	free_pattern(&(*patgrp)->recip);
	free_pattern(&(*patgrp)->partic);
	free_pattern(&(*patgrp)->from);
	free_pattern(&(*patgrp)->sender);
	free_pattern(&(*patgrp)->news);
	free_pattern(&(*patgrp)->subj);
	free_pattern(&(*patgrp)->alltext);
	free_pattern(&(*patgrp)->folder);
	free_arbhdr(&(*patgrp)->arbhdr);
	fs_give((void **)patgrp);
    }
}


void
free_pattern(pattern)
    PATTERN_S **pattern;
{
    if(pattern && *pattern){
	free_pattern(&(*pattern)->next);
	if((*pattern)->substring)
	  fs_give((void **)&(*pattern)->substring);
	fs_give((void **)pattern);
    }
}


void
free_arbhdr(arbhdr)
    ARBHDR_S **arbhdr;
{
    if(arbhdr && *arbhdr){
	free_arbhdr(&(*arbhdr)->next);
	if((*arbhdr)->field)
	  fs_give((void **)&(*arbhdr)->field);
	free_pattern(&(*arbhdr)->p);
	fs_give((void **)arbhdr);
    }
}


void
free_action(action)
    ACTION_S **action;
{
    if(action && *action){
	if((*action)->from)
	  mail_free_address(&(*action)->from);
	if((*action)->replyto)
	  mail_free_address(&(*action)->replyto);
	if((*action)->fcc)
	  fs_give((void **)&(*action)->fcc);
	if((*action)->litsig)
	  fs_give((void **)&(*action)->litsig);
	if((*action)->sig)
	  fs_give((void **)&(*action)->sig);
	if((*action)->template)
	  fs_give((void **)&(*action)->template);
	if((*action)->cstm)
	  free_list_array(&(*action)->cstm);
	if((*action)->nick)
	  fs_give((void **)&(*action)->nick);
	if((*action)->inherit_nick)
	  fs_give((void **)&(*action)->inherit_nick);
	if((*action)->incol)
	  free_color_pair(&(*action)->incol);
	if((*action)->folder)
	  free_pattern(&(*action)->folder);

	fs_give((void **)action);
    }
}


/*
 * Returns an allocated copy of the pat.
 *
 * Args   pat -- the source pat
 *
 * Returns a copy of pat.
 */
PAT_S *
copy_pat(pat)
    PAT_S *pat;
{
    PAT_S *new_pat = NULL;

    if(pat){
	new_pat = (PAT_S *)fs_get(sizeof(*new_pat));
	memset((void *)new_pat, 0, sizeof(*new_pat));

	new_pat->patgrp = copy_patgrp(pat->patgrp);
	new_pat->action = copy_action(pat->action);
    }

    return(new_pat);
}


/*
 * Returns an allocated copy of the patgrp.
 *
 * Args   patgrp -- the source patgrp
 *
 * Returns a copy of patgrp.
 */
PATGRP_S *
copy_patgrp(patgrp)
    PATGRP_S *patgrp;
{
    char     *p;
    PATGRP_S *new_patgrp = NULL;

    if(patgrp){
	new_patgrp = (PATGRP_S *)fs_get(sizeof(*new_patgrp));
	memset((void *)new_patgrp, 0, sizeof(*new_patgrp));

	if(patgrp->nick)
	  new_patgrp->nick = cpystr(patgrp->nick);
	
	if(patgrp->to){
	    p = pattern_to_string(patgrp->to);
	    new_patgrp->to = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->from){
	    p = pattern_to_string(patgrp->from);
	    new_patgrp->from = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->sender){
	    p = pattern_to_string(patgrp->sender);
	    new_patgrp->sender = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->cc){
	    p = pattern_to_string(patgrp->cc);
	    new_patgrp->cc = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->recip){
	    p = pattern_to_string(patgrp->recip);
	    new_patgrp->recip = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->partic){
	    p = pattern_to_string(patgrp->partic);
	    new_patgrp->partic = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->news){
	    p = pattern_to_string(patgrp->news);
	    new_patgrp->news = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->subj){
	    p = pattern_to_string(patgrp->subj);
	    new_patgrp->subj = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->alltext){
	    p = pattern_to_string(patgrp->alltext);
	    new_patgrp->alltext = string_to_pattern(p);
	    fs_give((void **)&p);
	}
	
	if(patgrp->arbhdr){
	    ARBHDR_S *aa, *a, *new_a;

	    aa = NULL;
	    for(a = patgrp->arbhdr; a; a = a->next){
		new_a = (ARBHDR_S *)fs_get(sizeof(*new_a));
		memset((void *)new_a, 0, sizeof(*new_a));

		if(a->field)
		  new_a->field = cpystr(a->field);

		if(a->p){
		    p = pattern_to_string(a->p);
		    new_a->p = string_to_pattern(p);
		    fs_give((void **)&p);
		}

		new_a->isemptyval = a->isemptyval;

		if(aa){
		    aa->next = new_a;
		    aa = aa->next;
		}
		else{
		    new_patgrp->arbhdr = new_a;
		    aa = new_patgrp->arbhdr;
		}
	    }
	}

	new_patgrp->fldr_type = patgrp->fldr_type;

	if(patgrp->folder){
	    p = pattern_to_string(patgrp->folder);
	    new_patgrp->folder = string_to_pattern(p);
	    fs_give((void **)&p);
	}

	new_patgrp->do_score  = patgrp->do_score;
	new_patgrp->score_min = patgrp->score_min;
	new_patgrp->score_max = patgrp->score_max;

	new_patgrp->stat_new  = patgrp->stat_new;
	new_patgrp->stat_del  = patgrp->stat_del;
	new_patgrp->stat_imp  = patgrp->stat_imp;
	new_patgrp->stat_ans  = patgrp->stat_ans;
    }

    return(new_patgrp);
}


/*
 * Returns an allocated copy of the action.
 *
 * Args   action -- the source action
 *
 * Returns a copy of action.
 */
ACTION_S *
copy_action(action)
    ACTION_S *action;
{
    ACTION_S *newaction = NULL;

    if(action){
	newaction = (ACTION_S *)fs_get(sizeof(*newaction));
	memset((void *)newaction, 0, sizeof(*newaction));

	newaction->repl_type   = action->repl_type;
	newaction->forw_type   = action->forw_type;
	newaction->comp_type   = action->comp_type;
	newaction->scoreval    = action->scoreval;
	newaction->move_only_if_not_deleted = action->move_only_if_not_deleted;
	newaction->is_a_role   = action->is_a_role;
	newaction->is_a_incol  = action->is_a_incol;
	newaction->is_a_score  = action->is_a_score;
	newaction->is_a_filter = action->is_a_filter;

	if(action->from)
	  newaction->from = copyaddr(action->from);
	if(action->replyto)
	  newaction->replyto = copyaddr(action->replyto);
	if(action->fcc)
	  newaction->fcc = cpystr(action->fcc);
	if(action->litsig)
	  newaction->litsig = cpystr(action->litsig);
	if(action->sig)
	  newaction->sig = cpystr(action->sig);
	if(action->template)
	  newaction->template = cpystr(action->template);
	if(action->nick)
	  newaction->nick = cpystr(action->nick);
	if(action->cstm)
	  newaction->cstm = copy_list_array(action->cstm);
	if(action->incol)
	  newaction->incol = new_color_pair(action->incol->fg,
					    action->incol->bg);
	if(action->inherit_nick)
	  newaction->inherit_nick = cpystr(action->inherit_nick);
	if(action->folder){
	    char *p = pattern_to_string(action->folder);
	    newaction->folder = string_to_pattern(p);
	    fs_give((void **) &p);
	}
    }

    return(newaction);
}


PATTERN_S *
copy_pattern(pattern)
    PATTERN_S *pattern;
{
    char      *p;
    PATTERN_S *new_pattern = NULL;

    if(pattern){
	p = pattern_to_string(pattern);
	new_pattern = string_to_pattern(p);
	fs_give((void **) &p);
    }

    return(new_pattern);
}


/*
 * Given a role, return an allocated role. If this role inherits from
 * another role, then do the correct inheriting so that the result is
 * the role we want to use. The inheriting that is done is just the set
 * of set- actions. This is for role stuff, no inheriting happens for scores
 * or for colors.
 *
 * Args   role -- The source role
 *
 * Returns a role.
 */
ACTION_S *
combine_inherited_role(role)
    ACTION_S *role;
{
    ACTION_S *newrole = NULL, *inherit_role = NULL;
    PAT_STATE pstate;

    if(role && role->is_a_role){
	newrole = (ACTION_S *)fs_get(sizeof(*newrole));
	memset((void *)newrole, 0, sizeof(*newrole));

	newrole->repl_type  = role->repl_type;
	newrole->forw_type  = role->forw_type;
	newrole->comp_type  = role->comp_type;
	newrole->is_a_role  = role->is_a_role;

	if(role->inherit_nick && role->inherit_nick[0] &&
	   nonempty_patterns(ROLE_DO_ROLES, &pstate)){
	    PAT_S    *pat;

	    /* find the inherit_nick pattern */
	    for(pat = first_pattern(&pstate);
		pat;
		pat = next_pattern(&pstate)){
		if(pat->patgrp &&
		   pat->patgrp->nick &&
		   !strucmp(role->inherit_nick, pat->patgrp->nick)){
		    /* found it, if it has a role, use it */
		    inherit_role = pat->action;
		    break;
		}
	    }
	}

	if(role->from)
	  newrole->from = copyaddr(role->from);
	else if(inherit_role && inherit_role->from)
	  newrole->from = copyaddr(inherit_role->from);

	if(role->replyto)
	  newrole->replyto = copyaddr(role->replyto);
	else if(inherit_role && inherit_role->replyto)
	  newrole->replyto = copyaddr(inherit_role->replyto);

	if(role->fcc)
	  newrole->fcc = cpystr(role->fcc);
	else if(inherit_role && inherit_role->fcc)
	  newrole->fcc = cpystr(inherit_role->fcc);

	if(role->litsig)
	  newrole->litsig = cpystr(role->litsig);
	else if(inherit_role && inherit_role->litsig)
	  newrole->litsig = cpystr(inherit_role->litsig);

	if(role->sig)
	  newrole->sig = cpystr(role->sig);
	else if(inherit_role && inherit_role->sig)
	  newrole->sig = cpystr(inherit_role->sig);

	if(role->template)
	  newrole->template = cpystr(role->template);
	else if(inherit_role && inherit_role->template)
	  newrole->template = cpystr(inherit_role->template);

	if(role->cstm)
	  newrole->cstm = copy_list_array(role->cstm);
	else if(inherit_role && inherit_role->cstm)
	  newrole->cstm = copy_list_array(inherit_role->cstm);

	if(role->nick)
	  newrole->nick = cpystr(role->nick);
    }

    return(newrole);
}


/*
 *  * * * * * * * *      RFC 2369 support routines      * * * * * * * *
 */

/*
 * * NOTE * These have to remain in sync with the MLCMD_* macros
 *	    in pine.h.  Sorry.
 */

static RFC2369FIELD_S rfc2369_fields[] = {
    {"List-Help",
     "get information about the list and instructions on how to join",
     "seek help"},
    {"List-Unsubscribe",
     "remove yourself from the list (Unsubscribe)",
     "UNsubscribe"},
    {"List-Subscribe",
     "add yourself to the list (Subscribe)",
     "Subscribe"},
    {"List-Post",
     "send a message to the entire list (Post)",
     "post a message"},
    {"List-Owner",
     "send a message to the list owner",
     "contact the list owner"},
    {"List-Archive",
     "view archive of messages sent to the list",
     "view the archive"}
};




char **
rfc2369_hdrs(hdrs)
    char **hdrs;
{
    int i;

    for(i = 0; i < MLCMD_COUNT; i++)
      hdrs[i] = rfc2369_fields[i].name;

    hdrs[i] = NULL;
    return(hdrs);
}



int
rfc2369_parse_fields(h, data)
    char      *h;
    RFC2369_S *data;
{
    char *ep, *nhp, *tp;
    int	  i, l, rv = FALSE;

    for(i = 0; i < MLCMD_COUNT; i++)
      data[i].field = rfc2369_fields[i];

    for(nhp = h; h; h = nhp){
	/* coerce h to start of field */
	for(ep = h;;)
	  if(tp = strpbrk(ep, "\015\012")){
	      if(strindex(" \t", *((ep = tp) + 2))){
		  *ep++ = ' ';		/* flatten continuation */
		  *ep++ = ' ';
		  for(; *ep; ep++)	/* advance past whitespace */
		    if(*ep == '\t')
		      *ep = ' ';
		    else if(*ep != ' ')
		      break;
	      }
	      else{
		  *ep = '\0';		/* tie off header data */
		  nhp = ep + 2;		/* start of next header */
		  break;
	      }
	  }
	  else{
	      while(*ep)		/* find the end of this line */
		ep++;

	      nhp = NULL;		/* no more header fields */
	      break;
	  }

	/* if length is within reason, see if we're interested */
	if(ep - h < MLCMD_REASON && rfc2369_parse(h, data))
	  rv = TRUE;
    }

    return(rv);
}


int
rfc2369_parse(h, data)
    char      *h;
    RFC2369_S *data;
{
    int   l, ifield, idata = 0;
    char *p, *p1, *url, *comment;

    /* look for interesting name's */
    for(ifield = 0; ifield < MLCMD_COUNT; ifield++)
      if(!struncmp(h, rfc2369_fields[ifield].name,
		   l = strlen(rfc2369_fields[ifield].name))
	 && *(h += l) == ':'){
	  /* unwrap any transport encodings */
	  if((p = (char *) rfc1522_decode((unsigned char *) tmp_20k_buf,
					  SIZEOF_20KBUF,
					  ++h, NULL)) == tmp_20k_buf)
	    strcpy(h, p);		/* assumption #383: decoding shrinks */

	  url = comment = NULL;
	  while(*h){
	      while(*h == ' ')
		h++;

	      switch(*h){
		case '<' :		/* URL */
		  if(p = strindex(h, '>')){
		      url = ++h;	/* remember where it starts */
		      *p = '\0';	/* tie it off */
		      h  = p + 1;	/* advance h */
		      for(p = p1 = url; *p1 = *p; p++)
			if(*p1 != ' ')
			  p1++;		/* remove whitespace ala RFC */
		  }
		  else
		    *h = '\0';		/* tie off junk */

		  break;

		case '(' :			/* Comment */
		  comment = rfc822_skip_comment(&h, LONGT);
		  break;

		case 'N' :			/* special case? */
		case 'n' :
		  if(ifield == MLCMD_POST
		     && (*(h+1) == 'O' || *(h+1) == 'o')
		     && (!*(h+2) || *(h+2) == ' ')){
		      ;			/* yup! */

		      url = h;
		      *(h + 2) = '\0';
		      h += 3;
		      break;
		  }

		default :
		  removing_trailing_white_space(h);
		  if(!url
		     && (url = rfc1738_scan(h, &l))
		     && url == h && l == strlen(h)){
		      removing_trailing_white_space(h);
		      data[ifield].data[idata].value = url;
		  }
		  else
		    data[ifield].data[idata].error = h;

		  return(1);		/* return junk */
	      }

	      while(*h == ' ')
		h++;

	      switch(*h){
		case ',' :
		  h++;

		case '\0':
		  if(url || (comment && *comment)){
		      data[ifield].data[idata].value = url;
		      data[ifield].data[idata].comment = comment;
		      url = comment = NULL;
		  }

		  if(++idata == MLCMD_MAXDATA)
		    *h = '\0';

		default :
		  break;
	      }
	  }
      }

    return(idata);
}
