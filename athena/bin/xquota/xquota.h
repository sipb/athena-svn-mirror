/*
 * xquota -  X window system quota display program.
 *
 * $Athena: xquota.h,v 1.3 89/03/10 18:36:39 kit Locked $
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

#include <X11/Intrinsic.h>

#define APP_CLASS ("Xquota")

#define VERSION   ("Xquota Version 0.1")
#define AUTHOR    ("Chris D. Peterson")

#ifndef DEFAULTS_FILE
#define DEFAULTS_FILE ("./Xquota.ad")
#endif /* DEFAULTS_FILE */

#ifndef TOP_HELP_FILE
#define TOP_HELP_FILE ("./top_help.txt")
#endif /* TOP_HELP_FILE */

#ifndef POPUP_HELP_FILE
#define POPUP_HELP_FILE ("./pop_help.txt")
#endif /* POPUP_HELP_FILE */

/*
 * This must be enough to hold each of the following when it is formatted.
 */

#define TEXT_BUFFER_LENGTH 5000	

#define SAFE_MESSAGE    ("You Are Safely Under Quota.")

#define WARNING_MESSAGE ("Warning - You are approaching your disk quota")

#define EXCEEDED_SOFT   ("****  WARNING  **** You have exceeded your disk \
quota.  You must remove %d %s by %s")

#define EXCEEDED_HARD   ("*** EXTREME DANGER *** You have exceeded your \
disk quota.  You must remove %d %s IMMEDIATELY.  All files are subject to \
deletion or truncation until get under quota.")

#define DEFAULT_WARNING 80
#define DEFAULT_UPDATE  15
#define MIN_UPDATE       5

#define QUOTA_OK         0
#define QUOTA_ERROR      1
#define QUOTA_NONE       2
#define QUOTA_PERMISSION 3

typedef struct _Info {
  int uid;			/* real uid of user who started the program. */
  char *host;			/* host of users home filsys. */
  char *path;			/* path of users home filsys. */
  Widget graph,			/* graphing widget. */
    quota_top,			/* "soft" quota widget. */
    used_top,			/* amount of file space used. */
    info_popup,			/* shell of popup information widget. */
    hard_widget,		/* Widget to display hard quotas. */
    soft_widget,		/* Widget to display soft quotas. */
    used_widget,		/* Widget to display space used. */
    last_widget,		/* Widget to display last check time. */
    message_widget;		/* Text Widget with message. */
  XtAppContext appcon;		/* The applicaions app context. */
  Boolean flipped;		/* True if we have already flipped this disp.*/
  XtIntervalId timeout;		/* id of active timeout. */
  Widget top_help,		/* top help widget. */
    popup_help;			/* popup help widget. */

  /* resources. */
  int warn;			/* If "used/quota * 100" is greater or equal to
				   this then we warn the user.*/
  int minutes;			/* Time to next update in minutes. */
  Boolean numbers;		/* If TRUE then display numbers as well as 
				   graph. */
  Boolean space_saver;		/* If TRUE then the space saver is on
				   and do not display anything but the graph
				   in the top window. */
  String top_help_file;		/* Tophelp filename. */
  String popup_help_file;	/* Popup help filename. */
} Info;

/************************************************************
 * 
 * Functions and Macro Definitions.
 *
 ************************************************************/

#define streq(a,b) ( strcmp( (a), (b) ) == 0)

/*
 * Standard C Lib functions.
 */

void exit();

/*
 * widgets.c
 */

Widget InitializeWorld();
void CreateWidgets(), CreateInfoPopup();
void SetLabel(), SetGraph(), SetTimeOut(), SetTextMessage();

/*
 * Handler.c
 */

void ShowQuota();

/*
 * Quota.c
 */

int getnfsquota();
