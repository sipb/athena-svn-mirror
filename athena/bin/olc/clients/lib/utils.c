/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains miscellaneous utilties for the olc and olcr programs.
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
 *	$Id: utils.c,v 1.30 1999-07-08 22:56:44 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: utils.c,v 1.30 1999-07-08 22:56:44 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

ERRCODE
fill_request(req)
     REQUEST *req;
{
#ifdef HAVE_KRB4
  CREDENTIALS k_cred;
  ERRCODE status;
#endif /* HAVE_KRB4 */

  memset(req, 0, sizeof(REQUEST));

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  req->requester.instance = User.instance;
  req->requester.uid      = User.uid;
  strncpy(req->requester.username, User.username, LOGIN_SIZE);
  strncpy(req->requester.realname, User.realname, NAME_SIZE);
  strncpy(req->requester.machine,  User.machine,
		 sizeof(req->requester.machine));

  req->target.instance = User.instance;
  req->target.uid      = User.uid;
  strncpy(req->target.username, User.username, LOGIN_SIZE);
  strncpy(req->target.realname, User.realname, NAME_SIZE);
  strncpy(req->target.machine,  User.machine,
		 sizeof(req->requester.machine));

#ifdef HAVE_KRB4
  status = krb_get_cred("krbtgt",REALM,REALM, &k_cred);
  if(status == KSUCCESS)
    {
      strncpy(req->target.username, k_cred.pname, LOGIN_SIZE);
      strncpy(req->requester.username, k_cred.pname, LOGIN_SIZE);
    }
  else
    fprintf(stderr,"%s\n",krb_err_txt[status]);
#endif /* HAVE_KRB4 */

  return(SUCCESS);
}

#ifdef CHECK_MAILHUB___BROKEN

#define MAIL_SUCCESS 250
char *host = "athena.mit.edu";
static struct hostent *hp_local = (struct hostent *) NULL;
static struct servent *sp_local = (struct servent *) NULL;

ERRCODE
open_connection_to_mailhost()
{
  static struct sockaddr_in sin;
  static int s;

  hp_local = gethostbyname(host);

  if (hp_local == (struct hostent *) NULL)
    {
      fprintf(stderr,"Unable to resolve name of local mail server %s.",host);
      return(ERROR);
    }

  sp_local = getservbyname("smtp", "tcp");
  if (sp_local == (struct servent *) NULL)
    {
      fprintf(stderr, "Unable to locate local mail service.");
      return(ERROR);
    }

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
     {
       olc_perror("no socket");
       return(ERROR);
     }

  memset(&sin, 0, sizeof (sin));
  memcpy(&sin.sin_addr, hp_local->h_addr, hp_local->h_length);
  sin.sin_family = AF_INET;
  sin.sin_port = sp_local->s_port;
  if (connect(s, (struct sockaddr *)(&sin), sizeof(sin)) < 0)
    {
      fprintf(stderr,"Unable to connect to mailhost\n");
      return(ERROR);
    }
  return(s);
}

ERRCODE
query_mailhost(s,name)
     int s;
     char *name;
{
  char buf[LINE_SIZE];
  int code;

  sread(s,buf,LINE_SIZE * sizeof(char));
  sprintf(buf,"vrfy %s\n",name);
  swrite(s,buf,strlen(buf) * sizeof(char));

  sread(s,buf,LINE_SIZE * sizeof(char));
  code = atoi(buf);

  sprintf(buf, "quit\n");
  swrite(s,buf,strlen(buf) * sizeof(char));

  sread(s,buf,LINE_SIZE * sizeof(char));

  return(code);
 }

#endif /* CHECK_MAILHUB___BROKEN */

ERRCODE
can_receive_mail(name)   /*ARGSUSED*/
     char *name;
{
#ifdef CHECK_MAILHUB___BROKEN
  static int fd, code;
  ERRCODE status;

  fd = open_connection_to_mailhost();
  if(fd == ERROR)
    return(ERROR);

  code = query_mailhost(fd,name);
  status = close(fd);
  if(status == ERROR)
    {
      olc_perror("fatal error");
      exit(1);
    }
  if(code != MAIL_SUCCESS)
    return(ERROR);
  else
    return(SUCCESS);

#else  /* not CHECK_MAILHUB___BROKEN */
  fprintf(stderr, "Warning: mailhub checking is not supported...\n");
  return(SUCCESS);
#endif /* not CHECK_MAILHUB___BROKEN */
}


/*
 * Function:	call_program() executes the named program by forking the
 *			olc process.
 * Arguments:	program:	Name of the program to execute.
 *		argument:	Argument to be passed to the program.
 *	Note: Currently we support only a single argument, though it
 *	may be necessary to extend this later.
 * Returns:	An error code.
 * Notes:
 *	First, we fork a new process.  If the fork is unsuccessful, this
 *	fact is logged, and we return an error.  Otherwise, the child
 *	process (pid = 0) exec's the desired program, while the parent
 *	program waits for it to finish.
 */

ERRCODE
call_program(program, argument)
     char *program;		/* Name of program to be called. */
     char *argument;		/* Argument to be passed to program. */
{
  int pid;		/* Process id for forking. */
  int child_status;           /* Status of child */
  struct sigaction sa, osa;

  pid = fork();
  if (pid == -1)
    {
      olc_perror("call_program");
      return(ERROR);
    }
  else 
    if (pid == 0) 
      {
	execlp(program, program, argument, 0);
	olc_perror("call_program");
	_exit(37); /* Flag value, hopefully it doesn't step on return
		    * values of program.
		    */
      }
    else 
      {
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler= SIG_IGN;
	sigaction(SIGINT, &sa, &osa);

	waitpid(pid, &child_status, 0);
	
	sigaction(SIGINT, &osa, NULL);
	if((WIFEXITED(child_status)) && (WEXITSTATUS(child_status) == 37))
	  return(ERROR); /* The execlp failed. */
	else
	  return(SUCCESS);
      }
}




/*
 * Function:    expand_hostname()
 * Arguments:   char *hostname:    
 *              char *instance:  return
 *              char *realm:     return
 *
 * Returns:     Nothing
 * Notes:       Parses hostname for instance & realm.
 *              Snarfed from kerberos document
 */

#ifdef HAVE_KRB4
void
expand_hostname(hostname, instance, realm)
     char *hostname;
     char *instance;
     char *realm;
{
  char *p;
  int i;

  realm[0] = '\0';
  p = strchr(hostname, '.');
  
  if(p == NULL)
    {
      strcpy(instance, hostname);
      krb_get_lrealm(realm,1);
    }
  else
    {
      char *krb_conf_realm;

      i = p-hostname;
      strncpy(instance,hostname,i);
      instance[i] = '\0';

      krb_conf_realm = krb_realmofhost(hostname);
      if (krb_conf_realm)
	strcpy(realm, krb_conf_realm);
      else
	strcpy(realm, p+1);   /* a poor man's guess */
    }

  /* lowercase host "instance" */
  for (i=0; instance[i] != '\0'; i++)
    if (isupper(instance[i]))
      instance[i] = tolower(instance[i]);

  /* upcase realm */
  for (i=0; realm[i] != '\0'; i++)
    if (islower(realm[i]))
      realm[i] = toupper(realm[i]);

  /* if the realm is one of LOCAL_REALMS[], map it to LOCAL_REALM */
  for(i=0; strlen(LOCAL_REALMS[i]) != 0; i++)
    if(strcmp(realm, LOCAL_REALMS[i]) == 0)
      strcpy(realm, LOCAL_REALM);
  return;
}
#endif /* HAVE_KRB4 */

/*
 * Function:	sendmail() forks a sendmail process to send mail to someone.
 * Arguments:	username:	Name of user receving the mail.
 *		machine:	His machine.
 * Returns:	A file descriptor of a pipe to the sendmail process.
 * Notes:
 *	First, create a pipe so we can fork a sendmail child process.
 *	Then execute the fork, logging an error if we are unable to do.
 *	As usual in a situtation like this, check the process ID returned
 *	by fork().  If it is zero, we are in the child, so we execl
 *	sendmail with the appropriate arguments.  Otherwise, close
 *	the zeroth file descriptor, which is for reading.  Then construct
 *	the mail address and write it to sendmail.  Finally, return the
 *	file descriptor so the message can be sent.
 */

int
sendmail(smargs)
     char **smargs;
{
  int fildes[2];	/* File descriptor array. */
  char *args[NAME_SIZE];
  int i = 2;

  args[0] = "sendmail";
  args[1] = "-t";

  if(smargs != (char **) NULL)
    while (smargs && *smargs)
      {
	args[i] = *smargs;
	++i;  ++smargs;
	if(i > NAME_SIZE)
	  break;
      }
  args[i] = (char *) 0;

  pipe(fildes);
  switch (fork()) 
    {
    case -1:		/* error */
      olc_perror("mail");
      printf("sendmail: error starting process.\n");
      return(-1);
    case 0:		/* child */
      close(fildes[1]);
      close(0);
      dup2(fildes[0], 0);
      execv("/usr/sbin/sendmail", args);
      execv("/usr/lib/sendmail", args);
      olc_perror("sendmail: exec");
      exit(1);
    default:
      close(fildes[0]);
      return(fildes[1]);
    }
}

int
file_length(file)
     char *file;
{
  struct stat statbuf;
  
  if(stat(file, &statbuf) < 0)
    return(ERROR);

  return(statbuf.st_size);
}
