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
 *	$Id: utils.c,v 1.26 1999-01-22 23:12:15 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: utils.c,v 1.26 1999-01-22 23:12:15 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

ERRCODE
OFillRequest(req)
     REQUEST *req;
{
  return(fill_request(req));
}

ERRCODE
fill_request(req)
     REQUEST *req;
{
#ifdef KERBEROS
  CREDENTIALS k_cred;
  int status;
#endif /* KERBEROS */

  memset(req, 0, sizeof(REQUEST));

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  req->requester.instance = User.instance;
  req->requester.uid      = User.uid;
  (void) strncpy(req->requester.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->requester.realname, User.realname, NAME_SIZE);
  (void) strncpy(req->requester.machine,  User.machine,
		 sizeof(req->requester.machine));

  req->target.instance = User.instance;
  req->target.uid      = User.uid;
  (void) strncpy(req->target.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->target.realname, User.realname, NAME_SIZE);
  (void) strncpy(req->target.machine,  User.machine,
		 sizeof(req->requester.machine));

#ifdef KERBEROS
  status = krb_get_cred("krbtgt",REALM,REALM, &k_cred);
  if(status == KSUCCESS)
    {
      (void) strncpy(req->target.username, k_cred.pname, LOGIN_SIZE);
      (void) strncpy(req->requester.username, k_cred.pname, LOGIN_SIZE);
    }
  else
    fprintf(stderr,"%s\n",krb_err_txt[status]);
#endif /* KERBEROS */

  return(SUCCESS);
}

#ifdef ATHENA

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

#endif /* ATHENA */

ERRCODE
can_receive_mail(name)   /*ARGSUSED*/
     char *name;
{
#ifdef ATHENA
  static int fd, code;
  int status;

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

#else  /* ATHENA */
  return(SUCCESS);
#endif /* ATHENA */
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
#ifdef POSIX
  struct sigaction sa, osa;
#else /* not POSIX */
#ifdef VOID_SIGRET
  void (*func)(int);
#else /* not VOID_SIGRET */
  int (*func)();
#endif /* not VOID_SIGRET */
#endif /* not POSIX */

#ifdef NO_VFORK
  pid = fork();
#else
  pid = vfork();
#endif
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
	return(ERROR);
      }
    else 
      {
#ifdef POSIX
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler= SIG_IGN;
	(void) sigaction(SIGINT, &sa, &osa);
#else /* not POSIX */
	func = signal(SIGINT, SIG_IGN);
#endif /* not POSIX */
	while (wait(0) != pid) 
	  {
			/* ho hum ... (yawn) */
            /* tap tap */
	  };
#ifdef POSIX
	(void) sigaction(SIGINT, &osa, NULL);
#else /* not POSIX */
	signal(SIGINT, func);
#endif /* not POSIX */
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

#ifdef KERBEROS
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
      (void) strcpy(instance, hostname);
      krb_get_lrealm(realm,1);
    }
  else
    {
      i = p-hostname;
      (void) strncpy(instance,hostname,i);
      instance[i] = '\0';
      (void) strcpy(realm, p+1);
    }

#ifdef REALM
  if(strlen(realm) == 0)
    (void) strcpy(realm, LOCAL_REALM);
#endif /* REALM */

  for(i=0; instance[i] != '\0'; i++)
    if(isupper(instance[i]))
      instance[i] = tolower(instance[i]);

  for(i=0; realm[i] != '\0'; i++)
    if(islower(realm[i]))
      realm[i] = toupper(realm[i]);
  
  for(i=0; strlen(LOCAL_REALMS[i]) !=0; i++)
    if(strcmp(realm, LOCAL_REALMS[i]) == 0)
      (void) strcpy(realm, LOCAL_REALM);
  return;
}
#endif /* KERBEROS */

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

  (void) pipe(fildes);
  switch (fork()) 
    {
    case -1:		/* error */
      olc_perror("mail");
      printf("sendmail: error starting process.\n");
      return(-1);
    case 0:		/* child */
      (void) close(fildes[1]);
      (void) close(0);
      (void) dup2(fildes[0], 0);
      execv("/usr/sbin/sendmail", args);
      execv("/usr/lib/sendmail", args);
      olc_perror("sendmail: exec");
      exit(1);
    default:
      (void) close(fildes[0]);
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
