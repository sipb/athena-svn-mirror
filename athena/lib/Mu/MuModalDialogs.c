
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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuModalDialogs.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 *
 */


#include "MuP.h"
#include <Xm/MessageB.h>

Widget _MuModalDialogWidget = (Widget)NULL;


static void CreateWidget()
{
    Arg args[5];
    int n;
    Widget dummy;
    
    n = 0;
    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON); n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;
    XtSetArg(args[n], XmNborderWidth, 2); n++;
    _MuModalDialogWidget = XmCreateMessageDialog(_MuToplevel,
						 "_MuModalDialogWidget",
						 args,n);
    dummy = XmMessageBoxGetChild(_MuModalDialogWidget,
				 XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(dummy);
    dummy = XmMessageBoxGetChild(_MuModalDialogWidget,
				 XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(dummy);
    XtRealizeWidget(_MuModalDialogWidget);
    MuSetStandardCursor(_MuModalDialogWidget);
}


void MuModalDialog(string,type)
char *string;
int type;
{
    Arg args[2];
    
    if (_MuModalDialogWidget == NULL)  CreateWidget();

    XtSetArg(args[0], XmNmessageString,
	     XmStringLtoRCreate(string,XmSTRING_DEFAULT_CHARSET));
    XtSetArg(args[1], XmNdialogType, type);
    XtSetValues(_MuModalDialogWidget, args, 2);

    if (!XtIsManaged(_MuModalDialogWidget)) 
	XtManageChild(_MuModalDialogWidget);
}


