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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v $
 *      $Author: tjcoppet $
 *
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/string_utils.c,v 1.1 1989-07-07 13:20:53 tjcoppet Exp $";
#endif

#include <olc/olc.h>

#include <ctype.h>              /* Character type macros. */
#include <signal.h>             /* System signal definitions. */
#include <sys/time.h>           /* System time definitions. */


/*
 * Function:	uncase() converts a string to lower case letters. It also
 *			strips leading whitespace from the string.
 * Arguments:	string:	The string to be converted.  It is converted
 *			in place.
 * Returns:	Nothing.
 * Notes:
 */

uncase(string)
     char *string;
{
  char *buf;		/* Temporary buffer. */
  char *bufp;		/* Pointer within buffer. */
  int i;		/* Index variable. */
	
  i = 0;
  buf = malloc((unsigned) (strlen(string) + 1));
  bufp = string;
  while (isspace(*bufp))
    bufp++;
  while ((*bufp != '\0'))
    {
      if (isupper(*bufp))
	buf[i] = tolower(*bufp);
      else buf[i] = *bufp;
      i++;
      bufp++;
    }
  buf[i] = '\0';
  (void) strcpy(string, buf);
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

time_now(time_buf)
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
get_next_word(line, buf) 
	char *line, *buf;
{
	char c;			/* Current character. */
	
	while ((c = *line) == ' ' || c == '\t')
		line++;
	if (c == '\0' || c == '\n') {
		*buf = '\0';
		return((char *)NULL);
	}
	while ((c = *line) != ' ' && c != '\t' && c != '\n' && c != '\0') {
		line++;
		*buf++ = c;
	}
	*buf = '\0';
	return(line);
}



char *
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
 */

ERRCODE
parse_command_line(command_line, arguments)
     char *command_line;
     char arguments[MAX_ARGS][MAX_ARG_LENGTH];
{
  char *current;		/* Current place in the command line.*/
  int argcount;		        /* Running counter of arguments. */
  char *get_next_word();	/* Get the next word in a line. */
  char buf[BUFSIZE];
	
  current = command_line;
  argcount = 0;
  while ((current = get_next_word(current, buf)) != (char *) NULL) 
    {
      (void) strcpy(arguments[argcount],buf);
      argcount++;
    }
  
  *arguments[argcount] = '\0';
  return(SUCCESS);
}



/*
 * Function:    make_temp_name() creates a temporary file name using the
 *                      current time.
 * Arguments:   name:   A pointer to space for the name.
 * Returns:     Nothing.
 * Notes:
 *      Get the process ID from the system and create a filename from it.
 */

static int joe = 0;

make_temp_name(name)
        char *name;
{
        (void) sprintf(name, "/tmp/OLC%d.%d", getpid(), joe++);
        (void) unlink(name);    /* just to be sure */
}

