/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 *
 * $Id: get_fallback_file.c,v 1.2 1991-06-19 15:40:02 probe Exp $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_get_fallback_file_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_fallback_file.c,v 1.2 1991-06-19 15:40:02 probe Exp $";
#endif lint

#include "globalmessage.h"
#include <sys/types.h>
#include <sys/file.h>

Code_t get_fallback_file(ret_data, ret_size, message_filename)
     char **ret_data;
     int *ret_size;
     char *message_filename;
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
