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
#include <unistd.h>
#include <string.h>
#include <glade/glade.h>

#include "gtkhtml.h"
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit-images.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmlsettings.h"

#include "image.h"
#include "properties.h"
#include "utils.h"

struct _GtkHTMLEditImageProperties {
	GtkHTMLControlData *cd;

	GtkWidget *page;

	HTMLImage *image;
	GtkHTML  *sample;
	gboolean  insert;

	GtkWidget *frame_sample;
	GtkWidget *pentry;
	gchar *location;

	GtkWidget *option_template;
	gint template;

	GtkWidget *spin_width;
	GtkWidget *option_width_percent;
	gint width;
	gint width_percent;


	GtkWidget *spin_height;
	GtkWidget *option_height_percent;
	gint height;
	gint height_percent;

	GtkWidget *spin_padh;
	gint padh;

	GtkWidget *spin_padv;
	gint padv;

	GtkWidget *spin_border;
	gint border;

	GtkWidget *option_align;
	HTMLVAlignType align;

	GtkWidget *entry_url;
	gchar *url;

	GtkWidget *entry_alt;
	gchar *alt;

	gboolean   disable_change;
};
typedef struct _GtkHTMLEditImageProperties GtkHTMLEditImageProperties;

#define CHANGE if (!d->disable_change) gtk_html_edit_properties_dialog_change (d->cd->properties_dialog)
#define FILL 	if (!d->disable_change) fill_sample (d)

#define TEMPLATES 3
typedef struct {
	gchar *name;
	gint offset;

	gboolean can_set_align;
	gboolean can_set_border;
	gboolean can_set_padding;
	gboolean can_set_size;

	HTMLVAlignType align;
	gint border;
	gint padh;
	gint padv;
	gint width;
	gboolean width_percent;
	gint height;
	gboolean height_percent;

	gchar *image;
} ImageInsertTemplate;

static ImageInsertTemplate image_templates [TEMPLATES] = {
	{
		N_("Plain"), 1,
		TRUE, TRUE, TRUE, TRUE, HTML_VALIGN_TOP, 0, 0, 0, 0, FALSE, 0, FALSE,
		N_("@link_begin@<img@alt@@width@@height@@align@ border=@border@@padh@@padv@@src@>@link_end@")
	},
	{
		N_("Frame"), 1,
		FALSE, TRUE, FALSE, TRUE, HTML_VALIGN_TOP, 1, 0, 0, 0, FALSE, 0, FALSE,
		N_("<center><table bgcolor=\"#c0c0c0\" cellspacing=\"0\" cellpadding=@border@>"
		   "<tr><td>"
		   "<table bgcolor=\"#f2f2f2\" cellspacing=\"0\" cellpadding=\"8\" width=\"100%\">"
		   "<tr><td align=\"center\">"
		   "<img @src@@alt@@width@@height@align=top border=0>"
		   "</td></tr></table></td></tr></table></center>")
	},
	{
		N_("Caption"), 1,
		FALSE, TRUE, FALSE, TRUE, HTML_VALIGN_TOP, 1, 0, 0, 0, FALSE, 0, FALSE,
		N_("<center><table bgcolor=\"#c0c0c0\" cellspacing=\"0\" cellpadding=@border@>"
		   "<tr><td>"
		   "<table bgcolor=\"#f2f2f2\" cellspacing=\"0\" cellpadding=\"8\" width=\"100%\">"
		   "<tr><td align=\"center\">"
		   "<img @src@@alt@@width@@height@align=top border=0>"
		   "</td></tr>"
		   "<tr><td><b>[Place your comment here]</td></tr>"
		   "</table></td></tr></table></center>")
	},
};

static GtkHTMLEditImageProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditImageProperties *data = g_new0 (GtkHTMLEditImageProperties, 1);

	/* fill data */
	data->cd             = cd;
	data->disable_change = TRUE;
	data->image          = NULL;

	/* default values */
	data->align          = HTML_VALIGN_TOP;
	data->width_percent  = 2;
	data->height_percent = 2;

	return data;
}

static gchar *
substitute_string (gchar *str, const gchar *var_name, const gchar *value)
{
	gchar *substr;

	substr = strstr (str, var_name);
	if (substr) {
		gchar *new_str;

		*substr = 0;
		new_str = g_strdup_printf ("%s%s%s", str, value, substr + strlen (var_name));
		g_free (str);
		str = new_str;
	}

	return str;
}

static gchar *
get_location (GtkHTMLEditImageProperties *d)
{
	gchar *file;
	gchar *url;

	file = gnome_pixmap_entry_get_filename (GNOME_PIXMAP_ENTRY (d->pentry));
	if (file) {
		url = g_strconcat ("file://", file, NULL);
	} else {
		GtkWidget *entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry));

		url = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
	}

	if (!url)
		url = g_strdup ("");
	g_free (file);

	return url;
}

static gchar *
get_sample_html (GtkHTMLEditImageProperties *d, gboolean insert)
{
	gchar *html, *image, *body, *width, *height, *align, *src, *alt, *border, *padh, *padv, *lbegin, *lend, *location;

	if ((d->width || d->width_percent == 1) && d->width_percent != 2)
		width  = g_strdup_printf (" width=\"%d%s\"", d->width, d->width_percent ? "%" : "");
	else
		width  = g_strdup ("");

	if ((d->height || d->height_percent == 1) && d->height_percent != 2)
		height = g_strdup_printf (" height=\"%d%s\"", d->height, d->height_percent ? "%" : "");
	else
		height = g_strdup ("");

	align  = g_strdup_printf (" align=%s", d->align == HTML_VALIGN_TOP
				   ? "top" : (d->align == HTML_VALIGN_MIDDLE ? "middle" : "bottom"));
	location = get_location (d);
	src    = g_strdup_printf (" src=\"%s\"", location);
	alt    = g_strdup_printf (" alt=\"%s\"", d->alt ? d->alt : "");
	padh   = g_strdup_printf (" hspace=%d", d->padh);
	padv   = g_strdup_printf (" vspace=%d", d->padv);
	border = g_strdup_printf ("%d", d->border);

	if (d->url && *d->url) {
		gchar *encoded_url;

		encoded_url = html_encode_entities (d->url, g_utf8_strlen (d->url, -1), NULL);
		lbegin = g_strdup_printf ("<a href=\"%s\">", encoded_url);
		lend   = "</a>";
		g_free (encoded_url);
	} else {
		lbegin = g_strdup ("");
		lend   = "";
	}

	image   = g_strdup (image_templates [d->template].image);
	image   = substitute_string (image, "@src@", src);
	image   = substitute_string (image, "@alt@", alt);
	image   = substitute_string (image, "@padh@", padh);
	image   = substitute_string (image, "@padv@", padv);
	image   = substitute_string (image, "@width@", width);
	image   = substitute_string (image, "@height@", height);
	image   = substitute_string (image, "@align@", align);
	image   = substitute_string (image, "@border@", border);
	image   = substitute_string (image, "@link_begin@", lbegin);
	image   = substitute_string (image, "@link_end@", lend);

	body   = html_engine_save_get_sample_body (d->cd->html->engine, NULL);
	if (insert) {
		html   = g_strconcat (body, image, NULL);
	} else {
		html   = g_strconcat (body,
				      _("The quick brown fox jumps over the lazy dog."),
				      " ",
				      image,
				      _("The quick brown fox jumps over the lazy dog."),
				      NULL);
	}

	g_free (location);
	g_free (lbegin);
	g_free (border);
	g_free (src);
	g_free (padv);
	g_free (padh);
	g_free (width);
	g_free (height);
	g_free (align);
	g_free (body);

	/* printf ("IMAGE: %s\n", html); */

	return html;
}

static void
fill_sample (GtkHTMLEditImageProperties *d)
{
	gchar *html;

	html = get_sample_html (d, FALSE);
	gtk_html_load_from_string (d->sample, html, -1);
	g_free (html);
}

static void
pentry_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	const gchar *text;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	if (!text || !d->location || strcmp (text, d->location)) {
		g_free (d->location);
		d->location = g_strdup (text);
		if (!d->width_percent)
			d->width = 0;
		if (!d->height_percent)
			d->height = 0;
		CHANGE;
		FILL;
	}
}

static void
url_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	g_free (d->url);
	d->url = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
	CHANGE;
	FILL;
}

static void
alt_changed (GtkWidget *entry, GtkHTMLEditImageProperties *d)
{
	g_free (d->alt);
	d->alt = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
	CHANGE;
	FILL;
}

static void
changed_align (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	d->align = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));;
	CHANGE;
	FILL;
}

static void
changed_width_percent (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	d->width_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));
	gtk_widget_set_sensitive (d->spin_width, d->width_percent != 2);
	CHANGE;
	FILL;
}

static void
changed_height_percent (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	d->height_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));
	gtk_widget_set_sensitive (d->spin_height, d->height_percent != 2);
	CHANGE;
	FILL;
}

static void
test_url_clicked (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	const char *url = gtk_entry_get_text (GTK_ENTRY (d->entry_url));

	if (url)
		gnome_url_show (url, NULL);
}

static void
fill_templates (GtkHTMLEditImageProperties *d)
{
	GtkWidget *menu;
	gint i;

	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template));

	for (i = 0; i < TEMPLATES; i ++)
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new_with_label (_(image_templates [i].name)));
	gtk_menu_set_active (GTK_MENU (menu), 0);
	gtk_container_remove (GTK_CONTAINER (menu), gtk_menu_get_active (GTK_MENU (menu)));
}

static void
set_ui (GtkHTMLEditImageProperties *d)
{
	d->disable_change = TRUE;

	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_template), d->template);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_align), d->align);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width_percent), d->width_percent);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_height_percent), d->height_percent);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width), d->width);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height), d->height);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_padh), d->padh);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_padv), d->padv);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_border), d->border);
	gtk_entry_set_text (GTK_ENTRY (d->entry_url), d->url ? d->url : "");
	gtk_entry_set_text (GTK_ENTRY (d->entry_alt), d->alt ? d->alt : "");
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry))),
			    d->location ? d->location : "");

	gtk_widget_set_sensitive (d->spin_width, d->width_percent != 2);
	gtk_widget_set_sensitive (d->spin_height, d->height_percent != 2);

	d->disable_change = FALSE;

	FILL;
}

static void
changed_template (GtkWidget *w, GtkHTMLEditImageProperties *d)
{
	d->template = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));

	d->border = image_templates [d->template].border;
	d->align = image_templates [d->template].align;
	d->padh = image_templates [d->template].padh;
	d->padv = image_templates [d->template].padv;
	d->padh = image_templates [d->template].padh;
	d->width = image_templates [d->template].width;
	d->width_percent = image_templates [d->template].width_percent;
	d->height = image_templates [d->template].height;
	d->height_percent = image_templates [d->template].height_percent;

	gtk_widget_set_sensitive (d->spin_width, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->option_width_percent, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->spin_height, image_templates [d->template].can_set_size);
	gtk_widget_set_sensitive (d->option_height_percent, image_templates [d->template].can_set_size);

	gtk_widget_set_sensitive (d->spin_padh, image_templates [d->template].can_set_padding);
	gtk_widget_set_sensitive (d->spin_padv, image_templates [d->template].can_set_padding);

	gtk_widget_set_sensitive (d->spin_border, image_templates [d->template].can_set_border);

	gtk_widget_set_sensitive (d->option_align, image_templates [d->template].can_set_align);

	set_ui (d);

	CHANGE;
	FILL;
}

static void
changed_border (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	d->border = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_border));
	FILL;
	CHANGE;
}

static void
changed_width (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	d->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_width));
	FILL;
	CHANGE;
}

static void
changed_height (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	d->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_height));
	FILL;
	CHANGE;
}

static void
changed_padh (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	d->padh = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_padh));
	FILL;
	CHANGE;
}

static void
changed_padv (GtkWidget *check, GtkHTMLEditImageProperties *d)
{
	d->padv = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_padv));
	FILL;
	CHANGE;
}

static void
set_size_all (HTMLObject *o, HTMLEngine *e, GtkHTMLEditImageProperties *d)
{
	if (d->location && HTML_IS_IMAGE (o) && HTML_IMAGE (o)->image_ptr && HTML_IMAGE (o)->image_ptr->url) {
		gchar *location = get_location (d);

		if (!strcmp (HTML_IMAGE (o)->image_ptr->url, location)) {
			HTMLImage *i = HTML_IMAGE (o);

			d->disable_change = TRUE;
			if ((d->width == 0 || d->width_percent == 2) && d->width_percent != 1) {
				d->width = html_image_get_actual_width (i, NULL);
				gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width), d->width);
			}
			if ((d->height == 0 || d->height_percent == 2) && d->height_percent != 1) {
				d->height = html_image_get_actual_height (i, NULL);
				gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height), d->height);
			}
			d->disable_change = FALSE;
		}
		g_free (location);
	}
}

static gboolean
set_size (GtkHTMLEditImageProperties *d)
{
	if (d->sample->engine->clue) {
		html_object_forall (d->sample->engine->clue, d->sample->engine, (HTMLObjectForallFunc) set_size_all, d);
	}
	return FALSE;
}

static void
image_url_requested (GtkHTML *html, const gchar *url, GtkHTMLStream *handle, GtkHTMLEditImageProperties *d)
{
	gchar *location;

	location = get_location (d);
	url_requested (html, url, handle);
	if (location && !strcmp (location, url))
		gtk_idle_add ((GtkFunction) set_size, d);
	g_free (location);
}

#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static GtkWidget *
image_widget (GtkHTMLEditImageProperties *d, gboolean insert)
{
	GladeXML *xml;
	GtkWidget *frame_template;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "image_page", NULL);
	if (!xml)
		g_error (_("Could not load glade file."));

	d->page = glade_xml_get_widget (xml, "image_page");
	d->frame_sample = glade_xml_get_widget (xml, "frame_image_sample");
	frame_template = glade_xml_get_widget (xml, "frame_image_template");

	d->option_align = glade_xml_get_widget (xml, "option_image_align");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_align)),
			  "selection-done", G_CALLBACK (changed_align), d);
	d->option_width_percent = glade_xml_get_widget (xml, "option_image_width_percent");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width_percent)),
			  "selection-done", G_CALLBACK (changed_width_percent), d);
	d->option_height_percent = glade_xml_get_widget (xml, "option_image_height_percent");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_height_percent)),
			  "selection-done", G_CALLBACK (changed_height_percent), d);

	d->spin_border = glade_xml_get_widget (xml, "spin_image_border");
	UPPER_FIX (border);
	g_signal_connect (d->spin_border, "value_changed", G_CALLBACK (changed_border), d);
	d->spin_width = glade_xml_get_widget (xml, "spin_image_width");
	UPPER_FIX (width);
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	d->spin_height = glade_xml_get_widget (xml, "spin_image_height");
	UPPER_FIX (height);
	g_signal_connect (d->spin_height, "value_changed", G_CALLBACK (changed_height), d);
	d->spin_padh = glade_xml_get_widget (xml, "spin_image_padh");
	UPPER_FIX (padh);
	g_signal_connect (d->spin_padh, "value_changed", G_CALLBACK (changed_padh), d);
	d->spin_padv = glade_xml_get_widget (xml, "spin_image_padv");
	UPPER_FIX (padv);
	g_signal_connect (d->spin_padv, "value_changed", G_CALLBACK (changed_padv), d);

	d->option_template = glade_xml_get_widget (xml, "option_image_template");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_template)),
			  "selection-done", G_CALLBACK (changed_template), d);
	if (insert)
		fill_templates (d);

	gtk_container_add (GTK_CONTAINER (d->frame_sample), sample_frame (&d->sample));
	g_signal_handlers_disconnect_matched (d->sample, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, G_CALLBACK (url_requested), NULL);
	g_signal_connect (GTK_OBJECT (d->sample), "url_requested", G_CALLBACK (image_url_requested), d);

	d->entry_url = glade_xml_get_widget (xml, "entry_image_url");
	g_signal_connect (GTK_OBJECT (d->entry_url), "changed", G_CALLBACK (url_changed), d);

	d->entry_alt = glade_xml_get_widget (xml, "entry_image_alt");
	g_signal_connect (d->entry_alt, "changed", G_CALLBACK (alt_changed), d);

	d->pentry = glade_xml_get_widget (xml, "pentry_image_location");
	g_signal_connect (GTK_OBJECT (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->pentry))),
			    "changed", G_CALLBACK (pentry_changed), d);

	gtk_widget_show_all (d->page);
	if (!insert)
		gtk_widget_hide (frame_template);
	gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (d->pentry), FALSE);

	glade_xml_signal_connect_data (xml, "image_test_url", GTK_SIGNAL_FUNC (test_url_clicked), d);

	return d->page;
}

GtkWidget *
image_insertion (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkWidget *w;
	GtkHTMLEditImageProperties *d;

	*set_data = d = data_new (cd);
	w = image_widget (d, TRUE);

	set_ui (d);
	gtk_html_edit_properties_dialog_change (d->cd->properties_dialog);

	gtk_widget_show (w);

	return w;
}

static void
get_data (GtkHTMLEditImageProperties *d, HTMLImage *image)
{
	HTMLImagePointer *ip = image->image_ptr;
	gint off = 0;

	d->image = image;
        if (!strncmp (ip->url, "file://", 7))
		off = 7;
        else if (!strncmp (ip->url, "file:", 5))
		off = 5;
	d->location = g_strdup (ip->url + off);
	if (image->percent_width) {
		d->width_percent = 1;
		d->width = image->specified_width;
	} else if (image->specified_width > 0) {
		d->width_percent = 0;
		d->width = image->specified_width;
	} else
		d->width_percent = 2;
	if (image->percent_height) {
		d->height_percent = 1;
		d->height = image->specified_height;
	} else if (image->specified_height > 0) {
		d->height_percent = 0;
		d->height = image->specified_height;
	} else
		d->height_percent = 2;
	d->align  = image->valign;
	d->padh   = image->hspace;
	d->padv   = image->vspace;
	d->border = image->border;
	d->url    = image->url ? g_strconcat (image->url, image->target ? "#" : "", image->target, NULL) : g_strdup ("");
	d->alt    = g_strdup (image->alt);
}

GtkWidget *
image_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkWidget *w;
	GtkHTMLEditImageProperties *d;
	HTMLImage *image = HTML_IMAGE (cd->html->engine->cursor->object);

	g_assert (HTML_OBJECT_TYPE (cd->html->engine->cursor->object) == HTML_TYPE_IMAGE);

	*set_data = d = data_new (cd);
	w = image_widget (d, FALSE);

	get_data (d, image);
	set_ui (d);
	gtk_widget_show (w);

	return w;
}

static gboolean
insert_or_apply (GtkHTMLControlData *cd, gpointer get_data, gboolean insert)
{	
	GtkHTMLEditImageProperties *d = (GtkHTMLEditImageProperties *) get_data;

	if (insert) {
		gchar *html;

		html = get_sample_html (d, TRUE);
		gtk_html_append_html (d->cd->html, html);
	} else {
		HTMLImage *image = HTML_IMAGE (d->image);
		HTMLEngine *e = d->cd->html->engine;
		gchar *location, *url, *target;
		gint position;

		position = e->cursor->position;

		g_assert (HTML_OBJECT_TYPE (d->image) == HTML_TYPE_IMAGE);

		if (e->cursor->object != HTML_OBJECT (d->image))
			if (!html_cursor_jump_to (e->cursor, e, HTML_OBJECT (d->image), 1)) {
				GtkWidget *dialog;
				printf ("d: %p\n", d->cd->properties_dialog);
				dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
								 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
								 _("The editted image was removed from the document.\nCannot apply your changes."));
				gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				html_cursor_jump_to_position (e->cursor, e, position);
				return FALSE;
			}

		html_image_set_border (image, d->border);
		html_image_set_size (image,
				     d->width_percent == 2 ? 0 : d->width,
				     d->height_percent == 2 ? 0 : d->height,
				     d->width_percent == 1, d->height_percent == 1);
		html_image_set_spacing  (image, d->padh, d->padv);
		html_image_set_valign   (image, d->align);

		location = get_location (d);
		html_image_edit_set_url (image, location);
		g_free (location);
		html_image_set_alt (image, d->url);

		url = d->url;
		target = NULL;

		if (d->url) {
			target = strchr (d->url, '#');
			url = target ? g_strndup (d->url, target - d->url) : d->url;
			if (target)
				target ++;
		}

		html_object_set_link (HTML_OBJECT (d->image),
				      url && *url
				      ? html_colorset_get_color (d->cd->html->engine->settings->color_set, HTMLLinkColor)
				      : html_colorset_get_color (d->cd->html->engine->settings->color_set, HTMLTextColor),
				      url, target);
		if (target)
			g_free (url);
		g_free (target);
		html_cursor_jump_to_position (e->cursor, e, position);
	}

	return TRUE;
}

gboolean
image_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	return insert_or_apply (cd, get_data, FALSE);
}

gboolean
image_insert_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	insert_or_apply (cd, get_data, TRUE);
	return TRUE;
}

void
image_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditImageProperties *d = (GtkHTMLEditImageProperties *) get_data;

	g_free (d->url);
	g_free (d->alt);
	g_free (d->location);
	g_free (d);
}
