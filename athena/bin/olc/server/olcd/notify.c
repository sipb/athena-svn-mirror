/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for handling the daemon's I/O.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: notify.c,v 1.41 1999-03-06 16:48:56 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: notify.c,v 1.41 1999-03-06 16:48:56 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>	        /* IPC socket defs. */
#include <sys/file.h>           /* File handling defs. */
#include <sys/stat.h>           /* File status defs. */
#include <sys/wait.h>           /* */
#include <netinet/in.h>
#include <pwd.h>                /* Directory defs. */
#include <signal.h>             /* System signal definitions. */
#include <sgtty.h>              /* Terminal param. definitions. */
#include <setjmp.h>

#include <netdb.h>              /* Net database defs. */
#ifdef HAVE_ZEPHYR
#include <com_err.h>
#include <zephyr/zephyr.h>
#endif /* HAVE_ZEPHYR */

#include <olcd.h>

/* External Variables. */

extern char DaemonHost[];	/* Name of daemon's machine. */
extern char DaemonInst[];	/* "OLC", "OLTA", etc. */
extern char DaemonZClass[];	/* Zephyr class to use, usually DaemonInst */

static int punt_zephyr = 0;
static long zpunt_time;
static int zpunt_duration;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static RETSIGTYPE notice_timeout P((int a ));

#ifdef HAVE_ZEPHYR
static ERRCODE zwrite_message P((char *username , char *message ));
static ERRCODE zsend_message P((char *c_class , char *instance , char *opcode , char *username , char *message ));
static ERRCODE zsend P((ZNotice_t *notice ));
#endif /* HAVE_ZEPHYR */
#undef P


static jmp_buf env;

/*
 * Function:	write_message() uses the program "write" to send a message
 *			from one person to another within the OLC system.
 * Arguments:	touser:		Username of person receiving the message.
 *		tomachine:	Name of machine he is using.
 *		fromuser:	Username of person sending the message.
 *		frommachine:	Name of machine she is using.
 *		message:	Ptr. to buffer containing message.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First try using zwrite_message() for Zephyr users; if that fails,
 *	use the standard Unix write code.
 *	This code is essentially the same as that in write.c for the
 *	program 'write'.
 *
 */

static int write_port = 0;

static ERRCODE
write_message(touser, tomachine, fromuser, frommachine, message)
     char *touser, *tomachine, *fromuser, *frommachine, *message;
{
	FILE *tf = NULL;	/* Temporary file. */
	int fds;		/* Socket descriptor. */
	char buf[BUF_SIZE];	/* Message buffer. */
	char error[ERROR_SIZE];	/* Error message. */
	struct hostent *host;	/* Host entry for receiver. */
	struct sockaddr_in sin;	/* Socket address. */
	int flag = 0;
	long time_now;
	unsigned long ip_addr;
#ifdef HAVE_SIGACTION
	struct sigaction action;
#endif

	if (touser == (char *)NULL) /* User sanity check. */
		return(ERROR);

#ifdef HAVE_ZEPHYR
 	/* First try using Zephyr write.  If return status is anything
	 * but SUCCESS, try again using Unix write.
	 */

	if (punt_zephyr)	/* If we've punted zephyr for more than */
				/* 15 minutes, try using it again. */
	  {
	    (void) time(&time_now);
	    if ((zpunt_duration != 0) && 
		((time_now - zpunt_time) > zpunt_duration)) {
	      log_zephyr_error("Attempting to unpunt zephyr...");
	      toggle_zephyr(0,0);
	    }
	  }

	if (!punt_zephyr)
	  {
	    if (zwrite_message(touser, message) == SUCCESS)
	      return(SUCCESS);
	  }
#endif /* HAVE_ZEPHYR */

	if (write_port == 0) {
		struct servent *service;
		service = getservbyname("write", "tcp");
		if (!service) {
			log_error("write_message: Can't find 'write' service");
			return(ERROR);
		}
		write_port = service->s_port;
	}

	/* "tomachine" may be a hostname or a numeric IP address. */
	if (strspn(tomachine, "01234567890.") == strlen(tomachine))
	  {
	    /* string contains a numeric IP address (#.#.#.#) */

	    ip_addr = inet_addr(tomachine);
	    if (ip_addr == (unsigned long)-1)
	      {
		log_error("Can't convert '%s' to IP address", tomachine);
		return(ERROR);
	      }

	    sin.sin_family = AF_INET;
	    sin.sin_addr.s_addr = ip_addr;
	  }
	else
	  {
	    /* string contains a hostname */

	    host = gethostbyname(tomachine);
	    if (host == (struct hostent *)NULL)
	      {
		log_error("Can't resolve hostname '%s'", tomachine);
		return(ERROR);
	      }

	    sin.sin_family = host->h_addrtype;
	    memcpy(&sin.sin_addr, host->h_addr, host->h_length);
	  }

	sin.sin_port = write_port;
	fds = socket(PF_INET, SOCK_STREAM, 0);
	if (fds < 0) {
		log_error("write_message: socket: %m");
		exit(2);
	}

#ifdef HAVE_SIGACTION
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	action.sa_handler = notice_timeout;
	sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
        signal(SIGALRM, notice_timeout);
#endif /* don't HAVE_SIGACTION */
        if(setjmp(env) != 0) {
                log_status("Unable to contact writed on %s", tomachine);
		if(tf!=NULL)
		  fclose(tf);
		close(fds);
                alarm(0);
#ifdef HAVE_SIGACTION
		action.sa_handler = SIG_IGN; /* struct already initialized */
		sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
		signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
                return(ERROR);
        }
        alarm(OLCD_TIMEOUT);


	if (connect(fds, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
	  alarm(0);
#ifdef HAVE_SIGACTION
	  action.sa_handler = SIG_IGN;       /* struct already initialized */
	  sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
	  signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
	  (void) close(fds);
	  return(MACHINE_DOWN);
	}
	(void) write(fds, fromuser, strlen(fromuser));
	(void) write(fds, "@", 1);
	(void) write(fds, frommachine, strlen(frommachine));
	(void) write(fds, " ", 1);
	(void) write(fds, touser, strlen(touser));
	(void) write(fds, "\r\n", 2);
	tf = fdopen(fds, "r");
	flag++;
	while (1) {
		if (fgets(buf, sizeof(buf), tf) == (char *)NULL) {
		  (void) fclose(tf);
		  (void) close(fds);
		  alarm(0);
#ifdef HAVE_SIGACTION
		  action.sa_handler = SIG_IGN; /* struct already initialized */
		  sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
		  signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
		  return(LOGGED_OUT);
		}
		if (buf[0] == '\n')
			break;
		(void) write(1, buf, strlen(buf));
	}
	(void) write(fds, message, strlen(message));
	(void) write(fds, "\r\n", 2);
	(void) fclose(tf);
	(void) close(fds);
	alarm(0);
#ifdef HAVE_SIGACTION
	action.sa_handler = SIG_IGN;         /* struct already initialized */
	sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
	signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
	return(SUCCESS);
}

 
static RETSIGTYPE
notice_timeout(a)
     int a;
{
#ifdef SABER
  a = a;  /* Rand lives! :-) */
#endif
    longjmp(env, 1);
  /*NOTREACHED*/
  return;
}

    
/*
 * Function:	write_message_to_user() sends a message from the
 *			daemon to a user using "write".
 * Arguments:	user:		Ptr. to user structure.
 *		message:	Ptr. to buffer containing the message.
 *		flags:		Specifies special actions.
 * Returns:	SUCCESS or ERROR.
 * Notes:
 *	First, try to write a message to the user.  If it does not
 *	succeed, notify the consultant.
 */


ERRCODE
write_message_to_user(k, message, flags)
     KNUCKLE *k;
     char *message;
     int flags;
{
  int result;		/* Result of writing the message. */
  char msgbuf[BUF_SIZE];	/* Message buffer. */
  static char namebuf[BUF_SIZE];
  int status;

  if (k == (KNUCKLE *) NULL)
    return(ERROR);

#ifdef OLCD_SILENT
  /* if OLCD_SILENT is defined, we never talk directly to users. */
  return(SUCCESS);
#else /* not OLCD_SILENT */

  if (namebuf[0] == 0)  sprintf(namebuf,"%s_Service",DaemonInst);

  if(k->user->no_knuckles > 1)
    {
      sprintf(msgbuf,"To: %s %s@%s [%d]\n",k->title,k->user->username,
	      k->user->realm,k->instance);
      strcat(msgbuf,message);
    }
  else
    strcpy(msgbuf,message);

  result = write_message(k->user->username, k->user->machine,
			 namebuf, DaemonHost, msgbuf);
  
  switch(result)
    {
    case ERROR:
      set_status(k->user, UNKNOWN_STATUS);
      (void) sprintf(msgbuf,"Unable to contact %s %s.  Cause unknown.",
		     k->title, k->user->username);
      log_daemon(k, msgbuf);
      if(!(flags & NO_RESPOND))
	  (void) write_message_to_user(k->connected, msgbuf, NO_RESPOND);
      status = UNKNOWN_STATUS;
      break;

    case MACHINE_DOWN:
      set_status(k->user, MACHINE_DOWN);
      (void) sprintf(msgbuf,"Unable to contact %s %s.  Host machine down.",
		     k->title, k->user->username);
      log_daemon(k, msgbuf);
      if(!(flags & NO_RESPOND))
	  (void) write_message_to_user(k->connected, msgbuf, NO_RESPOND);
      status = MACHINE_DOWN;
      break;

    case LOGGED_OUT:
      set_status(k->user, LOGGED_OUT);
      (void) sprintf(msgbuf,"Unable to contact %s %s.  User logged out.",
		     k->title, k->user->username);
      log_daemon(k, msgbuf);
      if(!(flags & NO_RESPOND))
	  (void) write_message_to_user(k->connected, msgbuf, NO_RESPOND);
      status = LOGGED_OUT;
      break;

    default:
      set_status(k->user,ACTIVE);
      status = SUCCESS;
      break;
    }
  return(status);
#endif /* not OLCD_SILENT */
}

#define MESSAGE_CLASS "MESSAGE"
#define PERSONAL_INSTANCE "PERSONAL"

/*
 * Function:	olc_broadcast_message(instance, message, code)
 *		Broadcasts a zephyr message to a specified instance.
 * Arguments:	instance:	Zephyr instance to broadcast to.
 *		message:	Message to send.
 *		code:		Zephyr Opcode to tack onto message.
 * Returns:	SUCCESS if message sent successfully, else ERROR.
 *
 */

ERRCODE
olc_broadcast_message(instance, message, code)
     char *instance, *message, *code;
{
#ifdef HAVE_ZEPHYR  
  if (punt_zephyr)
    return(ERROR);

  if(zsend_message(DaemonZClass, instance, code, "", message) == ERROR)
    return(ERROR);
#endif /* HAVE_ZEPHYR */

  return(SUCCESS);
}

#ifdef HAVE_ZEPHYR


/*
 * Function:	zwrite_message(username, message) writes a message to the
 *		specified user.
 * Arguments:	username:	Username of intended recipient.
 *		message:	Message to send.
 * Returns:	SUCCESS if message sent successfully, else ERROR.
 *
 * Inspired greatly by Robert French's zwrite.c (i.e. somewhat swiped).
 *
 */


static ERRCODE
zwrite_message(username, message)
     char *username, *message;
{

    /* Sanity check the username. */
  if (username == NULL)
    {
      log_error("zwrite_message: null username");
      return(ERROR);
    }
  if (strlen(username) == 0)
    {
      log_error("zwrite_message: zero length username");
      return(ERROR);
    }

  if(zsend_message(MESSAGE_CLASS,PERSONAL_INSTANCE,"olc hello", username,
		   message)
     == ERROR)
    return(ERROR);
   
   return(SUCCESS);
}


static ERRCODE
zsend_message(c_class, instance, opcode, username, message)
     char *c_class, *instance, *opcode, *username, *message;
{
  ZNotice_t notice;		/* Zephyr notice */
  int ret;			/* return value */
  char buf[BUF_SIZE];
  static char signature[100];	/* Zephyr Signature */

  if (signature[0] == 0)
    sprintf(signature,"From: %s Service\n",DaemonInst);
  memset(&notice, 0, sizeof(ZNotice_t));

  notice.z_kind = (username && username[0]) ? ACKED : UNSAFE;
  notice.z_port = 0;
  /*
   * The Zephyr header files don't deal with `const', but I don't
   * think these routines modify these fields.
   */
  notice.z_class = (char *) c_class;
  notice.z_class_inst = (char *) instance;
  notice.z_sender = 0;
  notice.z_recipient = (char *) username;
  notice.z_default_format = "Message $message";
  notice.z_opcode = (char *) opcode;

  if (strlen (signature) + strlen (message) + 5 > BUF_SIZE)
      return ERROR;
  (void) strcpy(buf,signature);
  (void) strcat(buf,message);
  if (buf[strlen(buf)-1] != '\n')
    strcat(buf, "\n");

  /* Watch the moving pointer.... */
  notice.z_message = buf;     
  notice.z_message_len = strlen(buf);
  ret = zsend(&notice); /* send real message */

  return(ret);  
}
  
/*
 * Function:	zsend(&notice, isreal): Send a Zephyr notice.
 * Arguments:	notice: Zephyr notice to send.
 * Returns:	SUCCESS on success, else ERROR
 */

static ERRCODE
zsend(notice)
     ZNotice_t *notice;
{
  int ret;
  ZNotice_t retnotice;
  char buf[BUFSIZ];
#ifdef HAVE_SIGPROCMASK
  sigset_t sigchld, oldmask;
#else /* don't HAVE_SIGPROCMASK */
  int sigm;
#endif /* don't HAVE_SIGPROCMASK */
#ifdef HAVE_SIGACTION
  struct sigaction action;

  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = notice_timeout;
  sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
  signal(SIGALRM, notice_timeout);
#endif /* don't HAVE_SIGACTION */

  alarm(6 * OLCD_TIMEOUT);	/* Longer timeout than for "write". */

  if(setjmp(env) != 0)
    {
      toggle_zephyr(1,ZEPHYR_PUNT_TIME);
      log_zephyr_error("Unable to send message via zephyr.  Punting.");
      alarm(0);
#ifdef HAVE_SIGACTION
      action.sa_handler = SIG_IGN;           /* struct already initialized */
      sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
      signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
      return(ERROR);
    }

  ret = ZSendNotice(notice, ZAUTH);
  if (ret != ZERR_NONE)
    {
      /* Some sort of unknown communications error. */
      log_zephyr_error("zsend: error '%s' (%d) from ZSendNotice",
		       error_message(ret), ret);
      alarm(0);
#ifdef HAVE_SIGACTION
      action.sa_handler = SIG_IGN;           /* struct already initialized */
      sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
      signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
      return(ERROR);
    }

  if(notice->z_kind != ACKED)
    {
      alarm(0);			/* If notice isn't acked, no need to wait. */
#ifdef HAVE_SIGACTION
      action.sa_handler = SIG_IGN;           /* struct already initialized */
      sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
      signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
      return(SUCCESS);
    }

  /*
   * Need to block SIGCHLD signals in ZIfnotice; ZIfnotice doesn't like
   * being interrupted, and the timing is such that the SIGCHLD coming from
   * the dying lumberjack hits during the ZIfnotice, causing an interrupted
   * system call error message.
   */

#ifdef HAVE_SIGPROCMASK
  sigemptyset(&sigchld);
  sigaddset(&sigchld, SIGCHLD);
  sigprocmask(SIG_BLOCK, &sigchld, &oldmask);
#else /* don't HAVE_SIGPROCMASK */
  sigm = sigblock(sigmask(SIGCHLD));
#endif /* don't HAVE_SIGPROCMASK */
  ret = ZIfNotice(&retnotice, (struct sockaddr_in *) 0,
		  ZCompareUIDPred, (char *) &notice->z_uid);
  if (ret != ZERR_NONE)
    {
      /* Server acknowledgement error here. */
#ifdef HAVE_SIGPROCMASK
      sigprocmask(SIG_SETMASK, &oldmask, NULL);
#else /* don't HAVE_SIGPROCMASK */
      sigsetmask(sigm);
#endif /* don't HAVE_SIGPROCMASK */
      log_zephyr_error("zsend: error %s from ZIfNotice",error_message(ret));
      ZFreeNotice(&retnotice);
      alarm(0);
#ifdef HAVE_SIGACTION
      action.sa_handler = SIG_IGN;           /* struct already initialized */
      sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
      signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */
      return(ERROR);
    }

#ifdef HAVE_SIGPROCMASK
  sigprocmask(SIG_SETMASK, &oldmask, NULL);
#else /* don't HAVE_SIGPROCMASK */
  sigsetmask(sigm);
#endif /* don't HAVE_SIGPROCMASK */
  alarm(0);			/* If ZIfNotice came back, shut off alarm. */
#ifdef HAVE_SIGACTION
  action.sa_handler = SIG_IGN;               /* struct already initialized */
  sigaction(SIGALRM, &action, NULL);
#else /* don't HAVE_SIGACTION */
  signal(SIGALRM, SIG_IGN);
#endif /* don't HAVE_SIGACTION */

  if (retnotice.z_kind == SERVNAK)
    {
      log_error("zsend: authentication failure (SERVNAK)");
      ZFreeNotice(&retnotice);
      return(ERROR);
    }

  if (retnotice.z_kind != SERVACK || !retnotice.z_message_len)
    {
      log_error("zsend: server failure during SERVACK");
      ZFreeNotice(&retnotice);
      return(ERROR);
    }

  if (! strcmp(retnotice.z_message, ZSRVACK_SENT))
    {
      ZFreeNotice(&retnotice);
      return(SUCCESS);		/* Message made it */
    }
  else
    {
      ZFreeNotice(&retnotice);
      return(ERROR);   	/* Some error, probably not using Zephyr */
    }
}


/*
 * Function:	toggle_zephyr(toggle,time)
 * Arguments:	toggle: 1 = turn zephyr on; 0 = off
 *		time = number of minutes to turn it off for; 0 = indefinitely.
 */

void
toggle_zephyr(toggle,punt_time)
     int toggle,punt_time;
{
  int fd;
  char errbuf[256];

  if (toggle) {
    punt_zephyr = 1;
    (void) time(&zpunt_time);
    zpunt_duration = punt_time * 60;
    /* Create file to tell other daemons to punt zephyr */
    fd = creat(ZEPHYR_DOWN_FILE,0644);
    if (fd < 0) {
      log_error("error trying to create %s: %m",ZEPHYR_DOWN_FILE);
      return;
    }
    close(fd);
  } else {
    punt_zephyr = 0;
    if (unlink(ZEPHYR_DOWN_FILE) < 0) {
      log_error("error unlinking %s: %m",ZEPHYR_DOWN_FILE);
      return;
    }
  }
  return;
}
#endif /* HAVE_ZEPHYR */

