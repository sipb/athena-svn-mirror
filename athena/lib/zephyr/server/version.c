/* This file is part of the Project Athena Zephyr Notification System.
 * It contains the version identification of the Zephyr server
 *
 *	Created by:	John T. Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/version.c,v $
 *	$Author: raeburn $
 *
 *	Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#include <zephyr/mit-copyright.h>

#ifdef DEBUG
extern const char version[] = "Zephyr Server (DEBUG) $Revision: 3.21 $";
#else
extern const char version[] = "Zephyr Server $Revision: 3.21 $";
#endif

#if !defined (lint) && !defined (SABER)
static const char rcsid_version_c[] =
    "$Id: version.c,v 3.21 1990-11-13 17:07:00 raeburn Exp $";
extern const char copyright[] =
    "Copyright (c) 1987,1988,1989,1990 Massachusetts Institute of Technology.\n";
#ifdef CONCURRENT
extern const char concurrent[] = "Brain-dump concurrency enabled";
#else
extern const char concurrent[] = "no brain-dump concurrency";
#endif
#endif
