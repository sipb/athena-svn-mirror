/* This code is heavily derived from Transarc's vos.c, which has the
 * following copyright. */

/*====================================================================
 * Copyright (C) 1990, 1989 Transarc Corporation - All rights reserved 
 *====================================================================*/
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/* voldump: Dump a volume by volume name/ID, server, and partition, without
 *	    using the VLDB.
 *
 * $Id: voldump.c,v 1.2 1996-06-20 15:25:27 ghudson Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/voldump/voldump.c,v $
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <afs/param.h>
#include <afs/stds.h>
#include <rx/rx.h>
#include <afs/cellconfig.h>
#include <ubik.h>
#include <afs/cmd.h>
#include <afs/volser.h>
#include <afs/volint.h>

extern int verbose;
extern int UV_SetSecurity();
extern struct rx_connection *UV_Bind();
extern struct ubik_client *cstruct;
extern jmp_buf env;
static int rxInitDone = 0;
static char confdir[] = AFSCONF_CLIENTNAME;

static int32 before(struct cmd_syndesc *as, char *arock);
static int32 dump_volume(struct cmd_syndesc *as);
static int32 get_server(const char *aname);
static int is_part_valid(int32 partId, int32 server, int32 *code);
static int32 dump(struct rx_call *call, char *rock);
static int receive_file(int fd, struct rx_call *call, struct stat *status);
static void print_diagnostics(const char *astring, int32 acode);
static int UV_DumpVolume(int32 afromvol, int32 afromserver, int32 afrompart,
			 int32 fromdate, int32 (*dumpfunc)(), char *rock);
static void dump_sig_handler(void);
static u_int32 get_volume_id(const char *name, int32 server, int32 part);

int main(int argc, char **argv)
{
    int32 code;
    struct cmd_syndesc *ts;

    cmd_SetBeforeProc(before, NULL);

    ts = cmd_CreateSyntax("initcmd", dump_volume, 0, "optional");
    cmd_AddParm(ts, "-id", CMD_SINGLE, 0, "volume name or ID");
    cmd_AddParm(ts, "-time", CMD_SINGLE, 0, "dump from time");
    cmd_AddParm(ts, "-server", CMD_SINGLE, 0, "machine name");
    cmd_AddParm(ts, "-partition", CMD_SINGLE, 0, "partition name");
    cmd_AddParm(ts, "-file", CMD_SINGLE, CMD_OPTIONAL, 0, "dump file");
    cmd_Seek(ts, 12);
    cmd_AddParm(ts, "-cell", CMD_SINGLE, CMD_OPTIONAL, "cell name");
    cmd_AddParm(ts, "-noauth", CMD_FLAG, CMD_OPTIONAL, "don't authenticate");
    cmd_AddParm(ts, "-localauth", CMD_FLAG, CMD_OPTIONAL,"use server tickets");
    cmd_AddParm(ts, "-verbose", CMD_FLAG, CMD_OPTIONAL, "verbose");

    code = cmd_Dispatch(argc, argv);
    if (rxInitDone)
	rx_Finalize();
    return (code) ? 1 : 0;
}

static int32 before(struct cmd_syndesc *as, char *arock)
{
    char *tcell;
    int32 code, sauth;

    sauth = 0;
    tcell = (char *) 0;
    if (as->parms[12].items)	/* if -cell specified */
	tcell = as->parms[12].items->data;
    if (as->parms[14].items)	/* -serverauth specified */
	sauth = 1;
    if (code = vsu_ClientInit((as->parms[13].items != 0), confdir, tcell,
			      sauth, &cstruct, UV_SetSecurity)) {
	fprintf(stderr, "could not initialize VLDB library (code=%u) \n",
		code);
	exit(1);
    }
    rxInitDone = 1;
    verbose = (as->parms[15].items != 0);
    return 0;
}

static int32 dump_volume(struct cmd_syndesc *as)
{    
    int32 avolid, aserver = 0, apart = -1, voltype, fromdate, code, err, i;
    char filename[512];

    rx_SetRxDeadTime(60 * 10);
    for (i = 0; i < MAXSERVERS; i++) {
	struct rx_connection *rxConn = ubik_GetRPCConn(cstruct, i);
	if (rxConn == 0)
	    break;
	rx_SetConnDeadTime(rxConn, rx_connDeadTime);
	if (rxConn->service)
	    rxConn->service->connDeadTime = rx_connDeadTime;
    }

    aserver = get_server(as->parms[2].items->data);
    if (aserver == -1) {
	fprintf(stderr, "vos: host '%s' not found in host table\n",
		as->parms[2].items->data );
	exit(1);
    }
    apart = volutil_GetPartitionID(as->parms[3].items->data);
    if (apart < 0) {
	fprintf(stderr, "vos: could not interpret partition name '%s'\n",
		as->parms[3].items->data );
	exit(1);
    }
    avolid = get_volume_id(as->parms[0].items->data, aserver, apart);
    if (avolid == 0) {
	fprintf(stderr, "vos: can't find volume '%s'\n",
		as->parms[0].items->data);
	return ENOENT;
    }
    if (!is_part_valid(apart, aserver, &code)){
	/*check for validity of the partition */
	if (code) {
	    PrintError("", code);
	} else {
	    fprintf(stderr, "vos: partition %s does not exist on the server\n",
		    as->parms[3].items->data);
	}
	exit(1);
    }

    fromdate = atol(as->parms[1].items->data);
    if (fromdate) {
	code = ktime_DateToInt32(as->parms[1].items->data, &fromdate);
	if (code) {
	    fprintf(stderr, "vos: failed to parse date '%s' (error=%d))\n",
		    as->parms[1].items->data, code);
	    return code;
	}
    }
    strcpy(filename, (as->parms[4].items) ? as->parms[4].items->data : "");
    code = UV_DumpVolume(avolid, aserver, apart, fromdate, dump, filename);
    if (code) {
	print_diagnostics("dump", code);
	return code;
    }
    if (*filename) {
	fprintf(stderr, "Dumped volume %s in file %s\n",
		as->parms[0].items->data, filename);
    } else {
	fprintf(stderr, "Dumped volume %s in stdout \n",
		as->parms[0].items->data);
    }
    return 0;
}

/* return host address in network byte order */
static int32 get_server(const char *aname)
{
    struct hostent *th;
    int32 addr, code;

    if (isdigit(*aname))
	return inet_addr(aname);
    th = gethostbyname(aname);
    if (!th)
	return -1;
    memcpy(&addr, th->h_addr, sizeof(addr));
    return addr;
}

static int is_part_valid(int32 partId, int32 server, int32 *code)
{   
    struct partList partlist;
    int32 cnt;
    int i, success = 0;

    *code = UV_ListPartitions(server, &partlist, &cnt);
    if (*code)
	return success;
    for (i = 0; i < cnt; i++) {
	if (partlist.partFlags[i] & PARTVALID)
	    if (partlist.partId[i] == partId)
		success = 1;
    }
    return success;
}

static int32 dump(struct rx_call *call, char *rock)
{
    char *filename;
    int fd;
    struct stat status;
    int32 error,code;

    error = 0;
    fd = -1;

    filename = rock;
    if (!strcmp(filename, "")){
	code = receive_file(1, call, &status);
	if (code)
	    error = code;
	goto dffail;
    }
    fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd < 0 || fstat(fd, &status) < 0){
	fprintf(stderr, "Could not create file '%s'\n", filename);
	error = VOLSERBADOP;
	goto dffail;
    }
    code = receive_file(fd, call, &status);
    if (code) {
	error = code;
	goto dffail;
    }

  dffail:
    if (*filename) {
	code = (fd >= 0) ? close(fd) : 0;
	if (code) {
	    fprintf(STDERR,"Could not close dump file %s\n",filename);
	    if (!error)
		error = code;
	}
    }
    return error; 
}   
    
/* Receive data from <call> stream into file associated with <fd> <status> */
static int receive_file(int fd, struct rx_call *call, struct stat *status)
{
    char *buffer = NULL;
    int32 bytesread, nbytes, bytesleft, w, out, error = 0;

    nbytes = (fd == 1) ? 4096 : status->st_blksize;
    buffer = (char *) malloc(nbytes);
    if (!buffer) {
	fprintf(stderr, "memory allocation failed\n");
	return -1;
    }
    bytesread = 1;
    out = 1 << fd;
    while (!error && (bytesread > 0)) {
	bytesread = rx_Read(call, buffer, nbytes);
	bytesleft = bytesread;
	while (!error && (bytesleft > 0)) {
	    /* Don't timeout if write blocks */
	    IOMGR_Select(32, 0, &out, 0, 0);
	    w = write(fd, &buffer[bytesread - bytesleft], bytesleft);
	    if (w < 0) {
	        fprintf(stderr, "File system write failed\n");
		error = -1;
	    } else {
	        bytesleft -= w;
	    }
	}
    }
    if (buffer)
	free(buffer);
    if (fd != 1) {
	if (!error)
	    fstat(fd, status);
    }
    return error;
}

static void print_diagnostics(const char *astring, int32 acode)
{
    if (acode == EACCES) {
	fprintf(stderr,
		"You are not authorized to perform the 'vos %s' command\n",
		astring, acode);
    } else {
	fprintf(stderr, "Error in vos %s command. ", astring);
	PrintError("", acode);
    }
}

/* This function is duplicated from vsprocs.c because the AFS version has a
 * gratuitous VLDB lookup at the beginning.  Maybe some day we can get rid of
 * it. */

/*dump the volume <afromvol> on <afromserver> and
* <afrompart> to <afilename> starting from <fromdate>.
* DumpFunction does the real work behind the scenes after
* extracting parameters from the rock  */
static int UV_DumpVolume(int32 afromvol, int32 afromserver, int32 afrompart,
			 int32 fromdate, int32 (*dumpfunc)(), char *rock)
{
    struct rx_connection  *fromconn;
    struct rx_call *fromcall;
    int32 fromtid;
    int32 vcode,tcode,rxError, terror;
    int32 rcode;

    register int32 code;
    int32 error;
    int islocked;

    islocked = 0;
    error = 0;
    rxError = 0;
    fromcall = (struct rx_call *)0;
    fromconn = (struct rx_connection *)0;
    fromtid = 0;
    fromcall = (struct rx_call *)0;
	
    if (setjmp(env)) {
	error = EPIPE;
	goto dufail;
    }
    signal(SIGPIPE, dump_sig_handler);
    signal(SIGINT, dump_sig_handler);

    /* get connections to the servers */
    fromconn = UV_Bind(afromserver, AFSCONF_VOLUMEPORT);
    code = AFSVolTransCreate(fromconn, afromvol, afrompart, ITBusy, &fromtid);
    if (code) {
	fprintf(stderr,
		"Could not start transaction on the volume %u to be dumped\n",
		afromvol);
	error = code;
	goto dufail;
    }
    if (verbose)
	fprintf(stderr, "Dumping ...");
    fromcall = rx_NewCall(fromconn);
    tcode = StartAFSVolDump(fromcall, fromtid, fromdate);
    if (tcode) {
	fprintf(stderr, "Could not start the dump process \n");
	error = tcode;
	goto dufail;
    }
    code = dumpfunc(fromcall, rock);
    if (code) {
	fprintf(stderr, "Error while dumping volume \n");
	error = code;
	goto dufail;
    }
    terror = rx_EndCall(fromcall, rxError);
    fromcall = NULL;
    if (terror) {
	fprintf(stderr, "Error in rx_EndCall \n");
	error = terror;
	goto dufail;
    }
    code = AFSVolEndTrans(fromconn, fromtid, &rcode);
    fromtid = 0;
    if (!code)
	code = rcode;
    if (code) {
	fprintf(stderr,
		"Could not end transaction on the volume %u being dumped\n",
		afromvol);
	error = code;
	goto dufail;
    }
    if (verbose)
	fprintf(stderr, "completed\n");
dufail:
    if(fromcall) {
	code = rx_EndCall(fromcall, rxError);
	if (code) {
	    fprintf(stderr, "Error in rx_EndCall\n");
	    if (!error)
		error = code;
	}	
    }
    if (fromtid) {
	code = AFSVolEndTrans(fromconn, fromtid, &rcode);
	if (!code)
	    code = rcode;
	if (code) {
	    fprintf(stderr,
		    "Could not end transaction on the volume %u\n", afromvol);
	    if (!error)
		error = code;
	}
    }

    if (fromconn)
	rx_DestroyConnection(fromconn);
    PrintError("",error);
    return error;
}

static void dump_sig_handler()
{
   fprintf(STDERR,"\nSignal handler: vos dump operation\n");
   longjmp(env,0);
}

/* The normal way to convert a volume name to ID is via a VLDB lookup, but we
 * don't want to use the VLDB, so instead list the volumes on the given server
 * and partition. */
static u_int32 get_volume_id(const char *name, int32 server, int32 part)
{
    int32 code, size, i;
    struct volintInfo *volumes;
    const char *p;

    /* Are we numeric? */
    for (p = name; *p; p++) {
	if (!isdigit(*p))
	    break;
    }
    if (!*p)
	return atol(name);

    code = UV_ListVolumes(server, part, 1, &volumes, &size);
    if (code)
	return 0;

    for (i = 0; i < size; i++) {
	if (strcmp(volumes[i].name, name) == 0) {
	    free(volumes);
	    return volumes[i].volid;
	}
    }

    free(volumes);
    return 0;
}
