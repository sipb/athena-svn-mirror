#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/config.c,v 2.3 1997-02-27 06:47:22 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 2.2  1994/08/15 15:04:10  cfields
 * Define NGROUPS for Solaris.
 * 7.7 checkin; changes by vrt
 *
 * Revision 2.1  93/06/18  14:32:32  tom
 * first cut at solaris port
 * 
 * Revision 2.0  92/04/22  01:49:35  tom
 * release 7.4
 * 	allowed specification of a domain name in snmpd.conf
 * 
 * Revision 1.3  92/04/18  19:19:57  tom
 * *** empty log message ***
 * 
 * Revision 1.2  90/05/26  13:35:06  tom
 * release 7.0e
 * 
 * Revision 1.1  90/04/26  15:33:45  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:29  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/config.c,v 2.3 1997-02-27 06:47:22 ghudson Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988.
 */
/*
 *  This file contains specific utilities encompassing
 *  the reading and initialization of values read from
 *  the configuration file.
 */

#include "include.h"

#ifdef SOLARIS
#include <limits.h>
#define NGROUPS NGROUPS_MAX
#endif

/*
 *  This file reads values from the config file and initializes the
 *  appropriate snmpd variables.  For example, the communities and
 *  certain variables that must be supported statically.
 *  return error code on having problems.
 */
int
parse_config(fd)
	FILE *fd;
{
	char buf[BUFSIZ];
	char keyword[SNMPMXSID];
	char name[SNMPMXSID];
	char var[SNMPMXSID];
	char val[SNMPMXSID];
	int line = 0, error = FALSE, i;

	/*
	 *  Fill in the default GW ID and GW REVision.  Specified in
	 *  config.h.  If the version and rev is specified in the
	 *  snmpd.conf file, we will prefer that and use it.
	 *  Ditto for the Operating system type/revision.
	 *  Enable the sending of authentication failure traps.
	 *  They might be disabled later on if specified in the
	 *  configuration file.
	 */

#ifdef MIT
	/*
         * we'll do things a bit differently. 
	 */

	memset(gw_version_id, 0, sizeof(gw_version_id));
	strcpy(mail_q,            MAIL_Q);
	strcpy(mail_alias_file,   MAIL_ALIAS_FILE);
	strcpy(rc_file,           RC_FILE);
	strcpy(rpc_cred_file,     RPC_CRED_FILE);
	strcpy(afs_cache_file,    AFS_CACHE_FILE);
	strcpy(afs_suid_file,     AFS_SUID_FILE);
	strcpy(afs_cell_file,     AFS_CELL_FILE);
	strcpy(afs_cellserv_file, AFS_CELLSRV_FILE);
	strcpy(login_file,        LOGIN_FILE);
	strcpy(version_file,      VERSION_FILE);
	strcpy(syspack_file,      SYSPACK_FILE);
	strcpy(srv_file,          SRV_FILE);
	strcpy(dns_stat_file,     DNS_STAT_FILE);
	strcpy(user,              USER);
	strcpy(weather_tty,       WEATHER_TTY);

#else  MIT
	(void) strcpy(gw_version_id, SYS_DESCR);
#endif MIT

	send_authen_traps = 1;
	tcprtoalg = -1;
	for (i = 0; i < MAXIFS; i++)
		adminstat[i] = OP_NORMAL;

	rewind(fd);

	/*
	 *  Keep reading the config file line by line until
	 *  the end!
	 */
	while (fgets(buf, sizeof(buf), fd) != NULL) {
		line++;
		if ((buf[0] == '#') || (buf[0] == '\n'))
			continue;
		if (sscanf(buf, "%s", keyword) != 1)
			continue;
		if (strcmp(keyword, "interface") == 0) {
			if (sscanf(buf, "%*s %s %s %s", var, name, val) != 3) {
				syslog(LOG_ERR, "config syntax error, line %d",
						line);
				error = TRUE;
			}
			else if ((strcmp(var, "type") != 0) &&
			         (strcmp(var, "speed") != 0)) {
				syslog(LOG_ERR,"invalid interface var, line %d",
						line);
				error = TRUE;
			}
			else {
				if (add_intf_var(var, name, val) < 0) {
					syslog(LOG_ERR,"error in adding intf val, line %d", line);
					error = TRUE;
				}
			}
		}
		else if (strcmp(keyword, "community") == 0) {
			if (sscanf(buf, "%*s %s %s %s", name, var, val) != 3) {
				syslog(LOG_ERR, "config syntax error, line %d",
						line);
				error = TRUE;
			}
			else if (add_sess(name, var, val) < 0) {
				syslog(LOG_ERR,"couldn't add community,line %d",
						line);
				error = TRUE;
			}
		}
		else if (strcmp(keyword, "sysdescr") == 0) {
			if (sscanf(buf, "%*s %s", name) != 1) {
				syslog(LOG_ERR, "config syntax error, line %d",
						line);
				error = TRUE;
			}
			else
			  (void) strncpy(gw_version_id, name, 
					 sizeof(gw_version_id));
		}
		else if (strcmp(keyword, "tcprtoalgorithm") == 0) {
			if (sscanf(buf, "%*s %s", name) != 1) {
				syslog(LOG_ERR, "config syntax error, line %d",
						line);
				error = TRUE;
			}
			else
				tcprtoalg = atoi(name);
		}
		else if (strcmp(keyword, "no_authen_traps") == 0) {
			send_authen_traps = 0;
		}
#ifdef MIT
		else if (strcmp(keyword, "suppsysdescr") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  strncpy(supp_sysdescr, name, sizeof(supp_sysdescr));
	        }
		else if (strcmp(keyword, "mailq") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  strncpy(mail_q, name, sizeof(mail_q));
	        }
		else if (strcmp(keyword, "rcfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  strncpy(rc_file, name, sizeof(rc_file));
	        }
		else if (strcmp(keyword, "rpccredfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  strncpy(rpc_cred_file, name, sizeof(rpc_cred_file));
	        }
		else if (strcmp(keyword, "afscachefile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			 strncpy(afs_cache_file, name, sizeof(afs_cache_file));
	        }
		else if (strcmp(keyword, "afssuidfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  strncpy(afs_suid_file, name, sizeof(afs_suid_file));
	        }
		else if (strcmp(keyword, "afscellfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			 strncpy(afs_cell_file, name, 
				 sizeof(afs_cell_file) - 1);
	        }
		else if (strcmp(keyword, "afscellservfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(afs_cellserv_file, name, 
			       sizeof(afs_cellserv_file));
	        }
		else if (strcmp(keyword, "versionfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(version_file, name, sizeof(version_file));
	        }
		else if (strcmp(keyword, "syspackfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(syspack_file, name, sizeof(syspack_file));
	        }
		else if (strcmp(keyword, "srvfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(srv_file, name, sizeof(srv_file));
	        }
		else if (strcmp(keyword, "dnsstatfile") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(dns_stat_file, name, sizeof(dns_stat_file));
	        }
		else if (strcmp(keyword, "group") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       add_group(name);
	        }
		else if (strcmp(keyword, "user") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(user, name, sizeof(user));
	        }	
		else if (strcmp(keyword, "wtty") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(weather_tty, name, sizeof(weather_tty));
	        }	
		else if (strcmp(keyword, "wlocation") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
		       strncpy(weather_location, name, sizeof(user));
	        }	
		else if (strcmp(keyword, "logintrap") == 0) {
		        if (sscanf(buf, "%*s %s", name) != 1) {
			        syslog(LOG_ERR, "config syntax error, line %d",
				               line);
				error = TRUE;
			}
			else
			  logintrap = atoi(name);
	        }		
#endif MIT
	} /* end while */
	(void) fclose(fd);
	if (error == TRUE) {
		syslog(LOG_ERR, "config %s file parse error", SNMPINITFILE);
		return(GEN_ERR);
	}
	return(BUILD_SUCCESS);
}

/*
 *  add the interface values passed in to the static interface value
 *  list.  Return an error code on spotting any difficulties.
 */
int
add_intf_var(vartype, nam, value)
	char *vartype;
	char *nam;
	char *value;
{
	int errcode = 1;
	struct intf_info *tmp, *tmp1;

	/*
	 *  do some debugging if applicable
	 */
	if (debuglevel > 1) {
		(void) printf("interface %s, %s is %s\n", nam, vartype, value);
		(void) fflush(stdout);
	}

	/*
	 *  look for the interface name in the list to see if it
	 *  already exists. If not, add it to the end.  If it is
	 *  found, place the value in the proper field.  When making
	 *  a new entry for the list, initialize all value fields to
	 *  -1.  This was, if we are queried for this variable and it
	 *  is -1, we know it is not supported and return a get_next. 
	 */
	tmp1 = iilst;
	while ((tmp1 != (struct intf_info *)NULL) &&
		(strcmp(tmp1->name, nam) != 0))
		tmp1 = tmp1->next;

	if (tmp1 != (struct intf_info *)NULL) {
		if (strcmp(vartype, "type") == 0)
			tmp1->itype = atoi(value);
		else if (strcmp(vartype, "speed") == 0)
			tmp1->speed = atoi(value);
		else {
			syslog(LOG_ERR, "unsupported intf var type %s",vartype);
			errcode = GEN_ERR;
		}
	}
	else {
		tmp = (struct intf_info *)malloc(sizeof(struct intf_info));
		if (tmp == (struct intf_info *)NULL) {
			syslog(LOG_ERR, "add_intf_var: malloc: %m");
			return(GEN_ERR);
		}
		tmp->next = (struct intf_info *)NULL;
		tmp->itype = -1;
		tmp->speed = -1;
		(void) strcpy(tmp->name, nam);
		if (strcmp(vartype, "type") == 0)
			tmp->itype = atoi(value);
		else if (strcmp(vartype, "speed") == 0)
			tmp->speed = atoi(value);
		else {
			syslog(LOG_ERR, "unsupported intf var type %s",vartype);
			errcode = GEN_ERR;
		}
		if (iilst == (struct intf_info *)NULL)  /* first entry */
			iilst = tmp;
		else {
			tmp1 = iilst;
			while (tmp1->next != (struct intf_info *)NULL)
				tmp1 = tmp1->next;
			tmp1->next = tmp;
		}
			
	}
	return(errcode);
}

/*
 *  add a community to the static community list.  Return an error code
 *  when there is difficulty.
 */
int
add_sess(nam, saddr, flgs)
	char *nam;
	char *saddr;
	char *flgs;
{
	struct snmp_session *tmp, *tmp1;
	struct in_addr haddr;
	int founduser = 0;
	struct inaddrlst *adlist;
#ifdef MIT
	struct hostent *hp = (struct hostent *) NULL;
	unsigned long laddr = 0;
#endif /* MIT */

	if (debuglevel > 1) {
		(void) printf("community %s with %s, flags: %s\n", nam, saddr, flgs);
		(void) fflush(stdout);
	}

#ifdef MIT
	/*
	 * Allow domain name service for community addresses.
	 * For simplicity we'll assume that hostnames begin
	 * with a non digit.
	 */
	haddr.s_addr = 0;
	if(!isdigit(*saddr))
	  if(hp = gethostbyname(saddr))
	    memcpy(&(haddr.s_addr), hp->h_addr, hp->h_length);
#endif /* MIT */

	/*
	 *  see if the community has been defined already.  If so,
	 *  check and see if the address has already been defined
	 *  for this community.  If it has, print out a warning message
	 *  and exit ok. If not, add the address to the address list of
	 *  the community.  If the community has not been defined yet, add
	 *  the community to the community list.  Set the flags, name,
	 *  and internet address of the community.
	 */
	tmp1 = sessions;
	while ((tmp1 != (struct snmp_session *)NULL) &&
		(strcmp(tmp1->name, nam) != 0))
		tmp1 = tmp1->next;

	if (tmp1 != (struct snmp_session *)NULL) {  /* defined already! */
#ifdef MIT
	    if(!haddr.s_addr)
#endif /* MIT */
		if ((haddr.s_addr = inet_addr(saddr)) < (u_long)0) {
			syslog(LOG_ERR,"add_sess: malformed address: %s", 
			       saddr);
			return(GEN_ERR);
		}
		/*
		 *  if it is a trap community, we can't have a
		 *  wildcard (0.0.0.0) address. Don't add the
		 *  address.
		 */
		if ((haddr.s_addr ==  0) && (tmp1->flags == TRAP_SESS)) {
			syslog(LOG_ERR,"add_sess: bad trap addr: %s, community: %s",saddr, tmp1->name);
			return(GEN_ERR);
		}
		adlist = tmp1->userlst;
		while (adlist != (struct inaddrlst *)NULL) {
			if (haddr.s_addr == adlist->sess_addr.s_addr) {
				founduser++;
				break;
			}
			if (adlist->nxt == (struct inaddrlst *)NULL)
				break;
			else
				adlist = adlist->nxt;
		}
		/*
		 *  if address in list, send a warning, else add it to
		 *  the list.
		 */
		if ((founduser == 0) && (adlist != (struct inaddrlst *)NULL)) {
			adlist->nxt = (struct inaddrlst *)malloc(sizeof(struct inaddrlst));
			if (adlist->nxt == (struct inaddrlst *)NULL) {
				syslog(LOG_ERR, "add_sess: malloc for userlst: %m");
				return(GEN_ERR);
			}
			adlist = adlist->nxt;
			adlist->sess_addr.s_addr = haddr.s_addr;
			adlist->nxt = (struct inaddrlst *)NULL;
		}
		else
			syslog(LOG_ERR, "community %s already defined.", nam);
	}
	else {  /* community not defined, add it */
		tmp = (struct snmp_session *)malloc(sizeof(struct snmp_session));
		if (tmp == (struct snmp_session *)NULL) {
			syslog(LOG_ERR, "add_sess: malloc: %m");
			return(GEN_ERR);
		}

		if (strcmp(flgs, "read-only") == 0)
			tmp->flags = READ_ONLY;
		else if (strcmp(flgs, "read-write") == 0)
			tmp->flags = READ_WRITE;
		else if (strcmp(flgs, "traps") == 0)
			tmp->flags = TRAP_SESS;
		else {
			syslog(LOG_ERR, "add_sess: bad community flag: %s", flgs);
			(void) free((char *)tmp);
			return(GEN_ERR);
		}

		tmp->userlst = (struct inaddrlst *)malloc(sizeof(struct inaddrlst));
		if (tmp->userlst == (struct inaddrlst *)NULL) {
			syslog(LOG_ERR, "add_sess: malloc for userlst: %m");
			(void) free((char *)tmp);
			return(GEN_ERR);
		}
		tmp->next = (struct snmp_session *)NULL;
		(void) strcpy(tmp->name, nam);
#ifdef MIT
		if(haddr.s_addr)
		      tmp->userlst->sess_addr.s_addr = haddr.s_addr;
		else
#endif /* MIT */	
		if ((tmp->userlst->sess_addr.s_addr = inet_addr(saddr)) < (u_long)0) {
			syslog(LOG_ERR,"add_sess: malformed address: %s",saddr);
			(void) free((char *)tmp->userlst);
			(void) free((char *)tmp);
			return(GEN_ERR);
		}
		/*
		 *  if it is a trap community, we can't have a
		 *  wildcard (0.0.0.0) address. Don't add the
		 *  community.
		 */
		if ((tmp->userlst->sess_addr.s_addr ==  0) &&
		    (tmp->flags == TRAP_SESS)) {
			syslog(LOG_ERR,"add_sess: bad trap addr: %s, community: %s",saddr, tmp->name);
			(void) free((char *)tmp->userlst);
			(void) free((char *)tmp);
			return(GEN_ERR);
		}
		tmp->userlst->nxt = (struct inaddrlst *)NULL;

		if (sessions == (struct snmp_session *)NULL)  /* first entry */
			sessions = tmp;
		else {
			tmp1 = sessions;
			while (tmp1->next != (struct snmp_session *)NULL)
				tmp1 = tmp1->next;
			tmp1->next = tmp;
		}
			
	}
	return(GEN_SUCCESS);
}


#ifdef MIT

#include <grp.h>

int
add_group(name)
     char *name;
{
  struct group *g;
  int set[NGROUPS];
  int gid;
  int len;

  if((len = getgroups(NGROUPS, set)) < 0)
    {
      syslog(LOG_ERR, "getgroups failed");
      return(GEN_ERR);
    }

  if(len == NGROUPS)
    {
      syslog(LOG_ERR, "too many groups!");
      return(GEN_ERR);
    }

  if((g = getgrnam(name)) == (struct group *) NULL)
    {
      syslog(LOG_ERR, "unknown group \"%s\".", name);
      return(GEN_ERR);
    }

  gid = g->gr_gid;
  set[len++] = gid;
  if(setgroups(len, set) < 0)
    {
      syslog(LOG_ERR, "unable to set gid to \"%s\".", name);
      return(GEN_ERR);
    }
  
  return(GEN_SUCCESS);
}

#endif MIT
