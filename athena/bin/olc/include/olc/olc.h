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
 *	$Id: olc.h,v 1.28 1999-01-22 23:13:42 ghudson Exp $
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

#include <olc/incarnate.h>

#define VERSION_STRING "3.2"

/* 
 * service definitions 
 */

#define OLC_SERV_NAME  "sloc"                 /* nameservice key */
#define OLC_SERVICE    "olc"                  /* olc service name */
#define OLC_PROTOCOL   "tcp"                  /* protocol */

#define OLC_FALLBACK_PROMPT  "ol??> "       /* prompt if no config file */
#define OLC_FALLBACK_TITLE   "consultant"   /* consultant title if no config */

/* Default path for the incarnation configuration files. */
#define OLC_CONFIG_PATH "/usr/athena/lib/olc:/mit/olta/config:/mit/library/config"
#define OLC_CONFIG_EXT  ".cfg"

#define OLC_DEFAULT_HELP_EXT	".help"

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
#define DEFAULT_MAILHUB "mit.edu"
#else
/* Define to be whatever's appropriate to your site.. */
#define DEFAULT_MAILHUB "foo.bar.edu"
#endif

#endif /* __olc_olc_h */
