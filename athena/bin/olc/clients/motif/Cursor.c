/*
 * This file is part of the On-Line Consulting system
 *
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/motif/Cursor.c,v $
 *      $Id: Cursor.c,v 1.1 1991-03-06 15:46:51 lwvanels Exp $
 *      $Author: lwvanels $
 */

#include <Xm/Mu.h>

extern Widget toplevel, w_send_form;

void
SetCursor(wait)
     int wait;
{
  if (wait) {
    MuSetWaitCursor(toplevel);
    MuSetWaitCursor(w_send_form);
  }
  else {
    MuSetStandardCursor(toplevel);
    MuSetStandardCursor(w_send_form);
  }
}

