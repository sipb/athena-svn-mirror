/*	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/rvdutil.c,v $
 *	$Author: lwvanels $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 */

#ifndef lint
static char rcsid_rvdutil_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/rvdutil.c,v 1.3 1991-07-09 15:10:09 lwvanels Exp $";
#endif lint

#include "attach.h"
#ifdef RVD
#include "rvdlib.h"
#include <setjmp.h>
#include <signal.h>

int vdcntrl = -1;
unsigned long rvderrno = 0;

/*
 * The RVD error list
 */

char *rvderlist[] = {
/* 0 */ "No error",
/* 1 */ "Non-existent drive",
/* 2 */ "Bad password for mode",
/* 3 */ "Already open in a different mode",
/* 4 */ "Invalid Checksum",
/* 5 */ "Index correction",
/* 6 */ "Non-existant disk-pack",
/* 7 */ "Drive already spun up",
/* 8 */ "Bad mode",
/* 9 */ "Unknown packet type",
/* 10 */ "Non Active Host",
/* 11 */ "Pack already spun up in EXCLUSIVE mode",
/* 12 */ "Zero blocks requested",
/* 13 */ "Too many blocks requested",
/* 14 */ "Pack not physically mounted",
/* 15 */ "Too many connections for server to support",
/* 16 */ "Too many connections for this host",
/* 17 */ "Server not currently active",
/* 18 */ "Identical pack already spun up in this drive, in the requested mode",
/* 19 */ "Physical device disused.",
/* 20 */ "Temporarily bad mode",
/* 21-30 */ "", "", "", "", "", "", "", "", "", "",
/* 31-40 */ "", "", "", "", "", "", "", "", "", "",
/* 41-51 */ "", "", "", "", "", "", "", "", "", "", "",
/* 52 */ "Timeout",
/* 53 */ "Invalid version",
NULL
};

static jmp_buf rvd_timeout;

static int rvd_timedout()
{
	longjmp(rvd_timeout,1);
}

/*
 * Open the RVD control device
 */

rvd_open()
{
    if (vdcntrl >= 0)
	return;
    vdcntrl = open(VDCONTROL, O_RDONLY, 0);
    if (vdcntrl < 0) {
	    fprintf(stderr, "Can't open RVD control device\n");
	    exit(ERR_FATAL);
    }
}

/*
 * Return an RVD error message
 */

char *rvd_error(n)
    unsigned int n;
{
    return (rvderlist[n]);
}

/*
 * Find a drive which already has the proper pack spunup on it, or the
 * first available drive.  If the pack had already been spunup, spin it
 * down and return that drive number.
 */

avail_drive(hostaddr, pack, drive)
    struct in_addr hostaddr;
    char *pack;
    int *drive;
{
    int i;
    struct vd_longstat lstats;
    struct vd_longstat *stats = &lstats;
    struct vd_long_dev *drives, *d;

    rvd_open();
    
    if (ioctl(vdcntrl, VDIOCGETSTAT, &stats) < 0) {
	    fprintf(stderr, "Can't perform VDIOCGETSTAT for RVD\n");
	    exit(ERR_FATAL);
    }

    if (!(drives = (struct vd_long_dev *)malloc(lstats.num_drives*
						sizeof(struct vd_long_dev)))) {
	    fprintf(stderr, "Can't malloc rvd information table.\n");
	    exit(ERR_FATAL);
    }
    if (ioctl(vdcntrl, VDIOCGETDRIVE, &drives) < 0) {
	    fprintf(stderr, "Can't perform VDIOCGETDRIVE for RVD\n");
	    exit(ERR_FATAL);
    }
    for (i=0, d=drives; i < lstats.num_drives; i++, d++) {
	if (!strncmp(d->args.name, pack, VD_NAME_LEN) &&
	    d->vd_device.server.s_addr == hostaddr.s_addr) {
	    if (debug_flag)
		printf("Found RVD spunup on drive %d!\n", i);
	    rvd_spindown(i);
	    *drive = i;
	    return (SUCCESS);
	}
    }
    for (i=0, d=drives; i < lstats.num_drives; i++, d++) {
	if (d->vd_device.state == OFF ||
	    d->vd_device.state == UNUSED) {
	    *drive = i;
	    return (SUCCESS);
	}
    }
    return (FAILURE);
}

/*
 * Spindown a drive
 */

rvd_spindown(drive)
    int drive;
{
    rvd_open();

    if (debug_flag)
	    printf("Spinning down drive %d\n", drive);
    
    if (setjmp(rvd_timeout)) {
	    if (debug_flag)
		    printf("rvd_spindown: timed out");
	    return(FAILURE);
    }
    signal(SIGALRM, rvd_timedout);
    alarm(RVD_ATTACH_TIMEOUT);

    /* XXX Error status? */
    if (ioctl(vdcntrl, VDIOCSPINDOWN, &drive)) {
	    if (debug_flag)
		    perror("ioctl: spindown:");
	    return(FAILURE);
    }
    
    alarm(0);
    return(SUCCESS);
}

/*
 * Spinup a drive
 */

rvd_spinup(server, pack, drive, mode, servername, pw)
    struct in_addr server;
    char *pack;
    int drive;
    char mode;
    char *servername;
    char *pw;
{
    struct vd_spinup vd_spinup;
    struct spinargs sargs;
    struct sockaddr_in sin;
    u_short rvdmode;
#ifdef KERBEROS
#ifdef OLD_KERBEROS
    extern char	*krb_getrealm();
#else
    extern char	*krb_realmofhost();
#endif
#endif

    if (debug_flag)
	printf("Spinning up pack %s from %s on drive %d\n",
	       pack, servername, drive);
    
    rvd_open();
    
    if (setjmp(rvd_timeout)) {
	    if (debug_flag)
		    printf("rvd_spinup: timed out");
	    alarm(0);
	    return(FAILURE);
    }
    signal(SIGALRM, rvd_timedout);
    alarm(RVD_ATTACH_TIMEOUT);

    errno = 0;

    strncpy(sargs.name, pack, RVD_NAM);
    if (pw)
	    strncpy(sargs.capab, pw, RVD_PW);
    else
	    sargs.capab[0] = '\0';

    if (mode == 'r')
	rvdmode = RVDMRO;
    else
	rvdmode = RVDMEXC;
    
    /*
     * First try a Kerberos authenticated spinup.  If 
     * it fails for any reason then try the regular spinup.
     */
#ifdef KERBEROS
    while (1) {
	int status;
	KTEXT_ST authent;
	struct vd_auth_spinup vd_auth_spinup;
	register char *dot;
	char *realm;

#ifdef OLD_KERBEROS
	realm = krb_getrealm(servername);
#else
	realm = krb_realmofhost(servername);
#endif	
	dot = index(servername, '.');
	if (dot)
	    *dot = '\0';
	status = krb_mk_req(&authent, "rvdsrv", servername, realm, NULL);
	if (dot)
	    *dot = '.';
	if (status != KSUCCESS)
	    break;
	vd_auth_spinup.drive = drive;
	vd_auth_spinup.uspin = &sargs;
	vd_auth_spinup.mode = rvdmode;
	sin.sin_family = AF_INET;
	sin.sin_addr = server;
	vd_auth_spinup.server = &sin;
	vd_auth_spinup.errp = &rvderrno;
	vd_auth_spinup.authent = &authent;

	rvderrno = 0;
	if (ioctl(vdcntrl, VDIOCAUTHSPINUP, &vd_auth_spinup) == 0) {
	    alarm(0);
	    return(SUCCESS);
    }
	if (debug_flag)
	    printf("Kerberos authenticated spinup failed\n");
	break;
    }
#endif /* KERBEROS */

    vd_spinup.drive = drive;
    vd_spinup.mode = rvdmode;
    sin.sin_family = AF_INET;
    sin.sin_addr = server;
    vd_spinup.server = &sin;
    vd_spinup.uspin = &sargs;
    vd_spinup.errp = &rvderrno;

    errno = 0;
    rvderrno = 0;
    if (ioctl(vdcntrl, VDIOCSPINUP, &vd_spinup)) {
	    alarm(0);
	    return (FAILURE);
    }
    alarm(0);
    if (errno || rvderrno)
	    return (FAILURE);
    return (SUCCESS);
}
#endif
