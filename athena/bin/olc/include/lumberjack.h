/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC daemon and the lumberjack program.
 *
 *      Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: lumberjack.h,v 1.9 1999-03-06 16:48:25 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __lumberjack_h
#define __lumberjack_h __FILE__

#include <stdio.h>
#include <olxx_paths.h>

/* Directory where done'd questions live (eg. "/usr/spool/olc/donelogs") */
#define DONE_DIR    OLXX_DONE_DIR

/* File (eg. "/etc/athena/olc/ds_prefix") containing prefix for Discuss
   archives (eg. "MATISSE.MIT.EDU:/usr/spool/discuss/o") */
#define PREFIXFILE   OLXX_CONFIG_DIR "/ds_prefix"

#define LOCKFILE  "lockfile"	/* name of lockfile, relative to DONE_DIR */

#define LINE_CHUNK	1024	/* length quantum for control-file buffer */
#define SYSLOG_LEN	 128	/* maximum length for strings sent to syslog */

/* prototypes for tools.c */

int do_lock(int fd);
int do_unlock(int fd);

char *get_line (FILE *fd);

#endif /* __lumberjack_h */
