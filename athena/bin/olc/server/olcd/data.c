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
 *	$Id: data.c,v 1.27 1999-03-06 16:48:53 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: data.c,v 1.27 1999-03-06 16:48:53 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olcd.h>

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
  OLC_LIST_SERVICES,   olc_list_services,    "olc list services",
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
  OLC_SET_USER_STATUS, olc_set_user_status,  "olc set login/out status",
  OLC_GET_HOURS,       olc_get_hours,	     "olc get hours",
  OLC_CHANGE_HOURS,    olc_change_hours,     "olc change hours",
  OLC_VERSION,	       olc_version,          "olc version",
#ifdef HAVE_ZEPHYR
  OLC_TOGGLE_ZEPHYR,   olc_toggle_zephyr,    "olc toggle zephyr",
#endif
  UNKNOWN_REQUEST,     (FUNCTION) NULL,  (char *) NULL,
};

PROC Maint_Proc_List[] = 
{
  OLC_STARTUP,         olc_startup,	     "olc startup",
  OLC_MOTD,            olc_motd,             "olc motd",
  /* NOTE we bind the show and replay requests to display the motd... */
  OLC_SHOW,            olc_motd,	     "olc show",
  OLC_REPLAY,          olc_motd,	     "olc replay",
  OLC_CHANGE_MOTD,     olc_change_motd,      "olc change motd",
  OLC_GET_HOURS,       olc_get_hours,	     "olc get hours",
  OLC_CHANGE_HOURS,    olc_change_hours,     "olc change hours",
  OLC_VERSION,	       olc_version,          "olc version",
  OLC_LIST,            olc_list,             "olc list",
  OLC_DUMP,            olc_dump,             "olc dump",
  UNKNOWN_REQUEST,     (FUNCTION) NULL,  (char *) NULL,
};

KNUCKLE **Knuckle_List  = (KNUCKLE **) NULL;
TOPIC   **Topic_List    = (TOPIC **) NULL;

KNUCKLE		*Knuckle_free = (KNUCKLE *) NULL;
KNUCKLE		*Knuckle_inuse = (KNUCKLE *) NULL;
USER		*User_free = (USER *) NULL;
USER		*User_inuse = (USER *) NULL;
QUESTION	*Question_free = (QUESTION *) NULL;
QUESTION	*Question_inuse = (QUESTION *) NULL;

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
