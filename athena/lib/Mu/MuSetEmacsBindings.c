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
 * $Id: MuSetEmacsBindings.c,v 1.7 1999-01-22 23:16:41 ghudson Exp $
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

