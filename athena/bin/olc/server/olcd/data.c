/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains static definitions for the OLC daemon.
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
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data.c,v $
 *	$Id: data.c,v 1.17 1991-01-01 13:53:13 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data.c,v 1.17 1991-01-01 13:53:13 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olcd.h>

#include "lumberjack.h"

char *DATABASE_FILE      = "/usr/lib/olc/database";
char *SPECIALTY_DIR      = "/usr/lib/olc/specialties";
char *ACL_DIR            = "/usr/lib/olc/acls";
char *LOG_DIR            = "/usr/spool/olc";
char *BACKUP_FILE        = "/usr/spool/olc/backup.dat";
char *BACKUP_TEMP        = "/usr/spool/olc/backup.temp";
char *ERROR_LOG          = "/usr/adm/olc/errors";
char *REQ_STATS_LOG      = "/usr/adm/olc/requests.stats";
char *QUES_STATS_LOG     = "/usr/adm/olc/question.stats";
char *STATUS_LOG         = "/usr/adm/olc/status";
char *ADMIN_LOG          = "/usr/adm/olc/admin";
char *STDERR_LOG         = "/usr/adm/olc/errors";
char *TOPIC_FILE         = "/usr/lib/olc/topics";
char *MOTD_FILE          = "/usr/lib/olc/motd";
char *MOTD_TIMEOUT_FILE  = "/usr/lib/olc/motd_timeout";
char *MOTD_HOLD_FILE	 = "/usr/lib/olc/motd_hold";
char *MACH_TRANS_FILE    = "/usr/lib/olc/translations";

char *LIST_FILE_NAME     = "/usr/spool/olc/qlist_-1.log";
char *LIST_TMP_NAME      = "/usr/spool/olc/queue.tmp";

#ifdef KERBEROS
char *TICKET_FILE        = "/usr/spool/olc/tkt.olc";
char *DFLT_SERVER_REALM  = "ATHENA.MIT.EDU";
char *SRVTAB_FILE        = "/usr/lib/olc/srvtab";
char SERVER_REALM[REALM_SZ];
char K_INSTANCEbuf[INST_SZ];
#endif /* KERBEROS */


/* declaraction of procedure table */

PROC Proc_List[] = 
{
  OLC_TOPIC,           olc_topic,	     "olc topic",
  OLC_COMMENT,         olc_comment,	     "olc comment",
  OLC_DESCRIBE,        olc_describe,         "olc describe",
  OLC_DONE,            olc_done,	     "olc done",
  OLC_CANCEL,          olc_cancel,           "olc cancel",
  OLC_FORWARD,         olc_forward,	     "olc forward",
  OLC_LIST,            olc_list,             "olc list",
  OLC_MAIL,            olc_mail,	     "olc mail",
  OLC_ON,              olc_on,	             "olc on",
  OLC_OFF,             olc_off,	             "olc off",
  OLC_REPLAY,          olc_replay,	     "olc replay",
  OLC_SEND,            olc_send,             "olc send",
  OLC_STARTUP,         olc_startup,	     "olc startup",
  OLC_SHOW,            olc_show,	     "olc show",
  OLC_GRAB,            olc_grab,	     "olc grab",
  OLC_CANCEL,          olc_cancel,	     "olc cancel",
  OLC_ASK,             olc_ask,              "olc ask",
  OLC_WHO,             olc_who,              "olc who",
  OLC_SET_TOPIC,       olc_chtopic,          "olc chtopic",
  OLC_CREATE_INSTANCE, olc_create_instance,  "olc split",
  OLC_LIST_TOPICS,     olc_list_topics,      "olc list topics",
  OLC_MOTD,            olc_motd,             "olc motd",
  OLC_VERIFY_TOPIC,    olc_verify_topic,     "olc verify topic",
  OLC_VERIFY_INSTANCE, olc_verify_instance,  "olc verify",
  OLC_DEFAULT_INSTANCE,olc_default_instance, "olc default instance",
  OLC_DUMP,            olc_dump,             "olc dump",
  OLC_LOAD_USER,       olc_load_user,        "olc load user",
  OLC_CHANGE_MOTD,     olc_change_motd,      "olc change motd",
  OLC_CHANGE_ACL,      olc_change_acl,       "olc change acl",
  OLC_LIST_ACL,        olc_list_acl,         "olc list acl",
  OLC_GET_DBINFO,      olc_get_dbinfo,       "olc db info",
/*  OLC_SET_DBINFO,      olc_change_dbinfo,    "olc db info"*/
  OLC_GET_ACCESSES,    olc_get_accesses,     "olc get access",
#ifdef __STDC__
  UNKNOWN_REQUEST,     (FUNCTION) NULL,  (char *) NULL,
#else
  UNKNOWN_REQUEST,     (ERRCODE(*)()) NULL,  (char *) NULL,
#endif
};

KNUCKLE **Knuckle_List  = (KNUCKLE **) NULL;
TOPIC   **Topic_List    = (TOPIC **) NULL;

ACL Acl_List[] = 
{
  ON_ACL,             "on.acl",            "on",
  MONITOR_ACL,        "monitor.acl",       "monitor",
  OLC_ACL,            "olc.acl",           "olc",
  CONSULT_ACL,        "consult.acl",       "consult",
  GRAB_ACL,           "grab.acl",          "grab",
  GRESOLVE_ACL,       "gresolve.acl",      "gresolve",
  GASK_ACL,           "gask.acl",          "gask",
  GCOMMENT_ACL,       "gcomment.acl",      "gcomment",
  GMESSAGE_ACL,       "gmessage.acl",      "gmessage",
  MOTD_ACL,           "motd.acl",          "motd",
  ADMIN_ACL,          "admin.acl",         "admin",
  GCHTOPIC_ACL,       "gchtopic.acl",      "gchtopic",
  0,                  (char *) NULL,       (char *) NULL,
};
