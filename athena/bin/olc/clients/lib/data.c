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
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/data.c,v $
 *	$Id: data.c,v 1.9 1990-11-17 15:10:20 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/data.c,v 1.9 1990-11-17 15:10:20 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>

PERSON User;
char DaemonHost[NAME_SIZE];

STATUS Status_Table[] = 
{
  {OFF,           "off"},
  {ON,            "on"},
  {FIRST,         "sp1"},
  {DUTY,          "duty"},
  {SECOND,        "sp2"},
  {URGENT,        "urgent"},
  {BUSY,          "busy"},
  {CACHED,        "cached"},
  {PENDING,       "pending"},
  {NOT_SEEN,      "unseen"},
  {DONE,          "done"},
  {CANCEL,        "cancel"},
  {PICKUP,        "pickup"},
  {REFERRED,      "refer"},
  {LOGGED_OUT,    "logout"},
  {MACHINE_DOWN,  "mach down"},
  {ACTIVE,        "active"},
  {SERVICED,      "active"},
  {UNKNOWN_STATUS,"unknown"},
};


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
