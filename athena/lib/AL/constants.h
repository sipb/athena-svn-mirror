/**********************************************************************
 *  constants.h -- header file for athena login library
 *
 * Copyright 1994 by the Massachusetts Institute of Technology
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 **********************************************************************/
#include <mit-copyright.h>

#ifdef SOLARIS
#define NMAX	sizeof(utmpx.ut_name)
#define HMAX	sizeof(utmpx.ut_host)
#else
#define NMAX	sizeof(utmp.ut_name)
#define HMAX	sizeof(utmp.ut_host)
#endif

#ifndef MAXBSIZE
#define MAXBSIZE 1024
#endif

#ifdef VFS
#define QUOTAWARN	"quota"	/* warn user about quotas */
#endif VFS

#define KRB_ENVIRON	"KRBTKFILE" /* Ticket file environment variable */
#define KRB_TK_DIR	"/tmp/tkt_" /* Where to put the ticket */
#define KRBTKLIFETIME	DEFAULT_TKT_LIFE

#define PROTOTYPE_DIR	"/usr/athena/lib/prototype_tmpuser" /* Source for temp files */
#define TEMP_DIR_PERM	0755	/* Permission on temporary directories */

#define MAXPWSIZE   	128	/* Biggest key getlongpass will return */

#define START_UID	200	/* start assigning arbitrary UID's here */
#define MIT_GID		101	/* standard primary group "mit" */

extern char *krb_err_txt[];	/* From libkrb */

extern char	nolog[];
extern char	qlog[];
extern char	maildir[;
extern char	lastlog[];
extern char	inhibit[];
extern char	noattach[];
extern char	noremote[];
extern char	go_register[];
extern char	get_motd[];
extern char	rmrf[];
