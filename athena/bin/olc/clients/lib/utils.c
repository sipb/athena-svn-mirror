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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/utils.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/utils.c,v 1.6 1989-11-17 14:20:58 tjcoppet Exp $";
#endif

#include <olc/olc.h>
#include <zephyr/zephyr.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

OFillRequest(req)
     REQUEST *req;
{
  return(fill_request(req));
}
  
fill_request(req)
     REQUEST *req;
{
#ifdef KERBEROS
  CREDENTIALS k_cred;
  int status;
#endif KERBEROS

  bzero(req, sizeof(REQUEST));

  req->options = NO_OPT;
  req->version = CURRENT_VERSION;

  req->requester.instance = User.instance;
  req->requester.uid      = User.uid;
  (void) strncpy(req->requester.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->requester.realname, User.realname, NAME_LENGTH);
  (void) strncpy(req->requester.machine,  User.machine,
		 sizeof(req->requester.machine));

  req->target.instance = User.instance;
  req->target.uid      = User.uid;
  (void) strncpy(req->target.username, User.username, LOGIN_SIZE);
  (void) strncpy(req->target.realname, User.realname, NAME_LENGTH);
  (void) strncpy(req->target.machine,  User.machine,
		 sizeof(req->requester.machine));

#ifdef KERBEROS
  if((status = krb_get_cred("krbtgt",REALM,REALM, &k_cred)) == KSUCCESS)
    {
      (void) strncpy(req->target.username, k_cred.pname, LOGIN_SIZE);
      (void) strncpy(req->requester.username, k_cred.pname, LOGIN_SIZE);
    }
  else
    fprintf(stderr,"%s\n",krb_err_txt[status]);
#endif KERBEROS

  return(SUCCESS);
}

#ifdef ATHENA

#define MAIL_SUCCESS 250
char *host = "athena.mit.edu";
static struct hostent *hp_local = (struct hostent *) NULL;
static struct servent *sp_local = (struct servent *) NULL;

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
       perror("no socket");
       return(ERROR);
     }

  bzero((char *)&sin, sizeof (sin));
  bcopy(hp_local->h_addr, (char *)&sin.sin_addr, hp_local->h_length);
  sin.sin_family = AF_INET;
  sin.sin_port = sp_local->s_port;
  if (connect(s, &sin, sizeof(sin)) < 0)
    {
      fprintf(stderr,"Unable to connect to mailhost\n");
      return(ERROR);
    }
  return(s);
}

query_mailhost(s,name)
     int s;
     char *name;
{
  char buf[LINE_LENGTH];
  int code;

  sread(s,buf,LINE_LENGTH * sizeof(char));
  sprintf(buf,"vrfy %s\n",name);
  swrite(s,buf,strlen(buf) * sizeof(char));

  sread(s,buf,LINE_LENGTH * sizeof(char));
  code = atoi(buf);

  sprintf(buf, "quit\n");
  swrite(s,buf,strlen(buf) * sizeof(char));

  sread(s,buf,LINE_LENGTH * sizeof(char));

  return(code);
 }

#endif ATHENA

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
      perror("fatal error");
      exit(1);
    }
  if(code != MAIL_SUCCESS)
    return(ERROR);
  else
    return(SUCCESS);

#else  ATHENA
  return(SUCCESS);
#endif ATHENA
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
  int (*func)();

  if ((pid = fork()) == -1) 
    {
      perror("call_program");
      return(ERROR);
    }
  else 
    if (pid == 0) 
      {
	execlp(program, program, argument, 0);
	perror("call_program");
	return(ERROR);
      }
    else 
      {
	func = signal(SIGINT, SIG_IGN);
	while (wait(0) != pid) 
	  {
			/* ho hum ... (yawn) */
            /* tap tap */
	  };
	signal(SIGINT, func);
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

void
expand_hostname(hostname, instance, realm)
     char *hostname;
     char *instance;
     char *realm;
{
  char *p;
  int i;

  realm[0] = '\0';
  p = index(hostname, '.');
  
  if(p == NULL)
    {
      (void) strcpy(instance, hostname);

#ifdef KERBEROS
      get_krbrlm(realm,1);
#endif KERBEROS

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
#endif REALM

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
  char *args[NAME_LENGTH];
  int i = 2;

  args[0] = "sendmail";
  args[1] = "-t";

  if(smargs != (char **) NULL)
    while (smargs && *smargs)
      {
	args[i] = *smargs;
	++i;  ++smargs;
	if(i > NAME_LENGTH)
	  break;
      }
  args[i] = (char *) 0;

  (void) pipe(fildes);
  switch (fork()) 
    {
    case -1:		/* error */
      perror("mail");
      printf("sendmail: error starting process.\n");
      return(-1);
    case 0:		/* child */
      (void) close(fildes[1]);
      (void) close(0);
      (void) dup2(fildes[0], 0);
      execv("/usr/lib/sendmail", args);
      perror("sendmail: exec");
      exit(1);
    default:
      (void) close(fildes[0]);
      return(fildes[1]);
    }
}


file_length(file)
     char *file;
{
  struct stat statbuf;
  
  if(stat(file, &statbuf) < 0)
    return(ERROR);

  return(statbuf.st_size);
}

  

#ifdef ZEPHYR
char *
zephyr_get_opcode(class, instance)
     char *class;
     char *instance;
{
  ZNotice_t notice;
  struct sockaddr_in from;
  Code_t retval;
  char *msg;

  while (1) 
    {
      if ((retval = ZReceiveNotice(&notice, &from)) != ZERR_NONE)
	{
	  com_err("olc", retval, "while receiving notice");
	  return((char *) NULL);
	}
      
      if ((strcmp(notice.z_class, class) != 0) ||
	  (strcmp(notice.z_class_inst, instance) != 0))
	continue;
      
      msg =  strcpy(malloc(strlen((notice.z_opcode))+1), (notice.z_opcode));
      ZFreeNotice(&notice);
      return(msg);
    }
}


zephyr_subscribe(class, instance, recipient)
     char *class;
     char *instance;
     char *recipient;
{
  ZSubscription_t sub;
  Code_t retval;

  sub.class = class;
  sub.classinst = instance;
  sub.recipient = recipient;

  if ((retval = ZSubscribeTo(&sub, 1, 0)) != ZERR_NONE)
    {
      com_err("olc", retval, "while subscribing");
      return(ERROR);
    }

  return(SUCCESS);
}

#endif ZEPHYR
