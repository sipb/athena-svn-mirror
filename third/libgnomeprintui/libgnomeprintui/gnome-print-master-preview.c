/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-master-preview.c: print preview window
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
 *    Michael Zucchi <notzed@helixcode.com>
 *    Miguel de Icaza <miguel@gnu.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 */

#define __GNOME_PRINT_MASTER_PREVIEW_C__
#define noGPMP_VERBOSE

#include <config.h>

#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <atk/atkobject.h>
#include <atk/atkrelationset.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkmenubar.h>
#include <libgnomeprint/private/gnome-print-private.h>

#include "gnome-print-i18n.h"
#include "gnome-print-master-preview.h"

#define GPMP_ZOOM_IN_FACTOR M_SQRT2
#define GPMP_ZOOM_OUT_FACTOR M_SQRT1_2
#define GPMP_ZOOM_MIN 0.0625
#define GPMP_ZOOM_MAX 16.0

#define GPMP_A4_WIDTH (210.0 * 72.0 / 25.4)
#define GPMP_A4_HEIGHT (297.0 * 72.0 / 2.54)

#define TOOLBAR_BUTTON_BASE 5
#define MOVE_INDEX 5

/* These are the callback action identifers for the GtkItemFactory used for
   the menubar. */
enum {
	PREVIEW_PRINT_ACTION,
	PREVIEW_CLOSE_ACTION,

	PREVIEW_GOTO_FIRST_ACTION,
	PREVIEW_GO_BACK_ACTION,
	PREVIEW_GO_FORWARD_ACTION,
	PREVIEW_GOTO_LAST_ACTION,

	PREVIEW_ZOOM_100_ACTION,
	PREVIEW_ZOOM_FIT_ACTION,
	PREVIEW_ZOOM_IN_ACTION,
	PREVIEW_ZOOM_OUT_ACTION
};

struct _GnomePrintMasterPreview {
	GtkWindow window;
	/* Main VBox */
	GtkWidget *vbox;
	/* Some interesting buttons */
	GtkWidget *bpf, *bpp, *bpn, *bpl;
	GtkWidget *bz1, *bzf, *bzi, *bzo;
	/* Number of pages master has */
	gint pages;
	/* Zoom factor */
	gdouble zoom;

	/* Physical area dimensions */
	gdouble paw, pah;
	/* Calculated Physical Area -> Layout */
	gdouble pa2ly[6];

	/* State */
	guint dragging : 1;
	gint anchorx, anchory;
	gint offsetx, offsety;

	gpointer priv;
};

struct _GnomePrintMasterPreviewClass {
	GtkWindowClass parent_class;
};

typedef struct _GPMPPrivate GPMPPrivate;

struct _GPMPPrivate {
	/* Our GnomePrintMaster */
	GnomePrintMaster *master;
	/* Our GnomePrintPreview */
	GnomePrintContext *preview;

	GtkWidget *page_entry;
	GtkWidget *scrolled_window;
	GtkWidget *last;
	GnomeCanvas *canvas;

	gint current_page;
	gint pagecount;

	GtkWidget *goto_first_menuitem;
	GtkWidget *go_back_menuitem;
	GtkWidget *go_forward_menuitem;
	GtkWidget *goto_last_menuitem;

	GtkWidget *zoom_100_menuitem;
	GtkWidget *zoom_fit_menuitem;
	GtkWidget *zoom_in_menuitem;
	GtkWidget *zoom_out_menuitem;
};

static void gpmp_parse_layout (GnomePrintMasterPreview *mp);

/*
 * Padding in points around the simulated page
 */

#define PAGE_PAD 4

static gint
render_page (GnomePrintMasterPreview *mp, gint page)
{
	GPMPPrivate *priv;
	GnomePrintConfig *config;
	ArtDRect region;
	gdouble transform[6];

	priv = (GPMPPrivate *) mp->priv;

	if (priv->preview) {
		g_object_unref (G_OBJECT (priv->preview));
		priv->preview = NULL;
	}

	/* Set page transformation */
	transform[0] =  1.0;
	transform[1] =  0.0;
	transform[2] =  0.0;
	transform[3] = -1.0;
	transform[4] =  0.0;
	transform[5] =  mp->pah;
	art_affine_multiply (transform, mp->pa2ly, transform);

	/* Reset scrolling region always */
	region.x0 = region.y0 = 0.0 - PAGE_PAD;
	region.x1 = mp->paw + PAGE_PAD;
	region.y1 = mp->pah + PAGE_PAD;

	config = gnome_print_master_get_config (priv->master);
	priv->preview = gnome_print_preview_new_full (config, priv->canvas, transform, &region);
	gnome_print_config_unref (config);

	gnome_print_master_render_page (priv->master, priv->preview, page, TRUE);

	return GNOME_PRINT_OK;
}

static gint
goto_page (GnomePrintMasterPreview *mp, gint page)
{
	GPMPPrivate *priv;
	guchar c[32];
	gboolean can_go_back = FALSE, can_go_forward = FALSE;

	priv = (GPMPPrivate *) mp->priv;

	g_snprintf (c, 32, "%d", page + 1);
	gtk_entry_set_text (GTK_ENTRY (priv->page_entry), c);

	if ((page != 0) && priv->pagecount > 1)
		can_go_back = TRUE;
	if ((page != (mp->pages - 1)) && priv->pagecount > 1)
		can_go_forward = TRUE;

	gtk_widget_set_sensitive (mp->bpf, can_go_back);
	gtk_widget_set_sensitive (mp->bpp, can_go_back);
	gtk_widget_set_sensitive (priv->goto_first_menuitem, can_go_back);
	gtk_widget_set_sensitive (priv->go_back_menuitem, can_go_back);

	gtk_widget_set_sensitive (mp->bpn, can_go_forward);
	gtk_widget_set_sensitive (mp->bpl, can_go_forward);
	gtk_widget_set_sensitive (priv->go_forward_menuitem, can_go_forward);
	gtk_widget_set_sensitive (priv->goto_last_menuitem, can_go_forward);

	if (page != priv->current_page) {
		priv->current_page = page;
		return render_page (mp, page);
	}

	return GNOME_PRINT_OK;
}

static gint
change_page_cmd (GtkEntry *entry, GnomePrintMasterPreview *pmp)
{
	GPMPPrivate *priv;
	const gchar *text;
	gint page;

	priv = (GPMPPrivate *) pmp->priv;

	text = gtk_entry_get_text (entry);

	page = CLAMP (atoi (text), 1, priv->pagecount) - 1;

	return goto_page (pmp, page);
}

#define CLOSE_ENOUGH(a,b) (fabs (a - b) < 1e-6)

static void
gpmp_zoom (GnomePrintMasterPreview *mp, gdouble factor, gboolean relative)
{
	GPMPPrivate *priv;
	gdouble zoom;
	gboolean can_zoom_100 = TRUE, can_zoom_in = TRUE, can_zoom_out = TRUE;

	priv = (GPMPPrivate *) mp->priv;

	if (relative) {
		zoom = mp->zoom * factor;
	} else {
		zoom = factor;
	}

	mp->zoom = CLAMP (zoom, GPMP_ZOOM_MIN, GPMP_ZOOM_MAX);

	if (CLOSE_ENOUGH (mp->zoom, 1.0))
		can_zoom_100 = FALSE;
	if (CLOSE_ENOUGH (mp->zoom, GPMP_ZOOM_MAX))
		can_zoom_in = FALSE;
	if (CLOSE_ENOUGH (mp->zoom, GPMP_ZOOM_MIN))
		can_zoom_out = FALSE;

	gtk_widget_set_sensitive (mp->bz1, can_zoom_100);
	gtk_widget_set_sensitive (mp->bzi, can_zoom_in);
	gtk_widget_set_sensitive (mp->bzo, can_zoom_out);

	gtk_widget_set_sensitive (priv->zoom_100_menuitem, can_zoom_100);
	gtk_widget_set_sensitive (priv->zoom_in_menuitem, can_zoom_in);
	gtk_widget_set_sensitive (priv->zoom_out_menuitem, can_zoom_out);

	gnome_canvas_set_pixels_per_unit (priv->canvas, mp->zoom);
}

/* Button press handler for the print preview canvas */

static gint
preview_canvas_button_press (GtkWidget *widget, GdkEventButton *event, GnomePrintMasterPreview *mp)
{
	gint retval;

	retval = FALSE;

	if (event->button == 1) {
		GdkCursor *cursor;

		mp->dragging = TRUE;

		gnome_canvas_get_scroll_offsets (GNOME_CANVAS (widget), &mp->offsetx, &mp->offsety);

		mp->anchorx = event->x - mp->offsetx;
		mp->anchory = event->y - mp->offsety;

		cursor = gdk_cursor_new (GDK_FLEUR);
		gdk_pointer_grab (widget->window, FALSE,
				  (GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_RELEASE_MASK),
				  NULL, cursor, event->time);
		gdk_cursor_unref (cursor);

		retval = TRUE;
	}

	return retval;
}

/* Motion notify handler for the print preview canvas */

static gint
preview_canvas_motion (GtkWidget *widget, GdkEventMotion *event, GnomePrintMasterPreview *mp)
{
	GdkModifierType mod;
	gint retval;

	retval = FALSE;

	if (mp->dragging) {
		gint x, y, dx, dy;

		if (event->is_hint) {
			gdk_window_get_pointer (widget->window, &x, &y, &mod);
		} else {
			x = event->x;
			y = event->y;
		}

		dx = mp->anchorx - x;
		dy = mp->anchory - y;

		gnome_canvas_scroll_to (((GPMPPrivate *) mp->priv)->canvas, mp->offsetx + dx, mp->offsety + dy);

		/* Get new anchor and offset */
		mp->anchorx = event->x;
		mp->anchory = event->y;
		gnome_canvas_get_scroll_offsets (GNOME_CANVAS (widget), &mp->offsetx, &mp->offsety);

		retval = TRUE;
	}

	return retval;
}

/* Button release handler for the print preview canvas */

static gint
preview_canvas_button_release (GtkWidget *widget, GdkEventButton *event, GnomePrintMasterPreview *mp)
{
	gint retval;

	retval = TRUE;

	if (event->button == 1) {
		mp->dragging = FALSE;
		gdk_pointer_ungrab (event->time);
		retval = TRUE;
	}

	return retval;
}


static void
preview_close_cmd (gpointer unused, GnomePrintMasterPreview *mp)
{
	gtk_widget_destroy (GTK_WIDGET (mp));
}

static gboolean
preview_delete_event (gpointer unused, GdkEventAny *event, GnomePrintMasterPreview *mp)
{
	gtk_widget_destroy (GTK_WIDGET (mp));

	return TRUE;
}

static void
preview_file_print_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	GPMPPrivate *priv;

	priv = (GPMPPrivate *) pmp->priv;

	gnome_print_master_print (priv->master);

	/* fixme: should we clean ourselves up now? */
}

static void
preview_first_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	goto_page (pmp, 0);
}

static void
preview_next_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	GPMPPrivate *priv;

	priv = (GPMPPrivate *) pmp->priv;

	if (priv->current_page < (priv->pagecount - 1)) {
		goto_page (pmp, priv->current_page + 1);
	}
}

static void
preview_prev_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	GPMPPrivate *priv;

	priv = (GPMPPrivate *) pmp->priv;

	if (priv->current_page > 0) {
		goto_page (pmp, priv->current_page - 1);
	}
}

static void
preview_last_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	GPMPPrivate *priv;

	priv = (GPMPPrivate *) pmp->priv;

	goto_page (pmp, priv->pagecount - 1);
}

static void
gpmp_zoom_in_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	gpmp_zoom (pmp, GPMP_ZOOM_IN_FACTOR, TRUE);
}

static void
gpmp_zoom_out_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	gpmp_zoom (pmp, GPMP_ZOOM_OUT_FACTOR, TRUE);
}

static void
preview_zoom_fit_cmd (GtkToggleButton *t, GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;
	gdouble zoomx, zoomy;
	gint width, height;

	priv = (GPMPPrivate *) mp->priv;

	width = GTK_WIDGET (priv->canvas)->allocation.width;
	height = GTK_WIDGET (priv->canvas)->allocation.height;

	zoomx = width / (mp->paw + 5.0 + PAGE_PAD);
	zoomy = height / (mp->pah + 5.0 + PAGE_PAD);

	gpmp_zoom (mp, MIN (zoomx, zoomy), FALSE);
}

static void
preview_zoom_100_cmd (GtkToggleButton *t, GnomePrintMasterPreview *mp)
{
	gpmp_zoom (mp, 1.0, FALSE);
}

static gint
preview_canvas_key (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GnomePrintMasterPreview *pmp;
	GPMPPrivate *priv;
	gint x,y;
	gint height, width;
	gint domove = 0;

	pmp = (GnomePrintMasterPreview *) data;

	priv = (GPMPPrivate *) pmp->priv;

	gnome_canvas_get_scroll_offsets (priv->canvas, &x, &y);
	height = GTK_WIDGET (priv->canvas)->allocation.height;
	width = GTK_WIDGET (priv->canvas)->allocation.width;

	switch (event->keyval) {
	case '1':
		preview_zoom_100_cmd (0, pmp);
		break;
	case '+':
	case '=':
	case GDK_KP_Add:
		gpmp_zoom_in_cmd (NULL, pmp);
		break;
	case '-':
	case '_':
	case GDK_KP_Subtract:
		gpmp_zoom_out_cmd (NULL, pmp);
		break;
	case GDK_KP_Right:
	case GDK_Right:
		if (event->state & GDK_SHIFT_MASK)
			x += width;
		else
			x += 10;
		domove = 1;
		break;
	case GDK_KP_Left:
	case GDK_Left:
		if (event->state & GDK_SHIFT_MASK)
			x -= width;
		else
			x -= 10;
		domove = 1;
		break;
	case GDK_KP_Up:
	case GDK_Up:
		if (event->state & GDK_SHIFT_MASK)
			goto page_up;
		y -= 10;
		domove = 1;
		break;
	case GDK_KP_Down:
	case GDK_Down:
		if (event->state & GDK_SHIFT_MASK)
			goto page_down;
		y += 10;
		domove = 1;
		break;
	case GDK_KP_Page_Up:
	case GDK_Page_Up:
	case GDK_Delete:
	case GDK_KP_Delete:
	case GDK_BackSpace:
	page_up:
		if (y <= 0) {
			if (priv->current_page > 0) {
				goto_page (pmp, priv->current_page - 1);
				y = GTK_LAYOUT (priv->canvas)->height - height;
			}
		} else {
			y -= height;
		}
		domove = 1;
		break;
	case GDK_KP_Page_Down:
	case GDK_Page_Down:
	case ' ':
	page_down:
		if (y >= GTK_LAYOUT (priv->canvas)->height - height) {
			if (priv->current_page < priv->pagecount - 1) {
				goto_page (pmp, priv->current_page + 1);
				y = 0;
			}
		} else {
			y += height;
		}
		domove = 1;
		break;
	case GDK_KP_Home:
	case GDK_Home:
		goto_page (pmp, 0);
		y = 0;
		domove = 1;
		break;
	case GDK_KP_End:
	case GDK_End:
		goto_page (pmp, priv->pagecount - 1);
		y = 0;
		domove = 1;
		break;
	case GDK_Escape:
		gtk_widget_destroy (GTK_WIDGET (pmp));
		return TRUE;
	default:
		return FALSE;
	}

	if (domove)
		gnome_canvas_scroll_to (priv->canvas, x, y);

	gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");
	return TRUE;
}

static void
create_preview_canvas (GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;
	GnomeCanvasItem *item;
	GnomePrintConfig *config;
	AtkObject *atko;

	priv = (GPMPPrivate *) mp->priv;

	priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->scrolled_window), GTK_SHADOW_IN);

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	priv->canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
	gtk_widget_pop_colormap ();
	atko = gtk_widget_get_accessible (GTK_WIDGET (priv->canvas));
	atk_object_set_name (atko, _("Page Preview"));
	atk_object_set_description (atko, _("The preview of a page in the document to be printed"));

	gtk_signal_connect (GTK_OBJECT (priv->canvas), "button_press_event",
			    GTK_SIGNAL_FUNC (preview_canvas_button_press), mp);
	gtk_signal_connect (GTK_OBJECT (priv->canvas), "button_release_event",
			    GTK_SIGNAL_FUNC (preview_canvas_button_release), mp);
	gtk_signal_connect (GTK_OBJECT (priv->canvas), "motion_notify_event",
			    GTK_SIGNAL_FUNC (preview_canvas_motion), mp);
	gtk_signal_connect (GTK_OBJECT (priv->canvas), "key_press_event",
			    GTK_SIGNAL_FUNC (preview_canvas_key), mp);

	gtk_container_add (GTK_CONTAINER (priv->scrolled_window), GTK_WIDGET (priv->canvas));

	config = gnome_print_master_get_config (priv->master);
	priv->preview = gnome_print_preview_new (config, priv->canvas);
	gnome_print_config_unref (config);

	/*
	 * Now add some padding above and below and put a simulated
	 * page on the background
	 */

	item = gnome_canvas_item_new (gnome_canvas_root (priv->canvas),
				      GNOME_TYPE_CANVAS_RECT,
				      "x1", 0.0, "y1", 0.0, "x2", (gdouble) mp->paw, "y2", (gdouble) mp->pah,
				      "fill_color", "white",
				      "outline_color", "black",
				      "width_pixels", 1, NULL);
	gnome_canvas_item_lower_to_bottom (item);
	item = gnome_canvas_item_new (gnome_canvas_root (priv->canvas),
				      GNOME_TYPE_CANVAS_RECT,
				      "x1", 3.0, "y1", 3.0, "x2", (gdouble) mp->paw + 3, "y2", (gdouble) mp->pah + 3,
				      "fill_color", "black", NULL);
	gnome_canvas_item_lower_to_bottom (item);
	gnome_canvas_set_scroll_region (priv->canvas, 0 - PAGE_PAD, 0 - PAGE_PAD, mp->paw + PAGE_PAD, mp->pah + PAGE_PAD);

	gtk_box_pack_start (GTK_BOX (mp->vbox), priv->scrolled_window, TRUE, TRUE, 0);

	gtk_widget_show_all (mp->vbox);

	gtk_widget_grab_focus (GTK_WIDGET (priv->canvas));
}

static GtkWidget*
create_page_number_field (GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;
	GtkWidget *status, *l, *pad;
	AtkObject *atko;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];

	priv = (GPMPPrivate *) mp->priv;

	status = gtk_hbox_new (FALSE, 0);
	l = gtk_label_new_with_mnemonic (_ ("P_age: "));
	gtk_box_pack_start (GTK_BOX (status), l, FALSE, FALSE, 4);
	priv->page_entry = gtk_entry_new ();
	gtk_widget_set_usize (priv->page_entry, 40, 0);
	gtk_signal_connect (GTK_OBJECT (priv->page_entry), "activate", GTK_SIGNAL_FUNC (change_page_cmd), mp);
	gtk_box_pack_start (GTK_BOX (status), priv->page_entry, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l, priv->page_entry);

	/* We are displaying 'Page: XXX of XXX'. */
	gtk_box_pack_start (GTK_BOX (status), gtk_label_new (_("of")),
			    FALSE, FALSE, 8);

	priv->last = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (status), priv->last, FALSE, FALSE, 0);
	atko = gtk_widget_get_accessible (priv->last);
	atk_object_set_name (atko, _("Page total"));
	atk_object_set_description (atko, _("The total number of pages in the document"));

	pad = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status), pad, FALSE, FALSE, 4);

	/* Add a LABELLED_BY relation from the page entry to the label. */
	atko = gtk_widget_get_accessible (priv->page_entry);
	relation_set = atk_object_ref_relation_set (atko);
	relation_targets[0] = gtk_widget_get_accessible (l);
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));

	gtk_widget_show_all (status);

	return status;
}

#ifdef GPMP_VERBOSE
#define PRINT_2(s,a,b) g_print ("GPMP %s %g %g\n", s, (a), (b))
#define PRINT_DRECT(s,a) g_print ("GPMP %s %g %g %g %g\n", (s), (a)->x0, (a)->y0, (a)->x1, (a)->y1)
#define PRINT_AFFINE(s,a) g_print ("GPMP %s %g %g %g %g %g %g\n", (s), *(a), *((a) + 1), *((a) + 2), *((a) + 3), *((a) + 4), *((a) + 5))
#else
#define PRINT_2(s,a,b)
#define PRINT_DRECT(s,a)
#define PRINT_AFFINE(s,a)
#endif

/* This is the GtkItemFactory callback used for all the menubar items.
   We just need to call the existing toolbar button callbacks and rearrange
   the arguments a little. */
static void
item_factory_cb (GnomePrintMasterPreview *pmp, guint callback_action,
		 GtkWidget *widget)
{
	switch (callback_action) {
	case PREVIEW_PRINT_ACTION:
		preview_file_print_cmd (NULL, pmp);
		break;
	case PREVIEW_CLOSE_ACTION:
		preview_close_cmd (NULL, pmp);
		break;
	case PREVIEW_GOTO_FIRST_ACTION:
		preview_first_page_cmd (NULL, pmp);
		break;
	case PREVIEW_GO_BACK_ACTION:
		preview_prev_page_cmd (NULL, pmp);
		break;
	case PREVIEW_GO_FORWARD_ACTION:
		preview_next_page_cmd (NULL, pmp);
		break;
	case PREVIEW_GOTO_LAST_ACTION:
		preview_last_page_cmd (NULL, pmp);
		break;
	case PREVIEW_ZOOM_100_ACTION:
		preview_zoom_100_cmd (NULL, pmp);
		break;
	case PREVIEW_ZOOM_FIT_ACTION:
		preview_zoom_fit_cmd (NULL, pmp);
		break;
	case PREVIEW_ZOOM_IN_ACTION:
		gpmp_zoom_in_cmd (NULL, pmp);
		break;
	case PREVIEW_ZOOM_OUT_ACTION:
		gpmp_zoom_out_cmd (NULL, pmp);
		break;
	default:
		g_assert_not_reached ();
	}
}

static GtkItemFactoryEntry menu_items[] =
{
  { N_("/_Preview"),			NULL,
    0,					0, "<Branch>" },
  { N_("/Preview/_Print"),		NULL,
    item_factory_cb,			PREVIEW_PRINT_ACTION,
    "<StockItem>",			GTK_STOCK_PRINT },
  { N_("/Preview/_Close"),		NULL,
    item_factory_cb,			PREVIEW_CLOSE_ACTION,
    "<StockItem>",			GTK_STOCK_CLOSE },

  { N_("/_View"),			NULL,
    0,					0, "<Branch>" },
  { N_("/View/Zoom _1:1"),		"<Ctrl>1",
    item_factory_cb,			PREVIEW_ZOOM_100_ACTION,
    "<StockItem>",			GTK_STOCK_ZOOM_100 },
  { N_("/View/Zoom to _Fit"),		"<Ctrl>F",
    item_factory_cb,			PREVIEW_ZOOM_FIT_ACTION,
    "<StockItem>",			GTK_STOCK_ZOOM_FIT },
  { N_("/View/Zoom _In"),		"<Ctrl>plus",
    item_factory_cb,			PREVIEW_ZOOM_IN_ACTION,
    "<StockItem>",			GTK_STOCK_ZOOM_IN },
  { N_("/View/Zoom _Out"),		"<Ctrl>minus",
    item_factory_cb,			PREVIEW_ZOOM_OUT_ACTION,
    "<StockItem>",			GTK_STOCK_ZOOM_OUT },

  { N_("/_Go"),				NULL,
    0,					0, "<Branch>" },
  { N_("/Go/_First Page"),		"Home",
    item_factory_cb,			PREVIEW_GOTO_FIRST_ACTION,
    "<StockItem>",			GTK_STOCK_GOTO_FIRST },
  { N_("/Go/_Previous Page"),		"<Alt>Left",
    item_factory_cb,			PREVIEW_GO_BACK_ACTION,
    "<StockItem>",			GTK_STOCK_GO_BACK },
  { N_("/Go/_Next Page"),		"<Alt>Right",
    item_factory_cb,			PREVIEW_GO_FORWARD_ACTION,
    "<StockItem>",			GTK_STOCK_GO_FORWARD },
  { N_("/Go/_Last Page"),		"End",
    item_factory_cb,			PREVIEW_GOTO_LAST_ACTION,
    "<StockItem>",			GTK_STOCK_GOTO_LAST },
};

static void
create_toplevel (GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;
	GtkWidget *tb;
	gint width, height;
	GtkItemFactory *item_factory;
	GtkWidget *menubar;
	GtkAccelGroup *accel_group;
	GtkWidget *page_number_field;

	priv = (GPMPPrivate *) mp->priv;

	width  = MIN (mp->paw + PAGE_PAD * 3, gdk_screen_width () - 40);
	height = MIN (mp->pah + PAGE_PAD * 3, gdk_screen_height () - 40);

	gtk_window_set_policy (GTK_WINDOW (mp), TRUE, TRUE, FALSE);
	gtk_widget_set_usize (GTK_WIDGET (mp), width, height);

	/* Create the menubar items, using the factory. */
	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (mp), accel_group);
	g_object_unref (accel_group);
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
					     accel_group);
	gtk_item_factory_set_translate_func (item_factory, 
					     (GtkTranslateFunc) libgnomeprintui_gettext,
					     NULL, NULL);
	gtk_item_factory_create_items (item_factory, G_N_ELEMENTS (menu_items),
				       menu_items, mp);
	menubar = gtk_item_factory_get_widget (item_factory, "<main>");
	gtk_widget_show (menubar);
	gtk_box_pack_start (GTK_BOX (mp->vbox), menubar, FALSE, FALSE, 0);

	priv->goto_first_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_GOTO_FIRST_ACTION);
	priv->go_back_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_GO_BACK_ACTION);
	priv->go_forward_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_GO_FORWARD_ACTION);
	priv->goto_last_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_GOTO_LAST_ACTION);

	priv->zoom_100_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_ZOOM_100_ACTION);
	priv->zoom_fit_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_ZOOM_FIT_ACTION);
	priv->zoom_in_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_ZOOM_IN_ACTION);
	priv->zoom_out_menuitem = gtk_item_factory_get_item_by_action
		(item_factory, PREVIEW_ZOOM_OUT_ACTION);

	/* This is not very beautiful, but works for now */

	tb = gtk_toolbar_new ();
	gtk_toolbar_set_style (GTK_TOOLBAR (tb), GTK_TOOLBAR_ICONS);
	gtk_widget_show (tb);
	gtk_box_pack_start (GTK_BOX (mp->vbox), tb, FALSE, FALSE, 0);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_PRINT,
				  _("Prints the current file"), "", 
				  GTK_SIGNAL_FUNC (preview_file_print_cmd), mp, -1);

	gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_CLOSE,
				  _("Closes print preview window"), "", 
				  GTK_SIGNAL_FUNC (preview_close_cmd), mp, -1);

	gtk_toolbar_append_space (GTK_TOOLBAR (tb));

	page_number_field = create_page_number_field (mp);
	gtk_toolbar_append_widget (GTK_TOOLBAR (tb), page_number_field,
				   NULL, NULL);

	gtk_toolbar_append_space (GTK_TOOLBAR (tb));

	mp->bpf = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_GOTO_FIRST, _("Shows the first page"), "", 
					    GTK_SIGNAL_FUNC (preview_first_page_cmd), mp, -1);
	mp->bpp = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_GO_BACK, _("Shows the previous page"), "", 
					    GTK_SIGNAL_FUNC (preview_prev_page_cmd), mp, -1);
	mp->bpn = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_GO_FORWARD, _("Shows the next page"), "", 
					    GTK_SIGNAL_FUNC (preview_next_page_cmd), mp, -1);
	mp->bpl = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_GOTO_LAST, _("Shows the last page"), "", 
					    GTK_SIGNAL_FUNC (preview_last_page_cmd), mp, -1);

	gtk_toolbar_append_space (GTK_TOOLBAR (tb));

	mp->bz1 = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_ZOOM_100, _("Zooms 1:1"), "", 
					    GTK_SIGNAL_FUNC (preview_zoom_100_cmd), mp, -1);
	mp->bzf = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_ZOOM_FIT, _("Zooms to fit the whole page"), "", 
					    GTK_SIGNAL_FUNC (preview_zoom_fit_cmd), mp, -1);
	mp->bzi = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_ZOOM_IN, _("Zooms the page in"), "", 
					    GTK_SIGNAL_FUNC (gpmp_zoom_in_cmd), mp, -1);
	mp->bzo = gtk_toolbar_insert_stock (GTK_TOOLBAR (tb), GTK_STOCK_ZOOM_OUT, _("Zooms the page out"), "", 
					    GTK_SIGNAL_FUNC (gpmp_zoom_out_cmd), mp, -1);
}

/*
  The GnomePrintMasterPreview object
*/

static void gnome_print_master_preview_class_init (GnomePrintMasterPreviewClass *klass);
static void gnome_print_master_preview_init (GnomePrintMasterPreview *pmp);

static GtkWindowClass *parent_class;

GtkType
gnome_print_master_preview_get_type (void)
{
	static GtkType print_master_preview_type = 0;

	if (!print_master_preview_type) {
		GtkTypeInfo print_master_preview_info = {
			"GnomePrintMasterPreview",
			sizeof (GnomePrintMasterPreview),
			sizeof (GnomePrintMasterPreviewClass),
			(GtkClassInitFunc) gnome_print_master_preview_class_init,
			(GtkObjectInitFunc) gnome_print_master_preview_init,
			NULL, NULL, NULL
		};

		print_master_preview_type = gtk_type_unique (GTK_TYPE_WINDOW, &print_master_preview_info);
	}

	return print_master_preview_type;
}

static void
gnome_print_master_preview_destroy (GtkObject *object)
{
	GnomePrintMasterPreview *pmp = GNOME_PRINT_MASTER_PREVIEW (object);
	GPMPPrivate *priv = pmp->priv;

	if (priv->preview != NULL) {
		g_object_unref (G_OBJECT (priv->preview));
		priv->preview = NULL;
	}

	if (priv->master != NULL) {
		g_object_unref (G_OBJECT (priv->master));
		priv->master = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnome_print_master_preview_finalize (GObject *object)
{
	GnomePrintMasterPreview *pmp;
	GPMPPrivate *priv;

	pmp = GNOME_PRINT_MASTER_PREVIEW (object);
	priv = pmp->priv;

	if (priv) {
		g_free (priv);
		pmp->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_print_master_preview_class_init (GnomePrintMasterPreviewClass *klass)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_WINDOW);

	object_class->destroy = gnome_print_master_preview_destroy;
	G_OBJECT_CLASS (object_class)->finalize = gnome_print_master_preview_finalize;
}

static void
gnome_print_master_preview_init (GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;

	mp->priv = priv = g_new0 (GPMPPrivate, 1);
	priv->current_page = -1;

	/* Main VBox */
	mp->vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mp->vbox);
	gtk_container_add (GTK_CONTAINER (mp), mp->vbox);

	mp->zoom = 1.0;
}

GtkWidget *
gnome_print_master_preview_new (GnomePrintMaster *gpm, const guchar *title)
{
	GnomePrintMasterPreview *gpmp;
	GPMPPrivate *priv;
	gchar *text;

	g_return_val_if_fail (gpm != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), NULL);

	if (!title) title = _("Gnome Print Preview");

	gpmp = gtk_type_new (GNOME_TYPE_PRINT_MASTER_PREVIEW);
	gtk_signal_connect (GTK_OBJECT (gpmp), "delete_event", GTK_SIGNAL_FUNC (preview_delete_event), gpmp);
	gtk_window_set_title (GTK_WINDOW (gpmp), title);

	priv = (GPMPPrivate *) gpmp->priv;

	priv->master = gpm;
	g_object_ref (G_OBJECT (gpm));

	gpmp_parse_layout (gpmp);
	create_toplevel (gpmp);
	create_preview_canvas (gpmp);

	/* this zooms to fit, once we know how big the window actually is */
	gtk_signal_connect (GTK_OBJECT (priv->canvas), "realize",
			    GTK_SIGNAL_FUNC (preview_zoom_fit_cmd), gpmp);

	priv->pagecount = gpmp->pages = gnome_print_master_get_pages (gpm);
	goto_page (gpmp, 0);
	
	if (priv->pagecount == 0) {
		priv->pagecount = 1;
	}
	text = g_strdup_printf ("%d", priv->pagecount);
	gtk_label_set_text (GTK_LABEL (priv->last), text);
	g_free (text);

	return (GtkWidget *) gpmp;
}

static void
gpmp_parse_layout (GnomePrintMasterPreview *mp)
{
	GPMPPrivate *priv;
	GnomePrintConfig *config;
	GnomePrintLayoutData *lyd;

	priv = (GPMPPrivate *) mp->priv;

	/* Calculate layout-compensated page dimensions */
	mp->paw = GPMP_A4_WIDTH;
	mp->pah = GPMP_A4_HEIGHT;
	art_affine_identity (mp->pa2ly);
	config = gnome_print_master_get_config (priv->master);
	lyd = gnome_print_config_get_layout_data (config, NULL, NULL, NULL, NULL);
	gnome_print_config_unref (config);
	if (lyd) {
		GnomePrintLayout *ly;
		ly = gnome_print_layout_new_from_data (lyd);
		if (ly) {
			gdouble pp2lyI[6], pa2pp[6];
			gdouble expansion;
			ArtDRect pp, ap, tp;
			/* Find paper -> layout transformation */
			art_affine_invert (pp2lyI, ly->LYP[0].matrix);
			PRINT_AFFINE ("pp2ly:", &pp2lyI[0]);
			/* Find out, what the page dimensions should be */
			expansion = art_affine_expansion (pp2lyI);
			if (expansion > 1e-6) {
				/* Normalize */
				pp2lyI[0] /= expansion;
				pp2lyI[1] /= expansion;
				pp2lyI[2] /= expansion;
				pp2lyI[3] /= expansion;
				pp2lyI[4] = 0.0;
				pp2lyI[5] = 0.0;
				PRINT_AFFINE ("pp2lyI:", &pp2lyI[0]);
				/* Find page dimensions relative to layout */
				pp.x0 = 0.0;
				pp.y0 = 0.0;
				pp.x1 = lyd->pw;
				pp.y1 = lyd->ph;
				art_drect_affine_transform (&tp, &pp, pp2lyI);
				/* Compensate with expansion */
				mp->paw = tp.x1 - tp.x0;
				mp->pah = tp.y1 - tp.y0;
				PRINT_2 ("Width & Height", mp->paw, mp->pah);
			}
			/* Now compensate with feed orientation */
			art_affine_invert (pa2pp, ly->PP2PA);
			PRINT_AFFINE ("pa2pp:", &pa2pp[0]);
			art_affine_multiply (mp->pa2ly, pa2pp, pp2lyI);
			PRINT_AFFINE ("pa2ly:", &mp->pa2ly[0]);
			/* Finally we need translation factors */
			/* Page box in normalized layout */
			pp.x0 = 0.0;
			pp.y0 = 0.0;
			pp.x1 = lyd->pw;
			pp.y1 = lyd->ph;
			art_drect_affine_transform (&ap, &pp, ly->PP2PA);
			art_drect_affine_transform (&tp, &ap, mp->pa2ly);
			PRINT_DRECT ("RRR:", &tp);
			mp->pa2ly[4] -= tp.x0;
			mp->pa2ly[5] -= tp.y0;
			PRINT_AFFINE ("pa2ly:", &mp->pa2ly[0]);
			/* Now, if master does PA2LY LY2PA concat it ends with scaled identity */
			gnome_print_layout_free (ly);
		}
		gnome_print_layout_data_free (lyd);
	}
}
