/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the main routine of the user program, "olc".
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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_data.c,v $
 *      $Author: tjcoppet $
 */


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

#ifndef TESTHOST
char *OLC_PROMPT = "olc> ";
char *OLCR_PROMPT = "olcr> ";
char *OLCA_PROMPT = "olca> ";
#else TESTHOST
char *OLC_PROMPT = "olc test> ";
char *OLCR_PROMPT = "olcr test> ";
char *OLCA_PROMPT = "olca test> ";
#endif TESTHOST

/* Where to find help. */

char *OLC_HELP_DIR  =   "/usr/athena/lib/olc/olc_help";
char *OLC_HELP_EXT  =   ".help";
char *OLC_HELP_FILE =   "olc";

char *OLCR_HELP_DIR  =   "/usr/athena/lib/olc/olcr_help";
char *OLCR_HELP_EXT  =   ".help";
char *OLCR_HELP_FILE =   "olcr";

char *OLCA_HELP_DIR  =   "/usr/athena/lib/olc/olca_help";
char *OLCA_HELP_EXT  =   ".help";
char *OLCA_HELP_FILE =   "olca";

char *HELP_FILE;
char *HELP_EXT;
char *HELP_DIR;
