/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
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
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: data.c,v 1.13 1999-01-22 23:12:03 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: data.c,v 1.13 1999-01-22 23:12:03 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <sys/param.h>
#ifdef SOLARIS
#include <netdb.h>
#endif


PERSON User;
char DaemonHost[MAXHOSTNAMELEN];

char REALM[40];
char INSTANCE[40];

char *LOCAL_REALM = "ATHENA.MIT.EDU";
char *LOCAL_REALMS[] =
{
  "MIT.EDU",
  "DU.MIT.EDU",
  "SIPB.MIT.EDU",
  "PIKA.MIT.EDU",
  "CARLA.MIT.EDU",
  "ZBT.MIT.EDU",
  "",
};

LIST  list_cache;
long  lc_time = 0;
