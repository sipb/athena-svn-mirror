/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1997, Patrick Powell, San Diego, CA
 *     papowell@sdsu.edu
 * See LICENSE for conditions of use.
 *
 ***************************************************************************
 * MODULE: sortorder.h
 * PURPOSE: read and write the spool queue control file
 * sortorder.h,v 3.6 1997/12/16 15:06:42 papowell Exp
 **************************************************************************/

struct cmp_struct {
	int error/*r*/;
	int redirect;
	int flags;
	time_t remove_time;
	time_t done_time;
	int held_class;
	time_t hold_time;
	time_t priority_time;
	int priority/*r*/;
	time_t ctime;
	int number;
	char end;
};

char *make_cmp_str( struct control_file *lcf );
