
#ifndef __gedit_marshal_MARSHAL_H__
#define __gedit_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOLEAN:OBJECT (gedit-marshal.list:1) */
extern void gedit_marshal_BOOLEAN__OBJECT (GClosure     *closure,
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
                                           gpointer      marshal_data);

/* VOID:VOID (gedit-marshal.list:2) */
#define gedit_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:BOOLEAN (gedit-marshal.list:3) */
#define gedit_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

G_END_DECLS

#endif /* __gedit_marshal_MARSHAL_H__ */

