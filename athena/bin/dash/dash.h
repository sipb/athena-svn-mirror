/*
 * $Id: dash.h,v 1.2 1999-01-22 23:08:48 ghudson Exp $
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef _dash_h
#define _dash_h

typedef struct _MyResources
{
  Boolean verifyOn, notifyOn, run, send, kill, restart, debug, nofork;
#ifdef HAVE_KRB4
  Boolean checkTickets;
#endif /* HAVE_KRB4 */
  char *startString;
} MyResources;

typedef struct _MyResources *MyResourcesPtr;

#endif /* _dash_h */
