/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/check_viewable.c,v $
 * $Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_check_viewable_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/check_viewable.c,v 1.5 1996-09-19 22:39:16 ghudson Exp $";
#endif lint

#include "globalmessage.h"
#include <sys/types.h>
#include <sys/file.h>
#include <pwd.h>

Code_t check_viewable(message, checktime, updateuser)
     char *message;
     int checktime;		/* should we check the timestamp at all? */
     int updateuser;		/* should we update the user timestamp file? */
{
  char *ptr, *usertfile;
  time_t ftime, utime;
  int status, usersize, ufd;
  char *usertfilename;
  
  if(strncmp(message, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN)) {
    return(GMS_SERVER_VERSION);
  }

  ptr = strchr(message, '\n');
  if(!ptr) {
    return(GMS_SERVER_GARBLED);
  }
  
  ftime = atol(&message[GMS_VERSION_STRING_LEN+1]);

  {
    struct passwd *pw;
    char *userdir;

    pw = getpwuid(getuid());
      
    if(pw) {
      userdir = pw->pw_dir;
    } else {
      /* couldn't check user's file, probably better send it. */
      free(usertfilename);
      return(0);
    }

    usertfilename = malloc(GMS_USERFILE_NAME_LEN+strlen(userdir)+1);
    strcpy(usertfilename, userdir);
    strcat(usertfilename, GMS_USERFILE_NAME);

    ufd = open(usertfilename, O_RDONLY, 0666);
  }
  
  if(ufd != -1) {
    /* read the file and close it */
    status = read_to_memory(&usertfile, &usersize, ufd);
    /* check the version string */
    if(!status) {
      if(!strncmp(usertfile, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN)) {
	/* now check for end of first line */
	ptr = strchr(usertfile, '\n');
	if(ptr) {
	  /* now we check the time stamp */
	  utime = atol(&usertfile[GMS_VERSION_STRING_LEN+1]);
	  if(ftime <= utime) {
	    /* user has already seen, we punt. */
	    free(usertfile);
	    free(usertfilename);
	    /* but we only punt if they have asked for it... */
	    if(checktime) {
	      return(GMS_OLD_MESSAGE);
	    } else {
	      return(0);
	    }
	  }
	}
      }
      /* only valid if read_to_memory worked... */
      free(usertfile);
    }
  }

  /* only write out the new time stamp if they want it. */
  if(updateuser) {
    /* now, reopen/create the file to write new timestamp */
    ufd = open(usertfilename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(ufd != -1) {
      char *msg = &message[GMS_VERSION_STRING_LEN];
    
      /* write out the version number */
      write(ufd, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN);
    
      /* write out the timestring from the message file */
      ptr = strchr(message, '\n')+1;
      write(ufd, msg, ptr - msg);
      close(ufd);
    }
  }
  /* filename no longer needed... */
  free(usertfilename);
  /* if we tried to write, we want to print... */
  return(0);
}
  


