/*
 *			  COPYRIGHT ) 1987 BY
 *	      DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
 *
 * This software is furnished under a license and may be used and  copied
 * only  in  accordance  with  the  terms  of  such  license and with the
 * inclusion of the above copyright notice.  This software or  any  other
 * copies  thereof may not be provided or otherwise made available to any
 * other person.  No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change  without  notice
 * and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT
 * CORPORATION.
 *
 * DIGITAL assumes no responsibility for the use or  reliability  of  its
 * software on equipment which is not supplied by DIGITAL.
 *
 *
 *
 * FACILITY:	
 *	NCS
 *
 * ABSTRACT:
 *	
 *
 * ENVIRONMENT: VMS V4.7 or higher.
 *
 * AUTHOR:  Hsin Lee LHH	CREATION DATE: 13-Nov-1987
 *
 * MODIFIED BY:
 *
 * Edit	Modifier	 Date
 *
 * 002  Nat Mishkin      16-May-1989
 *      Put ioctl back to the way it was.
 *
 * 001  Nat Mishkin      27-Apr-1989
 *      Eliminated bcmp (was commented out), fcntl_nck (not needed); renamed
 *      ioctl => ioctl_nck and made static (not called from outside).
 * 
 * 000	Hsin Lee LHH	 18-Mar-1989
 *	Wrote the original code.
 */

#include stdio
#include iodef
#include "std.h"

#undef select
#undef recvfrom
#undef sendto

/* #include <net/if.h> */
/* 
 * This really should be  #include <sys/ioctl.h>
 */
#include <nbase.h>

/* 
 * These are lacking from ucx$inetdef.h.
 */
#include <sys/ucx$inetdef.h>
#ifndef _IO
/*
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 * The IOC_VOID field of 0x20000000 is defined so that new ioctls
 * can be distinguished from old ioctls.
 */
#define IOCPARM_MASK	0x7f		/* Parameters are < 128 bytes	*/
#define IOC_VOID	(int)0x20000000	/* No parameters		*/
#define IOC_OUT 	(int)0x40000000	/* Copy out parameters		*/
#define IOC_IN		(int)0x80000000	/* Copy in parameters		*/
#define IOC_INOUT	(int)(IOC_IN|IOC_OUT)
#define _IO(x,y)	(int)(IOC_VOID|('x'<<8)|y)
#define _IOR(x,y,t)	(int)(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define _IOW(x,y,t)	(int)(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define _IOWR(x,y,t)	(int)(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#endif _IO

#define VMSOK(s) (s & 01)



int set_socket_non_blocking(sock)
int sock;
{
    int flag = true;
    ioctl(sock, FIONBIO, &flag);
    return (1);
}

int close_socket(chan)
int chan;
{
    return(close(chan));
}


int select_nck(nfds, readfds, writefds, exceptfds, timeout)
int nfds, *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    return(select(nfds, readfds, writefds, exceptfds, timeout));
}


int recvfrom_nck(s, buf, len, flags, from,fromlen)
int s;
char *buf;
int len, flags;
struct sockaddr *from;
int *fromlen;
{
    return(recvfrom(s, buf, len, flags, from,fromlen));
}


int sendto_nck(s, msg, len, flags, to, tolen)
int s;
char *msg;
int len, flags;
struct sockaddr *to;
int tolen;
{
    return(sendto(s, msg, len, flags, to, tolen));
}


/* 
 * ioctl: implements the unix ioctl call.
 * This is not intended to be a full ioctl implementation.  Only the
 * function related to inet are of interest.
 */
int ioctl(d, request, argp)
int d, request;
char *argp;
{
    int ef;			/* Event flag number */
    int sdc;			/* Socket device channel */
    unsigned short fun; 	/* Qiow function code  */
    unsigned short iosb[4];	/* Io status block */
    char *p5, *p6;			/* Args p5 & p6 of qiow */
    struct comm
    {
	int command;
	char *addr;
    } ioctl_comm;		/* Qiow ioctl commands. */
    struct it2 
    {
	unsigned short len;
	unsigned short opt;
	struct comm *addr;
    } ioctl_desc;		/* Qiow ioctl commands descriptor */
    int status;
    
    /* 
     * Gets an event flag for qio
     */
    status = LIB$GET_EF(&ef);
    if (!VMSOK(status))
    {
	/* No ef available. Use 0 */
	ef = 0;
    }

    /* 
     * Get the socket device channel number.
     */
    sdc = vaxc$get_sdc(d);
    if (sdc == 0)
    {
	/* Not an open socket descriptor. */
	errno = EBADF;
	return -1;
    }
    
    /* 
     * Fill in ioctl descriptor.
     */
    ioctl_desc.opt = UCX$C_IOCTL;
    ioctl_desc.len = sizeof(struct comm);
    ioctl_desc.addr = &ioctl_comm;
    
    /* 
     * Decide qio function code and in/out parameter.
     */
    if (request & IOC_OUT)
    {
	fun = IO$_SENSEMODE;
	p5 = 0;
	(struct it2 *)p6 = &ioctl_desc;
    }
    else
    {
	fun = IO$_SETMODE;
	(struct it2 *)p5 = &ioctl_desc;
	p6 = 0;
    }

    /* 
     * Fill in ioctl command.
     */
    ioctl_comm.command = request;
    ioctl_comm.addr = argp;
    
    /* 
     * Do ioctl.
     */
    status = SYS$QIOW(ef, sdc, fun, iosb, 0, 0,
		      0, 0, 0, 0,		/* p1 - p4: not used*/
		      p5, p6);
    
    if (!VMSOK(status))
    {
#ifdef DEBUG
	printf("ioctl failed: status = %d\n", status);
#endif DEBUG
	errno = status;
	return -1;
    }
    
    if (!VMSOK(iosb[0]))
    {
#ifdef DEBUG
	printf("ioctl failed: status = %x, %x, %x%x\n", iosb[0], iosb[1],
	       iosb[3], iosb[2]);
#endif DEBUG
	errno = iosb[0];
	return -1;
    }
    
    status = LIB$FREE_EF(&ef);
    return 0;
}
