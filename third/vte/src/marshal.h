
#ifndef ___vte_marshal_MARSHAL_H__
#define ___vte_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:VOID (marshal.list:1) */
#define _vte_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:STRING (marshal.list:2) */
#define _vte_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* VOID:STRING,UINT (marshal.list:3) */
extern void _vte_marshal_VOID__STRING_UINT (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:INT (marshal.list:4) */
#define _vte_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:INT,INT (marshal.list:5) */
extern void _vte_marshal_VOID__INT_INT (GClosure     *closure,
                                        GValue       *return_value,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint,
                                        gpointer      marshal_data);

/* VOID:UINT,UINT (marshal.list:6) */
extern void _vte_marshal_VOID__UINT_UINT (GClosure     *closure,
                                          GValue       *return_value,
                                          guint         n_param_values,
                                          const GValue *param_values,
                                          gpointer      invocation_hint,
                                          gpointer      marshal_data);

/* VOID:OBJECT,OBJECT (marshal.list:7) */
extern void _vte_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

G_END_DECLS

#endif /* ___vte_marshal_MARSHAL_H__ */

