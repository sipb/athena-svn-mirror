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

static const char rcsid[] = "$Id: get_fallback_file.c,v 1.1 1999-12-08 22:06:44 danw Exp $";

#include "globalmessage.h"
#include <sys/types.h>

Code_t get_fallback_file(char **ret_data, int *ret_size,
			 char *message_filename)
{
  char *message_data;
  int message_filedesc;
  int message_size;
  int readstat;

  /* guard against NULL arguments */
  if((!ret_data)||(!ret_size)||(!message_filename)) {
    return(GMS_NULL_ARG_ERR);
  }

  /* the return time stamp and version stuff is saved with the local
   * anyway...
   */
  message_filedesc = open(message_filename, O_RDONLY, 0);
  
  if(message_filedesc == -1) {
    /* handle open failure, use unix errors */
    return(errno);
  }

  readstat = read_to_memory(&message_data, &message_size, message_filedesc);
  close(message_filedesc);	/* regardless of errors */

  if(readstat) {
    /* handle read_to_memory errors, clean up timestamp */
    return(readstat);
  }
  
  *ret_data = message_data;
  *ret_size = message_size;
  return(0);
}
