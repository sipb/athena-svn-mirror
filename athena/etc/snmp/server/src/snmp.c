#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/snmp.c,v 2.0 1992-04-22 02:00:11 tom Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  90/05/26  13:40:34  tom
 * athena release 7.0e - silenced some common error conditions
 * 
 * Revision 1.1  90/04/26  17:59:47  tom
 * Initial revision
 * 
 * Revision 1.2  89/12/08  15:29:37  snmpdev
 * added chris tengi's patch for default port numbers (useful when you don't
 * have root, and the person who does is a fascist) -- kolb
 * 
 * Revision 1.1  89/11/03  15:43:00  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/snmp.c,v 2.0 1992-04-22 02:00:11 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains SNMP specific routines encompassing
 *  initialization, setup, and processing of SNMP packets.
 */

#include "include.h"

/*
 *  Check out a newly received SNMP packet.
 *  parse it, authenticate it, and get back all
 *  relevant information.  Pass parsed packet on for processing.  If
 *  Processing went well, then pass reply message on for sending.
 *  This is the top level routing from which an SNMP message is handled.
 *  (from initial receive to processing to final send)
 */
snmpin(from, size, pkt)
	struct sockaddr *from;
	int size;
	char *pkt;
{
	short sesslen;
	char decmsg[sizeof (pdu_type)];
	char replymsg[sizeof (pdu_type)];
	short snmptype;
	short retype;
  	char *thesession;
	struct sockaddr_in *sin_from = (struct sockaddr_in *)from;
	int pktsendcode, pktproccode;
	getrsp *respmsg;
	getrsp *initrespmsg;

	if (debuglevel > 0) {
		(void) printf("-------------------------------------------\n");
		(void) printf("SNMP packet *RECEIVED* from %s, port %d on %s\n",
                      	inet_ntoa(sin_from->sin_addr),
			ntohs(sin_from->sin_port),  strtime);
		(void) printf("Size: %d bytes\n", size);
		(void) fflush(stdout);
	}

	/*
	 *  zero out buffers and get space for the session name.
	 */
	thesession = (char *)malloc(sizeof(char) * (1 + SNMPMXSID));
	if (thesession == NULL) {
		syslog(LOG_ERR, "snmpin: malloc: %m");
		return;
	}
	bzero(thesession, (SNMPMXSID + 1));
	bzero(decmsg, sizeof (pdu_type));
	bzero(replymsg, sizeof (pdu_type));

	/*
	 *  Pass input buffer to ASN.1 parser for parsing.  See what
	 *  type of message this is.
	 */
	snmptype = snmpdecipher(thesession,&sesslen,decmsg,pkt,(short)size,MAXBUFSIZE);
	if (snmptype <= 0) {
		syslog(LOG_ERR, "snmpin: error in snmpdecipher, code %d",
							snmptype);
		s_stat.parseerrs++;
		goto bad;
	}

	/*
	 *  Check and see that the session is allowed and that the
	 *  sender is valid.  Also, if a set request, make sure the
	 *  session is READ_WRITE.  Return error code if invalid.
	 *  If we have Authentication traps enabled, send one out.
	 */
	if (check_sess(thesession, sin_from, snmptype) < 0) {
	  
#ifdef MIT
	 /*
	  * We don not wish to log incorrect session names..
	  * we're probably the ones providing them!
	  */
	        syslog(LOG_ERR, "bad use of session from %s",
				inet_ntoa(sin_from->sin_addr));
#else  /* MIT */
		syslog(LOG_ERR, "bad use of session %s, from %s",
				thesession, inet_ntoa(sin_from->sin_addr));
#endif /* MIT */
		s_stat.badsession++;

		if (send_authen_traps) {
#ifdef MIT
			if (send_snmp_trap(AUTHFAIL, 0) < 0)
#else  MIT
		        if (send_snmp_trap(AUTHFAIL) < 0)
#endif MIT
				syslog(LOG_ERR,"snmpin: trouble sending traps");
		}
		goto bad;
	}

	/*
	 *  We are processing a new packet!  Flag it as such.
	 *  Process the SNMP packet.  Be it a get request or a set
	 *  request or whatever.  Fill in replymsg.  When an error returns,
	 *  this function has placed the appropriate information in the
	 *  reply message.  We should still send the message.
	 */
	newpacket++;
	pktproccode = proc_snmp_msg(decmsg, replymsg, snmptype);
	if (pktproccode < 0) {
#ifdef MIT
		syslog(LOG_INFO, "snmpin: error in proc_snmp_msg, code %d",
							pktproccode);
#else  MIT
		syslog(LOG_ERR, "snmpin: error in proc_snmp_msg, code %d",
							pktproccode);
#endif MIT
		s_stat.procerr++;
	}
	respmsg = (getrsp *)replymsg;

	/*
	 *  if the reply was in error, we must send back the same
	 *  packet we received with the error status  and error index
	 *  set.  These variables were set while processing.  Just do
	 *  the switcheroo....
	 *
	 *  With SET messages, you always return the same packet you
	 *  received.  The error status and error index are the
	 *  indicators of error or success.  These were set in
	 *  the procset() function.
	 */
	if ((respmsg->errstat > 0) || (snmptype == SET)) {
		initrespmsg = (getrsp *)decmsg;
		initrespmsg->errstat = respmsg->errstat;
		initrespmsg->errindex = respmsg->errindex;
	}
	else {
		initrespmsg = (getrsp *)replymsg;
	}

	/*
	 *  Set message type we are sending back!
	 *  All the same for now.
	 */
	switch (snmptype) {
		case REQ:
		case RSP:
		case TRP:
		case SET:
		case NXT: retype = RSP; break;
		default:
			syslog(LOG_ERR, "snmpin: bad msgtype %d", snmptype);
			s_stat.badtype++;
			varlist_free(&initrespmsg->varlist);
			goto bad;
	}

	if (debuglevel > 2)
		(void) pr_pkt((char *)initrespmsg, retype);

	/*
	 *  Send back SNMP response to received packet.
	 *  If we get back an error code from the packet builder
	 *  that the packet is TOOLONG (large) for a UDP datagram,
	 *  we send back the request packet with a TOOBIG error as
	 *  per the SNMP RFC.  If this fails (it shouldn't), we just give
	 *  up.
	 */
	pktsendcode = snmpservsend(snmp_socket,retype,sin_from,(char *)initrespmsg,thesession,(short)strlen(thesession));
	s_stat.outhist[retype]++;
	s_stat.outpkts++;
	if (pktsendcode < 0) {
		syslog(LOG_ERR, "snmpin: error in snmpservsend, code %d",
							pktsendcode);
		s_stat.outerrs++;
		if (pktsendcode == TOOLONG) { /* too large for UDP! */
			syslog(LOG_ERR, "snmpin: pkt too large, code %d", pktsendcode);
			s_stat.toobig++;
			respmsg = (getrsp *)decmsg;
			respmsg->errstat = TOOBIG;
			respmsg->errindex = 0;
			pktsendcode = snmpservsend(snmp_socket,retype,sin_from,decmsg,thesession,strlen(thesession));
			if (pktsendcode < 0)
				syslog(LOG_ERR, "snmpin: error in sending too large request back, code %d, giving up.", pktsendcode);
				s_stat.outerrs++;
		}
		varlist_free(&initrespmsg->varlist);
		goto bad;
	}
	if (debuglevel > 0) {
		(void) printf("SNMP packet *SENT* to %s, port %d on %s\n",
                      	inet_ntoa(sin_from->sin_addr),
			ntohs(sin_from->sin_port),  strtime);
		(void) printf("-------------------------------------------\n");
		(void) fflush(stdout);
	}
	varlist_free(&initrespmsg->varlist);
bad:
	newpacket = 0;
	free(thesession);  /* don't need this anymore */
	return;
}

/*
 *  initialize the SNMP socket, port, etc.
 */
int
snmp_init()
{
	int snmpinits;

	/*
	 *  get the trap port first.
	 */
	trapport = getservbyname("snmp-trap", "udp");
	if (trapport == NULL) {
		syslog(LOG_ERR,
		    "Using default snmptrap port %d\n", SNMPTRAPPORT);
		snmptrap.s_name = "snmp-trap";
		snmptrap.s_aliases = NULL;
		snmptrap.s_port = htons(SNMPTRAPPORT);
		snmptrap.s_proto = "udp";
		trapport = &snmptrap;
	}
	else {
		bcopy((char *)trapport, (char *)&snmptrap, sizeof(snmptrap));
		trapport = &snmptrap;
	}

	/*
	 *  Now get the SNMP services port.
	 */
	sp = getservbyname("snmp", "udp");
	if (sp == NULL) {
		syslog(LOG_ERR,"Using default snmp port %d\n", SNMPPORT);
		sp = (struct servent *)malloc(sizeof(struct servent));
		sp->s_name = "snmp";
		sp->s_aliases = NULL;
		sp->s_port = htons(SNMPPORT);
		sp->s_proto = "udp";
	}
	addr.sin_family = AF_INET;
	addr.sin_port = sp->s_port;
	snmpinits = get_snmp_socket(AF_INET, SOCK_DGRAM, &addr);
	if (snmpinits < 0)
		return(GEN_ERR);
	return(snmpinits);
}

/*
 *  Open the SNMP socket.
 */
int
get_snmp_socket(domain, type, sin)
	int domain, type;
	struct sockaddr_in *sin;
{
	int snmpsocks;
#ifdef SO_RCVBUF
	int on = 1;
#endif SO_RCVBUF

	if ((snmpsocks = socket(domain, type, 0)) < 0) {
		syslog(LOG_ERR, "get_snmp_socket: socket %m");
		return (GEN_ERR);
	}
#ifdef SO_RCVBUF
	on = 48*1024;
	if (setsockopt(snmpsocks,SOL_SOCKET,SO_RCVBUF,(char *)&on,sizeof(on))<0)
		syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
#endif SO_RCVBUF
	if (bind(snmpsocks, (struct sockaddr *)sin, sizeof (*sin)) < 0) {
		syslog(LOG_ERR, "get_snmp_socket: bind %m");
		(void) close(snmpsocks);
		return (GEN_ERR);
	}
	return (snmpsocks);
}

/*
 *  given a session name, an address, and a message type, make
 *  sure the session is valid and the user is valid.  If an
 *  address specified with a session in the config file is 0.0.0.0,
 *  then anyone can use that session.  If a non-zero address is
 *  specified, only that address can use that session.  Also,
 *  don't allow a SET REQUEST to be used on a READ_ONLY session!
 */
int
check_sess(snam, whofrom, what)
	char *snam;
	struct sockaddr_in *whofrom;
	short what;
{
	struct snmp_session *tmp;
	int isvalid = 0;
	struct inaddrlst *adlist;

	/*
	 *  Find the proper session.  If there are no sessions of
	 *  this name or there are no sessions specified at all,
	 *  return an error.  We will not allow this request to
	 *  be serviced.
	 */
	tmp = sessions;
	while ((tmp != (struct snmp_session *)NULL) &&
		(strcmp(tmp->name, snam) != 0))
		tmp = tmp->next;

	if (tmp == (struct snmp_session *)NULL) {
#ifndef MIT
		syslog(LOG_ERR, "session %s not defined.", snam);
#endif /* MIT */
		return(GEN_ERR);
	}
	else {
		/*
		 *  We found a proper session name, but is the
		 *  correct address using it?  Run through the
		 *  address list for this session and see if we
		 *  have a valid user.
		 */
		adlist = tmp->userlst;
		while (adlist != (struct inaddrlst *)NULL) {
			if ((adlist->sess_addr.s_addr == whofrom->sin_addr.s_addr) ||
			    (adlist->sess_addr.s_addr == 0)) {
				isvalid++;
				break;
			}
			adlist = adlist->nxt;
		}
		if (isvalid == 0)
			return(GEN_ERR);

		/*
		 *  Make sure this session allows this type of
		 *  request.  If not, dropkick the packet.
		 */
		switch (what) {
			case SET:
				if ((tmp->flags & READ_WRITE) == 0)
					return(GEN_ERR);
				break;
			case TRP:
				if ((tmp->flags & TRAP_SESS) == 0)
					return(GEN_ERR);
				break;
			case REQ:
			case NXT:
				if ((tmp->flags & (READ_ONLY|READ_WRITE)) == 0)
					return(GEN_ERR);
				break;
			case RSP:
				return(GEN_ERR);
			default:
				return(GEN_ERR);
		}
	}
	return(GEN_SUCCESS);
}

/*
 *  process a parsed SNMP message.  stick the reply packet in
 *  reppkt.  Return a code on completion. Really just a switching
 *  routine as all the work is done in msg type specific routine.
 *  modularity makes things easier on us.
 *  The server/agent should never receive a trap (TRP) or
 *  response (RSP) packet.  If we get one, let everyone know and
 *  return.
 */
int
proc_snmp_msg(snmp_msg, reppkt, msgtype)
	char *snmp_msg;
	char *reppkt;
	short msgtype;
{
  	getreq *getreqmsg;
  	getnext *getnxtmsg;
  	setreq *setreqmsg;
	int reqflg;

	switch (msgtype) {
		case NXT:
			reqflg = NXT;
			getnxtmsg = (getnext *)snmp_msg;
			s_stat.inhist[msgtype]++;
			return(procreq(getnxtmsg, reppkt, reqflg));
		case REQ:
			reqflg = REQ;
			getreqmsg = (getreq *)snmp_msg;
			s_stat.inhist[msgtype]++;
			return(procreq(getreqmsg, reppkt, reqflg));
		case SET:
			reqflg = SET;
			setreqmsg = (setreq *)snmp_msg;
			s_stat.inhist[msgtype]++;
			return(procset(setreqmsg, reppkt, reqflg));
		case RSP:
		case TRP:
			syslog(LOG_ERR, "SNMP msg type: %d received, not supported", msgtype);
			s_stat.inhist[msgtype]++;
			return(BUILD_ERR);
		default:
			syslog(LOG_ERR, "No such SNMP msg type: %d", msgtype);
			return(BUILD_ERR);
	}
}

/*
 *  process a GET-REQUEST and GET-NEXT message.  The reply is put in reppkt.
 *  On an error, the response to these messages is a GET-RESPONSE packet
 *  of identical form to the received packet with the error status
 *  fields set accordingly.
 */
int
procreq(getmsg, reppkt, flgs)
	getreq *getmsg;
	char *reppkt;
	int flgs;
{
	getrsp *replymsg;
	int cnt = 0, lencnt, err = 1, isnext = 0, bad = 0;
	int not_truncated_var;
	struct snmp_tree_node *treeptr;
  	u_long *c;
  	objident inst_ptr;
	int saveflgs = flgs;

	/*
	 *  Do we have a GET-NEXT packet?
	 */
	if (flgs == NXT)
		isnext = 1;

	replymsg = (getrsp *)reppkt;

	/*
	 *  Keep a running total of variables requested.
	 */
	s_stat.totreqvars += getmsg->varlist.len;

	/*
	 *  make sure we don't have too many variables to process.
	 *  If so, return an error!
	 */
	if (getmsg->varlist.len > SNMPMAXVARS) {
		syslog(LOG_ERR, "procreq: too many vars in msg, %d",
					getmsg->varlist.len);
		replymsg->errstat = GENERRS;
		replymsg->errindex = 1;
		return(BUILD_ERR);
	}

	/*
 	 *  if we are debugging, dump the packet.
	 */
	if (debuglevel > 2)
		(void) pr_pkt((char *)getmsg, flgs);

	/*
	 *  go through each variable in the variable list.
	 */
	while (cnt < getmsg->varlist.len) {
		/*
		 *  if the length of the variable name is <= 0,
		 *  we have a bad request.  Send back a NOSUCH (name).
		 */
		if (getmsg->varlist.elem[cnt].name.ncmp <= 0) {
			bad++;
			replymsg->errstat = NOSUCH;
			goto done;
		}
		/*
		 *  Initialize certain variables, skip over the
		 *  constant MIB prefix we all know and love.
		 */
		flgs = saveflgs;
		err = 1;
		not_truncated_var = 0;
		lencnt = MIB_PREFIX_SIZE + 1;
		treeptr = top;		/* point to top of tree */
		c = getmsg->varlist.elem[cnt].name.cmp + MIB_PREFIX_SIZE;

		/*
		 *  OVERALL GET STRATEGY:
		 *  For each variable, try to locate it in the tree by
		 *  using the variable name and stepping down until we
		 *  reach our destination.  If we hit a snag (variable is
		 *  not there or variable is bad) and the message is a
		 *  GET-NEXT, find the next variable (get_lex_next())
		 *  else (a GET-REQUEST) send back an error.  A GET-REQUEST
		 *  must match exactly including the object instance.
		 *
		 *  Once the variable is found, call the lookup function
		 *  to get the variable value.  If this fails and we have
		 *  a GET-NEXT, go back (goto) and find the next variable.
		 *  Keep repeating this until we find a valid variable
		 *  or fall of the end of the tree.  If we have a
		 *  GET-REQUEST, send back an error.
		 *
		 *  The lookup function is responsible for doing the
		 *  lexi-next *WITHIN THE OBJECT INSTANCE*.  For instance,
		 *  the routing tables.  It is also responsible for
		 *  filling in the variable name and value into the
		 *  variable bindings list of the reply message.
		 */
		while ((lencnt <= getmsg->varlist.elem[cnt].name.ncmp) &&
			((treeptr->flags & LEAF_NODE) == 0)) {
			if (treeptr->child[*c] != SNMPNODENULL)
				treeptr = treeptr->child[*c];
			else if (treeptr == top) { /* still at root, error! */
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}
			else {
				/*
				 *  if only a get request, return ERROR!
				 *  else find the next!
				 */
				if (flgs == REQ) {
					bad++;
					replymsg->errstat = NOSUCH;
					goto done;
				}
				else {
					lencnt++;
					isnext = 1;   /* I am paranoid */
					not_truncated_var = 1; /* got bad var */
					break;
				}
			}
			lencnt++;
			c++;
		}

		/*
		 *  If there is no object instance and we have a GET
		 *  request message, we return an error.  If we have a
		 *  GET-NEXT message this is ok.  If we have a get
		 *  next message, set the GET_LEX_NEXT flag.
		 */
		if ((lencnt - 1) == getmsg->varlist.elem[cnt].name.ncmp) {
			if (flgs == REQ) {  /* no obj. instance! */
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}
			if (isnext) {
				flgs = GET_LEX_NEXT;
			}
		}

		/*
 		 *  copy the object instance of the variable into the
	 	 *  appropriate place.  This will be used by the
		 *  specific lookup routine.
		 */
		bzero((char *)&inst_ptr, sizeof(inst_ptr));
		if (treeptr->flags & LEAF_NODE) {
			inst_ptr.ncmp = getmsg->varlist.elem[cnt].name.ncmp - (lencnt - 1);
			bcopy((char *)c, (char *)inst_ptr.cmp, sizeof(u_long) * inst_ptr.ncmp);
		}


		/*
		 *  If we have a GET-NEXT message that has been truncated.
		 *  We will find the next valid variable and try that.
		 *  If the variable is not truncated, it means that
		 *  a lookup routine for a valid variable has failed or
		 *  was non-existent.  In this case, we increase the variable
		 *  to the lexi-next variable  (increase the last byte by one)
		 *  And find the next variable from there.  Note that
		 *  get_lex_next(), when passed a valid variable, will return
		 *  the valid passed in variable as the valid next.
		 */
		if ((isnext != 0) && ((treeptr->flags & LEAF_NODE) == 0)) {
			if (flgs == REQ) {
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}
trynext:
			if (not_truncated_var) {
				/*
			 	 *  want to start looking for the next
				 *  var from the next node over on our
				 *  current level.  This insures that.
		 		 */
				while (((lencnt - 2) >= 0) && ((getmsg->varlist.elem[cnt].name.cmp[lencnt-2]) == MAXCHILDPERNODE)) {
					lencnt--;
				}
				if ((lencnt - 1) <= 0) {
					bad++;
					replymsg->errstat = NOSUCH;
					goto done;
				}
				getmsg->varlist.elem[cnt].name.cmp[lencnt-2]++;
				getmsg->varlist.elem[cnt].name.ncmp = lencnt-1;
			}

			/*
			 *  get the next!
			 */
			treeptr = get_lex_next(top,
					 getmsg->varlist.elem[cnt].name.cmp+MIB_PREFIX_SIZE,
					 getmsg->varlist.elem[cnt].name.ncmp - MIB_PREFIX_SIZE);

			/*
			 *  End of tree!  No variable.
			 */
			if (treeptr == SNMPNODENULL) {
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}

			flgs = GET_LEX_NEXT;
			bzero((char *)&inst_ptr, sizeof(inst_ptr));
		}
	
		/*
		 *  We finally have our answer.  Call the function which
		 *  gets our answer.  The appropriate values will be
		 *  stashed into the reply message.  If not, an error
		 *  code is returned.
		 */
		if (treeptr->getval != NULL) {
			err = (*(treeptr->getval))(treeptr, &(replymsg->varlist.elem[cnt]), &inst_ptr, flgs);
		}

		/*
		 *  The lookup function returned an error.  if we have a
		 *  GET-REQUEST, return an error.  If we have a GET-NEXT,
		 *  find the next valid variable.  goto trynext and give
		 *  it a whirl again.  We copy the variable we just tried
		 *  into the getmsg variable binding list.  In other words,
		 *  We always keep the last variable we tried in the
		 *  variable bindings list.
		 */
		if ((treeptr->getval == NULL) || (err <= 0)) {
			/*
			 *  if only a get request, return ERROR!
			 *  else find the next!
			 */
			if (flgs == REQ) {
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}
			else {  /* get the NEXT! */
				bcopy((char *)treeptr->var_code,
	 				(char *)&getmsg->varlist.elem[cnt].name,
	 				sizeof(getmsg->varlist.elem[cnt].name));
				lencnt = getmsg->varlist.elem[cnt].name.ncmp+1;
				not_truncated_var = 1;

				goto trynext;
			}
		}
		cnt++;	/*  on to next variable  */
	}		/*  while there are other variables */
done:
	/*
	 *  If we had some sort of error, fill in the error fields and
	 *  ship it back as an error.  Return an error code.
	 *  snmpin() will eventually use the error index and status
	 *  fields in turning around the packet when an error occurs.
	 *
	 *  If not, the packet has been partially filled in.
	 *  fill in the remaining reply message header.  send back the
	 *  same request id.
	 */
	if (bad != 0) {
		replymsg->errindex = cnt + 1;
		return(BUILD_ERR);
	}
	else {
		replymsg->reqid = getmsg->reqid;
		replymsg->errstat = NOERR;
		replymsg->errindex = 0;
		replymsg->varlist.len = cnt;
		return (BUILD_SUCCESS);
	}
}

/*
 *  gets the next supported lexocographical tree value from the
 *  passed variable code.  "ch" points to the variable code.
 *  If none exists, a NULL is returned.
 *
 *  The strategy here is to recurse down to the tree node of the
 *  passed in variable code and then take off on a tree traversal
 *  to find the next valid variable.  If we get to the last node in
 *  the tree, we send back NULL.  I should clarify that we are looking
 *  for the lexicographically next valid variable in the tree from
 *  the passed in variable *until the end of the tree*.  We do not
 *  wrap around to the beginning of the tree.
 */
struct snmp_tree_node *
get_lex_next(parentptr, ch, varsz)
	struct snmp_tree_node *parentptr;
	u_long *ch;
	int varsz;
{
	int cnt;
	u_long x;
	struct snmp_tree_node *tmpptr;
	u_long *c;

	/*
	 *  as we recurse down, "x" will be set to the next byte
	 *  of the variable code.  Each time we recurse, we call
	 *  the function with the first byte stripped off of the
	 *  variable code from the previous call.  If we reach the
	 *  end of the variable code string and we are not yet
	 *  at a leaf, we received a truncated variable, start at
	 *  the beginning of this branch (set x = 1).
	 */
	if ((ch == NULL) || (varsz <= 0)) {
		c = NULL;
		x = 1;
		varsz = 0;
	}
	else {
		x = *ch;
		c = ch + 1;	/* next function call will use this */
		varsz--;
	}
	/*
	 *  are we at a leaf?  If there is a valid value there,
	 *  return it, back out of recursion.  If not, return
	 *  NULL, search will continue.
	 */
	if (parentptr->flags & LEAF_NODE) {
		if (parentptr->flags & NOT_AVAIL)
			return(SNMPNODENULL);
		return(parentptr);
	}
	/*
	 *  if possible, recurse down into tree.  We start from "x"
	 *  as we only want to search the tree starting from the
	 *  passed in variable code.  Recursion works nicely
	 *  here as an "x" will be saved at each level of recursion
	 *  as we go to the node of the passed in variable code.  We
	 *  will then start the tree traversal from that point.
	 */
	for (cnt = x; cnt <= MAXCHILDPERNODE; cnt++) {
		if (parentptr->child[cnt] != SNMPNODENULL) {
			tmpptr = get_lex_next(parentptr->child[cnt], c, varsz);
			if (tmpptr != SNMPNODENULL)  /* found an answer! */
				return(tmpptr);
		}
		c = NULL;
	}
	return(SNMPNODENULL);
}

/*
 *  This function processes a SET-REQUEST packet.  Much like the
 *  GET-REQUEST code, except that you really can't process the
 *  sets (actually perform the set) until you make sure the entire
 *  SET-REQUEST is valid (all variables).  Then you perform the
 *  actual set(s).   Atomicness, I guess.
 * 
 *  Note that the response to a SET-REQUEST packet is a GET-RESPONSE
 *  packet of identical form to the SET-REQUEST except the error status
 *  and index are set accordingly.  Therefore, reppkt is only used to pass
 *  back the error status and index values up to snmpin() where the
 *  response to the set is fixed up (just a switch of ptrs!).
 *
 *  Error status for a sucessful set is NOERR (0), index is 0.
 */
int
procset(setmsg, reppkt, flgs)
	setreq *setmsg;
	char *reppkt;
	int flgs;
{
	getrsp *replymsg;
	int cnt = 0, lencnt, bad = 0, setvaltype;
	int errstatus = NOERR;
	struct snmp_tree_node *treeptr;
  	u_long *c;
	struct set_struct *tail, *settmp;

	replymsg = (getrsp *)reppkt;
	setlst = NULL;
	tail = NULL;

	/*
	 *  Keep a running total of variables requested to be set.
	 */
	s_stat.totsetvars += setmsg->varlist.len;

	/*
	 *  make sure we don't have too many variables to process.
	 *  If so, return an error!
	 */
	if (setmsg->varlist.len > SNMPMAXVARS) {
		syslog(LOG_ERR, "procset: too many vars in msg, %d",
					setmsg->varlist.len);
		replymsg->errstat = GENERRS;
		replymsg->errindex = 1;
		return(BUILD_ERR);
	}

	/*
 	 *  if we are debugging, dump the packet.
	 */
	if (debuglevel > 2)
		(void) pr_pkt((char *)setmsg, flgs);

	/*
	 *  go through each variable in the variable list.
	 */
	while (cnt < setmsg->varlist.len) {
		/*
		 *  if the length of the variable name is <= 0,
		 *  we have a bad request.  Send back a NOSUCH (name).
		 */
		if (setmsg->varlist.elem[cnt].name.ncmp <= 0) {
			bad++;
			replymsg->errstat = NOSUCH;
			goto done;
		}

		/*
		 * initialize variables.  Also, skip over the
		 * constant mib variable prefix we all know and love.
		 */
		lencnt = MIB_PREFIX_SIZE + 1;
		treeptr = top;		/* point to top of tree */
		c = setmsg->varlist.elem[cnt].name.cmp + MIB_PREFIX_SIZE;

		/*
		 *  OVERALL SET STRATEGY:
		 *  set up the values to perform the request,  first
		 *  find a pointer to the proper variable tree node.
		 *  then fill the appropriate information into the
		 *  set list.  Because of atomic sets, you must go through
		 *  and make sure each variable in the packet is
		 *  a valid set request.  Once this is done, do_all_sets()
		 *  takes the set list and performs the sets "... as if
		 *  performed simultaneously".  If there is any error, all
		 *  the sets which have been finished must be undone and
		 *  an error packet must be returned.
		 *
		 *  The variables in the set-request packet must match
		 *  exactly!  Much like the get-request.  The code is
		 *  almost identical.  If no good, return NoSuch error status.
		 */
		while ((lencnt <= setmsg->varlist.elem[cnt].name.ncmp) &&
			((treeptr->flags & LEAF_NODE) == 0)) {
			if (treeptr->child[*c] != SNMPNODENULL)
				treeptr = treeptr->child[*c];
			else {
				/*
				 *  Bad "set" variable, return ERROR!
				 *  NO LEX-NEXT on sets!!!!!
				 */
				bad++;
				replymsg->errstat = NOSUCH;
				goto done;
			}
			lencnt++;
			c++;
		}

		/*
		 *  If there is no object instance, we return an error.
		 */
		if ((lencnt - 1) == setmsg->varlist.elem[cnt].name.ncmp) {
			bad++;
			replymsg->errstat = NOSUCH;
			goto done;
		}

		/*
		 *  Is it a writable variable?  if not, punt.
		 *  Condition (1) in set-request description in SNMP RFC.
		 */
		if ((treeptr->flags & WRITE_VAR) == 0) {
			bad++;
			replymsg->errstat = NOSUCH;
			goto done;
		}

		/*
		 *  Is the set value the right type?  If not,
		 *  send back BadValue.  Condition (2) in set-request
		 *  description in SNMP RFC.
		 */
		switch (setmsg->varlist.elem[cnt].val.type) {
			case INT:    setvaltype = VAL_INT;    break;
			case STR:    setvaltype = VAL_STR;    break;
			case OBJ:    setvaltype = VAL_OBJ;    break;
			case EMPTY:  setvaltype = VAL_EMPTY;  break;
			case IPADD:  setvaltype = VAL_IPADD;  break;
			case CNTR:   setvaltype = VAL_CNTR;   break;
			case GAUGE:  setvaltype = VAL_GAUGE;  break;
			case TIME:   setvaltype = VAL_TIME;   break;
			case OPAQUE: setvaltype = VAL_OPAQUE; break;
			default:
				bad++;
				replymsg->errstat = BADVAL;
				goto done;
		}
		if ((VAL_MASK & treeptr->flags) != setvaltype) {
			bad++;
			replymsg->errstat = BADVAL;
			goto done;
		}

		/*
		 *  This variable passes (so far).  We can now add it to
		 *  the set list and go back and check another variable in
		 *  the packet.  We will do all the sets when every variable
		 *  has passed the above criteria.
		 *
		 *  First we will make a new list node, then we will add
		 *  it to the end of the list.  If we experience any error
		 *  in doing this, we will return a GenErr.
		 *  Remember to free up this memory after usage!
		 */
		settmp = (struct set_struct *)malloc(sizeof(struct set_struct));
		if (settmp == (struct set_struct *)NULL) {
			syslog(LOG_ERR, "procset: malloc: %m");
			replymsg->errstat = GENERRS;
			(void) set_struct_free(tail);
			bad++;
			goto done;
		}

		/*
		 *  Fill in the structure.
		 */
		settmp->tptr = treeptr;
		settmp->ob_inst.ncmp = setmsg->varlist.elem[cnt].name.ncmp-(lencnt-1);
		bcopy((char *)c,(char *)settmp->ob_inst.cmp,sizeof(u_long)*settmp->ob_inst.ncmp);
		bcopy((char *)&setmsg->varlist.elem[cnt].val,(char *)&settmp->setv,sizeof(objval));
		settmp->next = (struct set_struct *)NULL;
		settmp->back = (struct set_struct *)NULL;

		/*
		 *  Add to the list.
		 */
		if (setlst == (struct set_struct *)NULL) {  /* first one */
			setlst = settmp;
			tail = setlst;
		}
		else {  /* add it to the end */
			tail->next = settmp;
			settmp->back = tail;
			tail = tail->next;
		}

		cnt++;	/*  on to next variable  */
	}		/*  while there are other variables */

	/*
	 *  Went through the variable list and all is ok.  Take
	 *  the set list and actually perform the sets.  If we have any
	 *  problems doing this, then we must go back and undo all the
	 *  sets we have already done. do_all_sets() takes care of
	 *  undoing the set variables.  Once this is accomplished, send
	 *  back a GenErr status.  If no errors, great!  We then send
	 *  back identical packet with NoErr status.
	 */
	if (do_all_sets(setlst, &errstatus) < 0) {
		bad++;
		replymsg->errstat = errstatus;
	}
done:
	/*
	 *  If we had some sort of error, fill in the error fields and
	 *  ship it back as an error.  Return an error code.
	 *  snmpin() will eventually use the error index and status
	 *  fields in turning around the packet when an error occurs.
	 *
	 *  If not, the packet has been partially filled in.
	 *  fill in the remaining reply message header.  send back the
	 *  same request id.
	 *
	 *  free up memory from the set list before returning.
	 */
	(void) set_struct_free(tail);

	if (bad != 0) {
		replymsg->errindex = cnt + 1;
		return(BUILD_ERR);
	}
	else {
		replymsg->errstat = NOERR;
		replymsg->errindex = 0;
		return (BUILD_SUCCESS);
	}
}

/*
 *  Take a list of sets and perform them.  If there is a problem with
 *  any of the sets, then go back and reset the values which were
 *  previously set to their original values.  Fill in errcond with the
 *  correct error code.
 */
int
do_all_sets(worklist, errcond)
	struct set_struct *worklist;
	int *errcond;
{
	struct set_struct *wltmp = worklist;
	int ers;

	/*
	 *  Traverse set list until end.
	 *  Call the variable set function (if present).
	 *  If we have some sort of error, go bak and reset values!
	 *  Old values are saved in the "set_struct".
	 */
	while (wltmp != (struct set_struct *)NULL) {
		if (wltmp->tptr->setval != NULL) {
			*errcond = (*(wltmp->tptr->setval))(wltmp, SETNEW,&ers);
		}
		if ((wltmp->tptr->setval == NULL) || (*errcond < 0)) {
			while (wltmp != setlst) {  /* Go back and reset! */
				wltmp = wltmp->back;
				(*(wltmp->tptr->setval))(wltmp, SETOLD, &ers);
			}
			*errcond = ers;
			return(BUILD_ERR);
		}
		wltmp = wltmp->next;
	}
	*errcond = NOERR;
	return(BUILD_SUCCESS);
}

/*
 *  Free up memory that has been allocated for the set list.
 *  sslist is always the tail of the list.
 */
set_struct_free(sslist)
	struct set_struct *sslist;
{
	/*
	 *  We have a NULL list to free, get outta here!
	 */
	if (sslist == (struct set_struct *)NULL)
		return;

	/*
	 *  We were passed in the tail of the set list.
	 *  Go backwards, freeing up memory allocated for
	 *  each of the list nodes.
	 */
	while (sslist != setlst) {
		sslist = sslist->back;
		free((char *)sslist->next);
	}

	/*
	 *  We are now at the head of the list.  Free it up and set
	 *  head to NULL.
	 */
	free((char *)sslist);
	setlst = NULL;
}

