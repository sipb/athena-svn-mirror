#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ip_grp.c,v 2.2 1994-08-15 15:05:01 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 2.1  93/06/18  14:33:05  tom
 * first cut at solaris port
 * 
 * Revision 2.0  92/04/22  02:03:19  tom
 * release 7.4
 * 	no change
 * 
 * Revision 1.2  90/05/26  13:37:50  tom
 * athena release 7.0e - also some ultrix-ism's added
 * 
 * Revision 1.1  90/04/26  16:36:08  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:50  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ip_grp.c,v 2.2 1994-08-15 15:05:01 cfields Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the IP group.
 */

#include "include.h"
#ifdef SOLARIS
#include <sys/sockio.h>
#endif

int
find_ifnet_byaddr(ipaddr, ifnetvar, ifindex, msgflgs, retaddr)
	struct in_addr	*ipaddr;
	struct ifnet *ifnetvar;
	int *ifindex;
	int msgflgs;
	struct in_addr *retaddr;
{
	int cnt = 1;
	off_t addrif;
	struct ifnet tmpif;
	struct sockaddr_in *ts;
#if defined(BSD43) || defined(ULTRIX2_2)
	int wasip;
	off_t theaddr = 0;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#endif BSD43 || ULTRIX2_2

	retaddr->s_addr = 0xffffffff;

	if (nl[N_IFNET].n_value == 0) {
		syslog(LOG_ERR, "_ifnet not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_IFNET].n_value, L_SET);
	if (read(kmem, (char *)&addrif, sizeof(addrif)) != sizeof(addrif)) {
		syslog(LOG_ERR, "find_ifnet_byaddr: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  keep stepping through the interface structure until
	 *  we reach the desired one.
	 */
	while (addrif) {
#if defined(BSD43) || defined(ULTRIX2_2)
		wasip = 0;
#endif BSD43 || ULTRIX2_2
		(void)lseek(kmem, (long)addrif, L_SET);
		if (read(kmem, (char *)ifnetvar, sizeof(struct ifnet)) !=
		    sizeof(struct ifnet)) {
			syslog(LOG_ERR, "find_ifnet_byaddr: read in while: %m");
			return(BUILD_ERR);
		}
		addrif = (off_t)ifnetvar->if_next;
#if defined(BSD43) || defined(ULTRIX2_2)
		theaddr = (off_t)ifnetvar->if_addrlist;
		while (theaddr != 0) {
			(void)lseek(kmem, (long)theaddr, L_SET);
			if (read(kmem,(char *)&ifaddr,sizeof(ifaddr))!=sizeof(ifaddr)) {
				syslog(LOG_ERR, "find_ifnet_byaddr: read ifaddr: %m");
				return(BUILD_ERR);
			}
			theaddr = (off_t)ifaddr.ifa.ifa_next;
			ts = (struct sockaddr_in *)&ifaddr.in.ia_addr;

			if (ifaddr.ifa.ifa_addr.sa_family == AF_INET) {
				if ((msgflgs == REQ) &&
				    (ts->sin_addr.s_addr == ipaddr->s_addr)) {
					bcopy((char *)ifnetvar, (char *)&tmpif,
							 sizeof(tmpif));
					bcopy((char *)&ts->sin_addr,
						 (char *)retaddr,
						 sizeof(struct in_addr));
					*ifindex = cnt;
					return(BUILD_SUCCESS);
				}
				else if (msgflgs & (NXT|GET_LEX_NEXT)) {
					if ((ts->sin_addr.s_addr > ipaddr->s_addr) && (ts->sin_addr.s_addr < retaddr->s_addr)) {
						bcopy((char *)ifnetvar,
							 (char *)&tmpif,
							 sizeof(tmpif));
						bcopy((char *)&ts->sin_addr,
							 (char *)retaddr,
							 sizeof(struct in_addr));
						*ifindex = cnt;
					}
				}
			wasip++;
			}
		}
		if (wasip)
			cnt++;
#else BSD43 || ULTRIX2_2
		ts = (struct sockaddr_in *)&ifnetvar->if_addr;
		if (ifnetvar->if_addr.sa_family == AF_INET) {
			if ((msgflgs & REQ) &&
			    (ts->sin_addr.s_addr == ipaddr->s_addr)) {
				bcopy((char *)ifnetvar, (char *)&tmpif,
						 sizeof(tmpif));
				bcopy((char *)&ts->sin_addr, (char *)retaddr,
						 sizeof(struct in_addr));
				*ifindex = cnt;
				return(BUILD_SUCCESS);
			}
			else if (msgflgs & (NXT|GET_LEX_NEXT)) {
				if ((ts->sin_addr.s_addr > ipaddr->s_addr) && (ts->sin_addr.s_addr < retaddr->s_addr)) {
					bcopy((char *)ifnetvar, (char *)&tmpif,
						 sizeof(tmpif));
					bcopy((char *)&ts->sin_addr,
						 (char *)retaddr,
						 sizeof(struct in_addr));
					*ifindex = cnt;
				}
			}
		cnt++;
		}
#endif BSD43 || ULTRIX2_2
	}
	if ((msgflgs & REQ) || (*ifindex == 0))
		return(BUILD_ERR);

	bcopy((char *)&tmpif, (char *)ifnetvar, sizeof(tmpif));

	return(BUILD_SUCCESS);
}

int
lu_ipadd(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct ifnet ifnet;
	struct ifreq if_info;
	char scope[16];
	struct sockaddr_in *brdcast;
	struct sockaddr_in *sin;
	int intnum = 0;
	struct in_addr keynet, tmp;
	u_long cnt = 0;
	char *ch;
#if defined(BSD43) || defined(ULTRIX2_2)
	struct sockaddr_in *ts;
	off_t theaddr = 0;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#endif BSD43 || ULTRIX2_2

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);

	bzero((char *)&keynet, sizeof(keynet));
	bzero((char *)&tmp, sizeof(tmp));

	/*
	 *  find the correct interface structure and
	 *  fill in the values!
	 *  The object instance here is the inet address of
	 *  the interface.
	 */
	if (instptr != (objident *)NULL) {
		while (cnt < sizeof(keynet.s_addr)) {
			keynet.s_addr <<= 8;
			if (cnt < instptr->ncmp) {
				keynet.s_addr |= instptr->cmp[cnt];
			}
			cnt++;
		}
	}

	keynet.s_addr = (u_long)ntohl(keynet.s_addr);

	/*
	 *  Tell us if we want to know what we are looking for.
	 */
	if (debuglevel > 2) {
		(void)printf("LOOKING FOR interface: %s\n",inet_ntoa(keynet));
		(void)fflush(stdout);
	}

	if (find_ifnet_byaddr(&keynet, &ifnet, &intnum, reqflg, &tmp) <= 0)
		return(BUILD_ERR);

	/*
	 *  fill in variable name we are sending back a response for.
	 */
	bcopy((char *)varnode->var_code, (char *)&repl->name,
		sizeof(repl->name));

	/*
	 *  fill in the object instance and return value!
	 */
	cnt = 0;
	ch = (char *)&tmp.s_addr;
	while (cnt < sizeof(tmp.s_addr)) {
		repl->name.cmp[repl->name.ncmp] = *ch & 0xff;
		repl->name.ncmp++;
		cnt++;
		ch++;
	}

	switch (varnode->offset) {
		case N_IPADD:
			bcopy((char *)&tmp, (char *)&repl->val.value.ipadd, sizeof(tmp));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_IPIND:
			repl->val.value.intgr = intnum;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_IPBRD:
			/*
			 *  First find the netmask so we can get out
			 *  the last portion of the broadcast adderess
			 *  and determine if it is all ones or Zeros...
			 */
			(void)lseek(kmem, (long)ifnet.if_name, L_SET);
			if (read(kmem, scope, 16) != 16) {
				syslog(LOG_ERR, "lu_ipadd: read scope: %m");
				return(BUILD_ERR);
			}
			scope[15] = '\0';
			ch = index(scope, '\0');
			*ch++ = ifnet.if_unit + '0';
			*ch = '\0';

			(void) strcpy(if_info.ifr_name, scope);
			if (ioctl(snmp_socket, SIOCGIFNETMASK, (char *)&if_info) < 0) {
				syslog(LOG_ERR, "lu_ipadd: ioctl %s: %m",scope);
				return(BUILD_ERR);
			}
			sin = (struct sockaddr_in *)&if_info.ifr_addr;
#if defined(BSD43) || defined(ULTRIX2_2)
			theaddr = (off_t)ifnet.if_addrlist;
			while (theaddr != 0) {
				(void)lseek(kmem, (long)theaddr, L_SET);
				if (read(kmem,(char *)&ifaddr,sizeof(ifaddr))!=sizeof(ifaddr)) {
					syslog(LOG_ERR, "lu_ipadd: read ifaddr: %m");
					return(BUILD_ERR);
				}
				theaddr = (off_t)ifaddr.ifa.ifa_next;
				ts = (struct sockaddr_in *)&ifaddr.in.ia_addr;
	
				if (ifaddr.ifa.ifa_addr.sa_family == AF_INET) {
					if (ts->sin_addr.s_addr == tmp.s_addr) {
						brdcast = (struct sockaddr_in *)&ifaddr.ifa.ifa_broadaddr;
						if ((brdcast->sin_addr.s_addr & ~(sin->sin_addr.s_addr)) == 0)
							repl->val.value.intgr = 0;
						else
							repl->val.value.intgr = 1;
						repl->val.type = INT;
						return(BUILD_SUCCESS);
					}
				}
			}
			return(BUILD_ERR);
#else BSD43 || ULTRIX2_2
			brdcast = (struct sockaddr_in *)&ifnet.if_broadaddr;
			if ((brdcast->sin_addr.s_addr & ~(sin->sin_addr.s_addr)) == 0)
				repl->val.value.intgr = 0;
			else
				repl->val.value.intgr = 1;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
#endif BSD43 || ULTRIX2_2
		case N_IPMSK:
			(void)lseek(kmem, (long)ifnet.if_name, L_SET);
			if (read(kmem, scope, 16) != 16) {
				syslog(LOG_ERR, "lu_ipadd: read scope: %m");
				return(BUILD_ERR);
			}
			scope[15] = '\0';
			ch = index(scope, '\0');
			*ch++ = ifnet.if_unit + '0';
			*ch = '\0';

			(void) strcpy(if_info.ifr_name, scope);
			if (ioctl(snmp_socket, SIOCGIFNETMASK, (char *)&if_info) < 0) {
				syslog(LOG_ERR, "lu_ipadd: ioctl %s: %m",scope);
				return(BUILD_ERR);
			}
			sin = (struct sockaddr_in *)&if_info.ifr_addr;
			bcopy((char *)&sin->sin_addr, (char *)&repl->val.value.ipadd, sizeof(sin->sin_addr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR,"lu_ipadd: invalid offset, %d",varnode->offset);
			return(BUILD_ERR);
	}
}

#ifndef SOLARIS
/*
 *  Given a network number, find the corresponding route entry from
 *  the kernel.  If there is no entry return the "next" entry from
 *  the kernel.  If we have reached the end of the table return an
 *  error code.  Also, only read from the kernel when we are processing
 *  a new packet.
 */
int
find_a_rt(rtaddr, srte, msgflgs)
	struct sockaddr_in *rtaddr;
	struct rtentry *srte;
	int msgflgs;
{
	struct rtentry *rte1, tmprt;
#ifdef ULTRIX3
        struct rtentry **rtehash, mb, *next;
#else
	struct mbuf **rtehash, mb, *next;
#endif
	struct sockaddr_in *sin;
	int hashsize, foundit = 0, i;
	struct in_addr tmpsa;
	int doinghost = 1;

	bzero((char *)&mb, sizeof(mb));
	tmpsa.s_addr = 0xffffffff;

	/*
	 *  Get the size of the hashed routing table.
	 */
	if (nl[N_RTHASH].n_value != 0) {
		(void)lseek(kmem, (long)nl[N_RTHASH].n_value, L_SET);
		if (read(kmem, (char *)&hashsize, sizeof(hashsize)) !=
		   sizeof(hashsize)) {
			syslog(LOG_ERR, "find_a_rt: read hashsize: %m");
			return(BUILD_ERR);
		}
	}
	else {
		hashsize = RTHASHSIZ;
	}

	/*
	 *  Temporary fix because under SunOS 3.5 we modified the kernel to
	 *  run traceroute.  The modified OBJ modules we receieved from the
	 *  network must have had the _rthashsize symbol compiled in to them.
	 *  When the symbol table was arranged, there was an entry for
	 *  _rthashsize.  When nl() was called, it found the symbol, but the
	 *  offset was garbage.  When read from the kernel, we got a 0 value.
	 *  This caused all problems.  I should "#ifdef" this part of the
	 *  code and not rely on whether there is a symbol table entry or
	 *  not.   a big sigh....
	 */
	if (hashsize <= 0)
		hashsize = RTHASHSIZ;
	/*
	 *  If we are processing a new packet, read a snapshot image
	 *  of the kernel routing tables.  If not, we have read them
	 *  already.  The idea here is to use one routing snapshot for
	 *  each packet.
	 */
	if (newpacket != 0) {
		(void)lseek(kmem, (long) nl[N_RTNET].n_value, L_SET);
		if (read(kmem, (char *)rtnet, sizeof(rtnet)) != sizeof(rtnet)) {
			syslog(LOG_ERR, "find_a_rt: read rtnet: %m");
			return(BUILD_ERR);
		}
		(void)lseek(kmem, (long)nl[N_RTHOST].n_value, L_SET);
		if (read(kmem,(char *)rthost,sizeof(rthost))!=sizeof(rthost)) {
			syslog(LOG_ERR, "find_a_rt: read rthost: %m");
			return(BUILD_ERR);
		}
		newpacket = 0;
	}

	/*
	 *  Do network tables first.  Then cycle through and do the host
	 *  tables.
	 */
	rtehash = rtnet;

	/*
	 *  Go through the routing table and find the one we want.
	 *  If we are looking for the NXT, then we must find the
	 *  route which is lexicographically next to the given
	 *  route.  Since the berkeley route tables are not sorted,
	 *  we must go through them and find the "next" one.  gross!
	 */
dohost:
	for (i = 0; i < hashsize; i++) {
#ifdef ULTRIX3
               for (next = rtehash[i]; next != NULL; next = mb.rt_next) {
                       (void)lseek(kmem, (long)next, L_SET);
                       if (read(kmem, (char *)&mb, sizeof(struct rtentry)) != sizeof(struct rtentry)) {
                               syslog(LOG_ERR, "find_a_rt: read mb: %m");
                               return(BUILD_ERR);
                       }
                       rte1 = &mb;

#else
		for (next = rtehash[i]; next != NULL; next = mb.m_next) {
			(void)lseek(kmem, (long)next, L_SET);
			if (read(kmem, (char *)&mb, MMINOFF+sizeof(struct rtentry)) != (MMINOFF+sizeof(struct rtentry))) {
				syslog(LOG_ERR, "find_a_rt: read mb: %m");
				return(BUILD_ERR);
			}
			rte1 = mtod(&mb, struct rtentry *);
#endif
			if (rte1->rt_dst.sa_family != AF_INET)
				continue;
			sin = (struct sockaddr_in *)&rte1->rt_dst;

			if ((msgflgs & (REQ|GET_LEX_NEXT)) &&
			   (sin->sin_addr.s_addr == rtaddr->sin_addr.s_addr)) {
				bcopy((char *)rte1, (char *)srte, sizeof(struct rtentry));
				return(BUILD_SUCCESS);
			}
			else if (msgflgs & (NXT|GET_LEX_NEXT)) {
				if ((sin->sin_addr.s_addr > rtaddr->sin_addr.s_addr) && (sin->sin_addr.s_addr < tmpsa.s_addr)) {
					bcopy((char *)rte1, (char *)&tmprt,
						 sizeof(tmprt));
					bcopy((char *)&sin->sin_addr,
					      (char *)&tmpsa, sizeof(tmpsa));
					foundit = 1;
				}
			}
		}
	}

	/*
	 *  Now go through and do the host route table.
	 */
	if (doinghost) {
		rtehash = rthost;
		doinghost = 0;
		goto dohost;
	}

	/*
	 *  If we get to this point and we have a get-request message,
	 *  we didn't find what we were looking for, send back an error.
	 *  also, if foundit was not set, we didn't find the next, we
	 *  fell off the end of the tree.
	 */
	if ((msgflgs & REQ) || (foundit == 0))
		return(BUILD_ERR);

	/*
	 *  If we found what we want, send back the route entry.
	 */
	bcopy((char *)&tmprt, (char *)srte, sizeof(struct rtentry));
	bcopy((char *)&tmprt.rt_dst,(char *)rtaddr,sizeof(struct sockaddr_in));
	return(BUILD_SUCCESS);
}

/*
 *  Looks up the routing variables and fills in the answer.
 *  returns an error code on problems.  Also, switches on the
 *  lookup index.
 */
int
lu_rtent(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct rtentry rte;
	struct sockaddr_in keynet, *sin;
	u_long cnt = 0;
	char *ch;

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);

	bzero((char *)&keynet, sizeof(keynet));

	/*
	 *  find the correct interface structure and
	 *  fill in the values!
	 *  The object instance here is the inet address of
	 *  the destination route.
	 */
	if (instptr != (objident *)NULL) {
		while (cnt < sizeof(keynet.sin_addr.s_addr)) {
			keynet.sin_addr.s_addr <<= 8;
			if (cnt < instptr->ncmp) {
				keynet.sin_addr.s_addr |= instptr->cmp[cnt];
			}
			cnt++;
		}
	}
	keynet.sin_family = AF_INET;
	keynet.sin_addr.s_addr = (u_long)ntohl(keynet.sin_addr.s_addr);

	/*
	 *  Tell us if we want to know what we are looking for.
	 */
	if (debuglevel > 2) {
		(void)printf("LOOKING FOR rt. dest.: %s\n",inet_ntoa(keynet.sin_addr));
		(void)fflush(stdout);
	}

	if (find_a_rt(&keynet, &rte, reqflg) <= 0)
		return(BUILD_ERR);

	/*
	 *  fill in variable name we are sending back a response for.
	 */
	bcopy((char *)varnode->var_code, (char *)&repl->name,
		sizeof(repl->name));

	/*
	 *  fill in the object instance and return value!
	 */
	cnt = 0;
	ch = (char *)&keynet.sin_addr.s_addr;
	while (cnt < sizeof(keynet.sin_addr.s_addr)) {
		repl->name.cmp[repl->name.ncmp] = *ch & 0xff;
		repl->name.ncmp++;
		cnt++;
		ch++;
	}

	switch (varnode->offset) {
		case N_RTDST:  /* Destination for a route */
			bcopy((char *)&keynet.sin_addr,
				(char *)&repl->val.value.ipadd,
				sizeof(keynet.sin_addr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_RTGAT:  /* Gateway for a route */
			sin = (struct sockaddr_in *)&rte.rt_gateway;
			bcopy((char *)&sin->sin_addr,
				(char *)&repl->val.value.ipadd,
				sizeof(sin->sin_addr));
			repl->val.type = IPADD;
			return(BUILD_SUCCESS);
		case N_RTTYP:  /* type of route: network, direct, subnet */
			if (rte.rt_flags & (RTF_GATEWAY|RTF_HOST))
				repl->val.value.intgr = TO_REMOTE;
			else if (((rte.rt_flags & RTF_GATEWAY) == 0) &&
				 ((rte.rt_flags & RTF_HOST) == 0))
				repl->val.value.intgr = TO_DIRECT;
			else
				repl->val.value.intgr = TO_OTHER;

			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_RTMT2:
		case N_RTMT3:
		case N_RTMT4: /* unsupported rte metrics, return -1 per RFC */
			repl->val.value.intgr = -1;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "lu_rtent: bad route offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

#endif /* SOLARIS */

int
lu_ipstat(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct ipstat icps;

#ifdef lint
	instptr = instptr;	/* make lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if (nl[N_IPSTAT].n_value == 0) {
		syslog(LOG_ERR, "_ipstat not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_IPSTAT].n_value, L_SET);
	if (read(kmem, (char *)&icps, sizeof(icps)) != sizeof(icps)) {
		syslog(LOG_ERR, "lu_ipstat: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The IP stat var group has a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	bcopy((char *)varnode->var_code, (char *)&repl->name,
                sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	switch (varnode->offset) {
		case N_IPTTL: /* ipDefaultTTL */
#ifdef MAXTTL
			repl->val.value.intgr = MAXTTL;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif MAXTTL
		case N_IPINR: /* ipInReceives */
#if defined(BSD43)
			repl->val.value.cntr = icps.ips_total;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		case N_IPHRE: /* ipInHdrErrors */
			repl->val.value.cntr = icps.ips_badsum +
					       icps.ips_tooshort +
					       icps.ips_toosmall +
					       icps.ips_badhlen +
					       icps.ips_badlen;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_IPADE: /* ipInAddrErrors */
#ifdef GATEWAY
			repl->val.value.cntr = 0;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif GATEWAY
		case N_IPFOR: /* ipInForwDatagrams */
#if defined(BSD43) || defined(ULTRIX2_2)
			repl->val.value.cntr = icps.ips_forward;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43 || ULTRIX2_2
		case N_IPNOR: /* ipOutNoRoutes */
#if defined(BSD43) || defined(ULTRIX2_2)
			repl->val.value.cntr = icps.ips_cantforward;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43 || ULTRIX2_2
		case N_IPRTO: /* ipReasmTimeout */
#ifdef IPFRAGTTL
			repl->val.value.intgr = IPFRAGTTL;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif IPFRAGTTL
		case N_IPRAS: /* ipReasmReqds */
#if defined(BSD43)
			repl->val.value.cntr = icps.ips_fragments;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		case N_IPRFL: /* ipReasmFails */
#if defined(BSD43)
			repl->val.value.cntr = icps.ips_fragdropped +
					       icps.ips_fragtimeout;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
#else
			return(BUILD_ERR);
#endif BSD43
		default:
			syslog(LOG_ERR, "lu_ipstat: bad ipstat offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}
		
int
lu_ipforw(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	int ipforvar;

#ifdef lint
	instptr = instptr;	/* make lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if (nl[N_IPFORWARD].n_value == 0) {
		syslog(LOG_ERR, "_ipforwarding not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_IPFORWARD].n_value, L_SET);
	if (read(kmem, (char *)&ipforvar, sizeof(int)) != sizeof(int)) {
		syslog(LOG_ERR, "lu_ipforward: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The IP forwarding var has a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	bcopy((char *)varnode->var_code, (char *)&repl->name,
                sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	if (ipforvar == 1)   /*  we have a gateway(1) as in RFC 1066 */
		repl->val.value.intgr = ipforvar;
	else   /* Does not forward!  host(2) as in RFC 1066 */
		repl->val.value.intgr = 2;

	repl->val.type = INT;
	return(BUILD_SUCCESS);
}

