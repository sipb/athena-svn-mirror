/*
 * bonobo-preferences.c:
 *
 * Authors:
 *   based on eog-preferences.c from Martin Baulig (baulig@suse.de)
 *   modified by Dietmar Maurer (dietmar@ximian.com)
 */
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>

#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-control.h>

#include "bonobo-preferences.h"

struct _BonoboPreferencesPrivate {
	Bonobo_PropertyControl  prop_control;
	BonoboUIContainer      *uic;
};

static GnomePropertyBoxClass *bonobo_preferences_parent_class;

static void
bonobo_preferences_destroy (GtkObject *object)
{
	BonoboPreferences *pref;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_PREFERENCES (object));

	pref = BONOBO_PREFERENCES (object);

	if (pref->priv->prop_control != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (pref->priv->prop_control, NULL);
	
	pref->priv->prop_control = CORBA_OBJECT_NIL;

	if (pref->priv->uic != NULL)
		bonobo_object_unref (BONOBO_OBJECT (pref->priv->uic));

	pref->priv->uic = NULL;

	GTK_OBJECT_CLASS (bonobo_preferences_parent_class)->destroy (object);
}

static void
bonobo_preferences_finalize (GtkObject *object)
{
	BonoboPreferences *preferences;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_PREFERENCES (object));

	preferences = BONOBO_PREFERENCES (object);

	g_free (preferences->priv);

	GTK_OBJECT_CLASS (bonobo_preferences_parent_class)->finalize (object);
}

static void
bonobo_preferences_class_init (BonoboPreferences *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;

	bonobo_preferences_parent_class = 
		gtk_type_class (gnome_property_box_get_type ());

	object_class->destroy = bonobo_preferences_destroy;
	object_class->finalize = bonobo_preferences_finalize;
}

static void
bonobo_preferences_init (BonoboPreferences *preferences)
{
	preferences->priv = g_new0 (BonoboPreferencesPrivate, 1);
}

GtkType
bonobo_preferences_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboPreferences",
			sizeof (BonoboPreferences),
			sizeof (BonoboPreferencesClass),
			(GtkClassInitFunc)  bonobo_preferences_class_init,
			(GtkObjectInitFunc) bonobo_preferences_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_property_box_get_type (), &info);
	}

	return type;
}

static void
add_property_control_page (BonoboPreferences *pref,
			   Bonobo_PropertyControl property_control,
			   Bonobo_UIContainer uic,
			   CORBA_long page_num,
			   CORBA_Environment *ev)
{
	GtkWidget *control_widget;
	Bonobo_PropertyBag property_bag;
	Bonobo_Control control;
	gchar *title = NULL;

	control = Bonobo_PropertyControl_getControl (property_control,
						     page_num, ev);

	if (control == CORBA_OBJECT_NIL)
		return;

	control_widget = bonobo_widget_new_control_from_objref (control, uic);

	gtk_widget_show_all (control_widget);

	property_bag = Bonobo_Unknown_queryInterface
		(control, "IDL:Bonobo/PropertyBag:1.0", ev);

	if (property_bag != CORBA_OBJECT_NIL)
		title = bonobo_property_bag_client_get_value_string
			(property_bag, "bonobo:title", ev);
	else
		title = g_strdup ("Unknown");

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (pref), 
					control_widget, gtk_label_new (title));
}

static void
property_control_changed_cb (BonoboListener    *listener,
			     char              *event_name, 
			     CORBA_any         *any,
			     CORBA_Environment *ev,
			     gpointer           user_data)
{
	GnomePropertyBox *pbox = GNOME_PROPERTY_BOX (user_data);
	
	gnome_property_box_changed (pbox);
}

static void
apply_cb (BonoboPreferences *pref, gint page_num, gpointer data)
{
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);

	if (page_num >= 0)
		Bonobo_PropertyControl_notifyAction (pref->priv->prop_control,
		        page_num, Bonobo_PropertyControl_APPLY, &ev);

	CORBA_exception_free (&ev);
}

GtkWidget *
bonobo_preferences_new (Bonobo_PropertyControl prop_control)
{
	BonoboPreferences *pref;
	CORBA_Environment ev;
	CORBA_long page_count, i;

	g_return_val_if_fail (prop_control != CORBA_OBJECT_NIL, NULL);
	
	if (!(pref = gtk_type_new (bonobo_preferences_get_type ())))
		return NULL;

	CORBA_exception_init (&ev);

	page_count = Bonobo_PropertyControl__get_pageCount (prop_control, &ev);
	if (BONOBO_EX (&ev)) {
		CORBA_exception_free (&ev);
		gtk_object_unref (GTK_OBJECT (pref));
		return NULL;
	}

	pref->priv->prop_control = bonobo_object_dup_ref (prop_control, NULL);
	pref->priv->uic = bonobo_ui_container_new ();
	
	gtk_window_set_title (&GNOME_PROPERTY_BOX (pref)->dialog.window,
			      "Preferences");

	gtk_signal_connect (GTK_OBJECT (pref), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), NULL);

	for (i = 0; i < page_count; i++) {
		add_property_control_page (pref, prop_control, 
					   BONOBO_OBJREF (pref->priv->uic),
					   i, &ev);
		if (BONOBO_EX (&ev))
			break;
	}

	bonobo_event_source_client_add_listener (prop_control, 
		property_control_changed_cb, BONOBO_PROPERTY_CONTROL_CHANGED, 
		NULL, pref);

	CORBA_exception_free (&ev);
	
	return GTK_WIDGET (pref);
}
