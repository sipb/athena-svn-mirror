#include	<stdio.h>
#include	<X11/IntrinsicP.h>
#include	<X11/StringDefs.h>
#include	<X11/CoreP.h>

#include        <Xaw/MenuButton.h>
#include        <Xaw/SimpleMenu.h>
#include        <Xaw/Sme.h>
#include        <Xaw/SmeBSB.h>
#include        <Xaw/Cardinals.h>

#include	<Xaw/List.h>
#include        <Xaw/SimpleMenP.h>
#include        <Xaw/Paned.h>
#include	<Xaw/Command.h>
#include	<Xaw/Box.h>
#include	<Xaw/AsciiText.h>
#include	<Xaw/TextP.h>
#include	<Xaw/TextSinkP.h>
#include	<Xaw/Dialog.h>
#include	<Xaw/Form.h>
#include	<Xaw/Label.h>
#include	"xdsc.h"

static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xdsc/xdsc.c,v 1.8 1991-02-11 16:30:27 sao Exp $";

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

/*
** External functions
*/

extern void	PutUpHelp();
extern void	SubmitTransaction();
extern void	WriteTransaction();
extern char	*strchr();
extern char	*tempnam();
extern void	PopdownCB();
extern char     *getenv();
extern void	TriggerAdd(), TriggerNum(), TriggerDelete();
extern void	TriggerWrite(), TriggerPopdown(), TriggerSend();
extern void	TriggerFocusMove();

/*
** Private functions
*/

/*
static void	TopCallback(), BotCallback();
*/
static void	MenuCallback();
static void	KeyCallback();
static void	QuitCB(), HelpCB();
static void	BuildUserInterface();
static void	Update(), Stub();
static void	DoTheRightThing();
static void	DoTheRightThingInReverse();
static void	DisplayHighlightedTransaction();

/*
** Private globals
*/

static int	filedesparent[2], filedeschild[2];
static FILE	*inputfile, *outputfile;

static char	*meetinglist;

static Widget	topboxW, botboxW;
static Widget	label1W;
static int	prevfirst = 0;
static Boolean	changedMeetingList = False;
static XawTextPosition	startOfCurrentMeeting = -1;
Display         *dpy;
Window          root_window;

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


EntryRec        toplevelbuttons[2][MAX_BUTTONS];

void
main(argc, argv)
int argc;
char *argv[];
{
	int	pid;
	char	*oldpath, *newpath;
	Arg	args[1];
	int	width;

	if (argc > 1 && !strcmp(argv[1], "-debug"))
		debug = True;

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

	ParseMeetingsFile();

	oldpath = getenv("XFILESEARCHPATH");

#ifdef mips
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

	topW = XtInitialize("topwidget", "Xdsc", NULL, 0, &argc, argv);

	myfree (newpath);

	BuildUserInterface ();

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

	dpy = XtDisplay(topW);
	root_window = XtWindow(topW);

	(void) MoveToMeeting(INITIALIZE);
	CheckButtonSensitivity(BUTTONS_UPDATE);
	XtMainLoop();
}

SetUpEdsc()
{
	int	retval;
	char	commandtorun[50];
	char	machtype[20];

#ifdef 0

/*
** Figure out what type of machine I'm running on
	sprintf (commandtorun, "/bin/athena/machtype > %s", filebase);
	if (system (commandtorun) == 127) {
		fprintf (stderr, "Cannot execute %s to determine machine type\n", commandtorun);
		exit (-1);
	}
*/

	if (! (tempfile = fopen(filebase, "r"))) {
		fprintf (stderr, "Cannot open file %s\n", filebase);
		exit (-1);
	}

	fgets (machtype, 20, tempfile);
	fclose (tempfile);
	unlink (filebase);

/*
** Remove trailing newline
*/
	machtype[strlen(machtype)-1] = '\0';

#endif

#if defined (mips) && defined (ultrix)
	strcpy (machtype, "decmips");
#else
#ifdef vax
	strcpy (machtype, "vax");
#else
#ifdef ibm032
	strcpy (machtype, "rt");
#else
	Need to define for this machine
#endif
#endif
#endif

	sprintf (	commandtorun, 
			"/mit/StaffTools/%sbin/edsc",
			machtype);

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

static void
BuildUserInterface()
{
	Arg		args[5];
	unsigned int	n;

	static XtActionsRec actions[] = {
		{"MenuCallback",	MenuCallback},
		{"KeyCallback",		KeyCallback},
		{"Update",		Update},
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
		{"PopdownCB",		PopdownCB},
		{"Stub",		Stub}};


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
	XtSetArg(args[n], XtNstring, meetinglist);		n++;
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
	botboxW = XtCreateManagedWidget(
			"botbox",
			boxWidgetClass,
			paneW,
			args,
			n);

	AddChildren (botboxW, 1);

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	bottextW = (TextWidget) XtCreateManagedWidget(
			"bottext",
			asciiTextWidgetClass,
			paneW,
			args,
			n);

/*
** Add the pane's accelerators to the text widgets and the other kids.
*/
	XtInstallAccelerators(toptextW, paneW);
	XtInstallAccelerators(bottextW, paneW);
	XtInstallAllAccelerators(paneW, paneW);

}

/*
** TopCallback is called for key hits on menu entries.
** TopCallback gets two parameters:  First, the number of the menubutton
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

#ifdef 0
static void
TopCallback(w, event, params, num_params)
Widget  w;
XEvent  *event;
String  *params;
int     *num_params;
{
	int	buttonnum, entrynum;

	buttonnum = atoi(params[0]);
	entrynum = atoi(params[1]);

/*
** If a menu item was selected, popdown the menu and relinquish keyboard focus.
*/
	if (entrynum != -1)  {
		TopSelect (NULL, buttonnum + (entrynum << 4));
	}
	XtPopdown(w);
	XtSetKeyboardFocus(topW, paneW);
}
#endif

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
		MoveToMeeting(INITIALIZE);
/* trial
		(void) HighlightNewItem((Widget) toptextW, NEXTNEWS, True);
*/
		break;

/*
** configure
*/
	case 3:
		switch (entrynum) {
		case 0:
			AddMeeting();
			changedMeetingList = True;
			break;
		case 1:
			DeleteMeeting();
			changedMeetingList = True;
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
			UpdateHighlightedTransaction(TransactionNum(CURRENT));
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
			UpdateHighlightedTransaction(TransactionNum(CURRENT));
			break;
		case 1:
			PutUpTransactionList(
				TransactionNum(FIRST), TransactionNum(LAST));
			prevfirst = TransactionNum(FIRST);
			UpdateHighlightedTransaction(TransactionNum(CURRENT));
			break;
		case 2:
			PutUpTransactionList(	prevfirst <= 10 ? 1 : prevfirst - 10, 
						TransactionNum(LAST));
			UpdateHighlightedTransaction(prevfirst - 1);
	
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

#ifdef 0
static void
BotCallback(w, event, params, num_params)
Widget  w;
XEvent  *event;
String  *params;
int     *num_params;
{
	int	buttonnum, entrynum;

	buttonnum = atoi(params[0]);
	entrynum = atoi(params[1]);

/*
** If a menu item was selected, popdown the menu and relinquish keyboard focus.
*/
	if (entrynum != -1)  {
		BotSelect (NULL, buttonnum + (entrynum << 4));
	}
	XtPopdown(w);
	XtSetKeyboardFocus(topW, paneW);
}
#endif


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
			GoToTransaction(TransactionNum(NEXT), True);
		else 
			GoToTransaction(TransactionNum(CURRENT), True);
		break;

	case 1:
		GoToTransaction(TransactionNum(PREV), True);
		break;
	case 2:
		GoToTransaction(TransactionNum(NREF), True);
		break;
	case 3:
		GoToTransaction(TransactionNum(PREF), True);
		break;
	case 4:
		switch (entrynum) {
		case 0:
			GetTransactionNum();
			break;
		case 1:
			GoToTransaction(TransactionNum(FIRST), True);
			break;
		case 2:
			GoToTransaction(TransactionNum(LAST), True);
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
		GoToTransaction(TransactionNum(CURRENT), False);
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
** Special case for initializing:  If we're already on a line with
** unread transactions, stay there.
*/
	if (mode == INITIALIZE)  {
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

	PutUpArrow(textW, start);

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

/*
** The 'Disc quota exceeded' and 'Permission denied' errors still produce 
** output, so they are treated as warnings.
*/
	if (message[0] == ';' ) {
		if (	!strcmp (message, ";Disc quota exceeded") ||
			!strcmp (message, ";Permission denied")) {
			PutUpWarning("WARNING", message + 1, False);
			myfree (message);
			goto READLINE;
		}
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

static void
Update()
{
	if (topscreen == MAIN) {
		(void) MoveToMeeting(UPDATE);
	}
	else if (topscreen == LISTTRNS) {
		DisplayHighlightedTransaction();
	}
}

/*
** Pull the transaction number out of the highlighted line, and 
** display it.
*/

static void
DisplayHighlightedTransaction()
{
	Arg		args[5];
	unsigned int	n, num;
	XawTextPosition	start, inspoint;
	char		*tempstring;

	n = 0;
	XtSetArg(args[n], XtNstring, &tempstring);		n++;
	XtGetValues (toptextW, args, n);

	inspoint = XawTextGetInsertionPoint(toptextW);

	if (tempstring[inspoint] == '\0')
		return;

	for (start = inspoint; start && tempstring[start-1] != '\n'; start--)
		;
	PutUpArrow(toptextW, start);

	num = atoi (strchr (tempstring + start, '[') + 1);

	GoToTransaction(num, True);
}

static void
Stub()
{
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
	if (!TryToScrollAPage(bottextW,1)) {
		return;
	}
		
/*
**  Are we at the end of a meeting?
	if (TransactionNum(CURRENT) < TransactionNum(LAST)) {
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
		if (MoveToMeeting(NEXTNEWS) == 0) {
			BotSelect(NULL, 0, NULL);
		}
/*
		TopSelect(NULL, (XtPointer)0, NULL);
		BotSelect(NULL, 0, NULL);
*/
		
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

/*
** Get the list of transaction headers from start to finish and
** put them into the upper text widget.
*/

PutUpTransactionList(start, finish)
int	start;
int	finish;
{
	char		command[LONGNAMELEN + 25];
	char		filename[70];
	char		*returndata;
	static char	oldmeeting[LONGNAMELEN];
	static int	oldstart=0, oldfinish=0;
	Arg		args[1];

	XtSetArg(args[0], XtNsensitive, True);
	XtSetValues ((toplevelbuttons[0][5]).button, args, 1);

/*
** Can we optimize by keeping some of the old data?
*/
	if (	*oldmeeting &&
		!strcmp (oldmeeting, CurrentMtg(0)) && 
		finish == oldfinish && 
		start <= oldstart) {

		if (oldstart == start) {
			sprintf (filename, "%s-list", filebase);
			FileIntoWidget(filename, toptextW);
			return;
		}

		sprintf (	command, 
				"mv %s-list %s-old", 
				filebase, filebase);
		if (system (command) != 0) {
			sprintf (	command, 
					"Cannot write to '%s-old'\n",
					filebase);
			PutUpWarning("WARNING", command, False);
		}

		sprintf (	command, 
				"Reading headers for transactions %d to %d%s...", 
				start, oldstart-1,
				(oldstart-start>100) ? " (This may take a while)":"");

		PutUpTempMessage(command);

		sprintf (filename, "%s-list", filebase);
		sprintf (command, "(ls %s %d %d 0 %s)\n", filename, 
			start, oldstart-1, CurrentMtg(0));
		returndata = RunCommand (command, NULL, NULL, True);
		if ((int) returndata <= 0) {
			TakeDownTempMessage();
			return (-1);
		}

		sprintf (	command, 
				"cat %s-old >> %s", 
				filebase, filename);
		if (system (command) != 0) {
			sprintf (	command, 
					"Cannot write to '%s'\n",
					filename);
			PutUpWarning("WARNING", command, False);
		}

		FileIntoWidget(filename, toptextW);

		myfree (returndata);
		sprintf (filename, "%s-old", filebase);
		unlink (filename);
	}

/*
** Get an entirely new list
*/
	else {

		sprintf (	command, 
			"Reading headers for transactions %d to %d%s...", 
			start, finish,
			(finish-start>100)?" (This may take a while)":"");

		PutUpTempMessage(command);
		sprintf (filename, "%s-list", filebase);
		unlink (filename);

		sprintf (filename, "%s-list", filebase);
		sprintf (command, "(ls %s %d %d 0 %s)\n", filename, 
			start, finish, CurrentMtg(0));
		returndata = RunCommand (command, (Widget) toptextW, filename, True);
		if ((int) returndata <= 0) {
			TakeDownTempMessage();
			return (-1);
		}

		myfree (returndata);
	}

	TakeDownTempMessage();
	strcpy (oldmeeting, CurrentMtg(0));
	oldstart = start;
	oldfinish = finish;
}

RestoreTopTextWidget()
{
	Arg		args[1];
	unsigned int	n;

	CheckButtonSensitivity(BUTTONS_ON);
/*
** If the meeting list's changed since we saved state, reread it in
** and highlight the next meeting with news.
*/
	if (changedMeetingList) {
		TopSelect (NULL, (XtPointer)2, NULL);
		changedMeetingList = False;
	}
/*
** Otherwise, restore the old meeting list.
*/

	else {
		n = 0;
		XtSetArg(args[n], XtNstring, meetinglist);		n++;
		XtSetValues (toptextW, args, n);
		PutUpArrow(toptextW, startOfCurrentMeeting);
	}
}

/*
** This should only be called if topscreen == LISTTRNS.   It moves
** the highlight of the upper text widget to the line for the
** specified transaction.
*/

UpdateHighlightedTransaction(num)
int num;
{
	static	int	lastend;
	Arg		args[5];
	unsigned int	n;
	char		*tempstring, *foo = NULL, *bar = NULL;
	char		buf[50];

	n = 0;
	XtSetArg(args[n], XtNstring, &tempstring);		n++;
	XtGetValues (toptextW, args, n);

/*
** Okay, the following is REAL STUPID code.  I'm looking for a specific
** transaction number, so I sprintf the specified transaction number
** into a string and search for it in each line of the text widget's string.
**
** Should really use a better way to find the line than sequential search!
*/

	sprintf (buf, " [%04d]", num);
	foo = tempstring;

/*
** Specialization for common case of moving one forwards...
*/
	if (	lastend && 
		lastend < strlen (tempstring) &&
		 (!strncmp (tempstring + lastend + 1, buf, strlen(buf))))
			foo = tempstring + lastend + 1;

	else {
		while (*foo) {
			if (!strncmp (foo, buf, strlen(buf)))
				break;
			for ( ; *foo && *foo != '\n'; foo++)
				;
			if (*foo) foo++;
		}
	}

	if (*foo) {
		for (bar = foo; *bar && *bar != '\n'; bar++)
			;
		PutUpArrow(toptextW, foo - tempstring);
	}
	if (bar)
		lastend = bar - tempstring;
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

PutUpArrow(textW, start)
TextWidget	textW;
XawTextPosition	start;
{
	XawTextBlock		textblock;
	static XawTextPosition	oldstart = -1;
	static int		oldtopscreen = -1;
	int			offset;

	offset = (topscreen == MAIN) ? 7 : 0;

/*
** Don't try to erase an arrow on another top screen
*/
	if (topscreen != oldtopscreen)
		oldstart = -1;

	oldtopscreen = topscreen;

	textblock.firstPos = 0;
	textblock.length = 1;
	textblock.format = FMT8BIT;

	if (oldstart != -1) {
		textblock.ptr = " ";
		XawTextReplace (	textW, oldstart + offset, 
					oldstart + offset + 1, &textblock);
	}

	textblock.ptr = "+";

	XawTextReplace (	textW, start + offset, 
				start + offset + 1, &textblock);

	XawTextSetInsertionPoint (textW, start);

	if (topscreen == MAIN)
		startOfCurrentMeeting = start;

	XFlush(XtDisplay(textW));

	oldstart = start;
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
/*
	inspoint = XawTextGetInsertionPoint(toptextW);
*/

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
