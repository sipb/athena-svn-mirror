/*
 * This file is part of the OLC On-Line Consulting System.
 * It defines the (improved) OLC server installation layout.
 * Its contents may decide to move somewhere more appropriate, but
 * this was a reasonably easy way to centralize all the paths which
 * started "escaping" from server_defines.h .
 *
 *      bert Dvornik
 *      MIT Team Athena
 *
 * Copyright (C) 1996 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: olxx_paths.h,v 1.1 1999-03-06 16:48:28 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __olxx_paths_h
#define __olxx_paths_h __FILE__

/* First, we define OLXX_SERVICE to be a string containing the
 * (lowercase) name of the particular OLxx service we're currently
 * using.  This will be used as a name of various server directories.
 */

#ifndef OLXX_SERVICE
#ifdef OLTA
#define OLXX_SERVICE	"olta"
#else /* not OLTA */
#ifdef OWL
#define OLXX_SERVICE	"owl"	/* ...this used to be "olrl" */
#else /* not OWL */
#define OLXX_SERVICE	"olc"
#endif /* not OWL */
#endif /* not OLTA */
#endif /* OLXX_SERVICE */

/* Next, we define the top-level directories, and then descend.
 * People who still use non-ANSI cpp's should probably download GCC. =)
 * ../DOC.server-layout should contain a better description then the
 * code, so update it if you much with the paths.
 */

#define OLXX_CONFIG_DIR    "/etc/athena/" OLXX_SERVICE	/* "/etc/athena/olc" */
#define OLXX_ACL_DIR       OLXX_CONFIG_DIR "/acls"
#define OLXX_SPEC_DIR      OLXX_CONFIG_DIR "/specialties"
#define OLXX_MAIL_DIR      OLXX_CONFIG_DIR "/olcm"

#define OLXX_SPOOL_DIR    "/var/athena/" OLXX_SERVICE	/* "/var/athena/olc" */
#define OLXX_QUEUE_DIR    OLXX_SPOOL_DIR "/questions"
#define OLXX_DONE_DIR     OLXX_SPOOL_DIR "/donelogs"
#define OLXX_LOG_DIR      OLXX_SPOOL_DIR "/admin"
#define OLXX_STAT_DIR     OLXX_SPOOL_DIR "/stats"

#endif /* __olxx_paths_h */
