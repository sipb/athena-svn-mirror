/*
 * $Id: warn.h,v 1.2 1999-01-22 23:17:10 ghudson Exp $
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
