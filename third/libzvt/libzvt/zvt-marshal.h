
#ifndef __zvt_marshal_MARSHAL_H__
#define __zvt_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT,POINTER (zvt-marshal.list:1) */
extern void zvt_marshal_VOID__INT_POINTER (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

/* VOID:POINTER,INT (zvt-marshal.list:2) */
extern void zvt_marshal_VOID__POINTER_INT (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

/* VOID:VOID (zvt-marshal.list:3) */
#define zvt_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:INT (zvt-marshal.list:4) */
#define zvt_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

G_END_DECLS

#endif /* __zvt_marshal_MARSHAL_H__ */

