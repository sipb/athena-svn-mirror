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
#include "var.h"
#include "cvt.h"
#include <AL/AL.h>

#define SOCK_SECURE 1
#define SOCK_INSECURE 2

typedef struct disp_state {
  pc_state *ps;
  cons_state *cs;
  ALut ut;
  ALsessionStruct sess;
  int socketSec;
  int consolePreference;
  varlist *vars;
} disp_state;

char *name;
int debug = 5;

typedef char *(*varHandler)(disp_state *, char *, varlist *, varlist *);

typedef struct {
  char *name;
  int flags;
  varHandler func;
} var_def;

#define NONE 0
#define READ_ONLY 1
#define SECURE_SET 2

char *setUser(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
}

char *setStd(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;

  var_getString(vin, name, &value);
  var_setString(ds->vars, name, value);
  var_setString(vout, name, "OK");
}

#define NANNYPORT "/tmp/nanny"

/* Want to be "Nanny," but SGI has a fine enhancement to "last"
   which would cause us to show up if we did this. I don't know
   quite what they were thinking. */
#define NANNYNAME "LOGIN"

#define DOLINK
static char *athconsole="/etc/athena/consdev";

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

char *do_login(char *uname, disp_state *ds)
{
  ds->ut.user = uname;
  ds->ut.type = ALutUSER_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);

  ds->socketSec = SOCK_INSECURE;
  return "logged in";
}

/*
 * Be sure to clear all connections that are not the LISTENER
 * when changing to SOCK_SECURE. secure(ds) probably.
 */
char *do_logout(disp_state *ds)
{
  ds->ut.user = NANNYNAME;
  ds->ut.type = ALutLOGIN_PROC;
  ALsetUtmpInfo(&ds->sess, ALutUSER | ALutTYPE, &ds->ut);
  ALputUtmp(&ds->sess);

  ds->socketSec = SOCK_SECURE;
  return "logged out";
}

char *setLogin(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;

  var_getString(vin, name, &value);

  if (!strcasecmp(value, "TRUE"))
    {
      if (ds->socketSec == SOCK_INSECURE)
	var_setString(vout, name, "A user is currently logged in.");
      else
	{
	  if (!var_getString(ds->vars, "USER", &value))
	    {
	      var_setString(vout, name, do_login(value, ds));
	      var_setString(ds->vars, name, "TRUE");
	    }
	  else
	    var_setString(vout, name, "USER not set");
	}
    }
  else
    if (!strcasecmp(value, "FALSE"))
      {
	if (ds->socketSec != SOCK_INSECURE)
	  var_setString(vout, name, "No user is logged in.");
	else
	  {
	    var_setString(vout, name, do_logout(ds));
	    var_setString(ds->vars, name, "FALSE");
	    var_setValue(ds->vars, "USER", NULL, 0);
	  }
      }
    else
      var_setString(vout, name, "BADVALUE");  
}

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

char *setConsPref(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;

  var_getString(vin, name, &value);

  if (!strcasecmp(value, "ON"))
    {
      ds->consolePreference = CONS_UP;
      var_setString(ds->vars, name, "ON");
      var_setString(vout, name, startConsole(ds));
    }
  else
    if (!strcasecmp(value, "OFF"))
      {
	ds->consolePreference = CONS_DOWN;
	var_setString(ds->vars, name, "OFF");
	var_setString(vout, name, stopConsole(ds));
      }
    else
      var_setString(vout, name, "BADVALUE");
}

char *setConsMode(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
}

char *setDebug(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;

  var_getString(vin, name, &value);
  debug = atoi(value);
  var_setString(ds->vars, name, value);
  var_setString(vout, name, "OK");
}

char *setNannyMode(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
}

/* List of magic variables - variables where changing the value
   causes something to happen immediately, or other special
   attributes. */
var_def vars[] = {
  { "USER",		SECURE_SET,	setStd },
  { "REMOVE_USER",	SECURE_SET,	setStd },
  { "TTY",		READ_ONLY,	setStd },
  { "LOGGED_IN",	NONE,		setLogin },
  { "NANNY_MODE",	SECURE_SET,	setNannyMode },
  { "XCONSOLE",		NONE,		setConsPref },
  { "CONSOLE_MODE",	NONE,		setConsMode },
  { "DEBUG",		NONE,		setDebug },

  /* These aren't actually special, but they are in standard use,
     so we list them for documentation. */
  { "ELMER_ARGS[",	NONE,		setStd },
  { "ENV[",		NONE,		setStd },
  { NULL,		NONE,		setStd }
};

char *handleVars(pc_message *input, disp_state *ds, buffer **buf)
{
  buffer inbuf;
  varlist *vin, *vout;
  char **vlist, **ptr, *value;
  var_def *vstep;

  var_init(&vin);
  var_init(&vout);

  inbuf.buf = input->data;
  inbuf.len = input->length;

  cvt_buf2vars(vin, &inbuf);
  var_listVars(vin, &vlist);

  if (!strcmp(*vlist, "dump"))
    cvt_vars2buf(buf, ds->vars);
  else
    {
      ptr = vlist;
      while (*ptr)
	{
	  var_getString(vin, *ptr, &value);
	  if (!strcmp(value, cvt_query))
	    {
	      if (var_getString(ds->vars, *ptr, &value))
		var_setString(vout, *ptr, "NONE");
	      else
		var_setString(vout, *ptr, value);
	    }
	  else
	    {
	      for (vstep = vars; vstep->name != NULL; vstep++)
		if (!strcmp(vstep->name, *ptr))
		  break;
	      
	      if ((vstep->flags & READ_ONLY) ||
		  ((vstep->flags & SECURE_SET) && !ds->socketSec))
		var_setString(vout, *ptr, "EPERM");
	      else
		vstep->func(ds, *ptr, vin, vout);
	    }

	  ptr++;
	}

      cvt_vars2buf(buf, vout);
    }

  var_freeList(vin, vlist);
  var_destroy(vin);
  var_destroy(vout);
  return "ha!";
}

int process(pc_message *input, disp_state *ds)
{
  char *reply = NULL;
  int ret = 0;
  pc_message output;
  buffer *buf = NULL;

  if (debug)
    syslog(LOG_INFO, "request: %s", input->data);

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

  if (reply == NULL)
    reply = handleVars(input, ds, &buf);

  if (reply == NULL)
    reply = "unknown command";

  if (input->source)
    {
      if (buf != NULL)
	{
	  output.data = buf->buf;
	  output.length = buf->len;
	}
      else
	{
	  output.data = reply;
	  output.length = strlen(reply);
	}

      output.source = input->source;
      pc_send(&output);

      if (debug)
	syslog(LOG_INFO, "reply: %s", reply);

    }

  if (buf != NULL)
    {
      free(buf->buf);
      free(buf);
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

void dumpMessage(pc_message *message)
{
  varlist *vout;
  buffer outbuf;
  char **vlist, **ptr, *val;

  var_init(&vout);

  outbuf.buf = message->data;
  outbuf.len = message->length;
  cvt_buf2vars(vout, &outbuf);

  var_listVars(vout, &vlist);

  for (ptr = vlist; *ptr != NULL; ptr++)
    {
      var_getString(vout, *ptr, &val);
      fprintf(stdout, "%s=%s\n", *ptr, val);
    }

  var_freeList(vout, vlist);
  var_destroy(vout);
}

int main(int argc, char **argv)
{
  struct sigaction sigact;
  sigset_t mask, omask;
  disp_state ds;
  pc_port *inport, *outport;
  pc_message *message;
  char *option;
  buffer *outbuf;
  char sillybuf[10];
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
    option = "start";

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
  var_init(&ds.vars);

  sprintf(sillybuf, "%d", debug);
  var_setString(ds.vars, "DEBUG", sillybuf);

  ds.consolePreference = CONS_DOWN;
  var_setString(ds.vars, "XCONSOLE", "OFF");

  ds.socketSec = SOCK_SECURE;

  var_setString(ds.vars, "NANNY_MODE", "RUN"); /* vs. sleep */
  var_setString(ds.vars, "CONSOLE_MODE", "OFF");
  var_setString(ds.vars, "LOGGED_IN", "FALSE");

  /* Start syslogging. */
  openlog(name, LOG_PID, LOG_USER);

  /* Initialize communications. */
  pc_init(&ds.ps);

  /* Initialize message to send to nanny. We set source to NULL
     in case we wind up sending the message to ourselves. */
  message = malloc(sizeof(pc_message));
  message->source = NULL;

  if (argc < 3)
    {
      message->data = option;
      message->length = strlen(option) + 1;
    }
  else
    {
      cvt_strings2buf(&outbuf, &argv[1]);
      message->data = outbuf->buf;
      message->length = outbuf->len;
    }

  /* If there's a nanny already running, give it the command. */
  
  if (!(code = pc_openport(&outport, NANNYPORT)))
    {
      message->source = outport;
      pc_send(message);
      if (argc > 2)
	{
	  free(outbuf->buf);
	  free(outbuf);
	}
      free(message);

      pc_addport(ds.ps, outport);
      if (!pc_wait(&message, ds.ps))
	{
	  dumpMessage(message);
	  pc_freemessage(message);
	}
      pc_removeport(ds.ps, outport);
      pc_close(outport);
      exit(0);
    }
  else
    {
      if (code != PCerrNoListener)
	{
	  if (code == PCerrNotAllowed)
	    fprintf(stderr,
		    "%s: Permission denied opening nanny socket\n", name);
	  else
	    ALERROR();
	  exit(1);
	}
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
  if (pc_makeport(&inport, NANNYPORT, PC_GRAB|PC_CHMOD, 0, 0, 0700))
    {
      fprintf(stderr, "couldn't make the port\n");
      syslog(LOG_ERR, "could not create socket, exiting");
      exit(1);
    }
  pc_addport(ds.ps, inport);

  /* Initialize console handling. */
  ds.cs = cons_init();
  cons_getpty(ds.cs);

  var_setString(ds.vars, "TTY", cons_name(ds.cs));

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
      pc_wait(&message, ds.ps);

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
