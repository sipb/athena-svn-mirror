
#ifndef __gedit_marshal_MARSHAL_H__
#define __gedit_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING (gedit-spell-checker-dialog-marshal.list:1) */
#define gedit_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:STRING,STRING (gedit-spell-checker-dialog-marshal.list:2) */
extern void gedit_marshal_VOID__STRING_STRING (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

G_END_DECLS

#endif /* __gedit_marshal_MARSHAL_H__ */

