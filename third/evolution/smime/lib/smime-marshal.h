
#ifndef __smime_marshal_MARSHAL_H__
#define __smime_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* BOOL:POINTER,BOOL,POINTER (./smime-marshal.list:1) */
extern void smime_marshal_BOOLEAN__POINTER_BOOLEAN_POINTER (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);
#define smime_marshal_BOOL__POINTER_BOOL_POINTER	smime_marshal_BOOLEAN__POINTER_BOOLEAN_POINTER

/* BOOL:POINTER,POINTER (./smime-marshal.list:2) */
extern void smime_marshal_BOOLEAN__POINTER_POINTER (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);
#define smime_marshal_BOOL__POINTER_POINTER	smime_marshal_BOOLEAN__POINTER_POINTER

/* BOOL:POINTER,POINTER,POINTER,POINTER (./smime-marshal.list:3) */
extern void smime_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER (GClosure     *closure,
                                                                    GValue       *return_value,
                                                                    guint         n_param_values,
                                                                    const GValue *param_values,
                                                                    gpointer      invocation_hint,
                                                                    gpointer      marshal_data);
#define smime_marshal_BOOL__POINTER_POINTER_POINTER_POINTER	smime_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER

G_END_DECLS

#endif /* __smime_marshal_MARSHAL_H__ */

