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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/notify.c,v $
 *      $Author: raeburn $
 */

#ifndef lint
static char rcsid[] =
    "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/notify.c,v 1.17 1990-03-01 18:28:28 raeburn Exp $";
#endif


#include <sys/types.h>
#include <sys/socket.h>	        /* IPC socket defs. */
#include <sys/file.h>           /* File handling defs. */
#include <sys/stat.h>           /* File status defs. */
#include <sys/wait.h>           /* */
#include <pwd.h>                /* Directory defs. */
#include <signal.h>             /* System signal definitions. */
#include <sgtty.h>              /* Terminal param. definitions. */
#include <setjmp.h>

#include <olc/lang.h>

#if is_cplusplus
extern "C" {
#endif
#include <netdb.h>              /* Net database defs. */
#include <sys/uio.h>		/* for perror() */
#ifdef ZEPHYR
#include <com_err.h>
#define class c_class
#include <zephyr/zephyr.h>
#undef class
#endif /* ZEPHYR */
#if is_cplusplus
    extern int writev (int, struct iovec *, int);
};
#endif

#include <olc/olc.h>
#include <olcd.h>

/* External Variables. */

extern char DaemonHost[];	/* Name of daemon's machine. */

int use_zephyr = 1;

#if __STDC__
int notice_timeout(int a);
#else
int notice_timeout();
#endif
static jmp_buf env;

#if __STDC__
#ifdef ZEPHYR
static ERRCODE zwrite_message (const char *, const char *);
static ERRCODE zsend_message (const char *, const char *, const char *,
			      const char *, const char *, int);
static ERRCODE zsend (ZNotice_t *);
#endif
#endif

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

ERRCODE
#if __STDC__
write_message(const char *touser, const char *tomachine, const char *fromuser,
	      const char *frommachine, const char *message)
#else
write_message(touser, tomachine, fromuser, frommachine, message)
     char *touser, *tomachine, *fromuser, *frommachine, *message;
#endif
{
	FILE *tf = NULL;	/* Temporary file. */
	int fds;		/* Socket descriptor. */
	char buf[BUF_SIZE];	/* Message buffer. */
	char error[ERROR_SIZE];	/* Error message. */
	struct hostent *host;	/* Host entry for receiver. */
	struct sockaddr_in sin;	/* Socket address. */
	int flag = 0;

	if (touser == (char *)NULL) /* User sanity check. */
		return(ERROR);

#ifdef ZEPHYR
 	/* First try using Zephyr write.  If return status is anything
	 * but SUCCESS, try again using Unix write.
	 */
	if (zwrite_message(touser, message) == SUCCESS)
	  return(SUCCESS);
#endif ZEPHYR

	if (write_port == 0) {
		struct servent *service;
		service = getservbyname("write", "tcp");
		if (!service) {
			log_error("write_message: Can't find 'write' service");
			return(ERROR);
		}
		write_port = service->s_port;
	}

	host = gethostbyname(tomachine);
	if (host == (struct hostent *)NULL) {
		(void) sprintf(error, 
			       "Can't resolve name of host '%s'", tomachine);
		log_error(error);
		return(ERROR);
	}
	sin.sin_family = host->h_addrtype;
	bcopy(host->h_addr, (char *) &sin.sin_addr, host->h_length);
	sin.sin_port = write_port;
	fds = socket(host->h_addrtype, SOCK_STREAM, 0);
	if (fds < 0) {
		perror("socket");
		exit(1);
	}


        signal(SIGALRM, notice_timeout);
        alarm(OLCD_TIMEOUT);
        if(setjmp(env) != 0) {
                sprintf(error, "Unable to contact writed on %s", tomachine);
                log_error(error);
		if(tf!=NULL)
		  fclose(tf);
		close(fds);
                alarm(0);
                return(ERROR);
        }


	if (connect(fds, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
	  alarm(0);
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
	return(SUCCESS);
}

 
int
#if __STDC__
notice_timeout(int a)
#else
notice_timeout(a)
     int a;
#endif
{
    longjmp(env, 1);
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
#if __STDC__
write_message_to_user(KNUCKLE *k, char *message, int flags)
#else
write_message_to_user(k, message, flags)
     KNUCKLE *k;
     char *message;
     int flags;
#endif
{
  int result;		/* Result of writing the message. */
  char msgbuf[BUF_SIZE];	/* Message buffer. */
  int status;

  if (k == (KNUCKLE *) NULL)
    return(ERROR);

  if(k->user->no_knuckles > 1)
    {
      sprintf(msgbuf,"To: %s %s@%s [%d]\n",k->title,k->user->username,
	      k->user->realm,k->instance);
      strcat(msgbuf,message);
    }
  else
    strcpy(msgbuf,message);

  result = write_message(k->user->username, k->user->machine,
			 "OLC-Service", DaemonHost, msgbuf);
  
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
}

#define MESSAGE_CLASS "MESSAGE"
#define PERSONAL_INSTANCE "PERSONAL"

#define OLC_CLASS    "OLC"
#ifdef OLZ
#define OLC_CLASS    "OLZ"
#endif
#ifdef OLTA
#define OLC_CLASS    "OLTA"
#endif

ERRCODE
#if __STDC__
olc_broadcast_message(const char *instance, const char *message,
		      const char *code)
#else
olc_broadcast_message(instance, message, code)
     char *instance, *message, *code;
#endif
{
#ifdef ZEPHYR  
  if(zsend_message(OLC_CLASS,instance,code,"",message,0) == ERROR)
    return(ERROR);
#endif ZEPHYR

  return(SUCCESS);
}

#ifdef ZEPHYR

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
#if __STDC__
zwrite_message(const char *username, const char *message)
#else
zwrite_message(username, message)
     char *username, *message;
#endif
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

  if(zsend_message(MESSAGE_CLASS,PERSONAL_INSTANCE,"olc hello",
		   username,message,0) 
     == ERROR)
    return(ERROR);
   
   return(SUCCESS);
}


static ERRCODE
#if __STDC__
zsend_message(const char *c_class, const char *instance, const char *opcode,
	      const char *username, const char *message, int flags)
#else
zsend_message(c_class, instance, opcode, username, message, flags)
     char *c_class, *instance, *opcode, *username, *message;
     int flags;
#endif
{
  ZNotice_t notice;		/* Zephyr notice */
  int ret, len;			/* return value, length */
  char error[ERROR_SIZE];
  char buf[BUF_SIZE];
  char *signature = "From: OLC Service\n";

#ifdef lint
  flags = flags;
#endif lint;

  if (!use_zephyr)
      return ERROR;

  if ((ret = ZInitialize()) != ZERR_NONE) {
      com_err ("zwrite_message", ret, "couldn't ZInitialize");
      return ERROR;		/* Oops, couldn't initialize. */
  }
  
  bzero(&notice, sizeof(ZNotice_t));

  notice.z_kind = (username && username[0]) ? ACKED : UNSAFE;
  notice.z_port = 0;
  /*
   * The Zephyr header files don't deal with `const', but I don't
   * think these routines modify these fields.
   */
  notice.z_class = (char *) c_class;
  notice.z_class_inst = (char *) instance;
  notice.z_sender = 0;
  notice.z_message_len = 0;
  notice.z_recipient = (char *) username;
  notice.z_default_format = "Message $message";
  notice.z_opcode = (char *) opcode;

  if (strlen (signature) + strlen (message) + 5 > BUF_SIZE)
      return ERROR;
  (void) strcpy(buf,signature);
  (void) strcat(buf,message);
  len = strlen (buf);
  if (buf[len-1] != '\n')
      buf[len++] = '\n';

  /* Watch the moving pointer.... */
  notice.z_message = buf;     
  notice.z_message_len = len + 1;
  ret = zsend(&notice); /* send real message */

  return(ret);  
}
  
/*
 * Function:	zsend(&notice, isreal): Send a Zephyr notice.
 * Arguments:	notice: Zephyr notice to send.
 * Returns:	SUCCESS on success, else ERROR
 */

static ERRCODE
#if __STDC__
zsend(ZNotice_t *notice)
#else
zsend(notice)
     ZNotice_t *notice;
#endif
{
  int ret;
  ZNotice_t retnotice;

  if ((ret = ZSendNotice(notice, ZAUTH)) != ZERR_NONE)
    {
      /* Some sort of unknown communications error. */
      fprintf(stderr, "zsend: error %s from ZSendNotice\n",
	      error_message (ret));
      return(ERROR);
    }

  if(notice->z_kind != ACKED)
    return(SUCCESS);

  if ((ret = ZIfNotice(&retnotice, (struct sockaddr_in *) 0,
		       ZCompareUIDPred, (char *) &notice->z_uid)) !=
      ZERR_NONE)
    {
      /* Server acknowledgement error here. */
      fprintf(stderr, "zsend: error %s from ZIfNotice\n",
	      error_message (ret));
      ZFreeNotice(&retnotice);
      return(ERROR);
    }

  if (retnotice.z_kind == SERVNAK)
    {
      fprintf(stderr, "zsend: authentication failure (SERVNAK)\n");
      ZFreeNotice(&retnotice);
      return(ERROR);
    }

  if (retnotice.z_kind != SERVACK || !retnotice.z_message_len)
    {
      fprintf(stderr, "zsend: server failure during SERVACK\n");
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
#ifdef TEST
      printf("zsend: unknown error sending Zephyr message\n");
#endif
      ZFreeNotice(&retnotice);
      return(ERROR);   	/* Some error, probably not using Zephyr */
    }
}



#endif ZEPHYR

/*
 * Function:	perror() similar to that of the C library, except that
 *	a datestamp precedes the message printed.
 * Arguments:	msg:	Message to print.
 * Returns:	nothing
 *
 */

extern int sys_nerr;
extern char *sys_errlist[];
extern int errno;
static char time_buf[20];

void
#if __STDC__
perror(const char *msg)
#else
perror(msg)
     char *msg;
#endif
{
	register int error_number;
	struct iovec iov[6];
	register struct iovec *v = iov;

	error_number = errno;

	time_now(time_buf);
	v->iov_base = time_buf;
	v->iov_len = strlen(time_buf);
	v++;

	v->iov_base = " ";
	v->iov_len = 1;
	v++;

	if (msg) {
		if (*msg) {
			v->iov_base = (char *) msg;
			v->iov_len = strlen(msg);
			v++;
			v->iov_base = ": ";
			v->iov_len = 2;
			v++;
		}
	}

	if (error_number < sys_nerr)
		v->iov_base = sys_errlist[error_number];
	else
		v->iov_base = "Unknown error";
	v->iov_len = strlen(v->iov_base);
	v++;

	v->iov_base = "\n";
	v->iov_len = 1;
	(void) writev(2, iov, (v - iov) + 1);
}
