
#ifndef __gucharmap_marshal_MARSHAL_H__
#define __gucharmap_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:UINT (gucharmap-marshal.list:2) */
#define gucharmap_marshal_VOID__UINT	g_cclosure_marshal_VOID__UINT

/* VOID:VOID (gucharmap-marshal.list:3) */
#define gucharmap_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:STRING (gucharmap-marshal.list:4) */
#define gucharmap_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:UINT,UINT (gucharmap-marshal.list:5) */
extern void gucharmap_marshal_VOID__UINT_UINT (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

G_END_DECLS

#endif /* __gucharmap_marshal_MARSHAL_H__ */

