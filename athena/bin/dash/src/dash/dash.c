/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/dash.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/dash.c,v 1.3 1993-07-02 03:02:55 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <fcntl.h>
#include <X11/Xj/Jets.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/Xj/Window.h>
#include <X11/Xj/Button.h>
#include <X11/Xj/Label.h>
#include <X11/Xj/Menu.h>
#include <X11/Xj/DClock.h>
#include <X11/Xj/AClock.h>
#include <X11/Xj/Form.h>
#include <X11/Xj/Tree.h>
/* #include <X11/Xj/StripChart.h> */
/* #include <X11/Xj/List.h> */
#include <X11/Xj/warn.h>
#include "dash.h"


#if  defined(UltrixArchitcture) \
  || defined(AIXArchitecture) \
  || defined(MacIIArchitecture) \
  || defined(SunArchitecture)
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;
#endif


extern int DEBUG;

#define LOADST "/usr/athena/lib/gnuemacs/etc/loadst"

#define DASH "Dash"
#define DASH_NOP (char)0x00
#define DASH_DEBUG (char)0x01
#define DASH_KILL (char)0x02
#define DASH_CREATE (char)0x03
#define DASH_DESTROY (char)0x04
#define DASH_MAP (char)0x05
#define DASH_UNMAP (char)0x06
#define DASH_CREATEORMAP (char)0x07
#define DASH_RESTART (char)0x08

Atom dashAtom, nameAtom;
#define DASH_ATOM "_ATHENA_DASH"



Jet root;
/*
 * These definitions are needed by the tree jet
 */
JetClass *jetClasses[] =
{ &treeJetClass, &windowJetClass, &buttonJetClass, &labelJetClass,
    &menuJetClass, &dClockJetClass,
    &aClockJetClass, &formJetClass,
    /* &stripChartJetClass, */
    /* &listJetClass, */};

int numJetClasses = XjNumber(jetClasses);

static XrmOptionDescRec opTable[] = {
{"+rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "off"},
{"-background", "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bd",         "*borderColor", XrmoptionSepArg,        (caddr_t) NULL},
{"-bg",         "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bordercolor","*borderColor", XrmoptionSepArg,        (caddr_t) NULL},
{"-borderwidth",".borderWidth", XrmoptionSepArg,        (caddr_t) NULL},
{"-bw",         ".borderWidth", XrmoptionSepArg,        (caddr_t) NULL},
{"-display",    ".display",     XrmoptionSepArg,        (caddr_t) NULL},
{"-fg",         "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-fn",         "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-font",       "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-foreground", "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-geometry",   "*menuTree.window.geometry",    XrmoptionSepArg,
   (caddr_t) NULL},
{"-reverse",    "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"+rude",	"*rude",	XrmoptionNoArg,		(caddr_t) "off"},
{"-rude",	"*rude",	XrmoptionNoArg,		(caddr_t) "on"},
{"+verify",	"*verify",	XrmoptionNoArg,		(caddr_t) "off"},
{"-verify",	"*verify",	XrmoptionNoArg,		(caddr_t) "on"},
{"-xrm",        NULL,           XrmoptionResArg,        (caddr_t) NULL},
{"-name",       ".name",        XrmoptionSepArg,        (caddr_t) NULL},
{"-appdefs",	".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-f",		".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-userdefs",	".userDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-menus",	"*Menu.file",	XrmoptionSepArg,	(caddr_t) NULL},
{"-send",	".send",	XrmoptionNoArg,		(caddr_t) "true"},
{"-kill",	".kill",	XrmoptionNoArg,		(caddr_t) "true"},
{"-run",	".run",		XrmoptionNoArg,		(caddr_t) "true"},
{"-restart",	".restart",	XrmoptionNoArg,		(caddr_t) "true"},
{"-debug",	".debug",	XrmoptionNoArg,		(caddr_t) "true"},
{"-nofork",	".nofork",	XrmoptionNoArg,		(caddr_t) "true"},
#ifdef KERBEROS
{"-nochecktickets",  ".checkTickets",  XrmoptionNoArg,	(caddr_t) "false"},
#endif /* KERBEROS */
};

MyResources parms;

#define offset(field) XjOffset(MyResourcesPtr,field)

static XjResource appResources[] =
{
  { "verify", "Verify", XjRBoolean, sizeof(Boolean),
      offset(verifyOn), XjRBoolean, (caddr_t) True },
  { "notify", "Notify", XjRBoolean, sizeof(Boolean),
      offset(notifyOn), XjRBoolean, (caddr_t) True },
  { "startString", "StartString", XjRString, sizeof(char *),
      offset(startString), XjRString, (caddr_t) NULL },
  { "run", "Run", XjRBoolean, sizeof(Boolean),
      offset(run), XjRBoolean, (caddr_t) False },
  { "send", "Send", XjRBoolean, sizeof(Boolean),
      offset(send), XjRBoolean, (caddr_t) False },
  { "kill", "Kill", XjRBoolean, sizeof(Boolean),
      offset(kill), XjRBoolean, (caddr_t) False },
  { "restart", "Restart", XjRBoolean, sizeof(Boolean),
      offset(restart), XjRBoolean, (caddr_t) False },
  { "debug", "Debug", XjRBoolean, sizeof(Boolean),
      offset(debug), XjRBoolean, (caddr_t) False },
  { "nofork", "Nofork", XjRBoolean, sizeof(Boolean),
      offset(nofork), XjRBoolean, (caddr_t) False },
#ifdef KERBEROS
  { "checkTickets", "CheckTickets", XjRBoolean, sizeof(Boolean),
      offset(checkTickets), XjRBoolean, (caddr_t) True },
#endif /* KERBEROS */
};

#undef offset


#if (HasPutenv)
/*
 *  setenv() doesn't exist on some systems...  it's putenv instead.
 *  So, we write our own setenv routine, and use it instead.
 *
 *  Arguments:  (same as setenv in BSD Unix)
 *      char *name;     name of environment variable to set or change.
 *      char *value;    value to set or change environment variable to.
 *      int overwrite;  if non-zero, force variable to value.
 *                      if zero, and variable does not exist, add variable
 *                              to environment.
 *                      else, do nothing.
 *
 *  Returns:   (same as setenv in bsd Unix)
 *      -1              if unsuccessful (unable to malloc enough space for
 *                              an expanded environment).
 *      0               otherwise.
 *
 */
static int setenv(name, value, overwrite)
     char *name, *value;
     int overwrite;
{
  char *string;

  if ((overwrite) ||
      ( (char *) getenv(name) == NULL))
    {
      string = (char *) malloc((strlen(name) + strlen(value) + 2)
                               * sizeof(char));
					/* add 1 for the null and 1 for "=" */
      if (string == NULL)
        return(-1);
      strcpy(string, name);
      strcat(string, "=");
      strcat(string, value);
      if (! putenv(string) )
        return(-1);
    }
  return(0);
}
#endif


/*
 * Set utilities
 */

typedef struct _Set
{
  struct _Set *next;
  char *name;
} Set;

Set *setCache[100];
int cacheSize = 0;

/*
 * Single level expansion of name
 */
Set *expandName(name)
     char *name;
{
  char *type;
  XrmValue value;
  Set *tmp, *list;
  char *start, *end, *more;
  char n[50], c[50];
  int i;

  if (!strcasecmp("NULL", name))
    return NULL;

  for (i = 0; i < cacheSize; i++)
    if (!strcmp(name, setCache[i]->name))
      return setCache[i]->next;

  sprintf(n, "%s.set.%s", programName, name);
  sprintf(c, "%s.Set.%s", programClass, name);

  if (XrmGetResource(rdb, n, c, &type, &value))
    start = (char *)(value.addr);
  else
    {
      char errtext[100];

      sprintf(errtext, "no set %s exists", name);
      XjWarning(errtext);
      return NULL;
    }

  list = (Set *)XjMalloc((unsigned) sizeof(Set));
  list->name = XjNewString(name);
  list->next = NULL;

  while (isspace(*start)) start++;
  while (*start != '\0')
    {
      end = start;
      while (!isspace(*end) && *end != '\0') end++;
      more = end;
      if (*end != '\0') more++;
      *end = '\0';

      tmp = (Set *)XjMalloc((unsigned) sizeof(Set));
      tmp->name = start;
      tmp->next = list->next;
      list->next = tmp;
	  
      start = more;
    }

  setCache[cacheSize++] = list;
  return list->next;
}

Set *resolveName(name, list)
     char *name;
     Set *list;
{
  Set *e, *tmp;

  e = expandName(name);
  while (e != NULL)
    {
      if (*(e->name) == '_')
	{
	  tmp = (Set *)XjMalloc((unsigned) sizeof(Set));
	  tmp->name = e->name + 1;
	  tmp->next = list;
	  list = tmp;
	}
      else
	list = resolveName(e->name, list);
      e = e->next;
    }

  return list;
}

void freeSet(set)
     Set *set;
{
  Set *back;

  while (set)
    {
      back = set;
      set = set->next;
      XjFree((char *) back);
    }
}

/*
 * Tree callbacks...
 */

int printTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Jet j;

  for (j = root->core.child; j != NULL; j = j->core.sibling)
    fprintf(stdout, "%s\n", j->core.name);
  return 0;
}

int mapTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Jet w;

  w = XjFindJet(what, root);

  if (w != NULL)
    {
      w = w->core.child;
      while (w)
	{
	  MapWindow(w, True);
	  w = w->core.sibling;
	}
      return 0;
    }
  else
    {
      char errtext[100];

      sprintf(errtext, "couldn't find %s to map it", what);
      XjWarning(errtext);
      return 1;
    }
}

int mapSet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set, *ptr;

  set = resolveName(what, NULL);
  if (set)
    {
      for (ptr = set; ptr != NULL; ptr = ptr->next)
	(void)mapTree(fromJet, ptr->name, data);
      freeSet(set);
    }
  return 0;
}

int unmapTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Jet w;

  w = XjFindJet(what, root);

  if (w != NULL)
    {
      w = w->core.child;
      while (w)
	{
	  UnmapWindow(w);
	  w = w->core.sibling;
	}
      return 0;
    }
  else
    {
      char errtext[100];

      sprintf(errtext, "couldn't find %s to unmap it", what);
      XjWarning(errtext);
      return 1;
    }
}

int unmapSet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set, *ptr;

  set = resolveName(what, NULL);
  if (set)
    {
      for (ptr = set; ptr != NULL; ptr = ptr->next)
	(void)unmapTree(fromJet, ptr->name, data);
      freeSet(set);
    }
  return 0;
}

int createTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  if (NULL == XjFindJet(what, root))
    XjRealizeJet(XjVaCreateJet(what, treeJetClass, root, NULL, NULL));
  return 0;
}

int createSet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set, *ptr;

  set = resolveName(what, NULL);
  if (set)
    {
      for (ptr = set; ptr != NULL; ptr = ptr->next)
	(void)createTree(fromJet, ptr->name, data);
      freeSet(set);
    }
  return 0;
}

int createMapTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  if (NULL == XjFindJet(what, root))
    XjRealizeJet(XjVaCreateJet(what, treeJetClass, root, NULL, NULL));
  else
    mapTree(fromJet, what, data);

  return 0;
}

int createMapSet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set, *ptr;

  set = resolveName(what, NULL);
  if (set)
    {
      for (ptr = set; ptr != NULL; ptr = ptr->next)
	(void)createMapTree(fromJet, ptr->name, data);
      freeSet(set);
    }
  return 0;
}

int destroyTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Jet w;

  w = XjFindJet(what, root);
  if (w != NULL)
    {
      XjDestroyJet(w);
      return 0;
    }
  else
    {
      char errtext[100];

      sprintf(errtext, "couldn't find %s to destroy it", what);
      XjWarning(errtext);
      return 1;
    }
}

int destroySet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set, *ptr;

  set = resolveName(what, NULL);
  if (set)
    {
      for (ptr = set; ptr != NULL; ptr = ptr->next)
	(void)destroyTree(fromJet, ptr->name, data);
      freeSet(set);
    }
  return 0;
}

int warpTree(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Jet w;

  w = XjFindJet(what, root);

  if (w != NULL)
    {
      w = w->core.child;
      if (w != NULL)
	XWarpPointer(w->core.display,
		     None,
		     w->core.window,
		     0, 0, 0, 0,
		     w->core.width / 2,
		     w->core.height / 2);
      return 0;
    }
  else
    {
      char errtext[100];

      sprintf(errtext, "couldn't find %s to warp there", what);
      XjWarning(errtext);
      return 1;
    }
}

int warpSet(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  Set *set;

  set = resolveName(what, NULL);
  if (set)
    {
      (void)warpTree(fromJet, set->name, data);
      freeSet(set);
    }
  return 0;
}

/*
 * Exec related callbacks
 */
int sh(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  if (127 == system(what))
    {
      XjWarning("couldn't execute shell");
      return 1;
    }
  return 0;
}

void expand(what, where)
     char *what, *where;
{
  char *home, *start;

  while (isspace(*what)) what++;
  start = what;

  while (*what != '\0')
    {
      while (*what != '\0' && *what != '%' && *what != '~')
	*where++ = *what++;

      if (*what == '~' && what == start)
	{
	  home = (char *) getenv("HOME");
	  strncpy(where, home, strlen(home));
	  where += strlen(home);
	  what++;
	}
      else
	if (*what == '%')
	  {
	    what++;
	    switch(*what)
	      {
	      case '%':
		*where++ = '%';
		break;
	      case 'M':
		strncpy(where, MACHTYPE, strlen(MACHTYPE));
		where += strlen(MACHTYPE);
		what++;
		break;
	      default:
		*where++ = '%';
		what++;
	      }
	  }
    }
  *where = '\0';
}

static char *cwd = NULL; /* working directory for exec'ed programs */
static char *subject = NULL;
static char buf[MAXPATHLEN];
static char addpath[MAXPATHLEN];

int setup(fromJet, what, data)
caddr_t fromJet;
char *what;
caddr_t data;
{
  subject = what;
  return 0;
}

int cd(fromJet, what, data)
caddr_t fromJet;
char *what;
caddr_t data;
{
  cwd = what;
  return 0;
}

void setupEnvironment()
{
  char *path, *space;

  if (displayName != NULL)
    setenv("DISPLAY", displayName, True);

  if (cwd != NULL)
    {
      expand(cwd, buf);
      if (-1 == chdir(buf))
	{
	  char errtext[100];

	  sprintf(errtext, "cd to %s failed with error %d",
		  cwd, errno);
	  XjWarning(errtext);
	}
    }

  if (subject != NULL)
    setenv("SUBJECT", subject, True);

  if (*addpath != '\0')
    {
      path = (char *) getenv("PATH");
      if (path == NULL)
	{
	  space = XjMalloc((unsigned) MAXPATHLEN);
	  space[0] = '\0';
	}
      else
	{
	  space = XjMalloc((unsigned) (MAXPATHLEN + strlen(path)));
	  strcpy(space, path);
	  strcat(space, ":");
	}

      strcat(addpath, "/%Mbin");
      expand(addpath, &space[strlen(space)]);
      setenv("PATH", space, True);
    }
}


int add(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  int err;
  FILE *a;
  int l;
  int omask;
  char line1[100], line2[100];

  line2[0] = '\0';
  sprintf(addpath, "attach -p %s", what);
  omask = sigblock(sigmask(SIGCHLD));		/* THIS IS GROSS... */
						/* block sigchlds for now... */
  if (NULL != (a = popen(addpath, "r")))
    {
      fgets(addpath, MAXPATHLEN, a);
      err = pclose(a);
      (void) sigsetmask(omask);			/* unblock sigchlds again... */

      if (err != 0)
	sprintf(line2, "Attach failed with error %d", err/256);
      else
	{
	  l = strlen(addpath);
	  if (l > 0)
	    if (addpath[l - 1] == '\n')
	      addpath[l - 1] = '\0';
	  return 0;
	}
    }
  (void) sigsetmask(omask);			/* unblock sigchlds again... */
  sprintf(line1, "Could not attach %s", what);
  XjUserWarning(root, NULL, True, line1, line2);

  return 1;
}

int attach(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  int err;

  err = add(fromJet, what, data);
  addpath[0] = '\0';

  return err;
}

typedef struct _Child
{
  struct _Child *next;
  int pid;
  char *title;
} Child;

static Child *firstChild = NULL;


#ifndef WSTOPPED
#define WSTOPPED 0177
#endif
#ifndef WIFEXITED
#define WIFEXITED(x)	(((union wait *)&(x))->w_stopval != WSTOPPED && \
			 ((union wait *)&(x))->w_termsig == 0)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(x)	(((union wait *)&(x))->w_retcode)
#endif
#ifndef WTERMSIG
#define WTERMSIG(x)	(((union wait *)&(x))->w_termsig)
#endif
#ifndef W_CORE
#define W_CORE(x)	(((union wait *)&(x))->w_coredump)
#endif

/*
 * Avoid zombies
 */
#if defined(RsArchitecture)
void checkChildren(sig)
     int sig;
#else
int checkChildren()
#endif
{
  Boolean found;
  Child *ch, **last;
  int child;

#if defined(MacIIArchitecture)
  int status;

  while ((child = wait3(&status, WNOHANG, 0)) > 0)
#else
  union wait status;
  struct rusage rus;

  while ((child = wait3(&status, WNOHANG, &rus)) > 0)
#endif

    {
      found = False;
      last = &firstChild;
      for (ch = firstChild;
	   ch != NULL && !found;
	   last = &ch->next, ch = ch->next)
	if (ch->pid == child)
	  {
	    found = True;
	    *last = ch->next;

#ifdef POSIX
	    if (WIFEXITED(status))
#endif /* POSIX */
	      if (WEXITSTATUS(status))
		{
		  char line1[100];

		  sprintf(line1, "%s exited with status %d",
			  ch->title, WEXITSTATUS(status));
		  XjUserWarning(root, NULL, True, line1, "");
		}
#ifdef POSIX
	    if (WIFSIGNALED(status))
#endif /* POSIX */
	      if (WTERMSIG(status))
		{
		  char line1[100], line2[100];

		  sprintf(line1, "%s exited with signal %d",
			  ch->title, WTERMSIG(status));
		  if (W_CORE(status))
		    strcpy(line2, "(core dumped!)");
		  else
		    line2[0] = '\0';
		  XjUserWarning(root, NULL, True, line1, line2);
		}

	    XjFree((char *) ch);
	  }
    }
#if defined(_IBMR2)
  return;
#else
  return 0;
#endif
}

char *NAME; /* oh no! a global variable! */


input(fd, name)
     int fd;
     char *name;
{
  char buf[MAXPATHLEN];
  char buf2[MAXPATHLEN + 7]; /* add some in for the length of errno */
  int n;

  if (read(fd, buf2, MAXPATHLEN + 7))
    {
      char line1[100], line2[100];

      sscanf(buf2, "%s %d", buf, &n);
      sprintf(line1, "Could not start %s:", name);
      if (n == 0 || n > sys_nerr)
	sprintf(line2, "%s: Error %d", buf, n);
      else
	sprintf(line2, "%s: %s", buf, sys_errlist[n]);
      XjUserWarning(root, NULL, True, line1, line2);
    }
  XjReadCallback((XjCallbackProc)NULL, fd, &fd);
  close(fd);
}


int exec(info, what, data)
     MenuInfo *info;
     char *what;
     caddr_t data;
{
  int pid;
  char *ptr;
  int argc = 0;
  char *argv[100];
  Child *ch;
  char *name;
  int fd[2];
  int pipes;
  int num_fds;

  if (info != NULL &&		/* exec through menu item */
      info->null == NULL)
    name = info->menu->title;
  else
    if (NAME != NULL)		/* exec through verify, because verify */
      {				/* can't pass the menuinfo structure */
	name = NAME;		/* it got, since the menus may have */
	NAME = NULL;		/* rearranged by now. There _is_ a better */
      }				/* solution, but if I had time to do it, you */
  else				/* wouldn't be reading this! */
    name = what;		/* exec through non-menu item */

  if ((pipes = pipe(fd)))
    {
      char line1[100], line2[100];

      if (errno > sys_nerr)
	sprintf(line1, "Could not set up pipe:  error %d", errno);
      else
	sprintf(line1, "Could not set up pipe:  %s", sys_errlist[errno]);
      sprintf(line2, "There will be no warning if `%s' can't be started.",
	      name);
      XjUserWarning(root, NULL, True, line1, line2);
    }
  pipes = !pipes;

  pid = fork();			/* vfork will block indefinitely... :-( */

  if (pid == -1)
    {
      char line1[100], line2[100];

      sprintf(line1, "Could not start %s:", name);
      if (errno > sys_nerr)
	sprintf(line2, "Fork failed with error %d", errno);
      else
	sprintf(line2, "Fork - %s", sys_errlist[errno]);
      XjUserWarning(root, NULL, True, line1, line2);
      if (pipes)
	{
	  close(fd[0]);
	  close(fd[1]);
	}
      return 1;
    }

  if (pid != 0)			/* We're in the parent... */
    {
      addpath[0] = '\0';
      cwd = NULL;
      subject = NULL;

      ch = (Child *)XjMalloc((unsigned) sizeof(Child));
      ch->pid = pid;
      ch->title = name;
      ch->next = firstChild;
      firstChild = ch;

      if (pipes)
	{
	  close(fd[1]);		/* Parent reads on fd[0] */
	  XjReadCallback((XjCallbackProc)input, fd[0], name);
	}
      return 0;
    }

  setupEnvironment();

  expand(what, buf);
  ptr = buf;

  while (*ptr != '\0')
    {
      while (isspace(*ptr))
	ptr++;

      argv[argc++] = ptr;

      while (!isspace(*ptr) && *ptr != '\0')
	ptr++;

      if (*ptr != '\0')
	{
	  *ptr = '\0';
	  ptr++;
	}
    }

  argv[argc] = NULL;

  for (num_fds = 3; num_fds < getdtablesize(); num_fds++)
    fcntl(num_fds, F_SETFD, 1);

  if (-1 == execvp(argv[0], argv))
    {
      char buf2[MAXPATHLEN + 7]; /* add some in for the length of errno */
      int n;

      /* Child writes on the wall with crayon...  just kidding - fd[1] */
      if (pipes)
	{
	  sprintf(buf2, "%s %d", buf, errno);
	  n = strlen(buf2);
	  if (write(fd[1], buf2, n) == n)
	    _exit(0);
	}
      _exit(42);
    }
  return 42;			/* never reached, but makes saber happy... */
}

int restart(info, what, data)
     MenuInfo *info;
     char *what;
     caddr_t data;
{
  char *ptr;
  int argc = 0;
  char *argv[100];
  char line1[100], line2[100];

  setupEnvironment();

  if (what != NULL &&
      *what != '\0')
    {
      expand(what, buf);
      ptr = buf;

      while (*ptr != '\0')
	{
	  while (isspace(*ptr))
	    ptr++;

	  argv[argc++] = ptr;

	  while (!isspace(*ptr) && *ptr != '\0')
	    ptr++;

	  if (*ptr != '\0')
	    {
	      *ptr = '\0';
	      ptr++;
	    }
	}
      execvp(argv[0], argv);
    }
  else
    execvp(global_argv[0], global_argv);

  sprintf(line1, "Attempt to restart failed with error %d", errno);
  if (errno > sys_nerr)
    line2[0] = '\0';
  else
    sprintf(line2, "%s: %s", global_argv[0], sys_errlist[errno]);
  XjUserWarning(root, NULL, True, line1, line2);
  return 1;
}

int addMenus(info, file, data)
     MenuInfo *info;
     char *file;
     caddr_t data;
{
  if (info->null != NULL) /* we weren't called by menu code */
    return 1;

  return loadNewMenus(info->menubar, file);
}

/*
 * Danger Will Robinson!
 * This mechanism makes it possible for callbacks to
 * be called after the calling jet has already been
 * destroyed. Ain't that special?
 */
typedef struct _vstruct {
  Jet top;
  Jet yes;
  XjCallback me;
  Menu *menu;
  char *string;
  char *title;
} Verify;

int yesorno(who, v, data)
     Jet who;
     Verify *v;
     caddr_t data;
{
  XUnmapWindow(v->top->core.display, XjWindow(v->top));
  XFlush(v->top->core.display);

  if (v->yes == who)
    {
      NAME = v->title;
      XjCallCallbacks((caddr_t) who, v->menu->activateProc, NULL);
    }

  XjDestroyJet(v->top);
  XjFree(v->string);
  XjFree((char *) v);
  return 0;
}

int verify(info, foo, data)
     MenuInfo *info;
     int foo;
     caddr_t data;
{
  Verify *v;
  Jet lqtop, lqform, logo, lqlabel, lqybutton, lqnbutton,
  lqnwindow, lqywindow, lqylabel, lqnlabel;

  if (info->null != NULL) /* we weren't called by menu code */
    return 1;

  if (!parms.verifyOn)
    {
      XjCallCallbacks((caddr_t) info, info->menu->activateProc, NULL);
      return 0;
    }

  v = (Verify *)XjMalloc((unsigned) sizeof(Verify));

  v->me.next = NULL;
  v->me.argType = argInt;
  v->me.passInt = (int)v;
  v->me.proc = yesorno;
  v->menu = info->menu;

  lqtop =  XjVaCreateJet("lqWindow", windowJetClass, root, NULL, NULL);
  lqform = XjVaCreateJet("lqForm", formJetClass, lqtop, NULL, NULL);

  /* in this structure because that widget may need it forever, long
     after it's off the stack and we have returned */
  v->title = info->menu->title;
  v->string = XjMalloc((unsigned) (strlen(info->menu->title) + 1
				   + strlen(parms.startString)));
  sprintf(v->string, parms.startString, info->menu->title);

  logo = XjVaCreateJet("dashLogo", labelJetClass, lqform,
		       NULL, NULL);
  lqlabel = XjVaCreateJet("lqLabel", labelJetClass, lqform,
			  XjNlabel, v->string, NULL, NULL);
  lqywindow = XjVaCreateJet("lqYWindow", windowJetClass, lqform, NULL, NULL);
  lqnwindow = XjVaCreateJet("lqNWindow", windowJetClass, lqform, NULL, NULL);
  lqybutton = XjVaCreateJet("lqYButton", buttonJetClass, lqywindow, 
			    XjNactivateProc, &(v->me), NULL, NULL);
  lqnbutton = XjVaCreateJet("lqNButton", buttonJetClass, lqnwindow,
			    XjNactivateProc, &(v->me), NULL, NULL);
  lqylabel = XjVaCreateJet("lqYLabel", labelJetClass, lqybutton, NULL, NULL);
  lqnlabel = XjVaCreateJet("lqNLabel", labelJetClass, lqnbutton, NULL, NULL);

  v->top = lqtop;
  v->yes = lqybutton;

  XjRealizeJet(lqtop);
  return 0;
}


int toggleVerify(info, otherName, data)
MenuInfo *info;
char *otherName;
caddr_t data;
{
  static int initialized = 0;
  static char *onName, *offName;

  if (info->null != NULL) /* we weren't called by menu code */
    return 1;

  if (initialized == 0)
    {
      char *ptr = otherName;

      initialized = 1;

				/* Deal with star BS... */
      if (ptr[0] == ' '  &&  ptr[1] == ' '  &&  ptr[2] == ' ')
	ptr += 3;
      if (ptr[0] == '*'  &&  ptr[1] == ' ')
	ptr += 2;
				/* Rip this code out in a future */
				/* release... */

      if (parms.verifyOn)
	{
	  onName = info->menu->title;
	  offName = ptr;
	}
      else
	{
	  onName = ptr;
	  offName = info->menu->title;
	}
    }

  if (parms.verifyOn)
    info->menu->title = offName;
  else
    info->menu->title = onName;

  parms.verifyOn = !parms.verifyOn;

  computeMenuSize(info->menubar, info->menu);
  computeMenuSize(info->menubar, info->menu->parent); /* bug */
  return 0;
}

int toggleHelp(info, otherName, data)
MenuInfo *info;
char *otherName;
caddr_t data;
{
  static int initialized = 0;
  static char *onName, *offName;
  XjSize size;

  if (info->null != NULL) /* we weren't called by menu code */
    return 1;

  if (initialized == 0)
    {
      char *ptr = otherName;

      initialized = 1;

				/* Deal with star BS... */
      if (ptr[0] == ' '  &&  ptr[1] == ' '  &&  ptr[2] == ' ')
	ptr += 3;
      if (ptr[0] == '*'  &&  ptr[1] == ' ')
	ptr += 2;
				/* Rip this code out in a future */
				/* release... */

      if (info->menubar->menu.showHelp == 1)
	{
	  onName = info->menu->title;
	  offName = ptr;
	}
      else
	{
	  onName = ptr;
	  offName = info->menu->title;
	}
    }

  if (info->menubar->menu.showHelp == 1)
    info->menu->title = offName;
  else
    info->menu->title = onName;

  info->menubar->menu.showHelp = !info->menubar->menu.showHelp;

  computeAllMenuSizes(info->menubar, info->menubar->menu.rootMenu);
  computeRootMenuSize(info->menubar, &size);
  return 0;
}

int quit(fromJet, what, data)
caddr_t fromJet;
int what;
caddr_t data;
{
  XjExit(what);
  return 0;				/* For linting... */
}

int debug(fromJet, what, data)
caddr_t fromJet;
char *what;
caddr_t data;
{
  fprintf(stdout, "malloced %d\n", malloced);
  return 0;
}

int usage(fromJet, what, data)
caddr_t fromJet;
char *what;
caddr_t data;
{
  XjUsage(what);
  return 0;
}

int printMenu(info, what, data)
     MenuInfo *info;
     char *what;
     caddr_t data;
{
  if (info->null != NULL)		/* we weren't called by menu code */
    return 1;

  PrintMenu(info->menubar); /* call only from a menu item. :-) */
  return 0;
}

/* when the menu works without being overrideRedirect, this should
   be made ICCCM compliant. */
int lowerMenu(info, what, data)
     MenuInfo *info;
     char *what;
     caddr_t data;
{
  if (info->null != NULL)
    return 1;

  XLowerWindow(info->menubar->core.display,
	       info->menubar->core.window);
  return 0;
}

int logout(fromJet, what, data)
caddr_t fromJet;
char *what;
caddr_t data;
{
  char *pid_string;
  int pid_int;

  if (pid_string = (char *) getenv("XSESSION"))
    if (pid_int = atoi(pid_string))
      if (!kill(pid_int, SIGHUP))
	return 0;

  NAME = "logout";
  /*return exec(NULL, "/usr/athena/end_session", NULL);*/
  return exec(NULL, "end_session", NULL);
}

fatal(display)
     Display *display;
{
  XjExit(-1);
}

static int (*def_handler)();

static int handler(display, error)
     Display *display;
     XErrorEvent *error;
{
  if (error->error_code == BadWindow  ||  error->error_code == BadAtom)
    return 0;

  def_handler(display, error);
  return 0;			/* it'll never get this far anyway... */
}



Window findDASH(display)
     Display *display;
{
  char *atom_name;
  Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long bytes_after;
  Window *prop;
  unsigned char *name;
/*  unsigned char *prop; */
  int status;

#ifdef TIME_STARTUP
  struct timeval start, end;

  gettimeofday(&start, NULL);
  printf("findDASH: - %d.%d + \n", start.tv_sec, start.tv_usec);
#endif

  atom_name = (char *) XjMalloc(strlen(programName) + strlen(DASH_ATOM)
				+ 2);
  sprintf(atom_name, "%s_%s", DASH_ATOM, programName);
  nameAtom = XInternAtom(display, atom_name, False);
  XjFree(atom_name);

  status = XGetWindowProperty(display, RootWindow(display, 0),
			      nameAtom, 0, 1,
			      False, AnyPropertyType, &actual_type,
			      &actual_format, &nitems, &bytes_after,
			      (unsigned char **) &prop);
  if (status==BadWindow)
    XjFatalError("rootWindow does not exist!");
  if (status!=Success)
    return (Window) NULL;
  if (! prop)
    return (Window) NULL;

  def_handler = XSetErrorHandler(handler);
  status = XGetWindowProperty(display, *prop,
			      dashAtom, 0, 1,
			      False, AnyPropertyType, &actual_type,
			      &actual_format, &nitems, &bytes_after,
			      &name);
  (void) XSetErrorHandler(def_handler);

  if (status==BadWindow)
    {
      XjFatalError("rootWindow does not exist!");
    }
  if (status!=Success)
    {
      return (Window) NULL;
    }

#ifdef TIME_STARTUP
	  gettimeofday(&end, NULL);
	  printf("findDASH: %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
		 (end.tv_usec > start.tv_usec)
		 ? end.tv_sec - start.tv_sec
		 : end.tv_sec - start.tv_sec - 1,
		 (end.tv_usec > start.tv_usec)
		 ? end.tv_usec - start.tv_usec
		 : end.tv_usec + 1000000 - start.tv_usec );
#endif
  
  if (!strcmp(name, programName))
    {
      XjFree(name);
      return *prop;
    }

  XjFree(name);
  return (Window) NULL;
}



Status sendEvent(display, window, opcode, data)
     Display *display;
     Window window;
     char opcode;
     char *data;
{
  Status s;
  XEvent e;

  e.xclient.type = ClientMessage;
  e.xclient.window = window;
  e.xclient.message_type = dashAtom;
  e.xclient.format = 8;
  bzero(e.xclient.data.b, sizeof(e.xclient.data.b));
  e.xclient.data.b[0] = opcode;
  if (data)
    strcpy(&e.xclient.data.b[1], data);

  if (s = XSendEvent(display, window, False, NoEventMask, &e))
    XFlush(display);

  return s;
}

int delete(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  XjDestroyJet(fromJet);
  return 0;
}

int deleteParent(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  XjDestroyJet(XjParent(fromJet));
  return 0;
}

int message(info, zilch, data)
     WindowInfo *info;
     char *zilch;
     caddr_t data;
{
  char errtext[100];
  info->event->xclient.data.b[19] = '\0'; /* just in case */

  if (info->event->xclient.message_type == dashAtom)
    switch(info->event->xclient.data.b[0])
      {
      case DASH_NOP:
	break;
      case DASH_DEBUG:
	DEBUG = !DEBUG;
	printTree(NULL, NULL, NULL);
	break;
      case DASH_KILL:
	XjExit(0);
	break;
      case DASH_CREATE:
	createSet(NULL, &info->event->xclient.data.b[1], NULL);
	break;
      case DASH_DESTROY:
	destroySet(NULL, &info->event->xclient.data.b[1], NULL);
	break;
      case DASH_MAP:
	mapSet(NULL, &info->event->xclient.data.b[1], NULL);
	break;
      case DASH_UNMAP:
	unmapSet(NULL, &info->event->xclient.data.b[1], NULL);
	break;
      case DASH_CREATEORMAP:
	createMapSet(NULL, &info->event->xclient.data.b[1], NULL);
	break;
      case DASH_RESTART:
	restart(NULL, NULL, NULL);
	break;
      default:
	sprintf(errtext, "unknown ClientMessage opcode: %d\n",
		(int)info->event->xclient.data.b[0]);
	break;
      }

#if !defined(MacIIArchitecture)
  else
    {
      sprintf(errtext, "unrecognized ClientMessage: %d\n",
	      info->event->xclient.message_type);
      XjWarning(errtext);
      return 1;
    }
#endif

  return 0;
}

/*
struct loadInfo {
  FILE *f;
  StripChartJet who;
};

void getLoad(fd, info)
     int fd;
     struct loadInfo *info;
{
  char s[40];
  int l1, l2, l3;

  fscanf(info->f, "%s %d.%d[%d]\n", s, &l1, &l2, &l3);
  XjStripChartData(info->who, 100*l1+l2);
}

int load(init, foo, data)
     StripChartInit *init;
     int foo;
     caddr_t data;
{
  FILE *loadFile;
  char command[80];
  struct loadInfo *info;

  sprintf(command, "%s %d", LOADST, init->interval/1000);
  loadFile = popen(command, "r");
  info = (struct loadInfo *)XjMalloc(sizeof(struct loadInfo));
  info->who = init->j;
  info->f = loadFile;

  XjReadCallback((XjCallbackProc)getLoad, fileno(loadFile), info);
  return 0;
}

int cpu(where)
     int *where;
{
  double l;

  getcpu(NULL, NULL, &l);
  *where = (int)(l * 100.0);
  return 0;
}

int pstat(where)
     int *where;
{
  FILE *ps;
  static int Pused, Ptext, Pfree, Pwasted, Pmissing;
  static int Pnum[6], Psize[6];
  static struct timeval last = { 0, 0 };
  struct timeval now;

  gettimeofday(&now, NULL);
  if (now.tv_sec - last.tv_sec > 60)
    {
      last = now;

      ps = popen("/etc/pstat -s", "r");

      fscanf(ps, "%dk used (%dk text), %dk free, %dk wasted, %dk missing\n",
	     &Pused, &Ptext, &Pfree, &Pwasted, &Pmissing);
      fscanf(ps, "avail: %d*%dk %d*%dk %d*%dk %d*%dk %d*%dk %d*%dk\n",
	     &Pnum[0], &Psize[0], &Pnum[1], &Psize[1], &Pnum[2], &Psize[2],
	     &Pnum[3], &Psize[3], &Pnum[4], &Psize[4], &Pnum[5], &Psize[5]);

      pclose(ps);
    }

  *where = 100 * Pused / (Pused + Pfree + Pwasted + Pmissing);

  return 0;
}
*/


int std_out(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  printf(what);
  fflush(stdout);
  return 0;				/* For linting... */
}

int std_err(fromJet, what, data)
     Jet fromJet;
     char *what;
     caddr_t data;
{
  fprintf(stderr, what);
  fflush(stderr);
  return 0;				/* For linting... */
}


XjCallbackRec callbacks[] =
{
  /* tree operations */
  { "createTree", createTree },
  { "createMapTree", createMapTree },
  { "destroyTree", destroyTree },
  { "warpTree", warpTree },
  { "mapTree", mapTree },
  { "unmapTree", unmapTree },
  { "printTree", printTree },
  /* set operations */
  { "createSet", createSet },
  { "createMapSet", createMapSet },
  { "destroySet", destroySet },
  { "warpSet", warpSet },
  { "mapSet", mapSet },
  { "unmapSet", unmapSet },
  /* misc */
  { "quit", quit },
  { "exec", exec },
  { "sh", sh },
  { "toggleHelp", toggleHelp },
  { "toggleVerify", toggleVerify },
  { "debug", debug },
  { "usage", usage },
  { "logout", logout },
  { "delete", delete },
  { "deleteParent", deleteParent },
  { "printMenu", printMenu },
  { "lowerMenu", lowerMenu },
  { "cd", cd },
#ifdef ATTACH
  { "attach", attach },
  { "add", add },
  { "setup", setup },
#endif
  { "addMenus", addMenus },
  { "verify", verify },
  { "restart", restart },
  { "message", message },
  { "stdout", std_out },
  { "stderr", std_err },
/*
  { "load", load },
  { "cpu", cpu },
  { "pstat", pstat },
*/
};

main(argc, argv)
int argc;
char **argv;
{
  Display *display;
  char *home;
  char userFile[100];
  Window handle = (Window) NULL;
  int cd[50];
  Set *tmp, *list;
  char *nameOptions[50];
  int i, numOptions = 0;
  int count, sign = 1;
  Status e;
  Jet handlejet;

  home = (char *) getenv("HOME");
  if (home != NULL)
    {
      sprintf(userFile, "%s/.dashrc", home);
      home = userFile;
    }

  (void)XSetIOErrorHandler(fatal);

  root = XjCreateRoot(&argc, argv, DASH, home,
		      opTable, XjNumber(opTable));

  XjLoadFromResources(NULL,
		      NULL,
		      programClass,
		      programName,
		      appResources,
		      XjNumber(appResources),
		      (caddr_t) &parms);
  display = root->core.display;

  dashAtom = XInternAtom(display, DASH_ATOM, False);

  /*
   * Parse special creation options out of command line.
   */
  argv++;
  while (*argv)
    {
      if (!strcmp(*argv, "-show"))
	sign = 1;
      else
	{
	  if (!strcmp(*argv, "-hide"))
	    sign = 0;
	  else
	    {
	      if (**argv == '+' || **argv == '-')
		{
		  cd[numOptions] = (**argv) == '+' ? 0 : 1;
		  nameOptions[numOptions++] = *argv + 1;
		}
	      else
		{
		  cd[numOptions] = sign;
		  nameOptions[numOptions++] = *argv;
		}
	    }
	}
      argv++;
    }

  if (numOptions == 0)
    {
      nameOptions[0] = "default";
      cd[0] = 1;
      numOptions = 1;
    }

  if (!parms.run) /* don't get a handle if we don't care */
    handle = findDASH(display);

  if (!handle && (parms.send || parms.kill))
    {
      /* try harder... */
      count = 3;
      while (!handle && count)
	{
	  sleep(10);
	  handle = findDASH(display);
	  count--;
	}

      if (!handle)
	{
	  char errtext[100];

	  sprintf(errtext, "couldn't find a running %s", programName);
	  XjWarning(errtext);
	  XjExit(1);
	}
    }

  /*
   * -kill
   */
  if (handle && parms.kill)
    {
      if (sendEvent(display, handle, DASH_KILL, NULL))
	XjExit(0);
      else
	XjFatalError("sendEvent failed");
    }

  /*
   * -restart
   */
  if (handle && parms.restart)
    {
      if (sendEvent(display, handle, DASH_RESTART, NULL))
	XjExit(0);
      else
	XjFatalError("sendEvent failed");
    }

  /*
   * -debug
   */
  if (handle && parms.debug)
    {
      if (sendEvent(display, handle, DASH_DEBUG, NULL))
	XjExit(0);
      else
	XjFatalError("sendEvent failed");
    }

  /*
   * Either -send, or a dash exists and not -run
   */
  if (handle)
    {
      for (i = 0; i < numOptions; i++)
	{
	  if (cd[i] == 0)
	    e = sendEvent(display, handle, DASH_UNMAP, nameOptions[i]);
	  else
	    e = sendEvent(display, handle, DASH_CREATEORMAP, nameOptions[i]);
	  if (!e)
	    XjFatalError("sendEvent failed");
	}
      XjExit(0);
    }

  /*
   * Now we deal with the rest of the command line args...
   */
  if (!parms.nofork)
    {
      switch (fork())
	{
	case 0:			/* child */
	  break;
	case -1:		/* error */
	  perror ("Can't fork");
	  XjExit(-1);
	default:		/* parent */
	  XjExit(0);
	}
    }

  if (parms.startString == NULL)
    parms.startString = "Start %s?";

  DEBUG = parms.debug;

  XjRegisterCallbacks(callbacks, XjNumber(callbacks));

  for (i = 0; i < numOptions; i++)
    {
      tmp = resolveName(nameOptions[i], NULL);
      if (cd[i] == 1)
	for (list = tmp; list != NULL; list = list->next)
	  (void)XjVaCreateJet(list->name, treeJetClass, root, NULL, NULL);
      freeSet(tmp);
    }

  XjRealizeJet(root);
  /*
   * Okay, so there is no handle...  we'll create one and install the
   * property on the root.
   */
  handlejet = XjVaCreateJet("handleWindow", windowJetClass, root,
			    XjNoverrideRedirect, True,
			    XjNmapped, False,
			    XjNx, -100,
			    XjNy, -100,
			    XjNwidth, 1,
			    XjNheight, 1,
			    XjNtitle, DASH_ATOM,
			    NULL, NULL);
  XjRealizeJet(handlejet);
  handle = XjWindow(handlejet);
  XChangeProperty(display, handle, dashAtom,
		  XA_STRING, 8, PropModeReplace,
		  (unsigned char *) programName, strlen(programName));
  XChangeProperty(display, RootWindow(display, 0), nameAtom,
		  XA_WINDOW, 32, PropModeReplace,
		  (unsigned char *) &handle, 1);
  XFlush(display);



  signal(SIGCHLD, checkChildren);

#ifdef KERBEROS
  if (parms.checkTickets)
    checkTkts(0,0);
#endif /* KERBEROS */

  XjEventLoop(root);
}
