/*
 * This file is part of the OLC On-Line Consulting System.
 * It deals with reading and parsing configuration files.
 *
 *      bert Dvornik
 *      MIT Athena Software Service
 *
 * Copyright (C) 1996 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: configure.c,v 1.2 1999-01-22 23:11:56 ghudson Exp $
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[] = "$Id: configure.c,v 1.2 1999-01-22 23:11:56 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include <cfgfile/configure.h>

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>

#define CONFIG_LINE_CHUNK 1024

#define COMMENT_CHAR	'#'
#define OPEN_STRING	'"'
#define CLOSE_STRING	'"'
#define QUOTE_CHAR	'\\'

static char *prog_name = "(???)";
static char free_prog_name = 0;

/*** Finding files ***/

/* Find a file with a given name and extension by searching through
 * all directories specified in a given colon-delimited path.
 * Arguments:   name:  base name of the file.
 *              ext:   filename extension (including the period).
 *              path:  a colon-separated path.
 * Returns: a file descriptor for the file on success, NULL on failure.
 * Non-local returns: exits with exit code 1 if out of memory.
 */
FILE *cfg_fopen_in_path(const char *name, const char *ext,
			const char *path)
{
  char *buf;
  const char *start, *end;
  FILE *ret;
  int len;

  /* This is likely too much space, but it means we won't have to realloc. */
  buf = malloc(strlen(path)+strlen(name)+strlen(ext)+2);
  if (buf == NULL)
    {
      fprintf(stderr, "cfg_fopen_in_path: out of memory!\n");
      exit(1);
    }

  for (start = end = path ; end ; start = end+1)
    {
      /* start contains the first char of the current path directory */
      end = strchr(start, ':');
      if (end == NULL)
	len = strlen(start);
      else
	len = end-start;

      strncpy(buf, start, len);
      if ((len > 0) && (buf[len-1] != '/'))
	buf[len++] = '/';
      buf[len] = '\0';
      strcat(buf, name);
      strcat(buf, ext);

      /* buf now contains "path_fragment/name.ext".  Try using it. */
      ret = fopen(buf, "r");
      if (ret)
	{
	  free(buf);
	  return ret;
	}
    }

  free(buf);
  return NULL;
}

/*** Utility functions for reading the configuration file ***/

/* Read a string delimited by OPEN_STRING and CLOSE_STRING ("").
 * To embed delimiters in the string, quote them using QUOTE_CHAR (\).
 * Quoting also works as expected on \\, \n, and \t.
 * Arguments:   pos: pointer to the start of the data to be parsed.
 *              string: location where the data is to be stored.
 *              line: line number in the configuration file (for error msgs).
 * Side effects: *string is set to point to a newly malloc()'d area
 *                 containing the parsed data; the old value is discarded.
 * Returns: Pointer to the unparsed part of the string (which will
 *    point to the final '\0' if everything was parsed).
 * Non-local returns: exits with exit code 1 if out of memory.
 * Note: this code is rather ugly, but would be hard to clean up because it
 *    deals with too many screw cases.  Unless you are better than I am. =)
 */
static char *read_quoted_string(char *pos, char **string, int line)
{
  char *buf;
  char *bufpos;
  char complained = 0;

  /* The constructed string will be at most as long as the line. */
  buf = malloc(strlen(pos)+1);
  if (buf == NULL)
    {
      fprintf(stderr, "%s: configuration: out of memory!\n", prog_name);
      exit(1);
    }

  bufpos = buf;
  if (*pos == OPEN_STRING)
    pos++;
  else
    {
      if (*pos == '\0')
	fprintf(stderr,
		"%s: configuration [line %d]: missing string argument\n",
		prog_name, line);
      else if (isprint(*pos))
	fprintf(stderr,
		"%s: configuration [line %d]: string starts with '%c',"
		" not '%c'\n", prog_name, line, *pos, OPEN_STRING);
      else
	fprintf(stderr,
		"%s: configuration [line %d]: string starts with '\%03o',"
		" not '%c'\n", prog_name, line, *pos, OPEN_STRING);
    }

  /* Go through the body of the string and use it to fill the buffer. */
  while ((*pos != '\0') && (*pos != CLOSE_STRING))
    {
      switch (*pos)
	{
	case QUOTE_CHAR:    /* deal with quoted characters... */
	  pos++;
	  switch (*pos)
	    {
	    case '\0':		/* this means \EOF, or  we'd see '\n' first */
	      fprintf(stderr, "%s: configuration [line %d]: string ends with"
		      "a quote at the end of file, instead of with '%c'\n",
		      prog_name, line, CLOSE_STRING);
	      complained = 1;
	      break;
	    case '\n':		/* \ at the end-of-line */
	      fprintf(stderr, "%s: configuration [line %d]: string ends with"
		      " a '%c' instead of with '%c'\n"
		      "   (use %cn to embed newlines)\n",
		      prog_name, line, QUOTE_CHAR, CLOSE_STRING, QUOTE_CHAR);
	      complained = 1;
	      break;
	    case 'n':		/* \n */
	      *(bufpos++) = '\n';
	      break;
	    case 'r':		/* \r */
	      *(bufpos++) = '\r';
	      break;
	    case 't':		/* \t */
	      *(bufpos++) = '\t';
	      break;
	    default:		/* \foo -> foo (\\ -> \ , \" -> ", etc) */
	      *(bufpos++) = *pos;
	      break;
	    }
	  break;
	case '\n':	    /* end-of-line */
	  fprintf(stderr, "%s: configuration [line %d]: string ends at the end"
		  " of line instead of with '%c'\n",
		  prog_name, line, CLOSE_STRING);
	  complained = 1;
	  break;
	default:	    /* anything else */
	  *(bufpos++) = *pos;
	  break;
	}
      if (*pos)
	pos++;  /* don't advance the pointer past the final \0 */
    }

  if (*pos == CLOSE_STRING)
    pos++;
  else
    {
      /* If there's no final quote, we may have already complained. */
      if (! complained)
	fprintf(stderr, "%s: configuration [line %d]: string ends at the end"
		" of file instead of with '%c'\n",
		prog_name, line, CLOSE_STRING);
    }


  /* put a copy of the string data into *string */
  *bufpos = '\0';
  *string = malloc(bufpos - buf + 1);
  if (*string == NULL)
    {
      fprintf(stderr, "%s: configuration: out of memory!\n", prog_name);
      exit(1);
    }
  strcpy(*string, buf);
  free(buf);

  return pos;
}

/* Read a line of arbitrary length from an I/O stream.
 * Arguments:   buf:  Address of a char* variable used as a buffer.  The
 *                    variable should be initialized to NULL or a malloc'd
 *                    buffer before the first call to this function, and
 *                    can simply be re-used later, but the buffer will be
 *                    overwritten and reallocated at will.  The value
 *                    should be free()'d after the last call to fget_line.
 *              size: Address of a size_t variable containing the size of
 *                    *buf.  The variable should be initialized to 0 if
 *                    *buf is NULL, or whatever the allocation size of *buf
 *                    is, otherwise.  It will be modified whenever buf is
 *                    grown.
 *              fd:   the stream to read.  (Must be open for reading!)
 * Returns: A pointer to the data read (same as *buf), or NULL if an error
 *          or EOF occurs and no data is available.
 * Non-local returns: exits with exit code 1 if out of memory.
 */

static char *fget_line (char **buf, size_t *size, FILE *fd)
{
  size_t len;

  /* If we don't have a buffer yet, get one. */
  if (*size == 0)
    {
      *size = CONFIG_LINE_CHUNK;
      *buf = malloc(*size);
      if (*buf == NULL)
	{
	  fprintf(stderr, "%s: configuration: out of memory!\n", prog_name);
	  exit(1);
	}
    }

  /* Read some data, returning NULL if we fail. */
  if (fgets(*buf, *size, fd) == NULL)
    return NULL;

 
  while (1)
    {
      len = strlen(*buf);
      /* If the string is empty, return NULL.
       * (This shouldn't ever happen unless fd contains '\0' characters.)
       */
      if (len == 0)
	return NULL;
      /* If the string contains a full line, return that. */
      if ((*buf)[len-1] == '\n')
	return *buf;
      /* If the line is shorter than *buf is, we ran into EOF; return data. */
      if (len+1 < *size)
	return *buf;

      /* OK, the only other option is that our line was too short... */
      *size += CONFIG_LINE_CHUNK;
      *buf = realloc(*buf, *size);
      if (*buf == NULL)
	{
	  fprintf(stderr, "%s: configuration: out of memory!\n", prog_name);
	  exit(1);
	}

      /* Try reading more data and see if we fare better. */
      if (fgets((*buf)+len, (*size)-len, fd) == NULL)
	return *buf;
    }
}

/* Find the first whitespace character in a string.
 * Arguments:   str:  the string to examine.
 * Returns:   a pointer to the first character in str for which isspace()
 *            returns true.  If there are no such characters, returns a
 *            pointer to the '\0' character terminating the string.
 * Note: this is similar to `str + strcspn(str, " \t\r\n\v\f")'.
 */
static char *first_isspace(char *str)
{
  while (*str && !isspace(*str))
    str++;
  return str;
}

/* Find the first non-whitespace character in a string.
 * Arguments:   str:  the string to examine.
 * Returns:   a pointer to the first character in str for which isspace()
 *            returns false.  If there are no such characters, returns a
 *            pointer to the '\0' character terminating the string.
 * Note: this is similar to `str + strspn(str, " \t\r\n\v\f")'.
 */
static char *first_nonspace(char *str)
{
  while (*str && isspace(*str))
    str++;
  return str;
}

/*** Top level of the parser ***/

/* Read the configuration file and parse lines of the form "keyword [arg(s)]".
 * Arguments:   name: program name to display in errors.
 *              cf:  the stream to read.  (Must be open for reading!)
 *              keywords:  an array of structures specifying what arguments
 *                should be expected for each keyword, and where to store
 *                the data specified by the arguments.
 * Returns: none.
 * Non-local returns: may exit with exit code 1 if out of memory.
 */
void cfg_read_config(const char *name, FILE *cf, config_keyword *keywords)
{
  char *buf = NULL;
  size_t bufsize = 0;
  char *key, *end, *args;
  config_keyword *entry;
  int line = 0;

  if (free_prog_name)
    free(prog_name);
  prog_name = malloc(strlen(name)+1);
  if (prog_name == NULL)
    {
      fprintf(stderr, "%s: configuration: out of memory!\n", name);
      exit(1);
    }
  free_prog_name = 1;
  strcpy(prog_name, name);

  while (fget_line(&buf, &bufsize, cf))
    {
      line++;

      key =  first_nonspace(buf);    /* next non-whitespace char */

      if ((*key == '\0') || (*key == COMMENT_CHAR))
	continue;       /* line is blank or starts with #, go on to the next */

      end =  first_isspace (key);    /* next whitespace char */
      args = first_nonspace(end);    /* next non-whitespace char */
      *end = '\0';

      /* find config_keyword table entry matching key */
      for (entry = keywords ;
	   entry->keyword && strcasecmp(key, entry->keyword) ;
	   entry++)
	{}

      if (! entry->keyword)
	{
	  fprintf(stderr,
		  "%s: configuration [line %d]: unknown keyword '%s'\n",
		  prog_name, line, key);
	  continue;
	}

      /* Run the function from the matching entry to extract the arguments. */
      args = (*(entry->parse))(args, entry->placement, entry->value, line);

      /* Make sure that the rest of the line is whitespace only. */
      args = first_nonspace(args);   /* next non-whitespace char */
      if (*args)
	fprintf(stderr, "%s: configuration [line %d]: too many arguments"
		" after keyword '%s'\n", prog_name, line, key);
    }

  if (buf)
    free(buf);
}

/*** Argument-parsing functions ***/

/* All functions in this section are of the type "config_set" (defined in
 * configure.c), and take the following four arguments and return value:
 *
 * Arguments:   input: pointer to the string which is to be parsed by
 *                this function.
 *              variable_arg: pointer to a location where the parsed
 *                data should be stored.  The type of the parsed data
 *                (and hence the type of the storage location) depends
 *                on which function is used.  Since there is no good
 *                way to let the compiler do the type checking, we
 *                have to hope the programmer supplied a correctly
 *                typed location here! =)
 *              value: an additional parameter passed to the function,
 *                ignored by most, but used by config_set_char_const.
 *              line: line number in the configuration file (for error msgs).
 * Returns: Pointer to the unparsed part of the string (which will
 *    point to the final '\0' if everything was parsed).
 */


/* Read a string delimited by OPEN_STRING and CLOSE_STRING ("").
 * To embed delimiters in the string, quote them using QUOTE_CHAR (\).
 * Quoting also works as expected on \\, \n, and \t.
 * Arguments:   (see start of this section)
 *              variable_arg: a polymorphic interface to "char **variable"
 *              [variable: pointer to a char* location which will be
 *                 modified to contain the parsed data.]
 * Side effects: If *variable isn't NULL, the old value is free()d
 *                 (which will break if it wasn't malloc()d).
 *               *variable is set to point to a newly malloc()'d area
 *                 containing the parsed data.
 * Returns:   (see start of this section)
 * Non-local returns: (indirectly) exits with exit code 1 if out of memory.
 */
char *config_set_quoted_string(char *input, void *variable_arg,
			      long dummy, int line)
{
  char **variable = (char**) variable_arg;

  if (*variable)
    {
      fprintf(stderr, "%s: configuration [line %d]: value already set!"
	      " (using last value specified)\n", prog_name, line);
      /* If we don't free() the old value, we will memory-leak a little.
       * On the other hand, if we *do* free() and the caller has set the
       * value to smoething that wasn't malloc'd (or left it uninitialized),
       * many malloc libraries will dump core or, worse, start behaving
       * really strangely.  [Moral: *initialize your variables*. =)]
       */
#ifndef CFGFILE_DONT_FREE
      free(*variable);
#endif /* not CFGFILE_DONT_FREE */
    }

  /* Read new value and return the error of parsed data. */
  return read_quoted_string(input, variable, line);
}

/* Set a numeric char constant to the value specified in the call.
 * (Nothing is actually parsed.)
 * Arguments:   (see start of this section)
 *              variable_arg: a polymorphic interface to "char *variable"
 *              [variable: pointer to a char location which will be
 *                 modified to contain the data from "value".]
 *              value: the value to set variable to.
 * Side effects: *variable is set to (char)value.
 * Returns:   (see start of this section)
 */
char *config_set_char_constant(char *input, void *variable_arg, long value,
			       int line)
{
  char *variable = (char*) variable_arg;
  *variable = (char) value;
  return input;
}

/* Read a string delimited by OPEN_STRING and CLOSE_STRING (""), and
 * append it to the end of a (possibly empty) NULL-delimited list of strings.
 * Arguments:   (see start of this section)
 *              variable_arg: a polymorphic interface to "char ***variable"
 *              [variable: pointer to a variable referencing a NULL-
 *                 terminated array of char*'s.  The list will be appended
 *                 to include the value specified.]
 * Side effects: *variable is appended to contain a pointer to a newly
 *                 malloc()'d string, read from the configuration.
 *                 If *variable is NULL, the list is created.
 * Returns:   (see start of this section)
 * Non-local returns: exits with exit code 1 if out of memory.
 */
char *config_set_string_list(char *input, void *variable_arg,
			     long dummy, int line)
{
  int nelem;
  char ***variable = (char***) variable_arg;
  char **list = *variable;

  if (list)
    {
      /* The list already exists; we need to realloc() it. */
      for (nelem = 0; list[nelem]; nelem++);	/* count the elements */

      list = realloc(list, (nelem + 2) * sizeof(char*));
    }
  else
    {
      /* The list needs to be allocated. */
      nelem = 0;

      list = malloc(2 * sizeof(char*));
    }

  if (list == NULL)
    {
      fprintf(stderr, "%s: configuration: out of memory! (growing list)\n",
	      prog_name);
      exit(1);
    }

  /* We need to fill in the next list element, null-terminate the list,
   * set *variable to list, and return the end of parsed data.
   * We do this out of order to avoid using an extra variable.
   */
  list[nelem+1] = NULL;
  *variable = list;
  return read_quoted_string(input, &list[nelem], line);
}

