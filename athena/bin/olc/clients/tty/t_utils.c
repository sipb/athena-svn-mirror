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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_utils.c,v $
 *	$Id: t_utils.c,v 1.31 1991-09-10 13:36:18 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/tty/t_utils.c,v 1.31 1991-09-10 13:36:18 lwvanels Exp $";
#endif
#endif

#ifdef _AUX_SOURCE
#define MORE_PROG "/bin/more"
#else
#define MORE_PROG "/usr/ucb/more"
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_tty.h>

#if defined(_POSIX_SOURCE)
#include <unistd.h>
#endif
#include <sys/time.h>		
#include <sys/file.h>		
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef TERMIO
#include <sys/termio.h>
struct termio mode;
#else
#include <sgtty.h>
struct sgttyb mode;
#endif
/*
 * Function:	display_file() prints a file on a user's terminal.
 * Arguments:	filename:	Name of file to be printed.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, open the file to make sure that it is accessible.  If it
 *	is not, log an error and return.  Otherwise, attempt to execute
 *	"more" to page the file on the terminal.  If this fails, simply
 *	print it line by line. In either case, end by closing the file
 *	and returning.
 */

ERRCODE
display_file(filename)
     char *filename;
{
  FILE *file;                  /* File structure pointer. */
  char line[LINE_SIZE];      /* Input line buffer. */
	
  if ((file = fopen(filename, "r")) == (FILE *)NULL) 
    {
      fprintf(stderr, "display_file: Unable to open file %s\n",
	      filename);
      return(ERROR);
    }
  
  if (call_program(MORE_PROG, filename) == ERROR) 
    {
      while(fgets(line, LINE_SIZE, file) != (char *)NULL)
	printf("%s", line);
      printf("\n");
    }
	
  (void) fclose(file);
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
  int status;

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
    if(status)
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
  (void) close(fd);
  return(SUCCESS);
}


ERRCODE
verify_terminal()
{
  char *tty;
  struct stat statbuf;

  if ((tty = (char *)ttyname(fileno(stdin))) == (char *) NULL)
    return(FAILURE);

  if (stat(tty, &statbuf) < 0)
    return(ERROR);

  if (!(statbuf.st_mode & 020))
    return(FAILURE);
      
  return(SUCCESS);
}


char
get_key_input(text)
     char *text;
{
  char c;

  printf("%s",text);
  fflush(stdout);
  raw_mode();
#ifdef TERMIO
  ioctl(0, TCFLSH, 2);
#else
  ioctl(0, TIOCFLUSH, 0);
#endif
  c = getchar();
  cooked_mode();
 
  return(c);
}


#ifdef TERMIO
raw_mode()
{
  ioctl(0, TCGETA, &mode);
  mode.c_lflag = mode.c_lflag & (~ECHO | ~ICANON | ~ISIG);
  ioctl(0, TCSETA, &mode);
}

cooked_mode()
{
  ioctl(0, TCGETA, &mode);
  mode.c_lflag = mode.c_lflag & (ICANON | ISIG | ECHO);
  ioctl(0, TCSETA, &mode);
}
#else
raw_mode()
{
  ioctl(0, TIOCGETP, &mode);
  mode.sg_flags = mode.sg_flags & ~ECHO | RAW;
  ioctl(0, TIOCSETP, &mode);
}

cooked_mode()
{
  ioctl(0, TIOCGETP, &mode);
  mode.sg_flags = mode.sg_flags & ~RAW | ECHO;
  ioctl(0, TIOCSETP, &mode);
}
#endif


ERRCODE
handle_response(response, req)
     int response;
     REQUEST *req;
{
#ifdef KERBEROS
  char *kmessage = "\nYou will have been properly authenticated when you do not see this\nmessage the next time you run olc.  If you were having trouble\nwith a program, try again.\n\n";
#ifdef ATHENA
  char *kmessage2 = "If you continue to have difficulty, feel free to contact a user\nconsultant by phone (253-4435).";
#else
  char *kmessage2 = "If you continue to have difficulty, please contact a user\nconsultant in person.";
#endif /* ATHENA */
#endif /* KERBEROS */

  switch(response)
    {
    case UNKNOWN_REQUEST:
      fprintf(stderr,"This function cannot be performed by the OLC server.\n");
      fprintf(stderr, "What you want is down the hall to the left.\n");
      return(NO_ACTION);   

    case SIGNED_OFF:
      if(isme(req))
	printf("You have signed off of OLC.\n");
      else
	printf("%s is singed off of OLC.\n",req->target.username);
      return(SUCCESS);

    case NOT_SIGNED_ON:
      if(isme(req))
	fprintf(stderr, "You are not signed on to OLC.\n");
      else
	fprintf(stderr, "%s [%d] is not signed on to OLC.\n",
		req->target.username,req->target.instance);
      return(NO_ACTION);   

    case NO_QUESTION:
      if(isme(req))
	{
	  fprintf(stderr,"You do not have a question in OLC.\n");
	  if(OLC)
	    {
	      printf("If you wish to ask another question, use 'olc' again.\n");
	      exit(1);
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
		OLC?"consultant":"user");	  
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
	      "Unable to resolve name of OLC daemon host. Seek help.\n");
      if(OLC)
	exit(ERROR);
      else
	return(ERROR);
      break;

    case ERROR_SLOC:
      fprintf(stderr,"Unable to locate OLC service. The /etc/services file\n");
      fprintf(stderr,"may be corrupt on this workstation; try another\n");
      fprintf(stderr,"workstation.  If the error persists, seek help.\n");
      if(OLC)
	exit(ERROR);
      else
	return(ERROR);
      break;

    case ERROR_CONNECT:
      fprintf(stderr,"Unable to connect to OLC daemon.  Please try ");
      fprintf(stderr,"again later.\n");
      if(OLC)
	exit(ERROR);
      else
	return(ERROR);
      break;

#ifdef KERBEROS     /* these error codes are < 100 */
    case MK_AP_TGTEXP: 
    case RD_AP_EXP:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Your Kerberos tickets have expired. ");
      printf(" To renew your Kerberos tickets,\n");
      printf("type:    kinit\n");
      if(OLC)
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      exit(ERROR);
    case NO_TKT_FIL: 
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("You do not have a Kerberos ticket file.  To ");
      printf("get one, \ntype:    kinit\n");
      if(OLC)
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      exit(ERROR);
    case TKT_FIL_ACC:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Cannot access your Kerberos ticket file.\n");
      printf("Try:              setenv   KRBTKFILE  /tmp/random\n");
      printf("                  kinit\n");
      if(OLC)
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      exit(ERROR);
    case RD_AP_TIME:
      fprintf(stderr, "(%s)\n",krb_err_txt[response]);
      printf("Kerberos authentication failed: workstation clock is ");
      printf("incorrect.\nPlease contact Athena operations and move to ");
      printf("another workstation.\n");
      if(OLC)
	printf("%s",kmessage);
	printf("%s\n",kmessage2);
      exit(ERROR);
#endif /* KERBEROS */

    case OK:
    case SUCCESS:
      return(SUCCESS);     

    default:
      if(response < 100)   /* this isn't so great */
#ifdef KERBEROS
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
 *	First, we print the prompt, then read a string using gets().
 *	If a ^D is typed, we exit.
 */

ERRCODE
get_prompted_input(prompt, buf)
     char *prompt;		/* Prompt to use. */
     char *buf;		        /* Input line buffer. */
{
  char *gets();		        /* Get a string from the stdin. */
	
  printf("%s", prompt);
  if (gets(buf) == (char *) NULL) 
    {
      printf("\n");
      exit(0);
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
	get_prompted_input (prompt, buf);
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

  if ((fd = open(file, O_RDWR | O_CREAT, 0644)) < 0) 
    {
      olc_perror("whatnow: unable to create temp file");
      return(ERROR);
    }
  (void) close(fd);
  
  if (edit_first == TRUE) 
    edit_message(file, editor);

  if(!isatty(0))
    return(SUCCESS);

  while (TRUE) 
    {
      inbuf[0] = '\0';
      while (inbuf[0] == '\0') 
	{
	  (void) get_prompted_input("\nWhat now? (type '?' for options): ", 
				    inbuf);
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
      
      if (string_equiv(inbuf,"edit",max(strlen(inbuf),1)))
	edit_message(file, editor);
      else if (string_equiv(inbuf,"quit",max(strlen(inbuf),1)))
	return(NO_ACTION);
      else if (string_equiv(inbuf,"send",max(strlen(inbuf),1)))
	return(SUCCESS);
      else if (string_equiv(inbuf,"list",max(strlen(inbuf),1)))
	display_file(file);
    }
}

/*
 * Function:	edit() invokes an editor on the specified file.
 * Arguments:	file:	name of file to be edited
 *		editor:	name of editor
 * Returns:	nothing.
 * Notes:
 *	If argument 'editor' is NULL, then try getenv("EDITOR"), and
 *	finally default to DFLT_EDITOR.
 */

static char *editor_name = (char *)NULL;

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
	  if ((editor_name = (char *) getenv("EDITOR")) == (char *)NULL)
	    editor_name = DEFAULT_EDITOR;
	}
      editor = editor_name;
    }
  (void) call_program(editor, file);
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

  if ((nbytes = file_length(msgfile)) == ERROR)
    {
      olc_perror("mail");
      printf("Unable to get message file.\n");
      return(ERROR);
    }

  msgbuf = (char *)malloc((unsigned) nbytes);
  if ((filedes = open(msgfile, O_RDONLY, 0)) <= 0) 
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
      (void) close(filedes);
      return(ERROR);
    }
  if ((fd = sendmail(args)) < 0) 
    {
      printf("Error sending mail.\n");
      free(msgbuf);
      (void) close(filedes);
      return(ERROR);
    }
  if (write(fd, msgbuf, nbytes) != nbytes)
    {
      olc_perror("mail");
      printf("Error sending mail.\n");
      free(msgbuf);
      (void) close(filedes);
      (void) close(fd);
      wait(0);	/* clean up sendmail process */
      return(ERROR);
    }
  (void) close(fd);
  free(msgbuf);
  (void) close(filedes);
  wait(0);		/* clean up sendmail process */
  return(SUCCESS);
}



char *
happy_message()
{
  if(random()%3 == 1)
    {
      switch(random()%12)
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
