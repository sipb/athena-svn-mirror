/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/xcluster.c,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/xcluster/xcluster.c,v 1.6 1994-05-08 23:27:39 cfields Exp $";
#endif	/* lint */

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
int lx = 32767, ly = 32767, hx = 0, hy = 0;
XPoint *points2 = NULL;
char *progname;
char form[BUFSIZ];
struct cluster *Current;

int set_auto();



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
{"-auto",	".auto",	XrmoptionSepArg,	(caddr_t) 60},
{"-circlecolor",".circlecolor", XrmoptionSepArg,	(caddr_t) NULL},
{"-zoom",       ".zoom",        XrmoptionNoArg,         (caddr_t) "on"},
{"-color0",	".color0",	XrmoptionSepArg,	(caddr_t) NULL},
{"-color1",	".color1",	XrmoptionSepArg,	(caddr_t) NULL},
{"-color2",	".color2",	XrmoptionSepArg,	(caddr_t) NULL},
{"-changetime",	".changetime",	XrmoptionSepArg,	(caddr_t) NULL}, /* s*/
{"-steps",	".steps",	XrmoptionSepArg,	(caddr_t) NULL},
{"-steptime",	".steptime",	XrmoptionSepArg,	(caddr_t) NULL}, /*ms*/
};

typedef struct _MyResources
{
  int inactive;
  int automatic;
  int ccolor;
  Boolean zoom;
  char *colors[3];
  int changetime;
  int steps, steptime;
} MyResources;

typedef struct _MyResources *MyResourcesPtr;

MyResources parms;

#define offset(field) XjOffset(MyResourcesPtr,field)

static XjResource appResources[] =
{
  { "inactive", "Inactive", XjRInt, sizeof(int),
      offset(inactive), XjRInt, (caddr_t) 0 },
  { "auto", "Auto", XjRInt, sizeof(int),
      offset(automatic), XjRInt, (caddr_t) 0 },
  { "circlecolor", XjCForeground, XjRColor, sizeof(int),
      offset(ccolor), XjRString, XjDefaultForeground },
  { "zoom", "Zoom", XjRBoolean, sizeof(Boolean),
      offset(zoom), XjRBoolean, (caddr_t) 0 },
  { "color0", "Color", XjRString, sizeof(char *),
      offset(colors[0]), XjRString, (caddr_t) "red green blue" },
  { "color1", "Color", XjRString, sizeof(char *),
      offset(colors[1]), XjRString, (caddr_t) "green blue red" },
  { "color2", "Color", XjRString, sizeof(char *),
      offset(colors[2]), XjRString, (caddr_t) "blue red green" },
  { "changetime", "Changetime", XjRInt, sizeof(int),
      offset(changetime), XjRInt, (caddr_t) 3600 },
  { "steps", "Steps", XjRInt, sizeof(int),
      offset(steps), XjRInt, (caddr_t) 256 },
  { "steptime", "Steptime", XjRInt, sizeof(int),
      offset(steptime), XjRInt, (caddr_t) 50 },
};

#undef offset

int div;
int xleft = 0, yleft = 0;
struct cluster *circled = NULL;
Pixmap pixmap_on = (Pixmap) NULL, pixmap_off = (Pixmap) NULL;

int resize(draw, foo, data)
     DrawingJet draw;
     int foo;
     caddr_t data;
{
  int div_x, div_y;
  int i;
  int width, height;

  if (parms.zoom)
    {
      width = hx - lx + 5000;
      height = hy - ly + 5000;
    }
  else
    {
      width = max_x;
      height = max_y;
    }

  div_x = (width / draw->core.width) + 1;
  div_y = (height / draw->core.height) + 1;

  div = (div_x > div_y) ? div_x : div_y;

  if (parms.zoom)
    {
      if (div == div_y)
	{
	  yleft = (2500-ly)/div;
	  xleft = -lx/div + ((draw->core.width - (hx - lx)/div) / 2);
	}
      else
	{
	  xleft = (2500-lx)/div;
	  yleft = -ly/div + ((draw->core.height - (hy - ly)/div) / 2);
	}
    }
  else
    {
      yleft = (draw->core.height - (max_y/div)) / 2;
      xleft = (draw->core.width - (max_x/div)) / 2;
    }

  if (points2 == NULL)
    points2 = (XPoint *) malloc(num_points * sizeof(XPoint));

  for (i=0; i < num_points; i++)
    {
      points2[i].x = points[i].x / div + xleft;
      points2[i].y = points[i].y / div + yleft;
    }
  XClearWindow(XjDisplay(draw), XjWindow(draw));

  if (pixmap_on != (Pixmap) NULL)
    XFreePixmap(XjDisplay(map), pixmap_on);
  if (pixmap_off != (Pixmap) NULL)
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
  if (circled != NULL  &&  circled != c  &&  map_gc != NULL)
    {
      XCopyArea(XjDisplay(map), pixmap_off, XjWindow(map),
		((DrawingJet) map)->drawing.foreground_gc,
		0, 0, 800/div+2, 800/div+2,
		(circled->x_coord-400)/div+xleft-1,
		(circled->y_coord-400)/div+yleft-1);
    }

  if (c != NULL  &&  circled != c  &&  map_gc != NULL)
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

  if (map_gc != NULL)
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
      values.foreground = parms.ccolor;
      valuemask = GCForeground | GCLineWidth | GCCapStyle;

      map_gc = XCreateGC(XjDisplay(draw), XjWindow(draw), valuemask, &values);

      valuemask = GCBackground | GCFunction | GCFont;
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


int timeout(data, id)
     int data, id;
{
  exit(0);
}

void reset_timer()
{
  static int timerid = -1;

  if (parms.inactive != 0)
    {
      if (timerid != -1)
	(void) XjRemoveWakeup(timerid);
      timerid = XjAddWakeup(timeout, 0, 1000 * parms.inactive);
    }
}


void set_curr(c, id)
     struct cluster *c;
     int id;
{
  static int timerid = -1;
  Current = c;

  for(c=cluster_list; c != NULL; c=c->next)
    if (c != Current)
      SetToggleState(c->btn, False, False);
    else
      SetToggleState(c->btn, True, False);

  XjCallCallbacks(text, ((DrawingJet) text)->drawing.exposeProc, NULL);
  draw_circle(Current);
  reset_timer();
  if (parms.automatic != 0)
    {
      if (timerid != -1  &&  timerid != id)
	(void) XjRemoveWakeup(timerid);
      timerid = XjAddWakeup(set_auto, 0, 1000 * parms.automatic);
    }
}


int set_auto(data, id)
     int data, id;
{
  static struct cluster *automatic = NULL;

  if (automatic == NULL)
    automatic = cluster_list;

  set_curr(automatic, id);
  automatic = Current->next;
  return 0;
}


int btn(me, curr, data)
     DrawingJet me;
     int curr;
     caddr_t data;
{
  int n;
  struct cluster *c;

  for(c=cluster_list, n=0; c != NULL  &&  n < curr; c=c->next, n++);
  set_curr(c, -1);
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
      c = find_cluster(event->xbutton.x, event->xbutton.y);
      set_curr(c, -1);
    }
  return 0;
}


int text_resize(draw, foo, data)
     DrawingJet draw;
     int foo;
     caddr_t data;
{
  XClearWindow(XjDisplay(draw), XjWindow(draw));
}


int text_hit(me, curr, event)
     DrawingJet me;
     int curr;
     XEvent *event;
{
  struct cluster *c;
  extern struct cluster *find_text();

  if (event->type == ButtonPress)
    {
      c = find_text(event->xbutton.x, event->xbutton.y);
      set_curr(c, -1);
    }
  return 0;
}


int state = 0;

int flash(data, id)
     int data, id;
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
  { "text_resize", text_resize },
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

compute_zoom()
{
  struct cluster *c;

  for(c=cluster_list; c != NULL; c=c->next)
    {
      lx = MIN(c->x_coord, lx);
      ly = MIN(c->y_coord, ly);
      hx = MAX(c->x_coord, hx);
      hy = MAX(c->y_coord, hy);
    }
}

#define MAXCOLORS 3
#define MAXCYCLES 15
XColor cycles[MAXCOLORS][MAXCYCLES];
int cycle, subcycle, numCycles;

void nextcycle()
{
  int i;
  XColor col;

  subcycle++;
  if (subcycle >= parms.steps)
    {
      subcycle = 0;
      cycle++;
      if (cycle == numCycles)
	cycle = 0;
    }

  if (parms.changetime != 0 && subcycle == 0)
    (void)XjAddWakeup(nextcycle, 0, parms.changetime * 1000);
  else
    (void)XjAddWakeup(nextcycle, 0, parms.steptime);

  for (i = 0; i < MAXCOLORS; i++)
    if (cycles[i][0].pixel != XjNoColor)
      {
	col.pixel = cycles[i][0].pixel;
	col.red = cycles[i][cycle].red +
	  (subcycle * (int)( cycles[i][(cycle + 1)%numCycles].red -
		       cycles[i][cycle].red ) / parms.steps);
	col.green = cycles[i][cycle].green +
	  (subcycle * (int)( cycles[i][(cycle + 1)%numCycles].green -
		       cycles[i][cycle].green ) / parms.steps);
	col.blue = cycles[i][cycle].blue +
	  (subcycle * (int)( cycles[i][(cycle + 1)%numCycles].blue -
		       cycles[i][cycle].blue ) / parms.steps);
	col.flags = DoRed | DoGreen | DoBlue;

	XStoreColors(XjDisplay(root), XjColormap(root), &col, 1);
      }
}

void init_color_cycle()
{
  int i, j, docycles;
  char name[50], *dest, *src;
  char errtext[100];

  docycles = 0;
  numCycles = 0;

  for (i = 0; i < MAXCOLORS; i++)
    for (j = 0; j < MAXCYCLES; j++)
      cycles[i][j].pixel = XjNoColor;

  for (i = 0; i < MAXCOLORS; i++)
    if (XjGetColor(i) != XjNoColor) /* a used color */
      {
	docycles = 1;
	src = parms.colors[i];

	for (j = 0; j < MAXCYCLES; j++)
	  {
	    while (isspace(*src))
	      src++;

	    if (*src == '\0')
	      {
		if (j == 0)
		  {
		    sprintf(errtext, "bad specification for color%d", i);
		    XjWarning(errtext);
		    src = "red";
		  }
		else
		  break;
	      }

	    dest = name;
	    while (!isspace(*src) && *src != '\0')
	      *dest++ = *src++; /* XXX no error checking for > 50 */
	    *dest = '\0';

	    if (!XParseColor(XjDisplay(root), XjColormap(root),
			     name, &cycles[i][j]))
	      {
		sprintf(errtext, "could not look up color \"%s\"", name);
		XjWarning(errtext);
		cycles[i][j].red = 256*255;
		cycles[i][j].green = 0;
		cycles[i][j].blue = 0;
		cycles[i][j].flags = DoRed | DoGreen | DoBlue;
	      }
	    cycles[i][j].pixel = XjGetColor(i);
	  }

	numCycles = MAX(numCycles, j);
      }

  if (docycles == 0) /* we're not using this feature */
    return;

  for (i = 0; i < MAXCOLORS; i++)
    for (j = 1; j < numCycles; j++)
      if (cycles[i][j].pixel == XjNoColor)
	cycles[i][j] = cycles[i][j-1];

  cycle = 0;
  subcycle = 0;

  for (i = 0; i < MAXCOLORS; i++)
    if (cycles[i][0].pixel != XjNoColor)
      XStoreColors(XjDisplay(root), XjColormap(root), &cycles[i][cycle], 1);

  if (parms.changetime != 0)
    (void)XjAddWakeup(nextcycle, 0, parms.changetime * 1000);
  else
    (void)XjAddWakeup(nextcycle, 0, parms.steptime);
}

main(argc, argv)
int argc;
char **argv;
{
  Jet xclusterWindow, xclusterForm, label1, mapwin;
  Jet qw, qb, ql,  hw, hb, hl;
  Jet twin;


  Current = cluster_list;

  (void)XSetIOErrorHandler(fatal);

  root = XjCreateRoot(&argc, argv, "Xcluster", NULL,
		      opTable, XjNumber(opTable));

  XjLoadFromResources(XjDisplay(root),
		      XjWindow(root),
		      programName,
		      programClass,
		      appResources,
		      XjNumber(appResources),
		      (caddr_t) &parms);

  progname = programName;

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

  if (parms.zoom == 1)
    compute_zoom();

  make_btns(xclusterForm);

  setForm(xclusterForm, form);

  XjRealizeJet(root);
  flash(NULL, NULL);
  reset_timer();

  if (parms.automatic != 0)
    set_auto(0, -1);

  init_color_cycle();

  XjEventLoop(root);
}
