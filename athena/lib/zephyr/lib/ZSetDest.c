/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the ZSetDestAddr function.
 *
 *	Created by:	Robert French
 *
 *	$Id: ZSetDest.c,v 1.5 1999-01-22 23:19:28 ghudson Exp $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#ifndef lint
static char rcsid_ZSetDestAddr_c[] = "$Id: ZSetDest.c,v 1.5 1999-01-22 23:19:28 ghudson Exp $";
#endif

#include <internal.h>

Code_t ZSetDestAddr(addr)
	struct	sockaddr_in *addr;
{
	__HM_addr = *addr;

	__HM_set = 1;
	
	return (ZERR_NONE);
}
