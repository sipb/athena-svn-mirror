/* Pthread-friendly coroutines
 * Copyright (C) 2002 Andy Wingo <wingo@pobox.com>
 * Portions Copyright (C) 1999-2001 Ralf S. Engelschall <rse@engelschall.com>
 *
 * cothreads.h: public API
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __COTHREADS_H__
#define __COTHREADS_H__

#include <glib.h>
#include <cothreads/cothreads-pth.h>


typedef cothreads_mctx_t cothread;
typedef void (*cothread_func) (int, void**);


gboolean	cothreads_initialized	(void);
gboolean	cothreads_init		(glong stack_size, gint ncothreads);
gboolean	cothreads_init_thread	(glong stack_size, gint ncothreads);

cothread*	cothread_create		(cothread_func func, int argc, void **argv, cothread *main);
cothread*	cothread_self		(void);
void		cothread_reset		(cothread *self, cothread_func func, int argc, void **argv, cothread *main);
void		cothread_setfunc	(cothread *self, cothread_func func, int argc, void **argv, cothread *main);
void		cothread_destroy	(cothread *thread);

gboolean	cothreads_alloc_thread_stack (void **stack, glong *stacksize);


/* 'old' and 'new' are of type (cothread*) */
#define cothread_switch(old,new) cothreads_mctx_switch((old),(new))
#define	cothread_yield(new) cothreads_mctx_restore(new);


#endif /* __COTHREADS_H__ */
