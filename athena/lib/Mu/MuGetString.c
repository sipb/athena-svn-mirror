/*
 *
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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuGetString.c,v $
 * $Author: djf $
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
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <setjmp.h>

/*
 * MuGetString
 * Uses a PromptDialog convenience widget to prompt the user to input
 * a string.  The string is returned in a passed buffer, and the
 * function returns a Boolean indicating whether the "Ok" or the
 * "Cancel"  button was pressed.
 * Inputs:
 * 	prompt		The string to prompt the user with.
 * 	buffer		A pointer to a buffer to return the string in.
 * 			The contents of the buffer will be displayed as
 * 			the default string.
 * 	bufsize		The size in bytes of the buffer.
 * 	helptext	The text to display if the help button is
 *			clicked.  If NULL, no Help button will be
 * 			displayed.
 * Outputs:
 * 	buffer		Contains the NULL-terminated string the user
 * 			selected.
 * Returns:
 *  	a Boolean indicating whether the user clicked the "Ok"
 *	button  (True) or the "Cancel" button (False).  If False is
 * 	returned, then the contents of buffer should be ignored.
 */

Boolean MuGetString(prompt, buffer, bufsize, helptext)
char *prompt;
char *buffer;
int bufsize;
char *helptext;
{
    Arg args[5];
    int n;
    jmp_buf mark;
    char *str;
    int status;
    
    static Widget StringDialog;
    static Widget HelpButton;
    static Widget TextBox;
   
    if (StringDialog == NULL) {   /* if the widget doesn't exist yet */
	n = 0;   
	XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON); n++;
	XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;
	/*    XtSetArg(args[n], XmNdialogType, XmDIALOG_PROMPT); n++; */
	XtSetArg(args[n], XmNborderWidth, 2); n++;
	XtSetArg(args[n], XmNautoUnmanage,False); n++;
	StringDialog=XmCreatePromptDialog(_MuToplevel,"_MuStringDialog",
					  args,n);
	HelpButton=XmSelectionBoxGetChild(StringDialog, XmDIALOG_HELP_BUTTON);
	TextBox = XmSelectionBoxGetChild(StringDialog, XmDIALOG_TEXT);
	XtRealizeWidget(StringDialog); 
	MuSetStandardCursor(StringDialog);
	MuSetSingleLineEmacsBindings(TextBox);
    }
   
    n = 0;
    XtSetArg(args[n], XmNtextColumns, bufsize-1); n++;
    XtSetArg(args[n], XmNselectionLabelString, 
	     XmStringLtoRCreate(prompt, XmSTRING_DEFAULT_CHARSET)); n++;
    XtSetValues(StringDialog, args, n);

    n = 0;
    XtSetArg(args[n],XmNvalue, buffer); n++;
    XtSetArg(args[n],XmNmaxLength,bufsize-1); n++;
    XtSetValues(TextBox,args, n);
   
    XtAddCallback(StringDialog, XmNokCallback, _MuOkCallback, mark);
    XtAddCallback(StringDialog, XmNcancelCallback,_MuCancelCallback,mark);

    if (helptext != NULL) {
	if (!XtIsManaged(HelpButton)) XtManageChild(HelpButton);
	XtAddCallback(StringDialog, XmNhelpCallback,_MuHelpCallback, helptext);
    }
    else { 
	if (XtIsManaged(HelpButton)) XtUnmanageChild(HelpButton);
    } 
    
    if (!XtIsManaged(StringDialog)) 
	XtManageChild(StringDialog);
   	  
    switch (status = setjmp(mark)) {
    case 0:
	XtMainLoop();
    case 1:
    case 2:
	if (status == 1) {
	    str = XmTextGetString(TextBox);
	    strncpy(buffer,str,bufsize);
	    XtFree(str);
	}
	XtUnmanageChild(StringDialog);
	XtRemoveCallback(StringDialog, XmNokCallback, _MuOkCallback, mark);
	XtRemoveCallback(StringDialog, XmNcancelCallback,
			 _MuCancelCallback,mark);
	if (helptext != NULL) XtRemoveCallback(StringDialog,XmNhelpCallback,
					       _MuHelpCallback,helptext);

    }
    return((status==1)?True:False);
}
