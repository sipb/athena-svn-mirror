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
 *      $Id: Cursor.c,v 1.3 1996-08-10 21:28:10 cfields Exp $
 *      $Author: cfields $
 */

#include <Mu.h>

extern Widget xolc, w_send_form;

void
SetCursor(wait)
     int wait;
{
  if (wait) {
    MuSetWaitCursor(xolc);
    MuSetWaitCursor(w_send_form);
  }
  else {
    MuSetStandardCursor(xolc);
    MuSetStandardCursor(w_send_form);
  }
}

