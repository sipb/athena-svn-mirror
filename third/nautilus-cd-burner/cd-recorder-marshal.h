
#ifndef __cdrecorder_marshal_MARSHAL_H__
#define __cdrecorder_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:BOOLEAN,BOOLEAN,BOOLEAN (./cdrecorder-marshal.list:1) */
extern void cdrecorder_marshal_BOOLEAN__BOOLEAN_BOOLEAN_BOOLEAN (GClosure     *closure,
                                                                 GValue       *return_value,
                                                                 guint         n_param_values,
                                                                 const GValue *param_values,
                                                                 gpointer      invocation_hint,
                                                                 gpointer      marshal_data);

/* VOID:INT,INT (./cdrecorder-marshal.list:2) */
extern void cdrecorder_marshal_VOID__INT_INT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

G_END_DECLS

#endif /* __cdrecorder_marshal_MARSHAL_H__ */

