#define BSDUNIX
#define BITS32
#define BIG
#if defined(i386) || defined(__i386__)
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#endif

#if 0
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#endif

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Reportedly, SVR4 does NOT include the NDBM library */
/*   /usr/include/rpcsvc/dbm.h ???   */
/* This unsets the #define in conf.h which makes NDBM the default.  */
#undef	NDBM

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1

/* Used in appl/bsd/login.c and kuser/ksu.c. */
#define	NO_SETPRIORITY	1

/* Used in lib/des/random_key.h, lib/kstream/kstream_des.c */
#define random	lrand48
#define srandom	srand48
