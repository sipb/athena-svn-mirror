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
 *	Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1985,1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/structs.h,v $
 *	$Id: structs.h,v 1.10 1991-03-28 13:24:13 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

/* Structure describing a person. */

typedef struct tPERSON 
{
  int     uid;                       /* Person's user ID. */
  int     instance;                  /* the user's instance id */
  char    username[LOGIN_SIZE+1];    /* Person's username. */
  char    realname[TITLE_SIZE];      /* Person's real name. */
#ifdef KERBEROS
  char    realm[REALM_SZ];           /* current realm */
  char    inst[INST_SZ];             /* oh well */
#endif /* KERBEROS */
  char    nickname[STRING_SIZE];     /* Person's first name. */
  char    title[TITLE_SIZE];         /* Person's title */
  char    machine[TITLE_SIZE];       /* Person's current machine. */
} PERSON;

/* Structure describing the list */

typedef struct tLIST 
{
  int    ustatus;
  int    cstatus;
  int    ukstatus;
  int    ckstatus;
  long   utime;
  long   ctime;
  int    umessage;
  int    cmessage;
  int    nseen;
  char   topic[TOPIC_SIZE];
  char   note[NOTE_SIZE];
  struct tPERSON user;
  struct tPERSON connected;
} LIST;

typedef struct tDBINFO
{
  int max_ask;
  int max_answer;
  char title1[TITLE_SIZE];
  char title2[TITLE_SIZE];
} DBINFO ;
