/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedure declarations for OLC.
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
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/procs.h,v $
 *	$Id: procs.h,v 1.5 1990-05-25 16:13:00 vanharen Exp $
 *	$Author: vanharen $
 */

#include <mit-copyright.h>

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
