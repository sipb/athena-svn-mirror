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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuHelp.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 *
 * help.c
 * This file contains the function MuHelp
 *
 * These two functions share an array of InformationDialog widgets.
 * As many as 25 dialog widgets may be managed at once, but only 5
 * are not destroyed after it's unmanaged.  When called, the function 
 * checks to see if helpbox[0] has been created. If yes, it checks to 
 * see if it's managed.  If not, it will simply set the resource to display  
 * the proper message string and manage it.  If it's managed, then it will
 * check to see if helpbox[1] has been created.  If not, it will create and
 * manage it.  If yes, it will continue to check if it's managed and so on.
 *
 */

#include "MuP.h"
#include <Xm/MessageB.h>
#include <setjmp.h>

static Widget helpbox[25];

static void Destroy(w, tag, callback_data)
Widget w;
caddr_t tag;
caddr_t callback_data;
/*ARGSUSED*/
{
      XtDestroyWidget(w);
      helpbox[(int)tag] = (Widget)NULL;
}

void MuHelp(string)
    char *string;
 {
    Arg args[10];
    int n, i;
    Widget dummy;

    /* look through the array for the first un-created or un-managed widget */
    /* on exit from this loop, helpbox[i] is an unmanaged dialog widget */
    for (i=0; i < 25 ; i++)   {
          if (helpbox[i] == NULL) { /* if not created, create it */
	    n = 0;
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON); n++;
	    /* XtSetArg(args[n], XmNdialogType, XmDIALOG_INFORMATION); n++;  */
	    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
	    XtSetArg(args[n], XmNborderWidth, 2); n++;
	    helpbox[i]=XmCreateInformationDialog(_MuToplevel,"helpbox",args,n);
	    XtRealizeWidget(helpbox[i]);
	    MuSetStandardCursor(helpbox[i]);

	    dummy = XmMessageBoxGetChild(helpbox[i], XmDIALOG_CANCEL_BUTTON);
	    XtDestroyWidget(dummy);
	    dummy = XmMessageBoxGetChild(helpbox[i], XmDIALOG_HELP_BUTTON);
	    XtDestroyWidget(dummy);

	    /* Once created, keep the first three dialog boxes */
	    /* for all others, destroy them when done. */
            if (i > 2)
              XtAddCallback(helpbox[i], XmNokCallback, Destroy, i);
            break;
	  }

         else if (!XtIsManaged(helpbox[i]))
                  break;

         if (i == 24) {
               MuError(" You can not have more than 25 help boxes at once. \n Please click on 'OK' in one of the help boxes."); 
               return;
	     }
	}
    XtSetArg(args[0], XmNmessageString,
	     XmStringLtoRCreate(string,XmSTRING_DEFAULT_CHARSET)); 
    XtSetValues(helpbox[i], args, 1);
    XtManageChild(helpbox[i]);
}

