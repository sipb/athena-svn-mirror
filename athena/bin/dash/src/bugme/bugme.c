/* Copyright 1997 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: bugme.c,v 1.1 1997-02-25 19:09:19 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/param.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/Xos.h>
#include <Jets.h>
#include <Window.h>
#include <Button.h>
#include <Label.h>
#include <DClock.h>
#include <Form.h>
#include <Tree.h>
#include "Snoop.h"
#include "bugme.h"

int ok_x, ok_y; /* most recent acceptable position for the counter window */

/* Constants computed in main() */
Jet root;
int proper_width, proper_height, display_width, display_height;
pid_t pid;

/* These definitions are needed by the tree jet */
JetClass *jetClasses[] =
{ &treeJetClass, &windowJetClass, &buttonJetClass, &labelJetClass,
    &formJetClass };

int numJetClasses = XjNumber(jetClasses);

static XrmOptionDescRec op_table[] = {
{"+rv",         "*reverseVideo", XrmoptionNoArg,        (void *) "off"},
{"-background", "*background",  XrmoptionSepArg,        (void *) NULL},
{"-bd",         "*borderColor", XrmoptionSepArg,        (void *) NULL},
{"-bg",         "*background",  XrmoptionSepArg,        (void *) NULL},
{"-bordercolor","*borderColor", XrmoptionSepArg,        (void *) NULL},
{"-borderwidth",".borderWidth", XrmoptionSepArg,        (void *) NULL},
{"-bw",         ".borderWidth", XrmoptionSepArg,        (void *) NULL},
{"-display",    ".display",     XrmoptionSepArg,        (void *) NULL},
{"-fg",         "*foreground",  XrmoptionSepArg,        (void *) NULL},
{"-fn",         "*font",        XrmoptionSepArg,        (void *) NULL},
{"-font",       "*font",        XrmoptionSepArg,        (void *) NULL},
{"-foreground", "*foreground",  XrmoptionSepArg,        (void *) NULL},
{"-geometry",   "*menuTree.window.geometry",    XrmoptionSepArg,
   (void *) NULL},
{"-reverse",    "*reverseVideo", XrmoptionNoArg,        (void *) "on"},
{"-rv",         "*reverseVideo", XrmoptionNoArg,        (void *) "on"},
{"-xrm",        NULL,           XrmoptionResArg,        (void *) NULL},
{"-name",       ".name",        XrmoptionSepArg,        (void *) NULL},
{"-appdefs",	".appDefs",	XrmoptionSepArg,	(void *) NULL},
{"-f",		".appDefs",	XrmoptionSepArg,	(void *) NULL},
{"-userdefs",	".userDefs",	XrmoptionSepArg,	(void *) NULL},
{"-bugnames",	".bugnames",	XrmoptionSepArg,	(void *) NULL},
{"-exec",	".exec",	XrmoptionSepArg,	(void *) NULL},
};

My_resources parms;

#define offset(field) XjOffset(My_resources *,field)

static XjResource app_resources[] =
{
  { "bugnames", "Bugnames", XjRString, sizeof(char *),
      offset(bugnames), XjRString, (void *) NULL },
  { "exec", "Exec", XjRString, sizeof(char *),
      offset(command), XjRString, (void *) NULL },
};

#undef offset

/* Tree callbacks... */

int print_tree(Jet from_jet, char *what, void *data)
{
  Jet j;

  for (j = root->core.child; j != NULL; j = j->core.sibling)
    fprintf(stdout, "%s\n", j->core.name);
  return 0;
}

int map_tree(Jet from_jet, char *what, void *data)
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

int unmap_tree(Jet from_jet, char *what, void *data)
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

int create_tree(Jet from_jet, char *what, void *data)
{
  if (NULL == XjFindJet(what, root))
    XjRealizeJet(XjVaCreateJet(what, treeJetClass, root, NULL, NULL));
  return 0;
}

int create_map_tree(Jet from_jet, char *what, void *data)
{
  if (NULL == XjFindJet(what, root))
    XjRealizeJet(XjVaCreateJet(what, treeJetClass, root, NULL, NULL));
  else
    map_tree(from_jet, what, data);

  return 0;
}

int destroy_tree(Jet from_jet, char *what, void *data)
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

int warp_tree(Jet from_jet, char *what, void *data)
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

fatal(Display *display)
{
  XjExit(-1);
}

static int (*def_handler)();

static int handler(Display *display, XErrorEvent *error)
{
  if (error->error_code == BadWindow  ||  error->error_code == BadAtom)
    return 0;

  def_handler(display, error);
  return 0;			/* it'll never get this far anyway... */
}

int delete(Jet from_jet, char *what, void *data)
{
  XjDestroyJet(from_jet);
  return 0;
}

int delete_parent(Jet from_jet, char *what, void *data)
{
  XjDestroyJet(XjParent(from_jet));
  return 0;
}

/* This kind of operation isn't actually safe in principle,
 * but is in practice. XjDestroyJet needs to be modified so
 * that this can be done safely.
 */
int delete_ancestor(Jet from_jet, int generationGap, void *data)
{
  Jet who = from_jet;

  for (; generationGap--; generationGap >= 0)
    who = XjParent(who);

  XjDestroyJet(who);
  return 0;
}

Boolean event(Jet from_jet, char *what, XEvent *event)
{
  int broken = 0;
  static struct timeval last;
  static int howmany;
  struct timeval now;
  static int clickx, clicky, movex, movey;

  switch(event->type)
    {
    case VisibilityNotify:
      /* If someone obscures us, come back. */
      if (event->xvisibility.window == from_jet->core.window)
	if (event->xvisibility.state != VisibilityUnobscured)
	  {
	    /* don't fight with ourselves. */
	    gettimeofday(&now, NULL);
	    if (now.tv_sec - last.tv_sec > 2)
	      {
		last = now;
		howmany = 0;
	      }
	    else
	      {
		howmany++;
		if (howmany > 20)
		  {
		    ok_x = rand() % (display_width - proper_width);
		    ok_y = rand() % (display_height - proper_height);
		    XMoveResizeWindow(root->core.display,
				      from_jet->core.window,
				      ok_x, ok_y, proper_width, proper_height);
		    howmany = 0;
		    last = now;
		  }
	      }

	    MapWindow(XjParent(from_jet), True);
	  }
      break;

    case UnmapNotify:
      /* If someone unmaps us, come back. */
      if (event->xunmap.window == from_jet->core.window)
	MapWindow(XjParent(from_jet), True);
      break;

    case ConfigureNotify:
      if (event->xconfigure.window == from_jet->core.window)
	{
	  /* If someone resizes us, go back to the size we like. */
	  if (event->xconfigure.width != proper_width ||
	      event->xconfigure.height != proper_height)
	    broken = 1;

	  /* If someone moves us off the screen, go back to where we were. */
	  if (event->xconfigure.x + proper_width > display_width ||
	      event->xconfigure.y + proper_height > display_height ||
	      event->xconfigure.x < 0 || event->xconfigure.y < 0)
	    broken = 1;
	  else
	    {
	      ok_x = event->xconfigure.x;
	      ok_y = event->xconfigure.y;
	    }

	  if (broken)
	    XMoveResizeWindow(root->core.display, from_jet->core.window,
			      ok_x, ok_y, proper_width, proper_height);
	}
      break;

    case ReparentNotify:
      /* If we are reparented away from the root, reparent back to
       * the root.
       */
      if (event->xreparent.window == from_jet->core.window &&
	  event->xreparent.event != root->core.window)
	XReparentWindow(root->core.display, from_jet->core.window,
			root->core.window, ok_x, ok_y);

      /* If someone reparents us a child, destroy it. */
      if (event->xreparent.window != from_jet->core.window)
	XDestroyWindow(root->core.display, event->xreparent.window);
      break;

    case CreateNotify:
      /* If someone creates a child for us, destroy it. */
      if (event->xcreatewindow.parent == from_jet->core.window)
	XDestroyWindow(root->core.display, event->xcreatewindow.window);
      break;

    /* Let the user drag the window around, since we aren't letting
     * the window manager do it.
     */
    case ButtonPress:
      clickx = event->xbutton.x_root;
      clicky = event->xbutton.y_root;
      movex = ok_x;
      movey = ok_y;
      break;

    case MotionNotify:
      movex += event->xmotion.x_root - clickx;
      movey += event->xmotion.y_root - clicky;

      if (movex < 0)
	movex = 0;
      if (movex + proper_width + 1 >= display_width)
	movex = display_width - proper_width - 2;
      if (movey < 0)
	movey = 0;
      if (movey + proper_height + 1 >= display_height)
	movey = display_height - proper_height - 2;

      clickx = event->xmotion.x_root;
      clicky = event->xmotion.y_root;

      XMoveWindow(root->core.display, from_jet->core.window, movex, movey);
      break;

    default:
      break;
    }

  return False; /* Pass the event on to whoever cares */
}

void post_warning(char *name, int id)
{
  XjRealizeJet(XjVaCreateJet(name, treeJetClass, root, NULL, NULL));
}

void post_intermittent_warning(Intermittent *i, int id)
{
  XjRealizeJet(XjVaCreateJet(i->name, treeJetClass, root, NULL, NULL));
  XjAddWakeup(post_intermittent_warning, i, i->timeout);
}

void setup_warnings()
{
  char name[50], class[50];
  char *type, *what, *next = NULL;
  XrmValue value;
  int timeout, freq;
  Intermittent *i;

  while (what = strtok(parms.bugnames, " "))
    {
      parms.bugnames = NULL;

      sprintf(name, "%s.%s.timeout", programName, what);
      sprintf(class, "%s.%s.Timeout", programClass, what);
      if (XrmGetResource(rdb, name, class, &type, &value))
	{
	  timeout = 1000 * strtol((char *)value.addr, &next, 0);
	  if (next)
	    freq = 1000 * strtol(next, NULL, 0);
	    
	  if (freq == 0)
	    XjAddWakeup(post_warning, what, timeout);
	  else
	    {
	      i = (Intermittent *)XjMalloc(sizeof(Intermittent));
	      i->name = what;
	      i->timeout = freq;
	      XjAddWakeup(post_intermittent_warning, i, timeout);
	    }
	}
      else
	{
	  char errtext[100];

	  sprintf(errtext, "no timeout setting for %s exists", what);
	  XjWarning(errtext);
	}
    }
}

XjCallbackRec callbacks[] =
{
  /* tree operations */
  { "createTree", create_tree },
  { "createMapTree", create_map_tree },
  { "destroyTree", destroy_tree },
  { "warpTree", warp_tree },
  { "mapTree", map_tree },
  { "unmapTree", unmap_tree },
  { "printTree", print_tree },
  /* misc */
  { "delete", delete },
  { "deleteParent", delete_parent },
  { "deleteAncestor", delete_ancestor },
  { "event", event },
};

/* Compare two strings, ignoring leading whitespace and
 * terminating at trailing whitespace, ignoring case.
 */
int strwhitecasecmp(char *s1, char *s2)
{
  char *c1, *c2;

  c1 = s1;
  c2 = s2;

  while (isspace(*c1) && *c1 != '\0')
    c1++;
  while (isspace(*c2) && *c2 != '\0')
    c2++;

  while (*c1 != '\0' && *c2 != '\0' &&
	 !isspace(*c1) && !isspace(*c2) &&
	 tolower(*c1) == tolower(*c2))
    c1++, c2++;

  if ((*c1 == '\0' || isspace(*c1)) &&
      (*c2 == '\0' || isspace(*c2)))
    return 0;

  return 1;
}

/* Return 1 if the timer should be started for this host, 0 otherwise. */
int start_timer()
{
  FILE *hosts;
  char ourname[MAXHOSTNAMELEN + 1], thisname[MAXHOSTNAMELEN + 1];

  if (gethostname(ourname, sizeof(ourname)))
    return 0;

  hosts = fopen(QUICK_FILE, "r");
  if (hosts == NULL)
    return 0;

  while (!feof(hosts) && fgets(thisname, sizeof(thisname), hosts))
    if (!strwhitecasecmp(ourname, thisname))
      {
	fclose(hosts);
	return 1;
      }

  fclose(hosts);

  return 0;
}

void child()
{
  int status;

  if (-1 == waitpid(pid, &status, WNOHANG))
    return;

  if (WIFEXITED(status))
    XjExit(WEXITSTATUS(status));

  if (WIFSIGNALED(status))
    XjExit(WTERMSIG(status));
}

main(int argc, char **argv)
{
  Display *display;
  char user_file[100];
  Jet window, form, timecounter, snooper;
  struct timeval t;
  char *fakeargv[2];
  int fakeargc = 1;
  struct sigaction sigact;

  if (argc < 2)
    {
      fprintf(stderr, "usage: %s command [arg ...]\n",
	      argv ? *argv : "bugme");
      exit(2);
    }

  if (!start_timer())
    {
      execvp(argv[1], &argv[1]);
      fprintf(stderr, "%s: couldn't exec %s (%s)\n",
	      argv[0], argv[1], strerror(errno));
      exit(3);
    }

  /* Not taking X args in this program, but make something to pass
   * to the toolkit.
   */
  fakeargv[0] = argv[0];
  fakeargv[1] = NULL;

  gettimeofday(&t, NULL);

  (void)XSetIOErrorHandler(fatal);

  strcpy(user_file, "Bugme");

  root = XjCreateRoot(&fakeargc, fakeargv, "Bugme", user_file,
		      op_table, XjNumber(op_table));

  XjLoadFromResources(NULL,
		      NULL,
		      programClass,
		      programName,
		      app_resources,
		      XjNumber(app_resources),
		      (void *) &parms);
  display = root->core.display;

  XjRegisterCallbacks(callbacks, XjNumber(callbacks));

  window = XjVaCreateJet("timerWindow", windowJetClass, root, NULL, NULL);
  snooper = XjVaCreateJet("snooper", snoop_jet_class, window, NULL, NULL);
  form = XjVaCreateJet("timerForm", formJetClass, window, NULL, NULL);
  timecounter = XjVaCreateJet("timer", dClockJetClass, form, 
			      XjNtimeOffset, -t.tv_sec, NULL, NULL);

  XjRealizeJet(root);
  XjSelectInput(display, window->core.window, SubstructureNotifyMask |
		ButtonPressMask | ButtonMotionMask);

  proper_width = window->core.width;
  proper_height = window->core.height;
  ok_x = window->core.x;
  ok_y = window->core.y;
  display_width = XDisplayWidth(display, DefaultScreen(display));
  display_height = XDisplayHeight(display, DefaultScreen(display));
  srand(t.tv_sec);

  setup_warnings();

  /* Now that we have a good head start (mainly we need to wait until
   * after we've read our resource database), start the user's session
   * (well, whatever command was passwd). Fortunately, Xj does not
   * support Xdefaults.
   */

  sigact.sa_flags = SA_NOCLDSTOP;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_handler = child;
  sigaction(SIGCHLD, &sigact, NULL);

  pid = fork();
  if (pid == 0)
    {
#ifdef HAS_PUTENV
      putenv("ATHENA_QUICK=1");
#else
      setenv("ATHENA_QUICK", "1", 1);
#endif
      execvp(argv[1], &argv[1]);
      fprintf(stderr, "%s: couldn't exec %s (%s)\n",
	      argv[0], argv[1], strerror(errno));
      XjExit(3);
    }

  XjEventLoop(root);
}
