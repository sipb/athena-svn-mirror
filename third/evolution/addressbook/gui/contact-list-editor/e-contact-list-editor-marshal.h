
#ifndef __e_contact_list_editor_marshal_MARSHAL_H__
#define __e_contact_list_editor_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* INT:OBJECT (./e-contact-list-editor-marshal.list:1) */
extern void e_contact_list_editor_marshal_INT__OBJECT (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* NONE:INT,OBJECT (./e-contact-list-editor-marshal.list:2) */
extern void e_contact_list_editor_marshal_VOID__INT_OBJECT (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);
#define e_contact_list_editor_marshal_NONE__INT_OBJECT	e_contact_list_editor_marshal_VOID__INT_OBJECT

/* NONE:INT,OBJECT (./e-contact-list-editor-marshal.list:3) */

/* NONE:NONE (./e-contact-list-editor-marshal.list:4) */
#define e_contact_list_editor_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID
#define e_contact_list_editor_marshal_NONE__NONE	e_contact_list_editor_marshal_VOID__VOID

G_END_DECLS

#endif /* __e_contact_list_editor_marshal_MARSHAL_H__ */

