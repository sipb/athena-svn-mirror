/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-selector.c: Bonobo Component Selector
 *
 * Authors:
 *   Richard Hestilow (hestgray@ionet.net)
 *   Miguel de Icaza  (miguel@kernel.org)
 *   Martin Baulig    (martin@home-of-linux.org)
 *   Anders Carlsson  (andersca@gnu.org)
 *   Havoc Pennington (hp@redhat.com)
 *   Dietmar Maurer   (dietmar@ximian.com)
 *   Michael Meeks    (michael@ximian.com)
 *
 * Copyright 1999, 2000 Richard Hestilow, Ximian, Inc,
 *                      Martin Baulig, Anders Carlsson,
 *                      Havoc Pennigton, Dietmar Maurer
 */
#include <config.h>
#include <string.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-macros.h>
#include <bonobo/bonobo-selector.h>

GNOME_CLASS_BOILERPLATE (BonoboSelector, bonobo_selector,
			 GtkDialog, GTK_TYPE_DIALOG);

#define DEFAULT_INTERFACE "IDL:Bonobo/Control:1.0"
#define BONOBO_PAD_SMALL 4

struct _BonoboSelectorPrivate {
	BonoboSelectorWidget *selector;
};

enum {
	OK,
	CANCEL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_INTERFACES
};

static guint bonobo_selector_signals [LAST_SIGNAL] = { 0, 0 };

static void       
bonobo_selector_finalize (GObject *object)
{
	g_return_if_fail (BONOBO_IS_SELECTOR (object));

	g_free (BONOBO_SELECTOR (object)->priv);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/**
 * bonobo_selector_get_selected_id:
 * @sel: A BonoboSelector widget.
 *
 * Returns: A newly-allocated string containing the ID of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this. It will give an oaf iid back.
 */
gchar *
bonobo_selector_get_selected_id (BonoboSelector *sel)
{
	g_return_val_if_fail (BONOBO_IS_SELECTOR (sel), NULL);

	return bonobo_selector_widget_get_id (sel->priv->selector);
}

/**
 * bonobo_selector_get_selected_name:
 * @sel: A BonoboSelector widget.
 *
 * Returns: A newly-allocated string containing the name of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this.
 */
gchar *
bonobo_selector_get_selected_name (BonoboSelector *sel)
{
	g_return_val_if_fail (BONOBO_IS_SELECTOR (sel), NULL);

	return bonobo_selector_widget_get_name (sel->priv->selector);
}

/**
 * bonobo_selector_get_selected_description:
 * @sel: A BonoboSelector widget.
 *
 * Returns: A newly-allocated string containing the description of the
 * currently-selected CORBA server (i.e., the corba server whose name
 * is highlighted in the list).  The user of this function is
 * responsible for freeing this.
 */
gchar *
bonobo_selector_get_selected_description (BonoboSelector *sel)
{
	g_return_val_if_fail (BONOBO_IS_SELECTOR (sel), NULL);

	return bonobo_selector_widget_get_description (sel->priv->selector);
}

static void
ok_callback (GtkWidget *widget, gpointer data)
{
	char *text = bonobo_selector_get_selected_id (
		BONOBO_SELECTOR (widget));

	g_object_set_data (G_OBJECT (widget), "UserData", text);
}

/**
 * bonobo_selector_select_id:
 * @title: The title to be used for the dialog.
 * @interfaces_required: A list of required interfaces.  See
 * bonobo_selector_new().
 *
 * Calls bonobo_selector_new() to create a new
 * BonoboSelector widget with the specified paramters, @title and
 * @interfaces_required.  Then runs the dialog modally and allows
 * the user to make a selection.
 *
 * Returns: The Oaf IID of the selected server, or NULL if no server is
 * selected.  The ID string has been allocated with g_strdup.
 */
gchar *
bonobo_selector_select_id (const gchar  *title,
			   const gchar **interfaces_required)
{
	GtkWidget *sel = bonobo_selector_new (title, interfaces_required);
	gchar     *name = NULL;
	int        n;

	g_return_val_if_fail (sel != NULL, NULL);

	g_signal_connect (sel, "ok",
			  G_CALLBACK (ok_callback), NULL);

	g_object_set_data (G_OBJECT (sel), "UserData", NULL);
	
	gtk_widget_show (sel);
		
	n = gtk_dialog_run (GTK_DIALOG (sel));

	switch (n) {
	case GTK_RESPONSE_CANCEL:
		name = NULL;
		break;
	case GTK_RESPONSE_APPLY:
	case GTK_RESPONSE_OK:
		name = g_object_get_data (G_OBJECT (sel), "UserData");
		break;
	default:
		break;
	}
		
	gtk_widget_destroy (sel);

	return name;
}

static void
response_callback (GtkWidget *widget,
		   gint       response_id,
		   gpointer   data) 
{
	switch (response_id) {
		case GTK_RESPONSE_OK:
			g_signal_emit (data, bonobo_selector_signals [OK], 0);
			break;
		case GTK_RESPONSE_CANCEL:
			g_signal_emit (data, bonobo_selector_signals [CANCEL], 0);
		default:
			break;
	}
}

static void
final_select_cb (GtkWidget *widget, BonoboSelector *sel)
{
	gtk_dialog_response (GTK_DIALOG (sel), GTK_RESPONSE_OK);
}

/**
 * bonobo_selector_construct:
 * @sel: the selector to construct
 * @title: the title for the window
 * @selector: the component view widget to put inside it.
 *
 * Don't use this ever - use construct-time properties instead.
 * TODO: Remove from header when we are allowed to change the API.
 * Constructs the innards of a bonobo selector window.
 *
 * Return value: the constructed widget.
 **/
GtkWidget *
bonobo_selector_construct       (BonoboSelector       *sel,
					    const gchar          *title,
					    BonoboSelectorWidget *selector)
{	
 	g_return_val_if_fail (BONOBO_IS_SELECTOR (sel), NULL);
 	g_return_val_if_fail (BONOBO_IS_SELECTOR_WIDGET (selector), NULL);
 
	sel->priv->selector = selector;

 	g_signal_connect (selector, "final_select",
 			  G_CALLBACK (final_select_cb), sel);
	
	gtk_window_set_title (GTK_WINDOW (sel), title ? title : "");
 
 	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (sel)->vbox),
 			    GTK_WIDGET (selector),
			    TRUE, TRUE, BONOBO_PAD_SMALL);

	gtk_dialog_add_button (GTK_DIALOG (sel), GTK_STOCK_OK,
			       GTK_RESPONSE_OK);
	gtk_dialog_add_button (GTK_DIALOG (sel), GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);
	gtk_dialog_set_default_response (GTK_DIALOG (sel), GTK_RESPONSE_OK);

	g_signal_connect (sel, "response",
			  G_CALLBACK (response_callback), sel);

	gtk_window_set_default_size (GTK_WINDOW (sel), 400, 300);
	gtk_widget_show_all (GTK_DIALOG (sel)->vbox);
	return GTK_WIDGET (sel);
}

static void
bonobo_selector_internal_construct (BonoboSelector       *sel)
{
	BonoboSelectorWidget* selector = sel->priv->selector;

	g_signal_connect (selector, "final_select",
			  G_CALLBACK (final_select_cb), sel);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (sel)->vbox),
			    GTK_WIDGET (selector),
			    TRUE, TRUE, BONOBO_PAD_SMALL);
	
	gtk_dialog_add_button (GTK_DIALOG (sel), GTK_STOCK_OK,
			       GTK_RESPONSE_OK);
	gtk_dialog_add_button (GTK_DIALOG (sel), GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);
	gtk_dialog_set_default_response (GTK_DIALOG (sel), GTK_RESPONSE_OK);
	
	g_signal_connect (sel, "response",
			  G_CALLBACK (response_callback), sel);
	
	gtk_window_set_default_size (GTK_WINDOW (sel), 400, 300);
	gtk_widget_show_all  (GTK_DIALOG (sel)->vbox);
}

static void
bonobo_selector_instance_init (BonoboSelector *sel)
{
	BonoboSelectorWidget *selectorwidget = NULL;
	
	sel->priv = g_new0 (BonoboSelectorPrivate, 1);
	
	selectorwidget = BONOBO_SELECTOR_WIDGET (bonobo_selector_widget_new ());
	sel->priv->selector = selectorwidget;
	
	bonobo_selector_internal_construct (sel);
}

static void
bonobo_selector_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	BonoboSelector *selector = BONOBO_SELECTOR (object);

	switch (prop_id) {
	case PROP_INTERFACES:
	{
		const gchar *query [2] = { DEFAULT_INTERFACE, NULL }; /* the default interfaces_required. */
		BonoboSelectorWidget *selectorwidget = NULL;
		
		/* Get the supplied array or interfaces, replacing it with a default if none have been provided: */
		const gchar **interfaces_required = NULL;
		if (!interfaces_required)
			interfaces_required = query;
		
		/* Set the interfaces_required in the child SelectorWidget: */
		selectorwidget = selector->priv->selector;;
		if (selectorwidget)
			bonobo_selector_widget_set_interfaces (selectorwidget, interfaces_required);
			
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
bonobo_selector_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	/* PROP_INTERFACES is read-only, because there is a virtual BonoboSelectorWidget::set_interfaces(), but no get_interfaces(). */
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
bonobo_selector_class_init (BonoboSelectorClass *klass)
{
	GObjectClass *object_class;
	
	object_class = (GObjectClass *) klass;
	object_class->finalize = bonobo_selector_finalize;
	
	object_class->set_property = bonobo_selector_set_property;
	object_class->get_property = bonobo_selector_get_property;

	bonobo_selector_signals [OK] =
		g_signal_new ("ok",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboSelectorClass, ok),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	
	bonobo_selector_signals [CANCEL] =
		g_signal_new ("cancel",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboSelectorClass, cancel),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
		
	/* properties: */	
   	g_object_class_install_property (object_class,
				 PROP_INTERFACES,
				 g_param_spec_value_array ("interfaces_required",
							   _("Interfaces required"),
							   _("A NULL-terminated array of interfaces which a server must support in order to be listed in the selector. Defaults to \"IDL:Bonobo/Embeddable:1.0\" if no interfaces are listed"),
							   g_param_spec_string ("interface-required-entry",
										_("Interface required entry"),
										_("One of the interfaces that's required"),
										NULL,
										G_PARAM_READWRITE),
							   G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}

/**
 * bonobo_selector_new:
 * @title: A string which should go in the title of the
 * BonoboSelector window.
 * @interfaces_required: A NULL-terminated array of interfaces which a
 * server must support in order to be listed in the selector.  Defaults
 * to "IDL:Bonobo/Embeddable:1.0" if no interfaces are listed.
 *
 * Creates a new BonoboSelector widget.  The title of the dialog
 * is set to @title, and the list of selectable servers is populated
 * with those servers which support the interfaces specified in
 * @interfaces_required.
 *
 * Returns: A pointer to the newly-created BonoboSelector widget.
 */
GtkWidget *
bonobo_selector_new (const gchar *title,
		     const gchar **interfaces_required)
{
	BonoboSelector *sel =  g_object_new (bonobo_selector_get_type (), "title", title, "interfaces_required", interfaces_required, 0);

	return GTK_WIDGET (sel);
}
