#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/udp_grp.c,v 1.3 1997-02-27 06:47:59 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  1990/05/26 13:41:41  tom
 * athena release 7.0e
 *
 * Revision 1.1  90/04/26  18:16:19  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:43:15  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/udp_grp.c,v 1.3 1997-02-27 06:47:59 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the UDP group.
 */

#include "include.h"

/*
 *  This function gets the UDP stat variables out of the kernel.
 *  Again, switches on lookup index.  negative is returned on error.
 */
int
lu_udpstat(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct udpstat udps;

#ifdef lint
	instptr = instptr;	/* makes lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if (nl[N_UDPSTAT].n_value == 0) {
		syslog(LOG_ERR, "_udpstat not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_UDPSTAT].n_value, L_SET);
	if (read(kmem, (char *)&udps, sizeof(udps)) != sizeof(udps)) {
		syslog(LOG_ERR, "lu_udpstat: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The UDP stat vars have a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	switch (varnode->offset) {
		case N_UIERR: /* udpInErrors */
			repl->val.value.cntr = udps.udps_hdrops +
					       udps.udps_badsum +
#if defined(SUN3_3PLUS)
					       udps.udps_fullsock +
#endif SUN3_3PLUS
					       udps.udps_badlen;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "lu_udpstat: bad udp offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

