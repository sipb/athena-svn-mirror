#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#define BSDUNIX
#define MUSTALIGN
typedef void sigtype;	/* Signal handler functions are declared "void".  */

/* Do name->address->name resolution for security checking */
/* FIXME, why is this only done on Solaris?  */
#define	DO_REVERSE_RESOLVE	1

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1

/* Used in appl/bsd/login.c and kuser/ksu.c. */
#define	NO_SETPRIORITY	1

/* Used in lib/des/random_key.h, lib/kstream/kstream_des.c */
#define random	lrand48
#define srandom	srand48
