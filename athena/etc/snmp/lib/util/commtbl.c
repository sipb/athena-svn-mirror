#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/lib/util/commtbl.c,v 1.2 1997-02-27 06:40:59 ghudson Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18 12:56:43  cfields
 * Initial revision
 *
 * Revision 1.1  89/11/03  15:16:17  snmpdev
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
THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
IS STRICTLY PROHIBITED.  (C) 1988 NYSERNET, INC.  (SUBJECT TO 
LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
*/
/*******************************************************************************
**
**			commtbl.c
**
** File contains all structures, definitions and functions that make
** up the communications table and its access functions.
**
**
*******************************************************************************/
#include "../../h/conf.h"
#ifdef BSD
#include <stdio.h>
#include <sys/types.h>			/* for system types */
#include <netinet/in.h>			/* for sockaddr_in definition */
#endif /* BSD */
#ifdef SVR3WIN
#include <stdio.h>
#include "/usr/netinclude/sys/types.h"  /* for system types */
#include "/usr/netinclude/netinet/in.h" /* for sockaddr_in definition */
#endif /* SVR3WIN */
#include "../../h/snmperrs.h"

#define MAXTBL		4		/* max number of table entries */

typedef struct {			/* a table entry */
	long			req_id;	/* key - the request id */
	struct sockaddr_in	comm;	/* the information */
} tblentry;

typedef struct {			/* the table */
/*
  note - make the occupancy vector large enough so there is 1 bit for
  every entry in the table. 
*/
	short		occvec;		/* the occupancy vector */
	tblentry	entry[MAXTBL];	/* the table entries */
} table;

static table commtbl;

/* macros to manipulate the occupancy vector */
#define isoccupied(n)	(commtbl.occvec & 1 << n)
#define mkoccupied(n)	(commtbl.occvec = (commtbl.occvec | 1 << n))
#define mkvacant(n)	(commtbl.occvec = (commtbl.occvec & ~(1 << n)))

/* comminfo - find communications information for req_id and return
              the corresponding sockaddr_in, or NULL
*/
struct sockaddr_in *
comminfo(req_id)
long req_id;
{ int i;				/* index and counter */
  for(i=0;i < MAXTBL; i++)
   if(commtbl.entry[i].req_id == req_id) /* if there is a match */
    return(&(commtbl.entry[i].comm));
  return((struct sockaddr_in *)NULL);
}

/* commadd - add an entry for req_id in communications table */
short
commadd(req_id,comm,commsz)
long req_id;				/* the request id */
struct sockaddr_in *comm;		/* the communications information */
short commsz;				/* size of information */
{ int i;				/* index and counter */
  for(i=0; i < MAXTBL; i++)
   if(!isoccupied(i))			/* if this table entry is vacant */
    { mkoccupied(i);			/* it's not vacant anymore */
      commtbl.entry[i].req_id = req_id;	/* fill in request id */
/* can't do a bcopy of comm, must do it element by element. sigh. */
      memcpy(&(comm->sin_addr),&(commtbl.entry[i].comm.sin_addr),
	     sizeof(comm->sin_addr));
      commtbl.entry[i].comm.sin_family = comm->sin_family;
      commtbl.entry[i].comm.sin_port = comm->sin_port;
      return(0);
    }
  return(TBLFULL);			/* return an error */
}

/* commrm - remove the entry corresponding to request id in the table */
short
commrm(req_id)
long req_id;				/* id whose table entry to be rm'd */
{ int i;				/* index and counter */
  for(i=0; i < MAXTBL; i++)
   if(commtbl.entry[i].req_id == req_id)
    { mkvacant(i);
      return(0);
    }
  return(NOSUCHID);
}
