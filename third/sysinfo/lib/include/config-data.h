/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __config_data_h__
#define __config_data_h__

#include "mcregex.h"

/*
 * Argument Types
 * The ARG_* values are set in options.h
 */

/*
 * Get Network InterFaces
 */
#define GETNETIF_IFNET		1
#define GETNETIF_IFCONF		2

/*
 * Get MAC Info
 */
#define GETMAC_NONE		0
#define GETMAC_NIT		1
#define GETMAC_PACKETFILTER	2
#define GETMAC_DLPI		3
#define GETMAC_IFREQ_ENADDR	4
#define GETMAC_SIOCGIFHWADDR	5

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
