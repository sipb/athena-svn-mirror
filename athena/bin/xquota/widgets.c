/*
 * xquota -  X window system quota display program.
 *
 * $Athena: man.c,v 1.7 89/02/15 16:06:44 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   March 5, 1989
 */

#if ( !defined(lint) && !defined(SABER))
  static char rcs_version[] = "$Athena: man.c,v 4.7 89/02/15 20:09:34 kit Locked $";
#endif

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/AsciiText.h>
#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/Scroll.h>
#include <X11/Shell.h>
#include <X11/Paned.h>

#include "xquota.h"

static void CheckProc(), DoneProc(), QuitProc(), TopHelpProc();
static void FlipColors(), TimeOutProc(), PopupHelpProc(), KillHelpProc();
static void CreateButtonPane(), CreateQuotaDisplay();
static void QuitAction(), PopInfoAction(), QuotaCheckAction();
static void PrintHelpAction();
static void LoadApplicationResources();
static Widget CreateHelpWidget();
static void ShowTopHelp(), ShowPopupHelp();

extern Info info;			/* external for action proc only. */

#define Offset(field) (XtOffset(Info *, field))

static XtResource quota_resources[] = {
  {"warning", "Warning", XtRInt, sizeof(int),
     Offset(warn), XtRImmediate, (caddr_t) DEFAULT_WARNING},
  {"update", "Update", XtRInt, sizeof(int),
     Offset(minutes), XtRImmediate, (caddr_t) DEFAULT_UPDATE},
  {"numbers", "Numbers", XtRBoolean, sizeof(Boolean),
     Offset(numbers), XtRImmediate, (caddr_t) TRUE},
  {"spaceSaver", "SpaceSaver", XtRBoolean, sizeof(Boolean),
     Offset(space_saver), XtRImmediate, (caddr_t) FALSE},
  {"topHelpFile", "TopHelpFile", XtCString, sizeof(String),
     Offset(top_help_file), XtRString, (caddr_t) TOP_HELP_FILE},
  {"popupHelpFile", "PopupHelpFile", XtCString, sizeof(String),
     Offset(popup_help_file), XtRString, (caddr_t) POPUP_HELP_FILE},
};

static XrmOptionDescRec quota_options[] = {
{"-warning",    ".warning",    XrmoptionSepArg, (caddr_t) NULL},
{"-update",     ".update",     XrmoptionSepArg, (caddr_t) NULL},
{"-nonumbers",  ".numbers",    XrmoptionNoArg,  (caddr_t) "off"},
{"-spacesaver", ".spaceSaver", XrmoptionNoArg,  (caddr_t) "on"},
};

static XtActionsRec actions[] = {
  { "PopInfo", PopInfoAction },
  { "QuotaCheck", QuotaCheckAction },
  { "PrintHelp", PrintHelpAction },
  { "Quit", QuitAction }
};

/*	Function Name: InitializeWorld
 *	Description: Initializes everything for the quota check program.
 *                   In preparation for createing widgets.
 *	Arguments: info - the psuedo globals struct.
 *                 argc, and argv - command line args.
 *	Returns: the top level shell widget.
 *
 * NOTES: sets the applications context.
 */

Widget
InitializeWorld(info, argc, argv)
Info * info;
int * argc;
char ** argv;
{
  Widget shell;
  Display * disp;

  XtToolkitInitialize();
  info->appcon = XtCreateApplicationContext();
  disp = XtOpenDisplay(info->appcon, NULL, argv[0], APP_CLASS, quota_options,
		       XtNumber(quota_options), argc, argv);
  LoadApplicationResources(DEFAULTS_FILE, disp);
  shell = XtAppCreateShell(NULL, APP_CLASS, applicationShellWidgetClass,
			   disp, NULL, (Cardinal) 0);
  XtGetApplicationResources(shell, (caddr_t) info, 
			    quota_resources, XtNumber(quota_resources),
			    NULL, (Cardinal) 0);

  XtAppAddActions(info->appcon, actions, XtNumber(actions));

/*
 * A few final checks.
 */

  info->flipped = FALSE;
  info->timeout = NULL;
  info->top_help = info->popup_help = NULL;

  if (info->minutes < MIN_UPDATE) {
    printf("The minimum update time is every %d minutes.\n", MIN_UPDATE);
    info->minutes = MIN_UPDATE;
  }
    
  return(shell);
}

/*	Function Name: CreateWidgets
 *	Description: Creates all widgets that are decendants of the shell.
 *	Arguments: shell - the shell widget that all these are decended from.
 *                 info - structure with psuedo global info.
 *	Returns: none.
 */

void
CreateWidgets(shell, info)
Widget shell;
Info * info;
{
  Arg arglist[2];
  Cardinal num_args;
  Widget pane;

  pane = XtCreateManagedWidget("topPane", panedWidgetClass, shell, 
			       NULL, (Cardinal) 0);
  
  if (info->numbers && !info->space_saver) {
    info->quota_top = XtCreateManagedWidget("quota", labelWidgetClass, pane, 
					NULL, (Cardinal) 0);

    info->used_top = XtCreateManagedWidget("used", labelWidgetClass, pane, 
				       NULL, (Cardinal) 0);
  }

  num_args = 0;
  XtSetArg( arglist[num_args], XtNtranslations, 
	    XtParseTranslationTable("")); num_args++;
  info->graph = XtCreateManagedWidget("graph", scrollbarWidgetClass, pane, 
				      arglist, num_args);

  if (!info->space_saver) {
    Widget help;
    num_args = 0;
    XtSetArg( arglist[num_args], XtNskipAdjust, TRUE); num_args++;
    help = XtCreateManagedWidget("help", commandWidgetClass, pane, 
				 arglist, num_args);
    XtAddCallback(help, XtNcallback, TopHelpProc, (caddr_t) info);
  }

  CreateInfoPopup(info);
  ShowQuota(info);
}

/*	Function Name: CreateInfoPopup
 *	Description: Creates the popup that will scream at you.
 *	Arguments: info - the info structure.
 *	Returns: none.
 */

void
CreateInfoPopup(info)
Info * info;
{
  Widget pane;
  Arg arglist[5];
  Cardinal num_args;
  Dimension width, height;
  Position x, y;
  char buf[BUFSIZ], *string;

  info->info_popup = XtCreatePopupShell("quotaInfo", transientShellWidgetClass,
				    info->graph, NULL, 0);

  pane = XtCreateManagedWidget("infoPane", panedWidgetClass, info->info_popup,
			       NULL, 0);

  num_args = 0;
  XtSetArg( arglist[num_args], XtNlabel, buf); num_args++;  

  sprintf(buf, "%s - %s", VERSION, AUTHOR);
  (void) XtCreateManagedWidget("version", labelWidgetClass, pane, 
			       arglist, num_args);

  sprintf(buf, "Machine:  %s - Filesystem:  %s ", info->host, info->path);
  (void) XtCreateManagedWidget("machine", labelWidgetClass, pane, 
			       arglist, num_args);

  CreateQuotaDisplay(info, pane);

  info->last_widget = XtCreateManagedWidget("last", labelWidgetClass, pane, 
					    NULL, (Cardinal) 0);

  string = XtMalloc( sizeof(char) * TEXT_BUFFER_LENGTH);
  strcpy(string, SAFE_MESSAGE);

  num_args = 0;
  XtSetArg( arglist[num_args], XtNtextOptions, scrollVertical | wordBreak);
  num_args++;  
  XtSetArg( arglist[num_args], XtNeditType, XttextEdit); num_args++;  
  XtSetArg( arglist[num_args], XtNlength, TEXT_BUFFER_LENGTH); num_args++;  
  XtSetArg( arglist[num_args], XtNstring, string); num_args++;  
  info->message_widget = XtCreateManagedWidget("message", 
					       asciiStringWidgetClass, pane, 
					       arglist, num_args);

  CreateButtonPane(info, pane);
  XtRealizeWidget(info->info_popup);

  num_args = 0;
  XtSetArg( arglist[num_args], XtNwidth, &width); num_args++;
  XtSetArg( arglist[num_args], XtNheight, &height);num_args++;
  XtGetValues(info->info_popup, arglist, num_args);
  
  x = (Position) (XtScreen(info->info_popup)->width - width)/2;
  y = (Position) (XtScreen(info->info_popup)->height - height)/2;

  num_args = 0;
  XtSetArg( arglist[num_args], XtNx, x); num_args++;  
  XtSetArg( arglist[num_args], XtNy, y); num_args++;  
  XtSetValues(info->info_popup, arglist, num_args);
}

/*	Function Name: CreateQuotaDisplay
 *	Description: Displays quota information.
 *	Arguments: info - the info struct.
 *                 parent - its parent.
 *	Returns: none
 */

static void
CreateQuotaDisplay(info, parent)
Info * info;
Widget parent;
{
  char buf[BUFSIZ];
  Widget pane;
  Arg arglist[10];
  Cardinal num_args;

  pane = XtCreateManagedWidget("quotaPane", panedWidgetClass, parent,
			       NULL, (Cardinal) 0);

  sprintf(buf, "%12s %10s %10s", "", "Kilobytes", "Files");
  num_args = 0;
  XtSetArg( arglist[num_args], XtNlabel, buf); num_args++;  
  (void) XtCreateManagedWidget("Types", labelWidgetClass, pane, 
			       arglist, num_args);

  info->hard_widget = XtCreateManagedWidget("hard", labelWidgetClass, pane, 
					    NULL, (Cardinal) 0);
  info->soft_widget = XtCreateManagedWidget("soft", labelWidgetClass, pane, 
					    NULL, (Cardinal) 0);
  info->used_widget = XtCreateManagedWidget("used", labelWidgetClass, pane, 
					    NULL, (Cardinal) 0);
}

/*	Function Name: CreateButtonPane
 *	Description: Create the pane with the command buttons in it.
 *	Arguments: info - info struct.
 *                 parent - parent widget.
 *	Returns: none.
 */

static void
CreateButtonPane(info, parent)
Info * info;
Widget parent;
{
  Widget hpane, check, help, done, quit;
  Arg arglist[2];
  Cardinal num_args;

  num_args = 0;
  XtSetArg( arglist[num_args], XtNvertical, FALSE); num_args++;  
  XtSetArg( arglist[num_args], XtNskipAdjust, TRUE); num_args++;  
  hpane = XtCreateManagedWidget("hPane", panedWidgetClass, parent,
				arglist, num_args);

  done = XtCreateManagedWidget("done", commandWidgetClass, hpane, 
			       NULL, (Cardinal) 0);

  help = XtCreateManagedWidget("help", commandWidgetClass, hpane, 
			       NULL, (Cardinal) 0);

  check = XtCreateManagedWidget("check", commandWidgetClass, hpane, 
			       NULL, (Cardinal) 0);
  num_args = 0;
  XtSetArg( arglist[num_args], XtNmappedWhenManaged, FALSE); num_args++;  
  (void) XtCreateManagedWidget("spacer", labelWidgetClass, hpane, 
			       arglist, num_args);
  num_args = 0;
  XtSetArg( arglist[num_args], XtNskipAdjust, TRUE); num_args++;  
  quit = XtCreateManagedWidget("quit", commandWidgetClass, hpane, 
			       arglist, num_args);

  XtAddCallback(help,  XtNcallback, PopupHelpProc, (caddr_t) info);
  XtAddCallback(check, XtNcallback, CheckProc,     (caddr_t) info);
  XtAddCallback(done,  XtNcallback, DoneProc,      (caddr_t) info);
  XtAddCallback(quit,  XtNcallback, QuitProc,      (caddr_t) NULL);
}

/*	Function Name: CreateHelpWidget
 *	Description: Creates a help widget.
 *	Arguments: filename - name of file to load.
 *                 parent - its parent widget.
 *	Returns: the help widget we just created.
 */

static Widget 
CreateHelpWidget(filename, parent)
String filename;
Widget parent;
{
  Widget shell, pane, kill;
  Arg arglist[4];
  Cardinal num_args = 0;

  shell = XtCreatePopupShell("help", topLevelShellWidgetClass,
			     parent, NULL, 0);

  pane = XtCreateManagedWidget("helpPane", panedWidgetClass, shell, 
			       arglist, num_args);

  kill = XtCreateManagedWidget("killHelp", commandWidgetClass, pane, 
			       arglist, num_args);

  XtAddCallback(kill, XtNcallback, KillHelpProc, (caddr_t) shell);
  
  num_args = 0;
  XtSetArg( arglist[num_args], XtNtextOptions, scrollVertical | wordBreak ) ;
  num_args++; 
  XtSetArg( arglist[num_args], XtNfile, filename ) ;
  num_args++; 
  (void) XtCreateManagedWidget("text", asciiDiskWidgetClass, pane,
			       arglist, num_args);
  return(shell);
}
  
/*	Function Name: ShowTopHelp
 *	Description: Show the top help.
 *	Arguments: info - the info struct.
 *	Returns: none
 */

static void
ShowTopHelp(info)
Info * info;
{
  if (info->top_help == NULL) 
    info->top_help = CreateHelpWidget(info->top_help_file, info->graph);

  XtPopup(info->top_help, XtGrabNone);
}

/*	Function Name: ShowPopupHelp
 *	Description: Show the popup help.
 *	Arguments: info - the info struct.
 *	Returns: none.
 */

static void
ShowPopupHelp(info)
Info * info;
{
  if (info->popup_help == NULL) 
    info->popup_help = CreateHelpWidget(info->popup_help_file,
					info->info_popup);

  XtPopup(info->popup_help, XtGrabNone);
}


/************************************************************
 *
 *  Action Proceedures.
 *
 ************************************************************/

/*	Function Name: PopInfoAction
 *	Description: Pops information window.
 *	Arguments: w - the widget that got the proceedure.    ** NOT USED **
 *                 event - the event that caused the actions. ** NOT USED **
 *                 params, num_params - args passed to proc.  
 *	Returns: none.
 */

/* ARGSUSED */
static void
PopInfoAction(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  if (*num_params != 1) {
    printf("PopInfo: number of parameters passed was not 1.\n");
    return;
  }
  switch (params[0][0]) {
  case 'U':
  case 'u':
    XtPopup(info.info_popup, XtGrabNone);
    break;
  case 'D':
  case 'd':
    XtPopdown(info.info_popup);
    break;
  default:
    printf("PopInfo: specify either 'Up' or 'Down'.\n");
    break;
  }
}

/*	Function Name: QuitAction
 *	Description: exits
 *	Arguments: w - the widget that got the proceedure.    ** NOT USED **
 *                 event - the event that caused the actions. ** NOT USED **
 *                 params, num_params - args passed to proc.  ** NOT USED **
 *	Returns: none.
 */

/* ARGSUSED */
static void
QuitAction(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  exit(0);
}

/*	Function Name: QuotaCheckAction
 *	Description: checks the quota
 *	Arguments: w - the widget that got the proceedure.    ** NOT USED **
 *                 event - the event that caused the actions. ** NOT USED **
 *                 params, num_params - args passed to proc.  ** NOT USED **
 *	Returns: none.
 */

/* ARGSUSED */
static void
QuotaCheckAction(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  ShowQuota( &info);
}

/*	Function Name: PrintHelpAction
 *	Description: Prints help.
 *	Arguments: w - the widget that got the proceedure.    ** NOT USED **
 *                 event - the event that caused the actions. ** NOT USED **
 *                 params, num_params - args passed to proc.  ** NOT USED **
 *	Returns: none.
 */

/* ARGSUSED */
static void
PrintHelpAction(w, event, params, num_params)
Widget w;
XEvent * event;
String * params;
Cardinal * num_params;
{
  if (*num_params != 1) {
    printf("PrintHelp: number of parameters passed was not 1.\n");
    return;
  }
  switch (params[0][0]) {
  case 'T':
  case 't':
    ShowTopHelp( &info );
    break;
  case 'P':
  case 'p':
    ShowPopupHelp( &info );
    break;
  default:
    printf("PrintHelp: specify either 'Top' or 'Popup'.\n");
    break;
  }
}

/************************************************************
 *
 *  Callback Proceedures.
 *
 ************************************************************/
  
/*	Function Name: CheckProc
 *	Description: Checks the user's quota.
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 info - a pointer the info structure.
 *                 garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
CheckProc(w, info_ptr, garbage)
Widget w;
caddr_t info_ptr, garbage;
{
  ShowQuota( (Info *) info_ptr);
}
  
/*	Function Name: QuitProc
 *	Description: exits
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 junk, garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
QuitProc(w, junk, garbage)
Widget w;
caddr_t junk, garbage;
{
  exit(0);
}
 
/*	Function Name: DoneProc
 *	Description: pops down info widget
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 info - a pointer the info structure.
 *                 garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
  
static void
DoneProc(w, info_ptr, garbage)
Widget w;
caddr_t info_ptr, garbage;
{
  Info * info = (Info *) info_ptr;
  XtPopdown(info->info_popup);
}
  
/*	Function Name: TopHelpProc
 *	Description: Print Top pane help info.
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 info - a pointer the info structure.
 *                 garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
TopHelpProc(w, info_ptr, garbage)
Widget w;
caddr_t info_ptr, garbage;
{
  ShowTopHelp( (Info *) info_ptr);
}
   
/*	Function Name: PopupHelpProc
 *	Description: Print Top pane help info.
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 info - a pointer the info structure.
 *                 garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
PopupHelpProc(w, info_ptr, garbage)
Widget w;
caddr_t info_ptr, garbage;
{
  ShowPopupHelp( (Info *) info_ptr);
}

/*	Function Name: KillHelpProc
 *	Description: removes the help window.
 *	Arguments: w - the widget that actived this proc. ** NOT USED **.
 *                 help - a pointer the help shell.
 *                 garbage - ** NOT USED **.
 *	Returns: none.
 */

/* ARGSUSED */
static void
KillHelpProc(w, help, garbage)
Widget w;
caddr_t help, garbage;
{
  XtPopdown( (Widget) help);
}
  
/************************************************************
 *
 *  Timeout proccedures.
 *
 ************************************************************/

/*	Function Name: SetTimeout
 *	Description: Sets the timeout for next quota check.
 *	Arguments: info - the psuedo global structure.
 *	Returns: none.
 */

#define MILLIES_PER_MINUTE 60000L

void
SetTimeOut(info)
Info * info;
{
  long time;

  if (info->timeout != NULL)	/* remove old timeout. */
    XtRemoveTimeOut(info->timeout);

  time = ((long) info->minutes ) * MILLIES_PER_MINUTE;
  info->timeout = XtAppAddTimeOut(info->appcon, time, 
				  TimeOutProc, (caddr_t) info);
}

/*	Function Name: TimeOutProc
 *	Description: This proceedure gets called when the timer times out.
 *	Arguments: info_ptr - pointer to info structure.
 *                 id - id of timeout ** NOT USED **.
 *	Returns: none
 */

/* ARGSUSED */
static void
TimeOutProc(info_ptr, id)
caddr_t info_ptr;
XtIntervalId *id;
{
  Info * info = (Info *) info_ptr;
  info->timeout = NULL;
  ShowQuota(info);
}  

/************************************************************
 *
 *  Widget Utility Functions.
 *
 ************************************************************/

/*	Function Name: SetLabel
 *	Description: Sets the label in a widget.
 *	Arguments: w - the widget who's label we are changing.
 *                 str - string to change it to.
 *	Returns: none.
 */

void
SetLabel(w, string)
Widget w;
char * string;
{
  Arg arglist[1];
  
  XtSetArg(arglist[0], XtNlabel, string);
  XtSetValues(w, arglist, (Cardinal) 1);
}

/*	Function Name: FlipColors
 *	Description: Swaps fg and bg colors.
 *	Arguments: w - widget to flip.
 *	Returns: none.
 */

static void
FlipColors(w)
Widget w;
{
  Arg arglist[2];
  Pixel fg, bg;
  Cardinal num_args;
  
  num_args = 0;
  XtSetArg(arglist[num_args], XtNforeground, &fg); num_args++;
  XtSetArg(arglist[num_args], XtNbackground, &bg); num_args++;
  XtGetValues(w, arglist, num_args);
  
  num_args = 0;
  XtSetArg(arglist[num_args], XtNforeground, bg); num_args++;
  XtSetArg(arglist[num_args], XtNbackground, fg); num_args++;
  XtSetValues(w, arglist, num_args);
}
  
/*	Function Name: SetGraph
 *	Description: Sets the quota graph.
 *	Arguments: w - the graph widget.
 *                 quota - quota in kilobytes.
 *                 used - amount of space used in kilobytes.
 *	Returns: none.
 */

void
SetGraph(info, w, quota, used)
Info * info;
Widget w;
int quota, used;
{
  float temp;

  temp = ( (float) used ) / ( (float) quota);
  if ( temp > 1.0 ) temp = 1.0;
  XtScrollBarSetThumb( w, 1.0 - temp, temp );
  temp *= 100;
  if ( (((int) temp) >= info->warn && !info->flipped) ||
       (((int) temp) < info->warn && info->flipped) ) {
    FlipColors(w);
    if (info->numbers)
      FlipColors(info->used_top);
    FlipColors(info->used_widget);
    info->flipped = !info->flipped;
  }
}

/*	Function Name: SetTextMessage
 *	Description: Sets the text in the text widget to be "string".
 *	Arguments: w - the text widget.
 *                 string - string to show in text widget.
 *	Returns: none.
 */

void
SetTextMessage(w, string)
Widget w;
char * string;
{
  XtTextBlock t_block;

  t_block.firstPos = 0;
  t_block.length = strlen(string);
  t_block.ptr = string;
  t_block.format = FMT8BIT;

  if (XtTextReplace(w, 0, TEXT_BUFFER_LENGTH, &t_block) != XawEditDone)
    printf("Xquota Error: could not replace text string.\n");
}

/************************************************************
 *
 *  Get Alternate Application Resources.
 *
 *  Since I cannot write to /usr/lib/X11/app-defaults
 *  I have to get my application resources from somewhere else.
 *  Here is one method.
 *
 ************************************************************/

/*	Function Name: LoadApplicationResources
 *	Description: Loads in my own personal app-defaults file
 *	Arguments: file - name of the defaults file.
 *                 disp - the display.
 *	Returns: none.
 */

static void
LoadApplicationResources(file, disp)
char * file;
Display * disp;
{
  XrmDatabase my_database;

  if (file == NULL || streq(file, "") ) return;

  if ( (my_database = XrmGetFileDatabase(file)) == NULL ) {
    printf("Could not open file %s to read resources.\n", file);
    return;
  }

  XrmMergeDatabases(my_database, &(disp->db));
}
