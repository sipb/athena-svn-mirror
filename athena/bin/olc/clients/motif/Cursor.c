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
 *      $Id: Cursor.c,v 1.5 1999-03-06 16:47:43 ghudson Exp $
 */

#include <mit-copyright.h>
#include "config.h"

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

