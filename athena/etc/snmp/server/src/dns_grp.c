/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the DNS portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 14 November 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/dns_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/dns_grp.c,v 2.0 1992-04-22 02:03:42 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef DNS

static int get_dns_value();
static FILE *fp;


/*
 * Function:    lu_dns()
 * Description: Top level callback. Supports the following:
 *                  
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_dns(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */


  if((fp = fopen(dns_stat_file, "r")) == (FILE *) NULL)
    return(BUILD_ERR);  
  
  /*
   * the deal here is that we are parsing the stats file dumped by named.
   */

  switch(varnode->offset)
    {
    case N_DNSSTATFILE:
      fclose(fp);
      return(make_str(&(repl->val), dns_stat_file));
    case N_DNSUPDATETIME:
      fclose(fp);
      return(make_str(&(repl->val), stattime(dns_stat_file)));
    case N_DNSBOOTTIME:
      repl->val.type = TIME;
      repl->val.value.cntr = get_dns_value("time since boot");
      break;
    case N_DNSRESETTIME:
      repl->val.type = TIME;
      repl->val.value.cntr = get_dns_value("time since reset");
      break;
    case N_DNSPACKETIN:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("input packets");
      break;
    case N_DNSPACKETOUT:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("output packets");
      break;
    case N_DNSQUERY:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("queries");
      break;
    case N_DNSIQUERY:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("iqueries");
      break;
    case N_DNSDUPQUERY:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("duplicate queries");
      break;
    case N_DNSRESPONSE:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("responses");
      break;
    case N_DNSDUPRESP:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("duplicate responses");
      break;
    case N_DNSOK:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("OK answers");
      break;
    case N_DNSFAIL:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("FAIL answers");
      break;
    case N_DNSFORMERR:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("FORMERR answers");
      break;
    case N_DNSSYSQUERY:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("system queries");
      break;
    case N_DNSPRIMECACHE:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("prime cache calls");
      break;
    case N_DNSCHECKNS:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("check_ns calls");
      break;
    case N_DNSBADRESP:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("bad responses dropped");
      break;
    case N_DNSMARTIAN:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("martian responses");
      break;
    case N_DNSUNKNOWN:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("Unknown query types");
      break;
    case N_DNSA:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("A queries");
      break;
    case N_DNSNS:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("NS queries");
      break;
    case N_DNSCNAME:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("CNAME queries");
      break;
    case N_DNSSOA:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("SOA queries");
      break;
    case N_DNSWKS:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("WKS queries");
      break;
    case N_DNSPTR:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("PTR queries");
      break;
    case N_DNSHINFO:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("HINFO queries");
      break;
    case N_DNSMX:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("MX queries");
      break;
    case N_DNSAXFR:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("AXFR queries");
      break;
    case N_DNSANY:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("ANY queries");
      break;
    case N_DNSTXT:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("TXT queries");
      break;
    case N_DNSUNSPECA:
      repl->val.type = CNTR;
      repl->val.value.cntr = get_dns_value("UNSPECA queries");
      break;
    default:
      syslog (LOG_ERR, "lu_afs: bad offset: %d", varnode->offset);
      fclose(fp);
      return(BUILD_ERR);
    }

  fclose(fp);
  return(BUILD_SUCCESS);
}




static int
get_dns_value(label)
     char *label;
{
  char *c;
  int n;

  while(fgets(lbuf, sizeof(lbuf) - 1, fp) != (char *) NULL)
    {
      c = lbuf;
      while((*c != ' ') && (*c != '\t') && (*c != '\n') && (*c != '\0'))
	++c;
      if((*c == '\0') || (*c == '\n'))
	continue;
      *c++ = '\0';
      while(!isascii(*c))
	++c;
      if(strncmp(label, c, strlen(label)) != 0)
	continue;
      n = atoi(lbuf);
      return(n);
    }
  return(0);
}


#endif DNS
#endif MIT
