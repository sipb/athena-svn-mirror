/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/xcluster.c,v $
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
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/xcluster.c,v 1.2 1991-07-17 10:58:17 epeisach Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <fcntl.h>
#include "Jets.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>
#include "Window.h"
#include "Button.h"
#include "Label.h"
#include "Icon.h"
#include "Form.h"
#include "Tree.h"
#include "Drawing.h"

#include "xcluster.h"
#include "points.h"


int check_cluster();
struct cluster *find_text();

extern int DEBUG;

Jet root, text, map;
GC map_gc = NULL;
XPoint *points2 = NULL;
char *progname;
char form[BUFSIZ];
struct cluster *Current;





/*
 * These definitions are needed by the tree jet
 */
JetClass *jetClasses[] =
{ &treeJetClass, &windowJetClass, &buttonJetClass, &labelJetClass,
    &formJetClass, &drawingJetClass};

int numJetClasses = XjNumber(jetClasses);

static XrmOptionDescRec opTable[] = {
{"+rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "off"},
{"-background", "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bd",         "*borderColor", XrmoptionSepArg,        (caddr_t) NULL},
{"-bg",         "*background",  XrmoptionSepArg,        (caddr_t) NULL},
{"-bordercolor","*borderColor", XrmoptionSepArg,        (caddr_t) NULL},
{"-borderwidth","*xclusterWindow.borderWidth", XrmoptionSepArg,
							(caddr_t) NULL},
{"-bw",         "*xclusterWindow.borderWidth", XrmoptionSepArg,
							(caddr_t) NULL},
{"-display",    ".display",     XrmoptionSepArg,        (caddr_t) NULL},
{"-fg",         "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-fn",         "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-font",       "*font",        XrmoptionSepArg,        (caddr_t) NULL},
{"-foreground", "*foreground",  XrmoptionSepArg,        (caddr_t) NULL},
{"-geometry",   "*xclusterWindow.geometry",XrmoptionSepArg,	(caddr_t) NULL},
{"-reverse",    "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-rv",         "*reverseVideo", XrmoptionNoArg,        (caddr_t) "on"},
{"-xrm",        NULL,           XrmoptionResArg,        (caddr_t) NULL},
{"-name",       ".name",        XrmoptionSepArg,        (caddr_t) NULL},
{"-appdefs",	".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-f",		".appDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-userdefs",	".userDefs",	XrmoptionSepArg,	(caddr_t) NULL},
{"-inactive",	".inactive",	XrmoptionSepArg,	(caddr_t) 60},
};

typedef struct _MyResources
{
  int inactive;
} MyResources;

typedef struct _MyResources *MyResourcesPtr;

MyResources parms;

#define offset(field) XjOffset(MyResourcesPtr,field)

static XjResource appResources[] =
{
  { "inactive", "Inactive", XjRInt, sizeof(int),
      offset(inactive), XjRInt, (caddr_t) 0 },
};

#undef offset



int div;
int xleft = 0, yleft = 0;
struct cluster *circled = NULL;
Pixmap pixmap_on = NULL, pixmap_off = NULL;

int resize(draw, foo, data)
     DrawingJet draw;
     int foo;
     caddr_t data;
{
  int div_x, div_y;
  int i;

  div_x = (max_x / draw->core.width) + 1;
  div_y = (max_y / draw->core.height) + 1;

  div = (div_x > div_y) ? div_x : div_y;

  yleft = (draw->core.height - (max_y/div)) / 2;
  xleft = (draw->core.width - (max_x/div)) / 2;

  if (points2 == NULL)
    points2 = (XPoint *) malloc(num_points * sizeof(XPoint));

  for (i=0; i < num_points; i++)
    {
      points2[i].x = points[i].x / div + xleft;
      points2[i].y = points[i].y / div + yleft;
    }
  XClearWindow(XjDisplay(draw), XjWindow(draw));

  if (pixmap_on != NULL)
    XFreePixmap(XjDisplay(map), pixmap_on);
  if (pixmap_off != NULL)
    XFreePixmap(XjDisplay(map), pixmap_off);
  pixmap_on = XCreatePixmap(XjDisplay(map), 
			    XjWindow(map),
			    800/div+2, 800/div+2,
			    DefaultDepth(XjDisplay(map), 
					 DefaultScreen(XjDisplay(map))));
  pixmap_off = XCreatePixmap(XjDisplay(map), 
			     XjWindow(map),
			     800/div+2, 800/div+2,
			     DefaultDepth(XjDisplay(map), 
					  DefaultScreen(XjDisplay(map))));
  circled = NULL;

  return 0;
}



/* 
 *    draw_circle draws a circle around a cluster.
 */
void draw_circle(c)
     struct cluster *c;
{
  if (circled != NULL  &&  circled != c)
    {
      XCopyArea(XjDisplay(map), pixmap_off, XjWindow(map),
		((DrawingJet) map)->drawing.foreground_gc,
		0, 0, 800/div+2, 800/div+2,
		(circled->x_coord-400)/div+xleft-1,
		(circled->y_coord-400)/div+yleft-1);
    }

  if (c != NULL  &&  circled != c)
    {
      XCopyArea(XjDisplay(map), XjWindow(map), pixmap_off,
		((DrawingJet) map)->drawing.foreground_gc,
		(c->x_coord-400)/div + xleft - 1,
		(c->y_coord-400)/div + yleft - 1,
		800/div+2, 800/div+2,
		0, 0);
      XDrawArc(XjDisplay(map), XjWindow(map), map_gc,
	       (c->x_coord-400)/div + xleft,
	       (c->y_coord-400)/div + yleft,
	       800/div, 800/div,
	       0, 64 * 365);
      XCopyArea(XjDisplay(map), XjWindow(map), pixmap_on,
		((DrawingJet) map)->drawing.foreground_gc,
		(c->x_coord-400)/div + xleft - 1,
		(c->y_coord-400)/div + yleft - 1,
		800/div+2, 800/div+2,
		0, 0);
      /* There's only 360 degrees in a circle, I know, but for the */
      /* xterminal, we have to go a bit extra.  Sigh. */

      XFlush(XjDisplay(map));
    }
  circled = c;
}



int expos(draw, foo, data)
     DrawingJet draw;
     int foo;
     caddr_t data;
{
  int i,k;
  struct cluster *c;

  k=0;
  for (i=0; i < num_groups; i++)
    {
      XDrawLines(XjDisplay(draw), XjWindow(draw),
		 draw->drawing.foreground_gc,
		 points2+k,
		 num_segs[i], CoordModeOrigin);
      k += num_segs[i];
    }

/*
 *  Draw the cluster locations on the map pixmap.
 */

  if (map_gc == NULL)
    {
      unsigned long valuemask;
      XGCValues values;

      values.line_width = 2;
      values.cap_style = CapProjecting;
      valuemask = GCLineWidth | GCCapStyle;

      map_gc = XCreateGC(XjDisplay(draw), XjWindow(draw), valuemask, &values);

      valuemask = GCForeground | GCBackground | GCFunction | GCFont;
      XCopyGC(XjDisplay(draw), draw->drawing.foreground_gc,
	      valuemask, map_gc);
    }


  for (c = cluster_list; c != NULL; c = c->next)
    {
      XDrawLine(XjDisplay(draw), XjWindow(draw), map_gc,
                (c->x_coord-100)/div +xleft, (c->y_coord-100)/div +yleft,
                (c->x_coord+100)/div +xleft, (c->y_coord+100)/div +yleft);
      XDrawLine(XjDisplay(draw), XjWindow(draw), map_gc,
                (c->x_coord-100)/div +xleft, (c->y_coord+100)/div +yleft,
                (c->x_coord+100)/div +xleft, (c->y_coord-100)/div +yleft);
      if (c == Current)
	{
	  draw_circle(c);
	}
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


int quit(fromJet, what, data)
     caddr_t fromJet;
     int what;
     caddr_t data;
{
  exit(what);
  return 0;				/* For linting... */
}


fatal(display)
     Display *display;
{
  exit(-1);
}



void reset_timer()
{
  static int timerid=-1;

  if (parms.inactive != 0)
    {
      if (timerid != -1)
	(void) XjRemoveWakeup(timerid);
      timerid = XjAddWakeup(quit, 0, 1000 * parms.inactive);
    }
}


void set_curr(c)
     struct cluster *c;
{
  Current = c;

  for(c=cluster_list; c != NULL; c=c->next)
    if (c != Current)
      SetToggleState(c->btn, False, False);
    else
      SetToggleState(c->btn, True, False);

  XjCallCallbacks(text, ((DrawingJet) text)->drawing.exposeProc, NULL);
  draw_circle(Current);
  reset_timer();
}


int btn(me, curr, data)
     DrawingJet me;
     int curr;
     caddr_t data;
{
  int n;
  struct cluster *c;

  for(c=cluster_list, n=0; c != NULL  &&  n < curr; c=c->next, n++);
  set_curr(c);
  return 0;
}

int unset(me, curr, data)
     DrawingJet me;
     int curr;
     caddr_t data;
{
  SetToggleState(me, True, True);
  return(0);
}




#define SQUARE(a)  (a) * (a)	/* obvious... */


/*
 *  find_cluster finds the closest cluster to point (a,b) and returns it.
 */
struct cluster *find_cluster(a, b)
     int a, b;
{
  double closest_distance, distance;
  struct cluster *closest_cluster, *curr;

  closest_cluster = curr = cluster_list;
  closest_distance = SQUARE(curr->x_coord/div + xleft - a)
    + SQUARE(curr->y_coord/div + yleft - b);

  for(; curr != NULL; curr=curr->next)
    {
      distance = SQUARE(curr->x_coord/div + xleft - a)
	+ SQUARE(curr->y_coord/div + yleft - b);
      if (distance < closest_distance)
        {
          closest_distance = distance;
          closest_cluster = curr;
        }
    }
  return(closest_cluster);
}


int map_hit(me, curr, event)
     DrawingJet me;
     int curr;
     XEvent *event;
{
  struct cluster *c;

  if (event->type == ButtonPress)
    {
/*
      printf("%d %d\n",
	     (event->xbutton.x-xleft)*div,
	     (event->xbutton.y-yleft)*div);
*/
      c = find_cluster(event->xbutton.x, event->xbutton.y);
      set_curr(c);
    }
  return 0;
}


int text_hit(me, curr, event)
     DrawingJet me;
     int curr;
     XEvent *event;
{
  int n;
  struct cluster *c;
  extern struct cluster *find_text();

  if (event->type == ButtonPress)
    {
      c = find_text(event->xbutton.x, event->xbutton.y);
      set_curr(c);
    }
  return 0;
}


int state = 0;

int flash(fromJet, what, data)
     caddr_t fromJet;
     int what;
     caddr_t data;
{
  if (circled != NULL)
    {
      if (state)
	XCopyArea(XjDisplay(map), pixmap_on, XjWindow(map),
		  ((DrawingJet) map)->drawing.foreground_gc,
		  0, 0, 800/div+2, 800/div+2,
		  (circled->x_coord-400)/div+xleft-1,
		  (circled->y_coord-400)/div+yleft-1);
      else
	XCopyArea(XjDisplay(map), pixmap_off, XjWindow(map),
		  ((DrawingJet) map)->drawing.foreground_gc,
		  0, 0, 800/div+2, 800/div+2,
		  (circled->x_coord-400)/div+xleft-1,
		  (circled->y_coord-400)/div+yleft-1);
    }
  state = !state;
  (void) XjAddWakeup(flash, 0, 1000);
  return 0;
}



XjCallbackRec callbacks[] =
{
  { "createTree", createTree },
  { "mapTree", mapTree },
  { "unmapTree", unmapTree },
  { "quit", quit },
  { "resize", resize },
  { "expose", expos },
  { "check_cluster", check_cluster },
  { "button", btn },
  { "unset", unset },
  { "map_hit", map_hit },
  { "text_hit", text_hit },
};


void make_btns(Form)
     Jet Form;
{
  struct cluster *c;
  int n;
  Jet w, l;
  int first = 1;
  char prev[10];
  char buf[BUFSIZ];
  char buf1[10];

  for(c=cluster_list, n=0; c != NULL; c=c->next, n++)
    {
      sprintf(buf1, "button%d", n);
      w = XjVaCreateJet(buf1, windowJetClass, Form, NULL, NULL);
      c->btn = (ButtonJet) XjVaCreateJet("foo", buttonJetClass, w, NULL, NULL);
      l = XjVaCreateJet("bar", labelJetClass, c->btn,
			XjNlabel, c->button_name, NULL, NULL);

      if (first)
	{
	  first = 0;
	  sprintf(buf, " %s: 1 - - twin ", buf1);
	}
      else
	sprintf(buf, " %s: %s - - twin ", buf1, prev);

      strcpy(prev, buf1);
      strcat(form, buf);
    }
}


main(argc, argv)
int argc;
char **argv;
{
  Display *display;
  Jet xclusterWindow, xclusterForm, label1, mapwin;
  Jet qw, qb, ql,  hw, hb, hl;
  Jet twin;


  Current = cluster_list;

  (void)XSetIOErrorHandler(fatal);

  root = XjCreateRoot(&argc, argv, "Xcluster", NULL,
		      opTable, XjNumber(opTable));

  XjLoadFromResources(NULL,
		      NULL,
		      programName,
		      programClass,
		      appResources,
		      XjNumber(appResources),
		      (caddr_t) &parms);

  progname = programName;
  display = root->core.display;

  XjRegisterCallbacks(callbacks, XjNumber(callbacks));

  xclusterWindow = XjVaCreateJet("xclusterWindow",
				 windowJetClass, root, NULL, NULL);
  xclusterForm = XjVaCreateJet("xclusterForm",
			       formJetClass, xclusterWindow, NULL, NULL);
  label1 = XjVaCreateJet("label1", labelJetClass, xclusterForm, NULL, NULL);
  mapwin = XjVaCreateJet("mapwin", windowJetClass, xclusterForm, NULL, NULL);
  map = XjVaCreateJet("map", drawingJetClass, mapwin, NULL, NULL);

  qw = XjVaCreateJet("quit", windowJetClass, xclusterForm, NULL, NULL);
  qb = XjVaCreateJet("buttonQuit", buttonJetClass, qw, NULL, NULL);
  ql = XjVaCreateJet("quitLabel", labelJetClass, qb, NULL, NULL);

  hw = XjVaCreateJet("help", windowJetClass, xclusterForm, NULL, NULL);
  hb = XjVaCreateJet("buttonHelp", buttonJetClass, hw, NULL, NULL);
  hl = XjVaCreateJet("helpLabel", labelJetClass, hb, NULL, NULL);

  twin = XjVaCreateJet("twin", windowJetClass, xclusterForm, NULL, NULL);
  text = XjVaCreateJet("text", drawingJetClass, twin, NULL, NULL);

  strcpy(form,
	 "label1: 0 1 100 -    twin: 0 - 100 100    quit: - - 99 twin   \
	  help: - - quit twin  mapwin: 1 label1 99 quit");

  read_clusters();
  make_btns(xclusterForm);

  setForm(xclusterForm, form);

  XjRealizeJet(root);
  flash();
  reset_timer();

  XjEventLoop(root);
}
