/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-typecodes.h: Type code definitions for WSDL types
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_TYPECODES_H_
#define _WSDL_TYPECODES_H_

#include <glib.h>

typedef enum {
	WSDL_TK_GLIB_NULL=0,
	WSDL_TK_GLIB_VOID,
	WSDL_TK_GLIB_BOOLEAN,
	WSDL_TK_GLIB_CHAR,
	WSDL_TK_GLIB_UCHAR,
	WSDL_TK_GLIB_INT,
	WSDL_TK_GLIB_UINT,
	WSDL_TK_GLIB_SHORT,
	WSDL_TK_GLIB_USHORT,
	WSDL_TK_GLIB_LONG,
	WSDL_TK_GLIB_ULONG,
	WSDL_TK_GLIB_INT8,
	WSDL_TK_GLIB_UINT8,
	WSDL_TK_GLIB_INT16,
	WSDL_TK_GLIB_UINT16,
	WSDL_TK_GLIB_INT32,
	WSDL_TK_GLIB_UINT32,
	WSDL_TK_GLIB_FLOAT,
	WSDL_TK_GLIB_DOUBLE,
	WSDL_TK_GLIB_STRING,
	WSDL_TK_GLIB_ELEMENT,
	WSDL_TK_GLIB_STRUCT,
	WSDL_TK_GLIB_LIST,
	WSDL_TK_GLIB_MAX
} wsdl_typecode_kind_t;

typedef void (*WsdlTypecodeFreeFn) (gpointer);

typedef struct _wsdl_typecode wsdl_typecode;
struct _wsdl_typecode {
	wsdl_typecode_kind_t   kind;
	const guchar          *name;
	const guchar          *ns;	/* used in the typecode struct name */
	const guchar          *nsuri;	/* used when looking up ns-qualified codes */
	
	gboolean              dynamic;
	
	gulong                sub_parts;
	const guchar        **subnames;	/* for struct */
	const wsdl_typecode **subtypes;	/* for struct, list, element */

	WsdlTypecodeFreeFn    free_func;
};

#define ALIGN_ADDRESS(this, boundary)                                      \
        ((gpointer)                                                        \
	 (( (((unsigned long) (this)) + (((unsigned long) (boundary)) -1)) \
	    & (~(((unsigned long) (boundary)) - 1)) )) )

extern const wsdl_typecode WSDL_TC_glib_null_struct;
extern const wsdl_typecode WSDL_TC_glib_void_struct;
extern const wsdl_typecode WSDL_TC_glib_boolean_struct;
extern const wsdl_typecode WSDL_TC_glib_char_struct;
extern const wsdl_typecode WSDL_TC_glib_uchar_struct;
extern const wsdl_typecode WSDL_TC_glib_int_struct;
extern const wsdl_typecode WSDL_TC_glib_uint_struct;
extern const wsdl_typecode WSDL_TC_glib_short_struct;
extern const wsdl_typecode WSDL_TC_glib_ushort_struct;
extern const wsdl_typecode WSDL_TC_glib_long_struct;
extern const wsdl_typecode WSDL_TC_glib_ulong_struct;
extern const wsdl_typecode WSDL_TC_glib_int8_struct;
extern const wsdl_typecode WSDL_TC_glib_uint8_struct;
extern const wsdl_typecode WSDL_TC_glib_int16_struct;
extern const wsdl_typecode WSDL_TC_glib_uint16_struct;
extern const wsdl_typecode WSDL_TC_glib_int32_struct;
extern const wsdl_typecode WSDL_TC_glib_uint32_struct;
extern const wsdl_typecode WSDL_TC_glib_float_struct;
extern const wsdl_typecode WSDL_TC_glib_double_struct;
extern const wsdl_typecode WSDL_TC_glib_string_struct;

wsdl_typecode_kind_t wsdl_typecode_kind           (const wsdl_typecode *const  tc);
wsdl_typecode_kind_t wsdl_typecode_element_kind   (const wsdl_typecode *const  tc);
gboolean             wsdl_typecode_is_simple      (const wsdl_typecode *const  tc);
guint                wsdl_typecode_member_count   (const wsdl_typecode *const  tc);
const guchar *       wsdl_typecode_member_name    (const wsdl_typecode *const  tc,
						   guint                       member);
const wsdl_typecode *wsdl_typecode_member_type    (const wsdl_typecode *const  tc,
						   guint                       member);
const wsdl_typecode *wsdl_typecode_content_type   (const wsdl_typecode *const  tc);
const guchar *       wsdl_typecode_name           (const wsdl_typecode *const  tc);
const guchar *       wsdl_typecode_ns             (const wsdl_typecode *const  tc);
const guchar *       wsdl_typecode_nsuri          (const wsdl_typecode *const  tc);
guchar *             wsdl_typecode_type           (const wsdl_typecode *const  tc);
guchar *             wsdl_typecode_param_type     (const wsdl_typecode *const  tc);
void                 wsdl_typecode_print          (const wsdl_typecode *const  tc,
						   guint                       ind);
const wsdl_typecode *wsdl_typecode_lookup         (const guchar               *name,
						   const guchar               *nsuri);
void                 wsdl_typecode_register       (const wsdl_typecode *const  tc);
void                 wsdl_typecode_unregister     (const guchar               *name,
						   const guchar               *nsuri);
void                 wsdl_typecode_free           (wsdl_typecode              *tc);
void                 wsdl_typecode_free_all       (void);

typedef void (*WsdlTypecodeForeachFn) (const wsdl_typecode *const tc,
				       gpointer                   user_data);

void                 wsdl_typecode_foreach        (gboolean                    predefined,
						   WsdlTypecodeForeachFn       callback,
						   gpointer                    user_data);
gpointer             wsdl_typecode_alloc          (const wsdl_typecode *const  tc);
guint                wsdl_typecode_find_alignment (const wsdl_typecode *const  tc);
guint                wsdl_typecode_size           (const wsdl_typecode *const  tc);
const wsdl_typecode *wsdl_typecode_offset         (const wsdl_typecode *const  tc,
						   const guchar               *name,
						   guint                      *offset);

#endif /* _WSDL_TYPECODES_H_ */
