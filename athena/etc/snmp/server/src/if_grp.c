#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/if_grp.c,v 1.4 1997-02-27 06:47:30 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1995/07/12 03:42:22  cfields
 * Was this code really built under Solaris before?
 *
 * Revision 1.2  1990/05/26  13:37:42  tom
 * athena release 7.0e
 *
 * Revision 1.1  90/04/26  16:33:41  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:44  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/if_grp.c,v 1.4 1997-02-27 06:47:30 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  lookup of MIB variables found in the Interface Group.
 */

#include "include.h"

/*
 *  Look-up the number of nets supported by this gateway.
 *  Actually, number of interfaces.
 */
int
lu_nnets(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	int num;

#ifdef lint
	instptr = instptr;	/* make lint happy */
#endif lint

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);
	if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
		return(BUILD_ERR);

	if ((num = find_nnets()) < 0)
		return(BUILD_ERR);

	/*
	 *  Copy the variable name and Object instance into the
	 *  reply message.  The nnets variable has a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	repl->val.value.intgr = num;
	repl->val.type = INT;
	return(BUILD_SUCCESS);
}

/*
 *  find out how many interfaces are on this machine.  This only includes
 *  any interface found at boot time and any software loopback interface.
 *  the number of interfaces is returned.
 */
int
find_nnets()
{
	struct ifnet ifnet;
	int cnt = 0;
	off_t nloffset;
#if defined(BSD43) || defined(ULTRIX2_2)
	off_t theaddr = 0;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#endif BSD43 || ULTRIX2_2

	if (nl[N_IFNET].n_value == 0) {
		syslog(LOG_ERR, "_ifnet not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_IFNET].n_value, L_SET);
	if (read(kmem,(char *)&nloffset,sizeof(nloffset)) != sizeof(nloffset)) {
		syslog(LOG_ERR, "find_nnets: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  just keep going through the linked list if interface
	 *  structures until we hit the end.
	 */
	while (nloffset) {
		(void)lseek(kmem, (long)nloffset, L_SET);
		if (read(kmem,(char *)&ifnet, sizeof(ifnet)) != sizeof(ifnet)) {
			syslog(LOG_ERR, "find_nnets: read in while: %m");
			return(BUILD_ERR);
		}
		nloffset = (off_t)ifnet.if_next;
#if defined(BSD43) || defined(ULTRIX2_2)
		theaddr = (off_t)ifnet.if_addrlist;
		while (theaddr != 0) {
			(void)lseek(kmem, (long)theaddr, L_SET);
			if (read(kmem,(char *)&ifaddr,sizeof(ifaddr))!=sizeof(ifaddr)) {
				syslog(LOG_ERR, "find_nnets: read ifaddr: %m");
				return(BUILD_ERR);
			}
			theaddr = (off_t)ifaddr.ifa.ifa_next;
			if (ifaddr.ifa.ifa_addr.sa_family == AF_INET) {
				cnt++;
				break;
			}
		}
#else
		if (ifnet.if_addr.sa_family == AF_INET)
			cnt++;
#endif BSD43 || ULTRIX2_2
	}
	return(cnt);  /* return total number of interfaces */
}

/*
 *  finds a particular interface structure from the kernel and fills
 *  it into "ifnetvar".
 */
int
find_ifnet_bynum(intinum, ifnetvar)
	u_long	intinum;
	struct ifnet *ifnetvar;
{
	int cnt = 0;
	off_t addrif;
#if defined(BSD43) || defined(ULTRIX2_2)
	off_t theaddr = 0;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#endif BSD43 || ULTRIX2_2

	if (intinum <= 0)
		return(BUILD_ERR);

	if (nl[N_IFNET].n_value == 0) {
		syslog(LOG_ERR, "_ifnet not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_IFNET].n_value, L_SET);
	if (read(kmem, (char *)&addrif, sizeof(addrif)) != sizeof(addrif)) {
		syslog(LOG_ERR, "find_ifnet_bynum: read: %m");
		return(BUILD_ERR);
	}

	/*
	 *  keep stepping through the interface structure until
	 *  we reach the desired one.
	 */
	while (cnt < intinum) {
		if (addrif == NULL)
			return(BUILD_ERR);
		(void)lseek(kmem, (long)addrif, L_SET);
		if (read(kmem, (char *)ifnetvar, sizeof(struct ifnet)) !=
		    sizeof(struct ifnet)) {
			syslog(LOG_ERR, "find_ifnet_bynum: read in while: %m");
			return(BUILD_ERR);
		}
		addrif = (off_t)ifnetvar->if_next;
#if defined(BSD43) || defined(ULTRIX2_2)
		theaddr = (off_t)ifnetvar->if_addrlist;
		while (theaddr != 0) {
			(void)lseek(kmem, (long)theaddr, L_SET);
			if (read(kmem,(char *)&ifaddr,sizeof(ifaddr))!=sizeof(ifaddr)) {
				syslog(LOG_ERR, "find_ifnet_bynum: read ifaddr: %m");
				return(BUILD_ERR);
			}
			theaddr = (off_t)ifaddr.ifa.ifa_next;
			if (ifaddr.ifa.ifa_addr.sa_family == AF_INET) {
				cnt++;
				break;
			}
		}
#else
		if (ifnetvar->if_addr.sa_family == AF_INET)
			cnt++;
#endif BSD43 || ULTRIX2_2
	}
	return(BUILD_SUCCESS);
}

/*
 *  Look up and fill in the answer for the various interface
 *  specific variables.  Most of the information is contained
 *  in one interface structure.  We use the lookup index (offset)
 *  to properly grab the right field.  Return an error code on
 *  experiencing difficulties.
 */
int
lu_intf(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	struct ifnet ifnet;
	char *ch;
#ifndef index
	char *index();
#endif
	char scope[16];
	struct intf_info *tmp1;
	u_long intnum;
#if defined(BSD43) || defined(ULTRIX2_2)
#ifndef MIT
	off_t theaddr = 0;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
#endif  MIT
#endif BSD43 || ULTRIX2_2

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
	if (varnode->offset <= 0)
		return(BUILD_ERR);

	/*
	 *  find the correct interface structure and
	 *  fill in the values!
	 */
	if ((instptr == (objident *)NULL) || (instptr->ncmp == 0))
		intnum = 1;
	else
		intnum = instptr->cmp[0];

	if ((reqflg == NXT) && ((instptr != (objident *)NULL) && (instptr->ncmp != 0)))
		intnum++;

	if (find_ifnet_bynum(intnum, &ifnet) < 0)
		return(BUILD_ERR);

	memset(&repl->name, varnode->var_code, sizeof(repl->name));

	/*
	 *  fill in the object instance!
	 */
	repl->name.cmp[repl->name.ncmp] = intnum;
	repl->name.ncmp++;

	/*
	 *  Now, each variable has a special offset that we
	 *  can switch on to grab the right information from
	 *  the interface structure.  At first, I had one routine
	 *  for each field, but this grew tiresome.  This is much
	 *  neater (cuts down on small functions) and doesn't
	 *  introduce a major performance loss.
	 */
	switch (varnode->offset) {
		case N_INDEX:	/* Interface Index */
			repl->val.value.intgr = intnum;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_IPKTS:	/* Input Packets */
			repl->val.value.intgr = ifnet.if_ipackets;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_IERRS:   /* Input Errors */
			repl->val.value.intgr = ifnet.if_ierrors;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OPKTS:   /* Output Packets */
			repl->val.value.intgr = ifnet.if_opackets;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_OERRS:   /* Input Errors */
			repl->val.value.intgr = ifnet.if_oerrors;
			repl->val.type = CNTR;
			return(BUILD_SUCCESS);
		case N_IFMTU:   /* MTU */
			repl->val.value.intgr = ifnet.if_mtu;
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_IFSTA:   /* Interface Status */
			if (ifnet.if_flags & IFF_UP)
				repl->val.value.intgr = OP_NORMAL;
			else
				repl->val.value.intgr = INTF_DOWN;

			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_IFATA:   /* Interface Admin Status */
			repl->val.value.intgr = adminstat[intnum-1];
			repl->val.type = INT;
			return(BUILD_SUCCESS);
		case N_SCOPE:   /* Interface Name */
			(void)lseek(kmem, (long)ifnet.if_name, L_SET);
			if (read(kmem, scope, 16) != 16) {
				syslog(LOG_ERR, "lu_intf: read scope: %m");
				return(BUILD_ERR);
			}
			scope[15] = '\0';
			ch = index(scope, '\0');
			*ch++ = ifnet.if_unit + '0';
			*ch = '\0';

			repl->val.value.str.str = (char *)malloc(sizeof(scope));
			if (repl->val.value.str.str == NULL) {
				syslog(LOG_ERR, "lu_intf: malloc: %m.");
				return(BUILD_ERR);
			}
			repl->val.value.str.len = sizeof(scope);
			(void) strcpy(repl->val.value.str.str, scope);
			repl->val.type = STR;
			return(BUILD_SUCCESS);
		case N_IFTYP:   /* Interface Type */
			/*
			 *  find the interface name or scope as we
			 *  like to call it.  this will be used to
			 *  look for the interface name in the list.
			 */
			(void)lseek(kmem, (long)ifnet.if_name, L_SET);
			if (read(kmem, scope, 16) != 16) {
				syslog(LOG_ERR, "lu_intf: read scope: %m");
				return(BUILD_ERR);
			}
			scope[15] = '\0';
			ch = index(scope, '\0');
			*ch++ = ifnet.if_unit + '0';
			*ch = '\0';

			/*
			 *  search for the right interface in the
			 *  list.  If we don't find it, return an
			 *  error so we can return the next lexocographic
			 *  variable.  If we find it, but it is not
			 *  available (value is -1) we return an error.
			 */
			tmp1 = iilst;
			while ((tmp1 != (struct intf_info *)NULL) &&
				 (strcmp(scope, tmp1->name) != 0))
				tmp1 = tmp1->next;

			if (tmp1 == (struct intf_info *)NULL)
				return(BUILD_ERR);

			if (tmp1->itype == -1)
				return(BUILD_ERR);
			else {
				repl->val.value.intgr = tmp1->itype;
				repl->val.type = INT;
			}
			return(BUILD_SUCCESS);
		case N_IFSPD:   /* Interface Speed */
			/*
			 *  find the interface name or scope as we
			 *  like to call it.  this will be used to
			 *  look for the interface name in the list.
			 */
			(void)lseek(kmem, (long)ifnet.if_name, L_SET);
			if (read(kmem, scope, 16) != 16) {
				syslog(LOG_ERR, "lu_intf: read scope: %m");
				return(BUILD_ERR);
			}
			scope[15] = '\0';
			ch = index(scope, '\0');
			*ch++ = ifnet.if_unit + '0';
			*ch = '\0';

			/*
			 *  search for the right interface in the
			 *  list.  If we don't find it, return an
			 *  error so we can return the next lexocographic
			 *  variable.  If we find it, but it is not
			 *  available (value is -1) we return an error.
			 */
			tmp1 = iilst;
			while ((tmp1 != (struct intf_info *)NULL) &&
				 (strcmp(scope, tmp1->name) != 0))
				tmp1 = tmp1->next;

			if (tmp1 == (struct intf_info *)NULL)
				return(BUILD_ERR);

			if (tmp1->speed == -1)
				return(BUILD_ERR);
			else {
				repl->val.value.gauge = tmp1->speed;
				repl->val.type = GAUGE;
			}
			return(BUILD_SUCCESS);
                case N_IFLCG:
                        repl->val.value.time = (u_long)0;
                        repl->val.type = TIME;
                        return(BUILD_SUCCESS);
                case N_OQLEN:
                        repl->val.value.gauge = ifnet.if_snd.ifq_len;
                        repl->val.type = GAUGE;
                        return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "lu_intf: bad interface offset: %d",
						varnode->offset);
			return(BUILD_ERR);
	}
}

int
set_intf(lstnode, reqflg, ercode)
	struct set_struct *lstnode;
	int reqflg;
	int *ercode;
{
	if (lstnode->tptr->flags & NOT_AVAIL) {
		*ercode = NOSUCH;
		return(BUILD_ERR);
	}
	if (lstnode->tptr->offset <= 0) {
		*ercode = NOSUCH;
		return(BUILD_ERR);
	}

	if ((lstnode->ob_inst.cmp[0] == 0) ||
	    (lstnode->ob_inst.cmp[0] > find_nnets())) {
		*ercode = NOSUCH;
		return(BUILD_ERR);
	}

	/*
	 *  Now, each variable has a special offset that we
	 *  can switch on to grab the right information from
	 *  the interface structure.  At first, I had one routine
	 *  for each field, but this grew tiresome.  This is much
	 *  neater (cuts down on small functions) and doesn't
	 *  introduce a major performance loss.
	 */
	switch (lstnode->tptr->offset) {
		case N_IFATA:	/* interface Admin status */
			if (reqflg == SETNEW) {
				if ((lstnode->setv.value.intgr < 1) ||
				    (lstnode->setv.value.intgr > 3)) {
					*ercode = BADVAL;
					return(BUILD_ERR);
				}
				lstnode->oldv.value.intgr = adminstat[lstnode->ob_inst.cmp[0] - 1];
				lstnode->oldv.type = INT;
				adminstat[lstnode->ob_inst.cmp[0] - 1] = lstnode->setv.value.intgr;
			}
			else {
				adminstat[lstnode->ob_inst.cmp[0] - 1] = lstnode->oldv.value.intgr;
			}
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "set_intf: unsupported offset: %d",
				lstnode->tptr->offset);
			*ercode = NOSUCH;
			return(BUILD_ERR);
	}
}

