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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc.c,v 1.11 1989-08-10 03:11:22 tjcoppet Exp $";
#endif 


#include <olc/olc.h>
#include <olc/olc_parser.h>
#include "olc.h"

#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <netdb.h>

extern  int subsystem;

/* External declarations for OLC functions. */

extern do_quit();			/* Quit olc. */
extern do_olc_list_cmds();		/* List commands, giving brief info. */
extern do_olc_help();			/* Give extended help. */
extern do_olc_send();			/* Send a message. */
extern do_olc_replay();			/* Print the log up to this point. */
extern do_olc_show();			/* Print any unseen messages. */
extern do_olc_done();			/* Mark problem as solved. */
extern do_olc_cancel();
extern do_olc_ask();
extern do_olc_list();
extern do_olc_comment();
extern do_olc_describe();
extern do_olc_topic();
extern do_olc_instance();
extern do_olc_grab();
extern do_olc_on();
extern do_olc_off();
extern do_olc_forward();
extern do_olc_motd();
extern do_olc_stock();
extern do_olc_mail();
extern do_olc_status();
extern do_olc_live();
extern do_olc_whoami();
extern do_olc_load_user();
extern do_olc_who();

extern int krb_ap_req_debug;



/*
 * OLC command table.  Each line is an entry for a function that OLC will
 * perform.  The syntax is:
 *
 *     "command name", function-name, "description"
 *
 *  "command name" is the name of the function as the user would type it.
 *  <function name> is the name of the function to be executed when the user
 *	types the name.
 *  "description" is a short descriptive phrase about what the command does
 *
 * Both of the function names above serve as pointers to the functions they
 * name.  They should be declared above as "extern" so the compiler knows
 * what to do.
 */

COMMAND OLC_Command_Table[] = {
  "?",		do_olc_list_cmds,"List available commands",
  "help",	do_olc_help,	"Describe the various commands.",
  "quit",	do_quit,	"Temporarily exit OLC",
  "send",	do_olc_send,	"Send a message to the consultant",
  "done",	do_olc_done,	"Mark your question resolved",
  "cancel",     do_olc_cancel,  "Cancel your question",
  "replay",	do_olc_replay, 	"Replay the conversation so far",
  "show",       do_olc_show,	"Show any new messages",
  "topic",      do_olc_topic,   "Find question topic",
  "motd",       do_olc_motd,    "See message of the day",
  "answers",    do_olc_stock,   "Read answers to common questions",
  "status",     do_olc_status,  "Print your status",
  "who",        do_olc_who,     "Find name of connected consultant",
  (char *) NULL, (int(*)()) NULL,	""
  };
  
COMMAND OLCR_Command_Table[] = {
  "?",		do_olc_list_cmds,"List available commands",
  "help",	do_olc_help,	 "Describe the various commands",
  "quit",	do_quit,	 "Temporarily exit OLC",
  "send",	do_olc_send,	 "Send a message",                     
  "done",	do_olc_done,	 "Resolve question",               
  "cancel",     do_olc_cancel,   "Cancel your question",
  "replay",	do_olc_replay,   "Replay the conversation",        
  "show",       do_olc_show,	 "Show any new messages",
  "list",       do_olc_list,     "List the world",
  "comment",    do_olc_comment,  "Make a comment",
  "describe",   do_olc_describe, "Show/Change summary info",
  "mail",       do_olc_mail,     "Mail a message",
  "topic",      do_olc_topic,    "Show/Change question topic",
  "instance",   do_olc_instance, "Show/Change default instance",
  "on",         do_olc_on,       "Sign on",
  "off",        do_olc_off,      "Sign off",
  "grab",       do_olc_grab,     "Grab a user",
  "forward",    do_olc_forward,  "Forward a question",
  "motd",       do_olc_motd,     "See motd",
  "stock",      do_olc_stock,    "Browse thru stock answers",
  "status",     do_olc_status,   "Find your status",
  "who",        do_olc_who,      "Find status for current instance",
  (char *) NULL, (int(*)()) NULL,	""
  };


COMMAND *Command_Table;

/* Other global variables. */
  
PERSON User;				/* Structure describing user. */
char DaemonHost[LINE_LENGTH];		/* Name of the daemon's machine. */
char *program;

int OLCR=0,OLC=0;

/*
 * Function:	main() is the startup for OLC.  It initializes the
 *			environment and starts the command interpreter.
 * Arguments:	argc:	Number of arguments in the command line.
 *		argv:	Array of words from the command line.
 *		Someday these will be used.
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

main(argc, argv)
     int argc;
     char **argv;
{
  struct passwd *getpwuid();   /* Get a password entry based on the user ID.*/
  struct passwd *pwent;	       /* Password entry. */
  char hostname[LINE_LENGTH];  /* Name of local machine. */
  struct hostent *host;
  int uid;		       /* User ID number. */
  char *tty;		       /* Terminal path name. */
  char *prompt;

#ifdef HESIOD
  char **hp;		       /* return value of Hesiod resolver */
#endif


/*
 * All client specific stuff should be initialized here, if they wish
 * to play after dinner.
 */

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

  if(string_eq(program,"olc"))
    {
      Command_Table = OLC_Command_Table;
      prompt=OLC_PROMPT;
      HELP_FILE = OLC_HELP_FILE;
      HELP_DIR =  OLC_HELP_DIR;
      HELP_EXT =  OLC_HELP_EXT;
      OLC=1;
    }
  else
    {       
      Command_Table = OLCR_Command_Table;
      prompt=OLCR_PROMPT;
      HELP_FILE = OLCR_HELP_FILE;
      HELP_DIR =  OLCR_HELP_DIR;
      HELP_EXT =  OLCR_HELP_EXT;
      OLCR=1;
    }

  if ((tty = ttyname(fileno(stdin))) == (char *)NULL) 
    goto no_tty;

#ifdef TERMINAL 
  if (stat(tty, &statbuf) < 0) 
    {
      printf("Unable to get terminal status. Please check your TERM variable.\n");
      exit(1);
    }
  if (!(statbuf.st_mode & 020)) 
    {
      printf("Your terminal is not set to receive messages.  To");
      printf(" do so, type 'mesg y'\n");
      printf("at the shell prompt, and then type 'olc' again.\n");
      exit(1);
    }
#endif TERMINAL

 no_tty:


#ifdef HESIOD
  if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
    {	

      fprintf(stderr,
	      "Unable to get name of OLC server host from the Hesiod nameserver.\n");
      fprintf(stderr, 
	      "This means that you cannot use OLC at this time. Any problems \n");
      fprintf(stderr,
	      "you may be experiencing with your workstation may be the result of this\n");
      fprintf(stderr,
	      "problem. \n");
      
      exit(ERROR);
    }
  else 
    (void) strcpy(DaemonHost, *hp);

#endif HESIOD


  uid = getuid();
  pwent = getpwuid(uid);
  (void) strcpy(User.username, pwent->pw_name);
  (void) strcpy(User.realname, pwent->pw_gecos);
  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';
  
  gethostname(hostname, LINE_LENGTH);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));     
  User.uid = uid;
 
  expand_hostname(DaemonHost,INSTANCE,REALM);

#ifdef TEST
printf("main: starting for %s/%s (%s)  on %s\n",User.realname, User.username,
       User.uid, User.machine);
#endif TEST

  signal(SIGPIPE, SIG_IGN);
  if (argc > 1) 
    {
      do_command(Command_Table, ++argv);
      exit(SUCCESS);
    }
  else 
    {
      subsystem = 1;
      (void) olc_init();
      command_loop(Command_Table, prompt);
    }
}


/*
 * Function:	olc_init() completes the initialization process for
 *			the user program.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 *	First, find out if the user has a current question.  If not,
 *	prompt for topic and question.  Next, send an OLC_STARTUP
 *	request to the daemon, either starting a new conversation or
 *	continuing an old one.  A message notifying the user of the
 *	status of the question is then printed.  Finally, we return
 *	to the OLC main loop.
 */

olc_init() 
{
  int fd;			/* File descriptor for socket. */
  RESPONSE response;	        /* Response code from daemon. */
  REQUEST Request;
  ERRCODE errcode=0;	        /* Error code to return. */
  FILE *stdstr;
  int n,first=0;
  char file[NAME_LENGTH];
  char topic[TOPIC_SIZE];
  int status;

  fill_request(&Request);
  Request.request_type = OLC_STARTUP;

  fd = open_connection_to_daemon();
  status = send_request(fd, &Request);
  if(status)
    {
      handle_response(status, &Request);
      exit(ERROR);
    }
  read_response(fd, &response);

#ifdef TEST
  printf("do_olcinit: requester %s, response: %d\n",
	 Request.requester.username, response);
#endif TEST

  switch(response) 
    {
    case USER_NOT_FOUND:
      if(subsystem)
	{
	  printf("Welcome to OLC, ");
	  printf("Project Athena's On-Line Consulting system.\n");
	  printf("Copyright (c) 1989 by ");
	  printf("the Massachusetts Institute of Technology.\n\n");
	  first = 1;
	}
#ifdef TEST
      printf("do_olc_init: %s not found\n", Request.requester.username);
#endif TEST

      break;
         
    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      printf("Welcome back to OLC. ");
      printf("Project Athena's On-Line Consulting system.\n");
      printf("Copyright (c) 1989 by ");
      printf("the Massachusetts Institute of Technology.\n\n");
      t_set_default_instance(&Request);
      t_personal_status(&Request,FALSE);
      break; 
   case PERMISSION_DENIED:
      printf("You are not allowed to use OLC.\n");
      exit(1);
    default:
      if(handle_response(response, &Request) == ERROR)
	exit(ERROR);
    }

  (void) close(fd);

  make_temp_name(file);
  t_get_motd(&Request,OLC,file,FALSE);
  unlink(file);

  if(OLC && first)
    {
      topic[0]='\0';
      if(t_input_topic(&Request,topic,TRUE) != SUCCESS)
	exit(1);
      if(t_ask(&Request,topic) != SUCCESS)
	exit(1);
      if(OLC)
	{
	  printf("\nSome other useful OLC commands are: \n\n");
	  printf("\tsend  - send a message\n");
	  printf("\tshow  - show new messages\n");
	  printf("\tdone  - mark your question resolved\n");
	  printf("\t?     - see entire listing of commands\n");
	}
      
    }

  return(errcode);
}
