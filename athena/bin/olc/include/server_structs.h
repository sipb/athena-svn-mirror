/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains structure definitions and global variables for the OLC daemon.
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
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: server_structs.h,v 1.5 1999-06-28 22:52:29 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __server_structs_h
#define __server_structs_h __FILE__

/* OLCD data definitions */

typedef ERRCODE (*FUNCTION) (int, struct tREQUEST *);
				/* A pointer to a function. */

typedef struct tUSER 
{
  struct tKNUCKLE **knuckles;       /* all user instances */
  int    uid;                       /* user id */
  char   username[LOGIN_SIZE];      /* user name */
  char   realname[NAME_SIZE];
  char   nickname[NAME_SIZE];         
  char   title1[NAME_SIZE];        /* title of user in OLC */
  char   title2[NAME_SIZE];        /* title of consultant in OLC */
  char   machine[HOSTNAME_SIZE];   /* user location */
  char   realm[NAME_SIZE];
  int    specialties[SPEC_SIZE];    /* Specialty list. */
  int    no_specialties;
  int    permissions;
  int    status;                    /* status of the user 
                                        (logout, idle, etc) */
  int    no_knuckles;               /* number of current connections */
  int    max_ask;                   /* maximum allowable connections */
  int    max_answer;
} USER;

typedef struct tdLIST
{
  char	*username;
  char	*machine;
  int	instance;
  int	ustatus;
  int	kstatus;
  char	*cusername;
  int	cinstance;
  int	cstatus;
  int	n_consult;
  char	*topic;
  long	timestamp;
  char	*note;
} D_LIST;

typedef struct tKNUCKLE
{
  struct tQUESTION *question;        /* question */
  struct tKNUCKLE *connected;        /* connected user */
  struct tUSER *user;                /* central user */
  char   *title;		     /* pointer to appropriate user title */
  int    instance;                   
  long   timestamp;                  /* specific to type */

  int    status;                     /* status of this instance 
                                        (on priorities, pending, etc..) */
  char   cusername[LOGIN_SIZE];
  int    cinstance;
  char   nm_file[NAME_SIZE+6];
  int    new_messages;              /* new messages for this knuckle */
				    /* 0 = none, 1 = yes, -1 = unknown */
} KNUCKLE;

typedef struct tQSTATS
{
  int n_crepl;			    /* number of "send"s by consultants */
  int n_cmail;			    /* number of "mail"s by consultants */
  int n_urepl;			    /* number of "send"s by user */
  int time_to_fr;		    /* time it took for first response by c. */
} QSTATS;


typedef struct tQUESTION
{
  struct tKNUCKLE *owner;
  char  logfile[NAME_SIZE];          /* Name of the logfile. */
  char	infofile[NAME_SIZE];	     /* Name of the file to store aux. info */
  int   seen[MAX_SEEN];              /* UIDs of users who have seen 
                                        this question */
  int   nseen;                       /* Number who have seen it. */
  char  topic[TOPIC_SIZE];           /* topic of this question. */
  int   topic_code;                  /* number version of the above */
  char  title[NAME_SIZE];            /* Title for log. */
  char  note[NOTE_SIZE];
  char  comment[COMMENT_SIZE];
  struct tQUESTION *next;	     /* For chaining */
  struct tQSTATS stats;	     /* For gathering statistics on question */
} QUESTION;

typedef struct tTOPIC
{
  char acl[NAME_SIZE];
  char name[NAME_SIZE];
  int  value;
  struct tTOPIC **subtopic;
} TOPIC;

typedef struct tOLC_PROC  
{
  int   proc_code;      /* Request code. */
  FUNCTION olc_proc;    /* Procedure to execute. */
  char  *description;   /* What it does. */
} PROC;

/* OLC status structure. */

typedef struct tQUEUE_STATUS	
{
  int   consultants;    /* Number of visible consultants. */
  int   invisible;      /* Number of invisible consultants; */
  int   busy;           /* Number of busy consultants. */
  int   waiting;        /* Number of waiting users. */
} QUEUE_STATUS;


typedef struct t_ACL
{
  int code;
  char *file;
  char *name;
} ACL;

typedef struct tTRANS
{
  char orig[80];
  char trans[80];
} TRANS;

/* Global variables */

extern KNUCKLE          **Knuckle_List;
extern TOPIC            **Topic_List;

#define KNUC_ALLOC_SZ	50
#define USER_ALLOC_SZ	50
#define QUES_ALLOC_SZ	50
extern KNUCKLE		*Knuckle_free,*Knuckle_inuse;
extern USER		*User_free, *User_inuse;
extern QUESTION		*Question_free, *Question_inuse;

extern int              needs_backup;
extern PROC  Proc_List[];	/* OLC Proceedure Table */
extern ACL  Acl_List[];
extern int request_count;
extern int request_counts[OLC_NUM_REQUESTS];
extern long start_time;
extern char DaemonInst[];

#endif
