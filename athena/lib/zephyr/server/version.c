/* This file is part of the Project Athena Zephyr Notification System.
 * It contains the version identification of the Zephyr server
 *
 *	Created by:	John T. Kohl
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/version.c,v $
 *	$Author: ghudson $
 *
 *	Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */

#include <zephyr/mit-copyright.h>

#include "zserver.h"
#include "version.h"

const char zephyr_version[] = "Zephyr system version 2.0";

#ifdef DEBUG
const char version[] = "Zephyr server (DEBUG) $Revision: 3.23 $";
#else
const char version[] = "Zephyr server $Revision: 3.23 $";
#endif

#if !defined (lint) && !defined (SABER)
static const char rcsid_version_c[] =
    "$Id: version.c,v 3.23 1997-09-14 21:54:36 ghudson Exp $";
static const char copyright[] =
    "Copyright (c) 1987,1988,1989,1990 Massachusetts Institute of Technology.\n";
#endif

char *
get_version()
{
  static char vers_buf[256];

  if (vers_buf[0] == '\0') {
#ifdef DEBUG
    sprintf(vers_buf,"Zephyr Server (DEBUG) $Revision: 3.23 $: %s",
	    ZSERVER_VERSION_STRING);
#else
    sprintf(vers_buf,"Zephyr Server $Revision: 3.23 $: %s",
	    ZSERVER_VERSION_STRING);
#endif /* DEBUG */

    (void) strcat(vers_buf, "/");
#ifdef vax
    (void) strcat(vers_buf, "VAX");
#endif /* vax */
#ifdef ibm032
    (void) strcat(vers_buf, "IBM RT");
#endif /* ibm032 */
#ifdef _IBMR2
    (void) strcat(vers_buf, "IBM RS/6000");
#endif /* _IBMR2 */
#ifdef sun
    (void) strcat(vers_buf, "SUN");
#ifdef sparc
    (void) strcat (vers_buf, "-4");
#endif /* sparc */
#ifdef sun386
    (void) strcat (vers_buf, "-386I");
#endif /* sun386 */
#endif /* sun */

#ifdef mips
#ifdef ultrix			/* DECstation */
    (void) strcat (vers_buf, "DEC-");
#endif /* ultrix */
    (void) strcat(vers_buf, "MIPS");
#endif /* mips */
#ifdef NeXT
    (void) strcat(vers_buf, "NeXT");
#endif /* NeXT */
  }
  return(vers_buf);
}





