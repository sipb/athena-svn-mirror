/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains definitions for the OLC polling daemon.
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/polld.h,v $
 *	$Id: polld.h,v 1.4 1991-11-05 13:58:36 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __polld_h
#define __polld_h __FILE__

#include <syslog.h>

#include <olc/olc.h>
#include <common.h>
#include <olc/procs.h>
#include <server_defines.h>

#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif

/* POLLD data structures */

typedef struct tPTF {
  char username[LOGIN_SIZE];
  char machine[NAME_SIZE];
  int status;
} PTF;

/* POLLD constants */

#define LOC_NO_CHANGE	0
#define LOC_CHANGED	1
#define LOC_ERROR	-1

#define FINGER_TIMEOUT	60  /* seconds */
#define CYCLE_TIME	10  /* minutes */

/* POLLD functions */

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif


/* comm.c */
void tell_main_daemon P((PTF user ));

/* get_list.c */
int get_user_list P((PTF *users , int *max_people ));

/* hosthash.c */
void init_cache P((void ));
struct hostent *c_gethostbyname P((char *name ));

/* locate.c */
int locate_person P((PTF *person ));
void check_zephyr P((void));

/* polld.c */
int main P((int argc , char *argv []));

#undef P
#endif /* __polld_h */
