/* Pthread-friendly coroutines
 * Copyright (C) 2002 Andy Wingo <wingo@pobox.com>
 * Portions Copyright (C) 1999-2001 Ralf S. Engelschall <rse@engelschall.com>
 *
 * config-public.h: public compile-time configuration
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

#ifndef _COTHREADS_CONFIG_PUBLIC_H_
#define _COTHREADS_CONFIG_PUBLIC_H_



/* sig{set,long}jmp macros */
#define cothreads_sigjmpbuf 
#define cothreads_sigsetjmp(buf) 
#define cothreads_siglongjmp(buf,val) 

/* stack setup macros */
#define pth_skaddr_sigstack(skaddr,sksize) ((skaddr))
#define pth_sksize_sigstack(skaddr,sksize) ((sksize))
#define pth_skaddr_sigaltstack(skaddr,sksize) ((skaddr))
#define pth_sksize_sigaltstack(skaddr,sksize) ((sksize))
#define pth_skaddr_makecontext(skaddr,sksize) ((skaddr))
#define pth_sksize_makecontext(skaddr,sksize) ((sksize))

/* mctx compile defines */
#define COTHREADS_MCTX_MTH(which)  (COTHREADS_MCTX_MTH_use == (COTHREADS_MCTX_MTH_##which))
#define COTHREADS_MCTX_DSP(which)  (COTHREADS_MCTX_DSP_use == (COTHREADS_MCTX_DSP_##which))
#define COTHREADS_MCTX_STK(which)  (COTHREADS_MCTX_STK_use == (COTHREADS_MCTX_STK_##which))
#define COTHREADS_MCTX_MTH_mcsc    1
#define COTHREADS_MCTX_MTH_sjlj    2
#define COTHREADS_MCTX_DSP_sc      1
#define COTHREADS_MCTX_DSP_ssjlj   2
#define COTHREADS_MCTX_DSP_sjlj    3
#define COTHREADS_MCTX_DSP_usjlj   4
#define COTHREADS_MCTX_DSP_sjlje   5
#define COTHREADS_MCTX_DSP_sjljlx  6
#define COTHREADS_MCTX_DSP_sjljisc 7
#define COTHREADS_MCTX_DSP_sjljw32 8
#define COTHREADS_MCTX_STK_mc      1
#define COTHREADS_MCTX_STK_ss      2
#define COTHREADS_MCTX_STK_sas     3
#define COTHREADS_MCTX_STK_none    4

#define COTHREADS_MCTX_MTH_use COTHREADS_MCTX_MTH_mcsc
#define COTHREADS_MCTX_DSP_use COTHREADS_MCTX_DSP_sc
#define COTHREADS_MCTX_STK_use COTHREADS_MCTX_STK_mc

#endif /* _COTHREADS_CONFIG_PUBLIC_H_ */

