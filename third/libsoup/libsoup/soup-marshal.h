
#ifndef __soup_marshal_MARSHAL_H__
#define __soup_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:NONE (./soup-marshal.list:1) */
#define soup_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define soup_marshal_NONE__NONE	soup_marshal_VOID__VOID

/* NONE:INT (./soup-marshal.list:2) */
#define soup_marshal_VOID__INT	g_cclosure_marshal_VOID__INT
#define soup_marshal_NONE__INT	soup_marshal_VOID__INT

/* NONE:OBJECT (./soup-marshal.list:3) */
#define soup_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT
#define soup_marshal_NONE__OBJECT	soup_marshal_VOID__OBJECT

/* NONE:OBJECT,STRING,STRING,POINTER,POINTER (./soup-marshal.list:4) */
extern void soup_marshal_VOID__OBJECT_STRING_STRING_POINTER_POINTER (GClosure     *closure,
                                                                     GValue       *return_value,
                                                                     guint         n_param_values,
                                                                     const GValue *param_values,
                                                                     gpointer      invocation_hint,
                                                                     gpointer      marshal_data);
#define soup_marshal_NONE__OBJECT_STRING_STRING_POINTER_POINTER	soup_marshal_VOID__OBJECT_STRING_STRING_POINTER_POINTER

G_END_DECLS

#endif /* __soup_marshal_MARSHAL_H__ */

