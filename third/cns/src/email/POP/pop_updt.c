/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * code for mh compatibility (tom) 
 */

#ifndef lint
static char copyright[] = "Copyright (c) 1990 Regents of the University of California.\nAll rights reserved.\n";
static char SccsId[] = "@(#)pop_updt.c  1.9 8/16/90";
#endif /* not lint */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "popper.h"	/* Include this (almost) first for NEED_*, etc */

#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif

#ifdef POSIX
/* POSIX means we don't have flock... */
#define flock(f,c)	emul_flock(f,c)
#ifndef LOCK_SH
#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */
#endif
#endif

extern int      errno;

static char standard_error[] =
    "Error updating primary drop. Mailbox unchanged.";

/* 
 *  updt:   Apply changes to a user's POP maildrop
 */

int pop_updt (p)
POP     *   p;
{
    FILE                *   md;                     /*  Stream pointer for 
                                                        the user's maildrop */
    int                     mfd;                    /*  File descriptor for
                                                        above */
    char                    buffer[BUFSIZ];         /*  Read buffer */

    MsgInfoList         *   mp;                     /*  Pointer to message 
                                                        info list */
    register int            msg_num;                /*  Current message 
                                                        counter */
    register int            status_written;         /*  Status header field 
                                                        written */
    int                     nchar;                  /*  Bytes read/written */

    long                    offset;                 /*  New mail offset */
    
    int                     begun;                  /*  Sanity check */

#ifdef DEBUG
    if (p->debug) {
        pop_log(p,POP_DEBUG,"Performing maildrop update...");
        pop_log(p,POP_DEBUG,"Checking to see if all messages were deleted");
    }
#endif /* DEBUG */

    if (p->msgs_deleted == p->msg_count) {
        /* Truncate before close, to avoid race condition,  DO NOT UNLINK!
           Another process may have opened,  and not yet tried to lock */
        (void)ftruncate ((int)fileno(p->drop),0);
        (void)fclose(p->drop) ;
        return (POP_SUCCESS);
    }

#ifdef DEBUG
    if (p->debug) 
        pop_log(p,POP_DEBUG,"Opening mail drop \"%s\"",p->drop_name);
#endif /* DEBUG */

    /*  Open the user's real maildrop */
    if (((mfd = open(p->drop_name,O_RDWR|O_CREAT,0666)) == -1 ) ||
        ((md = fdopen(mfd,"r+")) == NULL)) {
        return pop_msg(p,POP_FAILURE,"%s %s", 
		       p->drop_name, sys_errlist[errno]);
    }

    /*  Lock the user's real mail drop */
    if ( flock(mfd,LOCK_EX) == -1 ) {
        (void)fclose(md) ;
        return pop_msg(p,POP_FAILURE, "flock: '%s': %s", p->temp_drop,
            (errno < sys_nerr) ? sys_errlist[errno] : "");
    }

    /* Go to the right places */
    offset = lseek((int)fileno(p->drop),0,SEEK_END) ; 

    /*  Append any messages that may have arrived during the session 
        to the temporary maildrop */
    while ((nchar=read(mfd,buffer,BUFSIZ)) > 0)
        if ( nchar != write((int)fileno(p->drop),buffer,nchar) ) {
            nchar = -1;
            break ;
        }
    if ( nchar != 0 ) {
        (void)fclose(md) ;
        (void)ftruncate((int)fileno(p->drop),(int)offset) ;
        (void)fclose(p->drop) ;
        return pop_msg(p,POP_FAILURE,standard_error);
    }

    rewind(md);
    (void)ftruncate(mfd,0) ;

    /* Synch stdio and the kernel for the POP drop */
    rewind(p->drop);
    (void)lseek((int)fileno(p->drop),0,SEEK_SET);

    /*  Transfer messages not flagged for deletion from the temporary 
        maildrop to the new maildrop */
#ifdef DEBUG
    if (p->debug) 
        pop_log(p,POP_DEBUG,"Creating new maildrop \"%s\" from \"%s\"",
                p->drop_name,p->temp_drop);
#endif /* DEBUG */
    
    for (msg_num = 0; msg_num < p->msg_count; ++msg_num) {

        int doing_body;
      
        /*  Get a pointer to the message information list */
        mp = &p->mlp[msg_num];

        if (mp->del_flag) {
#ifdef DEBUG
            if(p->debug)
                pop_log(p,POP_DEBUG,
                    "Message %d flagged for deletion.",mp->number);
#endif /* DEBUG */
            continue;
        }
	
        (void)fseek(p->drop,mp->offset,0);

#ifdef DEBUG
        if(p->debug)
            pop_log(p,POP_DEBUG,"Copying message %d.",mp->number);
#endif /* DEBUG */

	begun = 0;

        for(status_written = doing_body = 0 ;
            fgets(buffer,MAXMSGLINELEN,p->drop);) {

            if (doing_body == 0) { /* Header */

	        /* panic, I'm tired and can't think contorted. */
	        if(is_msg_boundary(buffer) && begun) {  
		  pop_log(p, POP_ERROR,
			  "%s: mailbox detonation has begun!",  p->user);
		  (void)ftruncate(mfd,0);
		  (void)fclose(md);
		  (void)fclose(p->drop);
		  return(pop_msg(p, POP_FAILURE, "Unable to close mailbox door. Contact the postmaster to repair it."));
		}

		begun = 1;

                /*  Update the message status */
                if (strncasecmp(buffer,"Status:",7) == 0) {
                    if (mp->retr_flag)
                        (void)fputs("Status: RO\n",md);
                    else
                        (void)fputs(buffer, md);
                    status_written++;
                    continue;
                }
                /*  A blank line signals the end of the header. */
                if (*buffer == '\n') {
                    doing_body = 1;

#ifndef NOSTATUS
                    if (status_written == 0) {
                        if (mp->retr_flag)
                            (void)fputs("Status: RO\n\n",md);
                        else
                            (void)fputs("Status: U\n\n",md);
                    }
		    else
#endif /* NOSTATUS */
                    (void)fputs ("\n", md);
                    continue;
                }
                /*  Save another header line */
                (void)fputs (buffer, md);
	    }
	    else { /* Body */ 
		if (strncmp(buffer,"\001\001\001\001",4) == 0) {
		    (void)fputs (buffer, md);
		    break;
		}
		if(is_msg_boundary(buffer))
		  break;
                (void)fputs (buffer, md);
	    }
	}
    }

    /* flush and check for errors now!  The new mail will writen
       without stdio,  since we need not separate messages */

    (void)fflush(md) ;
    if (ferror(md)) {
        (void)ftruncate(mfd,0) ;
        (void)fclose(md) ;
        (void)fclose(p->drop) ;
        return pop_msg(p,POP_FAILURE,standard_error);
    }

    /* Go to start of new mail if any */
    (void)lseek((int)fileno(p->drop),offset,SEEK_SET);

    while((nchar=read((int)fileno(p->drop),buffer,BUFSIZ)) > 0)
        if ( nchar != write(mfd,buffer,nchar) ) {
            nchar = -1;
            break ;
        }
    if ( nchar != 0 ) {
        (void)ftruncate(mfd,0) ;
        (void)fclose(md) ;
        (void)fclose(p->drop) ;
        return pop_msg(p,POP_FAILURE,standard_error);
    }

    /*  Close the maildrop and empty temporary maildrop */
    (void)fclose(md);
    (void)ftruncate((int)fileno(p->drop),0);
    (void)fclose(p->drop);

    return(pop_quit(p));
}




is_msg_boundary(line)
     char *line;
{
  if(strncmp(line, "\001\001\001\001", 4) == 0) 
    return(1);
  
  if(strncmp(line, "From ", 5) != 0) 
    return(0);
  
  line += 5;
  while((*line != ' ') && (*line != '\0'))
    ++line;

  if(*line++ != ' ')
    return(0);
 
  /*
   * check the line length but some timestamps do not include time zone 
   */
 
  if(strlen(line) < 24)
    return(0);

  /* Tue */
  line += 3;
  if(*line++ != ' ')
    return(0);

  /* Jan */
  line += 3;
  if(*line++ != ' ')
    return(0);
  
  /* 22 */  
  line += 2;
  if(*line++ != ' ')
    return(0);

  /* 18:21:34 */
  line += 2;
  if(*line++ != ':')
    return(0);

  line += 2;
  if(*line++ != ':')
    return(0);

  line += 2;
  if(*line++ != ' ')
    return(0);

  return(1);
}
