/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * Ethernet related information
 */
#ifndef __mysysinfo_ether__
#define __mysysinfo_ether__ 

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

#if	!defined(_PATH_ETHERS)
#define _PATH_ETHERS		"/etc/ethers"
#endif

#if	!defined(HAVE_ETHER_ADDR)
struct ether_addr {
    unsigned char		ether_addr_octet[6];
};
#endif	/* !HAVE_ETHER_ADDR */

#endif	/* __mysysinfo_ether__ */
