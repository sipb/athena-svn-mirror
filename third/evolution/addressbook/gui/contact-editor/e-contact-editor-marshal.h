
#ifndef __e_contact_editor_marshal_MARSHAL_H__
#define __e_contact_editor_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:INT,OBJECT (./e-contact-editor-marshal.list:1) */
extern void e_contact_editor_marshal_VOID__INT_OBJECT (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);
#define e_contact_editor_marshal_NONE__INT_OBJECT	e_contact_editor_marshal_VOID__INT_OBJECT

/* NONE:NONE (./e-contact-editor-marshal.list:2) */
#define e_contact_editor_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define e_contact_editor_marshal_NONE__NONE	e_contact_editor_marshal_VOID__VOID

G_END_DECLS

#endif /* __e_contact_editor_marshal_MARSHAL_H__ */

