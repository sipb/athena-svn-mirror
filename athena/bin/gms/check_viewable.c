/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/check_viewable.c,v $
 * $Author: eichin $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_check_viewable_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/check_viewable.c,v 1.2 1988-09-28 22:42:55 eichin Exp $";
#endif lint

#include "globalmessage.h"
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pwd.h>

Code_t check_viewable(message, checktime)
     char *message;
     int checktime;
{
  char *ptr, *usertfile;
  time_t ftime, utime;
  int status, usersize, ufd;
  char *usertfilename;
  
  if(strncmp(message, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN)) {
    return(GMS_SERVER_VERSION);
  }

  ptr = index(message, '\n');
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
    if(status) {
      if(!strncmp(usertfile, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN)) {
	/* now check for end of first line */
	ptr = index(usertfile, '\n');
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
  
  /* now, reopen/create the file to write new timestamp */
  ufd = open(usertfilename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
  free(usertfilename);
  if(ufd != -1) {
    char *msg = &message[GMS_VERSION_STRING_LEN];
    
    /* write out the version number */
    write(ufd, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN);
    
    /* write out the timestring from the message file */
    ptr = index(message, '\n')+1;
    write(ufd, msg, ptr - msg);
    close(ufd);
  }
  /* if we tried to write, we want to print... */
  return(0);
}
  


