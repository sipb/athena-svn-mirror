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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_data.c,v $
 *	$Id: olc_data.c,v 1.8 1990-07-16 08:21:21 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/olc/olc_data.c,v 1.8 1990-07-16 08:21:21 lwvanels Exp $";
#endif

#include <mit-copyright.h>
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
