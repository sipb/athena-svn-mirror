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
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/structs.h,v $
 *      $Author: tjcoppet $
 *      $Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/structs.h,v 1.2 1989-08-08 10:46:52 tjcoppet Exp $
 */

/* Structure describing a person. */

typedef struct tPERSON 
{
  int     uid;                       /* Person's user ID. */
  int     instance;                  /* the user's instance id */
  char    username[LOGIN_SIZE+1];    /* Person's username. */
  char    realname[LABEL_LENGTH];    /* Person's real name. */
  char    realm[REALM_SZ];           /* current realm */
  char    inst[INST_SZ];             /* oh well */
  char    nickname[STRING_LENGTH];   /* Person's first name. */
  char    title[LABEL_LENGTH];       /* Person's title */
  char    machine[LABEL_LENGTH];     /* Person's current machine. */
} PERSON;


/* Structure describing the list */

typedef struct tLIST 
{
  int    ustatus;
  int    cstatus;
  int    ukstatus;
  int    ckstatus;
  int    nseen;
  char   topic[TOPIC_SIZE];
  char   note[NOTE_SIZE];
  struct tPERSON user;
  struct tPERSON connected;
} LIST;


typedef struct tSTATUS
{
  int status;
  char label[LABEL_LENGTH];
} STATUS;
