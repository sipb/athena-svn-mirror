/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "olc".
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Steve Dyer
 *      IBM/MIT Project Athena
 *      converted to use Hesiod in place of clustertab
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989-1997 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v $
 *	$Id: olc.c,v 1.38 1997-04-30 17:49:40 ghudson Exp $
 *	$Author: ghudson $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v 1.38 1997-04-30 17:49:40 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>
#include <olc/olc_tty.h>

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#ifdef KERBEROS
extern int krb_ap_req_debug;
#endif /* KERBEROS */

/* Template structure used to build the command table for different clients */
typedef struct tCOMMAND_template  {
  int when;
  COMMAND command;
} COMMAND_TMPL;

COMMAND *build_command_table(COMMAND_TMPL *tmpl);

/* Constants used for selecting when a command should be available.
 *   If you are adding a condition with a different meaning, add a new
 *   constant [here and in build_command_table()] and, possibly, a new
 *   configuration option in incarnate.c.  Please don't piggyback new
 *   functionality off of existing options just because they correspond in
 *   currently existing clients.
 */
#define CONSULT   (1<<0)	/* may be used only in consulting client */
#define USER      (1<<1)	/* may be used only in non-consluting client */
#define ANSWERS   (1<<2)	/* may be used if service has stock answers */
#define HOURS     (1<<3)	/* may be used if service has posted hours */
#define HELP      (1<<4)	/* may be used unless help is unavailable */
/* For values bigger than (1<<30), change COMMAND_TMPL.when to a long. =) */

/*
 * OLC command table.  Each line is an entry for a function that OLC may
 * perform.  The syntax is:
 *
 *     when, {"command name", function-name, "description"},
 *
 * <when> determines when the command is to be active.  It must be some
 *      combination of the constants defined above, or 0 indicating the
 *      command should work always.  [Bitwise ORing (|) of constants
 *      indicates that both/all conditions should be fulfilled.]
 *  "command name" is the name of the function as the user would type it.
 *  <function name> is the name of the function to be executed when the user
 *	types the name.
 *  "description" is a short descriptive phrase about what the command does
 *
 * Function names above serve as pointers to the functions they
 * name.  They should be declared above as "extern" so the compiler knows
 * what to do.
 */
COMMAND_TMPL Command_Table_Template[] = {
 {0,       {"?",        do_olc_list_cmds, "List available commands"}},
 {HELP,    {"help",     do_olc_help,      "Describe the various commands"}},
 {CONSULT, {"acl",      do_olc_acl,       "Display/Change access privileges"}},
 {ANSWERS, {"answers",  do_olc_stock,     "Read answers to common questions"}},
 {0,       {"ask",      do_olc_ask,       "Ask a question"}},
 {USER,    {"cancel",   do_olc_cancel,    "Cancel your question"}},
 {CONSULT, {"cancel",   do_olc_cancel,    "Cancel a question"}},
 {CONSULT, {"comment",  do_olc_comment,   "Make a comment"}},
 {CONSULT, {"dbinfo",   do_olc_dbinfo,    "Display database info."}},
 {CONSULT, {"dbload",   do_olc_load_user, "Reload user attributes."}},
 {CONSULT, {"describe", do_olc_describe,  "Show/Change summary info"}},
 {USER,    {"done",     do_olc_done,      "Mark your question resolved"}},
 {CONSULT, {"done",     do_olc_done,      "Resolve a question"}},
 {0,       {"exit",     do_quit,          "Temporarily exit"}},
 {CONSULT, {"forward",  do_olc_forward,   "Forward a question"}},
 {CONSULT, {"grab",     do_olc_grab,      "Grab a user"}},
 {HOURS,   {"hours",    do_olc_hours,     "Print hours when staffed"}},
 {CONSULT, {"instance", do_olc_instance,  "Show/Change default instance"}},
 {CONSULT, {"list",     do_olc_list,      "List the world"}},
 {CONSULT, {"mail",     do_olc_mail,      "Mail a message"}},
 {0,       {"motd",     do_olc_motd,      "See the message of the day"}},
 {CONSULT, {"off",      do_olc_off,       "Sign off"}},
 {CONSULT, {"on",       do_olc_on,        "Sign on"}},
 {0,       {"quit",     do_quit,          "Temporarily exit"}},
 {0,       {"replay",   do_olc_replay,    "Replay the conversation so far"}},
 {0,       {"send",     do_olc_send,      "Send a message"}},
 {0,       {"show",     do_olc_show,      "Show any new messages"}},
 {0,       {"status",   do_olc_status,    "Display your status"}},
 /* "stock" used to be consultants-only.  Now it's like "answers". --bert */
 {ANSWERS, {"stock",    do_olc_stock,     "Browse thru stock answers"}},
 {USER,    {"topic",    do_olc_topic,     "Find question topic"}},
 {CONSULT, {"topic",    do_olc_topic,     "Show/Change question topic"}},
 {0,       {"version",  do_olc_version,   "Print version number"}},
 {USER,    {"who",      do_olc_who,       "Find out who you're connected to"}},
 {CONSULT, {"who",      do_olc_who,       "Find status for current instance"}},
#ifdef ZEPHYR
 {CONSULT, {"zephyr",   do_olc_zephyr,    "Toggle daemon zephyr use"}},
#endif
 {-1,      {NULL,       NULL,             ""}}
};

COMMAND *Command_Table;
char *program;

int select_timeout = 300;

/*
 * Function:	main() is the startup for OLC.  It initializes the
 *			environment and starts the command interpreter.
 * Arguments:	argc:	Number of arguments in the command line.
 *		argv:	Array of words from the command line.
 *
 * Returns:	Nothing.
 * Notes:
 *	First, check the terminal mode to make sure that the program is
 *	interactive and that the user can receive 'write' messages from
 *	the daemon.  Then look up the necessary information about how to
 *	find the daemon in the Hesiod database.  Next, we get general
 *	information about the user, including some from her /etc/passwd
 *	entry.  The relevant information is copied into an OLC "person"
 *	structure, and the OLC request structure is initialized.  From the
 *	password entry we take the username and the real name, excluding
 *	any office or phone information.  Also, we construct the user
 *	identification string, which has the format "realname (user@machine)".
 *	We then	call olc_init() to complete the initialization and then invoke
 *	the command interpreter.
 */

int
main(argc, argv)
     int argc;
     char **argv;
{
  char *prompt = NULL;
  char *config;
  ERRCODE status;
#ifdef POSIX
  struct sigaction act;
#endif

/*
 * All client specific stuff should be initialized here, if they wish
 * to play after dinner.
 */

  program = strrchr(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  config = getenv("OLXX_CONFIG");
  if (config == NULL)
    config = OLC_CONFIG_PATH;
 
  ++argv, --argc;
  while (argc > 0 && argv[0][0] == '-') {
      if (!strcmp (argv[0], "-name") && (argc>1)) {
	program = argv[1];
	++argv, --argc;
      }
      else if (!strcmp (argv[0], "-config") && (argc>1)) {
	config = argv[1];
	++argv, --argc;
      }
      else if (!strcmp (argv[0], "-prompt") && (argc>1)) {
	prompt = argv[1];
	++argv, --argc;
      }
      else if (!strcmp (argv[0], "-server") && (argc>1)) {
	/*
	 * this is a kludge, but the other interface is already
	 * there
	 */
	set_env_var ("OLCD_HOST", argv[1]);
	++argv, --argc;
      }
      else if (!strcmp (argv[0], "-port") && (argc>1)) {
	set_env_var ("OLCD_PORT", argv[1]);
	++argv, --argc;
      }
      else if (!strcmp (argv[0], "-inst") && (argc>1)) {
	set_env_var ("OLCD_INST", argv[1]);
	++argv, --argc;
      }
      else {
	  fprintf (stderr, "%s: flag unknown or its argument is missing: %s\n",
		   program, argv[0]);
	  exit (1);
      }
      ++argv;
      --argc;
  }

  if (incarnate(program, config) == FATAL) {
    /* Fatal problem.  Messages indicating causes were already displayed... */
    exit(1);
  }

  Command_Table = build_command_table(Command_Table_Template);

#ifdef POSIX
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler= SIG_IGN;
  (void) sigaction(SIGPIPE, &act, NULL);
#else /* not POSIX */
  signal(SIGPIPE, SIG_IGN);
#endif /* not POSIX */
  if (argc)
    {
      OInitialize();
      status = do_command(Command_Table, argv);
      exit(status);
    }
  else 
    {
      if (prompt == NULL)
	prompt = client_default_prompt();
      (void) do_olc_init();
      command_loop(Command_Table, prompt);
    }
  exit(0);
}

/* Construct the command table.
 * Arguments:	tmpl: a template for building the table.
 * Returns:	a pointer to the table (which is allocated using malloc).
 * Non-local returns: exits with code 1 if malloc fails.
 * Notes:
 *	The code is currently somewhat inefficient, but it only runs
 *	on startup.
 */
COMMAND *build_command_table(COMMAND_TMPL *tmpl)
{
  COMMAND_TMPL *entry;
  COMMAND *table, *next;
  int num_entries;

  int consult = client_is_consulting_client();
  int user =    client_is_user_client();
  int answers = client_has_answers();
  int hours =   client_has_hours();
  int help =    client_has_help();

  /* first, make a dry run to count how many fields we need. */
  num_entries = 1;    /* count the trailing NULL-filled record */
  for (entry = tmpl; entry->when >= 0; entry++)
    {
      /* "entry->when" specifies which criteria must be satisfied for the
       * command to be included in the command table.  Each bit specifies a
       * criterion to consider (if the bit is set).  ORing bits means all
       * of the corresponding criteria must be true; 0 means no
       * requirements, i.e. always include.
       */
      if (((entry->when & CONSULT) ? consult : 1) &&
	  ((entry->when & USER) ?    user    : 1) &&
	  ((entry->when & ANSWERS) ? answers : 1) &&
	  ((entry->when & HOURS) ?   hours   : 1) &&
	  ((entry->when & HELP) ?    help    : 1))
	num_entries++;
    }

  /* allocate memory for a new table */
  table = malloc(num_entries * sizeof(COMMAND));
  if (table == NULL)
    {
      fprintf(stderr, "Out of memory, can't build command table.\n");
      exit(1);
    }

  /* fill the table */
  next = table;
  for (entry = tmpl; entry->when >= 0; entry++)
    {
      /* "entry->when" specifies which criteria must be satisfied for the
       * command to be included in the command table.  [See above.]
       */
      if (((entry->when & CONSULT) ? consult : 1) &&
	  ((entry->when & USER) ?    user    : 1) &&
	  ((entry->when & ANSWERS) ? answers : 1) &&
	  ((entry->when & HOURS) ?   hours   : 1) &&
	  ((entry->when & HELP) ?    help    : 1))
	{
	  memcpy(next, &(entry->command), sizeof(COMMAND));
	  next++;
	}
    }

  /* fill the last entry with empty data */
  next->command_name = NULL;
  next->command_function = NULL;
  next->description = "";

  return table;
}


/* Set an environment variable.
 * Arguments:	var: a string containing the name of the variable.
 *		value: a string containing the new value.
 * Returns:	nothing.
 * Non-local returns: on some platforms, exits with code 1 if malloc fails.
 */

void
set_env_var(const char *var, const char *value)
{
#ifdef PUTENV
  char *buf = malloc(strlen(var)+strlen(value)+2);
  if (buf == NULL)
    {
      fprintf(stderr, "Out of memory, can't expand environment.\n");
      exit(1);
    }
  sprintf(buf, "%s=%s", var, value);
  putenv(buf);
#else
  setenv (var, value, 1);
#endif
}

/*
 * Function:	olc_init() completes the initialization process for
 *			the user program.
 * Arguments:	None.
 * Returns:	an error code (which is always 0).
 * Notes:
 *	First, find out if the user has a current question.  If not,
 *	prompt for topic and question.  Next, send an OLC_STARTUP
 *	request to the daemon, either starting a new conversation or
 *	continuing an old one.  A message notifying the user of the
 *	status of the question is then printed.  Finally, we return
 *	to the OLC main loop.
 */

ERRCODE
do_olc_init() 
{
  int fd;			/* File descriptor for socket. */
  REQUEST Request;
  ERRCODE errcode=0;	        /* Error code to return. */
  int n,first=0;
  char file[NAME_SIZE];
  char topic[TOPIC_SIZE];
  int status;

  OInitialize();

  fill_request(&Request);
  Request.request_type = OLC_STARTUP;

  status = open_connection_to_daemon(&Request, &fd);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }
  status = send_request(fd, &Request);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }
  read_response(fd, &status);

  switch(status) 
    {
    case USER_NOT_FOUND:
      printf("Welcome to %s, ",client_service_name());
      printf("Project Athena's On-Line Consulting system. (v %s)\n",
	     VERSION_STRING);
      printf("Copyright (c) 1989-1997 by "
	     "the Massachusetts Institute of Technology.\n\n");
      first = 1;
	
      break;
         
    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      printf("Welcome back to %s. ",client_service_name());
      printf("Project Athena's On-Line Consulting system. (v %s)\n",
	     VERSION_STRING);
      printf("Copyright (c) 1989-1997 by "
	     "the Massachusetts Institute of Technology.\n\n");

      if(t_set_default_instance(&Request) != SUCCESS)
	fprintf(stderr,"Error setting default instance... continuing.\n");

      if(t_personal_status(&Request,FALSE) != SUCCESS)
	fprintf(stderr,"Error getting status... continuing.\n");

      break; 
   case PERMISSION_DENIED:
      printf("You are not allowed to use %s.\n",client_service_name());
      exit(1);
    default:
      if(handle_response(status, &Request) == ERROR)
	exit(ERROR);
    }

  (void) close(fd);

  make_temp_name(file);
  Request.request_type = OLC_MOTD;
  t_get_file(&Request,0,file,FALSE);
  unlink(file);

  if(client_is_user_client())
    {
      if (client_has_answers())
	printf("\nTo see answers to common questions, type:      answers\n");
      if (client_has_hours())
	printf("To see the hours %s is staffed, type:         hours\n",
	       client_service_name());
      if (first)
	{
	  topic[0]='\0';
	  printf("To ask a question, type:          ask\n");
	}
    }

  return(errcode);
}
