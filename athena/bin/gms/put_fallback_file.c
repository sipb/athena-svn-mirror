/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/put_fallback_file.c,v $
 * $Author: eichin $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_put_fallback_file_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/put_fallback_file.c,v 1.1 1988-09-26 15:29:16 eichin Exp $";
#endif lint

#include "globalmessage.h"
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>

Code_t put_fallback_file(message_data, message_size, message_filename)
     char *message_data;
     int message_size;
     char *message_filename;
{
  int errstat;
  int message_filedesc;
  time_t ftime;
  struct timeval tvp[2];

  /* in case we try to NULL out the file... */
  tvp[0].tv_sec = tvp[1].tv_sec = 0;
  tvp[0].tv_usec = tvp[1].tv_usec = 0;
  
  /* guard against bogus arguments */
  if((!message_data)||(!message_size)||(!message_filename)) {
    return(GMS_NULL_ARG_ERR);
  }
  if((message_size<0)) {
    return(GMS_BAD_ARG_ERR);
  }
  
  /*
   * First, check the time stamp in the file, to see if we should mark
   * it for deletion (if it exists) and warp the timestamp back or
   * just avoid creating it if it doesn't.
   */
  ftime = atol(&message_data[GMS_VERSION_STRING_LEN+1]);
  if(ftime == 0) {
    /* We want to set the time back if the file does exist; if it
     * doesn't, we want to leave it missing.
     */
    errstat = open(message_filename, O_RDONLY, 0);
    if(errstat) {
      if(errno == ENOENT) {
	/* it didn't exist, so we want to leave it that way. */
	return(0);
      } else {
	/* something went wrong, but we can't do much about it. */
	return(errno);
      }
    }
    /*
     * we never care about the old contents if we have new contents to
     * install, so just close this and reopen it later for writing.
     */
    close(errstat);
    /*
     * If the file was indeed there, we want to put the empty data
     * into it, so that when someone requests it, they don't get a
     * stale message; however, we still want to warp the clock.
     */
  }
    
  (void) umask(0);		/* we really want these open */
  /* write the file so that it is world writeable. Note that since
   * usr/tmp is sticky-bitted by default, we can't remove the file
   * directly, even with this openness; that is dealt with by warping
   * the clock backwards on the file and letting the daily find job do
   * the work.
   */
  message_filedesc = open(message_filename, O_CREAT|O_WRONLY, 0666);

  /* Just return the error if something fails. This will be later
   * ignored, since this is a non critical stage...
   */
  if(message_filedesc == -1) {
    return(errno);
  }

  /*
   * The timestamps are already in the message image, so just save
   * them.
   */
  errstat = write(message_filedesc, message_data, message_size);
  close(message_filedesc);

  /*
   * if the file should really go away, warp the clock back as well,
   * so the cron job can nuke it....
   */
  if(ftime == 0) {
    /*
     * We can reuse errstat, since if the write fails, we still want
     * to try the utimes in the hope that the file will really go
     * away.
     */
    errstat = utimes(message_filename, tvp);
  }
  if(errstat == -1) {
    return(errno);
  }
  /* everything worked. */
  return(0);
}
