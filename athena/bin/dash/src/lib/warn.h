/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/warn.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _warn_h
#define _warn_h

typedef struct _wstruct {
  Jet top;
  ButtonJet button;
  XjCallback me;
  char *l1, *l2;
} Warning;

extern Warning *XjUserWarning();

#endif /* _warn_h */
