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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuSyncDialogs.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 *
 */

#include "MuP.h"
#include <setjmp.h>

void MuSyncDialog(string,type)
char *string;
int type;
{
    jmp_buf mark;
    
    MuModalDialog(string,type);
    XtAddCallback(_MuModalDialogWidget, XmNokCallback, _MuOkCallback, mark);
    
    switch (setjmp(mark)) {
    case 0:
	XtMainLoop();
    case 1:
        XtUnmanageChild(_MuModalDialogWidget);
        XtRemoveCallback(_MuModalDialogWidget,XmNokCallback,
			 _MuOkCallback, mark);
        return;
    }
}
