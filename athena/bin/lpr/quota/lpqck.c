/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpqck.c,v $
 *	$Author: probe $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpqck.c,v 1.1 1993-10-12 05:27:53 probe Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */


#if (!defined(lint) && !defined(SABER))
static char lpqck_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/lpqck.c,v 1.1 1993-10-12 05:27:53 probe Exp $";
#endif (!defined(lint) && !defined(SABER))


/* Program to run quota database consistency checks and make repairs */

#include "mit-copyright.h"
#include "quota.h"
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>

char *progname;
char pbuf[BUFSIZ/2];
char *bp = pbuf;

#ifdef DEBUG
int quota_debug=1;
int gquota_debug=1;
int logger_debug=1;
#endif

/* These should be configured at startup - hardcode for now */
static char *stringFile = "/usr/spool/quota/string.db";
static char *userFile = "/usr/spool/quota/user.db";
static char *jourFile = "/usr/spool/quota/journal.db";
static char *uidFile  = "/usr/spool/quota/uid.db";

main(argc, argv)
int argc;
char **argv;
    {
    progname = argv[0];
    argv++;
    aclname[0] = saclname[0] = qfilename[0] = gfilename[0] = rfilename[0] 
	= quota_name[0] = '\0';
    (void) strcpy(aclname, DEFACLFILE);
    (void) strcpy(saclname, DEFSACLFILE);
    (void) strcpy(qfilename, DBM_DEF_FILE);
    (void) strcpy(gfilename, DBM_GDEF_FILE);
    (void) strcpy(qcapfilename, DEFCAPFILE);

    while(--argc) {
	if(argv[0][0] != '-')
	    usage(); /* Never returns */
	switch(argv[0][1]) {
	default:
	    usage(); /* Never returns */
	    break;
	case 'N':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) (void) strcpy(quota_name, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	case 'c':
	    if(!argv[0][2]) argc--, argv++;
	    if(argc) (void) strcpy(qcapfilename, argv[0]);
	    else usage();	/* Doesn't return */
	    break;
	}
    } /* while argc */

    if (read_quotacap()) {
	fprintf(stderr, "Unable to open/read quotacap file %s - FATAL\n",
		qcapfilename);
	    exit(1);
    }
    
    exit(0);
}

usage()
{
    fprintf(stderr, "%s: [-q quota_database_file] [-N quota_name]\n", progname);
    exit(1);
}
