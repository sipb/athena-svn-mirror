#include	<stdio.h>
#include	<X11/IntrinsicP.h>
#include	<X11/StringDefs.h>
#include	<X11/CoreP.h>
#include	<Xaw/List.h>
#include	<Xaw/Command.h>
#include	<Xaw/Box.h>
#include	<Xaw/Paned.h>
#include	<Xaw/AsciiText.h>
#include	<Xaw/TextP.h>
#include	<Xaw/TextSinkP.h>
#include	<Xaw/Dialog.h>
#include	<Xaw/Form.h>
#include	<Xaw/Label.h>
#include	"xdsc.h"

static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xdsc/xdsc.c,v 1.6 1990-12-20 15:29:00 sao Exp $";

/*
** Globals
*/

char		*RunCommand();
TextWidget	toptextW, bottextW;
Boolean		debug = False;
CommandWidget	botbuttons[MAX_BOT_BUTTONS];
char		filebase[50];
int		whichTopScreen = MAIN;
Widget		topW, paneW;
void		RemoveLetterC();
int		char_width;

/*
** External functions
*/

extern void	PutUpHelp();
extern void	SubmitTransaction();
extern void	WriteTransaction();
extern char	*strchr();
extern char	*tempnam();
extern void	PopDownCB();
extern char     *getenv();

/*
** Private functions
*/

static void	TopButton1CB(), TopButton2CB(), TopButton3CB();
static void	BotButtonCB();
static void	QuitCB(), HelpCB();
static void	BuildUserInterface(), SetLabelsAndCallback();
static void	Update(), Stub();
static void	DoTheRightThing();
static void	DoTheRightThingInReverse();
static void	TopFakeKeyPress();
static void	BotFakeKeyPress();
static void	DisplayHighlightedTransaction();

/*
** Private globals
*/

static int	filedesparent[2], filedeschild[2];
static FILE	*inputfile, *outputfile;

static char	*meetinglist;
static Boolean	reading = False;

static Widget	topboxW, botboxW;
static Widget	helpButton, quitButton;
static Widget	label1W;
static int	prevfirst = 0;
static Boolean	changedMeetingList = False;
static XawTextPosition	startOfCurrentLine = -1;

static char *botstrings[] = {
	"next", "prev", 
	"Next in chain", "Prev in chain",
	"reply", 
	"compose", "write", "first", "last", NULL};
static char *sampletop2 =
"	You can change the meetings you attend by\n\
	pressing the \"add meeting\" or \"delete meeting\"\n\
	buttons and filling in the blanks on the popups.\n\
\n\
	If the current transaction contains an announcement\n\
	of a new meeting, as found in the \"new_meetings\"\n\
	meeting, default values will be read from it.";

static CommandWidget	topbuttons[MAX_TOP_BUTTONS];

static XawTextPosition	savedstart = -1;

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

	sprintf (filebase,"/tmp/xdsc%d",getpid());

	if (debug)
		fprintf (stderr, "filebase is %s\n", filebase);

	pipe (filedesparent);
	pipe (filedeschild);

	pid = fork();

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

	SetLabelsAndCallback (MAIN);

	(void) MoveToMeeting(INITIALIZE);
	BotButtonCB(NULL, 0, NULL);
	CheckButtonSensitivity(BUTTONS_UPDATE);
	XtMainLoop();
}

SetUpEdsc()
{
	int	retval;
	FILE	*tempfile;
	char	commandtorun[50];
	char	machtype[20];

/*
** Figure out what type of machine I'm running on
*/
	sprintf (commandtorun, "/bin/athena/machtype > %s", filebase);
	if (system (commandtorun) == 127) {
		fprintf (stderr, "Cannot execute %s to determine machine type\n", commandtorun);
		exit (-1);
	}

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

	fprintf (stderr, "Shouldn't be here.  Return from exec was %d\n",retval);
	exit (-1);
}

static void
BuildUserInterface()
{
	CommandWidget	commandW;
	Arg		args[5];
	unsigned int	n, i;

	static XtActionsRec actions[] = {
		{"BotFakeKeyPress",	BotFakeKeyPress},
		{"TopFakeKeyPress",	TopFakeKeyPress},
		{"Update",		Update},
		{"DoTheRightThing",	DoTheRightThing},
		{"DoTheRightThingInReverse",	DoTheRightThingInReverse},
		{"HelpCB",		HelpCB},
		{"QuitCB",		QuitCB},
		{"PopDownCB",		PopDownCB},
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

	for (i = 0; i < MAX_TOP_BUTTONS; i++) {
		n = 0;
		XtSetArg(args[n], XtNlabel, "---");		n++;
		commandW = (CommandWidget) XtCreateManagedWidget(
			"box",
			commandWidgetClass,
			topboxW,
			NULL,
			0);
		topbuttons[i] = commandW;
	}

	n = 0;
	XtSetArg(args[n], XtNfromHoriz,	topboxW);		n++;
	XtSetArg(args[n], XtNright, XawChainRight);		n++;
	XtSetArg(args[n], XtNresizable, False);			n++;
	helpButton = XtCreateManagedWidget(
			"help",
			commandWidgetClass,
			topboxW,
			args,
			n);
	XtAddCallback (helpButton, XtNcallback, HelpCB, NULL);

	n = 0;
	XtSetArg(args[n], XtNfromHoriz,	topboxW);		n++;
	XtSetArg(args[n], XtNvertDistance, 4);			n++;
	XtSetArg(args[n], XtNfromVert,	helpButton);		n++;
	XtSetArg(args[n], XtNright, XawChainRight);		n++;
	XtSetArg(args[n], XtNresizable, False);			n++;
	quitButton = XtCreateManagedWidget(
			"quit",
			commandWidgetClass,
			topboxW,
			args,
			n);
	XtAddCallback (quitButton, XtNcallback, QuitCB, NULL);

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

/*
** This lets us notice that we've changed meetings
*/
	XtAppAddActions (	XtWidgetToApplicationContext(toptextW),
				actions, XtNumber(actions));

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

	for (i = 0; i < MAX_BOT_BUTTONS; i++) {
		n = 0;
		XtSetArg(args[n], XtNlabel, botstrings[i]);	n++;
		commandW = (CommandWidget) XtCreateManagedWidget(
			"command",
			commandWidgetClass,
			botboxW,
			args,
			n);
		botbuttons[i] = commandW;
		XtAddCallback (commandW, XtNcallback, BotButtonCB, i);
	}

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

static void
SetLabelsAndCallback(mode)
int mode;
{
	Arg		args[5];
	unsigned int	n, i;
	static void	(* oldcallback)();

	char		*newTranslations;
	void		(* callback)();
	char		**labels;

	static String	mainTranslations =
		"~Ctrl<Key>d:	TopFakeKeyPress(0) \n\
		~Ctrl<Key>u:	TopFakeKeyPress(1) \n\
		~Ctrl<Key>i:	TopFakeKeyPress(2) \n\
		~Ctrl<Key>e:	TopFakeKeyPress(3) \n\
		~Ctrl<Key>s:	TopFakeKeyPress(4) \n\
		~Ctrl<Key>a:	Stub() \n\
		~Ctrl<Key>m:	Stub() \n\
		~Ctrl<Key>b:	Stub() \n\
		Ctrl<Key>r:	search(backward) \n\
		Ctrl<Key>s:	search(forward) \n\
		";

	static String	editTranslations =
		"~Ctrl<Key>d:	TopFakeKeyPress(1) \n\
		~Ctrl<Key>u:	Stub() \n\
		~Ctrl<Key>i:	Stub() \n\
		~Ctrl<Key>e:	Stub() \n\
		~Ctrl<Key>s:	Stub() \n\
		~Ctrl<Key>a:	TopFakeKeyPress(0) \n\
		~Ctrl<Key>m:	TopFakeKeyPress(2) \n\
		~Ctrl<Key>b:	Stub() \n\
		Ctrl<Key>r:	search(backward) \n\
		Ctrl<Key>s:	search(forward) \n\
		";

	static String	listTranslations =
		"~Ctrl<Key>d:	Stub() \n\
		~Ctrl<Key>u:	TopFakeKeyPress(0) \n\
		~Ctrl<Key>i:	Stub() \n\
		~Ctrl<Key>e:	Stub() \n\
		~Ctrl<Key>s:	Stub() \n\
		~Ctrl<Key>a:	TopFakeKeyPress(1) \n\
		~Ctrl<Key>m:	TopFakeKeyPress(3) \n\
		~Ctrl<Key>b:	TopFakeKeyPress(2) \n\
		Ctrl<Key>r:	search(backward) \n\
		Ctrl<Key>s:	search(forward) \n\
		";


	static char *topstrings1[] = {
		"down meeting", "up meeting",
		"inc", "edit mtgs", "show trns",  NULL};

	static char *topstrings2[] = {
		"add meeting", "delete meeting",
		"main screen", NULL};
	
	static char *topstrings3[] = {
		"unread", "all", "back ten",
		"main screen", NULL};

	switch (mode) {
		case MAIN:
			newTranslations = mainTranslations;
			callback = TopButton1CB;
			labels = topstrings1;
			break;
		case EDITMTGS:
			newTranslations = editTranslations;
			callback = TopButton2CB;
			labels = topstrings2;
			break;
		case LISTTRNS:
			newTranslations = listTranslations;
			callback = TopButton3CB;
			labels = topstrings3;
			break;
		default:
			fprintf (stderr, "Bad mode, %d to SetLabelsAndCallback",mode);
	}

	whichTopScreen = mode;

	XtOverrideTranslations(	toptextW,
			XtParseTranslationTable(newTranslations));
	XtOverrideTranslations(	bottextW,
			XtParseTranslationTable(newTranslations));

	XtUnmapWidget(topboxW);
	for (i = 0; i < MAX_TOP_BUTTONS; i++) {
		XtUnmanageChild (topbuttons[i]);
	}

	for (i = 0; labels[i] && i < MAX_TOP_BUTTONS; i++) {
		n = 0;
		XtSetArg(args[n], XtNlabel, labels[i]);	n++;
		XtSetValues(topbuttons[i], args, n);

		if (XtHasCallbacks(topbuttons[i], XtNcallback) == XtCallbackHasSome) {
			XtRemoveAllCallbacks (topbuttons[i], XtNcallback);
		}

		XtAddCallback (	topbuttons[i], XtNcallback, 
				callback, (XtPointer)i );
	}

	for (i = 0; labels[i] && i < MAX_TOP_BUTTONS; i++)
		XtManageChild (topbuttons[i]);
	XtMapWidget(topboxW);

	oldcallback = callback;
}

/*
** Callback for "list meetings" mode
*/

static void
TopButton1CB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	int		buttonnum = (int) client_data;
	Arg		args[5];
	unsigned int	n;

	switch (buttonnum) {

/*
** Move to next or previous meeting
*/
	case 0:
		if (MoveToMeeting(NEXTNEWS) == 0) {
			reading = False;
			BotButtonCB(NULL, 0, NULL);
		}
		break;

	case 1:
		if (MoveToMeeting(PREVNEWS) == 0) {
			reading = False;
			BotButtonCB(NULL, 0, NULL);
		}
		break;
/*
** Check for changed meetings
*/
	case 2:
		PutUpTempMessage("Rereading meeting list...");
		ParseMeetingsFile();
		n = 0;
		XtSetArg(args[n], XtNstring, meetinglist);		n++;
		XtSetValues(toptextW, args, n);
		TakeDownTempMessage();
		reading = False;
		(void) HighlightNewItem((Widget) toptextW, NEXTNEWS, True);
		GoToTransaction(TransactionNum(HIGHESTSEEN), True);
		break;

/*
** Change modes to editting list of meetings
*/
	case 3:
		SaveTopTextWidget();
		n = 0;
		XtSetArg(args[n], XtNstring, sampletop2);		n++;
		XtSetValues (toptextW, args, n);
		SetLabelsAndCallback (EDITMTGS);
		break;

/*
** Put up list of transactions within the current meeting
*/
	case 4:
		SaveTopTextWidget();
		prevfirst = TransactionNum(CURRENT);
		PutUpTransactionList(TransactionNum(CURRENT), TransactionNum(LAST));
		UpdateHighlightedTransaction(TransactionNum(CURRENT));
		break;

	}
}

/*
** Callback for "edit meetings" mode
*/

static void
TopButton2CB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	int	buttonnum = (int) client_data;

	switch (buttonnum) {
	case 2:
/*
** Change modes to reading transactions
*/
		RestoreTopTextWidget();
		break;

	case 0:
		AddMeeting();
		changedMeetingList = True;
		break;
	case 1:
		DeleteMeeting();
		changedMeetingList = True;
		break;
	}
}


/*
** Callback for "list transactions" mode
*/

static void
TopButton3CB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	int	buttonnum = (int) client_data;

	switch (buttonnum) {
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

/*  Since "back ten" is typically used for searching through
**  a list of headers, I removed the following line because it made
**  the back tenning go too slowly.
		GoToTransaction(prevfirst - 1, True);
*/
		prevfirst -= 10;
		if (prevfirst <=0) prevfirst = 1;
		break;
	case 3:
		RestoreTopTextWidget();
		if (TransactionNum(HIGHESTSEEN) ==  TransactionNum(LAST))
			RemoveLetterC();
		break;

	default:
		fprintf (stderr, "No translations for this button\n");
	}
}

static void
BotButtonCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	int	buttonnum = (int) client_data;

	switch (buttonnum) {

	case 6:
		WriteTransaction(TransactionNum(CURRENT));
		break;

	case 0:

/*
** If in a new meeting
*/
		if (reading == False) {
/*
** Is there new stuff to read?  Y->go to next one.  N->go to the
** transaction last read.
*/
			GoToTransaction(TransactionNum(HIGHESTSEEN), False);

			if (TransactionNum(CURRENT) < TransactionNum(LAST))
				GoToTransaction(TransactionNum(NEXT), True);
			else 
				GoToTransaction(TransactionNum(CURRENT), True);

			reading = True;
		}
/*
** Else, continuing in current meeting
*/
		else {
			GoToTransaction(TransactionNum(NEXT), True);
		}

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
	case 7:
		GoToTransaction(TransactionNum(FIRST), True);
		break;
	case 8:
		GoToTransaction(TransactionNum(LAST), True);
		break;
	case 4:
		SubmitTransaction(TransactionNum(CURRENT));
/*
**  This makes the buttons update to reflect the new transaction.
*/
		GoToTransaction(TransactionNum(CURRENT), False);
		break;
	case 5:
		SubmitTransaction(0);
/*
**  Ditto.
*/
		GoToTransaction(TransactionNum(CURRENT), False);
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
	(void) EnterMeeting("", "");

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
**
** Watch out for that newline...A clicked-on selection includes the
** newline as the end of it.
*/
		if (tempstring[end-1] == '\n') {
			PutUpWarning(	"Yo, Andy!", 
					"This got called after all!",
					False);
			end--;
		}
	}

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

	if (EnterMeeting(longmtg, shortmtg) == -1) {
		PutUpStatusMessage("No current meeting");
		CheckButtonSensitivity(BUTTONS_OFF);
		return (-1);
	}

	PutUpArrow(textW, start);

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
	if (whichTopScreen == MAIN) {
		if (MoveToMeeting(UPDATE) == 0)
			reading = False;
		BotButtonCB(NULL, 0, NULL);
	}
	else if (whichTopScreen == LISTTRNS) {
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
	reading = True;
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
*/
	if (TransactionNum(CURRENT) < TransactionNum(LAST)) {
/*
**  No, read the next transaction
*/
		BotButtonCB(NULL, 0, NULL);
	}
	else {
/*
**  Yes, go to the next meeting and read its next transaction.
*/
		TopButton1CB(NULL, (XtPointer)0, NULL);
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
		BotButtonCB(NULL, (XtPointer)1, NULL);
	}
	else {
/*
**  Yes, go to the prev meeting and read its next transaction.
*/
		TopButton1CB(NULL, (XtPointer)1, NULL);
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


	SetLabelsAndCallback (LISTTRNS);

/*
** Can we optimize by keeping some of the old data?
*/
	if (	*oldmeeting &&
		!strcmp (oldmeeting, CurrentMtg(0)) && 
		finish == oldfinish && 
		start <= oldstart) {

		if (oldstart == start)
			return;

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

		FileIntoWidget(filename, (Widget) toptextW);

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


static void	
TopFakeKeyPress(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	switch (whichTopScreen) {
	case MAIN:
		TopButton1CB(NULL, (XtPointer)atoi (params[0]), NULL);
		break;
	case EDITMTGS:
		TopButton2CB(NULL, (XtPointer)atoi (params[0]), NULL);
		break;
	case LISTTRNS:
		TopButton3CB(NULL, (XtPointer)atoi (params[0]), NULL);
		break;
	}
}

static void	
BotFakeKeyPress(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	BotButtonCB(NULL, (XtPointer) atoi (params[0]), NULL);
}

SaveTopTextWidget()
{
	XawTextPosition	inspoint;

	if (savedstart != -1)
		return;

	inspoint = XawTextGetInsertionPoint(toptextW);
/*
** Find start of current line
*/
	for (savedstart = inspoint; savedstart && meetinglist[savedstart-1] != '\n'; savedstart--)
		;
}

RestoreTopTextWidget()
{
	Arg		args[1];
	unsigned int	n;

	SetLabelsAndCallback (MAIN);
	CheckButtonSensitivity(BUTTONS_ON);
/*
** If the meeting list's changed since we saved state, reread it in
** and highlight the next meeting with news.
*/
	if (changedMeetingList) {
		TopButton1CB (NULL, (XtPointer)2, NULL);
		changedMeetingList = False;
	}
/*
** Otherwise, restore the old meeting list.
*/

	else {
		n = 0;
		XtSetArg(args[n], XtNstring, meetinglist);		n++;
		XtSetValues (toptextW, args, n);
		PutUpArrow(toptextW, savedstart);
	}

	savedstart = -1;
}

/*
** This should only be called if whichTopScreen == LISTTRNS.   It moves
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
	static int		oldTopScreen = -1;

/*
** Don't try to erase an arrow on another top screen
*/
	if (whichTopScreen != oldTopScreen)
		oldstart = -1;

	oldTopScreen = whichTopScreen;

	textblock.firstPos = 0;
	textblock.length = 1;
	textblock.format = FMT8BIT;

	if (oldstart != -1) {
		textblock.ptr = " ";
		XawTextReplace (	textW, oldstart + 7, 
					oldstart + 8, &textblock);
	}

	textblock.ptr = "+";

	XawTextReplace (	textW, start + 7, 
				start + 8, &textblock);

	XawTextSetInsertionPoint (textW, start);
	startOfCurrentLine = start;

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

	inspoint = startOfCurrentLine;
/*
	inspoint = XawTextGetInsertionPoint(toptextW);
*/

	if (inspoint > strlen (meetinglist))
		return;

	XawTextReplace (toptextW, inspoint + 2, inspoint + 3, &textblock);

	*(meetinglist + inspoint + 2) = ' ';

	XFlush(XtDisplay(toptextW));
}
