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
 * $Id: MuP.h,v 1.2 1999-01-22 23:16:38 ghudson Exp $
 */

#ifndef MuP_h
#define MuP_H
    
#include "Mu.h"

extern void _MuOkCallback();
extern void _MuCancelCallback();
extern void _MuHelpCallback();

extern Widget _MuModalDialogWidget;
extern Widget _MuToplevel;

#endif
