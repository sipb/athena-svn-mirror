/*
 * Copyright (c) 1997,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/* When this symbol is defined allocations via memget are made slightly 
   bigger and some debugging info stuck before and after the region given 
   back to the caller. */
/* #define DEBUGGING_MEMCLUSTER */


#if !defined(LINT) && !defined(CODECENTER)
static char rcsid[] = "$Id: memcluster.c,v 1.1.1.2 1999-03-16 19:46:06 danw Exp $";
#endif /* not lint */

#include "port_before.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <isc/memcluster.h>
#include <isc/assertions.h>

#include "port_after.h"

#define DEF_MAX_SIZE		1100
#define DEF_MEM_TARGET		4096

typedef struct {
	void *			next;
#if defined(DEBUGGING_MEMCLUSTER)
	int			size;
	int			fencepost;
#endif
} memcluster_element;

#define SMALL_SIZE_LIMIT sizeof(memcluster_element)
#define P_SIZE sizeof(void *)
#define FRONT_FENCEPOST 0xfeba
#define BACK_FENCEPOST 0xabef

#ifndef MEMCLUSTER_LITTLE_MALLOC
#define MEMCLUSTER_BIG_MALLOC 1
#define NUM_BASIC_BLOCKS 64
#endif

struct stats {
	u_long			gets;
	u_long			totalgets;
	u_long			blocks;
	u_long			freefrags;
};

/* Private data. */

static size_t			max_size;
static size_t			mem_target;
static size_t			mem_target_half;
static size_t			mem_target_fudge;
static memcluster_element **	freelists;
#ifdef MEMCLUSTER_BIG_MALLOC
static memcluster_element *	basic_blocks;
#endif
static struct stats *		stats;

/* Forward. */

static size_t			quantize(size_t);

/* Public. */

int
meminit(size_t init_max_size, size_t target_size) {
	int i;

	if (freelists != NULL) {
		errno = EEXIST;
		return (-1);
	}
	if (init_max_size == 0)
		max_size = DEF_MAX_SIZE;
	else
		max_size = init_max_size;
	if (target_size == 0)
		mem_target = DEF_MEM_TARGET;
	else
		mem_target = target_size;
	mem_target_half = mem_target / 2;
	mem_target_fudge = mem_target + mem_target / 4;
	freelists = malloc(max_size * sizeof (memcluster_element *));
	stats = malloc((max_size+1) * sizeof (struct stats));
	if (freelists == NULL || stats == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	memset(freelists, 0,
	       max_size * sizeof (memcluster_element *));
	memset(stats, 0, (max_size + 1) * sizeof (struct stats));
#ifdef MEMCLUSTER_BIG_MALLOC
	basic_blocks = NULL;
#endif
	return (0);
}

void *
__memget(size_t size) {
	size_t new_size = quantize(size);
	memcluster_element *e;
	char *p;
	void *ret;

	if (freelists == NULL)
		if (meminit(0, 0) == -1)
			return (NULL);
	if (size == 0) {
		errno = EINVAL;
		return (NULL);
	}
	if (size >= max_size || new_size >= max_size) {
		/* memget() was called on something beyond our upper limit. */
		stats[max_size].gets++;
		stats[max_size].totalgets++;
#if defined(DEBUGGING_MEMCLUSTER)
		e = malloc(new_size);
		e->next = NULL;
		e->size = new_size;
		e->fencepost = FRONT_FENCEPOST;
		p = (char *)e + sizeof *e + size;
		*((int*)p) = BACK_FENCEPOST;
		return ((char *)e + sizeof *e);
#else
		return (malloc(size));
#endif
	}

	/* 
	 * If there are no blocks in the free list for this size, get a chunk
	 * of memory and then break it up into "new_size"-sized blocks, adding
	 * them to the free list.
	 */
	if (freelists[new_size] == NULL) {
		int i, frags;
		size_t total_size;
		void *new;
		char *curr, *next;

#ifdef MEMCLUSTER_BIG_MALLOC
		if (basic_blocks == NULL) {
			new = malloc(NUM_BASIC_BLOCKS * mem_target);
			if (new == NULL) {
				errno = ENOMEM;
				return (NULL);
			}
			curr = new;
			next = curr + mem_target;
			for (i = 0; i < (NUM_BASIC_BLOCKS - 1); i++) {
				((memcluster_element *)curr)->next = next;
				curr = next;
				next += mem_target;
			}
			/*
			 * curr is now pointing at the last block in the
			 * array.
			 */
			((memcluster_element *)curr)->next = NULL;
			basic_blocks = new;
		}
		total_size = mem_target;
		new = basic_blocks;
		basic_blocks = basic_blocks->next;
#else
		if (new_size > mem_target_half)
			total_size = mem_target_fudge;
		else
			total_size = mem_target;
		new = malloc(total_size);
		if (new == NULL) {
			errno = ENOMEM;
			return (NULL);
		}
#endif
		frags = total_size / new_size;
		stats[new_size].blocks++;
		stats[new_size].freefrags += frags;
		/* Set up a linked-list of blocks of size "new_size". */
		curr = new;
		next = curr + new_size;
		for (i = 0; i < (frags - 1); i++) {
			((memcluster_element *)curr)->next = next;
			curr = next;
			next += new_size;
		}
		/* curr is now pointing at the last block in the array. */
		((memcluster_element *)curr)->next = freelists[new_size];
		freelists[new_size] = new;
	}

	/* The free list uses the "rounded-up" size "new_size". */
#if defined (DEBUGGING_MEMCLUSTER)
	e = freelists[new_size];
#else
	ret = freelists[new_size];
#endif
	freelists[new_size] = freelists[new_size]->next;
#if defined(DEBUGGING_MEMCLUSTER)
	e->next = NULL;
	e->size = new_size;
	e->fencepost = FRONT_FENCEPOST;
	p = (char *)e + sizeof *e + size;
	*((int*)p) = BACK_FENCEPOST;
#endif

	/* 
	 * The stats[] uses the _actual_ "size" requested by the
	 * caller, with the caveat (in the code above) that "size" >= the
	 * max. size (max_size) ends up getting recorded as a call to
	 * max_size.
	 */
	stats[size].gets++;
	stats[size].totalgets++;
	stats[new_size].freefrags--;
#if defined(DEBUGGING_MEMCLUSTER)
	return ((char *)e + sizeof *e);
#else
	return (ret);
#endif
}

/* 
 * This is a call from an external caller, 
 * so we want to count this as a user "put". 
 */
void
__memput(void *mem, size_t size) {
	size_t new_size = quantize(size);
	memcluster_element *e;
	char *p;

	REQUIRE(freelists != NULL);

	if (size == 0) {
		errno = EINVAL;
		return;
	}

#if defined (DEBUGGING_MEMCLUSTER)
	e = (memcluster_element *) ((char *)mem - sizeof *e);
	INSIST(e->fencepost == FRONT_FENCEPOST);
	INSIST(e->size == new_size);
	p = (char *)e + sizeof *e + size;
	INSIST(*((int *)p) == BACK_FENCEPOST);
#endif

	if (size == max_size || new_size >= max_size) {
		/* memput() called on something beyond our upper limit */
#if defined(DEBUGGING_MEMCLUSTER)
		free(e);
#else
		free(mem);
#endif

		INSIST(stats[max_size].gets != 0);
		stats[max_size].gets--;
		return;
	}

	/* The free list uses the "rounded-up" size "new_size": */
#if defined(DEBUGGING_MEMCLUSTER)
	e->next = freelists[new_size];
	freelists[new_size] = (void *)e;
#else
	((memcluster_element *)mem)->next = freelists[new_size];
	freelists[new_size] = (memcluster_element *)mem;
#endif

	/* 
	 * The stats[] uses the _actual_ "size" requested by the
	 * caller, with the caveat (in the code above) that "size" >= the
	 * max. size (max_size) ends up getting recorded as a call to
	 * max_size.
	 */
	INSIST(stats[size].gets != 0);
	stats[size].gets--;
	stats[new_size].freefrags++;
}

void *
__memget_debug(size_t size, const char *file, int line) {
	void *ptr;
	ptr = __memget(size);
	fprintf(stderr, "%s:%d: memget(%lu) -> %p\n", file, line,
		(u_long)size, ptr);
	return (ptr);
}

void
__memput_debug(void *ptr, size_t size, const char *file, int line) {
	fprintf(stderr, "%s:%d: memput(%p, %lu)\n", file, line, ptr,
		(u_long)size);
	__memput(ptr, size);
}

/*
 * Print the stats[] on the stream "out" with suitable formatting.
 */
void
memstats(FILE *out) {
	size_t i;

	if (freelists == NULL)
		return;
	for (i = 1; i <= max_size; i++) {
		const struct stats *s = &stats[i];

		if (s->totalgets == 0 && s->gets == 0)
			continue;
		fprintf(out, "%s%5d: %11lu gets, %11lu rem",
			(i == max_size) ? ">=" : "  ",
			i, s->totalgets, s->gets);
		if (s->blocks != 0)
			fprintf(out, " (%lu bl, %lu ff)",
				s->blocks, s->freefrags);
		fputc('\n', out);
	}
}

/* Private. */

/* 
 * Round up size to a multiple of sizeof(void *).  This guarantees that a
 * block is at least sizeof void *, and that we won't violate alignment
 * restrictions, both of which are needed to make lists of blocks.
 */
static size_t
quantize(size_t size) {
	int remainder;
	/*
	 * If there is no remainder for the integer division of 
	 *
	 *	(rightsize/P_SIZE)
	 *
	 * then we already have a good size; if not, then we need
	 * to round up the result in order to get a size big
	 * enough to satisfy the request _and_ aligned on P_SIZE boundaries.
	 */
	remainder = size % P_SIZE;
	if (remainder != 0)
		size += P_SIZE - remainder;
#if defined(DEBUGGING_MEMCLUSTER)
	return (size + SMALL_SIZE_LIMIT + sizeof (int));
#else
	return (size);
#endif
}

