#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/agent.c,v 1.4 1997-02-27 06:46:58 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1993/06/18 14:35:19  root
 * first cut at solaris port
 *
 * Revision 1.2  90/05/26  13:34:53  tom
 * release 7.0e
 * 
 * Revision 1.1  90/04/26  15:28:15  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:22  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/agent.c,v 1.4 1997-02-27 06:46:58 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains routines that use a crude protocol to
 *  talk to other unix daemons to gather information.  For
 *  example, gated, named.
 */

#include "include.h"

/*
 *  initialize the AGENT socket, port, etc.
 */
int
agent_init()
{
	int agentinits;

	addr.sin_family = AF_INET;
	addr.sin_port = htons((short)AGENT_PORT);
	agentinits = get_snmp_socket(AF_INET, SOCK_DGRAM, &addr);
	if (agentinits < 0)
		return(GEN_ERR);
	return(agentinits);
}

/*
 *  This function processes a registration request from an agent/daemon.
 *  (like gated)  The strategy is to listen on a well known port
 *  AGENT_PORT and add variables to the tree as told to you by the
 *  daemon.
 */
agentin(from, size, pkt)
	struct sockaddr *from;
	int size;
	char *pkt;
{
	struct snmp_tree_info	tt;
	struct snmp_tree_node	*tn;
	struct sockaddr_in *sin_from = (struct sockaddr_in *)from;
	int var_size, cnt;
	u_long *ch;

	if (debuglevel > 0) {
		(void) printf("AGENT packet received from %s on %s\n",
                      	inet_ntoa(sin_from->sin_addr), strtime);
		(void) printf("Size: %d bytes\n", size);
		(void) fflush(stdout);
	}

	switch (*pkt) {
		case AGENT_REQ:
		case AGENT_ERR:
		case AGENT_RSP:
			syslog(LOG_ERR, "stray agent msg: type %d", *pkt);
			break;
		case AGENT_REG:
			tt.codestr = (objident *)malloc(sizeof(objident));
			if (tt.codestr == NULL) {
				syslog(LOG_ERR,"agentin: malloc: %m");
				return;
			}
			pkt++;   /* get to the first variable */
			size--;
			while (size > 0) {
				cnt = 0;
				var_size = *pkt;
				pkt++;		 /* point to start of var */
				size--;
				tt.codestr->ncmp = var_size;
				ch = tt.codestr->cmp;
				while (cnt++ < var_size)
					*ch++ = *pkt++;
				tt.var_flags = AGENT_VAR | LEAF_NODE;
				tt.nl_def = 0;
				tt.valfunc = get_agent_var;
				tt.valsetfunc = NULL;
				tn = init_var_tree_node(LEAF_NODE, &tt);
				if (tn == SNMPNODENULL) {
					syslog(LOG_ERR, "agentin: couldn't create new tree node");
					break;
				}
				tn->from = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
				if (tn->from == NULL) {
					syslog(LOG_ERR, "agentin: malloc: %m");
					break;
				}
				memcpy(tn->from, sin_from, sizeof(struct sockaddr_in));
				if (debuglevel > 3) {
					(void) printf("\nADDING AGENT VARIABLE:\n\n");
					pvartree(tn);
				}
				if (connect_node(tn) == CONNECT_FAIL) {
					syslog(LOG_ERR, "agentin: couldn't connect new node.");
					if (debuglevel > 3)
						(void) printf("ADD FAILED!!!\n");
				}
				size -= var_size;
			}
			(void) free((char *)tt.codestr);
			break;
		default:
			syslog(LOG_ERR, "unknown agent msg: type %d", *pkt);
			break;
	}
	return;
}

/*
 *  Get the agent variable from the agent.  Wait for a reply from
 *  the agent and act accordingly.
 *
 *	TO DO:  Make Lexi-stuff work on agent error
 */
sigjmp_buf env;

int
get_agent_var(varnode, repl, instptr, reqflg)
	struct snmp_tree_node *varnode;
	varbind *repl;
	objident *instptr;
	int reqflg;
{
	char reqpkt[SNMPMAXPKT];
	char agntpkt[SNMPMAXPKT];
	char *p, *ctmp;
	int size, pktsnd, pktrec, remotelen, rspsize, agntvartype;
	int cnt = 0, firstsend = 1;
	struct itimerval interval, disable;
	struct sockaddr_in remote, *krt, keynet;
	struct rtentry rte;
	void onintr();
	u_long *ch;
	struct sigaction action;

#define TMO	3

	if (varnode->flags & NOT_AVAIL)
		return(BUILD_ERR);
        if ((varnode->flags & NULL_OBJINST) && (reqflg == NXT))
                return(BUILD_ERR);

	interval.it_interval.tv_sec = 0;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = TMO;
	interval.it_value.tv_usec = 0;
	disable.it_interval.tv_sec = 0;
	disable.it_interval.tv_usec = 0;
	disable.it_value.tv_sec = 0;
	disable.it_value.tv_usec = 0;

	/*
	 *  Fill in the request packet to be sent to the
	 *  agent/daemon.  The format is as follows:
	 *
	 *  [msg_type] [var. length] [variable requested]
	 *   byte 1      byte 2       byte 3 - MAX
	 */
	p = reqpkt;
	ch = varnode->var_code->cmp;
	*p++ = AGENT_REQ;
	*p++ = varnode->var_code->ncmp;
	while (cnt++ < varnode->var_code->ncmp)
		*p++ = *ch++ & 0xff;
		
	size = varnode->var_code->ncmp + 2;

        memset(&keynet, 0, sizeof(keynet));
        keynet.sin_family = AF_INET;
	cnt = 0;

	/*
	 *  If we have a routing variable, copy the dst network into
	 *  the request.
	 */
	if ((varnode->flags & NULL_OBJINST) == 0) {
                while ((instptr != (objident *)NULL) &&
		       (cnt < sizeof(keynet.sin_addr.s_addr))) {
                        keynet.sin_addr.s_addr <<= 8;
                        if (cnt < instptr->ncmp) {
                                keynet.sin_addr.s_addr |= instptr->cmp[cnt];
                        }
                        cnt++;
                }
		keynet.sin_addr.s_addr = (u_long)ntohl(keynet.sin_addr.s_addr);
getanother:
#ifndef SOLARIS
        	if (find_a_rt(&keynet, &rte, reqflg) <= 0)
#endif
                	return(BUILD_ERR);

		krt = (struct sockaddr_in *)&rte.rt_dst;
		memcpy(p, &krt->sin_addr.s_addr, sizeof(krt->sin_addr.s_addr));
		if (firstsend) {
			size += sizeof(krt->sin_addr.s_addr);
			reqpkt[1] += sizeof(krt->sin_addr.s_addr);
		}

	        /*
	         *  Tell us if we want to know what we are looking for.
	         */
	        if (debuglevel > 2) {
	                (void) printf("LOOKING FOR Agent rt. dest.: %s\n",inet_ntoa(keynet.sin_addr));
	                (void) fflush(stdout);
	        }
        }

        /*
         *  fill in variable name we are sending back a response for.
         */
        memcpy(&repl->name, varnode->var_code, sizeof(repl->name));

        /*
         *  fill in the object instance and return value!
         */
	if ((varnode->flags & NULL_OBJINST) == 0) {
	        cnt = 0;
        	ctmp = (char *)&keynet.sin_addr.s_addr;
        	while (cnt < sizeof(keynet.sin_addr.s_addr)) {
                	repl->name.cmp[repl->name.ncmp] = *ctmp & 0xff;
                	repl->name.ncmp++;
                	cnt++;
                	ctmp++;
        	}
	}
	else {
		repl->name.ncmp++;	/* include the "0" instance */
	}

	/*
	 *  Save the environment, so in case of a time-out on
	 *  the request, we can return an error.
	 */
	if (sigsetjmp(env, 1) == 3)
		return(BUILD_ERR);

	/*
	 *  Ignore signal SIGALRM when we send the request.
	 *  Send the request packet out!
	 */
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = SIG_IGN;
	sigaction(SIGALRM, &action, NULL);

	pktsnd = sendto(agent_socket, reqpkt, size, 0,
			(struct sockaddr *)varnode->from,
			sizeof(struct sockaddr_in));
	if (pktsnd < 0) {
		syslog(LOG_ERR, "get_agent_var: sendto: %m");
		return(BUILD_ERR);
	}

	/*
	 *  Enable and set the timer, if we timeout, perform onintr()
	 *  which results in an error returned.
	 *
	 *  If, while we are waiting for a response, we get another
	 *  type of packet (AGENT_REG) register the variable and
	 *  go wait for the response again.
	 */
wantrsp:

	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = onintr;
	sigaction(SIGALRM, &action, NULL);
	(void) setitimer(ITIMER_REAL, &interval, (struct itimerval *)NULL);
	remotelen = sizeof(remote);
 	pktrec = recvfrom(agent_socket, agntpkt, SNMPMAXPKT, 0,
			  (struct sockaddr *)&remote, &remotelen);
	(void) setitimer(ITIMER_REAL, &disable, (struct itimerval *)NULL);

	/*
	 *  got a reply from the agent/daemon. See what it is.
	 */
	switch (agntpkt[0]) {
		case AGENT_REG:
			syslog(LOG_ERR, "get_agent_var: processing registration from %s", inet_ntoa(remote.sin_addr));
			agentin((struct sockaddr *)&remote, pktrec, agntpkt);
			goto wantrsp;
		case AGENT_REQ:
			syslog(LOG_ERR, "get_agent_var: invalid request from %s", inet_ntoa(remote.sin_addr));
			goto wantrsp;
		case AGENT_ERR:
			syslog(LOG_ERR, "get_agent_var: error response from %s", inet_ntoa(remote.sin_addr));
			/*
			 *  If we have a routing variable, try again
			 *  with the NXT route.  We will fall off
			 *  the end of the tree and find_a_rt() will
			 *  return an error so no chance of looping.
			 */
			if ((varnode->flags & NULL_OBJINST) == 0) {
				reqflg = NXT;
				firstsend = 0;
				goto getanother;
			}
			return(BUILD_ERR);
		case AGENT_RSP:  /* got the right one! */
			p = agntpkt;
			p++;
			agntvartype = *p++;
			rspsize = *p++;
			switch (agntvartype) {
				case INT:
					memcpy(&repl->val.value.intgr, p, rspsize);
					repl->val.type = INT;
					break;
				case CNTR:
					memcpy(&repl->val.value.cntr, p, rspsize);
					repl->val.type = CNTR;
					break;
				default:
					syslog(LOG_ERR, "get_agent_var: invalid variable type: %d", agntvartype);
					return(BUILD_ERR);
			}
			if (debuglevel > 3) {
				(void) printf("AGENT_RSP from %s\n", inet_ntoa(remote.sin_addr));
				(void) printf("          size %d\n\n", rspsize);
				(void) fflush(stdout);
			}
			return(BUILD_SUCCESS);
		default:
			syslog(LOG_ERR, "get_agent_var: invalid agent pkt from %s", inet_ntoa(remote.sin_addr));
			return(BUILD_ERR);
	}
}
			

void onintr()
{
	siglongjmp(env, 3);
}

