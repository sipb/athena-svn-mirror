/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Jets.c,v $
 * $Author: ghudson $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Jets.c,v 1.6 1996-09-19 22:21:40 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <X11/Xos.h>
#include <ctype.h>
#include <varargs.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#ifdef SOLARIS
#include <sys/filio.h>
#endif
#include "Jets.h"
#include "hash.h"

extern int StrToXFontStruct();
extern int StrToXColor();
extern int StrToPixmap();
extern int StrToDirection();
extern int StrToOrientation();
extern int StrToBoolean();
extern int StrToJustify();

int DEBUG = 0;

static XjCallbackProc checkSignals = NULL;
static XjEventProc eventHandler = NULL;

int global_argc;
char **global_argv;
XrmDatabase rdb = NULL;

char *displayName;
char *programName = NULL;
char *programClass;

int malloced = 0;
int lastmallocs = 0;
int lastbreak = 0;
int accounting = 1;

static int gotRegisterContextType = 0;
static XContext registerContext;
static int gotSelectContextType = 0;
static XContext selectContext;

#ifndef APPDEFDIR
#define APPDEFDIR "/usr/lib/X11/app-defaults"
#endif
static char *appdefdir = APPDEFDIR;

/*
static XrmOptionDescRec opTable[] = {
{"+rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "off"},
{"+synchronous","*synchronous", XrmoptionNoArg,         (caddr_t) "off"},
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
{"-geometry",   ".geometry",    XrmoptionSepArg,        (caddr_t) NULL},
{"-iconic",     ".iconic",      XrmoptionNoArg,         (caddr_t) "on"},
{"-name",       ".name",        XrmoptionSepArg,        (caddr_t) NULL},
{"-reverse",    "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-synchronous","*synchronous", XrmoptionNoArg,         (caddr_t) "on"},
{"-title",      ".title",       XrmoptionSepArg,        (caddr_t) NULL},
{"-xrm",        NULL,           XrmoptionResArg,        (caddr_t) NULL},
{"-appdefs",	".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-f",		".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-userdefs",	".userDefs",	XrmoptionSepArg,	(caddr_t) NULL}
};
*/

typedef struct _JetResources {
  char *appDefs;		/* name of application defaults file to use */
  char *userDefs;		/* name of user defaults file */
  char *display;		/* display name */
  char *name;			/* application name */
} JetResources;

typedef struct _JetResources *JetResourcesPtr;

JetResources startup;

#define offset(field) XjOffset(JetResourcesPtr,field)

static XjResource startupResources[] = {
  { "appDefs", "AppDefs", XjRString, sizeof(char *),
      offset(appDefs), XjRString, NULL },
  { "userDefs", "UserDefs", XjRString, sizeof(char *),
      offset(userDefs), XjRString, NULL },
  { "display", "Display", XjRString, sizeof(char *),
      offset(display), XjRString, NULL },
  { "name", "Name", XjRString, sizeof(char *),
      offset(name), XjRString, NULL }
};

#undef offset

RootClassRec rootClassRec = {
{
    /* class name */		"Root",
    /* jet size */		sizeof(RootRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */		NULL,
    /* realize */		NULL,
    /* event */			NULL,
    /* expose */		NULL,
    /* querySize */		NULL,
    /* move */			NULL,
    /* resize */		NULL,
    /* destroy */		NULL,
    /* resources */		NULL,
    /* number of 'em */		0
  }
};

JetClass rootJetClass = (JetClass)&rootClassRec;
int curID = 0;

typedef struct _alarmEntry {
  XjCallbackProc wakeup;
  caddr_t data;
  struct timeval when;
  struct _alarmEntry *next;
  int id;
} alarmEntry;

alarmEntry *alarmList = NULL;
alarmEntry *freeAlarms = NULL;

void XjLoadFromResources();

void XjExit(exitcode)
     int exitcode;
{
  /*  need to throw in some cleanup code here...  */
  exit(exitcode);
}

void XjFatalError(message)
     char *message;
{
  fprintf(stderr, "%s: %s\n", programName, message);
  XjExit(-1);
}

void XjWarning(string)
     char *string;
{
  fprintf(stderr, "%s: %s\n", programName, string);
}

void XjUsage(s)
     char *s;
{
/*
  struct rusage u;

  getrusage(RUSAGE_SELF, &u);

  fprintf(stdout, "usage: %d %d %d\n",
	  u.ru_ixrss, u.ru_idrss, u.ru_isrss);
*/

  fprintf(stdout, "usage (%s): %d\n", s, sbrk(0));
}

char *XjMalloc(size)
    unsigned size;
{
  char *ptr;

#ifdef MEM
  accounting = 0;
#endif

  if ((ptr = (char *)malloc(size)) == NULL)
    {
      char errtext[100];

      sprintf(errtext, "couldn't malloc %d bytes", size);
      XjFatalError(errtext);
    }

#ifdef MEM
  accounting = 1;
  malloced += size;

  brake = sbrk(0);
  if (brake != lastbreak)
    {
      fprintf(stdout, "(%d\t%d)\t(%d\t%d)\n",
	      malloced - lastmallocs, brake - lastbreak,
	      malloced, brake);
      lastmallocs = malloced;
      lastbreak = brake;
    }
#endif

  return(ptr);
}

char *XjRealloc(ptr, size)
     char *ptr;
     unsigned size;
{
  if ((ptr = (char *)realloc(ptr, size)) == NULL)
    XjFatalError("couldn't realloc");
  /*    fprintf(stderr, "adt: malloc %d\n", size); */

  return(ptr);
}

XjFree(ptr)
     char *ptr;
{
#ifdef MEM
  accounting = 0;
#endif
  if (ptr != NULL)
    free(ptr);

#ifdef MEM
  accounting = 1;
#endif
}

/*
static void printQueue()
{
  int count = 0;
  alarmEntry *ptr;

  for (ptr = alarmList; ptr != NULL; ptr = ptr->next)
    fprintf(stdout, "%d. %d %d\n", count++,
	    ptr->when.tv_sec, ptr->when.tv_usec);
}
*/

static void flushAlarmQueue()
{
  int first = 1;
  struct timeval now;
  alarmEntry *tmp;

  gettimeofday(&now, NULL);

  /*
   * Hello? Mr. Jones? You requested a wake-up call.
   */
  while ((first && alarmList != NULL) ||
	 (alarmList != NULL &&
	  (now.tv_sec > alarmList->when.tv_sec ||
	   (now.tv_sec == alarmList->when.tv_sec &&
	    now.tv_usec > alarmList->when.tv_usec))))
    {
      first = 0;

      /*
       * The wakeup call may add another time into the queue,
       * but that's ok because the time must be in the future
       * (interval is unsigned), and the queue is consistent.
       */
      alarmList->wakeup(alarmList->data, alarmList->id);

      tmp = alarmList;
      alarmList = alarmList->next;

      tmp->next = freeAlarms;
      freeAlarms = tmp;
    }

/*  fprintf(stdout, "%d %d\n%d %d\n",
	  alarmList->when.tv_sec, alarmList->when.tv_usec,
	  now.tv_sec, now.tv_usec); */
}

unsigned long XjRemoveWakeup(id)
     int id;
{
  struct timeval now;
  alarmEntry *ptr, **last;
  unsigned long timeleft;

  gettimeofday(&now, NULL);

  last = &alarmList;
  ptr = alarmList;

  while (ptr != NULL && ptr->id != id)
    {
      last = &(ptr->next);
      ptr = ptr->next;
    }

  if (ptr == NULL)
    return 0;

  /*
   * Work out how much longer we were
   * supposed to wait
   */
  ptr->when.tv_sec -= now.tv_sec;
  ptr->when.tv_usec -= now.tv_usec;

  if (ptr->when.tv_usec < 0)
    {
      ptr->when.tv_usec += 1000000;
      ptr->when.tv_sec -= 1;
    }

  timeleft = (ptr->when.tv_sec * 1000) + (ptr->when.tv_usec / 1000);

  *last = ptr->next;
  ptr->next = freeAlarms;
  freeAlarms = ptr;

  return timeleft;
}

int XjAddWakeup(who, data, interval)
     XjCallbackProc who;
     caddr_t data;
     unsigned long interval; /* milliseconds */
{
  int i;
  struct timeval now;
  alarmEntry *ptr, **lastPtr, *next;

  gettimeofday(&now, NULL);

  /*
   * Get freeAlarms pointing to something useful.
   */
  if (freeAlarms == NULL)
    {
      freeAlarms = (alarmEntry *)XjMalloc((unsigned) sizeof(alarmEntry) * 10);
      for (i = 0; i < 9; i++)
	freeAlarms[i].next = &freeAlarms[i + 1];
      freeAlarms[9].next = NULL;
    }

  /*
   * Fill in the structure; compute the wakeup time.
   */
  next = freeAlarms;
  freeAlarms = freeAlarms->next;

  next->wakeup = who;
  next->data = data;
  next->id = curID;
  next->when.tv_sec = interval / 1000;
  next->when.tv_usec = (interval % 1000) * 1000;

  next->when.tv_sec += now.tv_sec;
  next->when.tv_usec += now.tv_usec;
  if (next->when.tv_usec >= 1000000)
    {
      next->when.tv_usec -= 1000000;
      next->when.tv_sec += 1;
    }

  /*
   * Insert the entry into the list and take it out of the free list.
   */
  lastPtr = &alarmList;
  ptr = alarmList;

  while (ptr != NULL &&
	 (next->when.tv_sec > ptr->when.tv_sec ||
	  (next->when.tv_sec == ptr->when.tv_sec &&
	   next->when.tv_usec > ptr->when.tv_usec)))
    {
      lastPtr = &ptr->next;
      ptr = ptr->next;
    }

  next->next = ptr;
  *lastPtr = next;
  return curID++;
}

void XjCallCallbacks(info, callback, data)
     caddr_t info;
     XjCallback *callback;
     caddr_t data;
{
  int exit = 0;

  for (; callback != NULL && exit == 0; callback = callback->next)
    switch(callback->argType)
      {
      case argInt:
	exit = callback->proc(info, callback->passInt, data);
	break;
      case argString:
	exit = callback->proc(info, callback->passString, data);
	break;
      }
}

void XjQuerySize(jet, size)
     Jet jet;
     XjSize *size;
{
  if (jet->core.classRec->core_class.querySize != NULL)
    jet->core.classRec->core_class.querySize(jet, size);
  else
    {
      size->width = -1;
      size->height = -1;
    }
}

void XjMove(jet, x, y)
     Jet jet;
     int x, y;
{
  if (jet->core.classRec->core_class.move != NULL)
    if (jet->core.x != x || jet->core.y != y)
      jet->core.classRec->core_class.move(jet, x, y);
}

void XjResize(jet, size)
     Jet jet;
     XjSize *size;
{
  if (jet->core.classRec->core_class.resize != NULL)
    if (jet->core.width != size->width || jet->core.height != size->height)
      jet->core.classRec->core_class.resize(jet, size);
}

void XjExpose(jet, event)
     Jet jet;
     XEvent *event;
{
  if (jet->core.classRec->core_class.expose != NULL)
    jet->core.classRec->core_class.expose(jet, event);
}

XjCallback *XjConvertStringToCallback(address)
     char **address;
{
  char name[50];
  int type = argInt;
  char *ptr, *end, *strParam = NULL;
  int intParam = 0, barfed = 0;
  XjCallbackProc tmp;
  XjCallback *ret;
  char errtext[100];

  if (*address == NULL)
    return NULL;

  ptr = *address;

  while (isspace(*ptr)) ptr++;

  end = strchr(ptr, '(');
  if (end == NULL)
    {
      /* we don't advance the pointer in this case. */
      sprintf(errtext, "missing '(' in callback string: %s", ptr);
      XjWarning(errtext);
      return NULL;
    }

  strncpy(name, ptr, end - ptr);
  name[end - ptr] = '\0';

  tmp = XjGetCallback(name);
  
  if (tmp == NULL)
    {
      sprintf(errtext, "unregistered callback: %s", name);
      XjWarning(errtext);
      barfed = 1;
    }

  ptr = end + 1;
  while (isspace(*ptr))
    ptr++;

  if (isdigit(*ptr)  ||  *ptr == '-')
    {
      intParam = atoi(ptr);
      type = argInt;
    }
  else if (*ptr != ')')
    {
      char delim = *ptr;
      ptr++;
      end = strchr(ptr, delim);
      if (end == NULL)
	{
	  sprintf(errtext, "missing close quote in callback string: %s",
		  *address);
	  XjWarning(errtext);
	  barfed = 1;
	}
      else
	{
	  strParam = XjMalloc((unsigned) (end - ptr + 1));
	  strncpy(strParam, ptr, end - ptr);
	  strParam[end - ptr] = '\0';
	  ptr = end + 1;
	  type = argString;
	}
    }

  end = strchr(ptr, ')');
  if (end == NULL || barfed)
    {
      if (!barfed)
	{
	  sprintf(errtext, "missing close parenthesis in callback string: %s",
		  *address);
	  XjWarning(errtext);
	}

      /* bugs in here - if first callback unknown, rest oddly punted */

      if (end != NULL)
	ptr = end + 1;

      if (type == argString)
	XjFree(strParam);
      *address = ptr;
      return NULL;
    }

  ptr = end + 1;

  ret = (XjCallback *) XjMalloc((unsigned) sizeof(XjCallback));
  ret->argType = type;
  ret->proc = tmp;
  if (type == argInt)
    ret->passInt = intParam;
  else
    ret->passString = strParam;

  while (isspace(*ptr)) ptr++;
  if (*ptr == ',')
    {
      ptr++;
      ret->next = XjConvertStringToCallback(&ptr);
    }
  else
    ret->next = NULL;

  *address = ptr;
  return ret;
}


XjCallbackRec cvt_callbacks[] =
{
  { XjRFontStruct, StrToXFontStruct },
  { XjRColor, StrToXColor },
  { XjRJustify, StrToJustify },
  { XjROrientation, StrToOrientation },
  { XjRDirection, StrToDirection },
  { XjRBoolean, StrToBoolean },
  { XjRPixmap, StrToPixmap },
};


void XjFillInValue(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  /*
   * Nonconversion
   */
  if (!strcmp(type, resource->resource_type))
    {
      memcpy(where + resource->resource_offset,
	    &address,
	    (resource->resource_size > 4) ? 4 : resource->resource_size);
      return;
    }

  if (!strcmp(type, XjRString))
    {
      static int init = 1;
      XjCallbackProc tmp;

      if (init)
	{
	  XjRegisterCallbacks(cvt_callbacks, XjNumber(cvt_callbacks));
	  init = 0;
	}

      /*
       * string to integer conversion 
       */
      if (!strcmp(resource->resource_type, XjRInt))
	{
	  *((int *)((char *)where + resource->resource_offset))
	    = atoi(address);
	  return;
	}

      /*
       * string to string conversion :)
       */
      if (!strcmp(resource->resource_type, XjRString))
	{
/*      *(char **)((char *)where + resource->resource_offset) =
	(address == NULL) ? NULL : XjNewString(address); */
	  *(char **)((char *)where + resource->resource_offset) = address;
	  return;
	}

      /*
       * String to Callback conversion
       */
      if (!strcmp(resource->resource_type, XjRCallback))
	{
	  *((XjCallback **)((char *)where + resource->resource_offset)) =
	    XjConvertStringToCallback(&address);
	  return;
	}

      if ((tmp = XjGetCallback(resource->resource_type)) != NULL)
	{
	  tmp(display, window, where, resource, type, address);
	  return;
	}
    }

  {
    char errtext[100];
    sprintf(errtext, "unknown conversion: %s to %s",
	    type, resource->resource_type);
    XjWarning(errtext);
  }
}

void XjSelectInput(display, window, mask)
Display *display;
Window window;
long mask;
{
  long previous;

  if (!gotSelectContextType)
    {
      selectContext = XUniqueContext();
      gotSelectContextType = 1;
    }

  if (XCNOENT != XFindContext(display, window,
			      selectContext, (caddr_t *)&previous))
    {
      XDeleteContext(display, window, selectContext);
      mask |= previous;
    }

  if (XCNOMEM == XSaveContext(display, window, selectContext, (caddr_t)mask))
    {
      XjFatalError("out of memory in XSaveContext");
      XjExit(-1);
    }

  XSelectInput(display, window, mask);
}

void XjRegisterWindow(window, jet)
Window window;
Jet jet;
{
  if (!gotRegisterContextType)
    {
      registerContext = XUniqueContext();
      gotRegisterContextType = 1;
    }

#ifdef DEBUG
  if (XCNOENT != XFindContext(jet->core.display, window,
			      registerContext, (caddr_t *)&eventJet))
    fprintf(stdout, "%s usurped window from %s\n",
	    jet->core.name, eventJet->core.name);
#endif

  if (XCNOMEM == XSaveContext(jet->core.display, window,
			      registerContext, (caddr_t) jet))
    {
      XjFatalError("out of memory in XSaveContext");
      XjExit(-1);
    }
}

void XjUnregisterWindow(window, jet)
Window window;
Jet jet;
{
  (void)XDeleteContext(jet->core.display, window, registerContext);
  (void)XDeleteContext(jet->core.display, window, selectContext);
}

Jet XjCreateRoot(argc, argv, appClass, userFile, appTable, appTableCount)
int *argc;
char **argv;
char *appClass;
char *userFile;
XrmOptionDescList appTable;
int appTableCount;
{
  Display *display;
  Jet jet;
  char *display_resources;
  char *xenvironment;
  XrmDatabase ad_rdb = NULL, disp_rdb = NULL, user_rdb = NULL, cl_rdb = NULL;
  int i, screen;
  char errtext[100];

  /*
   * We need to keep our own copy of (argc, argv) around...
   * XrmParseCommand is going to munge the original.
   */
  global_argc = *argc;
  global_argv = (char **)XjMalloc((unsigned) (sizeof(char *) * (*argc + 1)));
  for (i = 0; i < *argc; i++)
    global_argv[i] = argv[i];

  /*
   * We have to parse "-name" ourselves. Sigh. This should be
   * mentioned in XrmParseCommand documentation. It isn't.
   */
  for (i = 1; i < *argc; i++)
    if (!strcmp("-name", argv[i]))
      {
	if (i + 1 < *argc)
	  programName = argv[i + 1];
	break;
      }

  if (programName == NULL)
    {
      programName = strrchr (argv[0], '/');
      if (programName)
	programName++;
      else
	programName = argv[0];
    }

  programClass = XjNewString(appClass);

  XrmInitialize();

  /*
   * Ok. There are four database sources here. In reverse priority:
   *
   *    1. application defaults (can be overriden by user)
   *    2. user's display resources
   *    3. user's home directory dotfile
   *    4. user's command line options
   */

  /*
   * Load command-line options into cl_rdb
   */
  cl_rdb = XrmGetStringDatabase("");
/*  XrmParseCommand(&cl_rdb, opTable, XjNumber(opTable), programName,
		  argc, argv); */
  XrmParseCommand(&cl_rdb, appTable, appTableCount, programName,
		  argc, argv);

  /*
   * Get the -display option out of it, if specified, and open the
   * display. Might as well do this first, since if it fails everything
   * else we did was wasting time.
   */

  rdb = cl_rdb;
  XjLoadFromResources(NULL, /* this is what we're tring to find out! */
		      NULL,
		      programName,
		      programClass,
		      startupResources,
		      XjNumber(startupResources),
		      (caddr_t) &startup);

  if (startup.name != NULL)
    programName = startup.name;

  displayName = startup.display;

  /*
  sprintf(dispRes, "%s.display", programName);
  sprintf(dispResClass, "%s.Display", programClass);

  if (XrmGetResource(cl_rdb, dispRes, dispResClass, &type, &value))
    displayName = (char *)(value.addr);
  */

  if ((display = XOpenDisplay(displayName)) == NULL)
    {
      sprintf(errtext, "could not open display %s", 
	       (displayName == NULL) ? "(null)" : displayName);
      XjFatalError(errtext);
      XjExit(-1);
    }

  /*
   * Load display resources into disp_rdb
   */
  display_resources = XResourceManagerString(display);
  if (display_resources != NULL)
    disp_rdb = XrmGetStringDatabase(display_resources);

  /*
   * Load application defaults into ad_rdb
   */
  xenvironment = startup.appDefs;
  if (xenvironment != NULL)
    {
      ad_rdb = XrmGetFileDatabase(xenvironment);
      if (ad_rdb == NULL)
	{
	  sprintf(errtext, "couldn't load %s; trying default", xenvironment);
	  XjWarning(errtext);
	}
    }

  if (ad_rdb == NULL) /* fallback */
    {
      char *appdefs;

      appdefs = (char *) XjMalloc(strlen(appdefdir) +
				  strlen(programClass) + 2);
      sprintf(appdefs, "%s/%s", appdefdir, programClass);
      ad_rdb = XrmGetFileDatabase(appdefs);
      XjFree(appdefs);

      xenvironment = (char *)getenv("XENVIRONMENT");
      if (xenvironment != NULL)
	{
	  XrmDatabase env_rdb = NULL;

	  env_rdb = XrmGetFileDatabase(xenvironment);
	  if (env_rdb == NULL)
	    {
	      sprintf(errtext, "couldn't load %s", xenvironment);
	      XjWarning(errtext);
	    }
	  else
	    {
	      if (ad_rdb == NULL)
		ad_rdb = env_rdb;
	      else
		XrmMergeDatabases(env_rdb, &ad_rdb);
	    }
	}
    }

  /*
   * Load user's defaults into user_rdb
   */
  if (startup.userDefs != NULL)
    userFile = startup.userDefs;
  if (userFile != NULL)
    {
      user_rdb = XrmGetFileDatabase(userFile);
      if (user_rdb == NULL && startup.userDefs != NULL)
	{
	  sprintf(errtext, "couldn't load %s", userFile);
	  XjWarning(errtext);
	}
    }

  /*
   * Ok! Now we merge them all together!
   * rdb = ((((ad_rdb)disp_rdb)user_rdb)cl_rdb)
   */
  if (ad_rdb != NULL)
    rdb = ad_rdb;
  else
    {
      if (disp_rdb != NULL)
	{
	  rdb = disp_rdb;
	  disp_rdb = NULL;
	}
      else
	{
	  if (user_rdb != NULL)
	    {
	      rdb = user_rdb;
	      user_rdb = NULL;
	    }
	  else
	    if (cl_rdb != NULL)
	      {
		rdb = cl_rdb;
		cl_rdb = NULL;
	      }
	}
    }

  if (disp_rdb != NULL)
    XrmMergeDatabases(disp_rdb, &rdb);
  if (user_rdb != NULL)
    XrmMergeDatabases(user_rdb, &rdb);
  if (cl_rdb != NULL)
    XrmMergeDatabases(cl_rdb, &rdb);

  /*
   * Create the root jet
   */
  screen = DefaultScreen(display);

  jet = (Jet)XjMalloc((unsigned) sizeof(RootRec));

  jet->core.classRec = rootJetClass;
  jet->core.name = programName;
  jet->core.display = display;
  jet->core.window = RootWindow(display, screen);
  jet->core.parent = NULL;
  jet->core.sibling = NULL;
  jet->core.child = NULL;

  /* hmmm... */
  jet->core.classRec->core_class.className = programClass;

  return jet;
}

#ifndef MAXSELFD
#define MAXSELFD 64
#endif

XjCallbackProc read_inputs[MAXSELFD];
char *read_args[MAXSELFD];

void XjReadCallback(where, fd, arg)
     XjCallbackProc where;
     int fd;
     char *arg;
{
    read_inputs[fd] = where;
    read_args[fd] = arg;
}

/*
 * Hacking this in right now because I don't have
 * the time right now to code up a general version.
 */
XjCallbackProc stdinavail = NULL;

void XjStdinCallback(where)
     XjCallbackProc where;
{
    XjReadCallback(where, 0, 0);
}

void XjRegisterEventHandler(where)
     XjEventProc where;
{
  eventHandler = where;
}

static int waitForSomething(jet)
     Jet jet;
{
  struct timeval now;
  struct timeval diff;
  static fd_set read, empty;
  static int inited = 0;
  int loop, nfds = 0;
  register int i;
  int n;
  int bytes = 0;
  char errtext[100];

  if (!inited)
    {
      inited = 1;
      FD_ZERO(&empty);
      FD_ZERO(&read);
    }

  while (XPending(jet->core.display) == 0)
    {
      /*
       * If bytes is nonzero, the last journey through the loop
       * reported activity on some socket. At this point, XPending
       * has said that there are no events. It doesn't check if
       * the connection was terminated. If the connection has been
       * terminated, the socket will be selected for read with no
       * data available on it. So we check if the X socket was one
       * of the ones selected. If so, and there is no data available
       * for reading (the ioctl) on it, it has probably been
       * terminated. So we call XNoOp, with the expectation that an
       * XIO error will be generated.
       */
      if (bytes &&
	  FD_ISSET(ConnectionNumber(jet->core.display), &read))
	{ /* no errors should be possible in ioctl */
	  (void)ioctl(ConnectionNumber(jet->core.display),
		      FIONREAD,
		      &bytes);
	  if (bytes == 0)
	    {
	      XNoOp(jet->core.display); /* should cause XIO */
	      XSync(jet->core.display, False); /* Yes, I really want this */
	    }
	  bytes = 0;
	}

      loop = 1;
      while (alarmList != NULL && loop)
	{
	  gettimeofday(&now, NULL);

	  diff.tv_sec = alarmList->when.tv_sec - now.tv_sec;
	  if (alarmList->when.tv_usec < now.tv_usec)
	    {
	      diff.tv_sec -= 1;
	      diff.tv_usec = (1000000 + alarmList->when.tv_usec) - now.tv_usec;
	    }
	  else
	    diff.tv_usec = alarmList->when.tv_usec - now.tv_usec;

	  if (diff.tv_sec >=0 && diff.tv_usec >=0)
	    loop = 0;
	  else
	    {
	      flushAlarmQueue();
	      XFlush(jet->core.display);
	    }
	}

      FD_SET(ConnectionNumber(jet->core.display), &read);

      nfds = 0;				/* RESET nfds to zero... */
      for (i = 0; i < MAXSELFD; i++)
	  if (read_inputs[i]) {
	      FD_SET(i, &read);
	      nfds = i;
	  }
      if (ConnectionNumber(jet->core.display) > nfds)
	  nfds = ConnectionNumber(jet->core.display);

      nfds++;				/* indexed based on 1, not 0 */

#ifdef notdefined
      if (checkSignals != NULL)
	checkSignals();		/* this could cause a late wakeup... */
#endif

      if (checkSignals != NULL  &&  checkSignals())
	continue;		/* this could cause a late wakeup... */


      if (DEBUG)
	{
	  printf("%d.%3.3d ", diff.tv_sec, diff.tv_usec);
	  fflush(stdout);
	}

      if (XPending(jet->core.display))
	break;

      n = select(nfds, &read, &empty, &empty,
		 (alarmList == NULL) ? 0 : &diff);
      switch(n)
	{
	case -1: /* some kind of error */
	  if (DEBUG)
	    printf("\nSELECT ERROR= -1\n");
	  if (errno == EINTR)
	    break; /* interrupt callback will be called before select */
	  sprintf(errtext, "%s %d\n\t%s %d\t%s %d\t%s %d\n",
		  "unexpected error from select:", errno,
		  "timeval.tv_sec =", diff.tv_sec,
		  "timeval.tv_usec =", diff.tv_usec,
		  "nfds =", nfds);
	  XjFatalError(errtext);
	  bytes = 0;
	  break;

	case 0: /* timed out */
	  if (DEBUG)
	    {
	      printf("| ");
	      fflush(stdout);
	    }
	  flushAlarmQueue();
	  XFlush(jet->core.display);
	  bytes = 0;
	  break;

	default: /* something must now be pending */
	  if (DEBUG)
	    printf("\nn=%d ", n);
	  for (i = 0; i < MAXSELFD; i++)
	      if (read_inputs[i] != NULL &&
		  FD_ISSET(i, &read))
	      {
		if (DEBUG)
		  printf("i=%d", i);
		  (*read_inputs[i])(i, read_args[i]);
		  FD_CLR(i, &read);
	      }
	  if (DEBUG)
	    {
	      if (FD_ISSET(ConnectionNumber(jet->core.display), &read))
		printf("X active");
	      printf("\n");
	    }
	  bytes = 1;
	  break;
	}
    }
  return 1;			/* currently never happens */
}

void XjSetSignalChecker(proc)
     XjCallbackProc proc;
{
  checkSignals = proc;
}

void XjEventLoop(jet)
     Jet jet;
{
  XEvent event;
  Boolean taken;
  Jet eventJet;

  while (1)
    {
      if (waitForSomething(jet) == 0)
	return; /* a signal arrived */

      XNextEvent(jet->core.display, &event);

      if (XCNOENT != XFindContext(jet->core.display, event.xany.window,
				  registerContext, (caddr_t *)&eventJet))
	{
	  /*
	   * If an event is not taken by its registered owner,
	   * pass it on to the parent if the window id matches
	   * the window id of the parent. Note that the registered
	   * owner will get the event whether the window id matches
	   * its own or not. The menu code depends on this.
	   */
	  if (eventJet->core.classRec->core_class.event != NULL)
	    { /* Really _should_ be non-NULL. Just check; not sure why */
	      taken = False;
	      do
		{
		  if (eventJet->core.classRec->core_class.event != NULL)
		    taken =
		      eventJet->core.classRec->
			core_class.event(eventJet, &event);
		  if (!taken)
		    eventJet = XjParent(eventJet);
		}
	      while (eventJet &&
		     (eventJet->core.window == event.xany.window)
		     && !taken);

	      if (!taken)
		{
		  switch(event.type)
		    {
		    case MappingNotify:
		      XRefreshKeyboardMapping(&(event.xmapping));
		      break;
		    default:
		      if (eventHandler)
			taken = eventHandler(&event);
		      break;
		    }
		}
	    }
	}
#ifdef DEBUG
      else
	fprintf(stderr, "Event on unclaimed window\n");
#endif
    }
}

void preRealizeJet(jet)
     Jet jet;
{
  Jet thisJet;

  thisJet = jet;

  while (thisJet != NULL)
    {
      while (thisJet->core.child != NULL)
	thisJet = thisJet->core.child;

      if (thisJet->core.classRec->core_class.preRealize != NULL)
	thisJet->core.classRec->core_class.preRealize(thisJet);

      while (thisJet != jet && thisJet->core.sibling == NULL)
	{
	  thisJet = XjParent(thisJet);
	  if (thisJet->core.classRec->core_class.preRealize != NULL)
	    thisJet->core.classRec->core_class.preRealize(thisJet);
	}

      if (thisJet != jet /* && thisJet->core.sibling != NULL */)
	thisJet = thisJet->core.sibling;
      else
	thisJet = NULL; /* a hack. :( */
    }	  
}

/*
 * Of course, this would make a lot more sense if it were recursive,
 * but didn't your parents tell you not to curse over and over again?
 * Besides, this is a family tree.
 */
void XjRealizeJet(jet)
     Jet jet;
{
  Jet thisJet;
  Jet parentJet, siblingJet;

  preRealizeJet(jet);

  parentJet = XjParent(jet);	/* cut off from ancestors temporarily */
  siblingJet = jet->core.sibling;
  XjParent(jet) = NULL;
  jet->core.sibling = NULL;

  thisJet = jet;
  while (thisJet != NULL)
    {
      /* may get its own window when we realize it */
      if (XjParent(thisJet) != NULL)
	thisJet->core.window = (XjParent(thisJet))->core.window;

      /* first realize the jet we're at */
      if (thisJet->core.classRec->core_class.realize != NULL)
	thisJet->core.classRec->core_class.realize(thisJet);

      /* now locate the next jet... */
      /* if this jet has a child, it's next. */
      if (thisJet->core.child != NULL)
	thisJet = thisJet->core.child;
      else
	/* no children; siblings next. */
	if (thisJet->core.sibling != NULL)
	  thisJet = thisJet->core.sibling;
	else
	  /* no siblings; aunts & uncles & relatives of grandparents next. */
	  while ((thisJet = XjParent(thisJet)) != NULL)
	    if (thisJet->core.sibling != NULL)
	      {
		thisJet = thisJet->core.sibling;
		break;
	      }
    }

  XjParent(jet) = parentJet;	/* unbastardize. or is that debastardize? */
  jet->core.sibling = siblingJet;
}

#define MAX_NAME_LEN 500
static char resClass[MAX_NAME_LEN], resInstance[MAX_NAME_LEN];

Jet XjFindJet(name, parent)
char *name;
Jet parent;
{
  Jet search;

  for (search = parent->core.child;
       search != NULL;
       search = search->core.sibling)
    if (strcmp(name, search->core.name) == 0)
      return search;

  return NULL;
}

void XjLoadFromResources(display, window, classPtr, instPtr,
			 resources, num_resources, destination)
Display *display;
Window window;
char *classPtr, *instPtr;
XjResource resources[];
int num_resources;
caddr_t destination;
{
  char *type;
  XrmValue value;
  int resCount;

  for (resCount = 0; resCount < num_resources; resCount++)
    {
      sprintf(resClass, "%s.%s", classPtr, 
	      resources[resCount].resource_class);

      sprintf(resInstance, "%s.%s", instPtr,
	      resources[resCount].resource_name);

      if (XrmGetResource(rdb, resInstance, resClass, &type, &value))
	XjFillInValue(display,		/* found it, so load it... */
		      window,
		      destination,
		      &resources[resCount],
		      XjRString, value.addr);
      else
	XjFillInValue(display,		/* didn't find it, load default... */
		      window,
		      destination,
		      &resources[resCount],
		      resources[resCount].default_type,
		      resources[resCount].default_addr);
    }
}


static void GetVal(src, dst, size)
     char *src;
     caddr_t *dst;
     int size;
{
  memcpy(dst, src, size);
}


void XjVaGetValues(jet, va_alist)
Jet jet;
va_dcl
{
  va_list args;
  int resCount;
  char *valName;
  caddr_t val;

  va_start(args);

  while (NULL != (valName = va_arg(args, char *)))
    {
      val = va_arg(args, caddr_t);

      for (resCount = 0;
	   resCount < jet->core.classRec->core_class.num_resources;
	   resCount++)
	{
	  XjResource resource;
	  resource = jet->core.classRec->core_class.resources[resCount];

	  if (!strcmp(valName, resource.resource_name))
	    {
	      GetVal(((caddr_t)jet) + resource.resource_offset, &val,
		     (resource.resource_size > 4)
		     ? 4 : resource.resource_size);
	      break;
	    }
	}
    }
  val = va_arg(args, caddr_t); /* pop the last one */
  va_end(args);
}


/* this is gross. and there are memory leaks having to do with
   resource allocation that need to be plugged up. */
void XjDestroyJet(jet)
     Jet jet;
{
  Jet thisJet;
  Jet parent, sibling;

  /* remove jet from tree */
  if ((XjParent(jet))->core.child == jet)
    (XjParent(jet))->core.child = jet->core.sibling;
  else
    {
      thisJet = (XjParent(jet))->core.child;
      while (thisJet->core.sibling != jet)
	thisJet = thisJet->core.sibling;
      thisJet->core.sibling = jet->core.sibling;
    }

  thisJet = jet;

  while (thisJet != NULL)
    {
      while (thisJet->core.child != NULL)
	thisJet = thisJet->core.child;

      parent = XjParent(thisJet);
      sibling = thisJet->core.sibling;
      if (thisJet->core.classRec->core_class.destroy != NULL)
	thisJet->core.classRec->core_class.destroy(thisJet);
      XjFree(thisJet->core.name);
      XjFree((char *)thisJet);

      while (thisJet != jet && sibling == NULL)
	{
	  thisJet = parent;

	  parent = XjParent(thisJet);
	  sibling = thisJet->core.sibling;
	  if (thisJet->core.classRec->core_class.destroy != NULL)
	    thisJet->core.classRec->core_class.destroy(thisJet);
	  XjFree(thisJet->core.name);
	  XjFree((char *)thisJet);
	}

      if (thisJet != jet /* && sibling != NULL */)
	thisJet = sibling;
      else
	thisJet = NULL; /* a hack. :( */
    }	  
}
