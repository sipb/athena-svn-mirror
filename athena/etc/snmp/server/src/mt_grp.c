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
 *    $Author: ghudson $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 *    Revision 2.0  1992/04/22 02:04:30  tom
 *    release 7.4
 *    	added new machtype variables
 *
 * Revision 1.5  90/07/17  14:18:57  tom
 * deleted unused variables
 * stripped newline after machtype string
 * 
 * Revision 1.4  90/05/26  13:39:52  tom
 * athena release 7.0e - fixed get-next
 * 
 * Revision 1.3  90/04/26  17:36:00  tom
 * closed ifdefs
 * 
 * Revision 1.2  90/04/26  17:28:46  tom
 * deleted unused constants
 * 
 *
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/mt_grp.c,v 2.1 1997-02-27 06:47:38 ghudson Exp $";
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
#define MACH_VOPT    "-A"
#define MACH_OSOPT   "-N"
#define MACH_OSVOPT  "-E"

/*
 * This will have to do for now
 */

#define MAX_DISKS   20
#define MAX_DISPLAY 10
#define DISK_SIZE   16
#define DPY_SIZE    16
#define DATA_LEN    128

static char *machine_type = (char *) NULL;
static char *version = (char *) NULL;
static char *os = (char *) NULL;
static char *osversion = (char *) NULL;
static char displays[MAX_DISPLAY][DPY_SIZE];
static char disks[MAX_DISKS][DISK_SIZE];

static int  ndisplays = -1;
static int  ndisks = -1;
static int  memory = -1;

static void mt_machtype();
static void mt_display();
static void mt_disks();
static void mt_memory();
static void mt_version();
static void mt_osversion();


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
      varnode->offset <= 0)
    return (BUILD_ERR);


  /*
   * variables with instances are mixed with other variables... I hope
   * you haven't eaten
   */

  switch (varnode->offset) 
    {
    case N_MACHVERSION:
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
	
      if(!version)
	mt_version();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;			/* include the "0" instance */
      return(make_str(&(repl->val), version));

    case N_MACHTYPE:
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
	
      if(!machine_type)
	mt_machtype();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;			/* include the "0" instance */
      return(make_str(&(repl->val), machine_type));

    case N_MACHNDISPLAY:
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);

      if(ndisplays < 0)
	mt_display();
      if(ndisplays < 0)
	return(BUILD_ERR);

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;			/* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = ndisplays;
      return(BUILD_SUCCESS);

    case N_MACHDISPLAY:      
      if(ndisplays < 0)
	mt_display();
      if(ndisplays <= 0)
	return(BUILD_ERR);

      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	num = 1;
      else 
	num = instptr->cmp[0];
      
      if((reqflg & NXT) && (instptr != (objident *) NULL) && 
	 (instptr->ncmp != 0))
	num++;

      if(num > ndisplays)
	return(BUILD_ERR);

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;	
      return(make_str(&(repl->val), displays[num-1]));

    case N_MACHNDISK:
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);

      if(ndisks < 0)
	mt_disks();
      if(ndisks < 0)
	return(BUILD_ERR);

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = ndisks;
      return(BUILD_SUCCESS);
      
    case N_MACHDISK:
      if(ndisks < 0)
	mt_disks();
      if(ndisks <= 0)
	return(BUILD_ERR);

      if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
	num = 1;
      else 
	num = instptr->cmp[0];
      
      if((reqflg & NXT) && (instptr != (objident *) NULL) && 
	 (instptr->ncmp != 0))
	num++;
      
      /*
       * you must increment the instance even if the instance is out of bounds
       */

      if(num > ndisks)
	return(BUILD_ERR);

      if(num < 1)
	return(BUILD_ERR);

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.cmp[repl->name.ncmp] = num;
      repl->name.ncmp++;                    /* include the "0" instance */
      return(make_str(&(repl->val), disks[num-1]));

    case N_MACHMEMORY:      
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
      
      if(memory < 0)
	mt_memory();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      repl->val.type = INT;
      repl->val.value.intgr = memory;
      return(BUILD_SUCCESS);

    case N_MACHOS:      
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
      
      if(!os)
	mt_osversion();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      return(make_str(&(repl->val), os));

    case N_MACHOSVERSION:      
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
      
      if(!osversion)
	mt_osversion();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */
      return(make_str(&(repl->val), osversion));

    case N_MACHOSVENDOR:      
      if((varnode->flags & NULL_OBJINST) && (reqflg & NXT))
	return(BUILD_ERR);
      
      if(!os)
	mt_osversion();

      memcpy (&repl->name, varnode->var_code, sizeof(repl->name));
      repl->name.ncmp++;                    /* include the "0" instance */

      if(os)
	{
	  if(strcasecmp(os, "BSD") == 0)
	    return(make_str(&(repl->val), "UCB"));
	  if(strcasecmp(os, "ULTRIX") == 0)
	    return(make_str(&(repl->val), "DEC"));
	  if(strcasecmp(os, "AIX") == 0)
	    return(make_str(&(repl->val), "IBM"));
	  if(strcasecmp(os, "SUNOS") == 0)
	    return(make_str(&(repl->val), "SUN"));
	}
      return(make_str(&(repl->val), "???"));
      
    default:
      syslog (LOG_ERR, "lu_machtype: bad mt offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}



char *
get_machtype()
{
  if(*gw_version_id != '\0')
    machine_type = gw_version_id;
  else
    if(machine_type == (char *) NULL)
      mt_machtype();
  return(machine_type);
}




/* 
 * Function:    mt_machtype()
 * Description: call machtype and get string
 */

static void
mt_machtype()
{
  char *c;

  memset(lbuf, 0, sizeof(lbuf));
  call_program(MACH_PROGRAM, MACH_MACHOPT, lbuf, sizeof(lbuf)-1);
  if(*lbuf == '\0')
    return;
  if(!(machine_type = (char *) malloc((strlen(lbuf) + 1) * sizeof(char))))
    syslog(LOG_ERR, "mt_machtype: unable to allocate teensie string");
  else
    strcpy(machine_type, lbuf);
  if(c = rindex(machine_type, '\n'))
    *c = '\0';
  return;
}


/* 
 * Function:    mt_version()
 * Description: call machtype and get version
 */

static void
mt_version()
{
  char *c;

  memset(lbuf, 0, sizeof(lbuf));
  call_program(MACH_PROGRAM, MACH_VOPT, lbuf, sizeof(lbuf)-1);
  if(!*lbuf)
    return;
  if(!(version = (char *) malloc((strlen(lbuf) + 1) * sizeof(char))))
    syslog(LOG_ERR, "mt_version: unable to allocate teensie string");
  else
    strcpy(version, lbuf);
  if(c = rindex(version, '\n'))
    *c = '\0';
  return;
}


/* 
 * Function:    mt_osversion()
 * Description: call machtype and get version
 */

static void
mt_osversion()
{
  char *c;

  memset(lbuf, 0, sizeof(lbuf));
  call_program(MACH_PROGRAM, MACH_OSOPT, lbuf, sizeof(lbuf)-1);
  if(!*lbuf)
    return;
  if(!(os = (char *) malloc((strlen(lbuf) + 1) * sizeof(char))))
    syslog(LOG_ERR, "mt_version: unable to allocate teensie string");
  else
    strcpy(os, lbuf);
  if(c = rindex(os, '\n'))
    *c = '\0';

  call_program(MACH_PROGRAM, MACH_OSVOPT, lbuf, sizeof(lbuf)-1);
  if(!*lbuf)
    return;
  if(!(osversion = (char *) malloc((strlen(lbuf) + 1) * sizeof(char))))
    syslog(LOG_ERR, "mt_version: unable to allocate teensie string");
  else
    strcpy(osversion, lbuf);
  if(c = rindex(osversion, '\n'))
    *c = '\0';
  return;
}


/* 
 * Function:    mt_display()
 * Description: call machtype and display list
 */

static void
mt_display()
{
  int mptr = 0;
  char *cptr;

  memset(lbuf, 0, sizeof(lbuf));
  memset(displays, 0, sizeof(displays));
  ndisplays = 0;
  call_program(MACH_PROGRAM, MACH_DISPOPT, lbuf, sizeof(lbuf)-1);
  if(*lbuf == '\0')
    return;
  for(cptr = &lbuf[0]; *cptr != '\0'; cptr++)
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
mt_disks()
{
  int  mptr = 0;
  int  copy = 0;
  char *cptr;

  memset(lbuf, 0, sizeof(lbuf));
  memset(disks, 0, sizeof(disks));
  call_program(MACH_PROGRAM, MACH_DROPT, lbuf, sizeof(lbuf));
  if(*lbuf == '\0')
    return;
  ndisks = 0;

  for(cptr = &lbuf[0]; *cptr != '\0'; cptr++)
    if(*cptr == '\n')
      {
	if(mptr > 0)
	  {
	    disks[ndisks++][mptr] = '\0';
	    mptr = 0;
	    if(ndisks == MAX_DISKS)
	      break;
	  }
	copy = 0;
      }
    else
      if((mptr < DISK_SIZE) && copy && isprint(*cptr))
	disks[ndisks][mptr++] = *cptr;
      else
	if(!copy  && (*cptr == ':'))
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

  memset(lbuf, 0, sizeof(lbuf));
  call_program(MACH_PROGRAM,MACH_MOPT, lbuf, sizeof(lbuf) - 1);
  if(strncmp(lbuf, "0x", 2) == 0)
    sscanf(&lbuf[2], "%x", &memory);
  else
    memory = atoi(lbuf);
}

#endif ATHENA
#endif MIT
