#define BITS32
#define BIG
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST
#define BSDUNIX
/* from Imakefile... */	
#define ATHENA 
#define HAVE_VSPRINTF

#define NOVFORK 
#define NEED_UTIMES 

/* Used in lib/des/random_key.h, lib/kstream/kstream_des.c */
#define random	lrand48
#define srandom	srand48
	
#define NO_FSYNC 
#define NO_SIGCONTEXT
	
#define HAS_DIRENT 
#define NOSTBLKSIZE 
#define NO_FCHMOD 
#define NO_SETPRIORITY
	
#define NO_GETUSERSHELL 
#define NOUTHOST
#define ftruncate(fd,sz) fcntl(fd,F_CHSIZE,sz)

#define NO_GETHOSTID_

#define HAVE_SYS_PTEM_H

typedef void sigtype;	/* Signal handler functions are declared "void".  */
