/*
 * This file hails from config_p.h.  Edit it to suit your system.
 */

/* Include <stdlib.h>, if it exists. */
#include <stdlib.h>

/* Always include this one. */
#include <stdio.h>

/* Define I_STDARG if there's a stdarg.h. */
#define I_STDARG

/* Include either <string.h> or <strings.h> here. */
#include <string.h>

/* Include <memory.h>, if appropriate.  Note that it may have been left
   out even though it exists.  We do this because of conflicts on many
   systems.  But if it works, then it works. */
/* #include <memory.h> */

/* Include <unistd.h> if it exists. */
#include <unistd.h>

/* If the system has vfork(), then it may require us to include vfork.h */
/* #include <vfork.h> */

/* Define this if POSIX <dirent.h> instead of old <sys/dir.h>. */
#define POSIX_DIRENT

/* If the system has bzero(), use it; otherwise hope they have memset() */
/* #define bzero(s, n)	memset((s), 0, (n)) */

/* If the system has memcmp(), use it; otherwise hope they have bcmp() */
/* #define memcmp	bcmp */

/* If the system has memcpy(), use it; otherwise hope they have bcopy() */
/* #define memcpy(d, s, n)	bcopy(s, d, n) */

/* If the system has strchr(), use it; otherwise hope they have index() */
/* Likewise for strrchr() and rindex() */
/* #define strchr	index */
/* #define strrchr	rindex */

/* Use vfork() if it's available; otherwise, fork() */
/* #define vfork	fork */
