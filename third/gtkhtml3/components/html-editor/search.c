/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.
    Authors:           Radek Doulik (rodo@helixcode.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h> 
#include <libgnome/gnome-i18n.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include "search.h"
#include "dialog.h"
#include "htmlengine-search.h"

struct _GtkHTMLSearchDialog {
	GtkDialog   *dialog;
	GtkHTML     *html;
	GtkWidget   *entry;
	GtkWidget   *backward;
	GtkWidget   *case_sensitive;

	GtkHTMLControlData *cd;
};

static void
entry_changed (GtkWidget *entry, GtkHTMLSearchDialog *d)
{
	if (d->cd->search_text)
		g_free (d->cd->search_text);
	d->cd->search_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (d->entry)));
}

static void
entry_activate (GtkWidget *entry, GtkHTMLSearchDialog *d)
{
	gtk_dialog_response (d->dialog, 0);
}

static void
search_dialog_response (GtkDialog *dialog, gint response_id, GtkHTMLSearchDialog *d)
{
	switch (response_id) {
	case 0: /* Search */
		gtk_widget_hide (GTK_WIDGET (d->dialog));
		html_engine_search (d->html->engine, gtk_entry_get_text (GTK_ENTRY (d->entry)),
				    GTK_TOGGLE_BUTTON (d->case_sensitive)->active,
				    GTK_TOGGLE_BUTTON (d->backward)->active == 0, d->cd->regular);
		break;
	}
}

GtkHTMLSearchDialog *
gtk_html_search_dialog_new (GtkHTML *html, GtkHTMLControlData *cd)
{
	GtkHTMLSearchDialog *dialog = g_new (GtkHTMLSearchDialog, 1);
	GtkWidget *hbox, *vbox;

	dialog->dialog         = GTK_DIALOG (gtk_dialog_new_with_buttons (_("Find"), NULL, 0,
									  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
									  GTK_STOCK_FIND, 0,
									  NULL));
	dialog->entry          = gtk_entry_new ();
	dialog->backward       = gtk_check_button_new_with_mnemonic (_("_Backward"));
	dialog->case_sensitive = gtk_check_button_new_with_mnemonic (_("Case _sensitive"));
	dialog->html           = html;
	dialog->cd             = cd;

	hbox = gtk_hbox_new (FALSE, 6);

	if (cd->search_text)
		gtk_entry_set_text (GTK_ENTRY (dialog->entry), cd->search_text);

	gtk_box_pack_start (GTK_BOX (hbox), dialog->backward, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), dialog->case_sensitive, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (vbox), dialog->entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog), 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->dialog->vbox), 6);
	gtk_box_set_spacing (GTK_BOX (dialog->dialog->vbox), 6);
	gtk_box_pack_start (GTK_BOX (dialog->dialog->vbox), vbox, FALSE, FALSE, 0);
	gtk_widget_show (dialog->entry);
	gtk_widget_show_all (hbox);

	gnome_window_icon_set_from_file (GTK_WINDOW (dialog->dialog), ICONDIR "/search-24.png");

	gtk_widget_grab_focus (dialog->entry);

	g_signal_connect (dialog->dialog, "response", G_CALLBACK (search_dialog_response), dialog);
	g_signal_connect (dialog->entry, "changed", G_CALLBACK (entry_changed), dialog);
	g_signal_connect (dialog->entry, "activate", G_CALLBACK (entry_activate), dialog);

	return dialog;
}

void
gtk_html_search_dialog_destroy (GtkHTMLSearchDialog *d)
{
	gtk_widget_destroy (GTK_WIDGET (d->dialog));
	g_free (d);
}

void
search (GtkHTMLControlData *cd, gboolean regular)
{
	cd->regular = regular;
	RUN_DIALOG (search, regular ? _("Find Regular Expression") :  _("Find"));
	gtk_html_search_dialog_destroy (cd->search_dialog);
	cd->search_dialog = NULL;
}

void
search_next (GtkHTMLControlData *cd)
{
	if (cd->html->engine->search_info) {
		html_engine_search_next (cd->html->engine);
	} else {
		search (cd, TRUE);
	}
}
