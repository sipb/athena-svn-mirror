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
#include	<ctype.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/Xaw/List.h>
#include	<X11/Xaw/Command.h>
#include	<X11/Xaw/Box.h>
#include	<X11/Xaw/Paned.h>
#include	<X11/Xaw/AsciiText.h>
#include	<X11/Xaw/Dialog.h>
#include	<X11/Shell.h>
#include	"xdsc.h"

static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xdsc/reply.c,v 1.13 1991-06-06 15:17:15 sao Exp $";

extern char	*strchr();
extern char     *getenv();
extern char	*RunCommand();
extern char	*CurrentMtg();
extern char	filebase[];
extern int	topscreen;
extern Widget	topW, paneW;
extern TextWidget	bottextW;
extern int	char_width;
extern char	*GetTransactionFile();
extern Boolean	nocache;

static char	sourcefile[80];

static void	SendCB();
void		TriggerSend();

static Widget	writePopupW = 0, writeTextW;
static void	WriteCB();
void		TriggerWrite();

static Widget	numPopupW = 0, numTextW;
static void	NumCB();
void		GetTransactionNum();
void		TriggerNum();

static Widget	addPopupW = 0, addHostTextW, addPathTextW;
static void	AddCB();
void		TriggerAdd();

static Widget	deletePopupW = 0, deleteMtgTextW;
static void	DeleteCB();
void		TriggerDelete();

static Widget	warningPopupW = 0;
void		DieCB();
void		TriggerPopdown();
void		TriggerFocusMove();
int		PopdownCB();

static Widget	helpPopupW = 0;

static char *GetDefaultValue();

static Boolean	CompareWithoutWhitespace();

typedef struct {
	Widget	sendPopupW;
	Widget	sendButtonW;
	Widget	subjectTextW;
	Widget	bodyTextW;
	char	mtg[80];
	int	replynum;
} SendData, *SendDataPtr;

typedef struct trnrec {
        SendDataPtr	data;
        struct trnrec	*nextrec;
} TransactionRec, *TransactionRecPtr;

static TransactionRecPtr	listhead = 0;

/*
** The structures TransactionRec and SendData are to keep track of the
** contexts for multiple enter-transaction windows.  listhead is the
** first of a chain of TransactionRec's, one per window.  This is
** so we can search through the list when a keyhit comes in trying to
** trigger a button, and figure out which button matches a particular
** text widget.
*/

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
	SendDataPtr	data;
	TransactionRecPtr	i;

	if (listhead == 0) {
		listhead = (TransactionRecPtr) malloc (sizeof (TransactionRec));
		listhead->data = 0;
		listhead->nextrec = 0;
	}

	for (i = listhead; i->data != 0 && i->nextrec != 0; i = i->nextrec)
		;

	if (i->data != 0) {
		i->nextrec = 
			(TransactionRecPtr) malloc (sizeof (TransactionRec));
		i = i->nextrec;
		i->data = 0;
		i->nextrec = 0;
	}

	data = (SendDataPtr) malloc (sizeof (SendData));

	i->data = data;

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
	SendDataPtr	data = (SendDataPtr) client_data;
	TransactionRecPtr	i, parent;

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

			for (n = 0; tempstring[n]; n++)
				if (tempstring[n] == '\n')
					tempstring[n] = ' ';

			sprintf(command, "%s\n",tempstring);

			returndata =  RunCommand (command, NULL, NULL, True);
			if ((int) returndata > 0)
				myfree(returndata);
			unlink (filename);
		}
	}

	XtDestroyWidget(data->sendPopupW);
	myfree (data);

	for (i = listhead, parent = 0; i; parent = i, i = i->nextrec) {
		if (data->subjectTextW == i->data->subjectTextW) {
			if (parent)
				parent->nextrec = i->nextrec;
			else
				listhead = i->nextrec;
			myfree (i->data);
			myfree (i);
			break;
		}
	}

	GoToTransaction (TransactionNum(CURRENT), False);

	XFlush(XtDisplay(topW));
}

void
WriteTransaction(current)
int	current;
{
	Arg		args[5];
	unsigned int	n;
	char		destfile[80];
	Widget		button, writePaneW, writeBox1W;
	Position	parentx, parenty;
	Dimension	parentwidth, mywidth;

	if (writePopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, &parentwidth);	n++;
	XtSetArg(args[n], XtNx, &parentx);		n++;
	XtSetArg(args[n], XtNy, &parenty);		n++;
	XtGetValues (topW, args, n);

	sprintf (	destfile, 
			"%s/xdsc/%s-%d", getenv("HOME"), 
			CurrentMtg(1), current);

	if (nocache)
		strcpy (sourcefile, GetTransactionFile(current));
	else
		sprintf (	sourcefile,
				"%s-%d", filebase, current);

	mywidth = 80 * char_width;

	n = 0;
	XtSetArg(args[n], XtNwidth, mywidth);		n++;
	XtSetArg(args[n], XtNtransient, True);		n++;
	XtSetArg(args[n], XtNtransientFor, topW);	n++;
	XtSetArg(args[n], XtNx, 
		parentx + ((parentwidth - mywidth) / 2));	n++;
	XtSetArg(args[n], XtNy, parenty + 100);	n++;
	writePopupW = XtCreatePopupShell(	
			"writepopup", 
			transientShellWidgetClass,
			topW,
			args,
			n);

	writePaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			writePopupW,
			NULL,
			0);

	n = 0;
	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			writePaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 15 * char_width);		n++;
	XtSetArg(args[n], XtNstring, destfile);			n++;

	writeTextW = XtCreateManagedWidget(
			"text",
			asciiTextWidgetClass,
			writePaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	writeBox1W = XtCreateManagedWidget(
			"box1",
			boxWidgetClass,
			writePaneW,
			args,
			n);

	n = 0;
	button = XtCreateManagedWidget(
			"write",
			commandWidgetClass,
			writeBox1W,
			args,
			n);

	XtAddCallback (button, XtNcallback, WriteCB, True);

	n = 0;
	button = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			writeBox1W,
			args,
			n);

	XtAddCallback (button, XtNcallback, WriteCB, False);
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
	Arg		args[5];
	unsigned int	n;

	if (!writePopupW)
		return;

	if ((Boolean) client_data) {
		n = 0;
		XtSetArg(args[n], XtNstring, &destfile);	n++;
		XtGetValues (writeTextW, args, n);

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

void
AddMeeting()
{
	Arg		args[5];
	unsigned int	n;
	char		*defaultvalue1, *defaultvalue2;
	Position	parentx, parenty;
	Dimension	parentwidth, mywidth;

	Widget		addPaneW, addBox1W, addLabel1W;
	Widget		addBox2W, addLabel2W, addBox3W, addButton1W;
	Widget		addButton2W;

	if (addPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, &parentwidth);	n++;
	XtSetArg(args[n], XtNx, &parentx);		n++;
	XtSetArg(args[n], XtNy, &parenty);		n++;
	XtGetValues (topW, args, n);

	mywidth = 80 * char_width;

	n = 0;
	XtSetArg(args[n], XtNwidth, mywidth);		n++;
	XtSetArg(args[n], XtNtransient, True);		n++;
	XtSetArg(args[n], XtNtransientFor, topW);	n++;
	XtSetArg(args[n], XtNx, 
		parentx + ((parentwidth - mywidth) / 2));	n++;
	XtSetArg(args[n], XtNy, parenty + 100);	n++;

	addPopupW = XtCreatePopupShell(	
			"addpopup",
			transientShellWidgetClass,
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

	if ((Boolean) client_data && addPopupW) {
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

/*  This forced an automatic update.  Unfortunately, it erased any list
    of transaction headers.

		TopSelect(NULL, 2, NULL);
*/
		myfree (returndata);
		myfree (tempstring1);
		myfree (tempstring2);
	}

	if (addPopupW) {
		XtDestroyWidget(addPopupW);
		XtSetKeyboardFocus(topW, topW);
	}

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
	Position	parentx, parenty;
	Dimension	parentwidth, mywidth;

	if (deletePopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, &parentwidth);	n++;
	XtSetArg(args[n], XtNx, &parentx);		n++;
	XtSetArg(args[n], XtNy, &parenty);		n++;
	XtGetValues (topW, args, n);

	mywidth = 80 * char_width;

	n = 0;
	XtSetArg(args[n], XtNwidth, mywidth);		n++;
	XtSetArg(args[n], XtNtransient, True);		n++;
	XtSetArg(args[n], XtNtransientFor, topW);	n++;
	XtSetArg(args[n], XtNx, 
		parentx + ((parentwidth - mywidth) / 2));	n++;
	XtSetArg(args[n], XtNy, parenty + 100);		n++;

	deletePopupW = XtCreatePopupShell(	
			"deletepopup",
			transientShellWidgetClass,
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
			"text",
			asciiTextWidgetClass,
			deleteBox1W,
			args,
			n);

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

	if (!deletePopupW)
		return;

	if ((Boolean) client_data) {
		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring1);		n++;
		XtGetValues (deleteMtgTextW, args, n);
/*
** Protect us when we remove the current meeting.  Watch out for leading
** and trailing spaces in the text widget's value.
*/
		if (	CompareWithoutWhitespace(tempstring1, CurrentMtg(0)) ||
			CompareWithoutWhitespace(tempstring1, CurrentMtg(1)))
			SaveMeetingNames("", "");

		sprintf (buffer, "(dm %s)\n", tempstring1);
		returndata = RunCommand (buffer, NULL, NULL, True);

		if ( (int) returndata <= 0)
			return;

/*
		TopSelect(NULL, 2, NULL);
*/
		myfree (returndata);
		myfree (tempstring1);
	}

	XtDestroyWidget(deletePopupW);
	deletePopupW = 0;
}

static Boolean
CompareWithoutWhitespace(s, t)
char	*s, *t;
{
	while (isspace(*s))
		*s++;

	while (isspace(*t))
		*t++;

	while (*s && *t && (*s++ == *t++))
		;

	if (!*t && !*s)
		return (True);

	if (!*t) {
		while (isspace(*s))
			s++;
		if (!*s)
			return (True);
		else
			return (False);
	}

	if (!*s) {
		while (isspace(*t))
			t++;
		if (!*t)
			return (True);
		else
			return (False);
	}

	return (False);
}

PutUpWarning(prefix, message, deathoption)
char	*prefix;
char	*message;
Boolean	deathoption;
{
	Arg		args[5];
	unsigned int	n;
	Widget		warningPaneW, warningButtonW, warningBoxW, deathButtonW;
	Position	parentx, parenty;
	Dimension	parentwidth, mywidth;
	int		flag = 0;

	if (warningPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, &parentwidth);	n++;
	XtSetArg(args[n], XtNx, &parentx);		n++;
	XtSetArg(args[n], XtNy, &parenty);		n++;
	XtGetValues (topW, args, n);

	mywidth = (strlen(message)+2) * char_width;

	n = 0;
	XtSetArg(args[n], XtNwidth, mywidth);		n++;
	XtSetArg(args[n], XtNtransient, True);		n++;
	XtSetArg(args[n], XtNtransientFor, topW);	n++;
	XtSetArg(args[n], XtNx, 
		parentx + ((parentwidth - mywidth) / 2));	n++;
	XtSetArg(args[n], XtNy, parenty + 100);	n++;
	warningPopupW = XtCreatePopupShell(	
			"warningpopup", 
			transientShellWidgetClass,
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

	if (*prefix) {
		n = 0;
		XtSetArg(args[n], XtNlabel, prefix);		n++;
		XtSetArg(args[n], XtNwidth, 300);		n++;
		(void) XtCreateManagedWidget(
				"label1",
				labelWidgetClass,
				warningPaneW,
				args,
				n);
	}

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

	XtAddCallback (warningButtonW, XtNcallback, PopdownCB, False);

	if (deathoption) {
		deathButtonW = XtCreateManagedWidget(
				"quit immediately",
				commandWidgetClass,
				warningBoxW,
				NULL, 0);

		XtAddCallback (deathButtonW, XtNcallback, DieCB, False);
	}

	XtInstallAllAccelerators(warningPopupW, paneW);
	XtSetKeyboardFocus(paneW, warningButtonW);

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

PopdownCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	int	didsomething = 0;

	if (warningPopupW) {
		XtDestroyWidget(warningPopupW);
		warningPopupW = 0;
		didsomething = 1;
	}

	if (helpPopupW) {
		XtDestroyWidget(helpPopupW);
		helpPopupW = 0;
		didsomething = 1;
	}
	XtSetKeyboardFocus(topW, topW);
	return (didsomething);
}


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
	XtAddCallback (buttonW, XtNcallback, PopdownCB, True);
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

void
GetTransactionNum()
{
	Arg		args[5];
	unsigned int	n;
	Widget		button, numPaneW, numBox1W;
	Position	parentx, parenty;
	Dimension	parentwidth, mywidth;

	if (numPopupW)
		return;

	n = 0;
	XtSetArg(args[n], XtNwidth, &parentwidth);	n++;
	XtSetArg(args[n], XtNx, &parentx);		n++;
	XtSetArg(args[n], XtNy, &parenty);		n++;
	XtGetValues (topW, args, n);

	mywidth = 35 * char_width;

	n = 0;
	XtSetArg(args[n], XtNwidth, mywidth);		n++;
	XtSetArg(args[n], XtNtransient, True);		n++;
	XtSetArg(args[n], XtNtransientFor, topW);	n++;
	XtSetArg(args[n], XtNx, 
		parentx + ((parentwidth - mywidth) / 2));	n++;
	XtSetArg(args[n], XtNy, parenty + 100);		n++;

	numPopupW = XtCreatePopupShell(	
			"numpopup", 
			transientShellWidgetClass,
			topW,
			args,
			n);

	numPaneW = XtCreateManagedWidget(
			"pane",
			panedWidgetClass,
			numPopupW,
			NULL,
			0);

	n = 0;

	(void) XtCreateManagedWidget(
			"label",
			labelWidgetClass,
			numPaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	XtSetArg(args[n], XtNeditType, XawtextEdit);		n++;
	XtSetArg(args[n], XtNwidth, 15 * char_width);		n++;

	numTextW = XtCreateManagedWidget(
			"text",
			asciiTextWidgetClass,
			numPaneW,
			args,
			n);

	n = 0;
	XtSetArg(args[n], XtNborderWidth, 0);			n++;
	numBox1W = XtCreateManagedWidget(
			"box1",
			boxWidgetClass,
			numPaneW,
			args,
			n);

	n = 0;
	button = XtCreateManagedWidget(
			"goto",
			commandWidgetClass,
			numBox1W,
			args,
			n);

	XtAddCallback (button, XtNcallback, NumCB, True);

	n = 0;
	button = XtCreateManagedWidget(
			"abort",
			commandWidgetClass,
			numBox1W,
			args,
			n);

	XtAddCallback (button, XtNcallback, NumCB, False);
	XtPopup(numPopupW, XtGrabNone);
}

static void
NumCB(w, client_data, call_data)
Widget	w;
XtPointer	client_data;
XtPointer	call_data;
{
	char		*tempstring;
	Arg		args[5];
	unsigned int	n;

	if (!numPopupW)
		return;

	if ((Boolean) client_data) {
		n = 0;
		XtSetArg(args[n], XtNstring, &tempstring);		n++;
		XtGetValues (numTextW, args, n);
		GoToTransaction(atoi(tempstring), True);
		myfree (tempstring);
	}

	XtDestroyWidget(numPopupW);
	XtSetKeyboardFocus(topW, topW);
	numPopupW = 0;
}

void
TriggerAdd(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	if (*num_params < 1)
		return;

	AddCB(w, (XtPointer) !strcmp(params[0], "Go"), NULL);
}

void
TriggerNum(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	if (*num_params < 1)
		return;

	NumCB(w, (XtPointer) !strcmp(params[0], "Go"), NULL);
}

void
TriggerWrite(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	if (*num_params < 1)
		return;

	WriteCB(w, (XtPointer) !strcmp(params[0], "Go"), NULL);
}

void
TriggerDelete(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	if (*num_params < 1)
		return;

	DeleteCB(w, (XtPointer) !strcmp(params[0], "Go"), NULL);
}

void
TriggerPopdown(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	if (*num_params < 1)
		return;

	PopdownCB(w, (XtPointer) !strcmp(params[0], "Go"), NULL);
}

void
TriggerSend(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	TransactionRecPtr	i;

	if (*num_params < 1)
		return;

	if (listhead == 0) {
		return;
	}

	for (i = listhead; i; i = i->nextrec) {
		if (w == i->data->subjectTextW)
			break;
		if (w == i->data->bodyTextW)
			break;
		if (w == i->data->sendButtonW)
			break;
		if (w == i->data->sendPopupW)
			break;
	}

	if (i == 0)
		return;

/*
** Okay, here's a kludge.  SendCB() checks the incoming widget ID,
** and if it's equal to the sendButtonW stored in the data structure,
** it actually does the send.  So we fool it here.
*/
	if (strcmp(params[0], "Go") == 0)
		SendCB(i->data->sendButtonW, i->data, NULL);
	else
		SendCB(NULL, i->data, NULL);
}

/*
** Pass TriggerFocusMove "Prev" to move focus up, "Next" to move it down.
** It figures out which widget got the event and moves focus to
** the appropriate neighbor.
*/

void
TriggerFocusMove(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	TransactionRecPtr	i;
	int			direction = -1;
	Widget			targetW = 0;
	Widget			shellparent;

	if (*num_params < 1)
		return;

	for (	shellparent = XtParent(w); 
		shellparent && !XtIsShell(shellparent);
		shellparent = XtParent(shellparent))
		;

	if (!strcmp (params[0], "Next"))
		direction = 2;
	if (!strcmp (params[0], "Prev"))
		direction = 1;
	if (!strcmp (params[0], "Toggle"))
		direction = 0;

	if (!strcmp (params[0], "Here")) {
		XtSetKeyboardFocus(topW, shellparent);
		XtSetKeyboardFocus(shellparent, w);
		return;
	}

/*
** Was the hit widget in the add-mtg popup?  All keys become toggle.
*/
	if (addPopupW) {
		if (w == addHostTextW)
			targetW = addPathTextW;
		if (w == addPathTextW)
			targetW = addHostTextW;
	}

/*
** Check the subject and body widgets in the list of entryrecs.
*/
	if (listhead != 0) {
		for (i = listhead; i; i = i->nextrec) {
			if (w == i->data->subjectTextW && 
					(direction == 2 || direction == 0))
				targetW = i->data->bodyTextW;
			if (w == i->data->bodyTextW && 
					(direction == 1 || direction == 0))
				targetW = i->data->subjectTextW;
		}
	}

	if (targetW) {
		XtSetKeyboardFocus(topW, shellparent);
		XtSetKeyboardFocus(shellparent, targetW);
	}
}
