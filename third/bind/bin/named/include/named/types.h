/*
 * Copyright (C) 1999-2001  Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 * INTERNET SOFTWARE CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: types.h,v 1.1.1.2 2002-02-03 04:22:39 ghudson Exp $ */

#ifndef NAMED_TYPES_H
#define NAMED_TYPES_H 1

#include <dns/types.h>

typedef struct ns_client		ns_client_t;
typedef struct ns_clientmgr		ns_clientmgr_t;
typedef struct ns_query			ns_query_t;
typedef struct ns_server 		ns_server_t;
typedef struct ns_interface 		ns_interface_t;
typedef struct ns_interfacemgr		ns_interfacemgr_t;
typedef struct ns_lwresd		ns_lwresd_t;
typedef struct ns_lwreslistener		ns_lwreslistener_t;
typedef struct ns_lwdclient		ns_lwdclient_t;
typedef struct ns_lwdclientmgr		ns_lwdclientmgr_t;
typedef struct ns_lwsearchlist		ns_lwsearchlist_t;
typedef struct ns_lwsearchctx		ns_lwsearchctx_t;
typedef struct ns_controls		ns_controls_t;

#endif /* NAMED_TYPES_H */
