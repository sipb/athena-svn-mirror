/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the AFS (Andrew File System) portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/afs_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 * Revision 1.6  90/07/17  14:16:53  tom
 * error in cast
 * 
 * Revision 1.5  90/06/05  15:24:20  tom
 * lbuf is static
 * 
 * Revision 1.4  90/05/26  13:34:28  tom
 * release 7.0e
 * 
 * Revision 1.3  90/04/26  15:24:09  tom
 * *** empty log message ***
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/afs_grp.c,v 2.0 1992-04-22 01:48:49 tom Exp $";
#endif

#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef AFS

static char *afs_this_cell();
static char *get_afs_db();
static char *get_afs_suid_cell();
static int crock_cachesize();

/*
 * Function:    lu_afs()
 * Description: Top level callback. Supports the following:
 *                  N_AFSCACHESIZE- (INT) afs cache size
 * Notes:       Because the cell names are so long, listings of cell names
 *              are indexed with integers instead of the cell name itself.
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_afs(varnode, repl, instptr, reqflg)
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

  switch(varnode->offset)
    {
    case N_AFSCACHESIZE:
      repl->val.type = INT;
      repl->val.value.intgr = crock_cachesize();
      return(BUILD_SUCCESS);
    case N_AFSCACHEFILE:
      return(make_str(&(repl->val), afs_cache_file));
    case N_AFSTHISCELL:
      return(make_str(&(repl->val), afs_this_cell()));
    case N_AFSCELLFILE:
      return(make_str(&(repl->val), afs_cell_file));
    case N_AFSSUIDFILE:      
      return(make_str(&(repl->val), afs_suid_file));
    case N_AFSSRVFILE:
      return(make_str(&(repl->val), afs_cellserv_file));
    default:
      syslog (LOG_ERR, "lu_afs: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


/*
 * Function:    lu_afsdb()
 * Description: 
 * Returns:     BUILD_ERR/BUILD_SUCCESS
 */

int
lu_afsdb(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  long tmp;
  char *str;
  int i = 0;
  int num;
  int first = 0;

  if (varnode->flags & NOT_AVAIL || varnode->offset <= 0)
    return(BUILD_ERR);

  bcopy ((char *)varnode->var_code, (char *) &repl->name,
	 sizeof(repl->name));
  
  switch(varnode->offset)
    {
    case N_AFSSUIDCELL:
      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	{
	  num = 1;
	  first = 1;
	}
      else
	num = instptr->cmp[0];
      
      if((reqflg & (NXT|GET_LEX_NEXT)) && (instptr != (objident *) NULL) &&
         (instptr->ncmp != 0))
	num++;

      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;
      return(make_str(&(repl->val), get_afs_suid_cell(num, reqflg, first)));

    default:
      
      if((str = get_afs_db(instptr, reqflg, varnode->offset)) 
	 == (char *) NULL)
	return(BUILD_ERR);
  
      while(i < 4)
	repl->name.cmp[repl->name.ncmp++] = instptr->cmp[i++];

      if(varnode->offset == N_AFSDBADDR)
	{
	  repl->val.type = IPADD;
	  tmp = inet_addr(str);
	  bcopy(&tmp, &(repl->val.value.ipadd), sizeof(tmp));
	  return(BUILD_SUCCESS);
	}
      else
	return(make_str(&(repl->val), str));
    }
}


static char *
afs_this_cell()
{
  FILE *fp;

  if((fp = fopen(afs_cell_file, "r")) == (FILE *) NULL)
    return((char *) NULL);
  
  bzero(lbuf, sizeof(lbuf));
  while(fgets(lbuf, sizeof(lbuf) - 1, fp) != (char *) NULL)
    {
      if((*lbuf == '\0') || (*lbuf == '#'))
	{
	  *lbuf = '\0';
	  continue;
	}
      fclose(fp);
      return(lbuf);
    }

  fclose(fp);
  return((char *) NULL);
}




static char *
get_afs_suid_cell(num, reqflg, first)
     int num;
     int reqflg;
{
  FILE *fp;
  int i = 1;
  int oncemore = 1;

  if((fp = fopen(afs_suid_file, "r")) == (FILE *) NULL)
    return((char *) NULL);
  
  if((reqflg == REQ) || first);
    oncemore = 0;

  bzero(lbuf, sizeof(lbuf));
  while(fgets(lbuf, sizeof(lbuf) - 1, fp) != (char *) NULL)
    {
      if((*lbuf == '\0') || (*lbuf == '#'))
	continue;
      
      if((i == num) && !oncemore)
	{
	  fclose(fp);
	  lbuf[strlen(lbuf) - 1] = '\0';
	  return(lbuf);
	}
      else
	if(i != num)
	  ++i;
	else
	  oncemore = 0;
    }
  fclose(fp);
  return((char *) NULL);
}



static char *
get_afs_db(instptr, reqflg, offset)
     objident *instptr;
     int reqflg;
     int offset;
{
  FILE *fp;
  static char comment[SNMPSTRLEN];
  static char cell[SNMPSTRLEN];
  static char addr[16];
  int more = 0;
  int first = 0;
  int now = 0;
  char *c;
  char *e;
  
  if((fp = fopen(afs_cellserv_file, "r")) == (FILE *) NULL)
    return((char *) NULL);
  
  if(reqflg & (NXT|GET_LEX_NEXT))
    now = 0;
  else
    now = 1;

  if(instptr->ncmp == 0)
    first = 1;
  else
    sprintf(addr, "%d.%d.%d.%d", instptr->cmp[0], instptr->cmp[1],
	    instptr->cmp[2], instptr->cmp[3]);

  bzero(lbuf, sizeof(lbuf));
  while(fgets(lbuf, sizeof(lbuf) - 1, fp) != (char *) NULL)
    {
      c = lbuf;
      if((*c == '\0') || (*c == '#'))
	continue;
	      
      if(*c == '>')
	{
	  while((*c != ' ') && (*c != '\t') && (*c != '\0'))
	    ++c;
	  more = 0;
	  if(*c != '\0')
	    {
	      *c++ = '\0';
	      more = 1;
	    }
	  strncpy(cell, &(lbuf[1]), sizeof(cell));
	  bzero(comment, sizeof(comment));
	  if(!more)
	    continue;
	    
	  while(((*c == '#') || (*c == ' ') || (*c == '\t')) && (*c != '\0'))
	    ++c;
	  if(*c == '\0')
	    continue;
	    
	  c[strlen(c) - 1] = '\0'; /* newline */
	  strncpy(comment, c, sizeof(comment));
	  continue;
	}
      while(((*c == ' ') || (*c == '\t')) && (*c != '\0'))
	++c;
      if(*c == '\0')
	continue;
	
      e = c;
      while((*e != ' ') && (*e != '\t') && (*e != '\0'))
	++e;
      *e = '\0';
      if(!first && strcmp(addr, c) && !now)
	continue;
      else
	if(!first && !now)
	  {
	    now = 1;
	    continue;
	  }
	else
	  {
	    fclose(fp);
	    instptr->ncmp = 4;
	    e = c;
	    if((e = index(c, '.')) == (char *) NULL)
	      return((char *) NULL);
	    *e++ = '\0';
	    instptr->cmp[0] = atoi(c);

	    c = e;
	    if((e = index(c, '.')) == (char *) NULL)
	      return((char *) NULL);
	    *e++ = '\0';
	    instptr->cmp[1] = atoi(c);

	    c = e;
	    if((e = index(c, '.')) == (char *) NULL)
	      return((char *) NULL);
	    *e++ = '\0';
	    instptr->cmp[2] = atoi(c);

	    c = e;
	    instptr->cmp[3] = atoi(c);

	    switch(offset)
	      {
	      case N_AFSDBNAME:
		return(cell);
	      case N_AFSDBCOMMENT:
		return(comment);
	      case N_AFSDBADDR:
		sprintf(addr, "%d.%d.%d.%d", instptr->cmp[0], instptr->cmp[1],
			instptr->cmp[2], instptr->cmp[3]);
		return(addr);
	      default:
		return((char *) NULL);
	      }	    
	  }
    }
  fclose(fp);
  return((char *) NULL);
}
	  
      


/*
 * Function:    crock_cachesize()
 * Description: Reads the cachefile, and returns the cache size.
 *              The cache description line better be the first line and
 *              the size after the last semi-colon.
 * Returns:     size of cache if successful
 *              0 if error
 */

static int
crock_cachesize()
{
  FILE *fp;
  char *cp;

  fp = fopen(afs_cache_file, "r");
  if(fp == (FILE *) NULL)
    {
      syslog(LOG_ERR, "lu_afs: cannot open %s", afs_cache_file);
      return(BUILD_ERR);
    }

  while(fgets(lbuf, sizeof(lbuf), fp) != (char *) NULL)
    {
      if(*lbuf == '\0')
	continue;
      cp = rindex(lbuf, ':');
      if(cp++ == (char *) NULL)
	return(0);
      
      fclose(fp);
      return(atoi(cp));
    }

  fclose(fp);
  return(0);
}

#endif AFS
#endif MIT
