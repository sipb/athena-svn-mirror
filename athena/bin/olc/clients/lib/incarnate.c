/*
 * This file is part of the OLC On-Line Consulting System.
 * It deals with customizing the client for different services (olc, olta, owl)
 *
 *      bert Dvornik
 *      MIT Athena Software Service
 *
 * Copyright (C) 1996 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/incarnate.c,v $
 *	$Id: incarnate.c,v 1.1 1997-04-30 17:34:30 ghudson Exp $
 *	$Author: ghudson $
 */

#if !defined(SABER) && !defined(lint)
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/incarnate.c,v 1.1 1997-04-30 17:34:30 ghudson Exp $";
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <cfgfile/configure.h>

#include <string.h>
#include <sys/param.h>
#include <ctype.h>

/*** global variables ***/

/* The following variables contain all incarnation data.  The variables are
 * intentionally declared as a static globals; you should be using the
 * accessor functions, making it easier to change the implementation
 * details of this file.
 */

/* Name of the client (olc/olcr/olta/oltr/...) */
static char  *clt_name       = NULL;

/* Which OL* service are we using? (OLC/OLTA/OWL) [should be in uppercase!] */
static char  *service_name   = NULL;

/* Fallback server host (or the default host used, if we don't have Hesiod). */
static char  *default_server = NULL;

/* Default prompt to give to the user. */
static char  *prompt         = NULL;

/* Default title for a person answering questions. (consultant/TA/...)*/
static char  *consult_title  = NULL;

/* Directory where the help files live. */
static char  *help_dir       = NULL;
/* Name of the "root" help file (the one displayed for "help" with no args). */
static char  *help_file      = NULL;
/* Filename extension for the help files. (probably ".help") */
static char  *help_ext       = NULL;

/* Directory under which the stock answers live. */
static char  *stock_dir      = NULL;
/* Browser executable for the stock answers. */
static char  *stock_browser  = NULL;
/* Command(s) which are executed to attach the stock answers. */
static char **stock_attach   = NULL;
/* File whose existence is used to check whether attach succeeded or not. */
static char  *stock_magic    = NULL;

/* Are we playing a consulting (...r) client? */
static char is_consulting    = 0;

/* Does the service support the "hours" command? */
static char has_hours        = 1;

/* The following keeps track of whether we have completed initialization yet.
 */

static char unincarnated = 1;   /* Keep track of whether we've initialized. */
                                /* (1==none, -1==client_name, 0==everything) */
static void karma(char*);       /* Function to complain if un-initialized. */

/* The following variable contains data which automates the parsing of
 * the configuration file.  Each line contains
 *     "KEYWORD",  PARSE_FUNCTION,  &DATA [, VALUE]
 *
 * "KEYWORD" is the keyword to be defined for the configuration.
 * PARSE_FUNCTION is the function that will parse out the argument(s)
 *    (if any) to this keyword.  Reasonable values include:
 *
 *   config_set_quoted_string:
 *      Read a quoted string and store it in DATA, which should be a char*
 *      initialized to NULL.  The keyword should be only used once per file.
 *   config_set_string_list:
 *      Read a quoted string and append it to the list of strings in DATA,
 *      which should be a char**.  May be repeated to create list.
 *   config_set_char_constant:
 *      Set DATA, which should be a single-character constant, to VALUE.
 *      Does not take any arguments.  The keyword should only be used once
 *      per configuration file, but this isn't checked.
 *
 * &DATA is a pointer to the location where the data is to be stored.
 *    For our purposes, this will be one of the fields of Incarnation.
 *    Since compiler type-checking is impossible here (because this
 *    field must take values of different types), make sure that in any
 *    fields you add, type of DATA matches that required by PARSE_FUNCTION.
 * VALUE is an additional value pased to the parse_function.  Most
 *    functions ignore this, but config_set_char_constant uses it.
 *    If unused, it should be omitted for clarity (it defaults to 0).
 */
static config_keyword olxx_config[] = {
  {"service",		config_set_quoted_string, &service_name},
#ifdef HESIOD
  {"fallback_server",	config_set_quoted_string, &default_server},
#else
  {"server",		config_set_quoted_string, &default_server},
#endif
  {"prompt",		config_set_quoted_string, &prompt},
  {"consultant_title",	config_set_quoted_string, &consult_title},

  {"help_dir",		config_set_quoted_string, &help_dir},
  {"help_name",		config_set_quoted_string, &help_file},
  {"help_ext",		config_set_quoted_string, &help_ext},
  {"stock_dir",		config_set_quoted_string, &stock_dir},
  {"stock_browser",	config_set_quoted_string, &stock_browser},
  {"stock_attach",	config_set_string_list,	  &stock_attach},
  {"stock_magic",	config_set_quoted_string, &stock_magic},

  {"answer_client",	config_set_char_constant, &is_consulting, 1},
  {"no_hours",		config_set_char_constant, &has_hours,	  0},
  {NULL,		NULL}
};

/*** exported accessor functions ***/

/* Return the name of the current client (most likely in lowercase). */
char *client_name(void)
{
  if (unincarnated > 0)  karma("client_name() called before incarnation");
  return clt_name;
}

/* Return the name of the OLxx service we're connecting to, in uppercase.
 * Note: Hesiod doesn't care about the case of the service names, but "OLC
 *	server" looks better than "olc server" in output.
 */
char *client_service_name(void)
{
  if (unincarnated)  karma(NULL);
  return service_name;
}

/* Return the fallback for the server hostname.
 * (On sites without Hesiod, this is the only value used.)
 */
char *client_hardcoded_server(void)
{
  if (unincarnated)  karma(NULL);
  return default_server;
}

/* Return true iff this is a "consulting" (as opposed to "user") client. */
char client_is_consulting_client(void)
{
  if (unincarnated)  karma(NULL);
  return is_consulting;
}

/* Return true iff this is client has posted hours. */
char client_has_hours(void)
{
  if (unincarnated)  karma(NULL);
  return has_hours;
}

/* Return the default prompt (for text clients). */
char *client_default_prompt(void)
{
  if (unincarnated)  karma(NULL);
  return prompt;
}

/* Return the default consultant title. */
char *client_default_consultant_title(void)
{
  if (unincarnated)  karma(NULL);
  return consult_title;
}

/* Return the directory with help files. */
char *client_help_directory(void)
{
  if (unincarnated)  karma(NULL);
  return help_dir;
}

/* Return the "root" help file name. */
char *client_help_primary_file(void)
{
  if (unincarnated)  karma(NULL);
  return help_file;
}

/* Return the help file extension. */
char *client_help_ext(void)
{
  if (unincarnated)  karma(NULL);
  return help_ext;
}

/* Return the stock answers directory. */
char *client_SA_directory(void)
{
  if (unincarnated)  karma(NULL);
  return stock_dir;
}

/* Return the stock answers browser executable. */
char *client_SA_browser_program(void)
{
  if (unincarnated)  karma(NULL);
  return stock_browser;
}

/* Return the list of commands needed before stock answers are available. */
char **client_SA_attach_commands(void)
{
  if (unincarnated)  karma(NULL);
  return stock_attach;
}

/* Return the "magic" file for the stock answers locker. */
char *client_SA_magic_file(void)
{
  if (unincarnated)  karma(NULL);
  return stock_magic;
}

/*** functions that look like accessors, but aren't ***/

/* Return the name of the non-locking OLxx service (eg. "OLC-query").
 * Return value: pointer to a static local variable containong the data.
 * Note: the code assumes that service_name may change, which
 *	currently never happens.
 */
char *client_nl_service_name(void)
{
  static char *nlserv = NULL;
  if (unincarnated)  karma(NULL);

  if (nlserv != NULL)
    free(nlserv);
  nlserv = malloc(strlen(service_name)+7);
  if (nlserv == NULL)
    {
      fprintf(stderr, "%s: out of memory, giving up!\n", clt_name);
      return FATAL;
    }

  strcpy(nlserv, service_name);
  strcat(nlserv, "-query");

  return nlserv;
}

/* Return true iff this is client has stock answers.
 * Note: stock_dir, stock_browser and stock_magic are all used to
 *      determine if the stock answers exist.
 */
char client_has_answers(void)
{
  static int warn_once = 1;

  if (unincarnated)  karma(NULL);

  if (stock_dir && stock_browser && stock_magic)
    return 1;

  if (stock_dir || stock_browser || stock_magic)
    {
      if (warn_once)
	{
	  fprintf(stderr, "%s: misconfigured!  "
		  "(some but not all browser options were set)\n",
		  clt_name);
	}
      warn_once = 0;
    }
  return 0;
}

/* Return true iff this is client has help available.
 * Note: help_dir, help_file and help_ext are all used to determine if the
 * 	stock answers exist.  All of them should be defined in every
 * 	configuration file, so this will generally be false only if we had
 * 	to guess.
 */
char client_has_help(void)
{
  static int warn_once = 1;

  if (unincarnated)  karma(NULL);

  if (help_dir && help_ext && help_file)
    return 1;

  if (help_dir || help_ext || help_file)
    {
      if (warn_once)
	{
	  fprintf(stderr, "%s: misconfigured!  "
		  "(some but not all help options were set)\n",
		  clt_name);
	}
      warn_once = 0;
    }
  return 0;
}

/*** private functions ***/

/* Complain about an incarnation problem (probably due to a bug).
 * Returns: a coredump. =)
 */
static void karma (char *bad)
{
  /* Most functions call karma(NULL), which gives the default message. */
  if (bad == NULL)
    bad = "attempted to use incarnation data before initialization";

  fprintf(stderr, "olxx: internal error: %s\n", bad);
  abort();
}

/*** code that does initialization of incarnation data ***/

/* Attempt to salvage from finding no incarnation data...
 * clt_name should be filled in, others are guessed.
 * Returns: ERROR on success (because real success is reading the cfg file),
 *          FATAL on failure
 * Note: a user client (eg. olc) will find it much easier to recover than
 *     a consulting client (eg. olcr), since the name of the service is
 *     OLC, not OLCR.  Names like "oltr" (consulting client for OLTA)
 *     require even more guessing; ones like "answer" (OWL) are hopeless.
 * Note: defined only if HESIOD (we can't really guess otherwise)
 */
#ifdef HESIOD
static ERRCODE incarnate_guess(void)
{
  char *try, *pos;
  int last;
  char found = 0;

  try = malloc(strlen(clt_name)+1);
  if (try == NULL)
    {
      fprintf(stderr, "%s: out of memory, giving up!\n", clt_name);
      return FATAL;
    }
  strcpy(try, clt_name);
  last = strlen(clt_name) - 1;
  if (last < 0)
    {
      fprintf(stderr,
	      "olxx: empty client name in incarnate_guess, giving up\n");
      return FATAL;
    }

  if (hes_resolve(try, OLC_SERV_NAME))
    {
      found = 1;
    }
  else
    {
      /* Make sure Hesiod didn't run into network/configuration trouble. */
      switch (hes_error())
	{
	case HES_ER_NET:
	  fprintf(stderr,
"This workstation seems to be having serious network problems, which may\n\
also be the cause of the configuration problems with the %s client.\n\
Please check your network cables; if that doesn't help, try another human.\n",
		  clt_name);
	  return FATAL;
	case HES_ER_CONFIG:
	  fprintf(stderr,
"This workstation seems to be having Hesiod configuration problems, in\n\
addition to being unable to locate the %s configuration.  Sorry, I can't\n\
offer you more assistance with this; try asking a human instead.\n",
		  clt_name);
	  return FATAL;
	}

      try[last] = '\0';        /* truncate try[] by one character and retry. */
      if (hes_resolve(try, OLC_SERV_NAME))
	{
	  found = 1;
	}
#ifndef INCARNATE_GUESS_EASY
      else
	{
	  /* Last chance; we try all letters replacing the last. */
	  for (try[last] = 'a'; try[last] <= 'z'; try[last]++)
	    {
	      if ((try[last] != clt_name[last])
		  && (hes_resolve(try, OLC_SERV_NAME) != NULL))
		{
		  found = 1;
		  break;
		}
	    }
	}
#endif /* not INCARNATE_GUESS_EASY */
    }

  if (! found)
    {
      fprintf(stderr,
	      "%s: can't guess the service name, fix the config file!\n",
	      clt_name);
      return FATAL;
    }

  /* A match! As the Russian guy in "Goldeneye" says, "Ay am inveensibul." =)*/
  service_name = try;

  /* service_name should be in uppercase */
  for (pos = service_name ; *pos ; pos++)
    {
      *pos = toupper(*pos);
    }

  /* If the client ends with "r" but the service doesn't, it's consulting */
  is_consulting = ((clt_name[last] == 'r') && (service_name[last] != 'R'));

  /* Prompt is usually "clientname> ", but in this case we can cut corners */
  prompt = OLC_FALLBACK_PROMPT;

  /* Default consultant title ("consultant"), yay hardcoding... */
  consult_title = OLC_FALLBACK_TITLE;

  unincarnated = 0;
  return ERROR; /* i.e. limited success, as opposed to FATAL */
}
#endif /* HESIOD */

/* Read in the configuration file and set various variables.
 * Arguments:
 *   client_hint -- a file name from which the "base" name of the client is
 * 	determined; for instance, "olcr" and "/mit/consult/bin/olcr.new"
 * 	both indicate that the client is "olcr".
 *   cfg_path -- a colon-separated path which is searched for the
 * 	configuration file.
 * Returns: SUCCESS on sucess, ERROR if config file not found but
 *          guessing succeeded, FATAL if incarnation failed and the
 *          client should clean up and exit without calling anything
 *          that depends on incarnation data.
 */
ERRCODE incarnate(const char *client_hint, const char *cfg_path)
{
  const char *start, *end;
  FILE *cfg;

  if (! unincarnated)
    fprintf(stderr, "%s: WARNING: multiple incarnation, leaking memory.\n",
	    client_hint);

  /* Extract the client name from inst_hint */
  start = strrchr(client_hint, '/');
  if (start == NULL)
    start = client_hint;
  else
    start++;
  end = strchr(start, '.');
  if (end == NULL)
    end = strchr(start, '\0');

  clt_name = malloc(end-start+1);
  if (clt_name == NULL)
    {
      fprintf(stderr, "%s: out of memory, giving up!\n", start);
      return FATAL;
    }
  strncpy(clt_name, start, end-start);
  clt_name[end-start] = '\0';
  unincarnated = -1;    /* things can now call client_name() for errors etc. */

  /* Find the configuration file for this client. */
  cfg = cfg_fopen_in_path(clt_name, OLC_CONFIG_EXT, cfg_path);
  if (cfg == NULL)
    {
#ifdef HESIOD
      fprintf(stderr, "%s: can't find %s%s in OLXX_CONFIG path (guessing).\n",
	      clt_name, clt_name, OLC_CONFIG_EXT);
      return incarnate_guess();
#else
      fprintf(stderr, "%s: can't find %s%s in OLXX_CONFIG path (giving up).\n",
	      clt_name, clt_name, OLC_CONFIG_EXT);
      return FATAL;
#endif
    }

  cfg_read_config(clt_name, cfg, olxx_config);

  /** Fill in defaults for fields which have them. **/

  /* prompt: default is "clt_name> " */
  if (prompt == NULL)
    {
      prompt = malloc(strlen(clt_name) + 3);
      if (prompt == NULL)
	{
	  fprintf(stderr, "%s: out of memory, giving up!\n", clt_name);
	  return FATAL;
	}
      strcpy(prompt, clt_name);
      strcat(prompt, "> ");
    }

  /* help_file: default is the client name. */
  if (help_dir && ! help_file)
    help_file = clt_name;

  /* help_ext: default is OLC_DEFAULT_HELP_EXT (probably ".help") */
  if (help_dir && ! help_ext)
    help_ext = OLC_DEFAULT_HELP_EXT;
  
  /** Complain about the lack of required fields. **/

  if (service_name == NULL)
    {
      fprintf(stderr, "%s: missing `service' in configuration file!\n",
	      clt_name);
      return FATAL;
    }
#ifndef HESIOD
  /* Not having default_server isn't fatal if we have Hesiod. */
  if (default_server == NULL)
    {
      fprintf(stderr, "%s: missing `server' in configuration file!\n",
	      clt_name);
      return FATAL;
    }
#endif /* HESIOD */

  /* Note: there are other fields which may cause problems if missing; read
   * the comments in olc.cfg (and documentation, when it gets written).  We
   * only complain about things we know will break *any* client.
   */

  unincarnated = 0;
  return SUCCESS;
}

/* Set a minimal set of incarnation variables by hand.
 * Arguments:   client: name of the client binary
 *              service: name of the OLxx service
 *              default_server: name of the fallback server machine
 * Returns: SUCCESS on sucess.
 * Note: the pointers are inserted directly into the incarnation table, so
 *    the data may *not* be changed after this function is called.
 * Note: the use of this function is discouraged, except to facilitate
 *    quick and dirty hacks.
 */
ERRCODE incarnate_hardcoded(char *client, char *service,
			    char *server)
{
  if (! unincarnated)
    fprintf(stderr, "%s: WARNING: multiple incarnation, leaking memory.\n",
	    client);

  clt_name = client;
  service_name = service;
  default_server = server;
  unincarnated = 0;

  return SUCCESS;
}
