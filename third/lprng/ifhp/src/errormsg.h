/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****
$Id: errormsg.h,v 1.1.1.2 1999-05-04 18:50:41 mwhitson Exp $
 **** ENDHEADER ****/

#ifndef _ERRORMSG_H
#define _ERRORMSG_H

#if defined(HAVE_STDARGS)
void logmsg (char *msg,...);
void fatal (char *msg,...);
void logerr (char *msg,...);
void logerr_die (char *msg,...);
void Diemsg (char *msg,...);
void Warnmsg (char *msg,...);
void logDebug (char *msg,...);
#else
void logmsg ();
void fatal ();
void logerr ();
void logerr_die ();
void Diemsg ();
void Warnmsg ();
void logDebug ();
#endif

const char * Errormsg ( int err );
char *Time_str(int shortform, time_t t);
extern int Write_fd_str( int fd, char *str);
extern int Write_fd_len( int fd, char *str, int len);
const char *Sigstr (int n);


#endif
