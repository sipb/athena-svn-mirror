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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/Mu.h,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 */

#ifndef Mu_h
#define Mu_h

#include <Xm/Xm.h>

#ifdef __STDC__

void MuInitialize(Widget);    
void MuSetEmacsBindings(Widget);
void MuSetStandardCursor(Widget);
void MuSetWaitCursor(Widget);
void MuHelp(char *);
Boolean MuGetBoolean(char *, char *, char *, char *, int);
Boolean MuGetString(char *, char *, int, char *);
Boolean MuGetFileName(char *, int, char *, int, char *);
void MuRegisterNames();
void MuRegisterWidget(Widget, char *);
Widget MuLookupWidget(char *);
void MuHelpFile(char *);
void MuModalDialog(char *, int);
void MuSyncDialog(char *, int);

#else

void MuInitialize();    
void MuSetEmacsBindings();
void MuSetStandardCursor();
void MuSetWaitCursor();
void MuHelp();
Boolean MuGetBoolean();
Boolean MuGetString();
Boolean MuGetFileName();
void MuRegisterNames();
void MuRegisterWidget();
Widget MuLookupWidget();
void MuHelpFile();
void MuModalDialog();
void MuSyncDialog();

#endif

#define MuError(msg)       MuModalDialog(msg,XmDIALOG_ERROR)
#define MuWarning(msg)     MuModalDialog(msg,XmDIALOG_WARNING)
#define MuErrorSync(msg)   MuSyncDialog(msg,XmDIALOG_ERROR)
#define MuWarningSync(msg) MuSyncDialog(msg,XmDIALOG_WARNING)

/* Don't use these macros.  For backwards compatibility only. */
#define MuSetSingleLineEmacsBindings(w) MuSetEmacsBindings(w)
#define MuGetWidget(w) MuLookupWidget(w)

#endif
