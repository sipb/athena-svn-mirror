/* src/config.h.  Generated automatically by configure.  */
/* src/config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/*
 * These are the defaults, but they can be changed by configure at compile
 * time, or on the command line at runtime.
 */
/* #undef DEFAULT_LOG */

/* define for openssl support (allows outgoing connections to use ssl) */
#define HAVE_OPENSSL 1

/* #undef ENABLE_PROFILER */

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the vsyslog function.  */
#define HAVE_VSYSLOG 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <syslog.h> header file.  */
#define HAVE_SYSLOG_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the popt library (-lpopt).  */
#define HAVE_LIBPOPT 1

/* Name of package */
#define PACKAGE "ammonite"

/* Version number of package */
#define VERSION "0.8.1"

