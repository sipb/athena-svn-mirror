/*
 * The code contained in this file is from Neal Nuckolls's (Sun Internet
 * Engineering) DLPI "test" kit.
 */

#ifndef lint
static char *RCSid = "$Id: dlpi.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * Common (shared) DLPI test routines.
 * Mostly pretty boring boilerplate sorta stuff.
 * These can be split into individual library routines later
 * but it's just convenient to keep them in a single file
 * while they're being developed.
 *
 * Not supported:
 *   Connection Oriented stuff
 *   QOS stuff
 */


#include "defs.h"

#if	defined(HAVE_DLPI)

#include	<sys/types.h>
#include	<sys/stream.h>
#include	<sys/stropts.h>
#include	<sys/dlpi.h>
#include	<sys/signal.h>
#include	<stdio.h>
#include	<string.h>
#include	"dlpi.h"

dlattachreq(fd, ppa)
int	fd;
u_long	ppa;
{
	dl_attach_req_t	attach_req;
	struct	strbuf	ctl;
	int	flags;

	attach_req.dl_primitive = DL_ATTACH_REQ;
	attach_req.dl_ppa = ppa;

	ctl.maxlen = 0;
	ctl.len = sizeof (attach_req);
	ctl.buf = (char *) &attach_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0)
		if (Debug) Error("dlattachreq:  putmsg");
}

dlphysaddrreq(fd, addrtype)
int	fd;
u_long	addrtype;
{
	dl_phys_addr_req_t	phys_addr_req;
	struct	strbuf	ctl;
	int	flags;

	phys_addr_req.dl_primitive = DL_PHYS_ADDR_REQ;
	phys_addr_req.dl_addr_type = addrtype;

	ctl.maxlen = 0;
	ctl.len = sizeof (phys_addr_req);
	ctl.buf = (char *) &phys_addr_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0)
		if (Debug) Error("dlphysaddrreq:  putmsg");
}

dldetachreq(fd)
int	fd;
{
	dl_detach_req_t	detach_req;
	struct	strbuf	ctl;
	int	flags;

	detach_req.dl_primitive = DL_DETACH_REQ;

	ctl.maxlen = 0;
	ctl.len = sizeof (detach_req);
	ctl.buf = (char *) &detach_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0)
		if (Debug) Error("dldetachreq:  putmsg");
}

dlbindreq(fd, sap, max_conind, service_mode, conn_mgmt, xidtest)
int	fd;
u_long	sap;
u_long	max_conind;
u_long	service_mode;
u_long	conn_mgmt;
u_long	xidtest;
{
	dl_bind_req_t	bind_req;
	struct	strbuf	ctl;
	int	flags;

	bind_req.dl_primitive = DL_BIND_REQ;
	bind_req.dl_sap = sap;
	bind_req.dl_max_conind = max_conind;
	bind_req.dl_service_mode = service_mode;
	bind_req.dl_conn_mgmt = conn_mgmt;
	bind_req.dl_xidtest_flg = xidtest;

	ctl.maxlen = 0;
	ctl.len = sizeof (bind_req);
	ctl.buf = (char *) &bind_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0)
		if (Debug) Error("dlbindreq:  putmsg");
}

dlunbindreq(fd)
int	fd;
{
	dl_unbind_req_t	unbind_req;
	struct	strbuf	ctl;
	int	flags;

	unbind_req.dl_primitive = DL_UNBIND_REQ;

	ctl.maxlen = 0;
	ctl.len = sizeof (unbind_req);
	ctl.buf = (char *) &unbind_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0)
		if (Debug) Error("dlunbindreq:  putmsg");
}

dlokack(fd, bufp)
int	fd;
char	*bufp;
{
	union	DL_primitives	*dlp;
	struct	strbuf	ctl;
	int	flags;

	ctl.maxlen = MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	strgetmsg(fd, &ctl, (struct strbuf*)NULL, &flags, "dlokack");

	dlp = (union DL_primitives *) ctl.buf;

	expecting(DL_OK_ACK, dlp);

	if (ctl.len < sizeof (dl_ok_ack_t))
		if (Debug) Error("dlokack:  response ctl.len too short:  %d", ctl.len);

	if (flags != RS_HIPRI)
		if (Debug) Error("dlokack:  DL_OK_ACK was not M_PCPROTO");

	if (ctl.len < sizeof (dl_ok_ack_t))
		if (Debug) Error("dlokack:  short response ctl.len:  %d", ctl.len);
}

dlbindack(fd, bufp)
int	fd;
char	*bufp;
{
	union	DL_primitives	*dlp;
	struct	strbuf	ctl;
	int	flags;

	ctl.maxlen = MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	strgetmsg(fd, &ctl, (struct strbuf*)NULL, &flags, "dlbindack");

	dlp = (union DL_primitives *) ctl.buf;

	expecting(DL_BIND_ACK, dlp);

	if (flags != RS_HIPRI)
		if (Debug) Error("dlbindack:  DL_OK_ACK was not M_PCPROTO");

	if (ctl.len < sizeof (dl_bind_ack_t))
		if (Debug) Error("dlbindack:  short response ctl.len:  %d", ctl.len);
}

dlphysaddrack(fd, bufp)
int	fd;
char	*bufp;
{
	union	DL_primitives	*dlp;
	struct	strbuf	ctl;
	int	flags;

	ctl.maxlen = MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	strgetmsg(fd, &ctl, (struct strbuf*)NULL, &flags, "dlphysaddrack");

	dlp = (union DL_primitives *) ctl.buf;

	expecting(DL_PHYS_ADDR_ACK, dlp);

	if (flags != RS_HIPRI)
		if (Debug) Error("dlbindack:  DL_OK_ACK was not M_PCPROTO");

	if (ctl.len < sizeof (dl_phys_addr_ack_t))
		if (Debug) Error("dlphysaddrack:  short response ctl.len:  %d", ctl.len);
}

static void
mysigalrm()
{
	Error("sigalrm:  TIMEOUT");
	exit(1);
}

strgetmsg(fd, ctlp, datap, flagsp, caller)
int	fd;
struct	strbuf	*ctlp, *datap;
int	*flagsp;
char	*caller;
{
	int	rc;
	static	char	errmsg[80];

	/*
	 * Start timer.
	 */
	(void) signal(SIGALRM, mysigalrm);
	if (alarm((unsigned) MAXWAIT) < (unsigned) 0) {
		(void) sprintf(errmsg, "%s:  alarm", caller);
		if (Debug) Error(errmsg);
	}

	/*
	 * Set flags argument and issue getmsg().
	 */
	*flagsp = 0;
	if ((rc = getmsg(fd, ctlp, datap, flagsp)) < 0) {
		(void) sprintf(errmsg, "%s:  getmsg", caller);
		if (Debug) Error(errmsg);
	}

	/*
	 * Stop timer.
	 */
	if (alarm((unsigned) 0) < (unsigned) 0) {
		(void) sprintf(errmsg, "%s:  alarm", caller);
		if (Debug) Error(errmsg);
	}

	/*
	 * Check for MOREDATA and/or MORECTL.
	 */
	if ((rc & (MORECTL | MOREDATA)) == (MORECTL | MOREDATA))
		if (Debug) Error("%s:  MORECTL|MOREDATA", caller);
	if (rc & MORECTL)
		if (Debug) Error("%s:  MORECTL", caller);
	if (rc & MOREDATA)
		if (Debug) Error("%s:  MOREDATA", caller);

	/*
	 * Check for at least sizeof (long) control data portion.
	 */
	if (ctlp->len < sizeof (long))
		if (Debug) Error("getmsg:  control portion length < sizeof (long):  %d", ctlp->len);
}

expecting(prim, dlp)
int	prim;
union	DL_primitives	*dlp;
{
	if (dlp->dl_primitive != (u_long)prim)
	    if (Debug) Error("DLPI: expected %d got %d", prim,
			     dlp->dl_primitive);
}

/*
 * Return string.
 */
addrtostring(addr, length, s)
u_char	*addr;
u_long	length;
u_char	*s;
{
	int	i;

	for (i = 0; i < length; i++) {
		(void) sprintf((char*) s, "%x:", addr[i] & 0xff);
		s = s + strlen((char*)s);
	}
	if (length)
		*(--s) = '\0';
}

#endif	/* HAVE_DLPI */
