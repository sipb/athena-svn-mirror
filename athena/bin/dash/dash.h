/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/dash.h,v $
 * $Author: ghudson $ 
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
