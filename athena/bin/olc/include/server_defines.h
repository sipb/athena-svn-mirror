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
 *	$Id: server_defines.h,v 1.5 1999-01-22 23:13:30 ghudson Exp $
 */
/*  NEED to change /usr/athena/lib to something like /var/server/olc/....  

*/
#include <mit-copyright.h>

#ifndef __server_defines_h
#define __server_defines_h __FILE__

#define DATABASE_FILE 		"/usr/athena/lib/olc/database"
#define SPECIALTY_DIR 		"/usr/athena/lib/olc/specialties"
#define ACL_DIR 		"/usr/athena/lib/olc/acls"
#define LOG_DIR 		"/usr/spool/olc"
#define BACKUP_FILE 		"/usr/spool/olc/backup.dat"
#define BACKUP_TEMP 		"/usr/spool/olc/backup.temp"
#define BACKUP_FILE_ASCII 	"/usr/spool/olc/backup.ascii"
#define BACKUP_TEMP_ASCII	"/usr/spool/olc/backup.ascii.temp"
#define ERROR_LOG 		"/usr/adm/olc/errors"
#define STATUS_LOG 		"/usr/adm/olc/status"
#define ADMIN_LOG 		"/usr/adm/olc/admin"
#define STDERR_LOG 		"/usr/adm/olc/errors"
#define TOPIC_FILE 		"/usr/athena/lib/olc/topics"
#define SERVICES_FILE 		"/usr/athena/lib/olc/services"
#define MOTD_FILE 		"/usr/athena/lib/olc/motd"
#define MOTD_TIMEOUT_FILE	"/usr/athena/lib/olc/motd_timeout"
#define MOTD_HOLD_FILE 		"/usr/athena/lib/olc/motd_hold"
#define MACH_TRANS_FILE 	"/usr/athena/lib/olc/translations"
#define LIST_FILE_NAME 		"/usr/spool/olc/qlist_-1.log"
#define LIST_TMP_NAME 		"/usr/spool/olc/queue.tmp"
#define HOURS_FILE		"/usr/athena/lib/olc/hours"
#define LUMBERJACK_LOC		"/usr/local/bin/lumberjack"
#define ASK_STATS_FILE		"/usr/spool/olc/stats/ask_stats"
#define RES_STATS_FILE		"/usr/spool/olc/stats/res_stats"

#ifdef ZEPHYR
#define ZEPHYR_DOWN_FILE	"/usr/spool/olc/punt_zephyr"
#define ZEPHYR_PUNT_TIME	15
#endif

#ifdef KERBEROS
#define TICKET_FILE		"/usr/spool/olc/tkt.olc"
#endif /* KERBEROS */

/* Use by the acl checking code, so you need it even if you don't have
   kerberos
*/

#ifdef ATHENA
#define DFLT_SERVER_REALM	"ATHENA.MIT.EDU"
#else
/* Put your realm here.... */
#define DFLT_SERVER_REALM	"ATHENA.MIT.EDU"
#endif /* ATHENA */

/* system defines */

#define NOW                    (time((time_t *)NULL))
#define DAEMON_TIME_OUT        10
#define MAX_CACHE_SIZE         500

#ifdef SYSLOG
#define SYSLOG_LEVEL LOG_LOCAL6
#endif

/* for notifications */

#define OLCD_TIMEOUT    10

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
#define DEFAULT_TITLE   "user"
#define DEFAULT_TITLE2  "consultant"
#endif

#endif
