/* $Id: converters.c,v 1.3 1999-03-06 16:47:44 ghudson Exp $ */
/*******************************************************************
  Copyright (C) 1990 by the Massachusetts Institute of Technology

   Export of this software from the United States of America is assumed
   to require a specific license from the United States Government.
   It is the responsibility of any person or organization contemplating
   export to obtain such a license before exporting.

WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
distribute this software and its documentation for any purpose and
without fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright notice and
this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission.  M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is" without express
or implied warranty.

***************************************************************** */

#include <mit-copyright.h>
#include "config.h"

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/Xmu/Converters.h>
#include <Xm/Xm.h>

static XtConvertArgRec parentCvtArgs[] = {
     {XtBaseOffset, (caddr_t)XtOffset(Widget, core.parent), sizeof(Widget)},
   };

extern void XmuCvtStringToWidget ();

void add_converter ()
{
    XtAddConverter( XmRString, XmRWindow, XmuCvtStringToWidget,
		   parentCvtArgs, XtNumber(parentCvtArgs) );
}