/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions common to the different programs.
 *
 *      Win Treese
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v $
 *	$Id: string_utils.c,v 1.16 1991-05-06 23:10:47 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v 1.16 1991-05-06 23:10:47 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */


/*
 * Function:	uncase() converts a string to lower case letters. It also
 *			strips leading whitespace from the string.
 * Arguments:	string:	The string to be converted.  It is converted
 *			in place.
 * Returns:	Nothing.
 * Notes:
 */

void uncase(string)
    char *string;
{
    char *s1 = string;

    while (isspace (*string))
	string++;
    while (*string) {
	*s1++ = isupper (*string) ? tolower (*string) : *string;
	string++;
    }
    *s1 = '\0';
}

void upcase_string(string)
     char *string;
{
  char *s1 = string;
  while(*s1 != '\0') {
    if (islower(*s1)) *s1 = toupper(*s1);
    s1++;
  }
}

char *
cap(string)
     char *string;
{
    static char buf[LINE_SIZE];
    char c;

    strncpy(buf,string,LINE_SIZE);
    c = buf[0];
    if(!isupper (c))
	buf[0] = toupper (c);
    return buf;
}

int isnumber(string)
     char *string;
{
    if (!string)
	return SUCCESS;
    while (*string) {
	if(!isdigit(*string))
	    return ERROR;
	++string;
    }
    return(SUCCESS);
}


/*
 * Function: get_next_word() gets the next word out of a line of text
 *		contained in the string <line>.  It takes optional leading
 *		whitespace and then returns a string containing all of
 *		the characters until the next delimiter.
 * Arguments:	line:	A pointer to the text to be scanned.
 *		buf:	A pointer to a buffer to store the word in.
 *		func:	A function that returns FALSE when you've hit a
 * 			delimiter.  Two are provided below, IsAlpha()
 *			and NotWhiteSpace().
 * Returns:	A pointer after the end of the word.
 * Notes:
 * 	Scan through <line> looking for the first non-whitespace character.
 *	If it's a NULL or CR, return NULL.  Otherwise, copy characters into
 *	<buf> until we find more whitespace.  Then return <line>.
 */

char *
get_next_word(line, buf, func)
     char *line, *buf;
#ifdef __STDC__
     int (*func)(int c);
#else
     int (*func)();
#endif
{
  char c;			/* Current character. */

  while ((c = *line) == ' ' || c == '\t')	/* strip leading whitepace */
    line++;
  if (c == '\0' || c == '\n') { /* degenerate case -- nothing to parse */
    *buf = '\0';
    return((char *)NULL);
  }

  while (func(c = *line) && c != '\0')
    {
      line++;
      *buf++ = c;
    }

  *buf = '\0';
  return(line);
}

int
IsAlpha(c)
     int c;
{
  return(isalpha(c));
}

int
NotWhiteSpace(c)
     int c;
{
  return(c != ' ' && c != '\t' && c != '\n');
}

/*
 * Function:    make_temp_name() creates a temporary file name using the
 *                      process id and a static counter.
 * Arguments:   name:   A pointer to space for the name.
 * Returns:     Nothing.
 * Notes:
 *      Get the process ID from the system and create a filename from it.
 */

void make_temp_name(name)
    char *name;
{
  static int counter = 0;

  (void) sprintf(name, "/tmp/OLC%d.%d", getpid(), counter++);
  (void) unlink(name);		/* just to be sure */
}
