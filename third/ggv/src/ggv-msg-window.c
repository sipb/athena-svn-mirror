/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-msg-window.c: display of GhostScript's output
 *
 * Copyright (C) 2001, 2002 the Free Software Foundation
 */

#include <config.h>

#include <gnome.h>
#include <gtk/gtk.h>

#include "ggv-msg-window.h"

/* clears the text in the tbox */
static void
del_gs_status_text (GgvMsgWindow *win)
{
        GtkTextBuffer *buffer;

        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (win->error_text));
        gtk_text_buffer_set_text(buffer, "", -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW (win->error_text), buffer); 
}

static gint
delete_statustext_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
        gtk_widget_hide(((GgvMsgWindow *)data)->window);
	return TRUE;
}

static void
close_clicked (GtkButton *button, gpointer data)
{
        gtk_widget_hide(((GgvMsgWindow *)data)->window);
}

static void
help_clicked (GtkButton *button, gpointer data)
{
        gnome_help_display("probs.html",
                           NULL,
                           NULL /* error */);
}

/* FIXME: this should be configurable */
#define STATUS_WIDTH 520
#define STATUS_HEIGHT 320

GgvMsgWindow *
ggv_msg_window_new(const gchar *title)
{
        GgvMsgWindow *win;
	GtkWidget *vbox, *hbox, *vscrollbar, *buttonbox, *button;
        GtkAccelGroup *accel_group;

        accel_group = gtk_accel_group_new ();

        win = g_new0(GgvMsgWindow, 1);

        win->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 
	gtk_window_set_default_size(GTK_WINDOW(win->window),
                                    STATUS_WIDTH, STATUS_HEIGHT);
  	gtk_window_set_policy(GTK_WINDOW(win->window), TRUE, TRUE, FALSE);

	g_signal_connect(GTK_OBJECT(win->window), "delete_event",
                         G_CALLBACK(delete_statustext_event), win);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(win->window), vbox);

        /* set up gtk_text */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

        win->error_text = gtk_text_view_new();
	gtk_widget_show(win->error_text);
	gtk_box_pack_start(GTK_BOX(hbox), win->error_text, TRUE, TRUE, 0);

	vscrollbar = gtk_vscrollbar_new(GTK_TEXT_VIEW(win->error_text)->vadjustment);
	gtk_box_pack_start(GTK_BOX(hbox), vscrollbar, FALSE, FALSE, 0);
	gtk_widget_show(vscrollbar);

	/* set up buttons */
        buttonbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox),
                                  GTK_BUTTONBOX_SPREAD);
	gtk_widget_show(buttonbox);
	gtk_box_pack_start(GTK_BOX (vbox), buttonbox, FALSE, FALSE, 4);

        button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
        gtk_widget_show(button);
        gtk_container_add(GTK_CONTAINER(buttonbox), button);
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_widget_add_accelerator(button, "clicked", accel_group,
                                   GDK_Return, 0, GTK_ACCEL_VISIBLE);
        gtk_widget_add_accelerator(button, "clicked", accel_group,
                                   GDK_Escape, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(GTK_OBJECT(button), "clicked",
                         G_CALLBACK(close_clicked), win);
        gtk_widget_grab_default (button);

        button = gtk_button_new_from_stock(GTK_STOCK_HELP); 
        gtk_widget_show(button);
	gtk_container_add(GTK_CONTAINER(buttonbox), button);
        g_signal_connect(GTK_OBJECT(button), "clicked",
                         G_CALLBACK(help_clicked), win);

        gtk_window_add_accel_group (GTK_WINDOW(win->window), accel_group);

        ggv_msg_window_init(win, title);

        return win;
}

void
ggv_msg_window_free(GgvMsgWindow *win)
{
        g_return_if_fail(win != NULL);

        gtk_widget_destroy(win->window);
        g_free(win);
}

/* set up status window, clear text field, but do not show window */
void
ggv_msg_window_init(GgvMsgWindow *win, const gchar *title)
{
	g_return_if_fail(win != NULL);

        del_gs_status_text (win);

	if(title)
                gtk_window_set_title (GTK_WINDOW(win->window), title);
}

/* shows the status window */
void
ggv_msg_window_show(GgvMsgWindow *win)
{
	g_return_if_fail(win != NULL);

	if(!GTK_WIDGET_VISIBLE(win->window)) {
                ggv_msg_window_init(win, NULL);
                gtk_widget_show(win->window);
        }
}

/* adds new_text to the text box */
void
ggv_msg_window_add_text(GgvMsgWindow *win, const gchar *text, gint show)
{
        GtkTextBuffer *buffer;

	g_return_if_fail(win != NULL);
	g_return_if_fail(text != NULL);

	if(show)
		ggv_msg_window_show(win);
        
        buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (win->error_text));
        gtk_text_buffer_set_text (buffer, text, -1);
        gtk_text_view_set_buffer(GTK_TEXT_VIEW(win->error_text), buffer); 
}
