/*
Copyright 1991 by the Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#include	<stdio.h>
#include	<X11/IntrinsicP.h>
#include	<X11/StringDefs.h>
#include	<X11/CoreP.h>

#include        <X11/Xaw/MenuButton.h>
#include        <X11/Xaw/SimpleMenu.h>
#include        <X11/Xaw/Sme.h>
#include        <X11/Xaw/SmeBSB.h>
#include        <X11/Xaw/Cardinals.h>

#include	<X11/Xaw/List.h>
#include	<X11/Xaw/SimpleMenP.h>
#include	<X11/Xaw/Paned.h>
#include	<X11/Xaw/Command.h>
#include	<X11/Xaw/Box.h>
#include	<X11/Xaw/AsciiText.h>
#include	<X11/Xaw/TextP.h>
#include	<X11/Xaw/TextSinkP.h>
#include	<X11/Xaw/Dialog.h>
#include	<X11/Xaw/Form.h>
#include	<X11/Xaw/Label.h>
#include	"xdsc.h"

static char rcsid[] = "";

/*
** Globals
*/

char		*RunCommand();
TextWidget	toptextW, bottextW;
Boolean		debug = False;
char		filebase[50];
int		topscreen = MAIN;
Widget		topW, paneW;
void		RemoveLetterC();
int		char_width;
char		axis[10];
void		TopSelect(), BotSelect();
void		Update(), Stub();
void		PrintEvent();
int		simplemode = 0;
int		edscversion;
Boolean		nocache;

/*
** External functions
*/

extern void	PutUpHelp();
extern void	SubmitTransaction();
extern void	WriteTransaction();
extern char	*strchr();
extern char	*tempnam();
extern int	PopdownCB();
extern char     *getenv();
extern void	TriggerAdd(), TriggerNum(), TriggerDelete();
extern void	TriggerWrite(), TriggerPopdown(), TriggerSend();
extern void	TriggerFocusMove();
extern void	DispatchClick();
extern void	FetchIfNecessary();
extern int	HighlightedTransaction();

/*
** Private functions
*/

static void	MenuCallback();
static void	KeyCallback();
static void	QuitCB(), HelpCB();
static void	BuildUserInterface();
static void	DoTheRightThing();
static void	DoTheRightThingInReverse();
static void	CheckEdscVersion();
static void	BuildSkeleton();

/*
** Private globals
*/

static int	filedesparent[2], filedeschild[2];
static FILE	*inputfile, *outputfile;
static char	*meetinglist;

static Widget	topboxW, botboxW;
static Widget	label1W;
static int	prevfirst = 0;
static XawTextPosition	startOfCurrentMeeting = -1;
Display         *dpy;
Window          root_window;
static XawTextPosition	oldarrow = -1;

/*
** Data for top row of buttons
*/

static char * menu_labels0[MAX_BUTTONS] = {
        "Down", "Up", "update", "configure", "mode",
        "show", "HELP", "QUIT"};

static char * menu_names0[MAX_BUTTONS] = {
        "downbutton", "upbutton", "updatebutton", "configurebutton", 
	"modebutton", "showbutton", "helpbutton", "quitbutton"};

static char * submenu_labels0[MAX_BUTTONS][MAX_MENU_LEN] = {
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { "add meeting", "delete meeting", NULL, NULL },
        { "transactions", "meetings", NULL, NULL },
        { "unread", "all", "back ten", NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL }};

static char * submenu_names0[MAX_BUTTONS][MAX_MENU_LEN] = {
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { "addbutton", "deletebutton", NULL, NULL },
        { "transbutton", "meetingbutton", NULL, NULL },
        { "unreadbutton", "allbutton", "backbutton", NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL }};

/*
** Data for bottom row of buttons
*/

static char * menu_labels1[MAX_BUTTONS] = {
        "next", "prev", "Next in chain", "Prev in chain",
        "goto", "enter", "write", NULL };

static char * menu_names1[MAX_BUTTONS] = {
        "nextbutton", "prevbutton", "nchainbutton", "pchainbutton",
        "gotobutton", "enterbutton", "writebutton", NULL };

static char * submenu_labels1[MAX_BUTTONS][MAX_MENU_LEN] = {
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { "number", "first", "last", NULL },
        { "reply", "new transaction", NULL },
        { "write to file", "mail to someone", NULL }};

static char * submenu_names1[MAX_BUTTONS][MAX_MENU_LEN] = {
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { NULL, NULL, NULL, NULL },
        { "numberbutton", "firstbutton", "lastbutton", NULL },
        { "replybutton", "newbutton", NULL },
        { "writebutton", "mailbutton", NULL }};

typedef struct{
	Boolean logging_on;
	String	log_file;
} defaults;

static defaults	defs;

static XtResource app_resources[] = {
	{ "loggingOn", "LoggingOn",
		XtRBoolean, sizeof (Boolean), XtOffset (defaults *, logging_on),
		XtRString, "true" },
	{ "logfile", "Logfile",
		XtRString, sizeof (String), XtOffset (defaults *, log_file),
		XtRString, "/afs/athena.mit.edu/user/s/sao/scores/xdsc.log"},
};


EntryRec        toplevelbuttons[2][MAX_BUTTONS];

void
main(argc, argv)
int argc;
char *argv[];
{
	int	pid;
	char	*oldpath, *newpath, *myname;
	Arg	args[1];
	int	width;
	char	commandline[100];

	if (argc > 1 && !strcmp(argv[1], "-debug"))
		debug = True;

	myname = (myname = rindex(argv[0], '/')) ? myname + 1 : argv[0];

	if (!strcmp (myname, "lucy"))
		simplemode = True;
	else
		simplemode = False;

	if (simplemode) {
		topW = XtInitialize("topwidget", "Lucy", NULL, 0, &argc, argv);
	}
	else
		topW = XtInitialize("topwidget", "Xdsc", NULL, 0, &argc, argv);

	BuildSkeleton();

/*
** Set our width to 80 chars wide in the current font.  Min value of 500
** means that all the lower buttons will fit.
*/
	char_width = (((TextSinkObject) (toptextW->text.sink))->
				text_sink.font->max_bounds.width);

	width = 80 * char_width;
	XtSetArg(args[0], XtNwidth, width < 500 ? 500 : width);
	XtSetValues(topW, args, 1);

	XtRealizeWidget(topW);
	XSync(XtDisplay(topW), False);

	if (debug)
		fprintf (stderr, "Debugging is on\n");

	sprintf (filebase,"/usr/tmp/xdsc%d",getpid());

	if (debug)
		fprintf (stderr, "filebase is %s\n", filebase);

	pipe (filedesparent);
	pipe (filedeschild);

	pid = vfork();

	if (pid == 0)
		SetUpEdsc();

	close (filedesparent[1]);
	close (filedeschild[0]);

	inputfile = fdopen (filedesparent[0], "r");
	outputfile = fdopen (filedeschild[1], "w");

	CheckEdscVersion();
	ParseMeetingsFile();

	oldpath = getenv("XFILESEARCHPATH");



#define	AppdefaultsInStafftools	0

#if AppdefaultsInStafftools
#if defined(mips) || defined(_AIX)
	if (!oldpath) {
		newpath = (char *) malloc (100);
		strcpy (newpath, "XFILESEARCHPATH=/mit/StaffTools/lib/X11/app-defaults/%N");
	}
	else {
		newpath = (char *) malloc (100 + strlen (oldpath));
		sprintf (newpath, "XFILESEARCHPATH=%s:/mit/StaffTools/lib/X11/app-defaults/%%N",oldpath);
	}
	putenv (newpath);
#else
	if (!oldpath) {
		newpath = (char *) malloc (50);
		strcpy (newpath, "/mit/StaffTools/lib/X11/app-defaults/%N");
	}
	else {
		newpath = (char *) malloc (50 + strlen (oldpath));
		sprintf (newpath, "%s:/mit/StaffTools/lib/X11/app-defaults/%%N",oldpath);
	}
	setenv ("XFILESEARCHPATH",newpath,1);

#endif
#endif


	myfree (newpath);

	XtGetApplicationResources(	topW, (XtPointer) &defs,
					app_resources, XtNumber (app_resources),
					NULL, 0);

	if (defs.logging_on) {
		sprintf (commandline, "machtype >> %s", defs.log_file);
		system (commandline);

		sprintf (commandline, "date >> %s", defs.log_file);
		system (commandline);
	}

	BuildUserInterface();
	dpy = XtDisplay(topW);
	root_window = XtWindow(topW);

	(void) MoveToMeeting(INITIALIZE);
/*
** If running in simplemode, switch to reading transactions and put up
** the list of unread ones.
*/
	if (simplemode) {
		TopSelect (NULL, 4, NULL);
		BotSelect (NULL, 0, NULL);
	}
	CheckButtonSensitivity(BUTTONS_UPDATE);
	XtMainLoop();
}

SetUpEdsc()
{
	int	retval;
	char	commandtorun[50];
	char	machtype[20];
	char	*envcommand;
	
	envcommand = getenv("EDSC");

	if (envcommand) {
		strcpy (commandtorun, envcommand);
	}
	else {

#if defined (mips) && defined (ultrix)
		strcpy (machtype, "decmips");
#else
#ifdef vax
		strcpy (machtype, "vax");
#else
#ifdef ibm032
		strcpy (machtype, "rt");
#else
#ifdef _IBMR2
		strcpy (machtype, "rsaix");
#else
#ifdef _AUX_SOURCE
		strcpy (machtype, "mac");
#else
#ifdef sparc
		strcpy (machtype, "sparc");
#else
		Need to define for this machine
#endif
#endif
#endif
#endif
#endif
#endif
#ifndef EDSC_PATH
#define EDSC_PATH "/mit/StaffTools/%sbin/edsc"
#endif
		sprintf (	commandtorun, 
				EDSC_PATH,
				machtype);
	}

	close (filedesparent[0]);
	close (filedeschild[1]);

	dup2 (filedeschild[0], 0);
	dup2 (filedesparent[1], 1);

	if (debug)
		fprintf (stderr,"commandtorun = '%s'\n", commandtorun);

	retval = execlp (commandtorun, commandtorun, 0);

	fprintf (stderr, "Fatal error:  Unable to exec '%s'\n",commandtorun);
	_exit (-1);
}

/*
** Put up enough of the application that the user doesn't think it's hung...
*/

static void
BuildSkeleton()
{
	Arg		args[5];
	unsigned int	n;
	static XtActionsRec actions[] = {
		{"FetchIfNecessary",	FetchIfNecessary},
		{"MenuCallback",	MenuCallback},
		{"KeyCallback",		KeyCallback},
		{"Update",		Update},
		{"DispatchClick",	DispatchClick},
		{"TriggerAdd",		TriggerAdd},
		{"TriggerDelete",	TriggerDelete},
		{"TriggerFocusMove",	TriggerFocusMove},
		{"TriggerNum",		TriggerNum},
		{"TriggerPopdown",	TriggerPopdown},
		{"TriggerSend",		TriggerSend},
		{"TriggerWrite",	TriggerWrite},
		{"DoTheRightThing",	DoTheRightThing},
		{"DoTheRightThingInReverse",	DoTheRightThingInReverse},
		{"HelpCB",		HelpCB},
		{"QuitCB",		QuitCB},
		{"PopdownCB",		(XtActionProc) PopdownCB},
		{"Stub",		Stub},
		{"PrintEvent",		PrintEvent}};


	n = 0;
	paneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			topW,
			args,
			n);

	n = 0;
	topboxW = XtCreateManagedWidget(
			"topbox",
			boxWidgetClass,
			paneW,
			args,
			n);

        XtAppAddActions (       XtWidgetToApplicationContext(topboxW),
                                actions, XtNumber(actions));

        XawSimpleMenuAddGlobalActions(XtWidgetToApplicationContext(topboxW));

        AddChildren (topboxW, 0);

	n = 0;
	XtSetArg(args[n], XtNstring, "Please wait...");		n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNuseStringInPlace, False);		n++;

	toptextW = (TextWidget) XtCreateManagedWidget(
			"toptext",
			asciiTextWidgetClass,
			paneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	label1W = XtCreateManagedWidget(
			"label",
			asciiTextWidgetClass,
			paneW,
			args,
			n);

	n = 0;
	botboxW = XtCreateWidget(
			"botbox",
			boxWidgetClass,
			paneW,
			args,
			n);

	AddChildren (botboxW, 1);

	if (!simplemode)
		XtManageChild (botboxW);

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	bottextW = (TextWidget) XtCreateManagedWidget(
			"bottext",
			asciiTextWidgetClass,
			paneW,
			args,
			n);
}

static void
BuildUserInterface()
{
	Arg		args[5];
	unsigned int	n;
	Dimension	foo;

	static XtActionsRec actions[] = {
		{"FetchIfNecessary",	FetchIfNecessary},
		{"MenuCallback",	MenuCallback},
		{"KeyCallback",		KeyCallback},
		{"Update",		Update},
		{"DispatchClick",	DispatchClick},
		{"TriggerAdd",		TriggerAdd},
		{"TriggerDelete",	TriggerDelete},
		{"TriggerFocusMove",	TriggerFocusMove},
		{"TriggerNum",		TriggerNum},
		{"TriggerPopdown",	TriggerPopdown},
		{"TriggerSend",		TriggerSend},
		{"TriggerWrite",	TriggerWrite},
		{"DoTheRightThing",	DoTheRightThing},
		{"DoTheRightThingInReverse",	DoTheRightThingInReverse},
		{"HelpCB",		HelpCB},
		{"QuitCB",		QuitCB},
		{"PopdownCB",		(XtActionProc) PopdownCB},
		{"Stub",		Stub},
		{"PrintEvent",		PrintEvent}};

/*
	n = 0;
	paneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			topW,
			args,
			n);


	n = 0;
	topboxW = XtCreateManagedWidget(
			"topbox",
			boxWidgetClass,
			paneW,
			args,
			n);

        XtAppAddActions (       XtWidgetToApplicationContext(topboxW),
                                actions, XtNumber(actions));

        XawSimpleMenuAddGlobalActions(XtWidgetToApplicationContext(topboxW));

        AddChildren (topboxW, 0);
*/

	n = 0;
	XtSetArg(args[n], XtNstring, meetinglist);		n++;
	XtSetValues(toptextW, args, n);

/*
	n = 0;
	XtSetArg(args[n], XtNstring, meetinglist);		n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNuseStringInPlace, False);		n++;

	toptextW = (TextWidget) XtCreateManagedWidget(
			"toptext",
			asciiTextWidgetClass,
			paneW,
			args,
			n);
*/

/*
	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	label1W = XtCreateManagedWidget(
			"label",
			asciiTextWidgetClass,
			paneW,
			args,
			n);

	n = 0;
	botboxW = XtCreateWidget(
			"botbox",
			boxWidgetClass,
			paneW,
			args,
			n);

	AddChildren (botboxW, 1);

	if (!simplemode) {
		XtManageChild (botboxW);
	}

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	bottextW = (TextWidget) XtCreateManagedWidget(
			"bottext",
			asciiTextWidgetClass,
			paneW,
			args,
			n);
*/

/*
** Add the pane's accelerators to the text widgets and the other kids.
*/
	XtInstallAccelerators(toptextW, paneW);
	XtInstallAccelerators(bottextW, paneW);
	XtInstallAllAccelerators(paneW, paneW);

}

/*
** MenuCallback is called for key hits on menu entries.
** It gets two parameters:  First, the number of the menubutton
** secondly, the index of the entry in the menu.  It
** packs the two parameters into an int and manually calls TopSelect().
*/

static void
MenuCallback(w, event, params, num_params)
Widget  w;
XEvent  *event;
String  *params;
int     *num_params;
{
	int	buttonnum, entrynum;
	int	whichrow;

	if (*num_params != 2)
		goto ABORT;

	for (buttonnum = 0; buttonnum < MAX_BUTTONS; buttonnum++) {
		if (	menu_names0[buttonnum] &&
			!strcmp(menu_names0[buttonnum], params[0])) {
				whichrow = 0;
				break;
		}
		if (	menu_names1[buttonnum] &&
			!strcmp(menu_names1[buttonnum], params[0])) {
				whichrow = 1;
				break;
		}
	}

	if (buttonnum == MAX_BUTTONS)
		goto ABORT;

	for (entrynum = 0; entrynum < MAX_MENU_LEN; entrynum++) {
		if (whichrow == 0) {
			if (	submenu_names0[buttonnum][entrynum] && 
				!strcmp(submenu_names0[buttonnum][entrynum],
					params[1]))
				break;
		}
		else {
			if (	submenu_names1[buttonnum][entrynum] &&
				!strcmp(submenu_names1[buttonnum][entrynum],
					params[1]))
				break;
		}
	}

	if (entrynum == MAX_MENU_LEN)
		goto ABORT;

/*
** If a menu item was selected, popdown the menu and relinquish keyboard focus.
*/
	if (whichrow == 0)
		TopSelect (NULL, buttonnum + (entrynum << 4));
	else
		BotSelect (NULL, buttonnum + (entrynum << 4));

	if (XtIsSubclass (w, simpleMenuWidgetClass))
		XtPopdown(w);
	XtSetKeyboardFocus(topW, paneW);
	return;

ABORT:

	if (XtIsSubclass (w, simpleMenuWidgetClass))
		XtPopdown(w);
	XtSetKeyboardFocus(topW, paneW);
}

/*
** TopSelect is called either automatically through the select callback
** on a button, or manually through TopCallback when triggered by a
** key hit.
*/

void
TopSelect(w, client_data, call_data)
Widget	w;				/*IGNORED*/
XtPointer	client_data;
XtPointer	call_data;
{
	int buttonnum, entrynum;
	Arg		args[5];
	unsigned int	n;

	entrynum = ((int) client_data) >> 4;
	buttonnum = ((int) client_data) & 0x0F;

	switch (buttonnum) {
/*
** Move to next or previous meeting if topscreen is showing meetings,
** to next or previous transaction otherwise
*/
	case 0:
		if (topscreen == MAIN) {
			(void) MoveToMeeting(NEXTNEWS);
		}
		else {
			BotSelect (NULL, 0, NULL);
		}
		break;

	case 1:
		if (topscreen == MAIN) {
			(void) MoveToMeeting(PREVNEWS);
		}
		else {
			BotSelect (NULL, 1, NULL);
		}
		break;
/*
** Check for changed meetings
*/
	case 2:
		MarkLastRead();
		PutUpTempMessage("Rereading meeting list...");
		ParseMeetingsFile();
		n = 0;
		XtSetArg(args[n], XtNstring, meetinglist);		n++;
		XtSetValues(toptextW, args, n);
		TakeDownTempMessage();
		InvalidateHeaders();
		MoveToMeeting(INITIALIZE);
		break;

/*
** configure
*/
	case 3:
		switch (entrynum) {
		case 0:
			AddMeeting();
			break;
		case 1:
			DeleteMeeting();
			break;
		}
		break;

/*
** mode
*/
	case 4:
		switch (entrynum) {
		case 0:
/*
** Put up list of transactions within the current meeting
*/
			prevfirst = TransactionNum(CURRENT);
			PutUpTransactionList(	TransactionNum(CURRENT), 
						TransactionNum(LAST));
			topscreen = LISTTRNS;
			UpdateHighlightedTransaction(TransactionNum(CURRENT),True);
			break;
		case 1:
/*
** Put up list of meetings
*/
			topscreen = MAIN;
			RestoreTopTextWidget();
			if (	TransactionNum(HIGHESTSEEN) ==  
						TransactionNum(LAST))
				RemoveLetterC();
			break;
		}
		break;

/*
** show
*/
	case 5:
		if (topscreen != LISTTRNS)
			break;
		switch (entrynum) {
		case 0:
			PutUpTransactionList(
				TransactionNum(HIGHESTSEEN), TransactionNum(LAST));
			prevfirst = TransactionNum(HIGHESTSEEN);
			UpdateHighlightedTransaction(TransactionNum(CURRENT),True);
			break;
		case 1:
			PutUpTransactionList(
				TransactionNum(FIRST), TransactionNum(LAST));
			prevfirst = TransactionNum(FIRST);
			UpdateHighlightedTransaction(TransactionNum(CURRENT),True);
			break;
		case 2:
			PutUpTransactionList(	prevfirst <= 10 ? 1 : prevfirst - 10, 
						TransactionNum(LAST));
			UpdateHighlightedTransaction(prevfirst - 1,True);
	
			prevfirst -= 10;
			if (prevfirst <=0) prevfirst = 1;
			break;
		}
		break;

	case 6:
		PutUpHelp();
		break;
	case 7:
		QuitCB (NULL, NULL, NULL);
		break;
	}
}

static void
BotSelect(w, client_data, call_data)
Widget	w;				/*IGNORED*/
XtPointer	client_data;
XtPointer	call_data;
{
	int buttonnum, entrynum;

	entrynum = ((int) client_data) >> 4;
	buttonnum = ((int) client_data) & 0x0F;

	switch (buttonnum) {

	case 0:
		if (TransactionNum(CURRENT) < TransactionNum(LAST))
			GoToTransaction(NEXT, True);
		else 
			GoToTransaction(CURRENT, True);
		break;

	case 1:
		GoToTransaction(PREV, True);
		break;
	case 2:
		GoToTransaction(NREF, True);
		break;
	case 3:
		GoToTransaction(PREF, True);
		break;
	case 4:
		switch (entrynum) {
		case 0:
			GetTransactionNum();
			break;
		case 1:
			GoToTransaction(FIRST, True);
			break;
		case 2:
			GoToTransaction(LAST, True);
			break;
		}
		break;
	case 5:
		switch (entrynum) {
		case 0:
			if (axis[0] != ' ')
				SubmitTransaction(TransactionNum(CURRENT));
			break;
		case 1:
			if (axis[6] != ' ')
				SubmitTransaction(0);
			break;
		}
/*
**  This makes the buttons update to reflect the new transaction.
*/
		GoToTransaction(CURRENT, False);
		break;
	case 6:
		switch (entrynum) {
		case 0:
			WriteTransaction(TransactionNum(CURRENT));
			break;
		case 1:
			break;
		}
		break;
	default:
		fprintf (stderr,"Stub function\n");
	}

}

RestoreTopTextWidget()
{
	Arg		args[1];
	unsigned int	n;

	CheckButtonSensitivity(BUTTONS_ON);

	n = 0;
	XtSetArg(args[n], XtNstring, meetinglist);		n++;
	XtSetValues (toptextW, args, n);
	PutUpArrow(toptextW, startOfCurrentMeeting, True);
}



static void
QuitCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	(void) SaveMeetingNames("", "");

	fputs("(quit)\n", outputfile);
	fflush (outputfile);
	exit (0);
}

static void
HelpCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	PutUpHelp();
}

/*
**  HighlightNewItem...Change the highlighted line in the passed textwidget
**  in the direction requested.  Return 0 if successful, -1 if there was
**  no meeting in that direction.
*/

HighlightNewItem(textW, mode, flag)
Widget	textW;		/* list of meetings */
int	mode;		/* one of { NEXTNEWS, PREVNEWS, UPDATE, INITIALIZE} */
Boolean	flag;		/* update current meeting? */
{
	Arg		args[5];
	unsigned int	n;
	XawTextPosition	start, end, inspoint;
	char		*tempstring, *foo, *tempptr;
	int		length;
	char		statusline[LONGNAMELEN + 25];
	char		longmtg[LONGNAMELEN];
	char		shortmtg[SHORTNAMELEN];

	inspoint = XawTextGetInsertionPoint(textW);

	n = 0;
	XtSetArg(args[n], XtNstring, &tempstring);		n++;
	XtGetValues (textW, args, n);

	if (tempstring[inspoint] == '\0')
		return (-1);

/*
** Find start and end of current line.
*/

	for (start = inspoint; start && tempstring[start-1] != '\n'; start--)
		;
	for (	end = inspoint;  
		tempstring[end] != '\0' && tempstring[end] != '\n'; 
		end++)
		;

/*
** Special case for initializing:  change mode to NEXTNEWS unless  we're 
** already on a line with unread transactions.  Do nothing in simplemode, 
** because we always want to stay on the first line.
*/
	if (mode == INITIALIZE && simplemode != True)  {
		if ( tempstring[start + 2] != 'c')
			mode = NEXTNEWS;
	}

	if (mode == NEXTNEWS) {

		if (tempstring[end] == '\0') {
			PutUpWarning(	"", 
					"Nothing more to read", False);
			return(-1);
		}

		do {
			if (tempstring[end] == '\n') {
				start = end + 1;
			}
			end = start + strcspn (tempstring + start, "\n\0");
		} while (	tempstring[end] != '\0' &&
				tempstring[start + 2] != 'c');

		if ( tempstring[start + 2] != 'c') {
			PutUpWarning(	"", 
					"Nothing more to read", False);
			return(-1);
		}

	}

	else if (mode == PREVNEWS) {
		if (start == 0) {
			PutUpWarning(	"", 
					"no previous meeting with unread news", False);
			return(-1);
		}
		do {
			end = start - 1;
			for (	start = end - 1; 
				start > 0 && tempstring[start] != '\n';
				start--)
					;
			if (start != 0) start++;
		} while (	start &&
				tempstring[start + 2] != 'c');
		if ( tempstring[start + 2] != 'c' ) {
			PutUpWarning(	"", 
					"no previous meeting with unread news", False);
			return(-1);
		}
	}

	else if (mode == UPDATE) {
/*
** Just need to update current meeting to match highlighted item.
*/
	}

	PutUpArrow(textW, start, True);

	if (flag) {
		length = (end - start);
		foo = (char *) calloc (length + 1, sizeof(char));
		strncpy (foo, tempstring + start + 8, length - 8);
		tempptr = strchr (foo, ',');
		*tempptr = '\0';
		strcpy (longmtg, foo);
		strcpy (shortmtg, tempptr + 2);

		free (foo);
	}

	if (SaveMeetingNames(longmtg, shortmtg) == -1) {
		PutUpStatusMessage("No current meeting");
		CheckButtonSensitivity(BUTTONS_OFF);
		return (-1);
	}
	CheckButtonSensitivity(BUTTONS_ON);

	sprintf (statusline, "Reading %s [%d-%d]", 
			longmtg,
			TransactionNum(FIRST),
			TransactionNum(LAST));

	PutUpStatusMessage(statusline);

	XFlush(XtDisplay(textW));
	return(0);
}


/*
** Run the command.  If textW and filename are passed,
** read the contents of filename after running the command
** and put them into textW.  
**
** If returnvalue is NULL, don't wait for a response from the
** command.  Otherwise, return a pointer to the return value,
** which must be freed by the calling routine.
**
** This version uses byte-by-byte i/o so it can recognize \n
** as end-of-data.  Can't use fgets because we don't know
** how long the data stream is, and need to keep reallocing.
**
** Return -1 on error.
*/

char	*
RunCommand (command, textW, filename, returnvalue)
char	*command;
Widget	textW;
char	*filename;
Boolean	returnvalue;
{
	char	*message;
	int	cursize = BUFSIZE;
	int	curbyte = 0;

/*
** Send command to child's input stream
*/
	if (fputs(command, outputfile) == EOF) {
		fprintf (stderr, "Error on fputs\n");
		return ((char *)-1);
	}
	fflush (outputfile);

	if (debug)
		fprintf (stderr, "appl: %s\n",command);
	fflush (stderr);

	if (!returnvalue)
		return (NULL);
/*
** Look for child's (one-line) response
*/

READLINE:

	message = (char *) malloc (BUFSIZE);
	
	while ((	message [cursize - BUFSIZE + curbyte] = 
			fgetc (inputfile)) != '\n') {

		curbyte++;
		if (curbyte == BUFSIZE) {
			cursize += BUFSIZE;
			curbyte = 0;
			message = (char *) realloc (message, cursize);
		}
	}

	message[cursize - BUFSIZE + curbyte] = '\0';

	if (debug)
		fprintf (stderr, "edsc: %s\n",message);

	if (message[0] == ';' ) {
		PutUpWarning("ERROR", message + 1, False);
		myfree (message);
		return ((char *)-1);
	}

	if (message[0] == '-') {
		PutUpWarning("WARNING", message + 1, False);
		myfree (message);
		goto READLINE;
	}
/*
** Read the verbose response and put it into the text widget
*/
	if (filename && textW)
		FileIntoWidget(filename, textW);

	return (message);
}


FileIntoWidget(filename, textW)
char		*filename;
TextWidget	textW;
{
	unsigned int	fd;
	Arg		args[5];
	char		*message;
	int		cursize = BUFSIZE;
	int		numread = 1, n;
	char		errormsg[80];

	fd = open(filename, O_RDONLY);

	if (fd == -1) {
		sprintf (errormsg,"Error opening file %s\n",filename);
		PutUpWarning("WARNING", errormsg, False);
		return (-1);
	}

/*
** Make certain we don't try to erase the (now nonexistant) plus sign.
*/
	if (textW == toptextW)
		oldarrow = -1;

	message = (char*) malloc (cursize);

	while (!HELL_FROZEN_OVER) {
		numread = read (fd, message + cursize - BUFSIZE, BUFSIZE);
		if (numread == -1) {
			sprintf (errormsg,"Error reading from file %s\n",filename);
			PutUpWarning("WARNING", errormsg, False);
			close (fd);
			return(-1);
		}
		if (numread < BUFSIZE)
			break;

		cursize += BUFSIZE;
		message = (char*) realloc (message, cursize);
	}

	if (*message) {
		message[cursize - BUFSIZE + numread] = '\0';
		n = 0;
		XtSetArg (args[n], XtNstring, message);		n++;
		XtSetValues (textW, args, n);
	}
	else {
		PutUpWarning(	"", 
				"No text in this transaction", False);
		XBell (XtDisplay(textW), 0);

	}

	XFlush(XtDisplay(textW));

	myfree (message);
	close (fd);
	return(0);
}

static void
CheckEdscVersion()
{
	char	*version;

	version = RunCommand ("(gpv)\n", NULL, NULL, True);

	if ((int) version == -1) {
		fprintf (stderr, "Cannot communicate with edsc.\n");
		exit (-1);
	}

	edscversion = atoi(version + 1);

	if (edscversion < 24) {
		fprintf (stderr, "Caution...using edsc version %1.1f.  ",(float) edscversion/10.0);
		fprintf (stderr, "Should be using at least 2.4.\n");
	}

	nocache = (edscversion >= 25) ? True : False;
}

ParseMeetingsFile()
{
	char	*fulllist;
	char	*movingptr;
	char	*firstquote, *secondquote;
	int	status;
	char	fullname[100], shortname[100];
	int	i;

	fulllist = RunCommand ("(gml)\n", NULL, NULL, True);
	if ((int) fulllist <= 0) return;

	if (meetinglist)
		myfree (meetinglist);

	i = strlen(fulllist) * 2;
	meetinglist = (char *) calloc (i, sizeof(char));
	if (debug) fprintf (stderr, "Allocated %x, %d long\n", meetinglist, i);

	secondquote = fulllist;

	for (i = 0; ; i++) {

		movingptr = strchr (secondquote + 1, '(');

		if (!movingptr)
			break;

		movingptr++;

		status = atoi(movingptr);

		firstquote = strchr(movingptr, '"');
		secondquote = strchr(firstquote+1, '"');
		*secondquote = '\0';
		strcpy (fullname, firstquote + 1);

		if (*(secondquote + 1) == ')') {
			*shortname = '\0';
		}

		else {
			firstquote = strchr(secondquote+1, '"');
			secondquote = strchr(firstquote+1, '"');
			*secondquote = '\0';
			strcpy (shortname, firstquote + 1);
		}

		if (debug)
			fprintf (stderr, "Got %d, '%s', '%s'\n",
				status, fullname, shortname);

		sprintf (meetinglist, "%s  %c     %s, %s\n",
				meetinglist, 
				status ? 'c' : ' ', 
				fullname, shortname);
	}

	sprintf (meetinglist, "%s\0",meetinglist);
	myfree(fulllist);
	if (debug) fprintf (stderr, "Actually used %d\n", strlen(meetinglist));
}

myfree(ptr)
char	*ptr;
{
	if (ptr > 0) {
		if (debug) fprintf (stderr, "Freeing %x\n",ptr);
		free(ptr);
	}
	else if (debug) fprintf (stderr, "Not freeing %x\n",ptr);
}

/*
** An item in the upper text window has been clicked on.  If it is a
** meeting, move there and put the first transaction on the screen. 
** If it's a transaction, we display it in the lower window.
*/

void
Update()
{
	if (topscreen == MAIN) {
		(void) MoveToMeeting(UPDATE);
	}
	else if (topscreen == LISTTRNS) {
		GoToTransaction(HighlightedTransaction(), True);
	}
}


void
Stub()
{
}

void
PrintEvent(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	fprintf(stderr, "event type %d for widget %s\n",
		event->type, XtName(w));
}


/*
**  If we're reading a transaction, scroll it one page down.
**  If we're at the end of a transaction, go to the next one.
**  If we're at the end of a meeting, find the next one with unread
**  transactions.
*/

static void
DoTheRightThing()
{
	if (PopdownCB(NULL, NULL, NULL))
		return;

	if (!TryToScrollAPage(bottextW,1)) {
		return;
	}
		
/*
**  Are we at the end of a meeting?
*/
	if (TransactionNum(NEXT) != 0) {
/*
**  No, read the next transaction
*/
		BotSelect(NULL, 0, NULL);
	}
	else {
/*
**  Yes, go to the next meeting and read its next transaction.
*/
		if ((simplemode != True) && (MoveToMeeting(NEXTNEWS) == 0)) {
			BotSelect(NULL, 0, NULL);
		}
	}
}

static void
DoTheRightThingInReverse()
{
	if (!TryToScrollAPage(bottextW,-1)) {
		return;
	}
		
/*
**  Are we at the start of a meeting?
*/
	if (TransactionNum(CURRENT) > TransactionNum(FIRST)) {
/*
**  No, read the prev transaction
*/
		BotSelect(NULL, (XtPointer)1, NULL);
	}
	else {
/*
**  Yes, go to the prev meeting and read its next transaction.
*/
		TopSelect(NULL, (XtPointer)1, NULL);
	}
}

/*
**  Code stolen from Text.c and TextAction.c to scroll the text widget.
**  Many text functions are static, so I had to take what I could get.
*/

TryToScrollAPage(ctx, direction)
TextWidget	ctx;
int		direction;
{
	int scroll_val = (ctx->text.lt.lines - 1) * direction;

/*
** Don't scroll off the bottom
*/
	if (ctx->text.lt.info[ctx->text.lt.lines].position > 
			ctx->text.lastPos &&
			direction > 0) {
		return (-1);
	}
/*
** Don't scroll off the top
*/
	if (ctx->text.lt.top == 0 &&
			direction < 0) {
		return (-1);
	}

	_XawTextPrepareToUpdate(ctx);
	_XawTextVScroll(ctx, scroll_val);

	ctx->text.insertPos = ctx->text.lt.top;
	_XawTextCheckResize(ctx);
	_XawTextExecuteUpdate(ctx);
	ctx->text.mult = 1;
	return (0);
}


static char		oldline[80];

PutUpTempMessage(string)
char	*string;
{
	char		*returndata;
	Arg		args[1];

	XtSetArg(args[0], XtNstring, &returndata);
	XtGetValues (label1W, args, 1);
	strcpy (oldline, returndata);

	XtSetArg(args[0], XtNstring, string);
	XtSetValues (label1W, args, 1);
	XFlush(XtDisplay(label1W));
}

TakeDownTempMessage()
{
	Arg		args[1];

	XtSetArg(args[0], XtNstring, oldline);
	XtSetValues (label1W, args, 1);
}

PutUpStatusMessage(string)
char	*string;
{
	Arg		args[1];

	XtSetArg(args[0], XtNstring, string);
	XtSetValues (label1W, args, 1);
	XFlush(XtDisplay(label1W));
}

/*
** PutUpArrow assumes that "start" is the first character position of a line
** in a text widget.  It puts a marker "+" on this line, and if moveinsert
** is True, moves the insert point there.
*/

PutUpArrow(textW, start, moveinsert)
TextWidget	textW;
XawTextPosition	start;
Boolean		moveinsert;
{
	XawTextBlock		textblock;
	static int		oldtopscreen = -1;
	int			offset;
	XawTextPosition		oldinspoint;

	offset = (topscreen == MAIN) ? 7 : 0;

	oldinspoint = XawTextGetInsertionPoint(textW);

/*
** Don't try to erase an arrow on another top screen
*/
	if (topscreen != oldtopscreen)
		oldarrow = -1;

	oldtopscreen = topscreen;

	textblock.firstPos = 0;
	textblock.length = 1;
	textblock.format = FMT8BIT;

	if (oldarrow != -1) {
		textblock.ptr = " ";
		XawTextReplace (	textW, oldarrow + offset, 
					oldarrow + offset + 1, &textblock);
	}

	textblock.ptr = "+";

	XawTextReplace (	textW, start + offset, 
				start + offset + 1, &textblock);

	if (moveinsert)
		XawTextSetInsertionPoint (textW, start);
	else
		XawTextSetInsertionPoint (textW, oldinspoint);

	if (topscreen == MAIN)
		startOfCurrentMeeting = start;

	XFlush(XtDisplay(textW));

	oldarrow = start;
}

/*
**  This assumes the insert position is at the start of the line containing
**  the letter 'c'.  
*/

/*
**  If UseStringInPlace were True for toptextW, we
**  wouldn't need both the XawTextReplace and the setting of
**  the char in meetinglist, but setting it to True causes weird
**  memory overlaps I haven't figured out yet.
*/

void
RemoveLetterC()
{
	XawTextBlock		textblock;
	XawTextPosition		inspoint;

	textblock.firstPos = 0;
	textblock.length = 1;
	textblock.format = FMT8BIT;
	textblock.ptr = " ";

	inspoint = startOfCurrentMeeting;

	if (inspoint > strlen (meetinglist))
		return;

	XawTextReplace (toptextW, inspoint + 2, inspoint + 3, &textblock);

	*(meetinglist + inspoint + 2) = ' ';

	XFlush(XtDisplay(toptextW));
}

AddChildren (parent, whichone)
Widget	parent;
int	whichone;
{
	Widget	command, menu, entry;
	int	i, j, n;
	char	*name, *label;
	Arg	args[5];
	EntryRec	*toprec, *newrec;
	void	(*mycallback)();

	char **buttonlabels;
	char **buttonnames;

	if (whichone == 0) {
		buttonlabels = menu_labels0;
		buttonnames = menu_names0;
		mycallback = TopSelect;
	}
	else {
		buttonlabels = menu_labels1;
		buttonnames = menu_names1;
		mycallback = BotSelect;
	}

	for (i = 0; i < MAX_BUTTONS && buttonnames[i]; i++) {
		n = 0;
		XtSetArg(args[n], XtNmenuName, buttonlabels[i]);	n++;
		XtSetArg(args[n], XtNlabel, buttonlabels[i]);		n++;

		if (whichone == 0) {
			label = submenu_labels0[i][0];
			name = submenu_names0[i][0];
		}
		else {
			label = submenu_labels1[i][0];
			name = submenu_names1[i][0];
		}
		toprec = &(toplevelbuttons[whichone][i]);

		if (name) {
			command = XtCreateManagedWidget(
				buttonnames[i],
				menuButtonWidgetClass, parent,
				args, n);
		}
		else {
			command = XtCreateManagedWidget(
				buttonnames[i],
				commandWidgetClass, parent,
				args, n);
			XtAddCallback(	command, XtNcallback, 
					mycallback, i);
		}

		toprec->button = command;
		toprec->nextrec = NULL;

		XtInstallAccelerators(topW, command);

		if (!name)
			continue;

/*
**  Add menus to menu buttons.
*/

		n = 0;
		menu = XtCreatePopupShell(
				buttonlabels[i],
				simpleMenuWidgetClass, 
				parent, 
				args, n);

		for (j = 0; j < MAX_MENU_LEN; j++) {
			if (whichone == 0) {
				label = submenu_labels0[i][j];
				name = submenu_names0[i][j];
			}
			else {
				label = submenu_labels1[i][j];
				name = submenu_names1[i][j];
			}
			if (!name)
				break;

			newrec = (EntryRec *) malloc (sizeof (EntryRec));
			toprec->nextrec = newrec; 
			newrec->nextrec = NULL; 
			n = 0;
			XtSetArg(args[n], XtNlabel, label);		n++;

			entry = XtCreateManagedWidget(	
					name, smeBSBObjectClass, menu,
				      	args, n);
			XtAddCallback(	entry, XtNcallback,
					mycallback, i + (j << 4));
			newrec->button = entry;
			toprec = newrec;
		}
	}
}
    
/*
** A keyboard equivalent has been hit.  For normal command widgets, send
** them a button-one-down and button-one-up.  For menu buttons, just send
** them the button-down, and their child will send the button-up.
*/

static void	
KeyCallback(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	Widget		button;
	XButtonEvent	MyEvent;
	int		i, whichrow;
	
	if (*num_params < 1)
		return;

	for (i = 0; i < MAX_BUTTONS; i++) {

		if (menu_names0[i] && !strcmp (params[0], menu_names0[i])) {
			whichrow = 0;
			break;
		}

		if (menu_names1[i] && !strcmp (params[0], menu_names1[i])) {
			whichrow = 1;
			break;
		}
	}

	if (i == MAX_BUTTONS) {
		fprintf (stderr, "Key not found.  Aborting.\n");
		return;
	}

	button = (toplevelbuttons[whichrow][i]).button;

	MyEvent.type = ButtonPress;
	MyEvent.display = dpy;
	MyEvent.window = XtWindow(button);
	MyEvent.button = 1;
	MyEvent.x = 1;
	MyEvent.y = 1;
	MyEvent.state = ButtonPressMask;
	XSendEvent(	dpy,
			XtWindow(button),
			False,
			ButtonPressMask,
			(XEvent *) &MyEvent);

	XSync(dpy, False);
	if (!(toplevelbuttons[whichrow][i]).nextrec) {
		MyEvent.type = ButtonRelease;
		MyEvent.state = ButtonReleaseMask;
		XSendEvent(	dpy,
				XtWindow(button),
				False,
				ButtonReleaseMask,
				(XEvent *) &MyEvent);
	}

}

/*
**  Check time interval between mouse clicks.  If it's less than the
**  intrinsics' multi-click timer, call my own "Update" procedure.
**  Otherwise, pass the click on to the text widget.
*/

void
DispatchClick(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;         /* unused */
Cardinal *num_params;   /* unused */
{
	static Time	lasttime = 0;

	if (	lasttime == 0 ||
		(XtLastTimestampProcessed(XtDisplay(w)) - lasttime) >
			(Time) XtGetMultiClickTime(XtDisplay(w))) {
		XtCallActionProc(w, "select-start", event, params, num_params);
	}
	else {
		XtCallActionProc(w, "Update", event, params, num_params);
	}
	lasttime = XtLastTimestampProcessed(XtDisplay(w));
}

