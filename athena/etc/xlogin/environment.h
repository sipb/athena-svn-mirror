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

/* Common macros. */
#define file_exists(f) (access((f), F_OK) == 0)
#define ROOT 0
