/**
 * bonobo-property-editor.h:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_PEDITOR_H_
#define _BONOBO_PEDITOR_H_

#include <gtk/gtkbin.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo-conf/Bonobo_Config.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_PEDITOR_TYPE        (bonobo_peditor_get_type ())
#define BONOBO_PEDITOR(o)          (GTK_CHECK_CAST ((o), BONOBO_PEDITOR_TYPE, BonoboPEditor))
#define BONOBO_PEDITOR_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PEDITOR_TYPE, BonoboPEditorClass))
#define BONOBO_IS_PEDITOR(o)       (GTK_CHECK_TYPE ((o), BONOBO_PEDITOR_TYPE))
#define BONOBO_IS_PEDITOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PEDITOR_TYPE))

typedef struct _BonoboPEditorPrivate BonoboPEditorPrivate;

typedef struct {
	GtkObject          base;

	CORBA_TypeCode     tc;
	gpointer           data;

	BonoboPEditorPrivate *priv;
} BonoboPEditor;

typedef void (*BonoboPEditorSetFn) (BonoboPEditor     *editor,
				    BonoboArg         *value,
				    CORBA_Environment *ev);

typedef struct {
	GtkObjectClass parent_class;

	/* virtual methods */

	BonoboPEditorSetFn set_value;

} BonoboPEditorClass;

GtkType            
bonobo_peditor_get_type     (void);

BonoboPEditor *
bonobo_peditor_construct    (GtkWidget          *widget,
			     BonoboPEditorSetFn  set_cb,
			     CORBA_TypeCode      tc);

void                
bonobo_peditor_set_property (BonoboPEditor      *editor,
			     Bonobo_PropertyBag  bag,
			     const char         *name,
			     CORBA_TypeCode      tc, 
			     CORBA_any          *defval);

void                
bonobo_peditor_set_value    (BonoboPEditor     *editor,
			     const BonoboArg   *value,
			     CORBA_Environment *opt_ev);

void
bonobo_peditor_set_guard    (GtkWidget          *widget,
			     Bonobo_PropertyBag  bag,
			     const char         *prop_name);

GtkWidget *
bonobo_peditor_get_widget   (BonoboPEditor *editor);

GtkObject *
bonobo_peditor_resolve      (CORBA_TypeCode tc);

GtkObject *
bonobo_peditor_new          (Bonobo_PropertyBag  pb,
			     const char         *name,
			     CORBA_TypeCode      tc, 		
			     CORBA_any          *defval);

GtkObject *
bonobo_peditor_option_new           (gboolean            horiz, 
				     char              **titles);
GtkObject *
bonobo_peditor_short_new            ();

GtkObject *
bonobo_peditor_ushort_new           ();

GtkObject *
bonobo_peditor_long_new             ();

GtkObject *
bonobo_peditor_ulong_new            ();

GtkObject *
bonobo_peditor_float_new            ();

GtkObject *
bonobo_peditor_double_new           ();

GtkObject *
bonobo_peditor_string_new           ();

GtkObject *
bonobo_peditor_boolean_new          (const char *label);

GtkObject *
bonobo_peditor_int_range_new        (gint32 lower, gint32 upper, gint32 incr);

GtkObject *
bonobo_peditor_enum_new             ();

GtkObject *
bonobo_peditor_default_new          ();

GtkObject *
bonobo_peditor_filename_new         ();

GtkObject *
bonobo_peditor_color_new            ();

/* For creating property editors if one already has a widget */

GtkObject *
bonobo_peditor_option_menu_construct (GtkWidget *widget);

GtkObject *
bonobo_peditor_option_radio_construct (GtkWidget **widgets);

GtkObject *
bonobo_peditor_short_construct      (GtkWidget *widget);

GtkObject *
bonobo_peditor_ushort_construct     (GtkWidget *widget);

GtkObject *
bonobo_peditor_long_construct       (GtkWidget *widget);

GtkObject *
bonobo_peditor_ulong_construct      (GtkWidget *widget);

GtkObject *
bonobo_peditor_float_construct      (GtkWidget *widget);

GtkObject *
bonobo_peditor_double_construct     (GtkWidget *widget);

GtkObject *
bonobo_peditor_string_construct     (GtkWidget *widget);

GtkObject *
bonobo_peditor_boolean_construct    (GtkWidget *widget);

GtkObject *
bonobo_peditor_int_range_construct  (GtkWidget *widget);

GtkObject *
bonobo_peditor_enum_construct       (GtkWidget *widget);

GtkObject *
bonobo_peditor_default_construct    (GtkWidget *widget);

GtkObject *
bonobo_peditor_filename_construct   (GtkWidget *widget);

GtkObject *
bonobo_peditor_color_construct      (GtkWidget *widget);

END_GNOME_DECLS

#endif /* _BONOBO_PEDITOR_H_ */
