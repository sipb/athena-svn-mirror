#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "pc.h"
#include "cons.h"
#include <AL/AL.h>

#define SOCK_SECURE 1
#define SOCK_INSECURE 2

typedef struct {
  pc_state *ps;
  cons_state *cs;
  ALut ut;
  ALsessionStruct sess;
  int socketSec;
  int consolePreference;
} disp_state;

char *name;
int debug = 5;

#define NANNYPORT "/tmp/nanny"

/* Want to be "Nanny," but SGI has a fine enhancement to "last"
   which would cause us to show up if we did this. I don't know
   quite what they were thinking. */
#define NANNYNAME "LOGIN"

#define DOLINK
static char *athconsole="/etc/athena/consdev";

char *startConsole(disp_state *ds)
{
  pc_fd fd;

  cons_grabcons(ds->cs);
  if (cons_start(ds->cs))
    return "console startup failed";
  else
    {
      fd.fd = cons_fd(ds->cs);
      fd.events = PC_READ;
      pc_addfd(ds->ps, &fd);
      return "console started";
    }
}

char *stopConsole(disp_state *ds)
{
  pc_fd fd;

  if (cons_stop(ds->cs))
    return "console stop failed";
  else
    {
      fd.fd = cons_fd(ds->cs);
      fd.events = PC_READ;
      pc_removefd(ds->ps, &fd);
      return "console stopped";
    }
}

void init_utmp(disp_state *ds, char *tty)
{
  if (tty == NULL)
    tty = "/dev/ttyq??";

  ds->ut.user = NANNYNAME;
  ds->ut.host = ":0.0";
  ds->ut.line = tty + 5;
  ds->ut.type = ALutLOGIN_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutHOST | ALutLINE | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);
}

void clear_utmp(disp_state *ds)
{
  ds->ut.user = NANNYNAME;
  ds->ut.type = ALutDEAD_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);  
}

char *do_login(pc_message *input, disp_state *ds)
{
  char *uname;

  if (ds->socketSec == SOCK_INSECURE)
    return "A user is currently logged in.";

  uname = strchr(input->data, ' ');
  if (uname == NULL)
    return "No username specified.";

  uname++;

  ds->ut.user = uname;
  ds->ut.type = ALutUSER_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);

  ds->socketSec = SOCK_INSECURE;
  return "logged in";
}

char *do_logout(disp_state *ds)
{
  ds->ut.user = NANNYNAME;
  ds->ut.type = ALutLOGIN_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);

  ds->socketSec = SOCK_SECURE;
  return "logged out";
}

int process(pc_message *input, disp_state *ds)
{
  char *reply = NULL;
  int ret = 0;
  pc_message output;

  if (debug)
    syslog(LOG_INFO, "request: %s", input->data);

  if (!strcmp(input->data, "xup"))
    {
      ds->consolePreference = CONS_UP;
      reply = startConsole(ds);
    }

  if (!strcmp(input->data, "xdown"))
    {
      ds->consolePreference = CONS_DOWN;
      reply = stopConsole(ds);
    }

  if (!strcmp(input->data, "die"))
    {
      if (ds->socketSec == SOCK_SECURE)
	{
	  ret = 1;
	  reply = "nanny shutting down";
	  clear_utmp(ds);
	  stopConsole(ds);
#ifdef DOLINK
	  unlink(athconsole);
#endif
	  cons_close(ds->cs);
	}
      else
	reply = "can't shut down with user logged in";
    }

  if (!strncmp(input->data, "login", 5))
    {
      reply = do_login(input, ds);
    }

  if (!strncmp(input->data, "logout", 5))
    {
      reply = do_logout(ds);
    }

  if (reply == NULL)
    reply = "unknown command";

  if (input->source)
    {
      output.source = input->source;
      output.data = reply;
      output.length = strlen(reply);
      pc_send(&output);

      if (debug)
	syslog(LOG_INFO, "reply: %s", reply);
    }

  return ret;
}

typedef struct _cinfo {
  pid_t pid;
  int status;
} cinfo;

int numc = 0;
cinfo cstack[10];

void children(int sig, int code, struct sigcontext *sc)
{
  pid_t ret;
  int status;

  while (ret = waitpid(-1, &status, WNOHANG))
    {
      if (ret == -1)
	return;

      if (numc < sizeof(cstack))
	{
	  cstack[numc].pid = ret;
	  cstack[numc++].status = status;
	}
    }
}

#define ALERROR() { com_err(argv[0], code, ALcontext(&ds.sess)); }

int main(int argc, char **argv)
{
  struct sigaction sigact;
  sigset_t mask, omask;
  disp_state ds;
  pc_port *inport, *outport;
  pc_message *message;
  char *option;
  int ret = 0;
  int silent = 0;
  long code;

  if (argv[0])
    {
      name = strrchr(argv[0], '/');
      if (name == NULL)
	name = argv[0];
      else
	name++;
    }
  else
    name = "Nanny";

  if (argc == 2)
    option = argv[1];
  else
    option = "ping";

  /* Convention: when xdm calls nanny, we can't send anything out
     to stderr or stdout, because that's the communication pipeline
     between xdm and xlogin. So we have xdm call nanny with '-' in
     front of the first option. (Also, when calling us, xdm blocks
     waiting for both our pid to exit and for stdout to be closed.) */
  if (option[0] == '-')
    {
      fclose(stdout);
      fclose(stderr);
      option++;
    }

  /* Initialize assorted variables. */
  ds.consolePreference = CONS_DOWN;
  ds.socketSec = SOCK_SECURE;

  /* Start syslogging. */
  openlog(name, LOG_PID, LOG_USER);

  /* Initialize communications. */
  ds.ps = pc_init();

  /* Initialize message to send to nanny. We set source to NULL
     in case we wind up sending the message to ourselves. */
  message = malloc(sizeof(pc_message));
  message->data = option;
  message->length = strlen(option) + 1;
  message->source = NULL;

  /* If there's a nanny already running, give it the command. */
  outport = pc_openport(NANNYPORT);
  if (outport)
    {
      message->source = outport;
      pc_send(message);
      free(message);

      pc_addport(ds.ps, outport);
      message = pc_wait(ds.ps);
      if (message != NULL)
	{
	  fprintf(stdout, "%s\n", message->data);
	  pc_freemessage(message);
	}
      pc_removeport(ds.ps, outport);
      pc_close(outport);
      exit(0);
    }

  /* There's not a nanny running, so we take over. */

  /* If we're trying to kill the running nanny, we know it doesn't
     exist, so just exit. */
  if (!strcmp(option, "die") || !strcmp(option, "ping"))
    {
      fprintf(stdout, "no nanny running\n");
      exit(0);
    }

  fprintf(stdout, "no nanny contacted; running\n");
  syslog(LOG_INFO, "nanny starting");

  /* Initialize needed Athena Login library code. */
  code = ALinit();
  if (code) ALERROR();

  code = ALinitSession(&ds.sess);
  if (code) ALERROR();

  code = ALinitUtmp(&ds.sess);
  if (code) ALERROR();

  /* Initialize our port for new connections. */
  inport = pc_makeport(NANNYPORT, PC_GRAB|PC_CHMOD, 0, 0, 0700);
  if (inport == NULL)
    {
      fprintf(stderr, "couldn't make the port\n");
      syslog(LOG_ERR, "could not create socket, exiting");
      exit(1);
    }
  pc_addport(ds.ps, inport);

  /* Initialize console handling. */
  ds.cs = cons_init();
  cons_getpty(ds.cs);

#ifdef DOLINK
  unlink(athconsole);
  symlink(cons_name(ds.cs), athconsole);
#endif

  init_utmp(&ds, cons_name(ds.cs));

  /* Signal handlers. */
  sigact.sa_flags = SA_NOCLDSTOP;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = children;
  sigaction(SIGCHLD, &sigact, NULL);

  /* Init once here for when we block chld later. */

  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);

  /*
   * fork() here to let the caller continue on. We actually need to
   * let the child process the command and exit only when it is finished.
   * Come to think of it, we should probably just fork and then ipc
   * the request to the child as though it was already running. But this
   * isn't so important to get done right now.
   */
  switch(fork())
    {
    case -1:
      syslog(LOG_ERR, "fork failed (%m), exiting");
      fprintf(stderr, "%s: initial fork failed\n", name);
      exit(1);
    case 0:
      break;
    default:
      exit(0);
    }

  /*
   * Process the command line option as though we have received it
   * from the port before we start processing from the port. This must
   * happen after the fork so that anything created is our child and
   * not our sibling.
   */
  process(message, &ds);
  free(message);

  while (!ret)
    {
      message = pc_wait(ds.ps);

      switch(message->type)
	{
	case PC_NEWCONN:
	  pc_addport(ds.ps, message->data);
	  break;
	case PC_DATA:
	  ret = process(message, &ds);
	  pc_removeport(ds.ps, message->source);
	  pc_close(message->source);
	  break;
	case PC_SIGNAL:
	  if (numc) /* then there are numc children signals */
	    {
	      sigprocmask(SIG_BLOCK, &mask, &omask);
	      while (numc)
		{
		  cons_child(ds.cs, cstack[numc-1].pid,
			     &cstack[numc-1].status);
		  numc--;
		}
	      sigprocmask(SIG_SETMASK, &omask, NULL);

	      if (cons_status(ds.cs) == CONS_DOWN &&
		  ds.consolePreference == CONS_UP)
		startConsole(&ds);
	    }
	  break;
	case PC_FD:
	  if (message->fd == cons_fd(ds.cs))
	    cons_io(ds.cs);
	  break;
	default:
	  fprintf(stderr, "type=%d\n", message->type);
	  break;
	}

      pc_freemessage(message);
    }

  pc_removeport(ds.ps, inport);
  pc_close(inport);
  exit(0);
}
