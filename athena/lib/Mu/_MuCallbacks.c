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
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/_MuCallbacks.c,v $
 * $Author: djf $
 * $Log: not supported by cvs2svn $
 *
 */

#include "MuP.h"
#include <setjmp.h>

void _MuOkCallback(w, tag, callback_data)
Widget w;
caddr_t tag;
caddr_t callback_data;
/*ARGSUSED*/
{
    longjmp(*(jmp_buf *)tag, 1);
}

void _MuCancelCallback(w, tag, callback_data)
Widget w;
caddr_t tag;
caddr_t callback_data;
/*ARGSUSED*/
{   
    longjmp(*(jmp_buf *)tag, 2);
} 

void _MuHelpCallback(w, tag, callback_data)
Widget w;
caddr_t tag;
caddr_t callback_data;
/*ARGSUSED*/
{   
    MuHelp((char *)tag);
}
