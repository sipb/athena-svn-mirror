 
#ifndef lint
static char sccsid[] = "@(#)xmlock.c  3.12 96/11/05 xlockmore";
 
#endif
 
/*-
 * xmlock.c - Motif GUI Launcher for XLock
 *          Charles Vidal <vidalc@univ-mlv.fr>
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * Nov-96: Continual minor improvements by David Bagley.
 * Oct-96: written.
 */

/*-
  XmLock Problems
  1. Allowing only one in -inroot.  Need a way to kill it.
  2. XLock resources need to be read and used to set initial values.
  3. Integer and floating point and string input.
 */

/* COMPILE  cc xmlock.c -lXm -lXt -lX11 -o xmlock */

#include <stdio.h>
#include <stdlib.h>
#ifdef VMS
#include <descrip.h>
#endif

/*#include <Xm/XmAll.h> Does not work on my version of Lesstif */
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>

#if HAS_XMU
#include <X11/Xmu/Editres.h>
#endif

#include "bitmaps/m-xlock.xbm"

#define  LAUNCH	0
#define  ROOT   1
#define  WINDOW 2
#define  EXIT   3
#define  PUSHBUTTONS  4
#define  TOGGLES  9

/* Widget */

static	Widget      toplevel, ScrolledListModes,
                PushButtons[PUSHBUTTONS], Toggles[TOGGLES];

 /*Resource string */

typedef struct LockStruct_s {
  char       *cmdline_arg;  /* mode name */
  int         def_delay;  /* default delay for mode */
  int         def_batchcount;
  int         def_cycles;
  int         def_size;
  float       def_saturation;
  char       *desc; /* text description of mode */
} LockStruct;

static LockStruct  LockProcs[] =
{
  {"ant", 1000, -3, 40000, -7, 1.0,
   "Shows Langton's and Turk's generalized ants"},
  {"bat", 100000, -8, 20, 0, 1.0,
   "Shows bouncing flying bats"},
  {"blot", 100000, 6, 30, 0, 0.4,
   "Shows Rorschach's ink blot test"},
  {"bouboule", 10000, 100, 1, 15, 1.0,
   "Shows Mimi's bouboule of moving stars"},
  {"bounce", 10000, -10, 20, 0, 1.0,
   "Shows bouncing footballs"},
  {"braid", 1000, 15, 100, 0, 1.0,
   "Shows random braids and knots"},
  {"bug", 75000, 10, 32767, -4, 1.0,
   "Shows Palmiter's bug evolution and garden of Eden"},
  {"clock", 100000, -16, 200, -200, 1.0,
   "Shows Packard's clock"},
  {"daisy", 100000, 300, 350, 0, 1.0,
   "Shows a meadow of daisies"},
  {"dclock", 10000, 20, 10000, 0, 0.2,
   "Shows a floating digital clock"},
  {"demon", 50000, 16, 1000, -7, 1.0,
   "Shows Griffeath's cellular automata"},
  {"eyes", 20000, -8, 5, 0, 1.0,
   "Shows eyes following a bouncing grelb"},
  {"flag", 50000, 1, 1000, -7, 1.0,
   "Shows a flying flag of your operating system"},
  {"flame", 750000, 20, 10000, 0, 1.0,
   "Shows cosmic flame fractals"},
  {"forest", 400000, 100, 200, 0, 1.0,
   "Shows binary trees of a fractal forest"},
  {"galaxy", 100, -5, 250, 1, 1.0,
   "Shows crashing spiral galaxies"},
#ifdef HAS_GL
  {"gear", 1, 1, 1, 0, 1.0,
   "Shows GL's gears"},
#endif
  {"geometry", 10000, -10, 20, 0, 1.0,
   "Shows morphing of a complete graph"},
  {"grav", 10000, -12, 20, 0, 1.0,
   "Shows orbiting planets"},
  {"helix", 25000, 1, 100, 0, 1.0,
   "Shows string art"},
  {"hop", 10000, 1000, 2500, 0, 1.0,
   "Shows real plane iterated fractals"},
  {"hyper", 10000, 1, 300, 0, 1.0,
   "Shows a spinning tesseract in 4D space"},
  {"image", 2000000, -10, 20, 0, 1.0,
   "Shows randomly appearing logos"},
  {"kaleid", 20000, 4, 700, 0, 1.0,
   "Shows a kaleidoscope"},
  {"laser", 20000, -10, 200, 0, 1.0,
   "Shows spinning lasers"},
  {"life", 750000, 40, 140, 0, 1.0,
   "Shows Conway's game of Life"},
  {"life1d", 10000, 10, 10, 0, 1.0,
   "Shows Wolfram's game of 1D Life"},
  {"life3d", 1000000, 35, 85, 16, 1.0,
   "Shows Bays' game of 3D Life"},
  {"lightning", 10000, 1, 1, 0, 0.6,
   "Shows Keith's fractal lightning bolts"},
  {"lissie", 10000, 1, 2000, 0, 0.6,
   "Shows lissajous worms"},
  {"marquee", 100000, 10, 20, 0, 1.0,
   "Shows messages"},
  {"maze", 1000, -40, 300, 0, 1.0,
   "Shows a random maze and a depth first search solution"},
  {"mountain", 1000, 30, 100, 0, 1.0,
   "Shows Papo's mountain range"},
  {"nose", 100000, 10, 20, 0, 1.0,
   "Shows a man with a big nose runs around spewing out messages"},
  {"penrose", 10000, 1, 1, -40, 1.0,
   "Shows Penrose's quasiperiodic tilings"},
  {"petal", 10000, -500, 400, 0, 1.0,
   "Shows various GCD Flowers"},
  {"puzzle", 10000, 250, 100, 0, 1.0,
   "Shows a puzzle being scrambled and then solved"},
  {"pyro", 15000, 40, 75, 0, 1.0,
   "Shows fireworks"},
  {"pop", 15000, 40, 75, 0, 1.0,
   "Shows fireworks"},
  {"qix", 30000, 100, 64, 0, 1.0,
   "Shows spinning lines a la Qix(tm)"},
  {"rotor", 10000, 4, 20, 0, 0.4,
   "Shows Tom's Roto-Rooter"},
  {"shape", 10000, 100, 256, 0, 1.0,
   "Shows stippled rectangles, ellipses, and triangles"},
  {"slip", 50000, 35, 50, 0, 1.0,
   "Shows slipping blits"},
  {"sphere", 10000, 1, 20, 0, 1.0,
   "Shows a bunch of shaded spheres"},
  {"spiral", 5000, -40, 350, 0, 1.0,
   "Shows helixes of dots"},
  {"spline", 30000, -6, 2048, 0, 0.4,
   "Shows colorful moving splines"},
  {"star", 40000, 100, 1, 100, 0.2,
   "Shows a star field with a twist"},
  {"swarm", 10000, 100, 20, 0, 1.0,
   "Shows a swarm of bees following a wasp"},
  {"swirl", 5000, 5, 20, 0, 1.0,
   "Shows animated swirling patterns"},
  {"tri", 10000, 2000, 100, 0, 1.0,
   "Shows a Sierpinski's triangle"},
  {"triangle", 10000, 100, 200, 0, 1.0,
   "Shows a triangle mountain range"},
  {"wator", 750000, 4, 32767, 0, 1.0,
   "Shows Dewdney's Water-Torus planet of fish and sharks"},
  {"wire", 500000, 1000, 150, -8, 1.0,
   "Shows a random circuit with 2 electrons"},
  {"world", 100000, -16, 20, 0, 0.3,
   "Shows spinning Earths"},
  {"worm", 17000, -20, 10, -3, 1.0,
   "Shows wiggly worms"},
#ifdef USE_HACKERS
  {"ball", 10000, 10, 20, 64, 1.0,
   "Shows bouncing balls"},
#if defined( HAS_XPM ) || defined( HAS_XPMINC )
  {"cartoon", 1000, 30, 50, 0, 1.0,
   "Shows bouncing cartoons"},
#endif
#ifdef DRIFT
  {"drift", 100, 3, 10000, 0, 1.0,
   "Shows new cosmic drift fractals"},
#else
  {"flamen", 100, 3, 10000, 0, 1.0,
   "Shows new cosmic flame fractals"},
#endif
  {"huskers", 2000000, 1, 20, 0, 1.0,
   "Shows the Huskers (American) Football icon"},
  {"julia", 10000, 1000, 2500, 0, 1.0,
   "Shows the Julia set"},
  {"pacman", 50000, 10, 200, 0, 1.0,
   "Shows Pacman(tm)"},
  {"polygon", 750000, 40, 140, 0, 1.0,
   "Shows a polygon"},
  {"roll", 100000, 25, 20, 64, 1.0,
   "Shows a rolling ball"},
  {"turtle", 500000, 6, 20, 0, 1.0,
   "Shows turtle fractals"},
#endif
  {"blank", 3000000, 1, 20, 0, 1.0,
   "Shows nothing but a black screen"},
#ifdef USE_BOMB
  {"bomb", 100000, 10, 20, 0, 1.0,
   "Shows a bomb and will autologout after a time"},
#endif
  {"random", 1, 0, 0, 0, 1.0,
   "Shows a random mode from above except blank (and bomb)"}
};
 
static int         numprocs = sizeof (LockProcs) / sizeof (LockProcs[0]);

static char       *r_PushButtons[PUSHBUTTONS] =
{"Launch", "In Root", "In Window", "Exit"};
static char       *r_Toggles[TOGGLES] =
{"mono", "nolock", "remote", "allowroot", "enablesaver",
   "allowaccess", "grabmouse", "echokeys", "usefirst"};

static int         numberinlist = 0;

/* CallBack */
static void
f_PushButtons(Widget w, XtPointer client_data, XtPointer call_data)
{
	int         i;
	char        command[500];
#ifdef VMS
  int mask = 17;
  struct dsc$descriptor_d vms_image;
#endif

	(void) strcpy(command, "xlock ");
	for (i = 0; i < TOGGLES; i++) {
		if (XmToggleButtonGetState(Toggles[i])) {
			(void) strcat(command, "-");
			(void) strcat(command, r_Toggles[i]);
			(void) strcat(command, " ");
		}
	}
	switch ((int) client_data) {
		case LAUNCH:
			/* the default value then nothing to do */
			break;
		case WINDOW:
			(void) strcat(command, "-inwindow ");
			break;
		case ROOT:
			(void) strcat(command, "-inroot ");
			break;
		case EXIT:
			exit(0);
			break;
	}
	(void) strcat(command, "-mode ");
	(void) strcat(command, LockProcs[numberinlist].cmdline_arg);
#ifdef VMS
          vms_image.dsc$w_length = strlen(command);
    vms_image.dsc$a_pointer = command;
          vms_image.dsc$b_class = DSC$K_CLASS_S;
         vms_image.dsc$b_dtype = DSC$K_DTYPE_T;
	(void) printf("%s\n", command);
  (void) lib$spawn(&vms_image, 0, 0, &mask);
#else
	(void) strcat(command, " & ");
	(void) printf("%s\n", command);
  (void) system(command);
#endif
}

static void 
f_ScrolledListModes(Widget w, XtPointer client_data, XtPointer call_data)
{
	numberinlist = ((XmListCallbackStruct *) call_data)->item_position - 1;
}

/* Setup Widget */
static void 
Setup_Widget(Widget father)
{
	Arg         args[15];
	int         i, ac = 0;
	Widget      row, PushButtonRow, TogglesRow;
  char        string[160];
	XmString    label_str;
#define NUMPROCS 100  /* Greater than or equal to numprocs */
	XmString    TabXmStr[NUMPROCS];

	ac = 0;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	row = XmCreateRowColumn(father, "row", args, ac);

	for (i = 0; i < numprocs; i++) {
    (void) sprintf(string, "%-10s%s", LockProcs[i].cmdline_arg,
       LockProcs[i].desc);
		TabXmStr[i] = XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);
  }
	ac = 0;
	XtSetArg(args[ac], XmNitems, TabXmStr);
	ac++;
	XtSetArg(args[ac], XmNitemCount, numprocs);
	ac++;
	XtSetArg(args[ac], XmNvisibleItemCount, 10);
	ac++;
	ScrolledListModes = XmCreateScrolledList(row, "ScrolledListModes",
    args, ac);
	XtAddCallback(ScrolledListModes, XmNbrowseSelectionCallback,
    f_ScrolledListModes, NULL);
	XtManageChild(ScrolledListModes);
	TogglesRow = XmCreateRowColumn(row, "TogglesRow", NULL, 0);
	for (i = 0; i < TOGGLES; i++) {
		ac = 0;
		label_str = XmStringCreate(r_Toggles[i], XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, label_str);
		ac++;
		Toggles[i] = XmCreateToggleButton(TogglesRow, "Check", args, ac);
		XtManageChild(Toggles[i]);
	}
	XtManageChild(TogglesRow);

	XtManageChild(row);
	ac = 0;
	XtSetArg(args[ac], XmNorientation, XmHORIZONTAL);
	ac++;
	XtSetArg(args[ac], XmNrightAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNleftAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNbottomAttachment, XmATTACH_FORM);
	ac++;
	XtSetArg(args[ac], XmNtopAttachment, XmATTACH_WIDGET);
	ac++;
	XtSetArg(args[ac], XmNtopWidget, row);
	ac++;
	PushButtonRow = XmCreateRowColumn(father, "PushButtonRow", args, ac);
	for (i = 0; i < PUSHBUTTONS; i++) {
		ac = 0;
		label_str = XmStringCreate(r_PushButtons[i], XmSTRING_DEFAULT_CHARSET);
		XtSetArg(args[ac], XmNlabelString, label_str);
		ac++;
		PushButtons[i] = XmCreatePushButton(PushButtonRow, "PushButtons",
       args, ac);
		XtAddCallback(PushButtons[i], XmNactivateCallback, f_PushButtons,
      (XtPointer) i);
		XtManageChild(PushButtons[i]);
	}
	XtManageChild(PushButtonRow);
}

int
main(int argc, char **argv)
{
	Widget      form;
	Arg         args[15];

	toplevel = XtInitialize(argv[0], "test", NULL, 0, &argc, argv);
  XtSetArg(args[0], XtNiconPixmap,
     XCreateBitmapFromData(XtDisplay(toplevel),
               RootWindowOfScreen(XtScreen(toplevel)),
          (char *) image_bits, image_width, image_height));
  XtSetValues(toplevel, args, 1);
  /* creation Widget */
	form = XmCreateForm(toplevel, "form", NULL, 0);
	Setup_Widget(form);
	XtManageChild(form);
	XtRealizeWidget(toplevel);
#if HAS_XMU
  XtAddEventHandler(toplevel, (EventMask) 0, TRUE,
     (XtEventHandler) _XEditResCheckMessages, NULL);
#endif
	XtMainLoop();
#ifdef VMS
  return 1;
#else
  return 0;
#endif
}
