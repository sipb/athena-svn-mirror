#if !defined(lint) && !defined(SABER)
static char rcsid[] = "$Id: ns_ixfr.c,v 1.1.1.1 1999-03-16 19:45:07 danw Exp $";
#endif /* not lint */

/*
 * Portions Copyright (c) 1999 by Check Point Software Technologies, Inc.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Check Point Software Technologies Incorporated not be used 
 * in advertising or publicity pertaining to distribution of the document 
 * or software without specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND CHECK POINT SOFTWARE TECHNOLOGIES 
 * INCORPORATED DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.   
 * IN NO EVENT SHALL CHECK POINT SOFTWARE TECHNOLOGIES INCORPRATED
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT 
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "port_before.h"

#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>

#include <isc/eventlib.h>
#include <isc/logging.h>
#include <isc/memcluster.h>

#include "port_after.h"

#include "named.h"

static void	sx_new_ixfrmsg(struct qstream * qsp),
		sx_send_ixfr(struct qstream * qsp);

static int	sx_flush(struct qstream * qsp),
		sx_addrr(struct qstream * qsp,
			 const char *dname,
			 struct databuf * dp);
extern void	sx_sendsoa(struct qstream * qsp);

/*
 * void ns_ixfr(qsp, znp, zone, class, type, opcode, id, serial_ixfr)
 * Initiate a concurrent (event driven) outgoing zone transfer.
 */
void
ns_ixfr(struct qstream * qsp, struct namebuf * znp,
	int zone, int class, int type, int opcode, int id, 
	u_int32_t serial_ixfr, struct tsig_record *in_tsig)
{
	FILE *		rfp;
	int		fdstat;
	pid_t		pid;
	server_info	si;
#ifdef SO_SNDBUF
	static const int sndbuf = XFER_BUFSIZE * 2;
#endif
#ifdef SO_SNDLOWAT
	static const int sndlowat = XFER_BUFSIZE;
#endif

	ns_info(ns_log_xfer_out,
		"incremental zone transfer of \"%s\" (%s) to %s",
		zones[zone].z_origin, p_class(class),
		sin_ntoa(qsp->s_from));

#ifdef SO_SNDBUF
	/*
	 * The default seems to be 4K, and we'd like it to have enough room
	 * to parallelize sending the pushed data with accumulating more
	 * write() data from us.
	 */
	(void) setsockopt(qsp->s_rfd, SOL_SOCKET, SO_SNDBUF,
			  (char *) &sndbuf, sizeof sndbuf);
#endif
#ifdef SO_SNDLOWAT
	/*
	 * We don't want select() to show writability 'til we can write an
	 * XFER_BUFSIZE block of data.
	 */
	(void) setsockopt(qsp->s_rfd, SOL_SOCKET, SO_SNDLOWAT,
			  (char *) &sndlowat, sizeof sndlowat);
#endif
	if (sq_openw(qsp, 64 * 1024) == -1)
		goto abort;
	memset(&qsp->xfr, 0, sizeof qsp->xfr);
	qsp->xfr.msg = (u_char *) memget(XFER_BUFSIZE);
	if (!qsp->xfr.msg) {
abort:
		(void) shutdown(qsp->s_rfd, 2);
		sq_remove(qsp);
		return;
	}
	qsp->xfr.eom = qsp->xfr.msg + XFER_BUFSIZE;
	qsp->xfr.state = s_x_ixfr;
	qsp->xfr.serial = serial_ixfr;
	qsp->xfr.zone = zone;
	qsp->xfr.class = class;
	qsp->xfr.id = id;
	qsp->xfr.opcode = opcode;
	qsp->xfr.cp = NULL;
	zones[zone].z_numxfrs++;
	qsp->flags |= STREAM_AXFR;
	qsp->xfr.top = (struct namebuf *) ixfr_get_change_list(&zones[zone], serial_ixfr, zones[zone].z_serial);
	if (!qsp->xfr.top)
		goto abort;

	si = find_server(qsp->s_from.sin_addr);
	if (si != NULL && si->transfer_format != axfr_use_default)
		qsp->xfr.transfer_format = si->transfer_format;
	else
		qsp->xfr.transfer_format = server_options->transfer_format;

	if (in_tsig == NULL)
		qsp->xfr.tsig_state = NULL;
	else {
		qsp->xfr.tsig_state = memget(sizeof(ns_tcp_tsig_state));
		ns_sign_tcp_init(in_tsig->key, in_tsig->sig, in_tsig->siglen,
				 qsp->xfr.tsig_state);
	}

	(void) sq_writeh(qsp, sx_send_ixfr);
}

/*
 * u_char * sx_new_ixfrmsg(msg) init the header of a message, reset the
 * compression pointers, and reset the write pointer to the first byte
 * following the header.
 */
static void
sx_new_ixfrmsg(struct qstream *qsp) {
	HEADER *	hp = (HEADER *) qsp->xfr.msg;
	ns_updrec *	up;

	memset(hp, 0, HFIXEDSZ);
	hp->id = htons(qsp->xfr.id);
	hp->opcode = qsp->xfr.opcode;
	hp->qr = 1;
	hp->rcode = NOERROR;

	qsp->xfr.ptrs[0] = qsp->xfr.msg;
	qsp->xfr.ptrs[1] = NULL;

	qsp->xfr.cp = qsp->xfr.msg + HFIXEDSZ;
	if (qsp->xfr.ixfr_zone == 0) {
		int		count, n;
		int		buflen;
		struct namebuf *np;
		struct hashbuf *htp;
		struct zoneinfo *zp;
		struct databuf *dp;
		const char *	fname;
		u_char **	edp = qsp->xfr.ptrs +
				      sizeof qsp->xfr.ptrs / sizeof(u_char *);

		qsp->xfr.ixfr_zone = qsp->xfr.zone;
		zp = &zones[qsp->xfr.zone];
		up = (ns_updrec *) qsp->xfr.top;
		n = dn_comp(zp->z_origin, qsp->xfr.cp,
		   XFER_BUFSIZE - (qsp->xfr.cp - qsp->xfr.msg), NULL, NULL);
		qsp->xfr.cp += n;
		PUTSHORT((u_int16_t) T_IXFR, qsp->xfr.cp);
		PUTSHORT((u_int16_t) zp->z_class, qsp->xfr.cp);
		hp->qdcount = htons(ntohs(hp->qdcount) + 1);
		count = qsp->xfr.cp - qsp->xfr.msg;
		htp = hashtab;
		np = nlookup(zp->z_origin, &htp, &fname, 0);
		buflen = XFER_BUFSIZE;
		foreach_rr(dp, np, T_SOA, qsp->xfr.class, qsp->xfr.zone) {
			n = make_rr(zp->z_origin, dp, qsp->xfr.cp, qsp->xfr.eom - qsp->xfr.cp, 0, qsp->xfr.ptrs, edp, 0);
			qsp->xfr.cp += n;
			hp->ancount = htons(ntohs(hp->ancount) + 1);
		}
	}
}

/*
 * int sx_flush(qsp) flush the intermediate buffer out to the stream IO
 * system. return: passed through from sq_write().
 */
static int
sx_flush(struct qstream *qsp) {
	int ret;

#ifdef DEBUG
	if (debug >= 10)
		fp_nquery(qsp->xfr.msg, qsp->xfr.cp - qsp->xfr.msg,
			  log_get_stream(packet_channel));
#endif
	ret = sq_write(qsp, qsp->xfr.msg, qsp->xfr.cp - qsp->xfr.msg);
	if (ret >= 0)
		qsp->xfr.cp = NULL;
	return (ret);
}

/*
 * int sx_addrr(qsp, name, dp) add name/dp's RR to the current assembly
 * message.  if it won't fit, write current message out, renew the message,
 * and then RR should fit. return: -1 = the sq_write() failed so we could not
 * queue the full message. 0 = one way or another, everything is fine. side
 * effects: on success, the ANCOUNT is incremented and the pointers are
 * advanced.
 */
static int
sx_addrr(struct qstream *qsp, const char *dname, struct databuf *dp) {
	HEADER *hp = (HEADER *) qsp->xfr.msg;
	u_char **edp = qsp->xfr.ptrs + sizeof qsp->xfr.ptrs / sizeof(u_char *);
	int n;

	if (qsp->xfr.cp != NULL) {
		if (qsp->xfr.transfer_format == axfr_one_answer &&
		    sx_flush(qsp) < 0)
			return (-1);
	}
	if (qsp->xfr.cp == NULL)
		sx_new_ixfrmsg(qsp);
	n = make_rr(dname, dp, qsp->xfr.cp, qsp->xfr.eom - qsp->xfr.cp,
		    0, qsp->xfr.ptrs, edp, 0);
	if (n < 0) {
		if (sx_flush(qsp) < 0)
			return (-1);
		if (qsp->xfr.cp == NULL)
			sx_new_ixfrmsg(qsp);
		n = make_rr(dname, dp, qsp->xfr.cp, qsp->xfr.eom - qsp->xfr.cp,
			    0, qsp->xfr.ptrs, edp, 0);
		INSIST(n >= 0);
	}
	hp->ancount = htons(ntohs(hp->ancount) + 1);
	qsp->xfr.cp += n;
	return (0);
}

static void
sx_send_ixfr(struct qstream *qsp) {
	char *		dname;
	char *		soa_dname;
	char *		cp;
	char *		acp;
	u_int16_t	class;
	u_int16_t	type;
	u_int32_t	ttl;
	u_int32_t	serial = 0;
	struct zoneinfo *zp = NULL;
	ns_updrec *	np;
	struct qstream *qtp = qsp;
	struct databuf *soa_dp;
	struct databuf *rdp;
	struct databuf *old_soadp;
	HEADER *	hp = (HEADER *) qsp->xfr.msg;
	int		buflen = XFER_BUFSIZE;

	if (qsp->xfr.top) {
		int		count, n;
		struct hashbuf *htp;
		struct zoneinfo *zp;
		struct namebuf *nbp;
		struct databuf *dp;
		const char *	fname;

		count = qsp->xfr.cp - qsp->xfr.msg;

		np = (ns_updrec *) qsp->xfr.top;
		zp = &zones[qsp->xfr.zone];
		dp = (struct databuf *) findzonesoa(zp);

		if (dp) {
			int len;

			soa_dp = dp;
			if (dp->d_type == T_SOA)
				soa_dname = zp->z_origin;
			else
				soa_dname = np->r_dname;
			old_soadp = memget(DATASIZE(dp->d_size));
			memcpy(old_soadp, dp, DATASIZE(dp->d_size));

			acp = (char *) old_soadp->d_data;
			memcpy(acp, dp->d_data, old_soadp->d_size);
		}
	}
	hp->ancount = htons(ntohs(0));

	cp = (char *)findsoaserial(old_soadp->d_data);
	PUTLONG(np->r_zone, cp);

	if (sx_addrr(qsp, soa_dname, old_soadp) < 0)
		return;

	while (qsp->xfr.top) {
		u_int section;

		dname = np->r_dname;
		class = np->r_class;
		type = np->r_type;
		ttl = np->r_ttl;
		rdp = np->r_dp;
		section = np->r_section;

		if ((np->r_opcode == DELETE) &&
		    (qsp->xfr.serial <= np->r_zone))
		{
			if (sx_addrr(qsp, dname, rdp) < 0)
				return;
			serial = np->r_zone;
		}
		np = (ns_updrec *) np->r_next;
		if (np == NULL)
			break;
	}

	if (sx_addrr(qsp, soa_dname, soa_dp) < 0)
		return;

	np = (ns_updrec *) qsp->xfr.top;

	while (qsp->xfr.top) {
		u_int section;

		dname = np->r_dname;
		rdp = np->r_dp;

		if ((np->r_opcode == ADD) && (qsp->xfr.serial <= np->r_zone)) {
			cp = (char *)findsoaserial(old_soadp->d_data);
			PUTLONG(np->r_zone, cp);
			if (sx_addrr(qsp, dname, rdp) < 0)
				return;
		}
		np = (ns_updrec *) np->r_next;
		if (np == NULL)
			break;
	}

	if (sx_addrr(qsp, soa_dname, soa_dp) < 0)
		return;

	if (qsp->xfr.ixfr_zone != 0) {
		int		count, n;
		struct hashbuf *htp;
		struct zoneinfo *zp;
		struct databuf *dp;
		struct namebuf *nbp;
		const char *	fname;

		zp = &zones[qsp->xfr.zone];
		count = qsp->xfr.cp - qsp->xfr.msg;
		htp = hashtab;
		nbp = nlookup(zp->z_origin, &htp, &fname, 0);
		n = finddata(nbp, class, T_SOA, hp, &zp->z_origin, &buflen, &count);
		dp = nbp->n_data;
		while (dp) {
			if (dp->d_type == T_SOA)
				break;
			dp = dp->d_next;
		}
		if (dp) {
			sx_addrr(qsp, zp->z_origin, dp);
		}
	}
	sx_flush(qsp);
	sq_writeh(qsp, sq_flushw);
	memput(old_soadp, DATASIZE(old_soadp->d_size));
}
