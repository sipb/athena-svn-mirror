/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC daemon and the lumberjack program.
 *
 *      Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/lumberjack.h,v $
 *	$Id: lumberjack.h,v 1.2 1990-12-07 09:16:41 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __lumberjack_h
#define __lumberjack_h __FILE__

char *DONE_DIR		 = "/usr/spool/olc/donelogs";

#define DSPIPE	"/usr/athena/dspipe" /* name of program to send off logs */

#ifdef OLX
#define PREFIX	"FIONAVAR.MIT.EDU:/usr/spool/discuss/ot" /* meeting prefix */
#else
#define PREFIX	"MATISSE.MIT.EDU:/usr/spool/discuss/o"
#endif

#ifdef LAVIN
#define PREFIX  "NEMESIS.MIT.EDU:/usr/spool/discuss/o"
#endif

#define LOCKFILE  "lockfile"	/* name of lockfile */
#define SIZE	1024

#endif /* __lumberjack_h */
