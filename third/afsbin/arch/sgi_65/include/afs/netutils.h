/* Copyright (C)  1999  Transarc Corporation.  All rights reserved.
 *
 * RCSID("$Header")
 */

#ifndef TRANSARC_NETUTILS_H
#define TRANSARC_NETUTILS_H

/* Network and IP address utility and file parsing functions */

extern int parseNetRestrictFile(
				u_long outAddrs[], 
				int32 mask[], 
				int32 mtu[],
				u_int32 maxAddrs,
				u_int32 *nAddrs, 
				char reason[], 
				const char *fileName
				);

extern int filterAddrs(
		       u_long addr1[],u_long addr2[],
		       int32  mask1[], int32 mask2[],
		       int32  mtu1[], int32 mtu2[]
		       );

extern int parseNetFiles(
			 u_long addrbuf[],
			 int32  maskbuf[],
			 int32  mtubuf[],
			 u_long max,
			 char reason[],
			 const char *niFilename,
			 const char *nrFilename
			 );

#endif /* TRANSARC_NETUTILS_H */ 
