/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-paper-selector.c: A paper selector widget
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    James Henstridge <james@daa.com.au>
 *
 *  Copyright (C) 1998 James Henstridge <james@daa.com.au>
 *
 */

#include <config.h>

#include <string.h>
#include <math.h>
#include <atk/atk.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print-paper.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/private/gpa-node.h>
#include <libgnomeprint/private/gpa-utils.h>
#include <libgnomeprint/private/gnome-print-private.h>
#include <libgnomeprint/private/gnome-print-config-private.h>

#include "gnome-print-i18n.h"
#include "gnome-print-paper-selector.h"
#include "gnome-print-unit-selector.h"

/* This two have to go away */
#include "gnome-print-paper-preview.h"
#include "gpaui/gpa-paper-preview-item.h"

#include "gpaui/gpa-option-menu.h"
#include "gpaui/gpa-spinbutton.h"

#define MM(v) ((v) * 72.0 / 25.4)
#define CM(v) ((v) * 72.0 / 2.54)
#define M(v)  ((v) * 72.0 / 0.0254)

#define PAD 6

/*
 * GnomePaperSelector widget
 */

struct _GnomePaperSelector {
	GtkHBox box;

	GnomePrintConfig *config;
	gint flags;

	GtkWidget *preview;

	GtkWidget *pmenu, *pomenu, *lomenu, *lymenu, *trmenu;
	GnomePrintUnitSelector *us;

	gdouble mt, mb, ml, mr; /* Values for margins   */
	gdouble pw, ph;         /* Values for page size */

	struct {GPASpinbutton *t, *b, *l, *r;} m; /* Spins for margins   */
	struct {GPASpinbutton *w, *h;} p;         /* Spins for page size */

	GPANode *paper_size_node;
	GPANode *printer;
	
	guint handler_unit, handler_preview, handler_paper_size, handler_printer;
};

struct _GnomePaperSelectorClass {
	GtkHBoxClass parent_class;
};

static void gnome_paper_selector_class_init (GnomePaperSelectorClass *klass);
static void gnome_paper_selector_init (GnomePaperSelector *selector);
static void gnome_paper_selector_finalize (GObject *object);

static GtkHBoxClass *selector_parent_class;

GType
gnome_paper_selector_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePaperSelectorClass),
			NULL, NULL,
			(GClassInitFunc) gnome_paper_selector_class_init,
			NULL, NULL,
			sizeof (GnomePaperSelector),
			0,
			(GInstanceInitFunc) gnome_paper_selector_init
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomePaperSelector", &info, 0);
	}
	
	return type;
}

static void
gnome_paper_selector_class_init (GnomePaperSelectorClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	selector_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gnome_paper_selector_finalize;
}

static void
gnome_paper_selector_update_spin_limits (GnomePaperSelector *ps)
{
	g_return_if_fail (GNOME_IS_PAPER_SELECTOR (ps));

	/*
	 * Margins cannot be larger than the page size minus the
	 * opposite margin
	 */
	ps->m.t->upper = ps->ph - ps->mb; gpa_spinbutton_update (ps->m.t);
	ps->m.b->upper = ps->ph - ps->mt; gpa_spinbutton_update (ps->m.b);
	ps->m.r->upper = ps->pw - ps->ml; gpa_spinbutton_update (ps->m.r);
	ps->m.l->upper = ps->pw - ps->mr; gpa_spinbutton_update (ps->m.l);
}

static void
gps_psize_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	g_return_if_fail (GNOME_IS_PAPER_SELECTOR (ps));

	/* Did the value really change? */
	if ((fabs (ps->pw - ps->p.w->value) < 0.1) &&
	    (fabs (ps->ph - ps->p.h->value) < 0.1))
		return; 
	ps->pw = ps->p.w->value;
	ps->ph = ps->p.h->value;

	gnome_paper_selector_update_spin_limits (ps);
}

static void
gps_m_size_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	g_return_if_fail (GNOME_IS_PAPER_SELECTOR (ps));

	/* Did the value really change? */
	if ((fabs (ps->mt - ps->m.t->value) < 0.1) &&
	    (fabs (ps->mb - ps->m.b->value) < 0.1) &&
	    (fabs (ps->ml - ps->m.l->value) < 0.1) &&
	    (fabs (ps->mr - ps->m.r->value) < 0.1))
		return; 
	ps->ml = ps->m.l->value;
	ps->mr = ps->m.r->value;
	ps->mt = ps->m.t->value;
	ps->mb = ps->m.b->value;

	gpa_paper_preview_item_set_logical_margins 
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW 
					 (ps->preview)->item),
		 ps->ml, ps->mr, ps->mt, ps->mb);
	gnome_paper_selector_update_spin_limits (ps);
}

static void
gnome_paper_selector_init (GnomePaperSelector *ps)
{
	ps->config = NULL;
	ps->flags = 0;
}

static void
gnome_paper_selector_finalize (GObject *object)
{
	GnomePaperSelector *ps;

	ps = GNOME_PAPER_SELECTOR (object);

	ps->preview = NULL;

	if (ps->config) {
		GObject *node;

		node = G_OBJECT (gnome_print_config_get_node (ps->config));

		if (ps->handler_preview) {
			g_signal_handler_disconnect (node, ps->handler_preview);
			ps->handler_preview = 0;
		}

		if (ps->handler_unit) {
			g_signal_handler_disconnect (node, ps->handler_unit);
			ps->handler_unit = 0;
		}

		if (ps->handler_printer) {
			g_signal_handler_disconnect (ps->printer, ps->handler_printer);
			ps->handler_printer = 0;
		}

		if (ps->handler_paper_size) {
			g_signal_handler_disconnect (ps->paper_size_node, ps->handler_paper_size);
			ps->handler_paper_size = 0;
			
			gpa_node_unref (ps->paper_size_node);
			ps->paper_size_node =  NULL;
		}

		gnome_print_config_unref (ps->config);
		ps->config = NULL;
	}
	
	G_OBJECT_CLASS (selector_parent_class)->finalize (object);
}

static gboolean
lmargin_top_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gpa_paper_preview_item_set_lm_highlights
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 TRUE, FALSE, FALSE, FALSE);
	return FALSE;
}

static gboolean
lmargin_bottom_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gpa_paper_preview_item_set_lm_highlights
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, TRUE, FALSE, FALSE);
	return FALSE;
}

static gboolean
lmargin_left_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gpa_paper_preview_item_set_lm_highlights
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, TRUE, FALSE);
	return FALSE;
}

static gboolean
lmargin_right_unit_activated (GtkSpinButton *spin_button,
		GdkEventFocus *event,
		GnomePaperSelector *ps)
{
	gpa_paper_preview_item_set_lm_highlights
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, FALSE, TRUE);
	return FALSE;
}

static gboolean
lmargin_unit_deactivated (GtkSpinButton *spin_button,
		  GdkEventFocus *event,
		  GnomePaperSelector *ps)
{
	gpa_paper_preview_item_set_lm_highlights
		(GPA_PAPER_PREVIEW_ITEM (GNOME_PAPER_PREVIEW (ps->preview)->item),
		 FALSE, FALSE, FALSE, FALSE);
	return FALSE;
}

static void
gnome_print_paper_selector_update_spin_units (GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;

	g_return_if_fail (GNOME_IS_PAPER_SELECTOR (ps));

	unit = gnome_print_unit_selector_get_unit (ps->us);
	if (!unit) return;

	gpa_spinbutton_set_unit (ps->m.t, unit->abbr);
	gpa_spinbutton_set_unit (ps->m.b, unit->abbr);
	gpa_spinbutton_set_unit (ps->m.r, unit->abbr);
	gpa_spinbutton_set_unit (ps->m.l, unit->abbr);
	gpa_spinbutton_set_unit (ps->p.h, unit->abbr);
	gpa_spinbutton_set_unit (ps->p.w, unit->abbr);
}

static void
gnome_paper_selector_unit_changed_cb (GnomePrintUnitSelector *sel, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit = gnome_print_unit_selector_get_unit (sel);
	if (NULL != unit)
		gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT, unit->abbr);
	gnome_print_paper_selector_update_spin_units (ps);
}

static void
gnome_paper_unit_selector_request_update_cb (GPANode *node, guint flags,
					     GnomePaperSelector *ps)
{
	guchar *unit_txt;

	unit_txt = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT);
	if (!unit_txt) {
		g_warning ("Could not get GNOME_PRINT_KEY_PREFERED_UNIT");
		return;
	}

	gnome_print_unit_selector_set_unit (ps->us,
			gnome_print_unit_get_by_abbreviation (unit_txt));
	g_free (unit_txt);
	gnome_print_paper_selector_update_spin_units (ps);
}

/**
 * gnome_paper_selector_paper_size_modified_cb
 * @node: 
 * @flags: 
 * @ps: 
 * 
 * Handle paper sizes changes
 **/
static void
gnome_paper_selector_paper_size_modified_cb (GPANode *node, guint flags, GnomePaperSelector *ps)
{
	guchar *id;
	gboolean sensitivity = FALSE;

	id = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PAPER_SIZE);
	if (id) {
		if (*id && !strcmp (id, "Custom")) sensitivity = TRUE;
		g_free (id);
	}

	gtk_widget_set_sensitive (GTK_WIDGET (ps->p.w), sensitivity);
	gtk_widget_set_sensitive (GTK_WIDGET (ps->p.h), sensitivity);

	gnome_paper_selector_update_spin_limits (ps);
}

static void
gnome_paper_selector_hook_paper_size (GnomePaperSelector *ps)
{
	if (ps->handler_paper_size) {
		g_signal_handler_disconnect (G_OBJECT (ps->paper_size_node), ps->handler_paper_size);
		ps->handler_paper_size = 0;
	}
	
	if (ps->paper_size_node) {
		gpa_node_unref (ps->paper_size_node);
		ps->paper_size_node = NULL;
	}

	ps->paper_size_node = gpa_node_get_child_from_path (GNOME_PRINT_CONFIG_NODE (ps->config),
							    GNOME_PRINT_KEY_PAPER_SIZE);

	if (ps->paper_size_node) {
		ps->handler_paper_size = g_signal_connect (
			G_OBJECT (ps->paper_size_node), "modified",
			(GCallback) gnome_paper_selector_paper_size_modified_cb, ps);
	} else {
		g_print ("No paper size node\n");
	}
	
	gnome_paper_selector_paper_size_modified_cb (NULL, 0, ps);
}

static void
gnome_paper_selector_printer_changed_cb (GPANode *node, guint flags, GnomePaperSelector *ps)
{
	gnome_paper_selector_hook_paper_size (ps);
}

static void
gnome_paper_selector_construct (GnomePaperSelector *ps)
{
	GtkWidget *vb, *f, *t, *l, *s;
	GtkWidget *margin_table, *margin_label;
	gdouble ml, mr, mt, mb, pw = 1., ph = 1.;
	AtkObject *atko;
	GPANode *printer;
	gchar *text;

	g_return_if_fail (ps != NULL);
	g_return_if_fail (ps->config != NULL);

	printer = gpa_node_get_child_from_path (GNOME_PRINT_CONFIG_NODE (ps->config), "Printer");
	g_return_if_fail (printer != NULL);
	ps->printer = printer;

	/* Fetch the current settings. */
	ml = MM(10);
	mr = MM(10);
	mt = MM(10);
	mb = MM(10);
	gnome_print_config_get_length (ps->config,
				GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &ml, NULL);
	gnome_print_config_get_length (ps->config,
				GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &mr, NULL);
	gnome_print_config_get_length (ps->config,
				GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &mt, NULL);
	gnome_print_config_get_length (ps->config,
				GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &mb, NULL);
	ps->ml = ml;
	ps->mr = mr;
	ps->mt = mt;
	ps->mb = mb;
	gnome_print_config_get_length (ps->config,
			GNOME_PRINT_KEY_PAPER_WIDTH, &pw, NULL);
	gnome_print_config_get_length (ps->config,
			GNOME_PRINT_KEY_PAPER_HEIGHT, &ph, NULL);
	ps->ph = ph;
	ps->pw = pw;

	gtk_box_set_spacing (GTK_BOX (ps), PAD);

	/* VBox for controls */
	vb = gtk_vbox_new (FALSE, PAD);
	gtk_widget_show (vb);
	gtk_box_pack_start (GTK_BOX (ps), vb, FALSE, FALSE, 0);

	/* Create frame for selection menus */
	f = gtk_frame_new ("");
	gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);
	l = gtk_label_new ("");
	text = g_strdup_printf ("<b>%s</b>", _("Paper and Layout"));
	gtk_label_set_markup (GTK_LABEL (l), text);
	g_free (text);
	gtk_frame_set_label_widget (GTK_FRAME (f), l);
	gtk_widget_show (l);
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (vb), f, FALSE, FALSE, 0);

	/* Create table for packing menus */
	t = gtk_table_new (4, 6, FALSE);
	gtk_widget_show (t);
	gtk_container_set_border_width (GTK_CONTAINER (t), PAD);
	gtk_table_set_row_spacings (GTK_TABLE (t), 2);
	gtk_table_set_col_spacings (GTK_TABLE (t), 4);
	gtk_container_add (GTK_CONTAINER (f), t);

	/* Paper size selector */
	l = gtk_label_new_with_mnemonic (_("Paper _size:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 0, 1);

	ps->pmenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_PAPER_SIZE);
	gtk_widget_show (ps->pmenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pmenu, 1, 4, 0, 1);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, GPA_OPTION_MENU (ps->pmenu)->menu);

	/* Custom paper Width */
	l = gtk_label_new_with_mnemonic (_("_Width:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 1, 2);
	s = gpa_spinbutton_new (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,
		0.0001, 10000, 1, 10, 10, 1, 2);
	ps->p.w = GPA_SPINBUTTON (s);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (ps->p.w->spinbutton),
				     TRUE);
	gtk_widget_show (s);
	gtk_table_attach_defaults (GTK_TABLE (t), s, 2, 3, 1, 2);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->p.w->spinbutton);

	/* Custom paper Height */
	l = gtk_label_new_with_mnemonic (_("_Height:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 2, 3);
	s = gpa_spinbutton_new (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT,
			0.0001, 10000, 1, 10, 10, 1, 2);
	ps->p.h = GPA_SPINBUTTON (s);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (ps->p.h->spinbutton),
				     TRUE);
	gtk_widget_show (s);
	gtk_table_attach_defaults (GTK_TABLE (t), s, 2, 3, 2, 3);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->p.h->spinbutton);

	/* Custom paper Unit selector */
	s = gnome_print_unit_selector_new (GNOME_PRINT_UNIT_ABSOLUTE);
	ps->us = GNOME_PRINT_UNIT_SELECTOR (s);
	gtk_table_attach_defaults (GTK_TABLE (t), s, 3, 4, 1, 2);
	atko = gtk_widget_get_accessible (s);
	atk_object_set_name (atko, _("Metric selector"));
	atk_object_set_description (atko, _("Specifies the metric to use when setting the width and height of the paper"));

	/* Feed orientation */
	l = gtk_label_new_with_mnemonic (_("_Feed orientation:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 3, 4);

	ps->pomenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_PAPER_ORIENTATION);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pomenu, 1, 4, 3, 4);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, GPA_OPTION_MENU (ps->pomenu)->menu);

	if (ps->flags & GNOME_PAPER_SELECTOR_FEED_ORIENTATION || TRUE) {
		gtk_widget_show_all (ps->pomenu);
		gtk_widget_show_all (l);
	}

	/* Page orientation */
	l = gtk_label_new_with_mnemonic (_("Page _orientation:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 4, 5);

	ps->lomenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_PAGE_ORIENTATION);
	gtk_widget_show (ps->lomenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->lomenu, 1, 4, 4, 5);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, GPA_OPTION_MENU (ps->lomenu)->menu);

	/* Layout */
	l = gtk_label_new_with_mnemonic (_("_Layout:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 5, 6);

	ps->lymenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_LAYOUT);
	gtk_widget_show (ps->lymenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->lymenu, 1, 4, 5, 6);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, GPA_OPTION_MENU (ps->lymenu)->menu);

	/* Paper source */
	l = gtk_label_new_with_mnemonic (_("Paper _tray:"));
	gtk_widget_show(l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 6, 7);

	ps->trmenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_PAPER_SOURCE);
	gtk_widget_show(ps->trmenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->trmenu, 1, 4, 6, 7);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, GPA_OPTION_MENU (ps->trmenu)->menu);

	/* Preview frame */
	f = gtk_frame_new ("");
	gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);
	l = gtk_label_new ("");
	text = g_strdup_printf ("<b>%s</b>", _("Preview"));
	gtk_label_set_markup (GTK_LABEL (l), text);
	g_free (text);
	gtk_frame_set_label_widget (GTK_FRAME (f), l);
	gtk_widget_show (l);
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (ps), f, TRUE, TRUE, 0);

	ps->preview = gnome_paper_preview_new (ps->config);
	gtk_widget_set_size_request (ps->preview, 160, 160);
	gtk_widget_show (ps->preview);
	gtk_container_add (GTK_CONTAINER (f), ps->preview);

	atko = gtk_widget_get_accessible (ps->preview);
	atk_object_set_name (atko, _("Preview"));
	atk_object_set_description (atko, _("Preview of the page size, orientation and layout"));

	/* Margins */
	f = gtk_frame_new ("");
	gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_NONE);
	l = gtk_label_new ("");
	text = g_strdup_printf ("<b>%s</b>", _("Margins"));
	gtk_label_set_markup (GTK_LABEL (l), text);
	g_free (text);
	gtk_frame_set_label_widget (GTK_FRAME (f), l);
	gtk_widget_show (l);

	gtk_box_pack_start (GTK_BOX (ps), f, FALSE, FALSE, 0);
	margin_table = gtk_table_new ( 8, 1, TRUE);
	gtk_container_set_border_width  (GTK_CONTAINER (margin_table), 4);

	s = gpa_spinbutton_new (ps->config,
		GNOME_PRINT_KEY_PAGE_MARGIN_TOP, 0., ps->ph,
		1., 10., 10., 1., 2.);
	ps->m.t = GPA_SPINBUTTON (s);
	gtk_table_attach (GTK_TABLE (margin_table), s,
			  0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	s = gpa_spinbutton_new (ps->config,
		GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, 0., ps->ph,
		1, 10, 10, 1., 2.);
	ps->m.b = GPA_SPINBUTTON (s);
	gtk_table_attach (GTK_TABLE (margin_table), s,
			   0, 1, 7, 8,  GTK_FILL, GTK_FILL, 0, 0);
	s = gpa_spinbutton_new (ps->config,
		GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, 0., ps->pw,
		1, 10, 10, 1., 2.);
	ps->m.l = GPA_SPINBUTTON (s);
	gtk_table_attach (GTK_TABLE (margin_table), s,
			  0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	s = gpa_spinbutton_new (ps->config,
		GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, 0., ps->pw,
		1, 10, 10, 1., 2.);
	ps->m.r = GPA_SPINBUTTON (s);
	gtk_table_attach (GTK_TABLE (margin_table), s, 
			  0, 1, 5, 6, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Top"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Bottom"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 6, 7, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Left"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);
	margin_label = gtk_label_new (_("Right"));
	gtk_table_attach (GTK_TABLE (margin_table), margin_label,
			  0, 1, 4, 5, GTK_FILL, GTK_FILL, 0, 0);
	gtk_container_add (GTK_CONTAINER (f), margin_table);
	if (ps->flags & GNOME_PAPER_SELECTOR_MARGINS)
		gtk_widget_show_all (f);

	/* Hook up the Custom size widgets */
	gnome_paper_selector_hook_paper_size (ps);
	ps->handler_printer = g_signal_connect (G_OBJECT (ps->printer),
		"modified", (GCallback) gnome_paper_selector_printer_changed_cb, ps);

	/* Setup the unit selector */
	gnome_paper_unit_selector_request_update_cb (NULL, 0, ps);
	g_signal_connect (G_OBJECT (ps->us), "modified",
		G_CALLBACK (gnome_paper_selector_unit_changed_cb), ps);
	ps->handler_unit = g_signal_connect (
		G_OBJECT (gnome_print_config_get_node (ps->config)), "modified",
		G_CALLBACK (gnome_paper_unit_selector_request_update_cb), ps);

	/* Connect signals */
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->p.w->spinbutton)->adjustment),
		"value_changed", G_CALLBACK (gps_psize_value_changed), ps);
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->p.h->spinbutton)->adjustment), 
		"value_changed", G_CALLBACK (gps_psize_value_changed), ps);
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->m.t->spinbutton)->adjustment),
		"value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->m.b->spinbutton)->adjustment),
		"value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->m.l->spinbutton)->adjustment),
		"value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (
		G_OBJECT (GTK_SPIN_BUTTON (ps->m.r->spinbutton)->adjustment),
		"value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (ps->m.t), "focus_in_event",
		G_CALLBACK (lmargin_top_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->m.t->spinbutton), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->m.l->spinbutton), "focus_in_event",
		G_CALLBACK (lmargin_left_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->m.l->spinbutton), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->m.r->spinbutton), "focus_in_event",
		G_CALLBACK (lmargin_right_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->m.r->spinbutton), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->m.b->spinbutton), "focus_in_event",
		G_CALLBACK (lmargin_bottom_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->m.b->spinbutton), "focus_out_event",
		G_CALLBACK (lmargin_unit_deactivated), ps);

	gtk_widget_show (GTK_WIDGET (ps->us));
}

GtkWidget *
gnome_paper_selector_new_with_flags (GnomePrintConfig *config, gint flags)
{
	GnomePaperSelector *selector;

	selector = g_object_new (GNOME_TYPE_PAPER_SELECTOR, NULL);

	if (config) {
		selector->config = gnome_print_config_ref (config);
	} else {
		selector->config = gnome_print_config_default ();
	}
	selector->flags = flags;

	gnome_paper_selector_construct (selector);

	return (GtkWidget *) selector;
}

GtkWidget *
gnome_paper_selector_new (GnomePrintConfig *config)
{
	return gnome_paper_selector_new_with_flags (config, 0);
}

