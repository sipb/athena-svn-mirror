/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains miscellaneous routines for handling the terminal.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: t_utils.c,v 1.50.6.1 2002-07-18 18:04:10 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_utils.c,v 1.50.6.1 2002-07-18 18:04:10 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_tty.h>

#include <unistd.h>
#include <time.h>
#include <sys/file.h>		
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <termios.h>

#include <readline/readline.h>
#include <readline/history.h>

#ifdef HAVE_LRAND48
#define RANDOM()       lrand48()
#define SRANDOM(seed)  srand48(seed)
#else /* don't HAVE_LRAND48 */
#define RANDOM()       random()
#define SRANDOM(seed)  srandom(seed)
#endif /* don't HAVE_LRAND48 */

/*
 * Function:    is_flag() Checks if string s is flag f,
 *                      matching a minimum of n letters.
 * Arguments:   string:  Null terminated string to test.
 *              flag:    Full flag value.
 *              mimimum: Minimum number of letters to test.
 * Returns:     TRUE or FALSE
 * Notes:
 *   Check each character in string against flag. If flag ends,
 *   or if the characters dont match, return FALSE. If string
 *   ends before the minimum number of characters have matched,
 *   return FALSE. Otherwise, return TRUE.
 */   

int
is_flag(string, flag, minimum)
     char *string;
     char *flag;
     int minimum;
{
  int i; /* Counter. */

  for(i = 0; (string[i] != '\0'); i++)
    if((string[i] != flag[i]) || (flag[i] == '\0'))
      return FALSE;
  if(i < minimum) /* String ended before minimum was reached. */
    return FALSE;
  else
    return TRUE;
}     

/*
 * Function:	display_file() prints a file on a user's terminal.
 * Arguments:	filename:	Name of file to be printed.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, open the file to make sure that it is accessible. If it
 *	is not, log an error and return. Otherwise, attempt to execute
 *	the desired pager ($PAGER or DEFAULT_PAGER (defined in
 *	olc_tty.h) to page the file on the terminal. If this fails,
 *	simply print it line by line. In either case, end by closing
 *	the file and returning.
 */

ERRCODE
display_file(filename)
     char *filename;
{
  FILE *file;			/* File structure pointer. */
  int c;
  char *pager;

  file = fopen(filename, "r");
  if (file == NULL) 
    {
      fprintf(stderr, "display_file: Unable to open file %s\n",
	      filename);
      return(ERROR);
    }

  pager = getenv("PAGER");
  if(pager == NULL)
    pager = DEFAULT_PAGER;

  if(call_program(pager, filename) == ERROR) 
    {
      while((c = getc(stdin)) != EOF)
	putc(c, stdout);
      printf("\n");
    }

  fclose(file);
  return(SUCCESS);
}

ERRCODE
cat_file(file)
     char *file;
{
  int fd;
  char buf[BUF_SIZE];
  int len;

  fd = open(file,O_RDONLY,0);
  if(fd < 0)
    return(ERROR);
  
  while((len = read(fd,buf,BUF_SIZE)) == BUF_SIZE)
    write(1,buf,len);
  
  if(len)
    write(1,buf,len);

  close(fd);
  return(SUCCESS);
}

ERRCODE
enter_message(file,editor)
     char *file;
     char *editor;
{
  ERRCODE status;

  if((editor == NULL) || (string_eq(editor, "")))
    {
      if(isatty(1))
        {
          printf("Please enter your message.  ");
          printf("End with a ^D or '.' on a line by itself.\n");
        }
      status = input_text_into_file(file);
    }
  else
    status = edit_message(file,editor);

  if (file_length(file) == 0)
      printf("Warning: no message was entered.\n");

  status = what_now(file, FALSE,NULL);
  if(status == ERROR)
    {
      fprintf(stderr,"An error occurred while reading your");
      fprintf(stderr," message; unable to continue.\n");
      unlink(file);
      return(ERROR);
    }
  else
    if(status != SUCCESS)
      return(status);

  if (file_length(file) == 0)
    {
      printf("No message was entered.\n");
      return(NO_ACTION);
    }

  return(SUCCESS);
}


/*
 * Function:	input_text_into_file() reads text from the standard input
 *			into a file.
 * Arguments:	filename:	Name of file to use.
 * Returns:	An error code.
 * Notes:
 *	First, open the file.  If it is not accessible, log the error
 *	and return.  Otherwise, read from the standard input until
 *	^D or a '.' by itself.  Write each line into the file named
 *	"filename".
 */

ERRCODE
input_text_into_file(filename)
     char *filename;
{
  char line[LINE_SIZE];	
  int fd;
  int nchars;                   
	
  fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd < 0) 
    {
      fprintf(stderr, "input_text: Can't open file %s for writing.",
	      filename);
      printf("Unable to open temporary file.\n");
      return(ERROR);
    }
	
  while ((nchars = read(fileno(stdin), line, LINE_SIZE)) > 0) 
    {
      if ((line[0] == '.') & (nchars == 2))
	break;
      if (write(fd, line, nchars) != nchars) 
	{
	  olc_perror("input_text: error writing to file");
	  return(ERROR);
	}
    }
  close(fd);
  return(SUCCESS);
}


ERRCODE
verify_terminal()
{
  char *tty;
  struct stat statbuf;

  tty = (char *)ttyname(fileno(stdin));
  if (tty == NULL)
    return(FAILURE);

  if (stat(tty, &statbuf) < 0)
    return(ERROR);

  if (!(statbuf.st_mode & 020))
    return(FAILURE);
      
  return(SUCCESS);
}

static struct termios saved_tty;
static struct termios my_tty;
static struct sigaction my_act;

void
sighandler(int sig)
{
  struct sigaction act;
  sigset_t set, oset;
  static int state = 0;

  switch(sig)
    {
    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU:
      /* We got a signal that we'll eventually return from with SIGCONT. */
      state = 1;
    case SIGINT:
      tcsetattr(0, TCSANOW, &saved_tty); /* Reset terminal state. */

      /* Disable the signal handler on all the the signals we might
	 have caught. */
      act.sa_flags = 0;
      act.sa_handler = SIG_DFL;
      sigemptyset(&act.sa_mask);
      sigaction(SIGTSTP, &act, NULL);
      sigaction(SIGTTOU, &act, NULL);
      sigaction(SIGTTIN, &act, NULL);
      sigaction(SIGINT, &act, NULL);

      kill(getpid(), sig); /* Please, sir, may I have another? */

      /* Now we return from the signal handler, which unblocks the signal,
	 so we should immediately get the one we just sent. */

      break;
    case SIGCONT:
      if (state)
	{
	  /* Set the handlers back. */
	  sigaction(SIGTSTP, &my_act, NULL);
	  sigaction(SIGTTIN, &my_act, NULL);
	  sigaction(SIGTTOU, &my_act, NULL);
	  sigaction(SIGINT, &my_act, NULL);
	  
	  /* The user may have altered the terminal state. */
	  tcgetattr(0, &saved_tty);
	  state=0;
	}
      tcsetattr(0, TCSANOW, &my_tty); /* set back to our altered state */
      break;
    }
}

char
get_key_input(text)
     char *text;
{
  int c;
  struct sigaction tstp_act;
  struct sigaction ttin_act;
  struct sigaction ttou_act;
  struct sigaction intr_act;
  struct sigaction cont_act;

  /* XXX This function deals with terminal setup and signal frobbing
     itself instead of farming it out to utility functions because in
     the absence of a program-wide signal infrastructure they would of
     necessity have some really odd side-effects involving signals.
     If any further attention to signals (particularly SIGINT) is paid
     in the command-line olc client, I beg that this function be split
     up and rewritten. -kcr */

  tcgetattr(0, &saved_tty); /* save the terminal state */

  /* Frob the state before setting up the signal handler, because the
     handler sometimes uses it. */
  my_tty = saved_tty;
  my_tty.c_lflag &= ~(ICANON|ECHO);
  my_tty.c_cc[VMIN] = 1;
  my_tty.c_cc[VTIME] = 0;

  /* Set up the sigaction structure for sighandler. */
  my_act.sa_flags = 0;
  my_act.sa_handler = sighandler;
  sigemptyset(&my_act.sa_mask);

  /* Set signal handlers. */
  sigaction(SIGTSTP, &my_act, &tstp_act);
  sigaction(SIGTTIN, &my_act, &ttin_act);
  sigaction(SIGTTOU, &my_act, &ttou_act);
  sigaction(SIGINT, &my_act, &intr_act);
  sigaction(SIGCONT, &my_act, &cont_act);

  tcsetattr(0, TCSANOW, &my_tty); /* Setup the terminal. */

  do
    {
      printf("%s", text);
      fflush(stdout); /* Print the prompt. */
      tcflush(0, TCIFLUSH); /* Flush the input. */
      c = getchar(); /* Get just one character. */
    }
  while (c == EOF && errno == EINTR); /* Reprint and rewait after a signal. */

  my_tty = saved_tty; /* To avoid race conditions. */

  /* Reset the signal handlers. */
  sigaction(SIGTSTP, &tstp_act, NULL);
  sigaction(SIGTTIN, &ttin_act, NULL);
  sigaction(SIGTTOU, &ttou_act, NULL);
  sigaction(SIGINT, &intr_act, NULL);
  sigaction(SIGCONT, &cont_act, NULL);

  tcsetattr(0, TCSANOW, &saved_tty); /* Reset the terminal. */

  return(c);
}

ERRCODE
handle_response(response, req)
     int response;
     REQUEST *req;
{
#ifdef HAVE_KRB4
  char *kmessage = "\nYou will have been properly authenticated when you do not see this\nmessage the next time you run this program.  If you were having trouble\nwith a program, try again.\n\n";
  char *kmessage2 = "If you continue to have difficulty, you can "
#ifdef CONSULT_PHONE_NUMBER
    "contact a user\nconsultant by phone at " CONSULT_PHONE_NUMBER
#else /* no CONSULT_PHONE_NUMBER */
    "contact a user\nconsultant in person"
#endif /* no CONSULT_PHONE_NUMBER */
    ".";
#endif /* HAVE_KRB4 */

  switch(response)
    {
    case UNKNOWN_REQUEST:
      fprintf(stderr,"This function cannot be performed by the %s server.\n",
	      client_service_name()); 
      fprintf(stderr, "What you want is down the hall to the left.\n");
      return(NO_ACTION);   

    case SIGNED_OFF:
      if(isme(req))
	printf("You have signed off of %s.\n", client_service_name());
      else
	printf("%s is signed off of %s.\n",req->target.username,
	       client_service_name());
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(isme(req))
	fprintf(stderr, "You are not signed on to %s.\n", client_service_name());
      else
	fprintf(stderr, "%s [%d] is not signed on to %s.\n",
		req->target.username,req->target.instance, client_service_name());
      return(NO_ACTION);   

    case NO_QUESTION:
      if(isme(req))
	{
	  fprintf(stderr,"You do not have a question in %s.\n",
		  client_service_name());
	  if(client_is_user_client())
	    {
	      printf("If you wish to ask a question, use \"ask\".\n",
		     client_service_name());
	      /* this used to exit, which kinda sucked.
	       * trying to just return and error
	       */
	      return(ERROR);
	    }
	}
      else
	fprintf(stderr,"%s [%d] does not have a question.\n",
		req->target.username, req->target.instance);
      return(ERROR);

    case HAS_QUESTION:
      if(isme(req))
	fprintf(stderr,"You have a question.\n");
      else
	fprintf(stderr,"%s [%d] does not have a question.\n",
		req->target.username,req->target.instance);
      return(ERROR);

    case NOT_CONNECTED:
      if(isme(req))
	fprintf(stderr,"You are not connected to a %s.\n", 
		client_is_user_client() ? client_default_consultant_title() : "user");	  
      else
	fprintf(stderr,"%s [%d] is not connected nor is asking a question.\n",
		req->target.username,req->target.instance);
      return(NO_ACTION);   

    case PERMISSION_DENIED: 		
      fprintf(stderr, "You are not allowed to do that.\n");
      return(NO_ACTION);   
 
    case TARGET_NOT_FOUND:
      fprintf(stderr,"Target user %s [%d] not found.\n",  req->target.username,
	      req->target.instance);
      return(ERROR);       

    case REQUESTER_NOT_FOUND:
      fprintf(stderr,"You (%s [%d]) are unknown.  There is a problem.\n",
	      req->requester.username, 
	      req->requester.instance);
      return(ERROR);       

    case INSTANCE_NOT_FOUND:
      fprintf(stderr,"Incorrect instance (%s [%d]) specified.\n",
	      req->target.username,
	      req->target.instance);
      return(ERROR);       

    case ERROR: 		
      fprintf(stderr, "Error response from daemon.\n");
      return(ERROR); 	   

    case USER_NOT_FOUND:
      fprintf(stderr,"User \"%s\" not found.\n", req->target.username);
      return(ERROR); 

    case NAME_NOT_UNIQUE:
      fprintf(stderr,
	      "The string %s is not unique.\n",req->target.username);
      return(ERROR);

    case ERROR_NAME_RESOLVE:
      fprintf(stderr, 
	      "Unable to resolve name of %s daemon host. Seek help.\n",
	      client_service_name());
      if(client_is_user_client())
	exit(ERROR);
      else
	return(ERROR);
      break;

    case ERROR_SLOC:
      fprintf(stderr,
"Unable to locate %s service. This may be caused by a network problem\n\
or a configuration problem on this workstation.  Try another workstation.\n\
If the error persists, seek help.\n", client_service_name());
      if(client_is_user_client())
	exit(ERROR);
      else
	return(ERROR);
      break;

    case ERROR_CONNECT:
      fprintf(stderr,"Unable to connect to the %s daemon.  Please try ",
	      client_service_name());
      fprintf(stderr,"again later.\n");
      if(client_is_user_client())
	exit(ERROR);
      else
	return(ERROR);
      break;

#ifdef HAVE_KRB4     /* these error codes are < 100 */
    case MK_AP_TGTEXP: 
    case RD_AP_EXP:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Your Kerberos tickets have expired. ");
      printf(" To renew your Kerberos tickets,\n");
      printf("type:    renew\n");
      if(client_is_user_client()) {
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      }
      exit(ERROR);
    case NO_TKT_FIL: 
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("You do not have a Kerberos ticket file.  To ");
      printf("get one, \ntype:    renew\n");
      if(client_is_user_client()) {
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      }
      exit(ERROR);
    case TKT_FIL_ACC:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Cannot access your Kerberos ticket file.\n");
      printf("Try:              setenv   KRBTKFILE  /tmp/random\n");
      printf("                  setenv   KRB5CCNAME /tmp/krb5cc_random\n");
      printf("                  renew\n");
      if(client_is_user_client()) {
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      }
      exit(ERROR);
    case RD_AP_TIME:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Kerberos authentication failed: this workstation's clock is set "
	     "incorrectly.\nPlease move to another workstation and notify "
#ifdef HARDWARE_MAINTAINER
	     HARDWARE_MAINTAINER
#else /* no HARDWARE_MAINTAINER */
	     "the workstation's maintainer"
#endif /* no HARDWARE_MAINTAINER */
	     " of this problem.");  /* was ATHENA */
      if(client_is_user_client()) {
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      }
      exit(ERROR);
#endif /* HAVE_KRB4 */

    case OK:
    case SUCCESS:
      return(SUCCESS);     

    default:
      if ((response < 100) && (response >0))   /* this isn't so great */
#ifdef HAVE_KRB4
	fprintf(stderr,"%s\n",krb_err_txt[response]);
      else
#endif
	fprintf(stderr, "Unknown response %d\n", response);
    }
  return(ERROR); 	   
}



/*
 * Function:	get_prompted_input() prompts the user for a command string.
 * Arguments:	prompt:		Prompt to use.
 *		buf:		Buffer to hold command line.
 * Returns:	Nothing.
 * Notes:
 *		Farms out just about everything to GNU libreadline.
 */

ERRCODE
get_prompted_input(prompt, buf, buflen, add_to_hist)
     char *prompt;		/* Prompt to use. */
     char *buf;		        /* Input line buffer. */
     int buflen;
     int add_to_hist;
{
  char *line, *p;
  static int done_gl_init = 0;
  
  rl_catch_signals = 0; /*dont catch sig int*/
  rl_catch_sigwinch = 0;
  rl_readline_name = client_name();
  line = readline(prompt);

  if(line)
    {
      strncpy(buf, line, buflen);

      if (add_to_hist)
	add_history(line);
      else
	free(line);
    }
  else
    {
      buf[0] = '\0';
      putchar('\n');
    }
  
  return(SUCCESS);
}

/*
 * Function:	get_yn() uses get_prompted_input to get a response
 *			which starts with 'y' or 'n', and returns the
 *			(lower-case) first input character.
 * Arguments:	prompt:		message to print
 * Returns:	'y':	yes
 * 		'n':	no
 */

int
get_yn (prompt)
    char *prompt;
{
    char buf[LINE_SIZE], *b;
    while (1) {
	get_prompted_input (prompt, buf, LINE_SIZE,0);
	b = buf;
	while (*b == ' ' || *b == '\t')
	    b++;
	if (*b == 'y' || *b == 'Y')
	    return 'y';
	if (*b == 'n' || *b == 'N')
	    return 'n';
	printf ("Please enter \"yes\" or \"no\".\n");
    }
}

/*
 * Function:	what_now() prompts the user for the next action and allows
 *			several rounds of editing before returning.
 * Arguments:	file:		File containing the message being edited.
 *		edit_first: 	Flag to indicate that message should be edited
 *				first.
 * Returns:	SUCCESS:	Message should be sent.
 *		ERROR:		An error occurred.
 *		NO_ACTION:	Do not send the message.
 * Notes:
 */

ERRCODE
what_now(file, edit_first, editor)
     char *file;
     int edit_first;
     char *editor;
{
  char inbuf[LINE_SIZE];      /* Input buffer. */
  int fd;			/* File descriptor. */

  fd = open(file, O_RDWR | O_CREAT, 0644);
  if (fd < 0) 
    {
      olc_perror("whatnow: unable to create temp file");
      return(ERROR);
    }
  close(fd);
  
  if (edit_first == TRUE) 
    edit_message(file, editor);

  if(!isatty(0))
    return(SUCCESS);

  while (TRUE) 
    {
      inbuf[0] = '\0';
      while (inbuf[0] == '\0') 
	{
	  putchar('\n');
	  get_prompted_input("What now? (type '?' for options): ", 
				    inbuf,LINE_SIZE,0);
	  if (inbuf[0] == '?' || inbuf[0] == '\0' || inbuf[0] == 'h') {
	    printf("Commands are:\n");
	    printf("\t?\tPrint help information.\n");
	    printf("\te\tEdit the message.\n");
	    printf("\tl\tList the message.\n");
	    printf("\ts\tSend the message.\n");
	    printf("\tq\tQuit without sending message.\n");
	    inbuf[0] = '\0';
	  }
	}
      
      if (is_flag(inbuf,"edit",1))
	edit_message(file, editor);
      else if (is_flag(inbuf,"quit",1))
	return(NO_ACTION);
      else if (is_flag(inbuf,"send",1))
	return(SUCCESS);
      else if (is_flag(inbuf,"list",1))
	display_file(file);
    }
}

/*
 * Function:	edit() invokes an editor on the specified file.
 * Arguments:	file:	name of file to be edited
 *		editor:	name of editor
 * Returns:     passes back return from call_program
 * Notes:
 *	If argument 'editor' is NULL, then try getenv("EDITOR"), and
 *	finally default to DFLT_EDITOR.
 */

static char *editor_name = (char *)NULL;

ERRCODE
edit_message(file, editor)
     char *file;
     char *editor;
{
  if (editor != (char *) NULL) 
    if(string_eq(editor,NO_EDITOR))
      editor = (char *) NULL;

  if (editor == (char *) NULL)
    {
      if (editor_name == (char *) NULL) 
	{
	  editor_name = getenv("EDITOR");
	  if (editor_name == NULL)
	    editor_name = DEFAULT_EDITOR;
	}
      editor = editor_name;
    }
  return (call_program(editor, file));
}


/*
 * Function:	mail_message() mails a message to someone.
 * Arguments:	user:		Name of user receiving the mail.
 *		consultant:	Name of consultant who gets a copy.
 *		msgfile:	Name of file containg message.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, call sendmail() to start a /bin/mail process.  If an
 *	illegal file descriptor is returned, return ERROR.  Otherwise,
 *	write the message to /bin/mail and return SUCCESS.
 */

ERRCODE
mail_message(user, consultant, msgfile, args) 
     char *user, *consultant, *msgfile;
     char **args;
{
  int fd;			/* File descriptor for sendmail. */
  int filedes;		        /* File descriptor for msgfile. */
  int nbytes;		        /* Number of bytes in message. */
  char *msgbuf;		        /* Ptr. to mail message buffer. */

  nbytes = file_length(msgfile);
  if (nbytes == ERROR)
    {
      olc_perror("mail");
      printf("Unable to get message file.\n");
      return(ERROR);
    }

  msgbuf = (char *)malloc((unsigned) nbytes);
  filedes = open(msgfile, O_RDONLY, 0);
  if (filedes <= 0) 
    {
      olc_perror("mail");
      printf("Error opening mail file.\n");
      free(msgbuf);
      return(ERROR);
    }
  if (read(filedes, msgbuf, nbytes) != nbytes) 
    {
      olc_perror("mail");
      printf("Error reading mail message.\n");
      free(msgbuf);
      close(filedes);
      return(ERROR);
    }
  fd = sendmail(args);
  if (fd < 0) 
    {
      printf("Error sending mail.\n");
      free(msgbuf);
      close(filedes);
      return(ERROR);
    }
  if (write(fd, msgbuf, nbytes) != nbytes)
    {
      olc_perror("mail");
      printf("Error sending mail.\n");
      free(msgbuf);
      close(filedes);
      close(fd);
      wait(0);	/* clean up sendmail process */
      return(ERROR);
    }
  close(fd);
  free(msgbuf);
  close(filedes);
  wait(0);		/* clean up sendmail process */
  return(SUCCESS);
}



char *
happy_message()
{
  static char do_init = 1;

  if (do_init) {
    SRANDOM( time(NULL) + getuid() + getpid() );
    do_init = 0;
  }

  if(RANDOM()%3 == 1)
    {
      switch(RANDOM()%12)
	{
	case 1: return("Have a nice day");
	case 2: return("Have a happy");
	case 3: return("Good day");
	case 4: return("Cheers");
	case 5: return("Enjoy");
	case 6: return("Pleasant dreams");
	case 7: return("Have a nice day");
	case 8: return("Have a happy");
	case 9: return("Drive safely");
        case 10: return("Do come again soon");
	case 11: return("Have a good one");
	default: return("Don't take any wooden nickels");
	}
    }
  return("Have a nice day");      
}

char *
article(word)
     char *word;
{
  switch(*word)
    {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
      return("an");
    }
  
  switch(*word)
    {
    case 'l':
    case 'f':
    case 'm':
    case 'n':
    case 'r':
    case 's':
      switch(*(word+1))
	{
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	  return("a");
	default:
	  return("an");
	}
    default:
      return("a");
    }
}
