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
 *	$Id: polld.h,v 1.7 1999-06-28 22:52:28 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __polld_h
#define __polld_h __FILE__

#include <syslog.h>

#include <olc/olc.h>
#include <common.h>
#include <olc/procs.h>
#include <server_defines.h>

#ifdef HAVE_ZEPHYR
#include <zephyr/zephyr.h>
#endif

/* POLLD data structures */

typedef struct tPTF {
  char username[LOGIN_SIZE];
  char machine[HOSTNAME_SIZE];
  int status;
} PTF;

/* POLLD constants */

#define LOC_NO_CHANGE	0
#define LOC_CHANGED	1
#define LOC_ERROR	-1

#define FINGER_TIMEOUT	60  /* seconds */
#define CYCLE_TIME	10  /* minutes */

/* POLLD functions */

/* comm.c */
void tell_main_daemon (PTF user );

/* get_list.c */
int get_user_list (PTF *users , int *max_people );

/* hosthash.c */
void init_cache (void );
struct hostent *c_gethostbyname (char *name );

/* locate.c */
int locate_person (PTF *person );
void check_zephyr (void);

/* polld.c */
int main (int argc , char *argv []);

#endif /* __polld_h */
