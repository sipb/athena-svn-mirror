
#ifndef __e_calendar_marshal_MARSHAL_H__
#define __e_calendar_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:INT (./e-calendar-marshal.list:1) */
#define e_calendar_marshal_VOID__INT	g_cclosure_marshal_VOID__INT
#define e_calendar_marshal_NONE__INT	e_calendar_marshal_VOID__INT

/* NONE:POINTER (./e-calendar-marshal.list:2) */
#define e_calendar_marshal_VOID__POINTER	g_cclosure_marshal_VOID__POINTER
#define e_calendar_marshal_NONE__POINTER	e_calendar_marshal_VOID__POINTER

/* NONE:OBJECT (./e-calendar-marshal.list:3) */
#define e_calendar_marshal_VOID__OBJECT	g_cclosure_marshal_VOID__OBJECT
#define e_calendar_marshal_NONE__OBJECT	e_calendar_marshal_VOID__OBJECT

/* NONE:INT,STRING (./e-calendar-marshal.list:4) */
extern void e_calendar_marshal_VOID__INT_STRING (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define e_calendar_marshal_NONE__INT_STRING	e_calendar_marshal_VOID__INT_STRING

/* NONE:INT,BOOL (./e-calendar-marshal.list:5) */
extern void e_calendar_marshal_VOID__INT_BOOLEAN (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);
#define e_calendar_marshal_NONE__INT_BOOL	e_calendar_marshal_VOID__INT_BOOLEAN

/* NONE:INT,POINTER (./e-calendar-marshal.list:6) */
extern void e_calendar_marshal_VOID__INT_POINTER (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);
#define e_calendar_marshal_NONE__INT_POINTER	e_calendar_marshal_VOID__INT_POINTER

/* NONE:INT,OBJECT (./e-calendar-marshal.list:7) */
extern void e_calendar_marshal_VOID__INT_OBJECT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define e_calendar_marshal_NONE__INT_OBJECT	e_calendar_marshal_VOID__INT_OBJECT

/* NONE:STRING,INT (./e-calendar-marshal.list:8) */
extern void e_calendar_marshal_VOID__STRING_INT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);
#define e_calendar_marshal_NONE__STRING_INT	e_calendar_marshal_VOID__STRING_INT

/* NONE:INT,INT (./e-calendar-marshal.list:9) */
extern void e_calendar_marshal_VOID__INT_INT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);
#define e_calendar_marshal_NONE__INT_INT	e_calendar_marshal_VOID__INT_INT

/* NONE:ENUM,ENUM (./e-calendar-marshal.list:10) */
extern void e_calendar_marshal_VOID__ENUM_ENUM (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);
#define e_calendar_marshal_NONE__ENUM_ENUM	e_calendar_marshal_VOID__ENUM_ENUM

/* NONE:ENUM,STRING (./e-calendar-marshal.list:11) */
extern void e_calendar_marshal_VOID__ENUM_STRING (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);
#define e_calendar_marshal_NONE__ENUM_STRING	e_calendar_marshal_VOID__ENUM_STRING

/* NONE:STRING,BOOL,INT,INT (./e-calendar-marshal.list:12) */
extern void e_calendar_marshal_VOID__STRING_BOOLEAN_INT_INT (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);
#define e_calendar_marshal_NONE__STRING_BOOL_INT_INT	e_calendar_marshal_VOID__STRING_BOOLEAN_INT_INT

/* NONE:STRING,STRING (./e-calendar-marshal.list:13) */
extern void e_calendar_marshal_VOID__STRING_STRING (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);
#define e_calendar_marshal_NONE__STRING_STRING	e_calendar_marshal_VOID__STRING_STRING

/* NONE:STRING,STRING,STRING (./e-calendar-marshal.list:14) */
extern void e_calendar_marshal_VOID__STRING_STRING_STRING (GClosure     *closure,
                                                           GValue       *return_value,
                                                           guint         n_param_values,
                                                           const GValue *param_values,
                                                           gpointer      invocation_hint,
                                                           gpointer      marshal_data);
#define e_calendar_marshal_NONE__STRING_STRING_STRING	e_calendar_marshal_VOID__STRING_STRING_STRING

/* NONE:POINTER,ENUM (./e-calendar-marshal.list:15) */
extern void e_calendar_marshal_VOID__POINTER_ENUM (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);
#define e_calendar_marshal_NONE__POINTER_ENUM	e_calendar_marshal_VOID__POINTER_ENUM

/* NONE:POINTER,STRING (./e-calendar-marshal.list:16) */
extern void e_calendar_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                                     GValue       *return_value,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint,
                                                     gpointer      marshal_data);
#define e_calendar_marshal_NONE__POINTER_STRING	e_calendar_marshal_VOID__POINTER_STRING

/* NONE:POINTER,POINTER (./e-calendar-marshal.list:17) */
extern void e_calendar_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);
#define e_calendar_marshal_NONE__POINTER_POINTER	e_calendar_marshal_VOID__POINTER_POINTER

/* NONE:LONG,LONG (./e-calendar-marshal.list:18) */
extern void e_calendar_marshal_VOID__LONG_LONG (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);
#define e_calendar_marshal_NONE__LONG_LONG	e_calendar_marshal_VOID__LONG_LONG

G_END_DECLS

#endif /* __e_calendar_marshal_MARSHAL_H__ */

