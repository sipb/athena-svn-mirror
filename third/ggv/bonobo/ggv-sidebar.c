/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/**
 * ggv-sidebar.c
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2002 Free Software Foundation
 */

#include <config.h>

#include <stdio.h>
#include <math.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>

#include <gnome.h>

#include <ggv-sidebar.h>
#include <gtkchecklist.h>
#include <gsdefaults.h>
#include <ggvutils.h>
#include <ggv-prefs.h>

struct _GgvSidebarPrivate {
	GgvPostScriptView *ps_view;

	GtkWidget *root;

	GtkWidget *checklist;
	GtkWidget *toggle_all, *toggle_even, *toggle_odd, *clear_all;
	GtkWidget *coordinates;

	GtkTooltips *toggle_tips;
};

struct _GgvSidebarClassPrivate {
	int dummy;
};

static BonoboControlClass *ggv_sidebar_parent_class;

static void
ggv_sidebar_destroy (BonoboObject *object)
{
	GgvSidebar *sidebar;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GGV_IS_SIDEBAR (object));

	sidebar = GGV_SIDEBAR (object);

	if(BONOBO_OBJECT_CLASS (ggv_sidebar_parent_class)->destroy)
		BONOBO_OBJECT_CLASS (ggv_sidebar_parent_class)->destroy (object);
}

static void
ggv_sidebar_finalize (GObject *object)
{
	GgvSidebar *sidebar;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GGV_IS_SIDEBAR (object));

	sidebar = GGV_SIDEBAR (object);

	if(sidebar->priv->toggle_tips)
		g_object_unref (G_OBJECT(sidebar->priv->toggle_tips));

	g_free (sidebar->priv);

	G_OBJECT_CLASS (ggv_sidebar_parent_class)->finalize (object);
}

static void
ggv_sidebar_class_init (GgvSidebarClass *klass)
{
	BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)klass;
	GObjectClass *object_class = (GObjectClass *)klass;

	ggv_sidebar_parent_class = gtk_type_class (bonobo_control_get_type ());

	bonobo_object_class->destroy = ggv_sidebar_destroy;
	object_class->finalize = ggv_sidebar_finalize;

	klass->priv = g_new0(GgvSidebarClassPrivate, 1);
}

static void
ggv_sidebar_init (GgvSidebar *sidebar)
{
	sidebar->priv = g_new0 (GgvSidebarPrivate, 1);
}


static gboolean
cl_toggle_all(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
        gboolean val;

        gtk_tree_model_get(model, iter, 0, &val, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), iter, 0, !val, -1);

        return FALSE;
}

static gboolean
cl_toggle_even(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
        gint *n = (gint *)data;
        if(*n%2 == 0) {
                gboolean val;
                gtk_tree_model_get(model, iter, 0, &val, -1);
                gtk_list_store_set(GTK_LIST_STORE(model), iter, 0, !val, -1);
        }
        (*n)++;
        return FALSE;
}

static gboolean
cl_toggle_odd(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gint *n = (gint *)data;
	if(*n%2 != 0) {
		gboolean val;
		gtk_tree_model_get(model, iter, 0, &val, -1);
		gtk_list_store_set(GTK_LIST_STORE(model), iter, 0, !val, -1);
	}
	(*n)++;
	return FALSE;
}

static gboolean
cl_clear_all(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gtk_list_store_set(GTK_LIST_STORE(model), iter, 0, FALSE, -1);
	return FALSE;
}

static void
toggle_all_clicked(GtkWidget *widget, gpointer data)
{
	GgvSidebar *sidebar = GGV_SIDEBAR(data);
	gtk_tree_model_foreach(GTK_TREE_MODEL(GTK_CHECK_LIST(sidebar->priv->checklist)->list_model),
						   cl_toggle_all, NULL);
}

static void
toggle_odd_clicked(GtkWidget *widget, gpointer data)
{
	gint n = 1;
	GgvSidebar *sidebar = GGV_SIDEBAR(data);
	gtk_tree_model_foreach(GTK_TREE_MODEL(GTK_CHECK_LIST(sidebar->priv->checklist)->list_model),
						   cl_toggle_odd, &n);
}

static void
toggle_even_clicked(GtkWidget *widget, gpointer data)
{
	GgvSidebar *sidebar = GGV_SIDEBAR(data);
	gint n = 1;
	gtk_tree_model_foreach(GTK_TREE_MODEL(GTK_CHECK_LIST(sidebar->priv->checklist)->list_model),
						   cl_toggle_even, &n);
}

static void
clear_all_clicked(GtkWidget *widget, gpointer data)
{
	GgvSidebar *sidebar = GGV_SIDEBAR(data);
	gtk_tree_model_foreach(GTK_TREE_MODEL(GTK_CHECK_LIST(sidebar->priv->checklist)->list_model),
						   cl_clear_all, NULL);
}

static void
page_list_selection_changed(GtkTreeSelection *sel,
                            gpointer user_data)
{
	GgvSidebar *sidebar = GGV_SIDEBAR(user_data);
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	if(gtk_tree_selection_get_selected(sel, &model, &iter)) {
		GtkTreePath *path;
		gint page;
		gchar *path_str;
		
		path = gtk_tree_model_get_path(model,
									   &iter);
		path_str = gtk_tree_path_to_string(path);
		page = atoi(path_str);
		g_free(path_str);
		if(page >= 0)
			ggv_postscript_view_goto_page(sidebar->priv->ps_view, page);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(sidebar->priv->checklist),
									 path, NULL, FALSE, 0.0, 0.0);
		gtk_tree_path_free(path);
	}
}

void
ggv_sidebar_create_page_list(GgvSidebar *sidebar)
{
	gint page_count, i;
	gchar **page_names;
#if 0
	gboolean sel_path;
	GtkTreePath *path;
	GtkTreeSelection *sel;
	GtkTreeIter iter;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(sidebar->priv->checklist));
	sel_path = gtk_tree_selection_get_selected(sel, NULL, &iter);
	if(sel_path)
		path = gtk_tree_model_get_path(GTK_TREE_MODEL(GTK_CHECK_LIST(sidebar->priv->checklist)->list_model),
									   &iter);
	else
		path = NULL;
#endif

	gtk_check_list_clear(GTK_CHECK_LIST(sidebar->priv->checklist));

	page_count = ggv_postscript_view_get_page_count(sidebar->priv->ps_view);
	page_names = ggv_postscript_view_get_page_names(sidebar->priv->ps_view);
	if(page_count <= 0)
		return;
	if(page_names == NULL)
		return;
	for(i = 0; page_names[i] != NULL; i++) {
		gtk_check_list_append_row(GTK_CHECK_LIST(sidebar->priv->checklist),
								  page_names[i]);
	}
	g_strfreev(page_names);
#if 0
	if(path) {
		sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(sidebar->priv->checklist));
		gtk_tree_selection_select_path(sel, path);
		gtk_tree_path_free(path);
	}
#endif
}

void
ggv_sidebar_page_changed(GgvSidebar *sidebar, gint page)
{
	GtkTreeSelection *sel;
	gchar path_str[16];
	GtkTreePath *path;

	if(page < 0)
		page = 0;
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(sidebar->priv->checklist));
	g_snprintf(path_str, 15, "%d", page);
	path = gtk_tree_path_new_from_string(path_str);
	if(!gtk_tree_selection_path_is_selected(sel, path)) {
		gtk_tree_selection_select_path(sel, path);
	}
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(sidebar->priv->checklist),
								 path, NULL, FALSE, 0.0, 0.0);
	gtk_tree_path_free(path);
}

gint *
ggv_sidebar_get_active_list(GgvSidebar *sidebar)
{
	return gtk_check_list_get_active_list(GTK_CHECK_LIST(sidebar->priv->checklist));
}

BONOBO_TYPE_FUNC (GgvSidebar, BONOBO_TYPE_CONTROL, ggv_sidebar);

GgvSidebar *
ggv_sidebar_construct (GgvSidebar *sidebar, GgvPostScriptView *ps_view)
{
	GtkWidget *hbox, *image, *sw;
	GtkTreeSelection *sel;

	g_return_val_if_fail (ps_view != NULL, NULL);
	g_return_val_if_fail (sidebar != NULL, NULL);
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (ps_view), NULL);
	g_return_val_if_fail (GGV_IS_SIDEBAR (sidebar), NULL);

	sidebar->priv->ps_view = ps_view;

	/* with a sidebar on the left */
	sidebar->priv->root = gtk_vbox_new(FALSE, 0);

	sidebar->priv->toggle_tips = gtk_tooltips_new();
	g_object_ref(G_OBJECT(sidebar->priv->toggle_tips));
	gtk_object_sink(GTK_OBJECT(sidebar->priv->toggle_tips));

	sidebar->priv->coordinates = gtk_entry_new();
	gtk_widget_set_size_request(sidebar->priv->coordinates, 0, -1);
	gtk_editable_set_editable(GTK_EDITABLE(sidebar->priv->coordinates), FALSE);
	gtk_widget_show(sidebar->priv->coordinates);
	gtk_box_pack_start(GTK_BOX(sidebar->priv->root),
					   sidebar->priv->coordinates,
					   FALSE, FALSE, 2);

	hbox = gtk_hbox_new(TRUE, 2);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(sidebar->priv->root), hbox, FALSE, TRUE, 0);

	image = gtk_image_new_from_file(GNOMEICONDIR "/ggv/toggleall.xpm");
	gtk_widget_show(image);
	sidebar->priv->toggle_all = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(sidebar->priv->toggle_all), GTK_RELIEF_NONE);
	gtk_tooltips_set_tip(sidebar->priv->toggle_tips, sidebar->priv->toggle_all,
						 _("Toggle marked state of all pages"),
						 _("Toggle marked state of all pages: previously "
						   "marked pages will be unmarked and unmarked ones "
						   "will become marked."));
	gtk_widget_show(sidebar->priv->toggle_all);
	gtk_box_pack_start(GTK_BOX(hbox), sidebar->priv->toggle_all, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sidebar->priv->toggle_all), image);
	g_signal_connect(G_OBJECT(sidebar->priv->toggle_all), "clicked",
					 G_CALLBACK(toggle_all_clicked), sidebar);
	image = gtk_image_new_from_file(GNOMEICONDIR "/ggv/toggleodd.xpm");
	gtk_widget_show(image);
	sidebar->priv->toggle_odd = gtk_button_new();
	gtk_tooltips_set_tip(sidebar->priv->toggle_tips, sidebar->priv->toggle_odd,
						 _("Toggle marked state of odd pages"),
						 _("Toggle marked state of odd pages: previously "
						   "marked odd pages will be unmarked and unmarked "
						   "ones will become marked."));
	gtk_button_set_relief(GTK_BUTTON(sidebar->priv->toggle_odd), GTK_RELIEF_NONE);
	gtk_widget_show(sidebar->priv->toggle_odd);
	gtk_box_pack_start(GTK_BOX(hbox), sidebar->priv->toggle_odd, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sidebar->priv->toggle_odd), image);
	g_signal_connect(G_OBJECT(sidebar->priv->toggle_odd), "clicked",
					 G_CALLBACK(toggle_odd_clicked), sidebar);
	image = gtk_image_new_from_file(GNOMEICONDIR "/ggv/toggleeven.xpm");
	gtk_widget_show(image);
	sidebar->priv->toggle_even = gtk_button_new();
	gtk_tooltips_set_tip(sidebar->priv->toggle_tips, sidebar->priv->toggle_even,
						 _("Toggle marked state of even pages"),
						 _("Toggle marked state of even pages: previously "
						   "marked even pages will be unmarked and unmarked "
						   "ones will become marked."));
	gtk_button_set_relief(GTK_BUTTON(sidebar->priv->toggle_even), GTK_RELIEF_NONE);
	gtk_widget_show(sidebar->priv->toggle_even);
	gtk_box_pack_start(GTK_BOX(hbox), sidebar->priv->toggle_even, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sidebar->priv->toggle_even), image);
	g_signal_connect(G_OBJECT(sidebar->priv->toggle_even), "clicked",
					 G_CALLBACK(toggle_even_clicked), sidebar);
	image = gtk_image_new_from_file(GNOMEICONDIR "/ggv/clearall.xpm");
	gtk_widget_show(image);
	sidebar->priv->clear_all = gtk_button_new();
	gtk_tooltips_set_tip(sidebar->priv->toggle_tips, sidebar->priv->clear_all,
						 _("Clear marked state of all pages"),
						 _("Clear marked state of all pages: all pages will "
						   "be unmarked."));
	gtk_button_set_relief(GTK_BUTTON(sidebar->priv->clear_all), GTK_RELIEF_NONE);
	gtk_widget_show(sidebar->priv->clear_all);
	gtk_box_pack_start(GTK_BOX(hbox), sidebar->priv->clear_all, FALSE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(sidebar->priv->clear_all), image);
	g_signal_connect(G_OBJECT(sidebar->priv->clear_all), "clicked",
					 G_CALLBACK(clear_all_clicked), sidebar);
	
	/* a checklist */
	sidebar->priv->checklist = gtk_check_list_new();
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(sidebar->priv->checklist));
	g_signal_connect(G_OBJECT(sel), "changed",
					 G_CALLBACK(page_list_selection_changed), sidebar);
	gtk_widget_show(sidebar->priv->checklist);
	
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
										GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
								   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show(sw);
	gtk_container_add(GTK_CONTAINER(sw), sidebar->priv->checklist);
	gtk_box_pack_start(GTK_BOX(sidebar->priv->root), sw, TRUE, TRUE, 0);
        
	gtk_widget_show(sidebar->priv->root);

	bonobo_control_construct (BONOBO_CONTROL(sidebar), sidebar->priv->root);

	return sidebar;
}

GgvSidebar *
ggv_sidebar_new (GgvPostScriptView *ps_view)
{
	GgvSidebar *sidebar;
	
	g_return_val_if_fail (ps_view != NULL, NULL);
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (ps_view), NULL);

	sidebar = g_object_new(GGV_SIDEBAR_TYPE, NULL);

	return ggv_sidebar_construct (sidebar, ps_view);
}

void
ggv_sidebar_update_coordinates(GgvSidebar *sidebar, gfloat xcoord, gfloat ycoord)
{
	if(sidebar->priv->coordinates != NULL) {
		gchar *clabel;
		clabel = g_strdup_printf("%.2f , %.2f",
								 xcoord*ggv_unit_factors[ggv_unit_index],
								 ycoord*ggv_unit_factors[ggv_unit_index]);
		gtk_entry_set_text(GTK_ENTRY(sidebar->priv->coordinates), clabel);
		g_free(clabel);
	}
}

GtkWidget *
ggv_sidebar_get_checklist(GgvSidebar *sidebar)
{
	return sidebar->priv->checklist;
}
