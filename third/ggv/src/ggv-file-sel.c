/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-file-sel.c: a file selector for ggv
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#include <config.h>

#include <libgnomevfs/gnome-vfs-utils.h>

#include "ggv-file-sel.h"
#include "ggvutils.h"

static GtkFileSelectionClass *parent_class;

static void
ggv_file_sel_finalize(GObject *object)
{
        GgvFileSel *sel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(GGV_IS_FILE_SEL(object));

        sel = GGV_FILE_SEL(object);

        if(sel->uri)
                g_free(sel->uri);

        if(G_OBJECT_CLASS(parent_class)->finalize)
                G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void
ggv_file_sel_destroy(GtkObject *object)
{
        GgvFileSel *sel;

        g_return_if_fail(object != NULL);
        g_return_if_fail(GGV_IS_FILE_SEL(object));

        sel = GGV_FILE_SEL(object);

        if(GTK_OBJECT_CLASS(parent_class)->destroy)
                GTK_OBJECT_CLASS(parent_class)->destroy(object);
}

static void
ggv_file_sel_class_init(GgvFileSelClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
        GObjectClass *gobject_class;

        gobject_class = (GObjectClass *) klass;
	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

        gobject_class->finalize = ggv_file_sel_finalize;
	object_class->destroy = ggv_file_sel_destroy;
}

static void
ggv_file_sel_init (GgvFileSel *sel)
{
        sel->uri = NULL;
}

GType
ggv_file_sel_get_type (void) 
{
	static GType ggv_file_sel_type = 0;
	
	if(!ggv_file_sel_type) {
		static const GTypeInfo ggv_file_sel_info =
		{
			sizeof(GgvFileSelClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) ggv_file_sel_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof(GgvFileSel),
			0,		/* n_preallocs */
			(GInstanceInitFunc) ggv_file_sel_init,
		};
		
		ggv_file_sel_type = g_type_register_static(GTK_TYPE_FILE_SELECTION, 
                                                         "GgvFileSel", 
                                                         &ggv_file_sel_info, 0);
	}

	return ggv_file_sel_type;
}

GtkWidget *
ggv_file_sel_new(void)
{
        GgvFileSel *sel;

	sel = GGV_FILE_SEL(g_object_new(GGV_TYPE_FILE_SEL, NULL));

	return GTK_WIDGET(sel);
}

void
ggv_file_sel_set_uri(GgvFileSel *sel, const gchar *uri)
{
        gchar *fname;

        if((fname = gnome_vfs_get_local_path_from_uri(uri)) == NULL)
                return;

        gtk_file_selection_set_filename(GTK_FILE_SELECTION(sel),
                                        fname);
        g_free(fname);
}

gchar *
ggv_file_sel_get_uri(GgvFileSel *sel)
{
        const gchar *fname;
        gchar *uri = NULL;

        fname = gtk_file_selection_get_filename(GTK_FILE_SELECTION(sel));
        if(!g_file_test(fname, G_FILE_TEST_EXISTS) ||
           g_file_test(fname,
                       G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK)) {
                uri = ggv_filename_to_uri(fname);
        }
        return uri;
}

static void
ggv_file_sel_ok_clicked(GtkWidget *w, gpointer data)
{
        GgvFileSel *sel = GGV_FILE_SEL(data);

        if(sel->uri)
                g_free(sel->uri);
        sel->uri = ggv_file_sel_get_uri(sel);
        gtk_main_quit();
}

static void
ggv_file_sel_cancel_clicked(GtkWidget *w, gpointer data)
{
        GgvFileSel *sel = GGV_FILE_SEL(data);

        if(sel->uri)
                g_free(sel->uri);
        sel->uri = NULL;
        gtk_main_quit();
}

static gboolean
ggv_file_sel_delete_event(GtkWidget *w, GdkEventAny *e, gpointer data)
{
        GgvFileSel *sel = GGV_FILE_SEL(data);

        if(sel->uri)
                g_free(sel->uri);
        sel->uri = NULL;
        gtk_main_quit();
        return TRUE;
}

gchar *
ggv_file_sel_request_uri(const gchar *title, const gchar *def_uri)
{
        GtkWidget *sel;
        GtkFileSelection *fsel;
        gchar *uri;

        sel = ggv_file_sel_new();
        fsel = GTK_FILE_SELECTION(sel);
        gtk_window_set_title(GTK_WINDOW(sel), title);
        gtk_window_set_modal(GTK_WINDOW(sel),TRUE);
        if(def_uri != NULL)
                ggv_file_sel_set_uri(GGV_FILE_SEL(sel), def_uri);
        g_signal_connect(G_OBJECT(fsel->ok_button), "clicked",
                         G_CALLBACK(ggv_file_sel_ok_clicked), sel);
        g_signal_connect(G_OBJECT(fsel->cancel_button), "clicked",
                         G_CALLBACK(ggv_file_sel_cancel_clicked), sel);
        g_signal_connect(G_OBJECT(sel), "delete-event",
                         G_CALLBACK(ggv_file_sel_delete_event), sel);
        gtk_widget_show(sel);
        gtk_main();
        if(GGV_FILE_SEL(sel)->uri != NULL)
                uri = g_strdup(GGV_FILE_SEL(sel)->uri);
        else
                uri = NULL;
        gtk_widget_destroy(sel);
        return uri;
}
