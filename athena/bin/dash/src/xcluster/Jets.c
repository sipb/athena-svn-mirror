/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Jets.c,v $
 * $Author: epeisach $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/Jets.c,v 1.1 1991-07-17 10:56:25 epeisach Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <X11/X.h>		/* For DEBUG */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "Jets.h"
#include "fd.h"
#include "hash.h"

int DEBUG = 0;

static XjCallbackProc checkSignals = NULL;

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
    /* class name */	"Root",
    /* jet size */	sizeof(RootRec),
    /* initialize */	NULL,
    /* prerealize */    NULL,
    /* realize */	NULL,
    /* event */		NULL,
    /* expose */	NULL,
    /* querySize */     NULL,
    /* move */		NULL,
    /* resize */        NULL,
    /* destroy */       NULL,
    /* resources */	NULL,
    /* number of 'em */	0
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

void XjFatalError(message)
     char *message;
{
  fprintf(stderr, "%s: %s\n", programName, message);
  exit(-1);
}

void XjWarning(string)
     char *string;
{
  fprintf(stderr, "%s: %s\n", programName, string);
}

extern end, etext, edata;

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
      alarmList->wakeup(alarmList->data);

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
    jet->core.classRec->core_class.move(jet, x, y);
}

void XjResize(jet, size)
     Jet jet;
     XjSize *size;
{
  if (jet->core.classRec->core_class.resize != NULL)
    jet->core.classRec->core_class.resize(jet, size);
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

  end = index(ptr, '(');
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

  if (*ptr == '"')
    {
      ptr++;
      end = index(ptr, '"');
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
  else
    {
      intParam = atoi(ptr);
      type = argInt;
    }

  end = index(ptr, ')');
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

XFontStruct *XjLoadQueryFontCache(display, name)
     Display *display;
     char *name;
{
  static struct hash *fonts = NULL;
  XrmQuark qname;
  XFontStruct *fs;

  if (fonts == NULL)
    fonts = create_hash(13);

  qname = XrmStringToQuark(name);
  fs = (XFontStruct *)hash_lookup(fonts, qname);

  if (fs == NULL)
    {
      fs = XLoadQueryFont(display, name);
      (void)hash_store(fonts, qname, fs);
    }

  return fs;
}

void XjFillInValue(display, window, where, resource, type, address)
     Display *display;
     Window window;
     caddr_t where;
     XjResource *resource;
     char *type;
     caddr_t address;
{
  XFontStruct *fontstr;
  XjPixmap p;
  char errtext[100];

  /*
   * Nonconversion
   */
  if (!strcmp(type, resource->resource_type))
    {
      bcopy((char *)&address,
	    ((char *)where + resource->resource_offset),
	    (resource->resource_size > 4) ? 4 : resource->resource_size);
      return;
    }

  if (!strcmp(type, XjRString))
    {
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
       * string to XFontStruct conversion
       */
      if (!strcmp(resource->resource_type, XjRFontStruct))
	{
	  if (strcmp(XjDefaultFont, address) != 0)
	    {
	      fontstr = XjLoadQueryFontCache(display, address);
	      if (fontstr != NULL)
		{
		  *(XFontStruct **)
		    ((char *)where + resource->resource_offset) =
		      fontstr;
		  return;
		}
	      else
		{
		  sprintf(errtext, "Unknown font: \"%s\"", address);
		  XjWarning(errtext);
		}
	    }

	  fontstr = XjLoadQueryFontCache(display,
				     "-*-*-*-R-*-*-*-120-*-*-*-*-ISO8859-1");
	      
	  if (fontstr == NULL)
	    XjFatalError("couldn't get a font");
	      
	  *(XFontStruct **)((char *)where + resource->resource_offset) =
	    fontstr;
	  return;
	}

      /*
       * string to Color conversion
       */
      if (!strcmp(resource->resource_type, XjRColor))
	{
	  XColor c;
      
	  if (strcasecmp(XjNone, address) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = -1;
	      return;
	    }

	  if (strcmp(XjDefaultForeground, address) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) =
		BlackPixel(display, DefaultScreen(display));
	      return;
	    }

	  if (strcmp(XjDefaultBackground, address) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) =
		WhitePixel(display, DefaultScreen(display));
	      return;
	    }

	  /* we give black in case we can't figure something better out */
	  *((int *)((char *)where + resource->resource_offset)) =
	    BlackPixel(display, DefaultScreen(display));

	  if (!XParseColor(display,
			   DefaultColormap(display, DefaultScreen(display)),
			   address,
			   &c))
	    {
	      sprintf(errtext, "couldn't parse color: %s", address);
	      XjWarning(errtext);
	    }

	  if (!XAllocColor(display,
			   DefaultColormap(display, DefaultScreen(display)),
			   &c))
	    {
	      sprintf(errtext, "couldn't allocate color: %s", address);
	      XjWarning(errtext);
	    }

	  *((int *)((char *)where + resource->resource_offset)) = c.pixel;

	  return;
	}

      /*
       * String to Justify conversion
       */
      if (!strcmp(resource->resource_type, XjRJustify))
	{
	  if (strcasecmp(address, XjLeftJustify) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = Left;
	      return;
	    }

	  if (strcasecmp(address, XjCenterJustify) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = Center;
	      return;
	    }

	  if (strcasecmp(address, XjRightJustify) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = Right;
	      return;
	    }

	  *((int *)((char *)where + resource->resource_offset)) = Center;

	  sprintf(errtext, "bad justify value: %s", address);
	  XjWarning(errtext);
	  return;
	}

      /*
       * String to Orientation conversion
       */
      if (!strcmp(resource->resource_type, XjROrientation))
	{
	  if (strcasecmp(address, XjVertical) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = Vertical;
	      return;
	    }

	  if (strcasecmp(address, XjHorizontal) == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset))
		= Horizontal;
	      return;
	    }

	  *((int *)((char *)where + resource->resource_offset)) = Vertical;

	  sprintf(errtext, "bad orientation value: %s", address);
	  XjWarning(errtext);
	  return;
	}

      /*
       * String to Boolean conversion
       */
      if (!strcmp(resource->resource_type, XjRBoolean))
	{
	  if (strcasecmp(address, "true") == 0 ||
	      strcasecmp(address, "yes") == 0 ||
	      strcasecmp(address, "on") == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = True;
	      return;
	    }

	  if (strcasecmp(address, "false") == 0 ||
	      strcasecmp(address, "no") == 0 ||
	      strcasecmp(address, "off") == 0)
	    {
	      *((int *)((char *)where + resource->resource_offset)) = False;
	      return;
	    }

	  *((int *)((char *)where + resource->resource_offset))
	    = atoi(address);
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

      /*
       * String to Pixmap conversion
       */
      if (!strcmp(resource->resource_type, XjRPixmap))
	{
	  XjPixmap *newPixmap = NULL;

	  if (address == NULL)
	    {
	      *((XjPixmap **)((char *)where + resource->resource_offset))
		= NULL;
	      return;
	    }

	  switch(XReadBitmapFile(display, window,
				 (char *)address, &p.width, &p.height,
				 &p.pixmap, &p.hot_x, &p.hot_y))
	    {
	    case BitmapOpenFailed:
	    case BitmapFileInvalid:
	    case BitmapNoMemory:
	      sprintf(errtext, "error reading bitmap %s",
		      address);
	      XjWarning(errtext);
	      break;
	    case BitmapSuccess:
	      newPixmap = (XjPixmap *)XjMalloc((unsigned) sizeof(XjPixmap));
	      bcopy((char *)&p, (char *)newPixmap, sizeof(XjPixmap));
	    }

	  *((XjPixmap **)((char *)where + resource->resource_offset)) =
	    newPixmap;

	  return;
	}
    }

  sprintf(errtext, "unknown conversion: %s to %s",
	  type, resource->resource_type); 
  XjWarning(errtext);
}

/*
 * This function is broken...
 * In fact, lots of things are broken.
 * C is broken... Unix is broken... Why break such a fine tradition?
 * (Watch out for paradoxes!)
 */
void XjCopyValue(where, resource, value)
     Jet where;
     XjResource *resource;
     XjArgVal value;
{
  bcopy((char *)&value,
	((char *)where + resource->resource_offset),
	(resource->resource_size > 4) ? 4 : resource->resource_size);
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
      exit(-1);
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

  if (XCNOMEM == XSaveContext(jet->core.display, window, registerContext, jet))
    {
      XjFatalError("out of memory in XSaveContext");
      exit(-1);
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
      programName = rindex (argv[0], '/');
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
		      &startup);

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
      exit(-1);
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
      xenvironment = (char *)getenv("XENVIRONMENT");
      if (xenvironment != NULL)
	{
	  ad_rdb = XrmGetFileDatabase(xenvironment);
	  if (ad_rdb == NULL)
	    {
	      sprintf(errtext, "couldn't load %s", xenvironment);
	      XjWarning(errtext);
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

static int waitForSomething(jet)
     Jet jet;
{
  struct timeval now;
  struct timeval diff;
  static Fd_set read, empty;
  static int inited = 0;
  int loop, nfds = 0;
  register int i;
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

      if (checkSignals != NULL)
	checkSignals(); /* this could cause a late wakeup... */

      switch(select(nfds,
		    &read, &empty, &empty, (alarmList == NULL) ? 0 : &diff))
	{
	case -1: /* some kind of error */
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
	  flushAlarmQueue();
	  XFlush(jet->core.display);
	  bytes = 0;
	  break;

	default: /* something must now be pending */
	  for (i = 0; i < MAXSELFD; i++)
	      if (read_inputs[i] != NULL &&
		  FD_ISSET(i, &read))
	      {
		  (*read_inputs[i])(i, read_args[i]);
		  FD_CLR(i, &read);
	      }
	  bytes = 1;
	  break;
	}
    }
  return 1; /* currently never happens */
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
      /* first realize the jet we're at */
      if (thisJet->core.classRec->core_class.realize != NULL)
	{
	  /* may get its own window when we realize it */
	  if (XjParent(thisJet) != NULL)
	    thisJet->core.window = (XjParent(thisJet))->core.window;
	  thisJet->core.classRec->core_class.realize(thisJet);
	}

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

#define MAXNAMELEN 500
static char className[MAXNAMELEN]; /* deserve to lose long before this */
static char instanceName[MAXNAMELEN];

static char resClass[MAXNAMELEN], resInstance[MAXNAMELEN];

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
  Boolean foundRes;
  int resCount;

  for (resCount = 0; resCount < num_resources; resCount++)
    {
      sprintf(resClass, "%s.%s", classPtr, 
	      resources[resCount].resource_class);

      sprintf(resInstance, "%s.%s", instPtr,
	      resources[resCount].resource_name);

      if (XrmGetResource(rdb, resInstance, resClass, &type, &value))
	foundRes = True;
      else
	foundRes = False;

      if (foundRes)
	XjFillInValue(display,
		      window,
		      destination,
		      &resources[resCount],
		      XjRString, value.addr);
      else
	XjFillInValue(display,
		      window,
		      destination,
		      &resources[resCount],
		      resources[resCount].default_type,
		      resources[resCount].default_addr);
    }
}

Jet XjVaCreateJet(name, class, parent, va_alist)
char *name;
JetClass class;
Jet parent;
va_dcl
{
  va_list args;
  char *classPtr, *instPtr;
  int resCount;
  char *valName;
  XjArgVal val;

  Jet jet, thisJet;
  int len;
  Boolean validName;

  jet = (Jet)XjMalloc((unsigned) class->core_class.jetSize);

  bzero((char *) jet, class->core_class.jetSize);

  jet->core.classRec = class;
  jet->core.name = XjNewString(name);
  jet->core.display = parent->core.display;
  jet->core.window = parent->core.window; /* may get its own window later */
  jet->core.borderWidth = 0;	/* Jet must change this if desired; supplied
				   for reasonable geometry management */
  jet->core.need_expose = False; /* it'll get an expose soon enough... */

  jet->core.parent = parent;
  jet->core.sibling = parent->core.child;
  jet->core.child = NULL;

  parent->core.child = jet;

  /* Generate the class and instance names */

  /*
   * Initialize the pointers to the place after the null, since
   * inside the loop we backup the length + 1 of the string to
   * add in order to skip over the '.' that we might want...
   * This hack makes it unnecessary to special case inside the
   * loop.
   */
  classPtr = className + MAXNAMELEN;
  instPtr = instanceName + MAXNAMELEN;
  classPtr[-1] = '\0'; instPtr[-1] = '\0';
  thisJet = jet;
  validName = True;

  while (thisJet != NULL)
    {
      if (thisJet->core.name)
	len = strlen(thisJet->core.name);
      else
	{
	  validName = False;
	  break;
	}

      if (len < (instPtr - instanceName)) /* includes '.' */
	{
	  instPtr -= len + 1;
	  bcopy(thisJet->core.name, instPtr, len);
	  instPtr[-1] = '.';
	}
      else
	{
	  XjWarning("Full jet instance name too long.");
	  validName = False;
	  break;
	}

      /* now do it all again for the class... */
      if (thisJet->core.classRec->core_class.className)
	len = strlen(thisJet->core.classRec->core_class.className);
      else
	{
	  validName = False;
	  break;
	}

      if (len < (classPtr - className)) /* includes . */
	{
	  classPtr -= len + 1;
	  bcopy(thisJet->core.classRec->core_class.className, classPtr, len);
	  classPtr[-1] = '.';
	}
      else
	{
	  XjWarning("Full jet class name too long.");
	  validName = False;
	  break;
	}

      thisJet = thisJet->core.parent;
    }

/*
  fprintf(stdout, "%s\n%s\n",
	  classPtr, instPtr);
*/

  if (validName)
    XjLoadFromResources(jet->core.display,
			jet->core.window,
			classPtr,
			instPtr,
			jet->core.classRec->core_class.resources,
			jet->core.classRec->core_class.num_resources,
			(caddr_t)jet);

  va_start(args);

  while (NULL != (valName = va_arg(args, char *)))
    {
      val = va_arg(args, XjArgVal);
/*      fprintf(stdout, "%d\n", val); */

      for (resCount = 0;
	   resCount < jet->core.classRec->core_class.num_resources;
	   resCount++)
	if (!strcmp(valName, jet->core.classRec->
		    core_class.resources[resCount].resource_name))
	  {
	    XjCopyValue(jet,
			&jet->core.classRec->core_class.resources[resCount],
			val);
	    break;
	  }

      if (resCount == jet->core.classRec->core_class.num_resources)
	fprintf(stdout, "no such resource name: %s\n", valName);
    }

  val = va_arg(args, XjArgVal); /* pop the last one */
  va_end(args);

  /* now call the initialize procedure */
  if (jet->core.classRec->core_class.initialize != NULL)
    jet->core.classRec->core_class.initialize(jet);

  return jet;
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
      for (thisJet = (XjParent(jet))->core.child;
	   thisJet->core.sibling != jet;
	   thisJet = thisJet->core.sibling);
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
