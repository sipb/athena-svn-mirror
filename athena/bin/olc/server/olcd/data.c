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
 *      MIT Project Athena
 *
 *      Copyright (c) 1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/data.c,v $
 *      $Author: tjcoppet $
 */


#include <olc/olc.h>
#include <olcd.h>


char *OLC_DATABASE  = "/usr/lib/olc/database";
char *SPECIALTY_DIR = "/usr/lib/olc/specialties";
char *ACL_DIR       = "/usr/lib/olc/acls";
char *LOG_DIR       = "/usr/spool/olc";
char *BACKUP_FILE   = "/usr/spool/olc/backup.dat";
char *BACKUP_TEMP   = "/usr/spool/olc/backup.temp";
char *ERROR_LOG     = "/usr/adm/olc/errors";
char *STATUS_LOG    = "/usr/adm/olc/status";
char *STDERR_LOG    = "/usr/adm/olc/errors";
char *TOPIC_FILE    = "/usr/lib/olc/topics";
char *USER_FILE     = "/usr/lib/olc/database";
char *MOTD_FILE     = "/usr/lib/olc/motd";

#ifdef KERBEROS
char *SERVER_REALM = "ATHENA.MIT.EDU";
char *SRVTAB_FILE  = "/etc/srvtab";
char K_INSTANCEbuf[INST_SZ];
#endif KERBEROS


/* NF_PREFIX is prepended to the topic to create a specific discuss mtg */
#ifndef TEST
char *NF_PREFIX = "MATISSE.MIT.EDU:/usr/spool/discuss/o";
#else TEST
char *NF_PREFIX = "PICASSO.MIT.EDU:/usr/spool/discuss/o";
#endif TEST

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
  OLC_CHANGE_TOPIC,    olc_chtopic,          "olc chtopic",
  OLC_CREATE_INSTANCE, olc_create_instance,  "olc split",
  OLC_LIST_TOPICS,     olc_list_topics,      "olc list topics",
  OLC_MOTD,            olc_motd,             "olc motd",
  OLC_VERIFY_TOPIC,    olc_verify_topic,     "olc verify topic",
  OLC_VERIFY_INSTANCE, olc_verify_instance,  "olc verify",
  OLC_DEFAULT_INSTANCE,olc_default_instance, "olc default instance",
  OLC_DUMP,            olc_dump,             "olc dump",
  OLC_LOAD_USER,       olc_load_user,        "olc load user",
  UNKNOWN_REQUEST,     (int(*)()) NULL,      (char *) NULL,
};

KNUCKLE **Knuckle_List  = (KNUCKLE **) NULL;
TOPIC   **Topic_List    = (TOPIC **) NULL;

ACL Acl_List[] = 
{
  ON_ACL,             "on.acl",
  MONITOR_ACL,        "monitor.acl",
  OLC_ACL,            "olc.acl",
  CONSULT_ACL,        "consult.acl",
  GRAB_ACL,           "grab.acl",
  GRESOLVE_ACL,       "gresolve.acl",
  GASK_ACL,           "gask.acl",
  GCOMMENT_ACL,       "gcomment.acl",
  GMESSAGE_ACL,       "gmessage.acl",
  ADMIN_ACL,          "admin.acl",
  GCHTOPIC_ACL,       "gchtopic.acl",
  0,                  (char *) NULL,
};
