
#ifndef __eog_collection_marshal_MARSHAL_H__
#define __eog_collection_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOL:INT,POINTER (eog-collection-marshal.list:1) */
extern void eog_collection_marshal_BOOLEAN__INT_POINTER (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);
#define eog_collection_marshal_BOOL__INT_POINTER	eog_collection_marshal_BOOLEAN__INT_POINTER

/* VOID:INT (eog-collection-marshal.list:2) */
#define eog_collection_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:POINTER (eog-collection-marshal.list:3) */
#define eog_collection_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER

/* OBJECT:OBJECT,UINT (eog-collection-marshal.list:4) */
extern void eog_collection_marshal_OBJECT__OBJECT_UINT (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:OBJECT,OBJECT,INT (eog-collection-marshal.list:5) */
extern void eog_collection_marshal_VOID__OBJECT_OBJECT_INT (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

/* VOID:POINTER,POINTER (eog-collection-marshal.list:6) */
extern void eog_collection_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                          GValue       *return_value,
                                                          guint         n_param_values,
                                                          const GValue *param_values,
                                                          gpointer      invocation_hint,
                                                          gpointer      marshal_data);

G_END_DECLS

#endif /* __eog_collection_marshal_MARSHAL_H__ */

