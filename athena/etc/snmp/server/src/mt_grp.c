/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the hardware (machtype) portion of the mib.
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
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/mt_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/mt_grp.c,v 1.2 1990-04-26 17:28:46 tom Exp $";
#endif


#include "include.h"
#include <mit-copyright.h>

#ifdef MIT
#ifdef ATHENA

/*
 * machtype is such a gross hack we'll just use it
 */

#define MACH_PROGRAM "/bin/athena/machtype"
#define MACH_MACHOPT "-c"
#define MACH_DISPOPT "-d"
#define MACH_DROPT   "-r"
#define MACH_MOPT    "-M"

/*
 * This will have to do for now
 */

#define MAX_DISKS  20
#define MAX_DISPLAY 10
#define DISK_SIZE  16
#define DPY_SIZE    16
char displays[MAX_DISPLAY][DPY_SIZE];
char disks[MAX_DISKS][DISK_SIZE];

char *type = (char *) NULL;
int  ndisplays = -1;
int  ndisks = -1;
int  memory = -1;

static void mt_machtype();
static void mt_display();
static void mt_disks();
static void mt_memory();

/* 
 * Function:    lu_machtype()
 * Description: Top level callback for machtype info. Suuports:
 *                    N_MACHTYPE:     (STR) machine type
 *                    N_MACHNDISPLAY: (INT) number of displays
 *                    N_MACHDISPLAY:  (STR) display
 *                    N_MACHNDISK:    (INT) number of disks
 *                    N_MACHDISK:     (STR) disk
 *                    N_MACHMEMORY:   (INT) amount of emmory
 *              Information is retrieved from /bin/athena/machtype once
 *              and stored permenantly. Warning! You must restart snmpd if you 
 *              change machine types!
 * Returns:     BUILD_SUCCESS/BUILD_ERR
 */


int
lu_machtype(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  int num = 0;

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);


  switch (varnode->offset) 
    {
    case N_MACHTYPE:
      if(type == (char *) NULL)
	mt_machtype();

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.ncmp++;			/* include the "0" instance */
      repl->val.type = STR;
      repl->val.value.str.len = strlen(type);
      repl->val.value.str.str = (char *) malloc(repl->val.value.str.len * 
						sizeof(char));
      strcpy(repl->val.value.str.str, type);
      return(BUILD_SUCCESS);

    case N_MACHNDISPLAY:

      if(ndisplays < 0)
	mt_display();
      if(ndisplays < 0)
	return(BUILD_ERR);

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.ncmp++;			/* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = ndisplays;
      return(BUILD_SUCCESS);

    case N_MACHDISPLAY:      
      if(ndisplays < 0)
	mt_display();
      if(ndisplays <= 0)
	return(BUILD_ERR);
      if(num > ndisplays)
	return(BUILD_ERR);

      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	num = 1;
      else 
	num = instptr->cmp[0];
      
      if((reqflg == NXT) && (instptr != (objident *) NULL) && 
	 (instptr->ncmp != 0))
	num++;

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;			
      repl->val.type = STR;
      repl->val.value.str.len = strlen(displays[num-1]);
      repl->val.value.str.str = (char *) malloc(repl->val.value.str.len * 
						sizeof(char));
      strcpy(repl->val.value.str.str, displays[num-1]);
      return(BUILD_SUCCESS);

    case N_MACHNDISK:
      if(ndisks < 0)
	mt_disks();
      if(ndisks < 0)
	return(BUILD_ERR);

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = ndisks;
      return(BUILD_SUCCESS);
      
    case N_MACHDISK:
      if(ndisks < 0)
	mt_disks();
      if(ndisks <= 0)
	return(BUILD_ERR);
      if(num > ndisks)
	return(BUILD_ERR);

      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	num = 1;
      else 
	num = instptr->cmp[0];
      
      if((reqflg == NXT) && (instptr != (objident *) NULL) && 
	 (instptr->ncmp != 0))
	num++;

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;                    /* include the "0" instance */
      repl->val.type = STR;
      repl->val.value.str.len = strlen(disks[num-1]);
      repl->val.value.str.str = (char *) malloc(repl->val.value.str.len  * 
						sizeof(char));
      strcpy(repl->val.value.str.str, disks[num-1]);
      return(BUILD_SUCCESS);

    case N_MACHMEMORY:
      if(memory < 0)
	mt_memory();

      bcopy ((char *)varnode->var_code, (char *) &repl->name, 
	     sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = memory;
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_machtype: bad mt offset: %d", varnode->offset);
      return(BUILD_ERR);
  }
}



/* 
 * Function:    mt_machtype()
 * Description: call machtype and get string
 */

static void
mt_machtype(string,size)
     char *string;
     int size;
{
  char buf[BUFSIZ];

  bzero(buf, BUFSIZ);
  call_program(MACH_PROGRAM, MACH_MACHOPT, buf, BUFSIZ-1);
  type = (char *) malloc((strlen(buf) + 1) * sizeof(char));
  if(type == (char *) NULL)
    syslog(LOG_ERR, "mt_machtype: unable to allocate teensie string");
  else
    strcpy(type, buf);
}



/* 
 * Function:    mt_display()
 * Description: call machtype and display list
 */

static void
mt_display(string,size)
     char *string;
     int size;
{
  char buf[BUFSIZ];
  int mptr = 0;
  char *cptr;

  bzero(buf, BUFSIZ);
  ndisplays = 0;
  call_program(MACH_PROGRAM, MACH_DISPOPT, buf, BUFSIZ);
  if(*buf == '\0')
    return;
  for(cptr = &buf[0]; *cptr != '\0'; cptr++)
    if(*cptr == '\n')
      {
	if(mptr != 0)
	  {
	    displays[ndisplays++][mptr] = '\0';
	    mptr = 0;
	    if(ndisplays == MAX_DISPLAY)
	      break;
	  }
      }
    else
      if(mptr < DPY_SIZE)	  
	displays[ndisplays][mptr++] = *cptr;
}


/* 
 * Function:    mt_disks()
 * Description: call machtype and get disk list
 */

static void
mt_disks(string,size)
     char *string;
     int size;
{
  char buf[BUFSIZ];
  int mptr = 0;
  int copy = 0;
  char *cptr;

  bzero(buf, BUFSIZ);
  call_program(MACH_PROGRAM, MACH_DROPT, buf, BUFSIZ);
  if(*buf == '\0')
    return;
  ndisks = 0;
  for(cptr = &buf[0]; *cptr != '\0'; cptr++)
    if(*cptr == '\n')
      {
	disks[ndisks++][mptr] = '\0';
	mptr = 0;
	copy = 0;
	if(ndisks == MAX_DISKS)
	  break;
      }
    else
      if((mptr < DISK_SIZE) && copy)
	disks[ndisks][mptr++] = *cptr;
      else
	if(!copy && (*cptr == ':'))
	  {
	    copy = 1;
	    cptr++;
	  }	  
}
  

/* 
 * Function:    mt_machtype()
 * Description: call machtype and get memory
 */

static void
mt_memory()
{
  char buf[BUFSIZ];

  bzero(buf, BUFSIZ);
  call_program(MACH_PROGRAM,MACH_MOPT, buf, BUFSIZ-1);
  if(strncmp(buf, "0x", 2) == 0)
    sscanf(&buf[2], "%x", &memory);
  else
    memory = atoi(buf);
}
