#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/tcp_grp.c,v 1.3 1997-02-27 06:47:55 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  1990/05/26 13:41:29  tom
 * athena release 7.0e
 *
 * Revision 1.1  90/04/26  18:15:13  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:43:06  snmpdev
 * Initial revision
 * 
 */

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */

/*
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/tcp_grp.c,v 1.3 1997-02-27 06:47:55 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the TCP group.
 */

#include "include.h"

int
find_currestabs()
{
	struct inpcb inpcb, cb, *next;
	struct tcpcb tcpb;
	int cnt = 0;

	if (nl[N_TCB].n_value == 0) {
		syslog(LOG_ERR, "_tcb not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_TCB].n_value, L_SET);
	if (read(kmem, (char *)&cb, sizeof(cb)) != sizeof(cb)) {
		syslog(LOG_ERR, "find_currestabs: read of cb: %m");
		return(BUILD_ERR);
	}

	inpcb = cb;

	while (inpcb.inp_next != (struct inpcb *)nl[N_TCB].n_value) {
		next = inpcb.inp_next;

		(void)lseek(kmem, (long)next, L_SET);
		if (read(kmem,(char *)&inpcb, sizeof(inpcb)) != sizeof(inpcb)) {
			syslog(LOG_ERR, "find_currestabs: read of inpcb: %m");
			return(BUILD_ERR);
		}

		(void)lseek(kmem, (long)inpcb.inp_ppcb, L_SET);
		if (read(kmem, (char *)&tcpb, sizeof(tcpb)) != sizeof(tcpb)) {
			syslog(LOG_ERR, "find_currestabs: read of tcpb: %m");
			return(BUILD_ERR);
		}
		
		if (tcpb.t_state == TCPS_ESTABLISHED)
			cnt++;
	}

	if (cnt == 0)
		return(BUILD_ERR);

	return(cnt);
}

int
find_tcpconn(connoid, inpcbv, tsock, tcbv, flgs)
	objident *connoid;
	struct inpcb *inpcbv;
	struct socket *tsock;
	struct tcpcb *tcbv;
	int flgs;
{
	struct inpcb inpcb, cb, *next;
	struct tcpcb tcpb;
	struct socket tcpsock;
	objident curroid, tmpoid;
	int foundone = 0, cmpval, firstone = 1, cnt;
	char *ch;

	if (nl[N_TCB].n_value == 0) {
		syslog(LOG_ERR, "_tcb not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_TCB].n_value, L_SET);
	if (read(kmem, (char *)&cb, sizeof(cb)) != sizeof(cb)) {
		syslog(LOG_ERR, "find_tcpconn: read: %m");
		return(BUILD_ERR);
	}

	inpcb = cb;

	while (inpcb.inp_next != (struct inpcb *)nl[N_TCB].n_value) {
		next = inpcb.inp_next;

		(void)lseek(kmem, (long)next, L_SET);
		if (read(kmem,(char *)&inpcb, sizeof(inpcb)) != sizeof(inpcb)) {
			syslog(LOG_ERR, "find_tcpconn: read of inpcb: %m");
			return(BUILD_ERR);
		}

		(void)lseek(kmem, (long)inpcb.inp_socket, L_SET);
		if (read(kmem, (char *)&tcpsock, sizeof(tcpsock)) != sizeof(tcpsock)) {
			syslog(LOG_ERR, "find_tcpconn: read of tcpsock: %m");
			return(BUILD_ERR);
		}
		
		(void)lseek(kmem, (long)inpcb.inp_ppcb, L_SET);
		if (read(kmem, (char *)&tcpb, sizeof(tcpb)) != sizeof(tcpb)) {
			syslog(LOG_ERR, "find_tcpconn: read of tcpb: %m");
			return(BUILD_ERR);
		}
		
		/*
		 *  Build up an object ID consisting of the following:
		 *  <loc-addr><loc-port><rem-addr><rem-port>
		 *
		 *  The idea on this is to compare this Obj-ID to the
		 *  requested one.  This makes Lexi-next in this
		 *  environment (with four components) much easier.
		 */
		/*
		 *  Do the local address.
		 */
		cnt = 0;
		ch = (char *)&inpcb.inp_laddr.s_addr;
		while (cnt < sizeof(inpcb.inp_laddr.s_addr)) {
			tmpoid.cmp[cnt] = *ch & 0xff;
			cnt++;
			ch++;
		}
		tmpoid.ncmp = cnt;

		/*
		 *  Do the local port.
		 */
		tmpoid.cmp[tmpoid.ncmp] = inpcb.inp_lport;
		tmpoid.ncmp++;
		
		/*
		 *  Do the Foreign (remote) address.
		 */
		cnt = 0;
		ch = (char *)&inpcb.inp_faddr.s_addr;
		while (cnt < sizeof(inpcb.inp_faddr.s_addr)) {
			tmpoid.cmp[tmpoid.ncmp] = *ch & 0xff;
			tmpoid.ncmp++;
			cnt++;
			ch++;
		}

		/*
		 *  Do the Foreign (remote) port.
		 */
		tmpoid.cmp[tmpoid.ncmp] = inpcb.inp_fport;
		tmpoid.ncmp++;
		
		/*
		 *  Now compare the object-ID's.  Do the usual
		 *  to find the best next, if needed.
		 */
		cmpval = oidcmp(connoid, &tmpoid);
		if ((cmpval == 0) && (flgs & (REQ|GET_LEX_NEXT))) {
			memcpy(tsock, &tcpsock, sizeof(tcpsock));
			memcpy(tcbv, &tcpb, sizeof(tcpb));
			memcpy(inpcbv, &inpcb, sizeof(inpcb));
			memcpy(connoid, &tmpoid, sizeof(tmpoid));
			return(BUILD_SUCCESS);
		}
		else if (flgs & (NXT|GET_LEX_NEXT)) {
			if ((cmpval < 0) && (firstone || (oidcmp(&tmpoid, &curroid) < 0))) {
				firstone = 0;
				foundone = 1;
				memcpy(tsock,&tcpsock,sizeof(tcpsock));
				memcpy(tcbv,&tcpb,sizeof(tcpb));
				memcpy(inpcbv,&inpcb,sizeof(inpcb));
				memcpy(&curroid,&tmpoid,sizeof(tmpoid));

			}
		}
	}

	if ((flgs == REQ) || (foundone == 0))
		return(BUILD_ERR);

	memcpy(connoid, &curroid, sizeof(curroid));

	return(BUILD_SUCCESS);
}

#define TCPCONNSIZE	10

int
lu_tcpconnent(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	objident tmpident;
	struct inpcb inpcbvar;
	struct socket tskt;
	struct tcpcb tcbvar;

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);

	memset(&tmpident, 0, sizeof(tmpident));

	/*
	 */
	if (instptr == (objident *)NULL)
		instptr = &tmpident;

	if (instptr->ncmp < TCPCONNSIZE) {
		if (reqflg == REQ)
			return(BUILD_ERR);
		else
			instptr->ncmp = TCPCONNSIZE;
	}

	if (find_tcpconn(instptr, &inpcbvar, &tskt, &tcbvar, reqflg) <= 0)
		return(BUILD_ERR);

	/*
	 *  fill in variable name we are sending back a response for.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));

	/*
	 *  fill in the object instance and return value!
	 */
	memcpy((repl->name.cmp + repl->name.ncmp), instptr->cmp,
		sizeof(u_long)*instptr->ncmp);
	repl->name.ncmp += instptr->ncmp;

	switch (varnode->offset) {
		case N_LADD:
			memcpy(&repl->val.value.ipadd, &inpcbvar.inp_laddr, sizeof(inpcbvar.inp_laddr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_FADD:
			memcpy(&repl->val.value.ipadd, &inpcbvar.inp_faddr, sizeof(inpcbvar.inp_faddr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_LPRT:
			repl->val.value.intgr = inpcbvar.inp_lport;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_FPRT:
			repl->val.value.intgr = inpcbvar.inp_fport;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_TSTE:
			repl->val.value.intgr = tcbvar.t_state + 1;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR,"lu_tcpconnent: invalid offset, %d",varnode->offset);
			return(BUILD_ERR);
	}
}

/*
 *  This function gets the TCP stat variables out of the kernel.
 *  Again, switches on lookup index.  negative is returned on error.
 */
int
lu_tcpstat(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct tcpstat tcps;

#ifdef lint
	instptr = instptr;	/* makes lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if (nl[N_TCPSTAT].n_value == 0) {
		syslog(LOG_ERR, "_tcpstat not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_TCPSTAT].n_value, L_SET);
	if (read(kmem, (char *)&tcps, sizeof(tcps)) != sizeof(tcps)) {
		syslog(LOG_ERR, "lu_tcpstat: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The TCP stat vars have a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	switch (varnode->offset) {
		case N_TMXCO: /* tcpMaxConn */
			repl->val.value.intgr = TCPMAXCONN;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_TACTO:   /* tcpActiveOpens */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_connects;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TATFA:   /* tcpAttemptFails */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_conndrops;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TESRE:   /* tcpEstabResets */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_drops;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TISEG:   /* tcpInSegs */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_rcvtotal;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TOSEG:   /* tcpOutSegs */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_sndtotal -
					       tcps.tcps_sndrexmitpack;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TRXMI:   /* tcpRetransSegs */
#ifdef VANJ_XTCP
			repl->val.value.cntr = tcps.tcps_sndrexmitpack;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif VANJ_XTCP
		case N_TCEST:
			if ((repl->val.value.gauge = find_currestabs()) < 0)
				return(BUILD_ERR);
			repl->val.type = GAUGE;
			return(BUILD_SUCCESS);
		case N_RTOAL:
			if ((repl->val.value.intgr = tcprtoalg) < 0)
				return(BUILD_ERR);
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "lu_tcpstat: bad tcp offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

int
lu_tcprtos(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
#ifdef lint
	instptr = instptr;	/* makes lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The TCP rto vars have a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	switch (varnode->offset) {
		case N_TRMI:
			repl->val.value.intgr = TCPTV_MIN * 1000;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_TRMX:
#ifdef TCPTV_REXMTMAX
			repl->val.value.intgr = TCPTV_REXMTMAX * 1000;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
#else
#  ifdef TCPTV_MAX
			repl->val.value.intgr = TCPTV_MAX * 1000;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
#  else
			return(BUILD_ERR);
#  endif TCPTV_MAX
#endif TCPTV_REXMTMAX
		default:
			syslog(LOG_ERR, "lu_tcprtos: bad tcprto offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

