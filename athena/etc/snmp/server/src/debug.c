#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/debug.c,v 1.2 1990-04-26 16:35:47 tom Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  90/04/26  15:48:46  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:32  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/debug.c,v 1.2 1990-04-26 16:35:47 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains routines that provide debug and tracing
 *  for snmpd.  While almost all of the errors generated get sent
 *  to syslog, the down and dirty packet traces and tree traces
 *  are done here.
 */

#include "include.h"

/*
 *  this prints out the tree info array (var_tree_info) which is defined
 *  in var_tree.h and initialized in ext.c.
 */
int
ptreeinfo(theinfo)
	struct snmp_tree_info theinfo[];
{
	int cnt = 0, cnt2;
	u_long *c;

	/*
    	 *  go until the end of the list
	 */
	while (theinfo[cnt].codestr != NULL) {
		(void) printf("FLAGS:    %d\n", theinfo[cnt].var_flags);
		(void) printf("NL_DEF:   %d\n", theinfo[cnt].nl_def);
		(void) printf("CODELEN:  %d\n", theinfo[cnt].codestr->ncmp);
		(void) printf("CODESTR: ");
		c = theinfo[cnt].codestr->cmp;
		cnt2 = 0;
		while (cnt2 < theinfo[cnt].codestr->ncmp) {
			(void) printf(" %lu", *c);
			c++;
			cnt2++;
		}
		(void) printf("\n\n");
		(void) fflush(stdout);
		cnt++;
	}
#ifdef MIT
	return(0);
#endif MIT
}

/*
 *  Crudely print out the SNMP variable tree.  If my memory serves me
 *  right, this is a depth first traversal.  If I'm wrong, don't
 *  tell my algorithm analysis professor!
 */
int
pvartree(head)
	struct snmp_tree_node *head;
{
	u_long *c;
	int cnt, cnt1;
	
	if (head->flags & PARENT_NODE)
		(void) printf("****** PARENT! ******\n");
	for (cnt = 0; cnt <= MAXCHILDPERNODE; cnt++) {
		if (head->flags & LEAF_NODE) {
			(void) printf("OFFSET: %d\n", head->offset);
			(void) printf("FLAGS:  %d\n", head->flags);
			(void) printf("CODE:   ");
			if (head->var_code == NULL) {
				(void) printf("NULL\n\n\n");
				(void) fflush(stdout);
#ifndef MIT
				return;
#else   MIT
				return(0);
#endif  MIT
			}
			c = head->var_code->cmp;
			if ((c == NULL) || (head->var_code->ncmp == 0))
				(void) printf("NULL");
			else {
				cnt1 = 0;
				while (cnt1 < head->var_code->ncmp) {
					(void) printf(" %lu", *c);
					c++;
					cnt1++;
				}
			}
			if (head->from != NULL) {
				(void) printf("\nFROM:  %s\n", inet_ntoa(head->from->sin_addr));
				(void) printf("PORT:  %d\n", ntohs(head->from->sin_port));
			}
			(void) printf("\n\n");
			(void) fflush(stdout);
#ifndef MIT
			return;
#else   MIT
			return(0);
#endif  MIT
		}
		/*
		 *  recurse! recurse! recurse!
		 */
		if (head->child[cnt] != SNMPNODENULL) {
			pvartree(head->child[cnt]);
		}
	}
	(void) fflush(stdout);
#ifdef MIT
	return(0);
#endif MIT
}

/*
 *  print out an SNMP packet given the type of packet.
 */
int
pr_pkt(thepkt, pkttype)
	char *thepkt;
	int pkttype;
{
  	getreq *getreqmsg;
  	getnext *getnxtmsg;
  	getrsp *getrspmsg;
  	trptype *trpreqmsg;
  	setreq *setreqmsg;
	int cnt = 0, cnt1 = 0, len;
  	u_long *c;

	switch (pkttype) {
		case REQ:
			(void) printf("GET-REQ MESSAGE:\n");
			getreqmsg = (getreq *)thepkt;
			(void)printf("REQ: reqid:      %ld\n",getreqmsg->reqid);
			(void)printf("REQ: errstat:    %ld\n",getreqmsg->errstat);
			(void)printf("REQ: errindex:   %ld\n",getreqmsg->errindex);
			(void)printf("REQ: varlistlen: %ld\n",getreqmsg->varlist.len);
			while (cnt < getreqmsg->varlist.len) {
				len = getreqmsg->varlist.elem[cnt].name.ncmp;
				c = getreqmsg->varlist.elem[cnt].name.cmp;
				(void)printf("REQ: varlen:     %d\n", len);
				(void)printf("REQ: varname:    ");
				while (cnt1 < len) {
					(void)printf("%lu ", *c);
					cnt1++;
					c++;
				}
				(void)printf("\n");
				cnt++;
				cnt1 = 0;
			}
			break;
		case NXT:
			(void)printf("GET-NEXT MESSAGE:\n");
			getnxtmsg = (getnext *)thepkt;
			(void)printf("NXT: reqid:      %ld\n",getnxtmsg->reqid);
			(void)printf("NXT: errstat:    %ld\n",getnxtmsg->errstat);
			(void)printf("NXT: errindex:   %ld\n",getnxtmsg->errindex);
			(void)printf("NXT: varlistlen: %ld\n",getnxtmsg->varlist.len);
			while (cnt < getnxtmsg->varlist.len) {
				len = getnxtmsg->varlist.elem[cnt].name.ncmp;
				c = getnxtmsg->varlist.elem[cnt].name.cmp;
				(void)printf("NXT: varlen:     %d\n", len);
				(void)printf("NXT: varname:    ");
				while (cnt1 < len) {
					(void)printf("%lu ", *c);
					cnt1++;
					c++;
				}
				(void)printf("\n");
				cnt++;
				cnt1 = 0;
			}
			break;
		case RSP:
			(void)printf("RESPONSE MESSAGE:\n");
			getrspmsg = (getrsp *)thepkt;
			(void)printf("RSP: reqid:      %ld\n",getrspmsg->reqid);
			(void)printf("RSP: errstat:    %ld\n",getrspmsg->errstat);
			(void)printf("RSP: errindex:   %ld\n",getrspmsg->errindex);
			(void)printf("RSP: varlistlen: %ld\n",getrspmsg->varlist.len);
			pr_var_list(&getrspmsg->varlist, "RSP");
			break;
		case TRP:
			trpreqmsg = (trptype *)thepkt;
			(void)printf("TRAP MESSAGE:\n");
			(void)printf("TRP: ent:        ");
			len = trpreqmsg->ent.ncmp;
			c = trpreqmsg->ent.cmp;
			while (cnt1 < len) {
				(void)printf("%lu ", *c);
				cnt1++;
				c++;
			}
			(void)printf("\n");
			(void)printf("TRP: agent:      %s\n",inet_ntoa(trpreqmsg->agnt));
			(void)printf("TRP: generic:    %ld\n",trpreqmsg->gtrp);
			(void)printf("TRP: specific:   %ld\n",trpreqmsg->strp);
			(void)printf("TRP: timestmp:   %lu\n",trpreqmsg->tm);
			(void)printf("TRP: varlistlen: %ld\n",trpreqmsg->varlist.len);
			pr_var_list(&trpreqmsg->varlist, "TRP");
			break;
		case SET:
			(void)printf("SET-REQUEST MESSAGE:\n");
			setreqmsg = (setreq *)thepkt;
			(void)printf("SET: reqid:      %ld\n",setreqmsg->reqid);
			(void)printf("SET: errstat:    %ld\n",setreqmsg->errstat);
			(void)printf("SET: errindex:   %ld\n",setreqmsg->errindex);
			(void)printf("SET: varlistlen: %ld\n",setreqmsg->varlist.len);
			pr_var_list(&setreqmsg->varlist, "SET");
			break;
		default:
			syslog(LOG_ERR, "No such SNMP msg type: %d", pkttype);
			return(BUILD_ERR);
	}
	(void) fflush(stdout);
	(void)printf("\n");
	return(BUILD_SUCCESS);
}

/*
 *  print out a var_list_type.  (variable bindings)
 */
pr_var_list(vlt, msgstr)
	var_list_type *vlt;
	char *msgstr;
{
	int cnt = 0, cnt1 = 0, len;
	u_long *c;

	while (cnt < vlt->len) {
		len = vlt->elem[cnt].name.ncmp;
		c = vlt->elem[cnt].name.cmp;
		(void)printf("%s: varlen:     %d\n", msgstr, len);
		(void)printf("%s: varname:    ", msgstr);
		while (cnt1 < len) {
			(void)printf("%lu ", *c);
			cnt1++;
			c++;
		}
		(void)printf("\nValue: ");
		switch (vlt->elem[cnt].val.type) {
			case STR:
				(void)printf("%s\n",vlt->elem[cnt].val.value.str.str);
				break;
			case INT:
				(void)printf("%d\n", vlt->elem[cnt].val.value.intgr);
				break;
			case CNTR:
				(void)printf("%ul\n", vlt->elem[cnt].val.value.cntr);
				break;
			case GAUGE:
				(void)printf("%ul\n", vlt->elem[cnt].val.value.gauge);
				break;
			case TIME:
				(void)printf("%ul\n", vlt->elem[cnt].val.value.time);
				break;
			case EMPTY:
				(void)printf("EMPTY\n");
				break;
			case IPADD:
				(void)printf("%s\n", inet_ntoa(vlt->elem[cnt].val.value.ipadd));
				break;
			case OBJ:
				len = vlt->elem[cnt].val.value.obj.ncmp;
				c = vlt->elem[cnt].val.value.obj.cmp;
				(void)printf("OBJID: len: %d\n", len);
				(void)printf("       OBJID: cmp: ");
				cnt1 = 0;
				while (cnt1 < len) {
					(void)printf("%lu ", *c);
					cnt1++;
					c++;
				}
				(void)printf("\n");
				break;
			default:
				(void)printf("NO VALUE\n");
				break;
		}
		cnt++;
		cnt1 = 0;
	}
#ifdef MIT
	return(0);
#endif MIT
}

