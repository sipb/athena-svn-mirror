/* Copyright (C)  1998  Transarc Corporation.  All rights reserved. */


#ifndef _AFSUTIL_H_
#define _AFSUTIL_H_

#include <time.h>
/* Include afs installation dir retrieval routines */
#include <afs/dirpath.h>

/* These macros are return values from extractAddr. They do not represent
 * any valid IP address and so can indicate a failure.
 */
#define	AFS_IPINVALID 		0xffffffff /* invalid IP address */
#define AFS_IPINVALIDIGNORE	0xfffffffe /* no input given to extractAddr */

/* logging defines
 */
extern int LogLevel;
#define ViceLog(level, str)  if ((level) <= LogLevel) (FSLog str)

/* special version of ctime that clobbers a *different static variable, so
 * that ViceLog can call ctime and not cause buffer confusion.  Barf.
 */
extern char *vctime(const time_t *atime);


/* Convert a 4 byte integer to a text string. */
extern char*	afs_inet_ntoa(int32 addr);


/* copy strings, converting case along the way. */
char *lcstring(char *d, char *s, int n);
char *ucstring(char *d, char *s, int n);
char *strcompose(char *buf, size_t len, ...);

/* abort the current process. */
#ifdef AFS_NT40_ENV
#define afs_abort() afs_NTAbort()
#else
#define afs_abort() abort()
#endif


#ifdef AFS_NT40_ENV
#include <winsock2.h>

/* Initialize the windows sockets before calling networking routines. */
extern int afs_winsockInit(void);

struct timezone {
    int  tz_minuteswest;     /* of Greenwich */
    int  tz_dsttime;    /* type of dst correction to apply */
};
#define gettimeofday afs_gettimeofday
int afs_gettimeofday(struct timeval *tv, struct timezone *tz);

/* Unbuffer output when Un*x would do line buffering. */
#define setlinebuf(S) setvbuf(S, NULL, _IONBF, 0)

/* regular expression parser for NT */
extern char *re_comp(char *sp);
extern int rc_exec(char *p);

/* get temp dir path */
char *gettmpdir(void);

/* Abort on error, possibly trapping to debugger or dumping a trace. */
void afs_NTAbort(void);
#endif

/* Base 32 conversions used for NT since directory names are
 * case-insensitive.
 */
typedef char b32_string_t[8];
char *int_to_base32(b32_string_t s, int a);
int base32_to_int(char *s);

/* This message preserves our ability to license AFS to the U.S. Government more than
 * once.
 */

#define AFS_GOVERNMENT_MESSAGE \
"===================== U.S. Government Restricted Rights ======================\n\
If you are licensing the Software on behalf of the U.S. Government\n\
(\"Government\"), the following provisions apply to you.  If the Software is\n\
supplied to the Department of Defense (\"DoD\"), it is classified as \"Commercial\n\
Computer Software\" under paragraph 252.227-7014 of the DoD Supplement to the\n\
Federal Acquisition Regulations (\"DFARS\") (or any successor regulations)\n\
and the Government is acquiring only the license rights granted herein (the\n\
license rights customarily provided to non-Government users).  If the Software\n\
is supplied to any unit or agency of the Government other than DoD, it is\n\
classified as \"Restricted Computer Software\" and the Government's rights in\n\
the Software are defined in paragraph 52.227-19 of the Federal Acquisition\n\
Regulations (\"FAR\") (or any successor regulations) or, in the case of NASA,\n\
in paragraph 18.52.227-86 of the NASA Supplement in the FAR (or any successor\n\
regulations).\n"

#endif /* _AFSUTIL_H_ */
