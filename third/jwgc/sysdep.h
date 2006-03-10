#ifndef _SYSDEP_H_
#define _SYSDEP_H_ 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif /* HAVE_SYS_UN_H */

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif /* HAVE_CTYPE_H */

#ifdef HAVE_MATH_H
#include <math.h>
#endif /* HAVE_MATH_H */

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif /* HAVE_PWD_H */

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif /* HAVE_SETJMP_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif /* HAVE_SYSLOG_H */

#ifdef HAVE_WAIT_H
#include <wait.h>
#endif /* HAVE_WAIT_H */

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif /* HAVE_TERMIO_H */

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif /* HAVE_TERMIOS_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif /* HAVE_DIRENT_H */

#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifdef HAVE_REGEX_H
#include <regex.h>
#else
#ifdef HAVE_REGEXP_H
#define __DO_NOT_DEFINE_COMPILE 1
#include <regexp.h>
#endif /* HAVE_REGEXP_H */
#endif /* HAVE_REGEX_H */

#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif /* HAVE_ARPA_INET_H */

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_NAMESER_H
#include <nameser.h>
#endif /* HAVE_NAMESER_H */

#ifdef HAVE_RESOLV_H
#include <resolv.h>
#endif /* HAVE_RESOLV_H */

#ifdef __STDC__
#ifdef HAVE_STDARG_H
#include <stdarg.h>

# define VA_LOCAL_DECL  va_list ap;
# define VA_START(f)    va_start(ap, f)
# define VA_END         va_end(ap)
#endif /* HAVE_STDARG_H */
#else /* __STDC__ */
#ifdef HAVE_VARARGS_H
#include <varargs.h>

# define VA_LOCAL_DECL  va_list ap;
# define VA_START(f)    va_start(ap)
# define VA_END         va_end(ap)
#endif /* HAVE_VARARGS_H */
#endif /* __STDC__ */

#ifdef HAVE_LIBSSL
#include <openssl/ssl.h>
#define USE_SSL 1
#endif /* HAVE_LIBSSL */

#ifdef HAVE_LIBGPGME
#define USE_GPGME 1
#endif /* HAVE_LIBGPGME */

#ifdef WINDOWS_H
#include <windows.h>
#endif /* WINDOWS_H */

#ifndef ulong
#define ulong u_long
#endif /* ulong */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif /* MAXHOSTNAMELEN */

#ifndef HAVE_LIBX11
#define X_DISPLAY_MISSING 1
#endif /* HAVE_LIBX11 */

#endif /* _SYSDEP_H_ */
