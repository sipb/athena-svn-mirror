/*
 * Common environment variable settings.
 */

#ifdef ultrix
#define HOSTTYPE "decmips"
#endif

#ifdef _IBMR2
#define HOSTTYPE "rsaix"
#endif

#ifdef SOLARIS
#define HOSTTYPE "sun4"
#endif

#ifdef sgi
#define HOSTTYPE "sgi"
#endif

/* Common macros. */
#define file_exists(f) (access((f), F_OK) == 0)
#define ROOT 0
#define WHEEL 0

#ifdef SOLARIS_MAE
/* Stuff for supporting MAE's need to talk to the network. Sigh. */

#ifndef NETSPY
#define NETSPY "/etc/netspy"
#endif

#define SYS 3		/* group for chown of NETDEV */

#define NETDEV "/dev/le"

extern int netspy;
#endif /* SOLARIS_MAE */
