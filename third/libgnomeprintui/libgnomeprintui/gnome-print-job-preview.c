/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-job-preview.c: print preview window
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

#include <config.h>

#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <atk/atk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libgnomeprint/private/gnome-print-private.h>

#include "gnome-print-preview-private.h"
#include "gnome-print-i18n.h"
#include "gnome-print-job-preview.h"

#define GPMP_ZOOM_IN_FACTOR M_SQRT2
#define GPMP_ZOOM_OUT_FACTOR M_SQRT1_2
#define GPMP_ZOOM_MIN 0.0625
#define GPMP_ZOOM_MAX 16.0

#define GPMP_A4_WIDTH (210.0 * 72.0 / 25.4)
#define GPMP_A4_HEIGHT (297.0 * 72.0 / 2.54)

#define TOOLBAR_BUTTON_BASE 5
#define MOVE_INDEX 5

struct _GnomePrintJobPreview {
	GtkWindow window;
	/* Main VBox */
	GtkWidget *vbox;
	/* Some interesting buttons */
	GtkWidget *bpf, *bpp, *bpn, *bpl;
	GtkWidget *bz1, *bzf, *bzi, *bzo;
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

	/* Number of pages displayed together */
	gulong nx, ny;
	gint update_func_id;

	GPtrArray *page_array;
};

struct _GnomePrintJobPreviewClass {
	GtkWindowClass parent_class;
};

typedef struct _GPMPPrivate GPMPPrivate;

struct _GPMPPrivate {
	/* Our GnomePrintJob */
	GnomePrintJob *job;
	/* Our GnomePrintPreview */
	GnomePrintContext *preview;

	GtkWidget *page_entry;
	GtkWidget *scrolled_window;
	GtkWidget *last;
	GnomeCanvas *canvas;
	GnomeCanvasItem *page;

	gint current_page;
	gint pagecount;

	/* Strict theme compliance [#96802] */
	gboolean theme_compliance;
	GtkWidget *use_theme_button;	
};

static void gpmp_parse_layout			  (GnomePrintJobPreview *pmp);
static void gnome_print_job_preview_update	  (GnomePrintJobPreview *pmp);
static void gnome_print_job_preview_set_nx_and_ny (GnomePrintJobPreview *pmp,
						   gulong nx, gulong ny);

/*
 * Padding in points around the simulated page
 */

#define PAGE_PAD 4

static gint
goto_page (GnomePrintJobPreview *mp, gint page)
{
	GPMPPrivate *priv;
	guchar c[32];

	g_return_val_if_fail (mp != NULL, GNOME_PRINT_ERROR_BADVALUE);

	priv = (GPMPPrivate *) mp->priv;

	g_return_val_if_fail ((priv->pagecount == 0) || 
			      (page < priv->pagecount),
			      GNOME_PRINT_ERROR_BADVALUE);

	g_snprintf (c, 32, "%d", page + 1);
	gtk_entry_set_text (GTK_ENTRY (priv->page_entry), c);

	gtk_widget_set_sensitive (mp->bpf, (page > 0) &&
					   (priv->pagecount > 1));
	gtk_widget_set_sensitive (mp->bpp, (page > 0) &&
					   (priv->pagecount > 1));
	gtk_widget_set_sensitive (mp->bpn, (page != (priv->pagecount - 1)) &&
					   (priv->pagecount > 1));
	gtk_widget_set_sensitive (mp->bpl, (page != (priv->pagecount - 1)) &&
					   (priv->pagecount > 1));

	if (page != priv->current_page) {
		priv->current_page = page;
		if (priv->pagecount > 0)
			gnome_print_job_preview_update (mp);
	}

	return GNOME_PRINT_OK;
}

static gint
change_page_cmd (GtkEntry *entry, GnomePrintJobPreview *pmp)
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
gpmp_zoom (GnomePrintJobPreview *mp, gdouble factor)
{
	gdouble	     zoom, xdpi, ydpi;
	int          w, h, w_mm, h_mm;
	GdkScreen   *screen;
	GPMPPrivate *priv = (GPMPPrivate *) mp->priv;

	if (factor <= 0.) {
		gint width = GTK_WIDGET (priv->canvas)->allocation.width;
		gint height = GTK_WIDGET (priv->canvas)->allocation.height;
		gdouble zoomx = width  / ((mp->paw + PAGE_PAD * 2) * mp->nx + PAGE_PAD * 2);
		gdouble zoomy = height / ((mp->pah + PAGE_PAD * 2) * mp->ny + PAGE_PAD * 2);

		zoom = MIN (zoomx, zoomy);
	} else
		zoom = mp->zoom * factor;

	mp->zoom = CLAMP (zoom, GPMP_ZOOM_MIN, GPMP_ZOOM_MAX);

	gtk_widget_set_sensitive (mp->bz1, (!CLOSE_ENOUGH (mp->zoom, 1.0)));
	gtk_widget_set_sensitive (mp->bzi, (!CLOSE_ENOUGH (mp->zoom, GPMP_ZOOM_MAX)));
	gtk_widget_set_sensitive (mp->bzo, (!CLOSE_ENOUGH (mp->zoom, GPMP_ZOOM_MIN)));

	screen = gtk_widget_get_screen (GTK_WIDGET (priv->canvas)),
#if 0
       /* there is no per monitor physical size info available for now */
	n = gdk_screen_get_n_monitors (screen);
	for (i = 0; i < n; i++) {
		gdk_screen_get_monitor_geometry (screen, i, &monitor);
		if (x >= monitor.x && x <= monitor.x + monitor.width &&
		    y >= monitor.y && y <= monitor.y + monitor.height) {
			break;
		}
	}
#endif
	/*
	 * From xdpyinfo.c:
	 *
	 * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
	 *
	 *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
	 *         = N pixels / (M inch / 25.4)
	 *         = N * 25.4 pixels / M inch
	 */
	w_mm = gdk_screen_get_width_mm (screen);
	xdpi = -1.;
	if (w_mm > 0) {
		w    = gdk_screen_get_width (screen);
		xdpi = (w * 25.4) / (gdouble) w_mm;
	} 
	if (xdpi < 30. || 600. < xdpi) {
		g_warning ("Invalid the x-resolution for the screen, assuming 96dpi");
		xdpi = 96.;
	}

	h_mm = gdk_screen_get_height_mm (screen);
	ydpi = -1.;
	if (h_mm > 0) {
		h    = gdk_screen_get_height (screen);
		ydpi = (h * 25.4) / (gdouble) h_mm;
	}
	if (ydpi < 30. || 600. < ydpi) {
		g_warning ("Invalid the y-resolution for the screen, assuming 96dpi");
		ydpi = 96.;
	}

	gnome_canvas_set_pixels_per_unit (priv->canvas, mp->zoom * (xdpi + ydpi) / (72. * 2.));
}

/* Button press handler for the print preview canvas */

static gint
preview_canvas_button_press (GtkWidget *widget, GdkEventButton *event, GnomePrintJobPreview *mp)
{
	gint retval;

	retval = FALSE;

	if (event->button == 1) {
		GdkCursor *cursor;
		GdkDisplay *display = gtk_widget_get_display (widget);

		mp->dragging = TRUE;

		gnome_canvas_get_scroll_offsets (GNOME_CANVAS (widget), &mp->offsetx, &mp->offsety);

		mp->anchorx = event->x - mp->offsetx;
		mp->anchory = event->y - mp->offsety;

		cursor = gdk_cursor_new_for_display (display, GDK_FLEUR);
		gdk_pointer_grab (widget->window, FALSE,
				  (GDK_POINTER_MOTION_MASK | 
				   GDK_POINTER_MOTION_HINT_MASK | 
				   GDK_BUTTON_RELEASE_MASK),
				  NULL, cursor, event->time);
		gdk_cursor_unref (cursor);

		retval = TRUE;
	}

	return retval;
}

/* Motion notify handler for the print preview canvas */

static gint
preview_canvas_motion (GtkWidget *widget, GdkEventMotion *event, GnomePrintJobPreview *mp)
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
preview_canvas_button_release (GtkWidget *widget, GdkEventButton *event, GnomePrintJobPreview *mp)
{
	gint retval;

	retval = TRUE;

	if (event->button == 1) {
		mp->dragging = FALSE;
		gdk_display_pointer_ungrab (gtk_widget_get_display (widget),
					    event->time);
		retval = TRUE;
	}

	return retval;
}


static void
preview_close_cmd (gpointer unused, GnomePrintJobPreview *mp)
{
	gtk_widget_destroy (GTK_WIDGET (mp));
}

static gboolean
preview_delete_event (gpointer unused, GdkEventAny *event, GnomePrintJobPreview *mp)
{
	gtk_widget_destroy (GTK_WIDGET (mp));

	return TRUE;
}

static void
preview_file_print_cmd (void *unused, GnomePrintJobPreview *pmp)
{
	GPMPPrivate *priv;

	priv = (GPMPPrivate *) pmp->priv;

	gnome_print_job_print (priv->job);

	/* fixme: should we clean ourselves up now? */
}

static void
preview_first_page_cmd (void *unused, GnomePrintJobPreview *pmp)
{
	goto_page (pmp, 0);
}

static void
preview_next_page_cmd (void *unused, GnomePrintJobPreview *pmp)
{
	GPMPPrivate *priv = (GPMPPrivate *) pmp->priv;

	goto_page (pmp, MIN (priv->current_page + pmp->nx * pmp->ny,
			     priv->pagecount - 1));
}

static void
preview_prev_page_cmd (void *unused, GnomePrintJobPreview *pmp)
{
	GPMPPrivate *priv = (GPMPPrivate *) pmp->priv;

	goto_page (pmp,
		   MAX ((gint) (priv->current_page - pmp->nx * pmp->ny), 0));
}

static void
preview_last_page_cmd (void *unused, GnomePrintJobPreview *pmp)
{
	GPMPPrivate *priv = (GPMPPrivate *) pmp->priv;

	goto_page (pmp, priv->pagecount - 1);
}

static void
gpmp_zoom_in_cmd (GtkToggleButton *t, GnomePrintJobPreview *pmp)
{
	gpmp_zoom (pmp, GPMP_ZOOM_IN_FACTOR);
}

static void
gpmp_zoom_out_cmd (GtkToggleButton *t, GnomePrintJobPreview *pmp)
{
	gpmp_zoom (pmp, GPMP_ZOOM_OUT_FACTOR);
}

static void
preview_zoom_fit_cmd (GnomePrintJobPreview *mp)
{
	gpmp_zoom (mp, -1.);
}

static void
preview_zoom_100_cmd (GnomePrintJobPreview *mp)
{
	gpmp_zoom (mp,  1. / mp->zoom); /* cheesy way to force 100% */
}

static gint
preview_canvas_key (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GnomePrintJobPreview *pmp;
	GPMPPrivate *priv;
	gint x,y;
	gint height, width;
	gint domove = 0;

	pmp = (GnomePrintJobPreview *) data;

	priv = (GPMPPrivate *) pmp->priv;

	gnome_canvas_get_scroll_offsets (priv->canvas, &x, &y);
	height = GTK_WIDGET (priv->canvas)->allocation.height;
	width = GTK_WIDGET (priv->canvas)->allocation.width;

	switch (event->keyval) {
	case '1':
		preview_zoom_100_cmd (pmp);
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
	case GDK_q:
	case GDK_w:
		if (event->state & GDK_CONTROL_MASK) {
			gtk_widget_destroy (GTK_WIDGET (pmp));
			return TRUE;		
		}
	        /* break skipped */
	default:
		return FALSE;
	}

	if (domove)
		gnome_canvas_scroll_to (priv->canvas, x, y);

	g_signal_stop_emission (G_OBJECT (widget), g_signal_lookup ("key_press_event", G_OBJECT_TYPE (widget)), 0);
	
	return TRUE;
}

static void
canvas_style_changed_cb (GtkWidget *canvas, GtkStyle *ps, GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	GtkStyle *style;
	gint32 border_color;
	gint32 page_color;
	guint i;
	GnomeCanvasItem *page;

	priv = (GPMPPrivate *) mp->priv;

	style = gtk_widget_get_style (GTK_WIDGET (canvas));
	if (priv->theme_compliance) {
		page_color = GPP_COLOR_RGBA (style->base [GTK_STATE_NORMAL], 
					     0xff);
		border_color = GPP_COLOR_RGBA (style->text [GTK_STATE_NORMAL], 
					       0xff);
	}
	else {
		page_color = GPP_COLOR_RGBA (style->white, 0xff);
		border_color = GPP_COLOR_RGBA (style->black, 0xff);
	}

	for (i = 0; i < mp->page_array->len; i++) {
		page = g_object_get_data (mp->page_array->pdata[i], "page");
		gnome_canvas_item_set (page,
			       "fill_color_rgba", page_color,
			       "outline_color_rgba", border_color,
			       NULL);
	}

	gnome_print_job_preview_update (mp);
}

static void
entry_insert_text_cb (GtkEditable *editable, const gchar *text, gint length, gint *position)
{
	gunichar c;
	const gchar *p;
 	const gchar *end;

	p = text;
	end = text + length;

	while (p != end) {
		const gchar *next;
		next = g_utf8_next_char (p);

		c = g_utf8_get_char (p);

		if (!g_unichar_isdigit (c)) {
			g_signal_stop_emission_by_name (editable, "insert_text");
			break;
		}

		p = next;
	}
}

static gboolean 
entry_focus_out_event_cb (GtkWidget *widget, GdkEventFocus *event, GnomePrintJobPreview *pjp)
{
	GPMPPrivate *priv;
	const gchar *text;
	gint page;

	priv = (GPMPPrivate *) pjp->priv;

	text = gtk_entry_get_text (GTK_ENTRY(widget));
	page = atoi (text) - 1;
	
	/* Reset the page number only if really needed */
	if (page != priv->current_page) {
		gchar *str;

		str = g_strdup_printf ("%d", priv->current_page + 1);
		gtk_entry_set_text (GTK_ENTRY (widget), str);
		g_free (str);
	}

	return FALSE;
}

static void
check_button_toggled_cb (GtkWidget *widget, GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	gboolean use_theme;
	
	priv = (GPMPPrivate *)mp->priv;
	use_theme = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	priv->theme_compliance = use_theme;
	gnome_print_preview_set_use_theme (use_theme);
	g_signal_emit_by_name (G_OBJECT (priv->canvas), "style_set", NULL);
}

static void
create_preview_canvas (GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	AtkObject *atko;
	GtkWidget *status;
	GtkWidget *l;

	priv = (GPMPPrivate *) mp->priv;

	priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (priv->scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_widget_push_colormap (gdk_rgb_get_colormap ());
	priv->canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
	gnome_canvas_set_center_scroll_region (priv->canvas, FALSE);
	gtk_widget_pop_colormap ();
	atko = gtk_widget_get_accessible (GTK_WIDGET (priv->canvas));
	atk_object_set_name (atko, _("Page Preview"));
	atk_object_set_description (atko, _("The preview of a page in the document to be printed"));

	g_signal_connect (G_OBJECT (priv->canvas), "button_press_event",
			  (GCallback) preview_canvas_button_press, mp);
	g_signal_connect (G_OBJECT (priv->canvas), "button_release_event",
			  (GCallback) preview_canvas_button_release, mp);
	g_signal_connect (G_OBJECT (priv->canvas), "motion_notify_event",
			  (GCallback) preview_canvas_motion, mp);
	g_signal_connect (G_OBJECT (priv->canvas), "key_press_event",
			  (GCallback) preview_canvas_key, mp);

	gtk_container_add (GTK_CONTAINER (priv->scrolled_window), GTK_WIDGET (priv->canvas));

	g_signal_connect (G_OBJECT (priv->canvas), "style_set",
			  G_CALLBACK (canvas_style_changed_cb), mp);
	
	status = gtk_hbox_new (FALSE, 0);
	l = gtk_label_new_with_mnemonic (_("P_age: "));
	gtk_box_pack_start (GTK_BOX (status), l, FALSE, FALSE, 4);
	priv->page_entry = gtk_entry_new ();
	gtk_widget_set_size_request (priv->page_entry, 40, -1);

	g_signal_connect (G_OBJECT (priv->page_entry), "activate", 
			  G_CALLBACK (change_page_cmd), mp);
	g_signal_connect (G_OBJECT (priv->page_entry), "insert_text",
			  G_CALLBACK (entry_insert_text_cb), NULL);
	g_signal_connect (G_OBJECT (priv->page_entry), "focus_out_event",
			  G_CALLBACK (entry_focus_out_event_cb), mp);

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

	priv->use_theme_button = gtk_check_button_new_with_mnemonic (_("Use _theme colors for content"));
	gtk_box_pack_end (GTK_BOX (status), priv->use_theme_button, FALSE,
			    FALSE, 2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->use_theme_button),
		gnome_print_preview_get_use_theme ());
	g_signal_connect (G_OBJECT (priv->use_theme_button), "toggled",
			  G_CALLBACK (check_button_toggled_cb), mp);

	gtk_box_pack_start (GTK_BOX (mp->vbox), status, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (mp->vbox), priv->scrolled_window, TRUE, TRUE, 0);

	gtk_widget_show_all (mp->vbox);

	gtk_widget_grab_focus (GTK_WIDGET (priv->canvas));
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

static void
on_1x1_clicked (GtkMenuItem *i, GnomePrintJobPreview *mp)
{
	gnome_print_job_preview_set_nx_and_ny (mp, 1, 1);
}

static void
on_1x2_clicked (GtkMenuItem *i, GnomePrintJobPreview *mp)
{
	gnome_print_job_preview_set_nx_and_ny (mp, 2, 1);
}

static void
on_2x1_clicked (GtkMenuItem *i, GnomePrintJobPreview *mp)
{
	gnome_print_job_preview_set_nx_and_ny (mp, 1, 2);
}

static void
on_2x2_clicked (GtkMenuItem *i, GnomePrintJobPreview *mp)
{
	gnome_print_job_preview_set_nx_and_ny (mp, 2, 2);
}

#if 0
static void
on_other_clicked (GtkMenuItem *i, GnomePrintJobPreview *mp)
{
	g_warning ("Not implemented!");
}
#endif

static void
gpmp_multi_cmd (GtkWidget *w, GnomePrintJobPreview *mp)
{
	GtkWidget *m, *i;

	m = gtk_menu_new ();
	gtk_widget_show (m);
	g_signal_connect (m, "selection_done", G_CALLBACK (gtk_widget_destroy),
			  m);

        i = gtk_menu_item_new_with_label ("1x1");
        gtk_widget_show (i);
	gtk_menu_attach (GTK_MENU (m), i, 0, 1, 0, 1);
        g_signal_connect (i, "activate", G_CALLBACK (on_1x1_clicked), mp);

        i = gtk_menu_item_new_with_label ("2x1");
        gtk_widget_show (i);
	gtk_menu_attach (GTK_MENU (m), i, 0, 1, 1, 2);
        g_signal_connect (i, "activate", G_CALLBACK (on_2x1_clicked), mp);

        i = gtk_menu_item_new_with_label ("1x2");
        gtk_widget_show (i);
	gtk_menu_attach (GTK_MENU (m), i, 1, 2, 0, 1);
        g_signal_connect (i, "activate", G_CALLBACK (on_1x2_clicked), mp);

        i = gtk_menu_item_new_with_label ("2x2");
        gtk_widget_show (i);
	gtk_menu_attach (GTK_MENU (m), i, 1, 2, 1, 2);
        g_signal_connect (i, "activate", G_CALLBACK (on_2x2_clicked), mp);

#if 0
        i = gtk_menu_item_new_with_label (_("Other"));
        gtk_widget_show (i);
	gtk_menu_attach (GTK_MENU (m), i, 0, 2, 2, 3);
        gtk_widget_set_sensitive (i, FALSE);
        g_signal_connect (i, "activate", G_CALLBACK (on_other_clicked), mp);
#endif

	gtk_menu_popup (GTK_MENU (m), NULL, NULL, NULL, mp, 0,
			GDK_CURRENT_TIME);
}

static void
create_toplevel (GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	GtkWidget *tb;
	gint width, height;
	GdkGeometry hints;
	GtkToolItem *i;

	priv = (GPMPPrivate *) mp->priv;

	tb = gtk_toolbar_new ();
	gtk_toolbar_set_style (GTK_TOOLBAR (tb), GTK_TOOLBAR_ICONS);
	gtk_widget_show (tb);
	gtk_box_pack_start (GTK_BOX (mp->vbox), tb, FALSE, FALSE, 0);

	i = gtk_tool_button_new_from_stock (GTK_STOCK_PRINT);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Prints the current file"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_file_print_cmd),mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_tool_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips, 
				   _("Closes print preview window"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_close_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_separator_tool_item_new ();
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_FIRST);
	mp->bpf = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Shows the first page"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_first_page_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK);
	mp->bpp = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Shows the previous page"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_prev_page_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	mp->bpn = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Shows the next page"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_next_page_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_LAST);
	mp->bpl = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				    _("Shows the last page"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (preview_last_page_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_separator_tool_item_new ();
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_100);
	mp->bz1 = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Zooms 1:1"), "");
	g_signal_connect_swapped (i, "clicked",
		G_CALLBACK (preview_zoom_100_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_FIT);
	mp->bzf = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Zooms to fit the whole page"), "");
	g_signal_connect_swapped (i, "clicked",
		G_CALLBACK (preview_zoom_fit_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_IN);
	mp->bzi = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Zooms the page in"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (gpmp_zoom_in_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);
	i = gtk_tool_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
	mp->bzo = GTK_WIDGET (i);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("Zooms the page out"), "");
	g_signal_connect (i, "clicked",
			  G_CALLBACK (gpmp_zoom_out_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_separator_tool_item_new ();
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	i = gtk_tool_button_new_from_stock (GTK_STOCK_DND_MULTIPLE);
	gtk_tool_item_set_tooltip (i, GTK_TOOLBAR (tb)->tooltips,
				   _("View multiple pages"), "");
	g_signal_connect (i, "clicked", G_CALLBACK (gpmp_multi_cmd), mp);
	gtk_widget_show (GTK_WIDGET (i));
	gtk_toolbar_insert (GTK_TOOLBAR (tb), i, -1);

	width  = MIN (mp->paw + PAGE_PAD * 3, gdk_screen_width () * 3 / 4);
	height = MIN (mp->pah + PAGE_PAD * 3, gdk_screen_height () * 3 / 4);

	hints.base_width = mp->paw;
        hints.base_height = mp->pah;
        hints.min_width = 150;
        hints.min_height = 150;
	
	gtk_window_set_default_size (&(mp->window),
				     width, 
				     height);
	gtk_window_set_geometry_hints 
		(&(mp->window), priv->scrolled_window,
		 &hints, GDK_HINT_MIN_SIZE | GDK_HINT_BASE_SIZE);
}

/*
  The GnomePrintJobPreview object
*/

static void gnome_print_job_preview_class_init (GnomePrintJobPreviewClass *klass);
static void gnome_print_job_preview_init (GnomePrintJobPreview *pmp);

static GtkWindowClass *parent_class;

GType
gnome_print_job_preview_get_type (void)
{
	static GType type;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomePrintJobPreviewClass),
			NULL, NULL,
			(GClassInitFunc) gnome_print_job_preview_class_init,
			NULL, NULL,
			sizeof (GnomePrintJobPreview),
			0,
			(GInstanceInitFunc) gnome_print_job_preview_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_WINDOW, "GnomePrintJobPreview", &info, 0);
	}
	return type;
}

static void
gnome_print_job_preview_destroy (GtkObject *object)
{
	GnomePrintJobPreview *pmp = GNOME_PRINT_JOB_PREVIEW (object);
	GPMPPrivate *priv = (GPMPPrivate *) pmp->priv;

	if (pmp->page_array != NULL) {
		g_ptr_array_free (pmp->page_array, TRUE);
		pmp->page_array = NULL;
	}

	if (priv->job != NULL) {
		g_object_unref (G_OBJECT (priv->job));
		priv->job = NULL;
	}

	if (pmp->update_func_id) {
		g_source_remove (pmp->update_func_id);
		pmp->update_func_id = 0;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gnome_print_job_preview_finalize (GObject *object)
{
	GnomePrintJobPreview *pmp;
	GPMPPrivate *priv;

	pmp = GNOME_PRINT_JOB_PREVIEW (object);
	priv = pmp->priv;

	if (priv) {
		g_free (priv);
		pmp->priv = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
update_func (gpointer data)
{
	GnomePrintJobPreview *pmp = GNOME_PRINT_JOB_PREVIEW (data);

	gnome_print_job_preview_update (pmp);
	pmp->update_func_id = 0;
	
	return FALSE;
}

static void
gnome_print_job_preview_set_nx_and_ny (GnomePrintJobPreview *pmp,
				       gulong nx, gulong ny)
{
	GPtrArray *a;
	GnomeCanvasItem *group, *item;
	GPMPPrivate *priv;
	guint i, col, row;
	GObject *preview;

	g_return_if_fail (GNOME_IS_PRINT_JOB_PREVIEW (pmp));
	g_return_if_fail (nx > 0);
	g_return_if_fail (ny > 0);

	pmp->nx = nx;
	pmp->ny = ny; 

	/* Remove unnecessary pages */
	priv = (GPMPPrivate *) pmp->priv;
	a = pmp->page_array;
	while (pmp->page_array->len > MIN (nx * ny, priv->pagecount)) {

		/* Remove preview (if any) */
		preview = g_object_get_data (a->pdata[a->len - 1], "preview");
		if (preview) g_object_unref (preview);
		g_object_set_data (a->pdata[a->len - 1], "preview", NULL);

		/* Remove page */
		gtk_object_destroy (GTK_OBJECT (a->pdata[a->len - 1]));
		g_ptr_array_remove_index (a, a->len - 1);
	}

	/* Create pages if needed */
	while (pmp->page_array->len < MIN (nx * ny, priv->pagecount)) {
		group = gnome_canvas_item_new (
			gnome_canvas_root (priv->canvas),
			GNOME_TYPE_CANVAS_GROUP, "x", 0., "y", 0., NULL);
		g_ptr_array_add (a, group);
		item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
			GNOME_TYPE_CANVAS_RECT, "x1", 0.0, "y1", 0.0,
			"x2", (gdouble) pmp->paw, "y2", (gdouble) pmp->pah,
			"fill_color", "white", "outline_color", "black",
			"width_pixels", 1, NULL);
		gnome_canvas_item_lower_to_bottom (item);
		g_object_set_data (G_OBJECT (group), "page", item);
		item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (group),
			GNOME_TYPE_CANVAS_RECT, "x1", 3.0, "y1", 3.0,
			"x2", (gdouble) pmp->paw + 3,
			"y2", (gdouble) pmp->pah + 3,
			"fill_color", "black", NULL);
		gnome_canvas_item_lower_to_bottom (item);
	}

	/* Position the pages */
	for (i = 0; i < a->len; i++) {
		col = i % pmp->nx;
		row = i / pmp->nx;
		g_object_set (a->pdata[i],
			"x", (gdouble) col * (pmp->paw + PAGE_PAD * 2),
			"y", (gdouble) row * (pmp->pah + PAGE_PAD * 2), NULL);
	}

	preview_zoom_fit_cmd (pmp);

	if (!pmp->update_func_id)
		pmp->update_func_id = g_idle_add_full (
			G_PRIORITY_DEFAULT_IDLE, update_func, pmp, NULL);
}

static void
gnome_print_job_preview_update (GnomePrintJobPreview *pmp)
{
	GnomePrintConfig *config;
	guint col, row, i, page;
	GPtrArray *a;
	gdouble transform[6];
	ArtDRect region;
	GnomePrintContext *preview;
	GPMPPrivate *priv;
	GnomeCanvasItem *group;

	g_return_if_fail (GNOME_IS_PRINT_JOB_PREVIEW (pmp));

	a = pmp->page_array;
	priv = pmp->priv;

	/* Show something on the pages */
	config = gnome_print_job_get_config (priv->job);
	for (i = 0; i < a->len; i++) {
		col = i % pmp->nx;
		row = i / pmp->nx;
		page = (guint) (priv->current_page / (pmp->nx * pmp->ny))
				* (pmp->nx * pmp->ny) + i;
		group = a->pdata[i];

		/* Remove existing preview (if any) */
		preview = g_object_get_data (G_OBJECT (group), "preview");
		if (preview) g_object_unref (preview);
		g_object_set_data (G_OBJECT (group), "preview", NULL);

		/* Do we need to show this page? */
		if (page < priv->pagecount)
			gnome_canvas_item_show (group);
		else {
			gnome_canvas_item_hide (group);
			continue;
		}

		transform[0] =  1.0; transform[1] =  0.0;
		transform[2] =  0.0; transform[3] = -1.0;
		transform[4] =  col * (pmp->paw + PAGE_PAD * 2);
		transform[5] =  pmp->pah + row * (pmp->pah + PAGE_PAD * 2);
		art_affine_multiply (transform, pmp->pa2ly, transform);
		region.x0 = region.y0 = 0.0 - PAGE_PAD;
		region.x1 = pmp->paw + PAGE_PAD;
		region.y1 = pmp->pah + PAGE_PAD;
		preview = gnome_print_preview_new_full (config,
					priv->canvas, transform, &region);
		g_object_set_data (G_OBJECT (group), "preview", preview);
		gnome_print_preview_theme_compliance (
			GNOME_PRINT_PREVIEW (preview), priv->theme_compliance);
		gnome_print_job_render_page (priv->job, preview, page, TRUE);
	}
	gnome_print_config_unref (config);

	gnome_canvas_set_scroll_region (priv->canvas,
		0 - PAGE_PAD, 0 - PAGE_PAD,
		(pmp->paw + PAGE_PAD * 2) * pmp->nx + PAGE_PAD,
		(pmp->pah + PAGE_PAD * 2) * pmp->ny + PAGE_PAD);
}

enum {
	PROP_0,
	PROP_NX,
	PROP_NY
};

static void
gnome_print_job_preview_get_property (GObject *object, guint n, GValue *v, GParamSpec *pspec)
{
	GnomePrintJobPreview *pmp = GNOME_PRINT_JOB_PREVIEW (object);

	switch (n) {
	case PROP_NX:
		g_value_set_ulong (v, pmp->nx);
		break;
	case PROP_NY:
		g_value_set_ulong (v, pmp->ny);
		break;
	default:
		break;;
	}
}

static void
gnome_print_job_preview_set_property (GObject *object, guint n,
				      const GValue *v, GParamSpec *pspec)
{
	GnomePrintJobPreview *pmp = GNOME_PRINT_JOB_PREVIEW (object);

	switch (n) {
	case PROP_NX:
		gnome_print_job_preview_set_nx_and_ny (pmp,
				g_value_get_ulong (v), pmp->ny);
		break;
	case PROP_NY:
		gnome_print_job_preview_set_nx_and_ny (pmp,
				pmp->nx, g_value_get_ulong (v));
		break;
	default:
		break;
	}
}

static void
gnome_print_job_preview_class_init (GnomePrintJobPreviewClass *klass)
{
	GtkObjectClass *object_class;
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_WINDOW);

	object_class->destroy = gnome_print_job_preview_destroy;

	gobject_class->finalize     = gnome_print_job_preview_finalize;
	gobject_class->set_property = gnome_print_job_preview_set_property;
	gobject_class->get_property = gnome_print_job_preview_get_property;

	g_object_class_install_property (gobject_class, PROP_NX,
		g_param_spec_ulong ("nx", _("Number of pages horizontally"),
			_("Number of pages horizontally"), 1, 0xffff, 1,
			G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class, PROP_NY,
		g_param_spec_ulong ("ny", _("Number of pages vertically"),
			_("Number of pages vertically"), 1, 0xffff, 1,
			G_PARAM_READWRITE));
}

static void
gnome_print_job_preview_init (GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	const gchar *env_theme_variable;
	
	mp->priv = priv = g_new0 (GPMPPrivate, 1);
	priv->current_page = -1;

	priv->theme_compliance = FALSE;
	env_theme_variable = g_getenv("GP_PREVIEW_STRICT_THEME");
	if (env_theme_variable && env_theme_variable [0])
		priv->theme_compliance = TRUE;

	/* Main VBox */
	mp->vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mp->vbox);
	gtk_container_add (GTK_CONTAINER (mp), mp->vbox);

	mp->zoom = 1.0;

	mp->nx = mp->ny = 1;
	mp->page_array = g_ptr_array_new ();
}

GtkWidget *
gnome_print_job_preview_new (GnomePrintJob *gpm, const guchar *title)
{
	GnomePrintJobPreview *gpmp;
	GPMPPrivate *priv;
	gchar *text;

	g_return_val_if_fail (gpm != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_JOB (gpm), NULL);

	if (!title)
		title = _("Gnome Print Preview");

	gpmp = g_object_new (GNOME_TYPE_PRINT_JOB_PREVIEW, NULL);
	g_signal_connect (G_OBJECT (gpmp), "delete_event",
			  (GCallback) preview_delete_event, gpmp);
	gtk_window_set_title (GTK_WINDOW (gpmp), title);

	priv = (GPMPPrivate *) gpmp->priv;

	priv->job = gpm;
	g_object_ref (G_OBJECT (gpm));

	gpmp_parse_layout (gpmp);
	create_toplevel (gpmp);
	create_preview_canvas (gpmp);

	/* this zooms to fit, once we know how big the window actually is */
	g_signal_connect_swapped (G_OBJECT (priv->canvas), "realize",
		(GCallback) preview_zoom_fit_cmd, gpmp);

	priv->pagecount = gnome_print_job_get_pages (gpm);
	goto_page (gpmp, 0);
	
	if (priv->pagecount == 0) {
		priv->pagecount = 1;
		text = g_strdup_printf ("<markup>%d   "
					"<span foreground=\"red\" "
					"weight=\"ultrabold\" "
					"background=\"white\">"
					"%s</span></markup>",
					1, 
					_("No visible output was created."));
	} else 
		text =  g_strdup_printf ("%d", priv->pagecount);
	gtk_label_set_markup_with_mnemonic (GTK_LABEL (priv->last), text);
	g_free (text);

	on_1x1_clicked (NULL, gpmp);
	return (GtkWidget *) gpmp;
}

static void
gpmp_parse_layout (GnomePrintJobPreview *mp)
{
	GPMPPrivate *priv;
	GnomePrintConfig *config;
	GnomePrintLayoutData *lyd;

	priv = (GPMPPrivate *) mp->priv;

	/* Calculate layout-compensated page dimensions */
	mp->paw = GPMP_A4_WIDTH;
	mp->pah = GPMP_A4_HEIGHT;
	art_affine_identity (mp->pa2ly);
	config = gnome_print_job_get_config (priv->job);
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
			/* Now, if job does PA2LY LY2PA concat it ends with scaled identity */
			gnome_print_layout_free (ly);
		}
		gnome_print_layout_data_free (lyd);
	}
}
