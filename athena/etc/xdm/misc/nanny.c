#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <grp.h>
#include <errno.h>
#ifdef SYSV /* for utime */
#include <utime.h>
#else
#include <sys/time.h>
#endif
#include "pc.h"
#include "cons.h"
#include "var.h"
#include "cvt.h"
#include "dpy.h"
#include "nanny.h"
#include <AL/AL.h>

#define COM_SECURE 1
#define COM_INSECURE 2

typedef struct disp_state {
  pc_state *ps;
  pc_port *listener;
  cons_state *cs;
  ALut Xut, Cut;
  ALsessionStruct Xsess, Csess;
  int cUtWritten;		/* do we have an entry in utmp for console? */
  int comSec, comSecNew;	/* is our com port considered secure? */
  int consolePreference;	/* should the X console be running? */
  int shuttingDown;
  varlist *vars;
  dpy_state *dpy;
} disp_state;

#define DPYNAME ":0.0"
char *dpyenv = "DISPLAY=" DPYNAME;
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
  void *value;
  int length;

  var_getValue(vin, name, &value, &length);
  if (!ALsetUser(&ds->Xsess, value))
    {
      pc_chprot(ds->listener, ALpw_uid(&ds->Xsess), ALpw_gid(&ds->Xsess), 0600);
      var_setValue(ds->vars, name, value, length);
      var_setString(vout, name, N_OK);

      ds->comSecNew = COM_INSECURE;
    }
  else
    var_setString(vout, name, "unknown user");
}

char *setStd(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  void *value;
  int length;

  var_getValue(vin, name, &value, &length);
  var_setValue(ds->vars, name, value, length);
  var_setString(vout, name, N_OK);
}

/* Want to be "Nanny," but SGI has a fine enhancement to "last"
   which would cause us to show up if we did this. I don't know
   quite what they were thinking. */
#define NANNYNAME "LOGIN"

void init_consUtmp(disp_state *ds)
{
  if (dpy_consDevice(ds->dpy))
    {
      ds->Cut.user = NANNYNAME;
      ds->Cut.host = "";
      ds->Cut.line = dpy_consDevice(ds->dpy) + 5;
      ds->Cut.type = ALutLOGIN_PROC;
      ALsetUtmpInfo(&ds->Csess, ALutUSER | ALutHOST | ALutLINE | ALutTYPE,
		    &ds->Cut);
      ALputUtmp(&ds->Csess);

      ds->cUtWritten = 1;
    }
}

void clear_consUtmp(disp_state *ds)
{
  if (dpy_consDevice(ds->dpy) && ds->cUtWritten)
    {
      ds->Cut.user = NANNYNAME;
      ds->Cut.type = ALutDEAD_PROC;
      ALsetUtmpInfo(&ds->Csess, ALutUSER | ALutTYPE, &ds->Cut);
      ALputUtmp(&ds->Csess);
      ds->cUtWritten = 0;
    }
}

void init_utmp(disp_state *ds)
{
  ds->Xut.user = NANNYNAME;
  ds->Xut.host = DPYNAME;
  ds->Xut.line = cons_name(ds->cs) + 5;
  ds->Xut.type = ALutLOGIN_PROC;
  ALsetUtmpInfo(&ds->Xsess, ALutUSER | ALutHOST | ALutLINE | ALutTYPE,
		&ds->Xut);
  ALputUtmp(&ds->Xsess);
}

void clear_utmp(disp_state *ds)
{
  ds->Xut.user = NANNYNAME;
  ds->Xut.type = ALutDEAD_PROC;
  ALsetUtmpInfo(&ds->Xsess, ALutUSER | ALutTYPE, &ds->Xut);
  ALputUtmp(&ds->Xsess);

/* Hmmmm... This isn't quite right. What if the user is still logged
   in? The normal model for doing things (which Irix is using in the
   case of /bin/login) doesn't really allow for the-thing-that-cleans-
   utmp to exit. Probably just trying to be too flexible, and don't
   care about losing in this case. */

  clear_consUtmp(ds);
}

/* Updating the tty information might be nice to have in AL. */
char *do_login(char *uname, disp_state *ds)
{
  char *tty = cons_name(ds->cs);
  struct group *gr;
#ifdef SYSV
  struct utimbuf times;
#else
  struct timeval times[2];
#endif

  /* Write user login to utmp. */
  ds->Xut.user = uname;
  ds->Xut.type = ALutUSER_PROC;
  ALsetUtmpInfo(&ds->Xsess, ALutUSER | ALutTYPE, &ds->Xut);
  ALputUtmp(&ds->Xsess);

  /* Update owner and times on the tty. */
  gr = getgrnam("tty");
  if (chown(tty, ALpw_uid(&ds->Xsess), gr ? gr->gr_gid : ALpw_gid(&ds->Xsess))
      && debug > 3)
    syslog(LOG_INFO, "chown of %s (%d %d) failed (%m)",
	   tty, ALpw_uid(&ds->Xsess), gr ? gr->gr_gid : ALpw_gid(&ds->Xsess));

#ifdef SYSV
  times.actime = times.modtime = time(NULL);
  utime(tty, &times);
#else
  gettimeofday(&times[0], NULL);
  times[1].tv_sec = times[0].tv_sec;
  times[1].tv_usec = times[0].tv_usec;
  utimes(tty, times);
#endif

  return "logged in";
}

char *do_logout(disp_state *ds)
{
  char *value;

  /* utmp was changed by setting LOGGED_IN=TRUE, so we check to see
     if it's true before removing. */
  var_getString(ds->vars, N_LOGGED_IN, &value);
  if (!strcmp(value, N_TRUE))
    {
      ds->Xut.user = NANNYNAME;
      ds->Xut.type = ALutLOGIN_PROC;
      ALsetUtmpInfo(&ds->Xsess, ALutUSER | ALutTYPE, &ds->Xut);
      ALputUtmp(&ds->Xsess);
    }

  /* socket changed by setting USER=something */
  pc_chprot(ds->listener, 0, 0, 0600);

  /* Remove the user from the password file if xlogin added them. */
  if (!var_getString(ds->vars, N_RMUSER, &value) && !strcmp(value, "1"))
    {
      /* AL isn't quite suitable for this yet. */
      ALflagSet(&ds->Xsess, ALdidGetHesiodPasswd);
      ALremovePasswdEntry(&ds->Xsess);
    }

  ds->comSecNew = COM_SECURE;
  return "logged out";
}

char *setLogin(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value, *state;

  var_getString(ds->vars, N_USER, &state);
  var_getString(vin, name, &value);

  if (!strcasecmp(value, N_TRUE))
    {
      if (!strcmp(state, N_TRUE))
	var_setString(vout, name, "Already logged in.");
      else
	{
	  if (!var_getString(ds->vars, N_USER, &value))
	    {
	      var_setString(vout, name, do_login(value, ds));
	      var_setString(ds->vars, name, N_TRUE);
	    }
	  else
	    var_setString(vout, name, "USER not set");
	}
    }
  else
    if (!strcasecmp(value, N_FALSE))
      {
	var_setString(vout, name, do_logout(ds));
	var_setString(ds->vars, name, N_FALSE);
	var_setValue(ds->vars, N_USER, NULL, 0);
      }
    else
      var_setString(vout, name, N_BADVALUE);  
}

char *startConsole(disp_state *ds)
{
  pc_fd fd;

  cons_grabcons(ds->cs);
  if (cons_start(ds->cs))
    {
      syslog(LOG_ERR, "failed to start console");
      return "console startup failed";
    }
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

  if (!strcasecmp(value, N_ON))
    {
      ds->consolePreference = CONS_UP;
      var_setString(ds->vars, name, N_ON);
      var_setString(vout, name, startConsole(ds));
    }
  else
    if (!strcasecmp(value, N_OFF))
      {
	ds->consolePreference = CONS_DOWN;
	var_setString(ds->vars, name, N_OFF);
	var_setString(vout, name, stopConsole(ds));
      }
    else
      var_setString(vout, name, N_BADVALUE);
}

char *setDebug(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;

  var_getString(vin, name, &value);
  debug = atoi(value);
  var_setString(ds->vars, name, value);
  var_setString(vout, name, N_OK);
}

/*
 * Do as much shutdown work as we can here. Some stuff we can't do
 * because it's still needed to send the reply; setting the shuttingDown
 * flag will cause higher code to clean up the rest before exiting.
 */
void cleanShutdown(disp_state *ds)
{
  clear_utmp(ds);
  stopConsole(ds);
  cons_close(ds->cs);
  ds->shuttingDown = 1;
}

/*
 * If dpy's state has gone to none, bring it up to date with Nanny's.
 * Right now, this means set Nanny's mode to X and fire it up, since
 * the console mode isn't persistent.
 *
 * This usually implies that the X console should be turned on as soon
 * as X comes up. Under Irix, and we will be informed by xdm when X is
 * running and the console will be started by other code. On other
 * platforms, this routine may be called when we receive a signal from
 * the X server, and we start the console here.
 */
void update_dpy(disp_state *ds)
{
  char *value;

  clear_consUtmp(ds);
  var_getString(ds->vars, N_MODE, &value);
  if (!strcmp(value, N_NONE))
    return;

  if (dpy_startX(ds->dpy))
    {
      syslog(LOG_ERR, "couldn't start X, trying login");
      init_consUtmp(ds);
      if (dpy_startCons(ds->dpy))
	{
	  clear_consUtmp(ds);
	  syslog(LOG_ERR, "couldn't start login either");
	  var_setString(ds->vars, N_MODE, N_NONE);
	  return;
	}
      var_setString(ds->vars, N_MODE, N_CONSOLE);
      return;
    }
  var_setString(ds->vars, N_MODE, N_X);
}

char *forceNannyMode(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  void *value;
  int length;

  var_getValue(vin, name, &value, &length);
  var_setValue(ds->vars, N_MODE, value, length);
  var_setString(vout, name, N_OK);
}

char *setNannyMode(disp_state *ds, char *name, varlist *vin, varlist *vout)
{
  char *value;
  char *current;

  var_getString(ds->vars, N_LOGGED_IN, &value);
  if (!strcmp(value, N_TRUE))
    {
      var_setString(vout, name, "can't change mode while logged in");
      return;
    }

  var_getString(vin, name, &value);
  if (!strcmp(value, N_DEAD))
    {
      if (ds->comSec == COM_SECURE)
	{
	  cleanShutdown(ds);
	  var_setString(ds->vars, name, N_DEAD);
	  var_setString(vout, name, "nanny shutting down");
	}
      else
	var_setString(vout, name, "EPERM");
      return;
    }

  var_getString(ds->vars, name, &current);
  if (!strcmp(value, current))
    var_setString(vout, name, "set already");
  else
    {
      if (strcmp(value, N_CONSOLE) && strcmp(value, N_X) &&
	  strcmp(value, N_NONE))
	{
	  var_setString(vout, name, N_BADVALUE);
	  return;
	}

      /* Any change in mode means we want to shut down whatever's running. */
      if (!strcmp(current, N_X)) /* XXX should shut down X console first */
	if (dpy_stopX(ds->dpy))
	  {
	    var_setString(vout, name, "X shutdown failed");
	    return;
	  }

      if (!strcmp(current, N_CONSOLE))
	if (dpy_stopCons(ds->dpy))
	  {
	    var_setString(vout, name, "console shutdown failed");
	    return;
	  }
	else
	  clear_consUtmp(ds);

      var_setString(ds->vars, name, N_NONE);

      if (!strcmp(value, N_X))
	if (dpy_startX(ds->dpy))
	  {
	    var_setString(vout, name, "X startup failed");
	    return;
	  }

      if (!strcmp(value, N_CONSOLE))
	{
	  init_consUtmp(ds);
	  if (dpy_startCons(ds->dpy))
	    {
	      clear_consUtmp(ds);
	      var_setString(vout, name, "console startup failed");
	      return;
	    }
	}

      var_setString(ds->vars, name, value);
      var_setString(vout, name, N_OK);
    }
}

/* List of magic variables - variables where changing the value
   causes something to happen immediately, or other special
   attributes. */
var_def vars[] = {
  { N_USER,		SECURE_SET,	setUser },
  { N_RMUSER,		SECURE_SET,	setStd },
  { N_TTY,		READ_ONLY,	setStd },
  { N_LOGGED_IN,	NONE,		setLogin },
  { N_MODE,		SECURE_SET,	setNannyMode },
  { N_FMODE,		SECURE_SET,	forceNannyMode },
  { N_CONSPREF,		NONE,		setConsPref },
  { N_DEBUG,		NONE,		setDebug },
  { N_RETRY,		READ_ONLY,	setStd },
  /* These aren't actually special, but they are in standard use,
     so we list them for documentation. */
  { N_XSESSARGS,	NONE,		setStd },
  { N_ENV,		NONE,		setStd },

  /* Default handler. */
  { NULL,		NONE,		setStd }
};

char *handleVars(pc_message *input, disp_state *ds, buffer **buf)
{
  buffer inbuf;
  varlist *vin, *vout;
  char **vlist, **ptr, *value;
  var_def *vstep;
  int length;

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
	      if (var_getValue(ds->vars, *ptr, (void **)&value, &length))
		var_setString(vout, *ptr, "NONE");
	      else
		var_setValue(vout, *ptr, value, length);
	    }
	  else
	    {
	      for (vstep = vars; vstep->name != NULL; vstep++)
		if (!strcmp(vstep->name, *ptr))
		  break;
	      
	      if ((vstep->flags & READ_ONLY) ||
		  ((vstep->flags & SECURE_SET) &&
		      (ds->comSec == COM_INSECURE)))
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

  if (debug > 4)
    syslog(LOG_INFO, "request: %s %s", input->data,
	   strlen(input->data) + 1 < input->length ? "..." : "");

  reply = handleVars(input, ds, &buf);

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
	  output.length = strlen(reply); /* ? + 1 */
	}

      output.source = input->source;
      pc_send(&output);

      if (debug > 4)
	syslog(LOG_INFO, "reply: %s %s", output.data,
	       strlen(output.data) + 1 < output.length ? "..." : "");

    }

  if (buf != NULL)
    {
      free(buf->buf);
      free(buf);
    }
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

#define ALERROR() { com_err(argv[0], code, ALcontext(&ds.Xsess)); }

void dumpMessage(pc_message *message)
{
  varlist *vout;
  buffer outbuf;
  char **vlist, **ptr, *val, *step;
  int len;

  var_init(&vout);

  outbuf.buf = message->data;
  outbuf.len = message->length;
  cvt_buf2vars(vout, &outbuf);

  var_listVars(vout, &vlist);

  for (ptr = vlist; *ptr != NULL; ptr++)
    {
      var_getValue(vout, *ptr, (void *)&val, &len);
      if (!strchr(*ptr, '['))
	fprintf(stdout, "%s=%s\n", *ptr, val);
      else
	{
	  fprintf(stdout, "%s%d]=\n", *ptr, len);
	  for (step = val; step < val + len; step += strlen(step) + 1)
	    fprintf(stdout, "   %s\n", step);
	}
    }

  var_freeList(vout, vlist);
  var_destroy(vout);
}

char *xuptrans[] = {
  N_LOGGED_IN "=" N_FALSE,
  N_CONSPREF "=" N_ON,
  NULL
};

char *xdowntrans[] = {
  N_CONSPREF "=" N_OFF,
  N_LOGGED_IN "=" N_FALSE,
  NULL
};

char *dietrans[] = {
  N_CONSPREF "=" N_OFF,
  N_LOGGED_IN "=" N_FALSE,
  N_MODE "=" N_DEAD,
  NULL
};

char *starttrans[] = {
  N_MODE,
  NULL
};

char *pingtrans[] = {
  N_MODE,
  NULL
};

int main(int argc, char **argv)
{
  struct sigaction sigact;
  sigset_t mask, omask;
  disp_state ds;
  pc_port *inport, *outport;
  pc_message *message, retry;
  char **option;
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

  option = &argv[1];

  /* Convention: when xdm calls nanny, we can't send anything out
     to stderr or stdout, because that's the communication pipeline
     between xdm and xlogin. So we have xdm call nanny with '-' in
     front of the first option. (Also, when calling us, xdm blocks
     waiting for both our pid to exit and for stdout to be closed.) */
  if (*option && **option == '-')
    {
      fclose(stdout);
      fclose(stderr);
    }

  /* Translate command-line option requests into their real operations. */
  if (*option)
    {
      if (!strcmp(*option, "xup") || !strcmp(*option, "-xup"))
	option = xuptrans;
      if (!strcmp(*option, "xdown") || !strcmp(*option, "-xdown"))
	option = xdowntrans;
      if (!strcmp(*option, "die") || !strcmp(*option, "-die"))
	option = dietrans;
      if (!strcmp(*option, "ping") || !strcmp(*option, "-ping"))
	option = pingtrans;
    }
  else
    option = starttrans;

  /* Initialize assorted variables. */
  putenv(dpyenv);

  var_init(&ds.vars);

  sprintf(sillybuf, "%d", debug);
  var_setString(ds.vars, N_DEBUG, sillybuf);

  ds.consolePreference = CONS_DOWN;
  var_setString(ds.vars, N_CONSPREF, N_OFF);

  ds.comSec = COM_SECURE;
  ds.comSecNew = COM_SECURE;
  ds.shuttingDown = 0;

  var_setString(ds.vars, N_MODE, N_NONE);
  var_setString(ds.vars, N_LOGGED_IN, N_FALSE);

  retry.data = N_RETRY "=" N_TRUE;
  retry.length = strlen(retry.data) + 1;

  /* Start syslogging. */
  openlog(name, LOG_PID, LOG_USER);

  /* Initialize communications. */
  pc_init(&ds.ps);

  /* Initialize message to send to nanny. We set source to NULL
     in case we wind up sending the message to ourselves. */
  message = malloc(sizeof(pc_message));
  message->source = NULL;

  /* This direct conversion allows ENV[xxx] to suck in later
     args and do the right magic, because we're not parsing
     the actual variables here. */
  cvt_strings2buf(&outbuf, option);
  message->data = outbuf->buf;
  message->length = outbuf->len;

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
	    ALERROR(); /* Errrr??? */
	  exit(1);
	}
    }

  /* There's not a nanny running, so we take over. */

  /* If we're trying to kill the running nanny, we know it doesn't
     exist, so just exit. */
  if (option == dietrans || (option == pingtrans))
    {
      fprintf(stdout, N_MODE "=" N_DEAD "\n");
      exit(0);
    }

  fprintf(stdout, "no nanny contacted; running\n");
  syslog(LOG_INFO, "nanny starting");

  /* Initialize needed Athena Login library code. */
  code = ALinit();
  if (code) ALERROR();

  code = ALinitSession(&ds.Xsess);
  if (code) ALERROR();

  code = ALinitUtmp(&ds.Xsess);
  if (code) ALERROR();

  code = ALinitSession(&ds.Csess);
  if (code) ALERROR();

  code = ALinitUtmp(&ds.Csess);
  if (code) ALERROR();

  /* Initialize our port for new connections. */
  if (pc_makeport(&inport, NANNYPORT, PC_GRAB|PC_CHMOD, 0, 0, 0600))
    {
      fprintf(stderr, "couldn't make the port\n");
      syslog(LOG_ERR, "could not create socket, exiting");
      exit(1);
    }
  ds.listener = inport;
  pc_addport(ds.ps, inport);

  /* Initialize X console handling. */
  ds.cs = cons_init();
  cons_getpty(ds.cs);

  var_setString(ds.vars, N_TTY, cons_name(ds.cs));

  /* Initialize display handling. */
  ds.dpy = dpy_init();
  ds.cUtWritten = 0;

  /* Grab our X entry in utmp */
  init_utmp(&ds);

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

  while (!ds.shuttingDown)
    {
      pc_wait(&message, ds.ps);

      switch(message->type)
	{
	case PC_NEWCONN:
	  pc_addport(ds.ps, message->data);
	  break;
	case PC_DATA:
	  process(message, &ds);
	  pc_removeport(ds.ps, message->source);
	  pc_close(message->source);

	  /* This code is not in process() because otherwise the
	     reply might get RETRY appended to it. */
	  if (ds.comSec == COM_INSECURE && ds.comSecNew == COM_SECURE)
	    pc_secure(ds.ps, ds.listener, &retry);
	  ds.comSec = ds.comSecNew;

	  break;
	case PC_SIGNAL:
	  if (numc) /* then there are numc children signals */
	    {
	      sigprocmask(SIG_BLOCK, &mask, &omask);
	      while (numc)
		{
		  if (cons_child(ds.cs, cstack[numc-1].pid,
				 &cstack[numc-1].status))
		    dpy_child(ds.dpy, cstack[numc-1].pid,
			      &cstack[numc-1].status);
		  numc--;
		}
	      sigprocmask(SIG_SETMASK, &omask, NULL);

	      if (cons_status(ds.cs) == CONS_DOWN &&
		  ds.consolePreference == CONS_UP)
		startConsole(&ds);
	      if (dpy_status(ds.dpy) == DPY_NONE)
		update_dpy(&ds);
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
  pc_destroy(ds.ps);
  pc_close(inport);
  var_destroy(ds.vars);
  exit(0);
}
