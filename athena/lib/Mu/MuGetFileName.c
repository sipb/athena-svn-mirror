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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuGetFileName.c,v $
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
#include <Xm/FileSB.h>
#include <setjmp.h>

/*
 * MuGetFileName
 * Uses a FileSelectionDialog widget to prompt the user to select a file.
 * The string is returned in a passed buffer, and the
 * function returns a Boolean indicating whether the "Ok" or the
 * "Cancel"  button was pressed.
 * Inputs:
 * 	buffer		A pointer to a buffer to return the file name in.
 * 			The contents of the buffer will be displayed as
 * 			the default ?.
 * 	bufsize		The size in bytes of the buffer.
 *      dirmask         A pointer to a buffer to return the value of the
 *                      directory mask.
 *      dirmasksize     The size in bytes of the dirmask.
 * 	helptext	The text to display if the help button is
 *			clicked.  If NULL, no Help button will be
 * 			displayed.
 * Outputs:
 * 	buffer		Contains the NULL-terminated file name the user
 * 			selected.
 *      dirmask         Contains the NULL-terminated directory mask used.
 * Returns:
 *  	a Boolean indicating whether the user clicked the "Ok"
 *	button  (True) or the "Cancel" button (False).  If False is
 * 	returned, then the contents of buffer should be ignored.
 */

Boolean MuGetFileName(buffer, bufsize, dirmask, dirmasksize,helptext)
char *buffer;
int bufsize, dirmasksize;
char *dirmask;
char *helptext;
{
    Arg args[10];
    int n;
    jmp_buf mark;
    XmString file_string, dirmask_string;
    char *str;
    int status;
   
    static Widget FileDialog = NULL;
    static Widget HelpButton = NULL;
    static Widget TextBox = NULL;
    static Widget FilterText = NULL;
     
    if (FileDialog == NULL) {        /* if widget doesn't exist yet */
	n = 0;
	/* XtSetArg(args[n], XmNdialogType, XmDIALOG_FILE_SELECTION); n++; */ 
	XtSetArg(args[n], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL); n++;
	XtSetArg(args[n],XmNdefaultButtonType, XmDIALOG_OK_BUTTON); n++;
        XtSetArg(args[n], XmNborderWidth, 2); n++;
	XtSetArg(args[n], XmNautoUnmanage,False); n++;
	FileDialog = XmCreateFileSelectionDialog(_MuToplevel,
						 "_MuFileDialog",args,n);
	HelpButton = XmFileSelectionBoxGetChild(FileDialog, 
						XmDIALOG_HELP_BUTTON);
	TextBox = XmFileSelectionBoxGetChild(FileDialog,XmDIALOG_TEXT);
        FilterText = XmFileSelectionBoxGetChild(FileDialog, 
						XmDIALOG_FILTER_TEXT); 
    
	XtRealizeWidget(FileDialog);  
	MuSetStandardCursor(FileDialog);
        MuSetSingleLineEmacsBindings(FilterText);
        MuSetSingleLineEmacsBindings(TextBox); 
    }
     
    n=0;
    XtSetArg(args[n], XmNdirMask,
	     XmStringLtoRCreate(dirmask,XmSTRING_DEFAULT_CHARSET)); n++;
    XtSetArg(args[n], XmNtextColumns, bufsize-1); n++;
    XtSetValues(FileDialog, args, n);

    n = 0;
    XtSetArg(args[n],XmNmaxLength,bufsize-1); n++;
    XtSetValues(TextBox,args, n);
     
    n = 0;
    XtSetArg(args[n],XmNmaxLength,dirmasksize-1); n++;
    XtSetValues(FilterText,args, n);
    
    XtAddCallback(FileDialog, XmNokCallback, _MuOkCallback, mark);
    XtAddCallback(FileDialog, XmNcancelCallback, _MuCancelCallback, mark);

    if (helptext != NULL) {
	if (!XtIsManaged(HelpButton)) XtManageChild(HelpButton);
	XtAddCallback(FileDialog, XmNhelpCallback, _MuHelpCallback, helptext);
    }
    else { 
	if (XtIsManaged(HelpButton)) XtUnmanageChild(HelpButton);
    } 
 
    if (!XtIsManaged(FileDialog)) 
          XtManageChild(FileDialog);
     
    switch (status = setjmp(mark)) {
    case 0:
	XtMainLoop();
    case 1:
    case 2:
	if (status == 1) {
	    n = 0;
	    XtSetArg(args[n], XmNdirSpec, &file_string); n++;
	    XtSetArg(args[n], XmNdirMask, &dirmask_string); n++;
	    XtGetValues(FileDialog, args, n);
	    XmStringGetLtoR(file_string, XmSTRING_DEFAULT_CHARSET, &str);
	    strncpy(buffer,str,bufsize);
	    XmStringGetLtoR(dirmask_string, XmSTRING_DEFAULT_CHARSET, &str);
	    strncpy(dirmask,str,dirmasksize);
	}
	XtUnmanageChild(FileDialog);
	XtRemoveCallback(FileDialog, XmNokCallback, _MuOkCallback, mark);
	XtRemoveCallback(FileDialog,XmNcancelCallback,_MuCancelCallback, mark);
	if (helptext != NULL) XtRemoveCallback(FileDialog,XmNhelpCallback,
					      _MuHelpCallback,helptext);

      }
    return((status==1)?True:False);
}
