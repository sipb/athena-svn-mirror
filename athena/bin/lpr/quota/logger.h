/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/logger.h,v 1.3 1990-07-10 20:38:27 epeisach Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/logger.h,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "config.h"

/*
 * Some useful defenitions for the Quota Server code
 */

/*
 * Quota Server global variables
 */

#include <sys/types.h>
#ifdef KERBEROS
#include <krb.h>
#endif


extern char *logString;
extern char *savestr();

#define LOGGER_VERSION	2	/* Version number of data format in files */


typedef u_long 	Time;		/* Time in seconds since Jan 1, 1970 */
typedef u_short String_num; 	/* Compressed ref number */
typedef u_long  Pointer;	/* Pointer to dbase record number */
typedef u_char	Function;	/* Function code i.e. SUBTRACT, ADD, CHARGE */
typedef u_long  Amount;		/* Amount in pmu's */
typedef u_short Num;		/* Number */

typedef struct {
	String_num	name;		/* Kerberos Username */
	String_num	instance;	/* Kerberos Instamce */
	String_num	realm;		/* Kerberos realm */
} User_str;

/* These are used for the user database */

typedef struct User_db {
	User_str	user;	/* User for db entry */
	Pointer		first;	/* First entry in log file (0 if none) */
	Pointer		last;	/* Last entry in log file (0 if none) */
} User_db;

/* The following is for the Journal Database */

typedef struct  {
  	u_short		version;	/* LOGGER_VERSION - format of file */
	Pointer		num_ent;	/* Number of entries in DB. */
	Time		last_q_time;	/* T of last entry added. Timestamp
					   entry from Quota Log */
	Pointer		quota_pos;		/* Position in Quota Log */
} log_header;


typedef struct {
	Time		time;		/* Quota server time stamp */
	User_str	user;		/* User who is referred to (can be __group# as well) */
	String_num	service;	/* Printer service type */
	Pointer		next;		/* Pointer in db to user next record*/
	Pointer		prev;	/* 0 indicates none */
	Function	func;		/* Indicates what is being logged */
	union {
		struct {
			Amount amt;	/* Amount added/subtracted */
			String_num name;	/* Who did change */
			String_num inst;	/* keberos inst of changer */
			String_num realm;	/* Realm of changer */
			} offset; 	/* for ADD, SUBTRACT, SET */
		struct {
			Time		subtime;	/* Job submission time*/
			Num		npages; 	/* Number pages */
			Num		med_cost;	/* media cost/page */
			String_num	where;		/* Server name */
			String_num      name;	        /* Who charged */
			String_num      inst;	        /* keberos inst of charger */
			String_num      realm;	        /* Realm of charger */
		        } charge;		        /* For charge */
		struct {
			String_num aname;	/* Who did change */
			String_num ainst;	/* keberos inst of changer */
			String_num arealm;	/* Realm of changer */
			String_num uname;       /* Which user */
			String_num uinst;       /* keberos inst of user */
			String_num urealm;	/* Realm of changer */
		        } group;                /* for {ADD,DELETE}_{USER,ADMIN} */
	} trans;
	char 		extra[7];	/* Reserved for future work */
} log_entity;




/* The following was taken from the Kerberos Distribution for dealing
   with NDBM/DBM incompatibilities 
*/

#if defined(ultrix) && defined(NULL)
/* The idiots redefine NULL in dbm.h */
#undef NULL
#endif

#ifdef NDBM
#include <ndbm.h>
#else /*NDBM*/
#include <dbm.h>
#endif /*NDBM*/

/* Macros to convert ndbm names to dbm names.
 * Note that dbm_nextkey() cannot be simply converted using a macro, since
 * it is invoked giving the database, and nextkey() needs the previous key.
 *
 * Instead, all routines call "dbm_next" instead.
 */

#ifndef NDBM
typedef char DBM;

#define dbm_open(file, flags, mode) ((dbminit(file) == 0)?"":((char *)0))
#define dbm_fetch(db, key) fetch(key)
#define dbm_store(db, key, content, flag) store(key, content)
#define dbm_firstkey(db) firstkey()
#define dbm_next(db,key) nextkey(key)
#define dbm_close(db) dbmclose()
#else
#define dbm_next(db,key) dbm_nextkey(db)
#endif


int logger_string_set_name(), logger_read_strings();
String_num logger_add_string(), logger_string_to_num();
char *logger_num_to_string();
	
int logger_user_set_name(), logger_set_user();
User_db *logger_find_user();

int logger_journal_set_name(), logger_journal_write_line(), 
	logger_journal_get_header(), logger_journal_put_header(),
	logger_journal_add_entry();
log_entity *logger_journal_get_line();

int logger_parse_quota(), logger_cvt_line();

#ifdef DEBUG
extern int logger_debug;
#endif



