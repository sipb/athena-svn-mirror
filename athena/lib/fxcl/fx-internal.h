/*
 * The FX (File Exchange) Library
 *
 * $Id: fx-internal.h,v 1.1 1999-09-28 22:07:18 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

/*
 * Definitions common to both library and server.
 */

#ifndef _fx_internal_h_

/*
 * ACL names.
 */

#define ACL_TURNIN "turnin"
#define ACL_GRADER "grader"
#define ACL_MAINT "maint"

/*
 * Kerberos authentication service name.
 */
#define KRB_SERVICE "rcmd"

/*
 * Hesiod name service.
 */
#define HES_NAME "turnin"
#define HES_TYPE "sloc"

/*
 * Static server list when compiled without Hesiod support
 */
#define SERVER_LIST_FILE "/var/exchange/server.list"

/*
 * Format of a paper displayed to the user - usually for debugging.
 */

#define PRINTPAPER(p) "%s:%s %d:\"%s\",%d (%d,%d,%d)\n    [%s:%d.%d] [%d.%d] [%d.%d]\n", \
    (p)->author, (p)->owner, (p)->type, (p)->filename, \
    (p)->assignment, (p)->size, (p)->words, (p)->lines, (p)->location.host, \
    (p)->location.time.tv_sec, (p)->location.time.tv_usec, \
    (p)->created.tv_sec, (p)->created.tv_usec, (p)->modified.tv_sec, \
    (p)->modified.tv_usec

#endif /* _fx_internal_h_ */
