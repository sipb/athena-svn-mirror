/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-config-widget.c: Bonobo Component UIConfig widget
 *
 * Authors:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <bonobo/bonobo-i18n.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-engine-private.h>
#include <bonobo/bonobo-ui-config-widget.h>
#include <bonobo/bonobo-ui-sync-toolbar.h>
#include <bonobo/bonobo-ui-toolbar.h>
#include <libgnome/gnome-macros.h>

GNOME_CLASS_BOILERPLATE (BonoboUIConfigWidget,
			 bonobo_ui_config_widget,
			 GtkVBox, GTK_TYPE_VBOX);

struct _BonoboUIConfigWidgetPrivate {
	GtkTreeView  *list_view;
	GtkListStore *list_store;

	GtkWidget    *left_attrs;
	GtkWidget    *right_attrs;

	GtkWidget    *show;
	GtkWidget    *hide;

	GtkWidget    *tooltips;

	GtkWidget    *icon;
	GtkWidget    *text;
	GtkWidget    *icon_and_text;
	GtkWidget    *priority_text;

	char         *cur_path;
};


static void
set_values (BonoboUIConfigWidget *config)
{
	const char *txt;
	BonoboUINode *node;
	gboolean hidden = FALSE;
	gboolean tooltips = TRUE;
	BonoboUIToolbarStyle style;

	g_return_if_fail (config->priv->cur_path != NULL);

	node = bonobo_ui_engine_get_path (config->engine, config->priv->cur_path);

	/* Set hidden flag */
	if ((txt = bonobo_ui_node_peek_attr (node, "hidden")))
		hidden = atoi (txt);

	if (hidden)
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (config->priv->hide),
			TRUE);
	else
		gtk_toggle_button_set_active (
			GTK_TOGGLE_BUTTON (config->priv->show),
			TRUE);

	/* Set the look */
	style = bonobo_ui_sync_toolbar_get_look (config->engine, node);

	switch (style) {
	case BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->icon), TRUE);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->icon_and_text), TRUE);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_TEXT_ONLY:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->text), TRUE);
		break;
		
	case BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (config->priv->priority_text), TRUE);
		break;
		
	default:
		g_warning ("Bogus style %d", style);
		break;
	}
	
	/* Set the tooltips */
	if ((txt = bonobo_ui_node_peek_attr (node, "tips")))
		tooltips = atoi (txt);
		
	gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON (config->priv->tooltips),
		tooltips);
}

static void
list_selection_changed (GtkTreeSelection     *selection,
			BonoboUIConfigWidget *config)
{
	GtkTreeIter iter;
	BonoboUINode *node;
	GtkTreeModel *model;

	if (!gtk_tree_selection_get_selected (
		gtk_tree_view_get_selection (config->priv->list_view), 
		&model, &iter))
		return;

	g_free (config->priv->cur_path);
	gtk_tree_model_get (model, &iter, 1, &config->priv->cur_path, -1);

	node = bonobo_ui_engine_get_path (
		config->engine, config->priv->cur_path);

	gtk_widget_set_sensitive (config->priv->left_attrs, node != NULL);
	gtk_widget_set_sensitive (config->priv->right_attrs, node != NULL);

	if (node)
		set_values (config);
	else
		g_warning ("Toolbar has been removed");
}

static void
populate_list (GtkTreeView          *list_view,
	       BonoboUIConfigWidget *config)
{
	int idx = 0;
	BonoboUINode *l, *start;
	GtkListStore *list_store;

	list_store = GTK_LIST_STORE (gtk_tree_view_get_model (list_view));

	start = bonobo_ui_node_children (
		bonobo_ui_engine_get_xml (config->engine)->root);

	if (!start)
		g_warning ("No tree");

	for (l = start; l; l = bonobo_ui_node_next (l)) {

		if (bonobo_ui_node_has_name (l, "dockitem")) {
			const char *name;

			if ((name = bonobo_ui_node_peek_attr (l, "tip")) ||
			    (name = bonobo_ui_node_peek_attr (l, "name"))) {
				char       *path;
				GtkTreeIter iter;

				path = bonobo_ui_xml_make_path (l);

				gtk_list_store_append (list_store, &iter);
				gtk_list_store_set (list_store, &iter,
						    0, name,
						    1, path, -1);

				if (!idx++) {
					gtk_tree_selection_select_iter (
						gtk_tree_view_get_selection (list_view),
						&iter);
					config->priv->cur_path = path;
				} else
					g_free (path);
			}
		}
	}
}

static void
show_hide_cb (GtkWidget            *button,
	      BonoboUIConfigWidget *config)
{
	g_return_if_fail (config->priv->cur_path != NULL);

	if (button == config->priv->show)
		bonobo_ui_engine_config_remove (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "hidden");
	else
		bonobo_ui_engine_config_add (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "hidden", "1");
}

static void
tooltips_cb (GtkWidget            *button,
	     BonoboUIConfigWidget *config)
{
	g_return_if_fail (config->priv->cur_path != NULL);

	if (gtk_toggle_button_get_active (
		GTK_TOGGLE_BUTTON (button)))
		bonobo_ui_engine_config_remove (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "tips");
	else
		bonobo_ui_engine_config_add (
			bonobo_ui_engine_get_config (config->engine),
			config->priv->cur_path, "tips", "0");
}

static void
look_cb (GtkWidget            *button,
	 BonoboUIConfigWidget *config)
{
	const char *value = NULL;

	g_return_if_fail (config->priv->cur_path != NULL);

	if (button == config->priv->icon)
		value = "icon";

	else if (button == config->priv->icon_and_text)
		value = "both";

	else if (button == config->priv->text)
		value = "text";

	else if (button == config->priv->priority_text)
		value = "both_horiz";

	else
		g_warning ("Unknown look selection");
	
	bonobo_ui_engine_config_add (
		bonobo_ui_engine_get_config (config->engine),
		config->priv->cur_path, "look", value);
}

static void
widgets_init (BonoboUIConfigWidget *config,
	      GtkAccelGroup        *accel_group)
{
	BonoboUIConfigWidgetPrivate *priv;
	GtkWidget *table2;
	GtkWidget *vbox6;
	GtkWidget *frame6;
	GtkWidget *vbox7;
	GSList *visible_group = NULL;
	GtkWidget *frame7;
	GtkWidget *toolbar_list;
	GtkWidget *frame5;
	GtkWidget *vbox5;
	GSList *look_group = NULL;

	priv = config->priv;

	table2 = gtk_table_new (2, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (config), table2, TRUE, TRUE, 0);

	priv->left_attrs = vbox6 = gtk_vbox_new (FALSE, 0);
	gtk_table_attach (GTK_TABLE (table2), vbox6, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);

	frame6 = gtk_frame_new (_("Visible"));
	gtk_box_pack_start (GTK_BOX (vbox6), frame6, FALSE, FALSE, 0);

	vbox7 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame6), vbox7);

	priv->show = gtk_radio_button_new_with_mnemonic (visible_group,
							 _("_Show"));
	g_signal_connect (priv->show, "clicked",
			  G_CALLBACK (show_hide_cb), config);
	visible_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->show));
	gtk_box_pack_start (GTK_BOX (vbox7), priv->show, FALSE, FALSE, 0);

	priv->hide = gtk_radio_button_new_with_mnemonic (visible_group,
							 _("_Hide"));
	g_signal_connect (priv->hide, "clicked",
			  G_CALLBACK (show_hide_cb), config);
	visible_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->hide));
	gtk_box_pack_start (GTK_BOX (vbox7), priv->hide, FALSE, FALSE, 0);

	priv->tooltips = gtk_check_button_new_with_mnemonic (_("_View tooltips"));
	gtk_box_pack_start (GTK_BOX (vbox6), priv->tooltips, FALSE, FALSE, 0);
	g_signal_connect (priv->tooltips, "clicked",
			  G_CALLBACK (tooltips_cb), config);

	frame7 = gtk_frame_new (_("Toolbars"));
	gtk_table_attach (GTK_TABLE (table2), frame7, 0, 2, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

	priv->list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	toolbar_list = gtk_tree_view_new_with_model (
		GTK_TREE_MODEL (priv->list_store));
	priv->list_view = GTK_TREE_VIEW (toolbar_list);
	gtk_tree_view_insert_column_with_attributes (
		priv->list_view, 0, _("toolbars"),
		gtk_cell_renderer_text_new (),
		"text", 0, NULL);
	gtk_tree_view_set_headers_visible (priv->list_view, FALSE);
	gtk_tree_selection_set_mode (
		gtk_tree_view_get_selection (priv->list_view),
		GTK_SELECTION_BROWSE);

	gtk_container_add (GTK_CONTAINER (frame7), toolbar_list);
	GTK_WIDGET_SET_FLAGS (toolbar_list, GTK_CAN_DEFAULT);

	frame5 = gtk_frame_new (_("Look"));
	gtk_table_attach (GTK_TABLE (table2), frame5, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 0, 0);

	priv->right_attrs = vbox5 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame5), vbox5);

	priv->icon = gtk_radio_button_new_with_mnemonic (look_group,
							 _("_Icon"));
	g_signal_connect (priv->icon, "clicked",
			  G_CALLBACK (look_cb), config);
	look_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->icon));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->icon, FALSE, FALSE, 0);

	priv->icon_and_text = gtk_radio_button_new_with_mnemonic (look_group, _("_Text and Icon"));
	g_signal_connect (priv->icon_and_text, "clicked",
			  G_CALLBACK (look_cb), config);
	look_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->icon_and_text));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->icon_and_text, FALSE, FALSE, 0);

	priv->text = gtk_radio_button_new_with_mnemonic (look_group, _("Text only"));
	g_signal_connect (priv->text, "clicked",
			  G_CALLBACK (look_cb), config);
	look_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->text));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->text, FALSE, FALSE, 0);

	priv->priority_text = gtk_radio_button_new_with_mnemonic (look_group, _("_Priority text only"));
	g_signal_connect (priv->priority_text, "clicked",
			  G_CALLBACK (look_cb), config);
	look_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (priv->priority_text));
	gtk_box_pack_start (GTK_BOX (vbox5), priv->priority_text, FALSE, FALSE, 0);

	populate_list (priv->list_view, config);

	g_signal_connect (
		gtk_tree_view_get_selection (priv->list_view),
		"changed",
		G_CALLBACK (list_selection_changed),
		config);

	set_values (config);

	gtk_widget_show_all (GTK_WIDGET (config));
	gtk_widget_hide (GTK_WIDGET (config));
}

static void
bonobo_ui_config_widget_instance_init (BonoboUIConfigWidget *config)
{
	config->priv = g_new0 (BonoboUIConfigWidgetPrivate, 1);
}

static void
bonobo_ui_config_widget_finalize (GObject *object)
{
	BonoboUIConfigWidget *config = BONOBO_UI_CONFIG_WIDGET (object);

	g_free (config->priv);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

GtkWidget *
bonobo_ui_config_widget_construct (BonoboUIConfigWidget *config,
				   BonoboUIEngine       *engine,
				   GtkAccelGroup        *accel_group)
{
	config->engine = engine;
	widgets_init (config, accel_group);

	return GTK_WIDGET (config);
}

/**
 * bonobo_ui_config_widget_new:
 *
 * Creates a new BonoboUIConfigWidget widget, this contains
 * a List of toolbars and allows configuration of the widgets.
 *
 * Returns: A pointer to the newly-created BonoboUIConfigWidget widget.
 */
GtkWidget *
bonobo_ui_config_widget_new (BonoboUIEngine *engine,
			     GtkAccelGroup  *accel_group)
{
	BonoboUIConfigWidget *config = g_object_new (
		bonobo_ui_config_widget_get_type (), NULL);

	return bonobo_ui_config_widget_construct (
		config, engine, accel_group);
}

static void
bonobo_ui_config_widget_class_init (BonoboUIConfigWidgetClass *klass)
{
	GObjectClass *gobject_class;
	
	g_return_if_fail (klass != NULL);
	
	gobject_class = (GObjectClass *) klass;
	
	gobject_class->finalize = bonobo_ui_config_widget_finalize;
}
