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
