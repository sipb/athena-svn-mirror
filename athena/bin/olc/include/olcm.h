/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains the defintions used in the program to receive questions via
 * mail
 *
 *      Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olcm.h,v $
 *      $Id: olcm.h,v 1.2 1991-11-06 15:43:34 lwvanels Exp $
 *      $Author: lwvanels $
 */

#include <mit-copyright.h>

#define DFLT_TOPIC	"test"
#define DFLT_SERVER	"fionavar.mit.edu"
#define DFLT_USERNAME	"nobody"
#define OLCR_PATH	"/usr/local/olcr"
#define SYSLOG_FACILITY LOG_LOCAL6

#ifdef KERBEROS
#ifdef ATHENA
#define DFLT_REALM	"ATHENA.MIT.EDU"
#else
/* Insert your realm here */
#define DFLT_REALM	"FOO.BAR.EDU"
#endif
#endif
