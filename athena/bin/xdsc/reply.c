#include	<stdio.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<Xaw/List.h>
#include	<Xaw/Command.h>
#include	<Xaw/Box.h>
#include	<Xaw/Paned.h>
#include	<Xaw/AsciiText.h>
#include	<Xaw/Dialog.h>
#include	<X11/Shell.h>
#include	"xdsc.h"

static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xdsc/reply.c,v 1.6 1990-12-20 15:23:55 sao Exp $";

extern char	*strchr();
extern char     *getenv();
extern char	*RunCommand();
extern char	filebase[];
extern int	whichTopScreen;
extern Widget	topW, paneW;
extern TextWidget	bottextW;
extern int	char_width;

static char	sourcefile[80];

static void	SendCB();

static Widget	writePopupW = 0, writeDialogW;
static void	WriteCB();

static Widget	addPopupW = 0, addHostTextW, addPathTextW;
static void	AddCB();

static Widget	deletePopupW = 0, deleteMtgTextW;
static void	DeleteCB();

static Widget	warningPopupW = 0;
void		PopDownCB(), DieCB();

static Widget	helpPopupW = 0;

static char *GetDefaultValue();

typedef struct {
	Widget	sendPopupW;
	Widget	sendButtonW;
	Widget	subjectTextW;
	Widget	bodyTextW;
	char	mtg[80];
	int	replynum;
} senddata, *senddataptr;

void
SubmitTransaction(myreplynum)
int	myreplynum;
{
	Arg		args[5];
	unsigned int	n;
	char		*subjectline;
	char		buffer[80];
	char		*ptr1, *ptr2;
	char		*returndata;
	Widget		localPaneW, box1W, box2W, buttonW;
	senddataptr	data;

	data = (senddataptr) malloc (sizeof (senddata));
/*
**  Save current meeting name and reply number
*/
	strcpy (data->mtg, CurrentMtg(0));
	data->replynum = myreplynum;

/*
**  Find and use the subject of the previous transaction if we are replying.
*/
	if (myreplynum != 0) {
		sprintf (buffer, "(gti %d %s)\n", myreplynum, CurrentMtg(0));
		returndata = RunCommand (buffer, NULL, NULL, True);

		if ((int) returndata <= 0)  {
			return;
		}

/*
**  Find the third quote
*/
		for (n = 0, ptr1 = returndata; n < 3; n++)
			while (*ptr1++ != '\"')
				;
/*
** Copy from just after the third quote to the next non-escaped quote
*/
		ptr2 = buffer;
		while (*ptr1 != '\"') {
			if (*ptr1 == '\\')
				ptr1++;
			*ptr2++ = *ptr1++;
		}
		*ptr2 = '\0';

		if (!strncmp (buffer, "Re: ", 4)) {
			subjectline = (char *) malloc (strlen(buffer) + 1);
			sprintf (subjectline, "%s", buffer);
		}
		else {
			subjectline = (char *) malloc (strlen(buffer) + 5);
			sprintf (subjectline, "Re: %s", buffer);
		}
		myfree(returndata);
	}
	else {
		subjectline = (char *) malloc (80);
		sprintf (subjectline, "");
	}

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	data->sendPopupW = XtCreatePopupShell(	
			"enterpopup",
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	n = 0;
	localPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			data->sendPopupW,
			args,
			n);

	if (myreplynum == 0) {
		sprintf (	buffer,
				" (Entering new transaction in %s)", 
				CurrentMtg(0));
	}

	else {
		sprintf (	buffer,
				" (Replying to transaction %d in %s)",
				myreplynum, CurrentMtg(0));
	}

	n = 0;
	XtSetArg(args[n], XtNstring, buffer);			n++;
	XtSetArg(args[n], XtNeditType, XawtextRead);		n++;
	(void) XtCreateManagedWidget(
			"desctext",
			asciiTextWidgetClass,
			localPaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	box1W = XtCreateManagedWidget(
			"topbox",
			boxWidgetClass,
			localPaneW,
			args,
			n);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			box1W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNstring, subjectline);		n++;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
	data->subjectTextW = XtCreateManagedWidget(
			"subjecttext",
			asciiTextWidgetClass,
			box1W,
			args,
			n);
	myfree(subjectline);

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	data->bodyTextW = XtCreateManagedWidget(
			"bodytext",
			asciiTextWidgetClass,
			localPaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	box2W = XtCreateManagedWidget(
			"botbox",
			boxWidgetClass,
			localPaneW,
			args,
			n);

	n = 0;
	data->sendButtonW = XtCreateManagedWidget(
			"send",
			commandWidgetClass,
			box2W,
			args,
			n);

	XtAddCallback (data->sendButtonW, XtNcallback, SendCB, data);

	n = 0;
	buttonW = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			box2W,
			args,
			n);
	XtAddCallback (buttonW, XtNcallback, SendCB, data);
	XtPopup(data->sendPopupW, XtGrabNone);
}

static void
SendCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	Arg		args[5];
	unsigned int	n;
	char		*tempstring;
	char		command [LONGNAMELEN + 25];
	char		filename[50];
	FILE		*fp;
	char		*returndata;
	senddataptr	data = (senddataptr) client_data;

	if (data->sendButtonW == w) {
		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring);		n++;
		XtGetValues (data->bodyTextW, args, n);
	
		sprintf (	filename, "%s-%dr", 
				filebase, data->replynum);
		if ((fp = fopen(filename, "w")) == (FILE *) NULL) {
			sprintf (command, "could not open file'%s'\n", filename);
			PutUpWarning("WARNING", command, False);
		}
		else {
			fprintf (fp, "%s\n", tempstring);
			fclose (fp);
/*
** Breakdown in the edsc protocol:  We have to first send it the (at...)
** to enter the transaction, then the newline-terminated subject line
** on the next line.  (No parens around subject line)
*/
			sprintf(command, "(at %d %s %s)\n",
				data->replynum, filename, data->mtg);
			(void) RunCommand (command, NULL, NULL, False);

			n = 0;
			XtSetArg(args[n], XtNstring, &tempstring);		n++;
			XtGetValues (data->subjectTextW, args, n);

			sprintf(command, "%s\n",tempstring);

			returndata =  RunCommand (command, NULL, NULL, True);
			if ((int) returndata > 0)
				myfree(returndata);
			unlink (filename);
		}
	}

	XtDestroyWidget(data->sendPopupW);
	myfree (data);

	GoToTransaction (TransactionNum(CURRENT), False);

	XFlush(XtDisplay(topW));
}

void
WriteTransaction(current)
int	current;
{
	Arg		args[5];
	unsigned int	n;
	Widget		writeButton1W, writeButton2W;
	char		destfile[80];

	if (writePopupW)
		return;

	sprintf (	destfile, 
			"%s/xdsc/%s-%d", getenv("HOME"), 
			CurrentMtg(1), current);

	sprintf (	sourcefile,
			"%s-%d", filebase, current);

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	writePopupW = XtCreatePopupShell(	
			"writepopup", 
			topLevelShellWidgetClass,
			topW,
			args,
			n);
	n = 0;
	XtSetArg(args[n], XtNvalue, destfile);			n++;
	writeDialogW = XtCreateManagedWidget(
			"dialog",
			dialogWidgetClass,
			writePopupW,
			args,
			n);

	n = 0;
	writeButton1W = XtCreateManagedWidget(
			"write",
			commandWidgetClass,
			writeDialogW,
			args,
			n);

	XtAddCallback (writeButton1W, XtNcallback, WriteCB, True);

	n = 0;

	writeButton2W = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			writeDialogW,
			args,
			n);

/*
** I really wish this wasn't necessary, but the dialog widget IGNORES
** XtNwidth at creation time and in the app-defaults file!
*/
	XtResizeWidget(writePopupW, 400, 80, 1);

	XtAddCallback (writeButton2W, XtNcallback, WriteCB, False);
	XtPopup(writePopupW, XtGrabNone);
}

static void
WriteCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	char	command[165];
	char	*tempptr1, *destfile;

	if ((Boolean) client_data) {
		destfile = XawDialogGetValueString(writeDialogW);
/*
** Make directory path.  We ignore errors until we actually try to copy.
*/
		tempptr1 = strchr(destfile + 1,'/');

		while (tempptr1) {
			*tempptr1 = '\0';
			sprintf (command, "test -d %s", destfile);
			if (system (command) != 0) {
				sprintf (command, "mkdir %s", destfile);
				system (command);
			}
			*tempptr1 = '/';
			tempptr1 = strchr(tempptr1+1,'/');
		}

		sprintf (command, "cp %s %s", sourcefile, destfile);
		if (system (command) != 0) {
			sprintf (command, "Cannot write to '%s'\n",destfile);
			PutUpWarning("WARNING", command, False);
		}
	}

	XtDestroyWidget(writePopupW);
	writePopupW = 0;
}

/*
void
AddMeeting()
{
	Arg		args[5];
	unsigned int	n;

	Widget		addPaneW, addBox1W, addLabel1W;
	Widget		addBox2W, addLabel2W, addBox3W, addButton1W;
	Widget		addButton2W;

	if (addPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	addPopupW = XtCreatePopupShell(	
			"addpopup",
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	addPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			addPopupW,
			args,
			n);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addPaneW,
			args,
			n);
	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox1W = XtCreateManagedWidget(
			"box1",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addLabel1W = XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addBox1W,
			args,
			n);


	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
	addHostTextW = XtCreateManagedWidget(
			"hosttext",
			asciiTextWidgetClass,
			addBox1W,
			args,
			n);
	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox2W = XtCreateManagedWidget(
			"box2",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addLabel2W = XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addBox2W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
	addPathTextW = XtCreateManagedWidget(
			"pathtext",
			asciiTextWidgetClass,
			addBox2W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox3W = XtCreateManagedWidget(
			"box3",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addButton1W = XtCreateManagedWidget(
			"add",
			commandWidgetClass,
			addBox3W,
			args,
			n);
	XtAddCallback (addButton1W, XtNcallback, AddCB, True);

	n = 0;
	addButton2W = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			addBox3W,
			args,
			n);

	XtAddCallback (addButton2W, XtNcallback, AddCB, False);
	XtPopup(addPopupW, XtGrabNone);
}
*/

void
AddMeeting()
{
	Arg		args[5];
	unsigned int	n;
	char		*defaultvalue1, *defaultvalue2;

/*
** Enforce single line input.
*/
	static String	specialTranslations =
		"<Key>Return:	Stub()";

	Widget		addPaneW, addBox1W, addLabel1W;
	Widget		addBox2W, addLabel2W, addBox3W, addButton1W;
	Widget		addButton2W;

	if (addPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	addPopupW = XtCreatePopupShell(	
			"addpopup",
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	addPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			addPopupW,
			NULL,
			0);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addPaneW,
			args,
			n);
	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox1W = XtCreateManagedWidget(
			"box1",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addLabel1W = XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addBox1W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
/*
** Is there a default hostname we can use?
*/
	if (defaultvalue1 = GetDefaultValue("Host:")) {
		XtSetArg(args[n], XtNstring, defaultvalue1);	n++;
	}

	addHostTextW = XtCreateManagedWidget(
			"hosttext",
			asciiTextWidgetClass,
			addBox1W,
			args,
			n);

	if (defaultvalue1)
		myfree (defaultvalue1);

	XtOverrideTranslations(	addHostTextW,
				XtParseTranslationTable(specialTranslations));
	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox2W = XtCreateManagedWidget(
			"box2",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addLabel2W = XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			addBox2W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
/*
** Is there a default pathname we can use?
*/
	if (defaultvalue2 = GetDefaultValue("Pathname:")) {
		XtSetArg(args[n], XtNstring, defaultvalue2);	n++;
	}

	addPathTextW = XtCreateManagedWidget(
			"pathtext",
			asciiTextWidgetClass,
			addBox2W,
			args,
			n);

	if (defaultvalue2)
		myfree (defaultvalue2);

	XtOverrideTranslations(	addPathTextW,
				XtParseTranslationTable(specialTranslations));

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	addBox3W = XtCreateManagedWidget(
			"box3",
			boxWidgetClass,
			addPaneW,
			args,
			n);

	n = 0;
	addButton1W = XtCreateManagedWidget(
			"add",
			commandWidgetClass,
			addBox3W,
			args,
			n);
	XtAddCallback (addButton1W, XtNcallback, AddCB, True);

	n = 0;
	addButton2W = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			addBox3W,
			args,
			n);

	XtAddCallback (addButton2W, XtNcallback, AddCB, False);
	XtPopup(addPopupW, XtGrabNone);
}

static void
AddCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	char		buffer[165];
	char		*tempstring1, *tempstring2, *returndata;
	Arg		args[5];
	unsigned int	n;

	if ((Boolean) client_data) {
		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring1);		n++;
		XtGetValues (addHostTextW, args, n);

		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring2);		n++;
		XtGetValues (addPathTextW, args, n);

		sprintf (buffer, "(am %s %s)\n", tempstring1, tempstring2);
		returndata = RunCommand (buffer, NULL, NULL, True);

		if ( (int) returndata <= 0)
			return;

		myfree (returndata);
		myfree (tempstring1);
		myfree (tempstring2);
	}

	XtDestroyWidget(addPopupW);
	addPopupW = 0;
}

void
DeleteMeeting()
{
	Arg		args[5];
	unsigned int	n;

	Widget		deletePaneW, deleteBox1W;
	Widget		deleteBox2W, deleteButton1W, deleteButton2W;
	char		*defaultvalue;
	static String	specialTranslations =
		"<Key>Return:	Stub()";

	if (deletePopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;
	deletePopupW = XtCreatePopupShell(	
			"deletepopup",
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	deletePaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			deletePopupW,
			NULL,
			0);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			deletePaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	deleteBox1W = XtCreateManagedWidget(
			"box1",
			boxWidgetClass,
			deletePaneW,
			args,
			n);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			deleteBox1W,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 60 * char_width);		n++;
	if (defaultvalue = GetDefaultValue("Name:")) {
		XtSetArg(args[n], XtNstring, defaultvalue);	n++;
	}
	deleteMtgTextW = XtCreateManagedWidget(
			"mtgtext",
			asciiTextWidgetClass,
			deleteBox1W,
			args,
			n);

	XtOverrideTranslations(	deleteMtgTextW,
				XtParseTranslationTable(specialTranslations));
	if (defaultvalue)
		myfree (defaultvalue);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	deleteBox2W = XtCreateManagedWidget(
			"box2",
			boxWidgetClass,
			deletePaneW,
			args,
			n);

	n = 0;
	deleteButton1W = XtCreateManagedWidget(
			"delete",
			commandWidgetClass,
			deleteBox2W,
			args,
			n);
	XtAddCallback (deleteButton1W, XtNcallback, DeleteCB, True);

	n = 0;
	deleteButton2W = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			deleteBox2W,
			args,
			n);
	XtAddCallback (deleteButton2W, XtNcallback, DeleteCB, False);

	XtPopup(deletePopupW, XtGrabNone);
}

static void
DeleteCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	char		buffer[165];
	char		*tempstring1, *returndata;
	Arg		args[5];
	unsigned int	n;

	if ((Boolean) client_data) {
		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring1);		n++;
		XtGetValues (deleteMtgTextW, args, n);
/*
** Protect us when we remove the current meeting.
*/
		if (!strcmp (tempstring1, CurrentMtg(0)))
			EnterMeeting("", "");

		sprintf (buffer, "(dm %s)\n", tempstring1);
		returndata = RunCommand (buffer, NULL, NULL, True);

		if ( (int) returndata <= 0)
			return;

		myfree (returndata);
		myfree (tempstring1);
	}

	XtDestroyWidget(deletePopupW);
	deletePopupW = 0;
}

PutUpWarning(prefix, message, deathoption)
char	*prefix;
char	*message;
Boolean	deathoption;
{
	Arg		args[5];
	unsigned int	n;
	Widget		warningPaneW, warningButtonW, warningBoxW, deathButtonW;

	if (warningPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, (strlen(message)+2) * char_width);	n++;
	warningPopupW = XtCreatePopupShell(	
			"warningpopup", 
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	warningPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			warningPopupW,
			NULL,
			0);
	XtInstallAccelerators(warningPaneW, paneW);

	n = 0;
	XtSetArg(args[n], XtNlabel, prefix);			n++;
	XtSetArg(args[n], XtNwidth, 300);			n++;
	(void) XtCreateManagedWidget(
			"label1",
			labelWidgetClass,
			warningPaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNlabel, message);			n++;
	(void) XtCreateManagedWidget(
			"label2",
			labelWidgetClass,
			warningPaneW,
			args,
			n);
	n = 0;
	warningBoxW = XtCreateManagedWidget(
			"box",
			boxWidgetClass,
			warningPaneW,
			args,
			n);

	n = 0;
	warningButtonW = XtCreateManagedWidget(
			"acknowledge",
			commandWidgetClass,
			warningBoxW,
			args,
			n);

	XtAddCallback (warningButtonW, XtNcallback, PopDownCB, False);

	if (deathoption) {
		deathButtonW = XtCreateManagedWidget(
				"quit immediately",
				commandWidgetClass,
				warningBoxW,
				NULL, 0);

		XtAddCallback (deathButtonW, XtNcallback, DieCB, False);
	}

	XtInstallAllAccelerators(warningPopupW, paneW);

	XtPopup(warningPopupW, XtGrabNone);
}

void
DieCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	exit (-1);
}

void
PopDownCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	if (warningPopupW) {
		XtDestroyWidget(warningPopupW);
		warningPopupW = 0;
	}

	if (helpPopupW) {
		XtDestroyWidget(helpPopupW);
		helpPopupW = 0;
	}
}


static char *helptext1 =
"                      Actions on main screen\n\
---------------------------------------------------------------------\n\
  down meeting	Move to the next meeting with new transactions\n\
  up meeting	Move to the previous meeting with new transactions\n\
  inc		Check for new transactions\n\
  edit mtgs	Change mode to edit the list of meetings\n\
  show trns	Change mode to show transaction headers\n\
  HELP 		Display this screen\n\
  QUIT 		Quit\n\
---------------------------------------------------------------------\n\
  next		Read the next transaction in the\n\
		current meeting\n\
  prev		Read the previous transaction in the\n\
		current meeting\n\
  Next in chain	Read the next transaction in this chain\n\
  Prev in chain	Read the previous transaction in this chain\n\
  reply		Enter a new transaction chained to the\n\
		current one\n\
  compose	Enter a new transaction\n\
  write		Save the current transaction to a file\n\
  first		Go to the first transaction in the meeting\n\
  last		Go to the last transaction in the meeting\n\
  spacebar	'do the right thing'\n\
  backspace	reverse what space did\n\
---------------------------------------------------------------------\n\
You can also enter a meeting by clicking on its title with mouse\n\
button two or three.\n\
\n\
The keyboard equivalent for clicking on a button is always the first\n\
character on the button.  Note that uppercase and lowercase\n\
letters can be distinct.\n\
\n\
If a button is grayed out, this action is not possible at this time.\n\
For example, the 'Next' button will gray out when you are at the last\n\
transaction in a meeting.\n\
";

static char *helptext2 =
"                Actions while editting meeting list\n\
---------------------------------------------------------------------\n\
  add meeting		Put a new meeting on the list of\n\
			meetings you attend\n\
  delete meeting	Remove a meeting from the list of\n\
			meetings you attend\n\
  main screen		Go back to the main screen\n\
  HELP 			Display this screen\n\
  QUIT 			Quit\n\
---------------------------------------------------------------------\n\
The keyboard equivalent for clicking on a button is always the first\n\
character of the button's label\n\
";

static char *helptext3 =
"              Actions while showing transaction headers\n\
---------------------------------------------------------------------\n\
  unread	Show headers of unread transactions\n\
  all		Show headers of all transactions\n\
			(This can take a while!)\n\
  back ten	Show headers of ten more transactions\n\
  main screen	Go back to the main screen\n\
  HELP 		Display this screen\n\
  QUIT 		Quit\n\
---------------------------------------------------------------------\n\
  next		Read the next transaction\n\
  prev		Read the previous transaction\n\
  Next in chain	Read the next transaction in this chain\n\
  Prev in chain	Read the previous transaction in this chain\n\
  reply		Enter a new transaction in response to the current one\n\
  compose	Enter a new transaction\n\
  write		Save the current transaction to a file\n\
  first		Go to the first transaction in the meeting\n\
  last		Go to the last transaction in the meeting\n\
  spacebar	'do the right thing'\n\
  backspace	reverse what space did\n\
---------------------------------------------------------------------\n\
You can read a specific transaction by clicking on its header with\n\
mouse button two or three.\n\
\n\
The keyboard equivalent for clicking on a button is always the first\n\
character on the button.  Note that uppercase and lowercase\n\
letters can be distinct.\n\
";

void
PutUpHelp()
{
	Arg		args[5];
	unsigned int	n;
	Widget	localPaneW, textW, boxW, buttonW;

	if (helpPopupW)
		return;

	n = 0;
	helpPopupW = XtCreatePopupShell(	
			"helppopup",
			topLevelShellWidgetClass,
			topW,
			args,
			n);

	XtInstallAccelerators(helpPopupW, paneW);
	n = 0;
	localPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			helpPopupW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 80 * char_width);		n++;

	switch (whichTopScreen) {
	case MAIN:
		XtSetArg(args[n], XtNstring, helptext1);	n++;
		break;
	case EDITMTGS:
		XtSetArg(args[n], XtNstring, helptext2);	n++;
		break;
	case LISTTRNS:
		XtSetArg(args[n], XtNstring, helptext3);	n++;
		break;
	}

	textW = XtCreateManagedWidget(
			"helptext",
			asciiTextWidgetClass,
			localPaneW,
			args,
			n);
	XtInstallAccelerators(textW, paneW);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	boxW = XtCreateManagedWidget(
			"box",
			boxWidgetClass,
			localPaneW,
			args,
			n);

	n = 0;
	buttonW = XtCreateManagedWidget(
			"okay",
			commandWidgetClass,
			boxW,
			args,
			n);

	XtInstallAllAccelerators(helpPopupW, paneW);
	XtAddCallback (buttonW, XtNcallback, PopDownCB, True);
	XtPopup(helpPopupW, XtGrabNone);
}


/*
**  Okay, here's some really crude stuff.  We assume the text in bottextW
**  contains lines like "   Host:      YABBA.DABBA.DOO" where "Host:" is
**  the sort of thing that we call a "prompt."  We find the line, if any,
**  including the string pointed to by prompt, and return a pointer to
**  the string from whitespace to \n following the prompt.
*/

static char *
GetDefaultValue(prompt)
char	*prompt;
{
	Arg		args[5];
	unsigned int	n;
	char		*prompt1, *string, *string1, *string2;
	char		*retval;

	n = 0;
	XtSetArg(args[n], XtNstring, &string);		n++;
	XtGetValues (bottextW, args, n);

/*
**  Now we look for the prompt.
*/

	for (string1 = string; *string1; string1++) {
		for (	prompt1 = prompt, string2 = string1; 
			*prompt1 == *string2;
			prompt1++, string2++)
			;

		if (!*prompt1) break;
	}

	if (!*string1) {
		return (NULL);
	}

	for (string2 = string1; *string2 != ' '; string2++);

	for ( ; *string2 == ' '; string2++);

	*strchr (string2, '\n') = '\0';

	retval = (char *) malloc (strlen(string2) + 1);
	strcpy (retval, string2);
	*(strchr(string2, '\0')) = '\n';
	return (retval);
}
