#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/icmp_grp.c,v 1.3 1997-02-27 06:47:28 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  1990/05/26 13:37:29  tom
 * athena release 7.0e
 *
 * Revision 1.1  90/04/26  16:32:49  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:40  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/icmp_grp.c,v 1.3 1997-02-27 06:47:28 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the ICMP group.
 */

#include "include.h"

/*
 *  This function gets the ICMP variables out of the kernel.
 *  Again, switches on lookup index.  error is returned on error.
 */
int
lu_icmp(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct icmpstat icps;

#ifdef lint
	instptr = instptr;	/* makes lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if (nl[N_ICMPSTAT].n_value == 0) {
		syslog(LOG_ERR, "_icmpstat not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_ICMPSTAT].n_value, L_SET);
	if (read(kmem, (char *)&icps, sizeof(icps)) != sizeof(icps)) {
		syslog(LOG_ERR, "lu_icmp: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The ICMP group has a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	switch (varnode->offset) {
		case N_INMSG: /* ICMP msgs in (icmpInMsgs) */
			repl->val.value.cntr = icps.icps_badcode +
					icps.icps_checksum +
					icps.icps_badlen +
					icps.icps_inhist[ICMP_UNREACH] +
					icps.icps_inhist[ICMP_TIMXCEED] +
					icps.icps_inhist[ICMP_PARAMPROB] +
					icps.icps_inhist[ICMP_SOURCEQUENCH] +
					icps.icps_inhist[ICMP_REDIRECT] +
					icps.icps_inhist[ICMP_ECHO] +
					icps.icps_inhist[ICMP_ECHOREPLY] +
					icps.icps_inhist[ICMP_TSTAMP] +
#ifdef BSD43
					icps.icps_inhist[ICMP_MASKREQ] +
					icps.icps_inhist[ICMP_MASKREPLY] +
#endif BSD43
					icps.icps_inhist[ICMP_TSTAMPREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ICERR:   /* ICMP errors in (icmpInErrors, 2) */
			repl->val.value.cntr = icps.icps_badcode +
						icps.icps_checksum +
						icps.icps_badlen;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_DSTUN:   /* icmpInDestUnreachs, 3 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_UNREACH];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_TTLEX:   /* icmpInTimeExcds, 4 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_TIMXCEED];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_IBHDR:   /* icmpInParmProbs, 5 */
			repl->val.value.cntr= icps.icps_inhist[ICMP_PARAMPROB];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_SRCQE:   /* icmpInSrcQuenchs, 6 */
			repl->val.value.cntr=icps.icps_inhist[ICMP_SOURCEQUENCH];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_IREDI:   /* icmpInRedirects, 7 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_REDIRECT];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ECREQ:   /* icmpInEchos, 8 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_ECHO];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ECREP:   /* icmpInEchoReps, 9 */
			repl->val.value.cntr=icps.icps_inhist[ICMP_ECHOREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ITIME:   /* icmpInTimestamps, 10 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_TSTAMP];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ITREP:   /* icmpInTimestampReps, 11 */
			repl->val.value.cntr = icps.icps_inhist[ICMP_TSTAMPREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_MSKRQ:   /* icmpInAddrMasks, 12 */
#ifdef BSD43
			repl->val.value.cntr = icps.icps_inhist[ICMP_MASKREQ];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		case N_MSKRP:   /* icmpInAddrMaskReps, 13 */
#ifdef BSD43
			repl->val.value.cntr= icps.icps_inhist[ICMP_MASKREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		case N_ORESP:   /* icmpOutMsgs, 14 */
			repl->val.value.cntr = icps.icps_reflect +
						icps.icps_error;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OERRR:   /* icmpOutErrors, 15 */
			repl->val.value.cntr = icps.icps_error;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OSTUN:   /* icmpOutDestUnreachs, 17 */
			repl->val.value.cntr = icps.icps_outhist[ICMP_UNREACH];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OTLEX:   /* icmpOutTimeExcds, 18 */
			repl->val.value.cntr= icps.icps_outhist[ICMP_TIMXCEED];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OBHDR:   /* icmpOutParmProbs, 18 */
			repl->val.value.cntr=icps.icps_outhist[ICMP_PARAMPROB];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_ORCQE:   /* icmpOutSrcQuenchs, 19 */
			repl->val.value.cntr=icps.icps_outhist[ICMP_SOURCEQUENCH];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OREDI:   /* icmpOutRedirects, 20 */
			repl->val.value.cntr= icps.icps_outhist[ICMP_REDIRECT];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OCREQ:   /* icmpOutEchos, 21 */
			repl->val.value.cntr = icps.icps_outhist[ICMP_ECHO];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OCREP:   /* icmpOutEchoReps, 22 */
			repl->val.value.cntr=icps.icps_outhist[ICMP_ECHOREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OTIME:   /* icmpOutTimestamps, 23 */
			repl->val.value.cntr = icps.icps_outhist[ICMP_TSTAMP];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OTREP:   /* icmpOutTimestampReps, 24 */
			repl->val.value.cntr = icps.icps_outhist[ICMP_TSTAMPREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OSKRQ:   /* icmpOutAddrMasks, 25 */
#ifdef BSD43
			repl->val.value.cntr = icps.icps_outhist[ICMP_MASKREQ];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		case N_OSKRP:   /* icmpOutAddrMaskReps, 26 */
#ifdef BSD43
			repl->val.value.cntr=icps.icps_outhist[ICMP_MASKREPLY];
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		default:
			syslog(LOG_ERR, "lu_icmp: bad icmp offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

