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
 *      MIT Project Athena
 *
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/data.c,v $
 *      $Author: tjcoppet $
 */

#ifndef lint
static char rcsid[]= "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/lib/data.c,v 1.4 1989-11-17 14:16:45 tjcoppet Exp $";
#endif

#include <olc/olc.h>

PERSON User;
char DaemonHost[NAME_LENGTH];

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
  {SERVICED,      "active"},
  {PICKUP,        "pickup"},
  {REFERRED,      "refer"},
  {LOGGED_OUT,    "logout"},
  {MACHINE_DOWN,  "mach down"},
  {ACTIVE,        "active"},
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
