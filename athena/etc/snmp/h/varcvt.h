/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/h/varcvt.h,v 1.1 1994-09-18 12:57:13 cfields Exp $
 *
 * $Log: not supported by cvs2svn $
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
/*****************************************************************************
**
**			varcvt.h
**
** This is the header file for the translator that converts SNMP
** variables from symbolic to numeric form and vice-versa. This converter
** will convert variables of form:
**
**	<symbolic prefix><dot notation IP address> to 
**	<numeric prefix><32 bit IP address>;		(SYMTONUM conversion)
**
** and
**
**	<numeric prefix><32 bit IP address> to
**	<symbolic prefix><dot notation IP address>	(NUMTOSYM conversion)
**
**
************************************************************************/
/* conversion flags */
#define SYMTONUM		1	/* convert symbolic to numeric */
#define NUMTOSYM		2	/* convert numeric to symbolic */

#define VARINITFL		"/mit/snmp/nyser/etc/snmp.variables"

extern short varcvt();
