/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 * 
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GSSAPI_GENERIC_H_
#define _GSSAPI_GENERIC_H_

/*
 * $Id: gssapi_generic.h,v 1.1.1.2 1999-05-03 21:54:09 danw Exp $
 */

#if defined(__MWERKS__) || defined(applec) || defined(THINK_C)
#include <gssapi.h>
#else
#include <gssapi/gssapi.h>
#endif

extern gss_OID gss_nt_user_name;
extern gss_OID gss_nt_machine_uid_name;
extern gss_OID gss_nt_string_uid_name;
extern gss_OID gss_nt_service_name;
extern gss_OID gss_nt_exported_name;
extern gss_OID gss_nt_service_name_v2;

#endif /* _GSSAPI_GENERIC_H_ */
