/*
 * bonobo-property-editor-option.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdarg.h>
#include <bonobo.h>

#include "bonobo-property-editor.h"

static void
selection_done_cb (GtkMenuShell *menushell,
		   gpointer      user_data)
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	BonoboArg *arg;
	GtkWidget *a;
	GList *l;
	guint32 v = 0;

	l = menushell->children;

	a = gtk_menu_get_active (GTK_MENU (menushell));

	while (l && (l->data != a)) {
		++v;
		l = l->next;
	}

	if (!l)
		return;

	arg = bonobo_arg_new (TC_ulong);
	
	BONOBO_ARG_SET_GENERAL (arg, v, TC_ulong, CORBA_unsigned_long, NULL);
	
	bonobo_peditor_set_value (editor, arg, NULL);

	bonobo_arg_release (arg);
}

static void
toggled_cb (GtkRadioButton  *rb,
	    gpointer         user_data) 
{
	BonoboPEditor *editor;
	GtkRadioButton **widget_list;
	BonoboArg *arg;
	gboolean active;
	guint32 v = 0;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (BONOBO_IS_PEDITOR (user_data));

	editor = BONOBO_PEDITOR (user_data);
	widget_list = gtk_object_get_data (GTK_OBJECT (editor), "widget-list");

	while (widget_list[v] != NULL && widget_list[v] != rb) v++;
	
	if ((active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb)))) {
		arg = bonobo_arg_new (TC_ulong);

		BONOBO_ARG_SET_GENERAL (arg, v, TC_ulong, CORBA_unsigned_long,
					NULL);
	
		bonobo_peditor_set_value (editor, arg, NULL);

		bonobo_arg_release (arg);
	}
}

static void
menu_set_value_cb (BonoboPEditor     *editor,
		   BonoboArg         *arg,
		   CORBA_Environment *ev)
{
	GtkWidget *om, *menu;
	guint32 v;

	if (!bonobo_arg_type_is_equal (arg->_type, TC_ulong, NULL))
		return;

	if (!(om = bonobo_peditor_get_widget (editor)))
		return;
	
	v = BONOBO_ARG_GET_GENERAL (arg, TC_ulong, CORBA_unsigned_long, NULL);

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (om));

	gtk_option_menu_set_history (GTK_OPTION_MENU (om), v);
}

static void
toggle_set_value_cb (BonoboPEditor     *editor,
		     BonoboArg         *arg,
		     CORBA_Environment *ev)
{
	GtkWidget **rbs;
	guint32 len, v;

	if (!bonobo_arg_type_is_equal (arg->_type, TC_ulong, NULL))
		return;

	if (!(rbs = gtk_object_get_data (GTK_OBJECT (editor), "widget-list")))
		return;

	v = BONOBO_ARG_GET_GENERAL (arg, TC_ulong, CORBA_unsigned_long, NULL);

	for (len = 0; rbs[len] != NULL; len++);

	if (v < len)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbs[v]), TRUE);
}

static void
destroy_cb (GtkObject *ed) 
{
	GtkWidget **widgets;

	widgets = gtk_object_get_data (ed, "widget-list");
	g_free (widgets);
}

GtkObject *
bonobo_peditor_option_new (gint   mode, 
			   char **titles)
{
	GtkWidget *widget, **rbs, *menu, *item;
	GtkObject *object;
	char *s;

	int len, i;

	g_return_val_if_fail (titles != NULL, NULL);
	g_return_val_if_fail (titles [0] != NULL, NULL);

	if (mode == 1 || mode == 2) {

		if (mode == 1)
			widget = gtk_hbox_new (FALSE, 0);
		else
			widget = gtk_vbox_new (FALSE, 0);

		for (len = 0; titles[len] != NULL; len++);
		rbs = g_new0 (GtkWidget *, len + 1);
		i = 0;

		while ((s = *(titles++))) {
			rbs[i] = gtk_radio_button_new_with_label_from_widget 
				(rbs[0] ? GTK_RADIO_BUTTON (rbs[0]) : NULL, s);
			gtk_box_pack_start_defaults (GTK_BOX (widget), rbs[i]);
			gtk_widget_show (rbs[i]);
			i++;
		}

		rbs[i] = NULL;

		object = bonobo_peditor_option_radio_construct (rbs);
		g_free (rbs);
		return object;
	} else {

		menu = gtk_menu_new ();

		while ((s = *(titles++))) {
			item = gtk_menu_item_new_with_label (s);
			gtk_widget_show (item);
			gtk_menu_append (GTK_MENU (menu), item);
		}

		widget = gtk_option_menu_new ();
		gtk_option_menu_set_menu (GTK_OPTION_MENU (widget), menu);
		gtk_widget_show (widget);

		return bonobo_peditor_option_menu_construct (widget);
	}
}

GtkObject *
bonobo_peditor_option_menu_construct (GtkWidget *widget)
{
	GtkWidget *menu;
	BonoboPEditor *ed;

	g_return_val_if_fail (widget != NULL, NULL);		
	g_return_val_if_fail (GTK_IS_OPTION_MENU (widget), NULL);

	ed = bonobo_peditor_construct (widget, menu_set_value_cb, TC_ulong);

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (widget));

	gtk_signal_connect (GTK_OBJECT (menu), "selection-done",
			    (GtkSignalFunc) selection_done_cb, ed);

	return GTK_OBJECT (ed);
}

GtkObject *
bonobo_peditor_option_radio_construct (GtkWidget **widgets)
{
	BonoboPEditor *ed;
	int len, i;
	GtkWidget **real_widgets;

	ed = bonobo_peditor_construct (widgets[0], toggle_set_value_cb, TC_ulong);

	for (len = 0; widgets[len] != NULL; len++);
	real_widgets = g_new0 (GtkWidget *, len + 1);
	i = 0;

	for (; *widgets != NULL; widgets++) {
		real_widgets[i] = *widgets;
		if (GTK_IS_RADIO_BUTTON (*widgets))
			gtk_signal_connect (GTK_OBJECT (*widgets), 
					    "toggled", (GtkSignalFunc) toggled_cb, ed);
		i++;
	}

	real_widgets[i] = NULL;
	gtk_object_set_data (GTK_OBJECT (ed), "widget-list", real_widgets);
	gtk_signal_connect (GTK_OBJECT (ed), "destroy", GTK_SIGNAL_FUNC (destroy_cb), NULL);

	return GTK_OBJECT (ed);
}
