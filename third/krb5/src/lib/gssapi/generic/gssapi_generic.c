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

/*
 * $Id: gssapi_generic.c,v 1.1.1.3 1999-10-05 16:12:08 ghudson Exp $
 */

#include "gssapiP_generic.h"

/*
 * See krb5/gssapi_krb5.c for a description of the algorithm for
 * encoding an object identifier.
 */

/*
 * The OID of user_name is:
 * 	iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 * 	generic(1) user_name(1) = 1.2.840.113554.1.2.1.1
 * machine_uid_name:
 * 	iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 * 	generic(1) machine_uid_name(2) = 1.2.840.113554.1.2.1.2
 * string_uid_name:
 * 	iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 * 	generic(1) string_uid_name(3) = 1.2.840.113554.1.2.1.3
 * service_name:
 * 	iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2)
 * 	generic(1) service_name(4) = 1.2.840.113554.1.2.1.4
 * exported_name:
 *      1(iso), 3(org), 6(dod), 1(internet), 5(security), 6(nametypes),
 *	    4(gss-api-exported-name)
 * host_based_service_name (v2):
 *      iso (1) org (3), dod (6), internet (1), security (5), nametypes(6),
 *      gss-host-based-services(2)
 */

static gss_OID_desc oids[] = {
   {10, "\052\206\110\206\367\022\001\002\001\001"},
   {10, "\052\206\110\206\367\022\001\002\001\002"},
   {10, "\052\206\110\206\367\022\001\002\001\003"},
   {10, "\052\206\110\206\367\022\001\002\001\004"},
   { 6, "\053\006\001\005\006\004"},
   { 6, "\053\006\001\005\006\002"},
};

GSS_DLLIMP gss_OID gss_nt_user_name = oids+0;
GSS_DLLIMP gss_OID gss_nt_machine_uid_name = oids+1;
GSS_DLLIMP gss_OID gss_nt_string_uid_name = oids+2;
GSS_DLLIMP gss_OID gss_nt_service_name = oids+3;
GSS_DLLIMP gss_OID gss_nt_exported_name = oids+4;
GSS_DLLIMP gss_OID gss_nt_service_name_v2 = oids+5;
