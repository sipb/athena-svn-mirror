/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions common to all parts of OLC.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *      MIT Project Athena
 *
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc.h,v $
 *      $Author: raeburn $
 *      $Id: olc.h,v 1.9 1990-01-05 06:12:50 raeburn Exp $
 */

#ifndef __olc_olc_h
#define __olc_olc_h

#include <stdio.h>
#include <string.h>

#include <olc/lang.h>

#if is_cplusplus
extern "C" {
#endif

#ifdef KERBEROS
#include <krb.h>
#endif

#ifdef HESIOD
#include <hesiod.h>
    char **hes_resolve OPrototype((const char *, const char *));
#if __STDC__
    int hes_error (void);
#else
    int hes_error OPrototype ((void));
#endif
#endif

#if is_cplusplus
};
#endif

#include <olc/os.h>
#include <olc/macros.h>
#include <olc/structs.h>
#include <olc/requests.h>
#include <olc/common.h>

#if is_cplusplus
extern "C" {
#endif

#include <olc/procs.h>
#include <olc/status.h>

void expand_hostname ();

#if is_cplusplus
};
#endif
    
#define VERSION_STRING "3.0a"

/* 
 * service definitions 
 */

#define OLC_SERV_NAME  "sloc"                 /* nameservice key */
#define OLC_SERVICE    "olc"                  /* olc service name */
#define OLC_PROTOCOL   "tcp"                  /* protocol */
#define OLC_SERVER     "PICASSO.MIT.EDU"      /* in case life fails */


#ifdef KERBEROS
#define K_SERVICE      "olc"                  /* Kerberos service name */
#define K_INSTANCE     "*"                    /* whatever instance applies */
extern char *LOCAL_REALM;
extern char *LOCAL_REALMS[];
extern char REALM[];
extern char INSTANCE[];
#endif KERBEROS

extern PERSON User;
extern STATUS Status_Table[];
extern char DaemonHost[];

/*
 * misc stuff
 */

#define CLIENT_TIME_OUT 300     
#define DEFAULT_MAILHUB "Athena.mit.edu"

#endif /* __olc_olc_h */
