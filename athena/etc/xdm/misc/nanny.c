#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "pc.h"
#include "cons.h"

char *name;
int consolePreference = CONS_DOWN;

#define NANNYPORT "/tmp/nanny"

#define DOLINK
static char *athconsole="/etc/athena/consdev";

char *startConsole(pc_state *ps, cons_state *cs)
{
  pc_fd fd;

  cons_grabcons(cs);
  if (cons_start(cs))
    return "console startup failed";
  else
    {
      fd.fd = cons_fd(cs);
      fd.events = PC_READ;
      pc_addfd(ps, &fd);
      return "console started";
    }
}

char *stopConsole(pc_state *ps, cons_state *cs)
{
  pc_fd fd;

  if (cons_stop(cs))
    return "console stop failed";
  else
    {
      fd.fd = cons_fd(cs);
      fd.events = PC_READ;
      pc_removefd(ps, &fd);
      return "console stopped";
    }
}

int process(pc_message *input, pc_state *ps, cons_state *cs)
{
  char *reply;
  int ret = 0;
  pc_message output;

  fprintf(stderr, "%s\n", input->data);

  if (!strcmp(input->data, "-xup"))
    {
      consolePreference = CONS_UP;
      reply = startConsole(ps, cs);
    }
  else
    if (!strcmp(input->data, "-xdown"))
      {
	consolePreference = CONS_DOWN;
	reply = stopConsole(ps, cs);
      }
    else
      if (!strcmp(input->data, "-die"))
	{
	  ret = 1;
	  reply = "nanny shutting down";
	  stopConsole(ps, cs);
#ifdef DOLINK
	  unlink(athconsole);
#endif
	  cons_close(cs);
	}
      else
	reply = "unknown command";

  if (input->source)
    {
      output.source = input->source;
      output.data = reply;
      output.length = strlen(reply);
      pc_send(&output);
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

int main(int argc, char **argv)
{
  struct sigaction sigact;
  sigset_t mask, omask;
  pc_state *ps;
  pc_port *inport, *outport;
  pc_message *message;
  cons_state *cs;
  char *option;
  int ret = 0;

  fclose(stdout);
  fclose(stderr);

  name = argv[0];

  if (argc == 2)
    option = argv[1];
  else
    option = "ping";

  openlog(name, LOG_PID, LOG_USER);

  ps = pc_init();

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

      pc_addport(ps, outport);
      message = pc_wait(ps);
      if (message != NULL)
	{
	  fprintf(stderr, "%s\n", message->data);
	  pc_freemessage(message);
	}
      pc_removeport(ps, outport);
      pc_close(outport);
      exit(0);
    }

  /* There's not a nanny running, so we take over. */

  /* If we're trying to kill the running nanny, we know it doesn't
     exist, so just exit. */
  if (!strcmp(option, "-die"))
    exit(0);

  fprintf(stderr, "no nanny contacted; running\n");

  /* Initialize our port for new connections. */
  inport = pc_makeport(NANNYPORT, PC_GRAB|PC_CHMOD, 0, 0, 0700);
  if (inport == NULL)
    {
      fprintf(stderr, "couldn't make the port\n");
      exit(1);
    }
  pc_addport(ps, inport);

  /* Initialize console handling. */
  cs = cons_init();
  cons_getpty(cs);

#ifdef DOLINK
  unlink(athconsole);
  symlink(cons_name(cs), athconsole);
#endif

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
      syslog(LOG_INFO, "fork failed with error %d", errno);
      fprintf(stderr, "%s: initial fork failed\n", name);
      exit(1);
    case 0:
      break;
    default:
      exit(0);
    }

  /*
   * Turns out, our caller (xdm) is waiting for a close on stdout,
   * besides caring for our pid to exit. We could direct our
   * stdout to stderr here too, but I don't feel like it.
   * This only matters if xdm is potentially starting us for the
   * first time; this could go if we start from init.d.
   */
  fclose(stdout);

  /*
   * Process the command line option as though we have received it
   * from the port before we start processing from the port. This must
   * happen after the fork so that anything created is our child and
   * not our sibling.
   */
  process(message, ps, cs);
  free(message);

  while (!ret)
    {
      message = pc_wait(ps);

      switch(message->type)
	{
	case PC_NEWCONN:
	  pc_addport(ps, message->data);
	  break;
	case PC_DATA:
	  ret = process(message, ps, cs);
	  pc_removeport(ps, message->source);
	  pc_close(message->source);
	  break;
	case PC_SIGNAL:
	  if (numc) /* then there are numc children signals */
	    {
	      sigprocmask(SIG_BLOCK, &mask, &omask);
	      while (numc)
		{
		  cons_child(cs, cstack[numc-1].pid, &cstack[numc-1].status);
		  numc--;
		}
	      sigprocmask(SIG_SETMASK, &omask, NULL);

	      if (cons_status(cs) == CONS_DOWN &&
		  consolePreference == CONS_UP)
		startConsole(ps, cs);
	    }
	  break;
	case PC_FD:
	  if (message->fd == cons_fd(cs))
	    cons_io(cs);
	  break;
	default:
	  fprintf(stderr, "type=%d\n", message->type);
	  break;
	}

      pc_freemessage(message);
    }

  pc_removeport(ps, inport);
  pc_close(inport);
  exit(0);
}
