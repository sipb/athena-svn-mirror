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

/*
**  headers.c:  manage the list of transaction headers
**
*/

#include	<stdio.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<X11/IntrinsicP.h>
#include	<X11/CoreP.h>
#include	<X11/Xaw/AsciiText.h>
#include	<X11/Xaw/TextP.h>
#include	"xdsc.h"

static char rcsid[] = "";

extern char	*RunCommand();
extern EntryRec		toplevelbuttons[2][MAX_BUTTONS];
extern TextWidget	bottextW, toptextW;
extern Boolean	debug;
extern char	filebase[];
extern int	topscreen;
extern void	TopSelect();

static void	FetchHeaders();

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
	static char	oldmeeting[LONGNAMELEN];
	static int	oldstart=0, oldfinish=0;
	Arg		args[1];

	XtSetArg(args[0], XtNsensitive, True);
	XtSetValues ((toplevelbuttons[0][5]).button, args, 1);

	if (start < TransactionNum(FIRST))
		start = TransactionNum(FIRST);

	if (finish > TransactionNum(LAST))
		finish = TransactionNum(LAST);

/*
** Can we optimize by keeping some of the old data?
*/
	if (	*oldmeeting &&
		!strcmp (oldmeeting, CurrentMtg(0)) && 
		finish == oldfinish && 
		start <= oldstart) {

/*
** Just put up old list
*/
		if (oldstart == start) {
			sprintf (filename, "%s-list", filebase);
			FileIntoWidget(filename, toptextW);
			return;
		}

/*
** Prepend to old list
*/
		sprintf (	command, "mv %s-list %s-old", 
				filebase, filebase);

		if (system (command) != 0) {
			sprintf (	command, 
					"Cannot write to '%s-old'\n",
					filebase);
			PutUpWarning("WARNING", command, False);
		}

		FetchHeaders(start, oldstart-1);

		sprintf (	command, 
				"cat %s-old >> %s-list", 
				filebase, filebase);
		if (system (command) != 0) {
			sprintf (	command, 
					"Cannot write to '%s-list'\n",
					filebase);
			PutUpWarning("WARNING", command, False);
		}

		sprintf (filename, "%s-old", filebase);
		unlink (filename);
	}
/*
** Get an entirely new list
*/
	else {
		FetchHeaders(start, finish);
	}

	sprintf (filename, "%s-list", filebase);
	FileIntoWidget(filename, toptextW);

	strcpy (oldmeeting, CurrentMtg(0));
	oldstart = start;
	oldfinish = finish;

	sprintf (command, "Reading %s [%d-%d], #%d", 
			CurrentMtg(0),
			TransactionNum(FIRST),
			TransactionNum(LAST),
			TransactionNum(CURRENT));

	PutUpStatusMessage(command);
}

/*
**  Get the headers for the specified range of transactions.  Get them in
**  packets of CHUNKSIZE if necessary, and update status line as doing so.
*/

static void
FetchHeaders(start, finish)
int	start;
int	finish;
{
	int	localstart, localfinish;
	char	command[LONGNAMELEN + 25];
	char	filename[70];
	char	*returndata;

#define	MIN(x,y)	((x) < (y) ? (x) : (y))
#define CHUNKSIZE	25

	localstart = start;
	localfinish = finish;

	sprintf (filename, "%s-list", filebase);
	unlink (filename);
	sprintf (filename, "%s-temp", filebase);
	unlink (filename);

	while (localstart <= finish) {
		localfinish = MIN (localstart + CHUNKSIZE - 1, finish);

		if (localfinish != finish)

			sprintf (	command, 
				"Reading headers for transactions %d to %d (%d remaining)...", 
				start, finish, (finish - localstart));
		else
			sprintf (	command, 
				"Reading headers for transactions %d to %d...",
				start, finish);

		if (start == finish)
			sprintf (	command, 
				"Reading header for transaction %d...",
				start);


		PutUpTempMessage(command);

		sprintf (filename, "%s-temp", filebase);
		sprintf (command, "(ls %s %d %d 0 %s)\n", filename, 
			localstart, localfinish, CurrentMtg(0));
		returndata = RunCommand (command, NULL, NULL, True);
		if ((int) returndata <= 0) {
			TakeDownTempMessage();
			sprintf (filename, "%s-temp", filebase);
			unlink (filename);
			return;
		}
		myfree (returndata);

		sprintf (	command, 
				"cat %s-temp >> %s-list", 
				filebase, filebase);
		if (system (command) != 0) {
			sprintf (	command, 
					"Cannot write to '%s-list'\n",
					filebase);
			PutUpWarning("WARNING", command, False);
		}

		localstart += CHUNKSIZE;
	}
	TakeDownTempMessage();
	sprintf (filename, "%s-temp", filebase);
	unlink (filename);
}

/*
** This should only be called if topscreen == LISTTRNS.   It moves
** the marker on the upper text widget to the line for the
** specified transaction.  If moveinsert is true, it tells PutUpArrow
** to move the text insert carat there, too.
*/

UpdateHighlightedTransaction(num, moveinsert)
int	num;
Boolean	moveinsert;
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
		PutUpArrow(toptextW, foo - tempstring, moveinsert);
	}
	if (bar)
		lastend = bar - tempstring;
}

/*
**  An arrow key has been hit in the upper text window.  If we're showing
**  transactions, see if we need to fetch another header.
*/

void
FetchIfNecessary(w, event, params, num_params)
Widget	w;
XEvent	*event;
String 	*params;
int	*num_params;
{
	TextWidget	ctx = toptextW;
	int		num;
	int		step;
	XawTextPosition	inspoint = ctx->text.insertPos;

	if (*num_params < 2)
		return;

	if (topscreen != LISTTRNS)
		return;

	step = atoi (params[1]);

	if (!strcmp(params[0], "Up")) {
		num = HighlightedTransaction();
		if (	(ctx->text.insertPos<ctx->text.lt.info[1].position) &&
			ctx->text.lt.top == 0 &&
			num != TransactionNum(FIRST)) {
			XawTextDisableRedisplay(ctx);
			PutUpTransactionList(num - step, TransactionNum(LAST));
			UpdateHighlightedTransaction(TransactionNum(CURRENT),False);
			XawTextSetInsertionPoint(ctx, inspoint + ctx->text.lt.info[step].position);
			XawTextEnableRedisplay(ctx);
		}
	}
}

/*
** Pull the transaction number out of the highlighted line and return it
*/

HighlightedTransaction()
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

	num = atoi (strchr (tempstring + start, '[') + 1);

	return (num);
}
