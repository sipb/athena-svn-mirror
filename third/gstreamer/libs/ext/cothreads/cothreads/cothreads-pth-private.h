/* Pthread-friendly coroutines with pth
 * Copyright (C) 2002 Andy Wingo <wingo@pobox.com>
 *
 * cothreads-pth-private.h: pth compatibility code
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


#ifndef __COTHREADS_PTH_PRIVATE_H__
#define __COTHREADS_PTH_PRIVATE_H__


#ifndef CURRENT_STACK_FRAME
#define CURRENT_STACK_FRAME  ({ char __csf; &__csf; })
#endif /* CURRENT_STACK_FRAME */

/* wrap the pth namespace in our own so that publicly we present only
   cothreads_* */
#ifdef COTHREADS_DEBUG
#define PTH_DEBUG
#endif
#define pth_mctx_t cothread
#define pth_mctx_save cothreads_mctx_save
#define pth_mctx_switch cothreads_mctx_switch
#define pth_mctx_restore cothreads_mctx_restore
#define pth_mctx_restored cothreads_mctx_restored
#define PTH_MCTX_MTH COTHREADS_MCTX_MTH
#define PTH_MCTX_DSP COTHREADS_MCTX_DSP
#define PTH_MCTX_STK COTHREADS_MCTX_STK
#define pth_sigjmpbuf cothreads_sigjmpbuf
#define pth_sigsetjmp cothreads_sigsetjmp
#define pth_siglongjmp cothreads_siglongjmp
#define pth_skaddr(func,skaddr,sksize) pth_skaddr_##func(skaddr,sksize)
#define pth_sksize(func,skaddr,sksize) pth_sksize_##func(skaddr,sksize)
#define PTH_STACKGROWTH COTHREADS_STACKGROWTH

int pth_mctx_set(pth_mctx_t *, void (*)(void), char *, char *);


#endif /* __COTHREADS_PTH_PRIVATE_H__ */
