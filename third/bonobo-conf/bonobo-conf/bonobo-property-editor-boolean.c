/*
 * bonobo-property-editor-boolean.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <bonobo.h>

#include "bonobo-property-editor.h"

static void
toggled_cb (GtkToggleButton *toggle,
	    gpointer         user_data) 
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	CORBA_Environment ev;
	BonoboArg *arg;
	gboolean active;

	CORBA_exception_init (&ev);
	
	active = gtk_toggle_button_get_active (toggle);	
	arg = bonobo_arg_new (TC_boolean);

	BONOBO_ARG_SET_BOOLEAN (arg, active);

	bonobo_peditor_set_value (editor, arg, &ev);

	bonobo_arg_release (arg);

	CORBA_exception_free (&ev);
}

static void
set_value_cb (BonoboPEditor     *editor,
	      BonoboArg         *value,
	      CORBA_Environment *ev)
{
	GtkWidget *widget;
	CORBA_boolean new_value;

	if (!bonobo_arg_type_is_equal (value->_type, TC_boolean, NULL))
		return;

	widget = bonobo_peditor_get_widget (editor);

	new_value = BONOBO_ARG_GET_BOOLEAN (value);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), new_value);
}

GtkObject *
bonobo_peditor_boolean_new (const char *label)
{
	GtkWidget *check;

	if (label)
		check = gtk_check_button_new_with_label (label);
	else
		check = gtk_check_button_new ();

	return bonobo_peditor_boolean_construct (check);
}

GtkObject *
bonobo_peditor_boolean_construct (GtkWidget *widget) 
{
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_CHECK_BUTTON (widget), NULL);

	ed = bonobo_peditor_construct (widget, set_value_cb, TC_boolean);

	gtk_signal_connect (GTK_OBJECT (widget), "toggled",
			    (GtkSignalFunc) toggled_cb, ed);
	
	return GTK_OBJECT (ed);
}
