/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/view_message_by_tty.c,v $
 * $Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_view_message_by_tty_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/view_message_by_tty.c,v 1.2 1996-09-19 22:39:20 ghudson Exp $";
#endif lint

#include "globalmessage.h"

void view_message_by_tty(message)
     char *message;
{
  char *ptr;

  /* skip magic headers */
  ptr = strchr(message, '\n')+1;
  /* note that if there is nothing, strlen(ptr) == 0 */
  write(1,ptr,strlen(ptr));
}
