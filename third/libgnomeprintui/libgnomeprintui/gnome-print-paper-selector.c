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

#define MM(v) ((v) * 72.0 / 25.4)
#define CM(v) ((v) * 72.0 / 2.54)
#define M(v)  ((v) * 72.0 / 0.0254)

#define PAD 4

/*
 * GnomePaperSelector widget
 */

struct _GnomePaperSelector {
	GtkHBox box;

	guint updating : 1;

	GnomePrintConfig *config;
	gint flags;

	GtkWidget *preview;

	GtkWidget *pmenu, *pomenu, *lomenu, *lymenu;
	GtkWidget *pw, *ph, *us;
	GtkWidget *pf;
	gdouble w, h;

	GtkWidget *margin_frame;
	GtkSpinButton *margin_top,  *margin_bottom, *margin_left, *margin_right;
	gdouble mt, mb, ml, mr;

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

typedef struct {
	guchar *id, *name;
	gdouble affine[6];
} GPPOrientation;

static const GPPOrientation porient[] = {
	{"R0", "Straight", {1, 0, 0, 1, 0, 0}},
	{"R90", "Rotated 90 degrees", {0, -1, 1, 0, 0, 1}},
	{"R180", "Rotated 180 degrees", {-1, 0, 0, -1, 1, 1}},
	{"R270", "Rotated 270 degrees", {0, 1, -1, 0, 1, 0}}
};

static const GPPOrientation lorient[] = {
	{"R0", "Portrait", {1, 0, 0, 1, 0, 0}},
	{"R90", "Landscape", {0, 1, -1, 0, 0, 1}},
	{"R180", "Upside down portrait", {-1, 0, 0, -1, 1, 1}},
	{"R270", "Upside down landscape", {0, -1, 1, 0, 1, 0}}
};

typedef struct {
	guchar *id;
	guchar *name;
	gdouble width, height;
	gint num_affines;
	const gdouble *affines;
} GPPLayout;

static const gdouble lyid[] = {1, 0, 0, 1, 0, 0};
static const gdouble ly21[] = {0, -0.707, 0.707, 0, 0, 1,
			       0, -0.707, 0.707, 0, 0, 0.5};
static const gdouble ly41[] = {0.5, 0, 0, 0.5, 0, 0.5,
			       0.5, 0, 0, 0.5, 0.5, 0.5,
			       0.5, 0, 0, 0.5, 0, 0,
			       0.5, 0, 0, 0.5, 0.5, 0};
static const gdouble ly2[] = {0, -1, 1, 0, 0, 1,
			       0, -1, 1, 0, 0, 0.5};
static const gdouble ly2f[] = {0, -1, 1, 0, 0, 0.5,
			       0, 1, -1, 0, 1, 0.5};

static const GPPLayout layout[] = {
	{"Plain", "Plain", 1, 1, 1, lyid},
	{"2_1", "2 Pages to 1", 0.5, 1, 2, ly21},
	{"4_1", "4 Pages to 1", 0.5, 0.5, 4, ly41},
	{"I2_1", "Divided", 0.5, 1, 2, ly2},
	{"IM2_1", "Folded", 0.5, 1, 2, ly2f}
};
#define NUM_LAYOUTS (sizeof (layout) / sizeof (layout[0]))

static void
gps_psize_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;
	gdouble w, h, max_wh;

	if (ps->updating)
		return;

	unit = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));
	w = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->pw));

	gnome_print_convert_distance (&w, unit, GNOME_PRINT_PS_UNIT);
	h = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->ph));

	gnome_print_convert_distance (&h, unit, GNOME_PRINT_PS_UNIT);

	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,
				       w, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT,
				       h, GNOME_PRINT_PS_UNIT);

	if ((fabs (ps->w - w) < 0.1) && (fabs (ps->h - h) < 0.1))
		return;

	ps->w = w;
	ps->h = h;
	max_wh = MAX (w, h);
	gtk_spin_button_get_adjustment (ps->margin_top)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_top));
	gtk_spin_button_get_adjustment (ps->margin_bottom)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_bottom));
	gtk_spin_button_get_adjustment (ps->margin_left)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_left));
	gtk_spin_button_get_adjustment (ps->margin_right)->upper = max_wh;
	gtk_adjustment_changed (gtk_spin_button_get_adjustment (ps->margin_right));
}

static void
gps_m_size_value_changed (GtkAdjustment *adj, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;
	gdouble mt, mb, ml, mr;

	unit = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));

	mt = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_top));
	gnome_print_convert_distance (&mt, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, mt, GNOME_PRINT_PS_UNIT);

	mb = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_bottom));
	gnome_print_convert_distance (&mb, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, mb, GNOME_PRINT_PS_UNIT);

	ml = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_left));
	gnome_print_convert_distance (&ml, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, ml, GNOME_PRINT_PS_UNIT);

	mr = gtk_spin_button_get_value (GTK_SPIN_BUTTON (ps->margin_right));
	gnome_print_convert_distance (&mr, unit, GNOME_PRINT_PS_UNIT);
	gnome_print_config_set_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, mr, GNOME_PRINT_PS_UNIT);

	if ((fabs (ps->mt - mt) < 0.1) && (fabs (ps->mb - mb) < 0.1) &&
	    (fabs (ps->ml - ml) < 0.1) && (fabs (ps->mr - mr) < 0.1))
		return;

	ps->ml = ml;
	ps->mr = mr;
	ps->mt = mt;
	ps->mb = mb;

}

static void
gps_set_labelled_by_relation (GtkWidget *widget, GtkWidget *label)
{
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];
	AtkObject *atko;

	atko = gtk_widget_get_accessible (widget);
	relation_set = atk_object_ref_relation_set (atko);
	relation_targets[0] = gtk_widget_get_accessible (label);
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));
}

static void
gnome_paper_selector_init (GnomePaperSelector *ps)
{
	ps->config = NULL;
	ps->flags = 0;
	ps->updating = FALSE;
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

typedef struct {
	guchar *abbr;
	gint digits;
	gfloat step_increment;
} GnomePrintPaperSelectorSpinProps_t ;

static const GnomePrintPaperSelectorSpinProps_t gpps_spin_props[] = {
	{N_("Pt"), 1, 1.0},
	{N_("mm"), 1, 1.0},
	{N_("cm"), 2, 0.5},
	{N_("m"),  3, 0.01},
	{N_("in"), 2, 0.25},
	{NULL,     2, 1.0}          /* Default must be last */
};

static void
gnome_paper_selector_spin_adapt_to_unit (GtkSpinButton *spin, const GnomePrintUnit *unit)
{
	gint num_of_units = sizeof (gpps_spin_props) / sizeof (GnomePrintPaperSelectorSpinProps_t);
	gint i;
	GtkAdjustment *adjustment;

	g_return_if_fail (GTK_IS_SPIN_BUTTON (spin));
	adjustment = gtk_spin_button_get_adjustment (spin);
	g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));

	for (i = 0; i < num_of_units; i++) {
		if (gpps_spin_props[i].abbr == NULL ||
		    !strcmp (unit->abbr, gpps_spin_props[i].abbr)) {
			adjustment->step_increment = gpps_spin_props[i].step_increment;
			adjustment->page_increment = gpps_spin_props[i].step_increment * 10;
			gtk_adjustment_changed (adjustment);
			gtk_spin_button_set_digits (spin, gpps_spin_props[i].digits);
			return;
		}
	}
}

static void
gnome_paper_selector_unit_changed_cb (GnomePrintUnitSelector *sel, GnomePaperSelector *ps)
{
	const GnomePrintUnit *unit;

	g_return_if_fail (ps != NULL);

	unit = gnome_print_unit_selector_get_unit (sel);
	if (unit) {
		gnome_print_config_set (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT, unit->abbr);
		gnome_paper_selector_spin_adapt_to_unit (GTK_SPIN_BUTTON (ps->pw), unit);
		gnome_paper_selector_spin_adapt_to_unit (GTK_SPIN_BUTTON (ps->ph), unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_top, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_bottom, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_left, unit);
		gnome_paper_selector_spin_adapt_to_unit (ps->margin_right, unit);
	}
}

static void
gnome_paper_unit_selector_request_update_cb (GPANode *node, guint flags,  GnomePaperSelector *ps)
{
	guchar *unit_txt;

	if (ps->updating)
		return;

	unit_txt = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PREFERED_UNIT);
	if (!unit_txt) {
		g_warning ("Could not get GNOME_PRINT_KEY_PREFERED_UNIT");
		return;
	}

	gnome_print_unit_selector_set_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us),
					    gnome_print_unit_get_by_abbreviation
					    (unit_txt));
	g_free (unit_txt);
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
	const GnomePrintUnit *unit, *to;
	guchar *id;
	gboolean sensitivity = FALSE;
	gdouble height, width;
	gint retval = 0;

	id = gnome_print_config_get (ps->config, GNOME_PRINT_KEY_PAPER_SIZE);
	if (id && *id && !strcmp (id, "Custom"))
		sensitivity = TRUE;
	
	gtk_widget_set_sensitive (ps->pw, sensitivity);
	gtk_widget_set_sensitive (ps->ph, sensitivity);

	to = gnome_print_unit_selector_get_unit (GNOME_PRINT_UNIT_SELECTOR (ps->us));
	height = 0.0;
	width  = 0.0;
	retval += gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT, &height, &unit);
	retval += gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,  &width,  &unit);
	g_return_if_fail (retval == 2 && to != NULL);
	gnome_print_convert_distance (&width,  unit, to);
	gnome_print_convert_distance (&height, unit, to);
	ps->updating = TRUE;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->ph), height);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (ps->pw), width);
	ps->updating = FALSE;
	if (id)
		g_free (id);
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
	GtkWidget *vb, *f, *t, *l;
	GtkWidget *margin_table, *margin_label;
	gdouble ml, mr, mt, mb;
	GtkObject *wa, *ha;
	AtkObject *atko;
	gdouble config_height, config_width, config_max;
	GPANode *printer;

	g_return_if_fail (ps != NULL);
	g_return_if_fail (ps->config != NULL);

	printer = gpa_node_get_child_from_path (GNOME_PRINT_CONFIG_NODE (ps->config), "Printer");
	g_return_if_fail (printer != NULL);
	ps->printer = printer;
	
	gtk_box_set_spacing (GTK_BOX (ps), PAD);

	/* VBox for controls */
	vb = gtk_vbox_new (FALSE, PAD);
	gtk_widget_show (vb);
	gtk_box_pack_start (GTK_BOX (ps), vb, FALSE, FALSE, 0);

	/* Create frame for selection menus */
	f = gtk_frame_new (_("Paper and layout"));
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
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pmenu);
	gps_set_labelled_by_relation (ps->pmenu, l);

	/* Custom paper Width */
	l = gtk_label_new_with_mnemonic (_("_Width:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 1, 2);

	config_width = 1;
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_WIDTH,
				       &config_width, NULL);
	ps->w = config_width;
	wa = gtk_adjustment_new (config_width, 0.0001, 10000, 1, 10, 10);
	ps->pw = gtk_spin_button_new (GTK_ADJUSTMENT (wa), 1, 2);
	gtk_widget_show (ps->pw);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pw, 2, 3, 1, 2);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pw);
	gps_set_labelled_by_relation (ps->pw, l);

	/* Custom paper Height */
	l = gtk_label_new_with_mnemonic (_("_Height:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 1, 2, 2, 3);

	config_height = 1;
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAPER_HEIGHT,
				       &config_height, NULL);
	ps->h = config_height;
	ha = gtk_adjustment_new (config_height, 0.0001, 10000, 1, 10, 10);
	ps->ph = gtk_spin_button_new (GTK_ADJUSTMENT (ha), 1, 2);
	gtk_widget_show (ps->ph);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->ph, 2, 3, 2, 3);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->ph);
	gps_set_labelled_by_relation (ps->ph, l);

	/* Custom paper Unit selector */
	ps->us = gnome_print_unit_selector_new (GNOME_PRINT_UNIT_ABSOLUTE);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->us, 3, 4, 1, 2);
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us), GTK_ADJUSTMENT (wa));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us), GTK_ADJUSTMENT (ha));
	g_signal_connect (G_OBJECT (wa), "value_changed", (GCallback) gps_psize_value_changed, ps);
	g_signal_connect (G_OBJECT (ha), "value_changed", (GCallback) gps_psize_value_changed, ps);
	atko = gtk_widget_get_accessible (ps->us);
	atk_object_set_name (atko, _("Metric selector"));
	atk_object_set_description (atko, _("Specifies the metric to use when setting the width and height of the paper"));

	/* Feed orientation */
	l = gtk_label_new_with_mnemonic (_("_Feed orientation:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 3, 4);

	ps->pomenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_PAPER_ORIENTATION);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->pomenu, 1, 4, 3, 4);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->pomenu);
	gps_set_labelled_by_relation (ps->pomenu, l);

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
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->lomenu);
	gps_set_labelled_by_relation (ps->lomenu, l);

	/* Layout */
	l = gtk_label_new_with_mnemonic (_("_Layout:"));
	gtk_widget_show (l);
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (t), l, 0, 1, 5, 6);

	ps->lymenu = gpa_option_menu_new (ps->config, GNOME_PRINT_KEY_LAYOUT);
	gtk_widget_show (ps->lymenu);
	gtk_table_attach_defaults (GTK_TABLE (t), ps->lymenu, 1, 4, 5, 6);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, ps->lymenu);
	gps_set_labelled_by_relation (ps->lymenu, l);

	/* Preview frame */
	ps->pf = gtk_frame_new (_("Preview"));
	gtk_widget_show (ps->pf);
	gtk_box_pack_start (GTK_BOX (ps), ps->pf, TRUE, TRUE, 0);

	ps->preview = gnome_paper_preview_new (ps->config);
	gtk_widget_set_size_request (ps->preview, 160, 160);
	gtk_widget_show (ps->preview);
	gtk_container_add (GTK_CONTAINER (ps->pf), ps->preview);

	atko = gtk_widget_get_accessible (ps->preview);
	atk_object_set_name (atko, _("Preview"));
	atk_object_set_description (atko, _("Preview of the page size, orientation and layout"));

	/* Margins */
	ml = MM(10);
	mr = MM(10);
	mt = MM(10);
	mb = MM(10);
	
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_LEFT, &ml, NULL);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_RIGHT, &mr, NULL);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_TOP, &mt, NULL);
	gnome_print_config_get_length (ps->config, GNOME_PRINT_KEY_PAGE_MARGIN_BOTTOM, &mb, NULL);

	ps->ml = ml;
	ps->mr = mr;
	ps->mt = mt;
	ps->mb = mb;

	ps->margin_frame = gtk_frame_new (_("Margins"));
	gtk_box_pack_start (GTK_BOX (ps), ps->margin_frame, FALSE, FALSE, 0);
	margin_table = gtk_table_new ( 8, 1, TRUE);
	gtk_container_set_border_width  (GTK_CONTAINER (margin_table), 4);

	config_max = MAX (config_height, config_width);
	ps->margin_top = GTK_SPIN_BUTTON (gtk_spin_button_new
					  (GTK_ADJUSTMENT (gtk_adjustment_new
							   (mt, 0, config_max,
							    1, 10, 10)),
					   1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_top),
			  0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_bottom = GTK_SPIN_BUTTON (gtk_spin_button_new
					     (GTK_ADJUSTMENT (gtk_adjustment_new
							      (mb, 0, config_max,
							       1, 10, 10)),
					      1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_bottom),
			   0, 1, 7, 8,  GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_left = GTK_SPIN_BUTTON (gtk_spin_button_new
					   (GTK_ADJUSTMENT (gtk_adjustment_new
							    (ml, 0, config_max,
							     1, 10, 10)),
					    1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_left),
			  0, 1, 3, 4, GTK_FILL, GTK_FILL, 0, 0);
	ps->margin_right = GTK_SPIN_BUTTON (gtk_spin_button_new
					    (GTK_ADJUSTMENT (gtk_adjustment_new
							     (mr, 0, config_max,
							      1, 10, 10)),
					     1, 2));
	gtk_table_attach (GTK_TABLE (margin_table), GTK_WIDGET (ps->margin_right),
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

	gtk_container_add (GTK_CONTAINER (ps->margin_frame), margin_table);
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_top));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_bottom));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_left));
	gnome_print_unit_selector_add_adjustment (GNOME_PRINT_UNIT_SELECTOR (ps->us),
						  gtk_spin_button_get_adjustment (ps->margin_right));
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_top)),
			  "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_bottom)),
			  "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_left)),
			  "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (gtk_spin_button_get_adjustment (ps->margin_right)),
			  "value_changed", G_CALLBACK (gps_m_size_value_changed), ps);
	g_signal_connect (G_OBJECT (ps->margin_top), "focus_in_event",
			  G_CALLBACK (lmargin_top_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_top), "focus_out_event",
			  G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_left), "focus_in_event",
			  G_CALLBACK (lmargin_left_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_left), "focus_out_event",
			  G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_right), "focus_in_event",
			  G_CALLBACK (lmargin_right_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_right), "focus_out_event",
			  G_CALLBACK (lmargin_unit_deactivated), ps);
	g_signal_connect (G_OBJECT (ps->margin_bottom), "focus_in_event",
			  G_CALLBACK (lmargin_bottom_unit_activated), ps);
	g_signal_connect (G_OBJECT (ps->margin_bottom), "focus_out_event",
			  G_CALLBACK (lmargin_unit_deactivated), ps);

	if (ps->flags & GNOME_PAPER_SELECTOR_MARGINS)
		gtk_widget_show_all (ps->margin_frame);

	/* Hook up the Custom size widgets */
	gnome_paper_selector_hook_paper_size (ps);
	ps->handler_printer = g_signal_connect (G_OBJECT (ps->printer), "modified",
						(GCallback) gnome_paper_selector_printer_changed_cb, ps);
	
	/* Setup the unit selector */
	gnome_paper_unit_selector_request_update_cb (NULL, 0, ps);
	g_signal_connect (G_OBJECT (ps->us), "modified",
			  G_CALLBACK (gnome_paper_selector_unit_changed_cb), ps);
	ps->handler_unit = g_signal_connect (
		G_OBJECT (gnome_print_config_get_node (ps->config)), "modified",
		G_CALLBACK (gnome_paper_unit_selector_request_update_cb), ps);

	gtk_widget_show (ps->us);
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

