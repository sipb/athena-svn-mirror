#include <stdio.h>
#include <stdlib.h>
#include "xdvi-config.h"	/* for xmalloc etc. */

#ifndef ALLOC_DEBUG_H__
#define ALLOC_DEBUG_H__


/*
 * Simplistic macros to help finding `hot spots'/bugs in memory handling.
 * Require that MALLOC(x) is never a single statement following an if(),
 * else() etc. without braces, and never in the middle of a variable
 * declarations list.
 */

/* #define DEBUG_MEMORY_HANDLING */

#ifdef DEBUG_MEMORY_HANDLING
#define XMALLOC(x,y) ( \
	fprintf(stderr, "%s:%d: malloc(%s, %d)\n", __FILE__, __LINE__, y, x), \
	xmalloc(x))
#define XREALLOC(x,y) ( \
	fprintf(stderr, "%s:%d: realloc(%s, %d)\n", __FILE__, __LINE__, #x, y), \
	xrealloc(x, y))
#define FREE(x)	( \
	fprintf(stderr, "%s:%d: free(%s)\n", __FILE__, __LINE__, #x), \
	free(x))
#else
#define XMALLOC(x,y) xmalloc(x)
#define XREALLOC(x,y) xrealloc(x,y)
#define FREE(x) free(x)
#endif

#endif /* ALLOC_DEBUG_H__ */
