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



#ifndef TESTHOST
char *OLC_PROMPT = "olc> ";
char *OLCR_PROMPT = "olcr> ";
#else TESTHOST
char *OLC_PROMPT = "olc test> ";
char *OLCR_PROMPT = "olcr test> ";
#endif TESTHOST

/* Where to find help. */

char *OLC_HELP_DIR  =   "/usr/athena/lib/olc/olc_help";
char *OLC_HELP_EXT  =   ".help";
char *OLC_HELP_FILE =   "olc";

char *OLCR_HELP_DIR  =   "/usr/athena/lib/olc/olcr_help";
char *OLCR_HELP_EXT  =   ".help";
char *OLCR_HELP_FILE =   "olcr";

char *HELP_FILE;
char *HELP_EXT;
char *HELP_DIR;
