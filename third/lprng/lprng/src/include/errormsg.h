/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: errormsg.h,v 1.1.1.2 1999-05-04 18:07:08 danw Exp $
 ***************************************************************************/



#ifndef _ERRORMSG_H_
#define _ERRORMSG_H_ 1

#if defined(HAVE_STDARGS)
void logmsg(int kind, char *msg,...);
void fatal(int kind, char *msg,...);
void logerr(int kind, char *msg,...);
void logerr_die(int kind, char *msg,...);
void Diemsg(char *msg,...);
void Warnmsg(char *msg,...);
void logDebug(char *msg,...);
void Msg(char *msg,...);
#else
void logmsg();
void fatal();
void logerr();
void logerr_die();
void Diemsg();
void Warnmsg();
void logDebug();
void Msg();
#endif

/* PROTOTYPES */
const char * Errormsg( int err );
const char *Sigstr(int n);
const char *Decode_status(plp_status_t *status);
char *Server_status( int d );

#endif
