/* $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_65/include/rx/rx_null.h,v 1.1.1.3 2000-04-12 19:45:39 ghudson Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_65/include/rx/rx_null.h,v $ */

/*
****************************************************************************
*        Copyright IBM Corporation 1988, 1989 - All Rights Reserved        *
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appear in all copies and        *
* that both that copyright notice and this permission notice appear in     *
* supporting documentation, and that the name of IBM not be used in        *
* advertising or publicity pertaining to distribution of the software      *
* without specific, written prior permission.                              *
*                                                                          *
* IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL IBM *
* BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY      *
* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER  *
* IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING   *
* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.    *
****************************************************************************
*/

#ifndef RX_NULL_HEADER
#define RX_NULL_HEADER

#ifdef	KERNEL
#include "../rx/rx.h"
#else /* KERNEL */
#include "rx.h"
#endif /* KERNEL */

/* The null security object.  No authentication, no nothing. */

extern struct rx_securityClass *rxnull_NewServerSecurityObject();
extern struct rx_securityClass *rxnull_NewClientSecurityObject();

#endif /* RX_NULL_HEADER */
