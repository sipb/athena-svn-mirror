
#ifndef __libgnomeprintui_marshal_MARSHAL_H__
#define __libgnomeprintui_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT,BOOLEAN (gnome-printui-marshal.list:1) */
extern void libgnomeprintui_marshal_VOID__INT_BOOLEAN (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);

/* VOID:VOID (gnome-printui-marshal.list:2) */
#define libgnomeprintui_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:BOOLEAN (gnome-printui-marshal.list:3) */
#define libgnomeprintui_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:POINTER (gnome-printui-marshal.list:4) */
#define libgnomeprintui_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* VOID:UINT,UINT (gnome-printui-marshal.list:5) */
extern void libgnomeprintui_marshal_VOID__UINT_UINT (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);

G_END_DECLS

#endif /* __libgnomeprintui_marshal_MARSHAL_H__ */

