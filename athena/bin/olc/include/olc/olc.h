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
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/olc.h,v $
 *	$Id: olc.h,v 1.26 1996-09-20 02:26:23 ghudson Exp $
 *	$Author: ghudson $
 */

#include <mit-copyright.h>

#ifndef __olc_olc_h
#define __olc_olc_h

#include <stdio.h>
#if defined(__STDC__) && !defined(__HIGHC__) && !defined(SABER)
/* Stupid High-C claims to be ANSI but doesn't have the include files.. */
/* Ditto for saber */
#include <stdlib.h>
#endif
#include <string.h>
#if defined(_AUX_SOURCE) || defined(SOLARIS)
#include <string.h>
#endif

#include <olc/lang.h>

#ifdef KERBEROS
#include <krb.h>
#endif

#ifdef HESIOD
#include <hesiod.h>
#endif

struct tREQUEST;

#include <olc/os.h>
#include <olc/macros.h>
#include <olc/structs.h>
#include <olc/requests.h>
#include <common.h>

#include <olc/procs.h>
#include <olc/status.h>

#define VERSION_STRING "3.1"

/* 
 * service definitions 
 */

#define OLC_SERV_NAME  "sloc"                 /* nameservice key */
#define OLC_SERVICE    "olc"                  /* olc service name */
#define OLC_PROTOCOL   "tcp"                  /* protocol */
#ifdef ATHENA
#define OLC_SERVER     "MATISSE.MIT.EDU"      /* in case life fails */
#else
/* Define to be whatever's appropriate for your site.. */
#define OLC_SERVER	"FOO.BAR.EDU"
#endif

#define OLC_PROMPT	"olc> "		      /* Default OLC prompt */
#define OLCR_PROMPT	"olcr> "	      /* Default OLCR prompt */

#define DEFAULT_CONSULTANT_TITLE "consultant"

#define OLC_HELP_DIR	"/usr/athena/lib/olc/olc_help"
#define OLC_HELP_EXT	".help"
#define OLC_HELP_FILE	"olc"

#define OLCR_HELP_DIR	"/usr/athena/lib/olc/olcr_help"
#define OLCR_HELP_EXT	".help"
#define OLCR_HELP_FILE	"olcr"

#define OLC_SERVICE_NAME	"OLC"

#ifdef KERBEROS
#define K_SERVICE      "olc"                  /* Kerberos service name */
#define K_INSTANCE     "*"                    /* whatever instance applies */
extern char *LOCAL_REALM;
extern char *LOCAL_REALMS[];
extern char REALM[];
extern char INSTANCE[];
#endif /* KERBEROS */

extern PERSON User;
extern STATUS Status_Table[];
extern char DaemonHost[];

/*
 * misc stuff
 */

#define CLIENT_TIME_OUT 300     
#ifdef ATHENA
#define DEFAULT_MAILHUB "athena.mit.edu"
#else
/* Define to be whatever's appropriate to your site.. */
#define DEFAULT_MAILHUB "foo.bar.edu"
#endif

#endif /* __olc_olc_h */
