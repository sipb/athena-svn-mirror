/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions common to all parts of OLC.
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
 *      Copyright (c) 1985,1988 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/procs.h,v $
 *      $Author: vanharen $
 *      $Id: procs.h,v 1.4 1990-01-09 16:03:21 vanharen Exp $
 */

/* Declarations of common functions. */

#include <olc/lang.h>

#if ! __STDC__
char *malloc(), *realloc();
#else
void *malloc (unsigned int), *realloc (void *, unsigned int);
#endif

char *read_text_from_fd OPrototype((int));
char *ttyname OPrototype((int));
char *getenv OPrototype((const char *));
char *parse_list();
char *get_next_line();
void expand_hostname();

ERRCODE OAsk();
ERRCODE OGrab();
ERRCODE OForward();
ERRCODE OSignOn();
ERRCODE OSignOff();
ERRCODE ORequest();
ERRCODE OVerifyInstance();
ERRCODE OReplayLog();
ERRCODE OShowMessage();
ERRCODE OGetMessage();
ERRCODE OGetMOTD();
ERRCODE OChangeMOTD();
ERRCODE OListQueue();
ERRCODE OReadList();
ERRCODE OResolve();
ERRCODE OComment();
ERRCODE OMail();
ERRCODE OSend();
ERRCODE OMailHeader();
ERRCODE OGetTopic();
ERRCODE OChangeTopic();
ERRCODE OListTopics();
ERRCODE OVerifyTopic();
ERRCODE OHelpTopic();
