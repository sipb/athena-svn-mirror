/*
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * MotifUtils:   Utilities for use with Motif and UIL
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuGetBoolean.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 * 
 * SyncDialogs.c
 * This file contains the functions MuGetBoolean, MuGetString, and
 * MuGetFileName. These popup a modal dialog box for communication with 
 * the user.  They provide a synchronous interface to the programmer.
 * That is, like gets() or scanf() they return the desired value 
 * directly to the programmer, without any intervening callbacks. 
 * This is accomplished by invoking a recursive copy of XtMainLoop
 * inside of the each utility routine, and using longjmp() from within
 * the ok and cancel callbacks to abort from that recursive copy.
 * This is witchcraft, but provides powerful interface tools to the
 * programmer.  
 * For additional convenience, these functions never take compound 
 * strings as arguments.  All strings are passed as char *.
 *
 */

#include "MuP.h"
#include <Xm/MessageB.h>
#include <setjmp.h>

/*
 *  MuGetBoolean
 *
 * Uses an XmMessageDialog widget to prompt the user for an answer to
 * a yes-or-no type question.  
 * Inputs:
 * 	prompt		The question to present the user.  
 * 	yeslabel	The text to appear on the "True" button.  
 * 	nolabel		The text to appear on the "False" button.
 * 	helptext	The text to appear when the "Help" button is clicked.
 * 			If NULL is specified, the help button will not appear.
 * 	defaultbutton	A Boolean indicating which button should be the default
 * Returns:
 * 	a Boolean indicating which button was pressed.
 */

Boolean MuGetBoolean(prompt, yeslabel, nolabel, helptext, defaultbutton)
char *prompt, *yeslabel, *nolabel;
char *helptext;
int defaultbutton;
{
    Arg args[5];
    int n;
    jmp_buf mark;
    int status;
    
    static Widget BooleanDialog = NULL;
    static Widget HelpButton = NULL;
   
    if (BooleanDialog == NULL) {        /* if widget doesn't exist yet */
	n = 0;
	/* XtSetArg(args[n], XmNdialogType, XmDIALOG_QUESTION); n++; */ 
	XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;
        XtSetArg(args[n], XmNborderWidth, 2); n++;
	XtSetArg(args[n], XmNautoUnmanage,False); n++;
	BooleanDialog = XmCreateQuestionDialog(_MuToplevel,
					       "_MuBooleanDialog",args,n);
	HelpButton = XmMessageBoxGetChild(BooleanDialog, XmDIALOG_HELP_BUTTON);
	XtRealizeWidget(BooleanDialog);
	MuSetStandardCursor(BooleanDialog);
    }
    n = 0;
    XtSetArg(args[n],XmNmessageString,
	     XmStringLtoRCreate(prompt, XmSTRING_DEFAULT_CHARSET) ); n++;
    XtSetArg(args[n],XmNokLabelString,
             XmStringLtoRCreate(yeslabel, XmSTRING_DEFAULT_CHARSET)); n++;
    XtSetArg(args[n],XmNcancelLabelString,
             XmStringLtoRCreate(nolabel, XmSTRING_DEFAULT_CHARSET)); n++;
    if (defaultbutton == True) 
	XtSetArg(args[n],XmNdefaultButtonType, XmDIALOG_OK_BUTTON);
    else
	XtSetArg(args[n],XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON);
    n++;
    XtSetValues(BooleanDialog, args, n);

    XtAddCallback(BooleanDialog, XmNokCallback, _MuOkCallback, mark);
    XtAddCallback(BooleanDialog, XmNcancelCallback, _MuCancelCallback, mark);

    if (helptext != NULL) {
	if (!XtIsManaged(HelpButton)) XtManageChild(HelpButton);
	XtAddCallback(BooleanDialog,XmNhelpCallback,_MuHelpCallback, helptext);
    }
    else {   
	if (XtIsManaged(HelpButton)) XtUnmanageChild(HelpButton);
    } 
  
    if (!XtIsManaged(BooleanDialog))
	XtManageChild(BooleanDialog);

    switch (status = setjmp(mark)) {
    case 0:
	XtMainLoop();
    case 1:
    case 2:
      	XtUnmanageChild(BooleanDialog);
	XtRemoveCallback(BooleanDialog,XmNokCallback, _MuOkCallback, mark);
	XtRemoveCallback(BooleanDialog,XmNcancelCallback,
			 _MuCancelCallback, mark);
	if (helptext != NULL) XtRemoveCallback(BooleanDialog,XmNhelpCallback,
					       _MuHelpCallback,helptext);
     }
    return((status==1)?True:False);
}


