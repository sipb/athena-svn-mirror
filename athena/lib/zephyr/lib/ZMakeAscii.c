/* This file is part of the Project Athena Zephyr Notification System.
 * It contains source for the internal Zephyr routines.
 *
 *	Created by:	Robert French
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZMakeAscii.c,v $
 *	$Author: rfrench $
 *
 *	Copyright (c) 1987 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
/* $Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZMakeAscii.c,v 1.5 1987-07-29 15:17:06 rfrench Exp $ */

#ifndef lint
static char rcsid_ZMakeAscii_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/lib/ZMakeAscii.c,v 1.5 1987-07-29 15:17:06 rfrench Exp $";
#endif lint

#include <zephyr/mit-copyright.h>

#include <zephyr/zephyr_internal.h>

Code_t ZMakeAscii(ptr,len,field,num)
	char *ptr;
	int len;
	unsigned char *field;
	int num;
{
	int i;

	for (i=0;i<num;i++) {
		if (!(i%4)) {
			if (len < 3+(i!=0))
				return (ZERR_FIELDLEN);
			(void) sprintf(ptr,"%s0x",i?" ":"");
			ptr += 2+(i!=0);
			len -= 2+(i!=0);
		} 
		if (len < 3)
			return (ZERR_FIELDLEN);
		(void) sprintf(ptr,"%02x",field[i]);
		ptr += 2;
		len -= 2;
	}

	return (ZERR_NONE);
}
