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
 *      $Author: tjcoppet $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc.h,v 1.2 1989-08-08 10:44:48 tjcoppet Exp $
 */

#include <stdio.h>                  
#include <strings.h>                

#ifdef KERBEROS
#include <krb.h>
#endif KERBEROS

#ifdef HESIOD
#include <hesiod.h>
#endif HESIOD

#include <olc/macros.h>
#include <olc/structs.h>
#include <olc/requests.h>
#include <olc/procs.h>
#include <olc/status.h>

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

extern STATUS Status_Table[];

/*
 * misc stuff
 */

#define CLIENT_TIME_OUT 300     
#define DEFAULT_MAILHUB "Athena.mit.edu"

#ifdef HESIOD
char **hes_resolve();
#endif HESIOD

void expand_hostname();
