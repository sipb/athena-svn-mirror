/*
 * memchunk.h
 *
 * Beecrypt memory block handling, header
 *
 * Copyright (c) 2001 Virtual Unlimited B.V.
 *
 * Author: Bob Deblier <bob@virtualunlimited.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MEMCHUNK_H
#define _MEMCHUNK_H

#include "types.h"

typedef struct
{
	int		size;
/*@only@*/ byte*	data;
} memchunk;

#ifdef __cplusplus
extern "C" {
#endif

/**
 */
BEECRYPTAPI /*@only@*/ /*@null@*/
memchunk* memchunkAlloc(int size)
	/*@*/;

/**
 */
BEECRYPTAPI
/*@unused@*/ void memchunkFree(/*@only@*/ /*@null@*/memchunk* m)
	/*@*/;

/**
 */
BEECRYPTAPI /*@only@*/ /*@null@*/
memchunk* memchunkResize(/*@only@*/ /*@null@*/memchunk* m, int size)
	/*@*/;

/**
 */
BEECRYPTAPI /*@only@*/ /*@null@*/ /*@unused@*/
memchunk*	memchunkClone(const memchunk* m);

#ifdef __cplusplus
}
#endif

#endif