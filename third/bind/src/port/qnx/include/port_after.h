#define	NEED_DAEMON
extern int daemon(int,int);
#define NEED_MKTEMP
extern char *mktemp(const char *);
#define NEED_MKSTEMP
extern int mkstemp(const char *);
#define NEED_STRCASECMP
extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, size_t);
#define NEED_STRSEP
extern char *strsep(char **, const char *);
#define NEED_PSELECT

#undef	RENICE

#define WAIT_T	int

#ifndef AF_INET6
#define AF_INET6	24
#endif

