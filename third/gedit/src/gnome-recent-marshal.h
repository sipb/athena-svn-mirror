
#ifndef __gnome_recent_MARSHAL_H__
#define __gnome_recent_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOL:STRING (gnome-recent-marshal.list:1) */
extern void gnome_recent_BOOLEAN__STRING (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);
#define gnome_recent_BOOL__STRING	gnome_recent_BOOLEAN__STRING

G_END_DECLS

#endif /* __gnome_recent_MARSHAL_H__ */

