/*
 * MotifUtils:   Utilities for use with Motif and UIL
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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuRegisterNames.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 *
 * This file contains the function MuRegisterNames, which calls 
 * MrmRegisterNames to register all the "Callback" MotifUtilities 
 * functions. Listed below are all the MotifUtilities functions which
 * can be called from uil.  They are called without the "Callback" tag
 * in their names.
 */

#include "Mu.h"
#include <Mrm/MrmPublic.h>

static void MuHelpCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuHelp((char *)tag);
}

static void MuErrorCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuError((char *)tag);
}

static void MuWarningCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuWarning((char *)tag);
}

static void MuHelpFileCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuHelpFile((char *)tag);
}

static void MuSetEmacsBindingsCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuSetEmacsBindings(w);
}

static void MuSetWaitCursorCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuSetWaitCursor(w);
}

static void MuSetStandardCursorCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuSetStandardCursor(w);
}

static void MuRegisterWidgetCallback(w,tag,call_data)
Widget w;
caddr_t tag;
caddr_t call_data;
/* ARGSUSED */
{
    MuRegisterWidget(w,(char *)tag);
}

static MRMRegisterArg regvec[] = {
    {"MuHelp", (caddr_t)MuHelpCallback},
    {"MuError", (caddr_t)MuErrorCallback},
    {"MuWarning", (caddr_t)MuWarningCallback},
    {"MuHelpFile", (caddr_t)MuHelpFileCallback},
    {"MuSetEmacsBindings", (caddr_t)MuSetEmacsBindingsCallback},
    {"MuSetSingleLineEmacsBindings", (caddr_t)MuSetEmacsBindingsCallback},
    {"MuSetStandardCursor", (caddr_t)MuSetStandardCursorCallback},
    {"MuSetWaitCursor", (caddr_t)MuSetWaitCursorCallback},
    {"MuRegisterWidget", (caddr_t)MuRegisterWidgetCallback},

    {"MUHELP", (caddr_t)MuHelpCallback},
    {"MUERROR", (caddr_t)MuErrorCallback},
    {"MUWARNING", (caddr_t)MuWarningCallback},
    {"MUHELPFILE", (caddr_t)MuHelpFileCallback},
    {"MUSETEMACSBINDINGS", (caddr_t)MuSetEmacsBindingsCallback},
    {"MUSETSINGLELINEEMACSBINDINGS", (caddr_t)MuSetEmacsBindingsCallback},
    {"MUSETSTANDARDCURSOR", (caddr_t)MuSetStandardCursorCallback},
    {"MUSETWAITCURSOR", (caddr_t)MuSetWaitCursorCallback},
    {"MUREGISTERWIDGET", (caddr_t)MuRegisterWidgetCallback},
};

void MuRegisterNames()
{
    MrmRegisterNames (regvec, XtNumber(regvec));
}

