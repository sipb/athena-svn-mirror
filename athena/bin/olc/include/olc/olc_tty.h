/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions common to all parts of OLC.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_tty.h,v $
 *      $Author: tjcoppet $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc_tty.h,v 1.3 1989-08-22 13:59:01 tjcoppet Exp $
 */

extern int OLC, OLCR, OLCA;

#define DEFAULT_EDITOR "/usr/athena/emacs"
#define NO_EDITOR      "madman_across_the_water"

#define newline() write(1,"\n",1);
