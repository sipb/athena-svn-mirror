#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/traps.c,v 1.3 1997-02-27 06:47:57 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  1990/05/26 13:41:37  tom
 * athena release 7.0e
 *
 * Revision 1.1  90/04/26  18:16:12  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:43:08  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/traps.c,v 1.3 1997-02-27 06:47:57 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988.
 */
/*
 *  This file includes the utilities to send SNMP traps.
 */

#include "include.h"

int
#ifdef MIT
send_snmp_trap(strptyp,vtrptyp)
        int strptyp,vtrptyp;
#else  MIT
send_snmp_trap(strptyp)
	int strptyp;
#endif MIT
{
	trptype trpmsgbuf;
	struct snmp_session *p;
	struct inaddrlst *adlist;
	int trpsendcode;
	int bad = 0;
	struct sockaddr_in trpdst;

	/*
	 * clean the slate
	 */
	memset(&trpmsgbuf, 0, sizeof(trpmsgbuf));
	memset(&trpdst, 0, sizeof(trpdst));

	/*
	 *  construct the trap message.
	 */
	memcpy(&trpmsgbuf.agnt, &local.sin_addr, sizeof(trpmsgbuf.agnt));
	memcpy(&trpmsgbuf.ent, &sys_obj_id, sizeof(sys_obj_id));

	trpmsgbuf.gtrp = strptyp;
#ifdef MIT
	trpmsgbuf.strp = vtrptyp;
#else  MIT
	trpmsgbuf.strp = 0;
#endif MIT
	trpmsgbuf.tm = get_sysuptime();

	if (debuglevel > 2)
		(void) pr_pkt((char *)&trpmsgbuf, TRP);

	/*
	 *  fill in parts of the trap destination
	 */
	trpdst.sin_family = AF_INET;
	if (trapport != (struct servent *)NULL)
		trpdst.sin_port = trapport->s_port;
	else {
		syslog(LOG_ERR,"send_snmp_trap: No TRAP port, abort trap send");
		return(BUILD_ERR);
	}
	/*
	 *  send off the trap message.  if there are multiple addresses
	 *  for one trap session, send off the trap message to
	 *  each one.
	 */
	p = sessions;
	while (p != (struct snmp_session *)NULL) {
		if (p->flags & TRAP_SESS) {
			adlist = p->userlst;
			while (adlist != (struct inaddrlst *)NULL) {
				memcpy(&trpdst.sin_addr,&adlist->sess_addr,
				      sizeof(trpdst.sin_addr));
        			trpsendcode = snmpservsend(snmp_socket,
						TRP,
						&trpdst,
						(char *)&trpmsgbuf,
						p->name,
						strlen(p->name));
				s_stat.outhist[TRP]++;
				s_stat.outpkts++;
				if (trpsendcode < 0) {
					syslog(LOG_ERR, "send_snmp_trap: error in snmpservsend, code %d", trpsendcode);
					s_stat.outerrs++;
					bad++;
				}
				else if (debuglevel > 0) {
                			(void)printf("SNMP trap *SENT* to %s, port %d on %s\n",
					inet_ntoa(trpdst.sin_addr),
					ntohs(trpdst.sin_port),  strtime);
					(void)printf("-------------------------------------------\n");
					(void)fflush(stdout);
				}
				adlist = adlist->nxt;
			}
		}
		p = p->next;
	
	}
	if (bad != 0)
		return(BUILD_ERR);
	else
		return(BUILD_SUCCESS);
}

