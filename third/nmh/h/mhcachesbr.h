
/*
 * mhcachesbr.h -- definitions for manipulating MIME content cache
 *
 * $Id: mhcachesbr.h,v 1.1.1.1 1999-02-07 18:14:06 danw Exp $
 */

/*
 * various cache policies
 */
static struct swit caches[] = {
#define CACHE_NEVER    0
    { "never", 0 },
#define CACHE_PRIVATE  1
    { "private", 0 },
#define CACHE_PUBLIC   2
    { "public", 0 },
#define CACHE_ASK      3
    { "ask", 0 },
    { NULL, 0 }
};
