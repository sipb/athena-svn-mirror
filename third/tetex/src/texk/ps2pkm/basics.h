/* FILE:    basics.h
 * PURPOSE: basic definitions
 * AUTHOR:  Piet Tutelaers
 * VERSION: 1.0 (September 1995)
 */


#ifndef NOBASICS

#if defined(KPATHSEA)
#  include <kpathsea/config.h>
#  include <kpathsea/c-pathch.h>
#  include <kpathsea/c-fopen.h>
#  include <c-auto.h>
#  define PATHSEP ENV_SEP
#  define DIRSEP  DIR_SEP
#  define RB FOPEN_RBIN_MODE
#  define WB FOPEN_WBIN_MODE
#  if defined(DOSISH)
#    define ESCAPECHAR '!'
#    define RECURSIVE "!!"
#  endif /* DOSISH */
#  define basename ps2pk_basename
#elif defined(MSDOS) || defined(WIN32)
#  define PATHSEP ';'
#  define DIRSEP '\\'
#  define ESCAPECHAR '!'
#  define RECURSIVE "!!"
#  define PSRES_NAME	"psres.dpr"
#  define RB "rb"
#  define WB "wb"
#endif
 
#ifndef PATHSEP
#define PATHSEP ':'
#endif

#ifndef DIRSEP
#define DIRSEP '/'
#endif

#ifndef ESCAPECHAR
#define ESCAPECHAR '\\'
#endif

#ifndef RECURSIVE
#define RECURSIVE "//"
#endif

/* TeX PS Resource database name */
#ifndef PSRES_NAME
#define PSRES_NAME	"PSres.upr"
#endif

#ifndef RB
#define RB "r"
#endif

#ifndef WB
#define WB "w"
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

#ifndef MAXSTRLEN
#define MAXSTRLEN 256
#endif

#ifndef IS_DIR_SEP
#define IS_DIR_SEP(c) ((c) == DIRSEP)
#endif

#define NOBASICS
#endif

#include <stdarg.h>

void fatal(char *fmt, ...);
void msg(char *fmt, ...);

/* For debugging purposes it is handy to have a fopen() function that
 * shows which files are opened. The verbose my_fopen() can be installed
 * by a user program by assigning it to the pfopen function pointer
 *    pfopen = my_fopen
 */

#include <stdio.h>
extern FILE * (*pfopen)(const char *, const char *);
FILE * my_fopen(const char *, const char *);

/* For debugging purposes it is handy to have a stat() function that
 * shows which files it looks at. The verbose my_stat() can be installed
 * by a user program by assigning it to the pstat function pointer
 *    pstat = my_stat
 */
#include <sys/types.h>  /* struct stat */
#include <sys/stat.h>   /* stat() */

extern int (*pstat)(const char *, struct stat *);
int my_stat(const char *path, struct stat *buf);
