
#ifndef __cal_util_marshal_MARSHAL_H__
#define __cal_util_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:INT,INT (cal-util-marshal.list:1) */
extern void cal_util_marshal_VOID__INT_INT (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);
#define cal_util_marshal_NONE__INT_INT	cal_util_marshal_VOID__INT_INT

/* NONE:ENUM,ENUM (cal-util-marshal.list:2) */
extern void cal_util_marshal_VOID__ENUM_ENUM (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);
#define cal_util_marshal_NONE__ENUM_ENUM	cal_util_marshal_VOID__ENUM_ENUM

/* NONE:ENUM,STRING (cal-util-marshal.list:3) */
extern void cal_util_marshal_VOID__ENUM_STRING (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);
#define cal_util_marshal_NONE__ENUM_STRING	cal_util_marshal_VOID__ENUM_STRING

/* NONE:STRING,BOOL,INT,INT (cal-util-marshal.list:4) */
extern void cal_util_marshal_VOID__STRING_BOOLEAN_INT_INT (GClosure     *closure,
                                                           GValue       *return_value,
                                                           guint         n_param_values,
                                                           const GValue *param_values,
                                                           gpointer      invocation_hint,
                                                           gpointer      marshal_data);
#define cal_util_marshal_NONE__STRING_BOOL_INT_INT	cal_util_marshal_VOID__STRING_BOOLEAN_INT_INT

/* NONE:STRING,STRING (cal-util-marshal.list:5) */
extern void cal_util_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);
#define cal_util_marshal_NONE__STRING_STRING	cal_util_marshal_VOID__STRING_STRING

/* NONE:POINTER,ENUM (cal-util-marshal.list:6) */
extern void cal_util_marshal_VOID__POINTER_ENUM (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define cal_util_marshal_NONE__POINTER_ENUM	cal_util_marshal_VOID__POINTER_ENUM

/* NONE:POINTER,STRING (cal-util-marshal.list:7) */
extern void cal_util_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);
#define cal_util_marshal_NONE__POINTER_STRING	cal_util_marshal_VOID__POINTER_STRING

/* NONE:POINTER,POINTER (cal-util-marshal.list:8) */
extern void cal_util_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);
#define cal_util_marshal_NONE__POINTER_POINTER	cal_util_marshal_VOID__POINTER_POINTER

G_END_DECLS

#endif /* __cal_util_marshal_MARSHAL_H__ */

