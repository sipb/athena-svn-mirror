/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

/*
 * $Id: config-data.h,v 1.1.1.2 1998-02-12 21:32:24 ghudson Exp $
 */

#ifndef __config_data_h__
#define __config_data_h__

/*
 * Argument Types
 */
#define ARG_VARARGS		1
#define ARG_STDARG		2

/*
 * Get Network InterFaces
 */
#define GETNETIF_IFNET		1
#define GETNETIF_IFCONF		2

/*
 * Get MAC Info
 */
#define GETMAC_NIT		1
#define GETMAC_PACKETFILTER	2
#define GETMAC_DLPI		3
#define GETMAC_IFREQ_ENADDR	4

/*
 * Type of wait() system call
 */
#define WAIT_WAITPID		1
#define WAIT_WAIT4		2

/*
 * Types of regular expression functions
 */
#define RE_REGCOMP		1		/* POSIX regcomp() */
#define RE_COMP			2		/* BSD re_comp() */
#define RE_REGCMP		3		/* SYSV regcmp() */

#endif /* __config_data_h__ */
