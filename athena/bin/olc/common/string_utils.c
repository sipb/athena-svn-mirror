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
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v $
 *	$Id: string_utils.c,v 1.11 1990-07-16 08:25:45 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v 1.11 1990-07-16 08:25:45 lwvanels Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */

char *wday[] = 
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
} ;

char *month[] = 
{
  "January", "February", "March", "April", "May", "June", 
  "July", "August", "September", "October", "November", "December",
};

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

extern struct tm *localtime();

/*
 * Function:	time_now() returns a formatted string with the current time.
 * Arguments:	time_buf:	Buffer to store formatted time.
 * Returns:	Nothing.
 * Notes:
 *	First, get the current time and format it into a readable form.  Then
 *	copy the hour and minutes into the front of the buffer, retaining only
 *	the time through the minutes.
 */

#if 0
void time_now_old(time_buf)
     char *time_buf;		/* should be at least 18 chars */
{
  long current_time;     	/* Current time. */
  struct tm *time_info;

  (void) time(&current_time);
  time_info = localtime(&current_time);
  (void) sprintf(time_buf, "%02d/%02d/%02d %02d:%02d:%02d",
	  time_info->tm_year, time_info->tm_mon+1, time_info->tm_mday,
	  time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
}
#endif

void time_now(time_buf)
     char *time_buf;
{
  long current_time;     	/* Current time. */
  struct tm *time_info;

  (void) time(&current_time);
  time_info = localtime(&current_time);
  strcpy(time_buf, format_time(time_info));
}

char *format_time(time_info)
     struct tm *time_info;
{
  char time_buf[BUF_SIZE];
  int hour;

  hour = time_info->tm_hour;

  if(hour > 12)  hour -= 12;
  if(hour == 0)  hour = 12;	/* If it's the midnight hour... */

  (void) sprintf(time_buf, "%3.3s %s%d-%3.3s-%s%d %s%d:%s%d%s",
		 wday[time_info->tm_wday],
		 time_info->tm_mday > 9 ? "" : "0", 
		 time_info->tm_mday,
		 month[time_info->tm_mon], 
		 time_info->tm_year > 9 ? "" : "0",
		 time_info->tm_year,
		 hour > 9 ? "" : " ",
		 hour,
		 time_info->tm_min > 9 ? "" : "0", 
		 time_info->tm_min,
		 time_info->tm_hour > 11 ? "pm" : "am");
  return(time_buf);
}


/*
 * Function: get_next_word() gets the next word out of a line of text
 *		contained in the string <line>.  It takes optional leading
 *		whitespace and then returns a string containing all of
 *		the characters until the next whitespace.
 * Arguments:	line:	A pointer to the text to be scanned.
 *		buf:	A pointer to a buffer to store the word in.
 * Returns:	A pointer after the end of the word.
 * Notes:
 * 	Scan through <line> looking for the first non-whitespace character.
 *	If it's a NULL or CR, return NULL.  Otherwise, copy characters into
 *	<buf> until we find more whitespace.  Then return <line>.
 */

char *
get_next_word(line, buf, func)
     char *line, *buf;
     int (*func)();
{
  char c;			/* Current character. */
  int done = FALSE;		/* TRUE when we have hit the next word. */
  int i;

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

#if 0
    for (i=0; (i < strlen(separators)) && !done; i++)
    if ((c == separators[i]) || (c == '\0'))
      done = TRUE;
    else {
      *buf++ = c;
      c = *(++line);
      i = 0;
    }
#endif
#if 0
	while ((c = *line) != ' ' && c != '\t' && c != '\n' && c != '\0')
	  {
	    line++;
	    *buf++ = c;
	  }
#endif

	*buf = '\0';
	return(line);
}

int
IsAlpha(c)
     char c;
{
  return(isalpha(c));
}

int
NotWhiteSpace(c)
     char (c);
{
  return(c != ' ' && c != '\t' && c != '\n');
}

#if 0 /* unused */
static char *
get_next_line(line, buf) 
     char *line, *buf;
{
  char c;			
  
  if(*line == '\0' || *line == NULL || (*line == '\n' && *(line+1) == '\0'))
    return((char *) NULL);

  while ((c = *line) != '\n' && c != '\0') 
    {
      line++;
      *buf++ = c;
    }
  if(c=='\0')
    *line = '\0';
  *buf = '\0';
  return(line);
}
#endif


#define MAX_ARGS         20             /* Maximum number of arguments. */
#define MAX_ARG_LENGTH   80 

/*
 * Function: parse_command_line() parses a command line into individual words.
 * Arguments:	command_line:	A pointer to the command line string.
 *		arguments:	A pointer to an array that will contain the
 *				parsed arguments.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	Get each word in the command line and set up a pointer to it.
 *      Kludge it so phrases can be quoted.
 */

ERRCODE
parse_command_line(command_line, arguments)
     char *command_line;
     char arguments[MAX_ARGS][MAX_ARG_LENGTH];
{
  char *current;		/* Current place in the command line.*/
  int argcount;		        /* Running counter of arguments. */
  char *get_next_word();	/* Get the next word in a line. */
  char buf[BUF_SIZE];
  char *bufP;
  int quote = 0;
  int first = 1;

  current = command_line;
  argcount = 0;
  while ((current = get_next_word(current, buf, NotWhiteSpace))
	 != (char *) NULL)
    {
      if(buf[0] == '\"')
	{
	  quote = TRUE;
	  bufP = &buf[1];
	}
      else
	bufP = &buf[0];

      if(quote)
	{
	  if(bufP[strlen(bufP)-1] == '\"')
	    {
	      bufP[strlen(bufP)-1] = '\0';
	     (void) strcat(arguments[argcount],bufP);
	      quote = FALSE;
	      first = 1;
	      ++argcount;
	    }
	  else
	    {
	      if(first)
		{
		  (void) strcpy(arguments[argcount],bufP);
		  first = 0;
		}
	      else
		(void) strcat(arguments[argcount],bufP);

	      (void) strcat(arguments[argcount]," ");
	    }
	}
      else
	{
	  (void) strcpy(arguments[argcount],bufP);
	  argcount++;
	}
    }
  
  *arguments[argcount] = '\0';
  return(SUCCESS);
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
