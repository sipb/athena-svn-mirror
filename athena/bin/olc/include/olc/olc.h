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
 *	$Id: olc.h,v 1.32 2002-07-01 05:16:03 zacheiss Exp $
 */

#include <mit-copyright.h>

#ifndef __olc_olc_h
#define __olc_olc_h

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <olc/lang.h>

#ifdef HAVE_KRB4
#include <krb.h>
#endif

#ifdef HAVE_HESIOD
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
#define OLC_PROTOCOL   "tcp"                  /* protocol */

#define OLC_FALLBACK_PROMPT  "ol\?\?> "     /* prompt if no config file */
#define OLC_FALLBACK_TITLE   "consultant"   /* consultant title if no config */

/* Default path for the incarnation configuration files. */
#define OLC_CONFIG_PATH "/usr/athena/lib/olc:/mit/olta/config:/mit/library/config"
#define OLC_CONFIG_EXT  ".cfg"

#define OLC_DEFAULT_HELP_EXT	".help"

#ifdef HAVE_KRB4
#define K_SERVICE      "olc"                  /* Kerberos service name */
#define K_INSTANCE     "*"                    /* whatever instance applies */
/* Kerberos ticket lifetime, in units of 5-minute chunks.  [6 hours] */
#define TICKET_LIFE    (6*12)
/* at what age we try getting new tickets, in units of 5-minute chunks. */
#define TICKET_WHEN    (TICKET_LIFE-3)
/* Delay between checking the state of the tickets, in minutes. */
#define TICKET_FREQ    1
extern char *LOCAL_REALM;
extern char *LOCAL_REALMS[];
extern char REALM[];
extern char INSTANCE[];
#endif /* HAVE_KRB4 */

extern PERSON User;
extern STATUS Status_Table[];
extern char DaemonHost[];

/*
 * misc stuff
 */

#define CLIENT_TIME_OUT 300     
#define OLC_MAIL_REPLY_ADDRESS	"olc@mit.edu"

#endif /* __olc_olc_h */
