/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF Properties Display Widget
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <aconf.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-macros.h>
#include <glade/glade.h>
#include "gpdf-util.h"
#include "pdf-properties-display.h"

#define PARENT_TYPE GTK_TYPE_HBOX
GPDF_CLASS_BOILERPLATE (GPdfPropertiesDisplay, gpdf_properties_display,
			GtkHBox, PARENT_TYPE)

struct _GPdfPropertiesDisplayPrivate
{
	GladeXML *glade_xml;
};

enum {
	PROP_0 = 0,
	PROP_TITLE,
	PROP_SUBJECT,
	PROP_AUTHOR,
	PROP_KEYWORDS,
	PROP_CREATOR,
	PROP_PRODUCER,
	PROP_CREATED,
	PROP_MODIFIED,
	PROP_SECURITY,
	PROP_PDFVERSION,
	PROP_NUMPAGES,
	PROP_OPTIMIZED
};

/* From eel */
/**
 * gpdf_gtk_label_make_bold.
 *
 * Switches the font of label to a bold equivalent.
 * @label: The label.
 **/
static void
gpdf_gtk_label_make_bold (GtkLabel *label)
{
	PangoFontDescription *font_desc;

	font_desc = pango_font_description_new ();

	pango_font_description_set_weight (font_desc,
					   PANGO_WEIGHT_BOLD);

	/* 
	 * This will only affect the weight of the font, the rest is
	 * from the current state of the widget, which comes from the
	 * theme or user prefs, since the font desc only has the
	 * weight flag turned on.
	 */
	gtk_widget_modify_font (GTK_WIDGET (label), font_desc);

	pango_font_description_free (font_desc);
}

static void
gpdf_properties_display_set_label_text (GPdfPropertiesDisplay *pdisplay,
					const gchar *widget_name,
					const gchar *text)
{
	GladeXML *xml;
	GtkLabel *label;

	g_return_if_fail (GPDF_IS_PROPERTIES_DISPLAY (pdisplay));

	xml = pdisplay->priv->glade_xml;
	if (xml == NULL)
		return;

	label = GTK_LABEL (glade_xml_get_widget (xml, widget_name));
	
	gtk_label_set_text (label, text);	
}

static void
gpdf_properties_display_set_property (GObject *object,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *pspec)
{
	GPdfPropertiesDisplay *pdisplay;

	pdisplay = GPDF_PROPERTIES_DISPLAY (object);
	switch ((gint)prop_id) {
	case PROP_TITLE: 
		gpdf_properties_display_set_label_text (
			pdisplay, "Title_Display", g_value_get_string (value));
		break;
	case PROP_SUBJECT:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Subject_Display", g_value_get_string (value));
		break;
	case PROP_AUTHOR:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Author_Display", g_value_get_string (value));
		break;
	case PROP_KEYWORDS:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Keywords_Display", g_value_get_string (value));
		break;
	case PROP_CREATOR:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Creator_Display", g_value_get_string (value));
		break;
	case PROP_PRODUCER:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Producer_Display", g_value_get_string (value));
		break;
	case PROP_CREATED:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Created_Display", g_value_get_string (value));
		break;
	case PROP_MODIFIED:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Modified_Display", g_value_get_string (value));
		break;
	case PROP_SECURITY:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"Security_Display", g_value_get_string (value));
		break;
	case PROP_PDFVERSION:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"PDFVersion_Display", g_value_get_string (value));
		break;
	case PROP_NUMPAGES:
		gpdf_properties_display_set_label_text (
			pdisplay,
			"NumPages_Display", g_value_get_string (value));
		break;
	case PROP_OPTIMIZED:
		gpdf_properties_display_set_label_text (
			pdisplay, 
			"Optimized_Display", g_value_get_string (value));
		break;
	default:
		break;
	}
}

static void
gpdf_properties_display_dispose (GObject *object)
{
	GPdfPropertiesDisplayPrivate *priv;
	
	priv = GPDF_PROPERTIES_DISPLAY (object)->priv;

	if (priv->glade_xml) {
		g_object_unref (priv->glade_xml);
		priv->glade_xml = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_properties_display_finalize (GObject *object)
{
	GPdfPropertiesDisplay *pdisplay;

	pdisplay = GPDF_PROPERTIES_DISPLAY (object);
	
	if (pdisplay->priv) {
		g_free (pdisplay->priv);
		pdisplay->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

void
gpdf_properties_display_class_init (GPdfPropertiesDisplayClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gpdf_properties_display_dispose;
	object_class->finalize = gpdf_properties_display_finalize;

	object_class->set_property = gpdf_properties_display_set_property;

	g_object_class_install_property (
		object_class,
		PROP_TITLE,
		g_param_spec_string ("title",
				     "Title",
				     "Title of the document",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_SUBJECT,
		g_param_spec_string ("subject",
				     "Subject",
				     "Subject of the document",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_AUTHOR,
		g_param_spec_string ("author",
				     "Author",
				     "The person who created the document",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_KEYWORDS,
		g_param_spec_string ("keywords",
				     "Keywords",
				     "Keywords associated with the document",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_CREATOR,
		g_param_spec_string ("creator",
				     "Creator",
				     "The program that created the file that "
				     "was converted to PDF",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_PRODUCER,
		g_param_spec_string ("producer",
				     "Producer",
				     "The program that converted the original "
				     "file to PDF",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_CREATED,
		g_param_spec_string ("created",
				     "CreationDate",
				     "The date the document was created",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_MODIFIED,
		g_param_spec_string ("modified",
				     "ModificationDate",
				     "The date the document was last modified"
				     "using a PDF editor",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_SECURITY,
		g_param_spec_string ("security",
				     "Security",
				     "Security of the PDF file",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_PDFVERSION,
		g_param_spec_string ("pdfversion",
				     "PDFVersion",
				     "Version of the PDF specification the PDF "
				     "file conforms to",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_NUMPAGES,
		g_param_spec_string ("numpages",
				     "NumPages",
				     "Number of pages in the PDF file",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
	g_object_class_install_property (
		object_class,
		PROP_OPTIMIZED,
		g_param_spec_string ("optimized",
				     "Optimized",
				     "Is the PDF file optimized",
				     _("Unknown"),
				     G_PARAM_WRITABLE));
}

static const gchar *bold_labels [] = {
	"Title_Label", "Subject_Label", "Author_Label",	"Keywords_Label",
	"Creator_Label", "Producer_Label", "Created_Label", "Modified_Label",
	"Security_Label", "PDFVersion_Label",
	"NumPages_Label", "Optimized_Label"
};

static void
gpdf_properties_display_boldify_labels (GPdfPropertiesDisplay *pdisplay,
					GladeXML *xml)
{
	int i;

	for (i = 0; i < G_N_ELEMENTS (bold_labels); ++i) {
		GtkWidget *widget;
		
		widget = glade_xml_get_widget (xml, bold_labels [i]);
		gpdf_gtk_label_make_bold (GTK_LABEL (widget));
	}
}

static void
gpdf_properties_display_setup_glade (GPdfPropertiesDisplay *pdisplay)
{
	GladeXML *xml;
	GtkWidget *glade_widget;

	xml = glade_xml_new (DATADIR "/gpdf/glade/gpdf-properties-dialog.glade",
			     "pdf-properties",
			     NULL);
	pdisplay->priv->glade_xml = xml;

	glade_widget = glade_xml_get_widget (xml, "pdf-properties");
	gpdf_properties_display_boldify_labels (pdisplay, xml);

	gtk_widget_show (glade_widget);
	gtk_container_add (GTK_CONTAINER (pdisplay), glade_widget);
}

void
gpdf_properties_display_instance_init (GPdfPropertiesDisplay *pdisplay)
{
	GPdfPropertiesDisplayPrivate *priv;

	priv = g_new0 (GPdfPropertiesDisplayPrivate, 1);
	pdisplay->priv = priv;

	gpdf_properties_display_setup_glade (pdisplay);
}

