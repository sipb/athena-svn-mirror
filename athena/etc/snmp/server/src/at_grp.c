#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/at_grp.c,v 1.4 1997-02-27 06:47:20 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1993/06/18 14:35:28  root
 * first cut at solaris port
 *
 * Revision 1.2  90/05/26  13:35:00  tom
 * release 7.0e
 * 
 * Revision 1.1  90/04/26  15:30:36  tom
 * Initial revision
 * 
 * Revision 1.2  89/12/08  15:16:11  snmpdev
 * added chris tengi's patch to prevent the return of incomplete arp cache
 * entries -- kolb
 * 
 * Revision 1.1  89/11/03  15:42:24  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/at_grp.c,v 1.4 1997-02-27 06:47:20 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the AT (address translation) group.
 */

#include "include.h"

/*
 *  This routine looks up a specific ARP entry from the
 *  UNIX kernel.  It handles the LEXI-ordering.
 *  Returns an error on detecting problems.  Returns a valid
 *  ARP table entry and ARP IF index for manipulation by calling
 *  function.
 */
int
find_arpent(obid, arpvar, arpind, flgs)
	objident *obid;
	struct arptab *arpvar;
	int *arpind;
	int flgs;
{
	char *ch;
	u_long *tmpoid;
	int curind, numnets, arpsize, totsize, tarpsize;
	int foundone = 0, cmpval, firstone = 1, cnt;
	objident comparp, curroid;
	struct arptab *arp, *tmparp;

	numnets = find_nnets();

	if (nl[N_ARPTAB].n_value == 0) {
		syslog(LOG_ERR, "find_arpent: _arptab not in nlist");
		return(BUILD_ERR);
	}

	/*
	 *  Get the size of the ARP table
	 */
	if (nl[N_ARPSIZE].n_value != 0) {
		(void)lseek(kmem, nl[N_ARPSIZE].n_value, L_SET);
		if (read(kmem, (char *)&arpsize, sizeof(arpsize)) !=
		   sizeof(arpsize)) {
			syslog(LOG_ERR, "find_arpent: read arpsize: %m");
			return(BUILD_ERR);
		}
	}
	else {
		syslog(LOG_ERR, "find_arpent: no arpsize in nlist: %m");
		return(BUILD_ERR);
	}

	totsize = arpsize * sizeof(struct arptab);
	arp = (struct arptab *)malloc(totsize);
	if (arp == (struct arptab *)NULL) {
		syslog(LOG_ERR, "find_arpent: malloc: %m");
		return(BUILD_ERR);
	}
	(void)lseek(kmem, (off_t) nl[N_ARPTAB].n_value, L_SET);
	if (read(kmem, (char *)arp, totsize) != totsize) {
		syslog(LOG_ERR, "find_arpent: read arp: %m");
		free((char *)arp);
		return(BUILD_ERR);
	}

	/*
	 *  We have the ARP table, now make sure we are looking
	 *  for a valid entry.
	 */
morearp:
	/*
	 *  The interface index.
	 */
	tmpoid = obid->cmp;
	if (*tmpoid > numnets) {
		free((char *)arp);
		return(BUILD_ERR);
	}

	curind = *tmpoid;

	tmpoid++;	/* next component of sub-Obj-ID */

	/*
	 *  The address translation table code of "1" is used
	 *  to specify ARP.
	 */
	if (*tmpoid != 1) {
		free((char *)arp);
		return(BUILD_ERR);
	}

	tarpsize = arpsize;

	/*
	 *  Run through the ARP table looking for the right
	 *  entry.  If we have a GET-NEXT message or we are
	 *  looking for the LEXI-NEXT, compare OID's and save
	 *  the best entry.
	 */
        for (tmparp = arp; tarpsize-- > 0; tmparp++) {
                if (tmparp->at_iaddr.s_addr == 0 || tmparp->at_flags == 0)
                        continue;

		/*  Only respond with complete entries *
		if ( !(tmparp->at_flags & ATF_COM))
			continue;

		/*
		 *  Build up an object ID consisting of the following:
		 *  <intf-index><1><ip-addr>
		 *
		 *  The idea on this is to compare this Obj-ID to the
		 *  requested one.  This makes Lexi-next in this
		 *  environment (with 3 components) much easier.
		 */
		/*
		 *  Do the interface index.
		 */
		comparp.ncmp = 0;
		comparp.cmp[comparp.ncmp] = curind;
		comparp.ncmp++;

		/*
		 *  Do the ARP code of SUB-OID.
		 */
		comparp.cmp[comparp.ncmp] = 1;
		comparp.ncmp++;
		
		/*
		 *  Do the IP address of the ARP entry.
		 */
		cnt = 0;
		ch = (char *)&tmparp->at_iaddr.s_addr;
		while (cnt < sizeof(tmparp->at_iaddr.s_addr)) {
			comparp.cmp[comparp.ncmp] = *ch & 0xff;
			comparp.ncmp++;
			cnt++;
			ch++;
		}
		
		/*
		 *  Print out some useful info if we want it.
		 */
		if (debuglevel > 5) {
			printf("COMPARING: ");
			cnt = 0;
			while (cnt < comparp.ncmp) {
				printf("%lu ", comparp.cmp[cnt]);
				cnt++;
			}
			printf("\n");
			printf("TO: ");
			cnt = 0;
			while (cnt < obid->ncmp) {
				printf("%lu ", obid->cmp[cnt]);
				cnt++;
			}
			printf("\n\n");
			fflush(stdout);
		}

		/*
		 *  Now compare the object-ID's.  Do the usual
		 *  to find the best next, if needed.
		 */
		cmpval = oidcmp(obid, &comparp);
		if ((cmpval == 0) && (flgs & (REQ|GET_LEX_NEXT))) {
			memcpy(arpvar,tmparp,sizeof(struct arptab));
			*arpind = curind;
			free((char *)arp);
			return(BUILD_SUCCESS);
		}
		else if (flgs & (NXT|GET_LEX_NEXT)) {
			if ((cmpval < 0) && (firstone || (oidcmp(&comparp, &curroid) < 0))) {
				firstone = 0;
				foundone = 1;
				memcpy(arpvar,tmparp,sizeof(struct arptab));
				*arpind = curind;
				memcpy(&curroid,&comparp,sizeof(comparp));
			}
		}
	}

	/*
	 *  If it is a request packet and we fell through to here, then
	 *  return an error.  If it was a NXT or GET-NEXT and we did not
	 *  find an entry, increase the Interface index and try again.
	 *  Also, before sending back to try again, Zero out the rest
	 *  of the Object instance leaving the interface index and the
	 *  ARP code of 1 (the first two elements of the oid).
	 *  Up above, if the interface index is too large, then return
	 *  an error, we fell of the end of the table.
	 */
	if (flgs == REQ) {
		free((char *)arp);
		return(BUILD_ERR);
	}

	if (foundone == 0) {
		obid->cmp[0]++;
		memset(obid->cmp + (2 * sizeof(u_long)), 0, (obid->ncmp - 2) * sizeof(u_long));
		goto morearp;
	}

	memcpy(obid, &curroid, sizeof(curroid));

	free((char *)arp);
	return(BUILD_SUCCESS);
}

#define ARPINSTSIZE	6

/*
 *  This routine finds a value for each entry in the AT table.
 *  Return an error on any sort of problem.
 */
int
lu_atent(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	objident tmpident;
	struct arptab at;
	int intindex;

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);

	memset(&tmpident, 0, sizeof(tmpident));

	/*
	 *  If instptr is NULL provide a OID to do the lookup.
	 */
	if (instptr == (objident *)NULL)
		instptr = &tmpident;

	if (instptr->ncmp < ARPINSTSIZE) {
		if (reqflg == REQ)
			return(BUILD_ERR);
		else
			instptr->ncmp = ARPINSTSIZE;
	}

	/*
	 *  The first component of the Sub_OID is the interface
	 *  index.  Check it out. Make sure we are starting on the
	 *  right foot.
	 */
	if ((instptr->cmp[0] ==  0) && (reqflg & REQ))
		return(BUILD_ERR);

	if ((reqflg & (NXT|GET_LEX_NEXT)) && (instptr->cmp[0] == 0))
		instptr->cmp[0]++;

	/*
	 *  The second component of the SUB-OID is the address
	 *  translation code of "1" for an ARP entry.
	 */
	if ((instptr->cmp[1] ==  0) && (reqflg & REQ))
		return(BUILD_ERR);

	if ((reqflg & (NXT|GET_LEX_NEXT)) && (instptr->cmp[1] == 0))
		instptr->cmp[1] = 1;

	/*
	 *  Find the appropriate ARP entry.  Since this routing
	 *  will take care of LEX-NEXT within the SUB-OID, on error
	 *  return.
	 */
	if (find_arpent(instptr, &at, &intindex, reqflg) <= 0)
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
		case N_ARPA:
			memcpy(&repl->val.value.ipadd, &at.at_iaddr, sizeof(at.at_iaddr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_AETH:
			repl->val.value.str.str = malloc(sizeof(at.at_enaddr));
			if (repl->val.value.str.str == NULL) {
				syslog(LOG_ERR, "lu_atent: malloc: %m");
				return(BUILD_ERR);
			}
#if defined(SUN40) || defined (SOLARIS)
			memcpy(repl->val.value.str.str, &at.at_enaddr, sizeof(at.at_enaddr));
#else
#      if defined(BSD43) || defined(ULTRIX2_2)
			memcpy(repl->val.value.str.str, at.at_enaddr, sizeof(at.at_enaddr));
#      else
			memcpy(repl->val.value.str.str, at.at_enaddr.ether_addr_octet, sizeof(at.at_enaddr));
#      endif BSD43
#endif SUN40
			repl->val.value.str.len = sizeof(at.at_enaddr);
			repl->val.type = STR;
			return(BUILD_SUCCESS);
		case N_AINDE:
			repl->val.value.intgr = intindex;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR,"lu_atent: invalid offset, %d",varnode->offset);
			return(BUILD_ERR);
	}
}

