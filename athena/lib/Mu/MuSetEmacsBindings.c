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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuSetEmacsBindings.c,v $
 * $Author: cfields $
 * $Log: not supported by cvs2svn $
 * Revision 1.5  1990/08/30  12:40:31  vanharen
 * Two strings were not terminated correctly -- the end-quotes were on the
 * next line down.  GCC didn't complain, but other compilers did...
 *
 * Revision 1.4  90/08/28  14:33:58  vanharen
 * added return binding for single-line bindings.
 * 
 * Revision 1.3  90/08/23  13:27:24  vanharen
 * fixed translation table.
 * 
 * Revision 1.2  89/12/09  16:36:03  djf
 * 
 * changed arg in call to XtGetValues to &arg
 * 
 * Revision 1.1  89/12/09  15:14:49  djf
 * Initial revision
 * 
 *
 * These functions add emacs key bindings to multiple line or single
 * line text widgets when they are passed as arguments.
 * 
 */

#include "Mu.h"
#include <Xm/Text.h>


static XtTranslations EmacsBindings = NULL;
static char EmacsBindingsText[] =
	"~Shift ~Ctrl ~Meta ~Alt <Key>osfDelete:       delete-previous-character() \n\
	~Shift ~Ctrl  Meta ~Alt <Key>osfDelete: kill-previous-word() \n\
	~Meta ~Alt <Key>osfBackSpace:   delete-previous-character() \n\
	Meta ~Alt <Key>osfBackSpace:   kill-previous-word() \n\
	Ctrl<Key>D:		delete-next-character() \n\
	Meta<Key>D:		kill-next-word() \n\
	Ctrl<Key>K:		kill-to-end-of-line() \n\
	Ctrl<Key>W:		kill-selection() \n\
	Ctrl<Key>Y:		unkill() \n\
	Ctrl<Key>F:		forward-character() \n\
	Ctrl<Key>B:		backward-character() \n\
	Meta<Key>F:		forward-word() \n\
	Meta<Key>B:		backward-word() \n\
	Meta<Key>]:		forward-paragraph() \n\
	Meta<Key>[:		backward-paragraph() \n\
	Ctrl<Key>A:		beginning-of-line() \n\
	Ctrl<Key>E:		end-of-line() \n\
	Ctrl<Key>N:		next-line() \n\
	Ctrl<Key>P:		previous-line() \n\
	Ctrl<Key>V:		next-page() \n\
	Meta<Key>V:		previous-page() \n\
	~Shift Meta<Key><:	beginning-of-file() \n\
	Shift Meta<Key><:	end-of-file() \n\
	Meta<Key>>:		end-of-file() \n\
	Ctrl<Key>L:		redraw-display() \n\
	Ctrl<Key>M:		newline() \n\
	Ctrl<Key>J:		newline() \n\
	<Key>Return:		newline() ";

static XtTranslations SingleLineEmacsBindings = NULL;
static char SingleLineEmacsBindingsText[] = 
	"~Shift ~Ctrl ~Meta ~Alt <Key>osfDelete:       delete-previous-character() \n\
	~Shift ~Ctrl  Meta ~Alt <Key>osfDelete: kill-previous-word() \n\
	~Meta ~Alt <Key>osfBackSpace:   delete-previous-character() \n\
	Meta ~Alt <Key>osfBackSpace:   kill-previous-word() \n\
	Ctrl<Key>D:		delete-next-character() \n\
	Meta<Key>D:		kill-next-word() \n\
	Ctrl<Key>K:		kill-to-end-of-line() \n\
	Ctrl<Key>W:		kill-selection() \n\
	Ctrl<Key>Y:		unkill() \n\
	Ctrl<Key>F:		forward-character() \n\
	Ctrl<Key>B:		backward-character() \n\
	Ctrl<Key>A:		beginning-of-line() \n\
	Ctrl<Key>E:		end-of-line() \n\
	Meta<Key>F:		forward-word() \n\
	Meta<Key>B:		backward-word() \n\
	~Shift Meta<Key><:	beginning-of-line() \n\
	Shift Meta<Key><:	end-of-line() \n\
	Meta<Key>>:		end-of-line() \n\
	<Key>Return:		activate() ";

void MuSetEmacsBindings(w)
Widget w;
{
    Arg arg;
    int editmode;
    
    if (!XtIsSubclass(w,xmTextWidgetClass)) {
	XtWarning("MuSetEmacsBindings() called on non-text widget");
    }
    
    XtSetArg(arg, XmNeditMode, &editmode);
    XtGetValues(w,&arg,1);

    if (editmode == XmMULTI_LINE_EDIT) {
	if (EmacsBindings == NULL) 
	    EmacsBindings = XtParseTranslationTable(EmacsBindingsText);
	XtOverrideTranslations(w,EmacsBindings);
    }
    else {
	if (SingleLineEmacsBindings == NULL)
	    SingleLineEmacsBindings = 
		XtParseTranslationTable(SingleLineEmacsBindingsText);
	XtOverrideTranslations(w,SingleLineEmacsBindings);
    }
}

