
#ifndef __fr_marshal_MARSHAL_H__
#define __fr_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT (fr-marshal.list:1) */
#define fr_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:INT,POINTER (fr-marshal.list:2) */
extern void fr_marshal_VOID__INT_POINTER (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:POINTER (fr-marshal.list:3) */
#define fr_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:VOID (fr-marshal.list:4) */
#define fr_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

G_END_DECLS

#endif /* __fr_marshal_MARSHAL_H__ */

