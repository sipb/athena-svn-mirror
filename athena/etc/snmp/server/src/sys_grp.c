#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/sys_grp.c,v 2.2 1997-03-27 03:08:21 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 2.1  1997/02/27 06:47:50  ghudson
 * BSD -> ANSI memory functions
 *
 * Revision 2.0  1992/04/22 01:58:58  tom
 * release 7.4
 * 	altered format of sysdescr string
 *
 * Revision 1.2  90/05/26  13:41:23  tom
 * athena release 7.0e
 * 
 * Revision 1.1  90/04/26  18:15:05  tom
 * Initial revision
 * 
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/sys_grp.c,v 2.2 1997-03-27 03:08:21 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains specific utilities encompassing
 *  the lookup of MIB variables found in system group.
 */

#include "include.h"
#ifdef SOLARIS
#include <utmpx.h>
#endif

#ifdef MIT
static char buf[BUFSIZ]; 

extern char *get_machtype();
extern char *get_ws_version();
extern char *get_snmp_version_str();
#endif MIT

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
	char *c;

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
	memcpy(&repl->name, varnode->var_code, sizeof(repl->name));
	repl->name.ncmp++;			/* include the "0" instance */

	/*
	 *  switch on lookup index.
	 */
	switch (varnode->offset) {
		case N_VERID:
#ifdef MIT
	                memset(buf, 0, sizeof(buf));

			c = get_machtype();
			if(c && (*c != '\0'))
			  sprintf(buf, "%s\n", c);

#ifdef ATHENA	                
			c = get_ws_version(version_file);
			if(c != (char *) NULL)
			  {
			    strcat(buf, c);
			    strcat(buf, "\n");
			  }
#endif ATHENA

			c = get_snmp_version_str();
			if(c != (char *) NULL)
			  strcat(buf, c);
			  
			if(*supp_sysdescr != '\0')
			  {
			    strcat(buf, "\n");
			    strcat(buf, supp_sysdescr);
			  }
			return(make_str(&(repl->val), buf));

#else  MIT
			repl->val.value.str.str = (char *)malloc((unsigned)strlen(gw_version_id)+1);
			if (repl->val.value.str.str == NULL) {
				syslog(LOG_ERR, "lu_vers: malloc: %m.\n");
				return(BUILD_ERR);
			}
			(void) strcpy(repl->val.value.str.str, gw_version_id);
			repl->val.value.str.len = strlen(gw_version_id);
			repl->val.type = STR;
			break;
#endif MIT
		case N_VEREV:
			memcpy(&repl->val.value.obj, &sys_obj_id,
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
#ifdef SOLARIS
	time_t now;
	struct utmpx *utmpx;

	setutxent();
	while ((utmpx = getutxent()) != NULL) {
		if (utmpx->ut_type == BOOT_TIME) {
			(void) time(&now);
			return((now - utmpx->ut_tv.tv_sec) * 100);
		}
	}
	syslog(LOG_ERR, "BOOT_TIME utmpx entry not in utmpx file");
	return(BUILD_ERR);
#else
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
#endif
}

