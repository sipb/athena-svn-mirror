/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC daemon.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: server_defines.h,v 1.7 2002-07-05 21:32:34 zacheiss Exp $
 */

#include <mit-copyright.h>

#ifndef __server_defines_h
#define __server_defines_h __FILE__

#include "olc/macros.h"    /* some useful definitions */
#include "olxx_paths.h"    /* basic server paths, conditionalized for OLxx */

#define OLC_SRVTAB 		OLXX_CONFIG_DIR "/srvtab"
#define DATABASE_FILE 		OLXX_CONFIG_DIR "/database"
#define SPECIALTY_DIR 		OLXX_SPEC_DIR
#define ACL_DIR 		OLXX_ACL_DIR
#define TOPIC_FILE 		OLXX_CONFIG_DIR "/topics"
#define MOTD_FILE 		OLXX_CONFIG_DIR "/motd"
#define MOTD_TIMEOUT_FILE	OLXX_CONFIG_DIR "/motd_timeout"
#define MOTD_HOLD_FILE 		OLXX_CONFIG_DIR "/motd_hold"
#define MACH_TRANS_FILE 	OLXX_CONFIG_DIR "/translations"
#define HOURS_FILE		OLXX_CONFIG_DIR "/hours"
#define SERVICES_FILE 		OLXX_CONFIG_DIR "/services" /*used by MacOLX*/

#define ERROR_LOG 		OLXX_LOG_DIR "/errors"
#define STATUS_LOG 		OLXX_LOG_DIR "/status"
#define ADMIN_LOG 		OLXX_LOG_DIR "/admin"
#define STDERR_LOG 		OLXX_LOG_DIR "/errors"

#define CORE_DIR		OLXX_SPOOL_DIR
#define LOG_DIR 		OLXX_QUEUE_DIR
#define BINARY_BACKUP_FILE 	OLXX_SPOOL_DIR "/backup.dat"
#define BINARY_BACKUP_TEMP 	BINARY_BACKUP_FILE ".temp"
#define ASCII_BACKUP_FILE 	OLXX_SPOOL_DIR "/backup.ascii"
#define ASCII_BACKUP_TEMP 	ASCII_BACKUP_FILE ".temp"
#define LIST_FILE_NAME 		OLXX_SPOOL_DIR "/queue" /* was: qlist_-1.log */
#define LIST_TMP_NAME 		OLXX_SPOOL_DIR "/queue.tmp"
#define ASK_STATS_FILE		OLXX_STAT_DIR "/ask_stats"
#define RES_STATS_FILE		OLXX_STAT_DIR "/res_stats"

#if defined (OLTA) || defined (OWL)
#define LUMBERJACK_LOC          "/usr/athena/etc/lumberjack." OLXX_SERVICE
#else
#define LUMBERJACK_LOC		"/usr/athena/etc/lumberjack"
#endif

#ifdef HAVE_ZEPHYR
#define ZEPHYR_DOWN_FILE	OLXX_SPOOL_DIR "/punt_zephyr"
#ifndef ZEPHYR_PUNT_TIME
#define ZEPHYR_PUNT_TIME	15
#endif
#endif /* HAVE_ZEPHYR */

#ifdef HAVE_KRB4
#define TICKET_FILE		"/tmp/tkt_" OLXX_SERVICE
#endif /* HAVE_KRB4 */

/* Use by the acl checking code, so you need it even if you don't have
   kerberos
*/

/* system defines */

#define NOW                    (time((time_t *)NULL))
#define DAEMON_TIME_OUT        10
#define MAX_CACHE_SIZE         500

#define SYSLOG_FACILITY LOG_LOCAL6

/* for notifications */

#define OLCD_TIMEOUT    5

/* random internal flags */

#define NULL_FLAG       0
#define FORWARD         1	
#define NOTIFY          2	
#define UNANSWERED      3       /* Question was never answered. */
#define ANSWERED        4       /* Question was answered. */
#define PING            5
#define NO_RESPOND      6

/* Additional size constants. */

#define DB_LINE         1000    /* maximum length of a database line.*/
#define MAX_SEEN        81      /* Maximum number of consultants to */
#define SPEC_SIZE       10                     

/* priority queues */

#define EMERGENCY_Q       1
#define ACTIVE_Q          2
#define INACTIVE_Q        4
#define PIT_Q             8

/* type classifiers */

#define IS_TOPIC               500
#define IS_SUBTOPIC            501

#ifdef OLTA
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "TA"
#else
#ifdef OWL
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "Librarian"
#else
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "consultant"
#endif /* OWL */
#endif /* OLTA */

#define OLCD_SERVICE_NAME  OLXX_SERVICE "-locking"
#define RPD_SERVICE_NAME   OLXX_SERVICE "-query"

#endif
