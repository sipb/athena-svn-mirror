/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 *
 * $Id: put_fallback_file.c,v 1.6 1991-06-19 15:40:49 probe Exp $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_put_fallback_file_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/put_fallback_file.c,v 1.6 1991-06-19 15:40:49 probe Exp $";
#endif lint

#include "globalmessage.h"
#include <sys/types.h>
#include <sys/file.h>
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
  int oumask;
  
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
  {
    /* We want to set the time back if the file does exist; if it
     * doesn't, we want to leave it missing.
     */
    errstat = open(message_filename, O_RDONLY, 0);
    if((errstat != -1)&&(ftime>0)) {
      char dummy[1];
      /* We read one byte so that the =access= time gets set; utimes
       * doesn't work if you don't own the file.
       * We can ignore any error return, since if the read fails the
       * file will later be cleared and it will work anyhow.
       */
      read(errstat, dummy, 1);
      /* we never care about the old contents if we have new contents to
       * install, so just close this and reopen it later for writing.
       */
      close(errstat);
    } else if((errstat == -1)&&(ftime == 0)) {
      if(errno == ENOENT) {
	/* it didn't exist, so we want to leave it that way. */
	return(0);
      } else {
	/* something went wrong, but we can't do much about it.
	 * We just have to hope the cron job clears it.
	 */
	return(errno);
      }
    }
    /*
     * Don't bother worrying about an open error here, the failure
     * modes are such that either it will work later (ie. it was just
     * a missing file, which we are about to create anyway) or the
     * cron job will clean it up. Neither should be reported, as they
     * do not affect the user.  If the file was indeed there, we want
     * to put the empty data into it, so that when someone requests
     * it, they don't get a stale message; however, we still want to
     * try to warp the clock.
     */
  }

  /*
   * We could use an open and then a chmod to set the permissions of
   * the file, rather than using umask, but if we do things that way,
   * there is a window of time during which the file has the wrong permissions.
   */
  oumask = umask(0);		/* we really want these open */
  /* write the file so that it is world writeable. Note that since
   * usr/tmp is sticky-bitted by default, we can't remove the file
   * directly, even with this openness; that is dealt with by warping
   * the clock backwards on the file and letting the daily find job do
   * the work.
   */
  message_filedesc = open(message_filename,
			  O_CREAT|O_WRONLY|O_TRUNC, 0666);
  (void) umask(oumask);
  
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
   * if we don't want to force the timestamps, we can exit now on an
   * error instead of waiting.
   */
  if((ftime>0) && errstat == -1) {
    return(errno);
  }

  /*
   * if the file should really go away, warp the clock back as well,
   * so the cron job can nuke it....
   */
  if(ftime>0) {
    /* the read we did earlier should have set the access time, so we
     * don't need to do anything else here.
     */
    return(0);
  }
  /*
   * We can reuse errstat (having not checked it), since if the write
   * fails, we still want to try the utimes in the hope that the
   * timestamp will get set and the cron job will really delete the
   * file. 
   */
  errstat = utimes(message_filename, tvp);
  if(errstat == -1) {
    return(errno);
  }
  /* everything worked. */
  return(0);
}
