/* Copyright 1988, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: read_to_memory.c,v 1.1 1999-12-08 22:06:46 danw Exp $";

#include "globalmessage.h"

Code_t read_to_memory(char **ret_block, int *ret_size, int filedesc)
{
  char buf[BFSZ], *message_data = NULL;
  int message_size = 0;
  int stat;
  
  do {
    /* read the block */
    stat = read(filedesc, buf, BFSZ);
    if(stat == -1) {
      /* handle read failed error */
      free(message_data);
      return(errno);
    }

    /* allocate a memory area for copying */
    /* the +1 are for trailing NULs */
    message_data = realloc(message_data, message_size + stat + 1);
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
  
