/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.3 $
 */

#ifndef __config_data_h__
#define __config_data_h__

#include "mcregex.h"

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
 * What level of Root access is required by the OS
 */
#define RA_NONE			1
#define RA_ADVISED		2

#endif /* __config_data_h__ */
