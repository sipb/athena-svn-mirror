/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/warn.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/warn.c,v 1.2 1991-12-17 11:22:45 vanharen Exp $";
#endif

#include "mit-copyright.h"
#include "Jets.h"
#include "Window.h"
#include "Button.h"
#include "Label.h"
#include "Form.h"
#include "warn.h"

extern char line1[], line2[];
extern Jet root;

static int ok(who, w, data)
     Jet who;
     Warning *w;
     caddr_t data;
{
  XjDestroyJet(w->top);
  XFlush(who->core.display);

  XjFree(w->l1);
  XjFree(w->l2);
  XjFree((char *) w);
  return 0;
}

Warning *UserWarning(okProc, realize)
     Warning *okProc;
     Boolean realize;
{
  Jet warnTop, warnForm, warn1label, warn2label, warn1icon,
  warnokwindow, warnoklabel;
  ButtonJet warnokbutton;
  Warning *w;

  if (okProc)
    w = okProc;
  else
    {
      w = (Warning *)XjMalloc((unsigned) sizeof(Warning));

      w->me.next = NULL;
      w->me.argType = argInt;
      w->me.passInt = (int)w;
      w->me.proc = ok;

      w->l1 = XjNewString(line1);
      w->l2 = XjNewString(line2);
    }

  warnTop =  XjVaCreateJet("warnWindow", windowJetClass, root, NULL, NULL);
  warnForm = XjVaCreateJet("warnForm", formJetClass, warnTop, NULL, NULL);

  warn1label = XjVaCreateJet("warn1Label", labelJetClass, warnForm,
			     XjNlabel, w->l1, NULL, NULL);
  warn2label = XjVaCreateJet("warn2Label", labelJetClass, warnForm,
			     XjNlabel, w->l2, NULL, NULL);
  warn1icon = XjVaCreateJet("dashLogo", labelJetClass, warnForm,
			    NULL, NULL);
  warnokwindow = XjVaCreateJet("warnOKWindow", windowJetClass, warnForm,
			       NULL, NULL);
  warnokbutton = (ButtonJet) XjVaCreateJet("warnOKButton", buttonJetClass,
					   warnokwindow,
			       XjNactivateProc, &(w->me), NULL, NULL);
  warnoklabel = XjVaCreateJet("warnOKLabel", labelJetClass, warnokbutton,
			      NULL, NULL);

  w->top = warnTop;
  w->button = warnokbutton;

  if (realize)
    XjRealizeJet(warnTop);

  return w;
}
