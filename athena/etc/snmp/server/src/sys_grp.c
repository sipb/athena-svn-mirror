#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/sys_grp.c,v 1.1 1990-04-26 18:15:05 tom Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/11/03  15:43:03  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/sys_grp.c,v 1.1 1990-04-26 18:15:05 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  the lookup of MIB variables found in system group.
 */

#include "include.h"

/*
 */
int
lu_vers(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	time_t sysup;

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
	 *  reply message.  The system group has a NULL Object Instance.
	 *  Because of this the Object instance must be a zero.
	 *  Since the repl->name has been bzero'ed, we can just
	 *  inc the size of the name by one and magically include a
	 *  zero object Instance.
	 */
	bcopy((char *)varnode->var_code, (char *)&repl->name,
		sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	/*
	 *  switch on lookup index.
	 */
	switch (varnode->offset) {
		case N_VERID:
			repl->val.value.str.str = (char *)malloc((unsigned)strlen(gw_version_id)+1);
			if (repl->val.value.str.str == NULL) {
				syslog(LOG_ERR, "lu_vers: malloc: %m.\n");
				return(BUILD_ERR);
			}
			(void) strcpy(repl->val.value.str.str, gw_version_id);
			repl->val.value.str.len = strlen(gw_version_id);
			repl->val.type = STR;
			break;
		case N_VEREV:
			bcopy((char *)&sys_obj_id,
					 (char *)&repl->val.value.obj,
					 sizeof(sys_obj_id));
			repl->val.type = OBJ;
			break;
		case N_LINIT:
			if ((sysup = get_sysuptime()) < 0) {
				syslog(LOG_ERR, "lu_vers: can't get sysuptime");
				return(BUILD_ERR);
			}
			repl->val.value.time = (u_long)sysup;
			repl->val.type = TIME;
			break;
		default:
			syslog(LOG_ERR, "lu_vers: bad static variable");
			return(BUILD_ERR);
	}
	return(BUILD_SUCCESS);
}

/*
 * return system uptime in hundreths of a second.
 */
time_t
get_sysuptime()
{
	time_t now;
	struct timeval bootime;

	(void) time(&now);
	if (nl[N_BOOT].n_value == 0) {
		syslog(LOG_ERR, "_boottime not in namelist.");
		return(BUILD_ERR);
	}

	(void)lseek(kmem, (long)nl[N_BOOT].n_value, L_SET);
	if (read(kmem, (char *)&bootime, sizeof(bootime)) != sizeof(bootime)) {
		syslog(LOG_ERR, "get_sysuptime: read: %m");
		return(BUILD_ERR);
	}
	return((now - bootime.tv_sec) * 100);
}

