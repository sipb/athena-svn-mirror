/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/read_to_memory.c,v $
 * $Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_read_to_memory_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/read_to_memory.c,v 1.2 1996-09-19 22:39:20 ghudson Exp $";
#endif lint

#include "globalmessage.h"

Code_t read_to_memory(ret_block, ret_size, filedesc)
     char **ret_block;
     int *ret_size, filedesc;
{
  char buf[BFSZ], *message_data;
  int message_size = 0;
  int stat;
  
  do {
    /* read the block */
    stat = read(filedesc, buf, BFSZ);
    if(stat == -1) {
      /* handle read failed error */
      if(message_size) {
	free(message_data);
      }
      return(errno);
    }

    /* allocate a memory area for copying */
    /* the +1 are for trailing NUL's */
    if(message_size) {
      message_data=realloc(message_data, message_size + stat +1);
    } else {
      message_data = malloc(message_size + stat +1);
    }
    if(!message_data) {
      return(GMS_MALLOC_ERR);
    }
      
    /* copy it into the right place */
    memcpy(&message_data[message_size], buf, stat);

    message_size += stat;
  } while(stat);
    /* but only until we stop getting blocks. */

  /* Just to make it consistent, for lazy calling routines... */
  message_data[message_size] = '\0';
  
  *ret_block = message_data;
  *ret_size = message_size;
  return(0);
}
  
