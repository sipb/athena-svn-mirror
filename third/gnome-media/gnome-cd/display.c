/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * display.c
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#include <string.h>
#include <libgnome/gnome-util.h>
#include <libgnome/gnome-i18n.h>

#include <libxml/tree.h>

#include "gnome-cd.h"
#include "cdrom.h"
#include "display.h"

#define X_OFFSET 2
#define Y_OFFSET 2

static GtkDrawingAreaClass *parent_class = NULL;

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

#define TOPLEFT 0
#define TOPRIGHT 1
#define BOTTOMLEFT 2
#define BOTTOMRIGHT 3

typedef struct _CDImage {
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GdkRectangle rect;
} CDImage;

typedef struct _CDDisplayTheme {
	gboolean need_resize;
	
	CDImage *corners[4];
	CDImage *straights[4];
	CDImage *middle;

	CDImage *cd, *track, *loop, *once;
} CDDisplayTheme;

struct _CDDisplayPrivate {
	GnomeCDText *layout[CD_DISPLAY_END];
	int max_width;
	int height;
	int need_height;

	CDDisplayTheme *theme;
	/* These need to be removed */
	CDImage *cd, *track, *loop, *once;

	GnomeCDRomMode playmode, loopmode;
};

enum {
	PLAYMODE_CHANGED,
	LOOPMODE_CHANGED,
	LAST_SIGNAL
};

static gulong display_signals[LAST_SIGNAL] = { 0, };

static char *default_text[CD_DISPLAY_END] = {
	"0:00",
	" ",
	N_("Unknown Artist"),
	N_("Unknown Album")
};

static void
free_cd_text (GnomeCDText *text)
{
	g_free (text->text);
	g_object_unref (text->layout);

	g_free (text);
}

static void
scale_image (CDImage *image)
{
	if (image->scaled != NULL) {
		g_object_unref (image->scaled);
	}

	image->scaled = gdk_pixbuf_scale_simple (image->pixbuf,
						 image->rect.width, 16,
						 GDK_INTERP_BILINEAR);
}

static void
cd_display_resize_images (CDDisplay *display,
			  GtkAllocation *allocation)
{
	CDDisplayPrivate *priv;
	CDDisplayTheme *theme;

	priv = display->priv;
	theme = priv->theme;

	if (GTK_WIDGET_REALIZED (GTK_WIDGET (display))) {
		if (allocation == NULL) {
			allocation = &(GTK_WIDGET (display)->allocation);
		}
	} else {
		/* Don't need to scale yet */
		return;
	}

	theme->need_resize = FALSE;
	theme->corners[TOPRIGHT]->rect.x = allocation->width - 16;
	theme->corners[BOTTOMLEFT]->rect.y = allocation->height - 16;
	theme->corners[BOTTOMRIGHT]->rect.x = allocation->width - 16;
	theme->corners[BOTTOMRIGHT]->rect.y = allocation->height - 16;

	theme->straights[BOTTOM]->rect.y = allocation->height - 16;
	theme->straights[RIGHT]->rect.x = allocation->width - 16;

	theme->straights[TOP]->rect.width = allocation->width - 32;
	theme->straights[BOTTOM]->rect.width = allocation->width - 32;
	scale_image (theme->straights[TOP]);
	scale_image (theme->straights[BOTTOM]);

	theme->straights[LEFT]->rect.height = allocation->height - 32;
	theme->straights[RIGHT]->rect.height = allocation->height - 32;

	theme->middle->rect.width = allocation->width - 32;
	theme->middle->rect.height = allocation->height - 32;
	scale_image (theme->middle);
}
	
static void
size_allocate (GtkWidget *drawing_area,
	       GtkAllocation *allocation)
{
	CDDisplay *disp;
	CDDisplayPrivate *priv;
	PangoContext *context;
	PangoDirection base_dir;
	int i;
	
	disp = CD_DISPLAY (drawing_area);
	priv = disp->priv;

	context = pango_layout_get_context (priv->layout[0]->layout);
	base_dir = pango_context_get_base_dir (context);

	for (i = 0; i < CD_DISPLAY_END; i++) {
		PangoRectangle rect;

		pango_layout_set_alignment (priv->layout[i]->layout,
					    base_dir == PANGO_DIRECTION_LTR ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT);
		pango_layout_set_width (priv->layout[i]->layout, allocation->width * 1000);
		pango_layout_get_extents (priv->layout[i]->layout, NULL, &rect);
		priv->layout[i]->height = rect.height / 1000;
	}

	/* Resize and position pixbufs */
	cd_display_resize_images (disp, allocation);
	GTK_WIDGET_CLASS (parent_class)->size_allocate (drawing_area, allocation);
}

static void
size_request (GtkWidget *widget,
	      GtkRequisition *requisition)
{
	CDDisplay *disp;
	CDDisplayPrivate *priv;
	int i, height = 0, width = 0;
	
	disp = CD_DISPLAY (widget);
	priv = disp->priv;

	GTK_WIDGET_CLASS (parent_class)->size_request (widget, requisition);
	
	for (i = 0; i < CD_DISPLAY_END; i++) {
		PangoRectangle rect;

		if (i == CD_DISPLAY_LINE_INFO) {
			priv->track->rect.y = height;
			priv->cd->rect.y = height;
			priv->once->rect.y = height;
			priv->loop->rect.y = height;

			height += priv->track->rect.height;
			width = MAX (width, priv->track->rect.width * 2);
		} else {
			pango_layout_get_extents (priv->layout[i]->layout, NULL, &rect);
			height += (rect.height / 1000);
			width = MAX (width, rect.width / 1000);
		}
	}

	requisition->width = width + (2 * X_OFFSET);

	requisition->height = height + (2 * Y_OFFSET);
}

static void
draw_pixbuf (GdkPixbuf *pixbuf,
	     GdkDrawable *drawable,
	     GdkGC *gc,
	     int src_x, int src_y,
	     int dest_x, int dest_y,
	     int width, int height)
{
	gdk_pixbuf_render_to_drawable_alpha (pixbuf,
					     drawable,
					     src_x, src_y,
					     dest_x, dest_y,
					     width, height,
					     GDK_PIXBUF_ALPHA_FULL,
					     128,
					     GDK_RGB_DITHER_NORMAL,
					     0, 0);
}
	     
static gboolean
expose_event (GtkWidget *drawing_area,
	      GdkEventExpose *event)
{
	CDDisplay *disp;
	CDDisplayPrivate *priv;
	CDDisplayTheme *theme;
	PangoContext *context;
	PangoDirection base_dir;
	GdkRectangle *area;
	GtkStateType state;
	int height = 0;
	int i;

	disp = CD_DISPLAY (drawing_area);
	priv = disp->priv;
	theme = priv->theme;

	g_assert (theme != NULL);
	
	area = &event->area;

	state = GTK_WIDGET_STATE (drawing_area);
	if (theme->need_resize == TRUE) {
		cd_display_resize_images (disp, NULL);
	}
	/* Check corners and draw them */
	for (i = 0; i < 4; i++) {
		GdkRectangle inter;
		
		if (gdk_rectangle_intersect (area, &theme->corners[i]->rect, &inter) == TRUE) {
			draw_pixbuf (theme->corners[i]->pixbuf,
				     drawing_area->window,
				     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
				     inter.x - theme->corners[i]->rect.x,
				     inter.y - theme->corners[i]->rect.y,
				     inter.x, inter.y,
				     inter.width, inter.height);
		}
	}

	for (i = TOP; i <= BOTTOM; i++) {
		GdkRectangle inter;

		if (gdk_rectangle_intersect (&theme->straights[i]->rect, area, &inter) == TRUE) {
			draw_pixbuf (theme->straights[i]->scaled,
				     drawing_area->window,
				     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
				     inter.x - theme->straights[i]->rect.x,
				     inter.y - theme->straights[i]->rect.y,
				     inter.x, inter.y,
				     inter.width, inter.height);
		}
	}

	for (i = LEFT; i <= RIGHT; i++) {
		GdkRectangle inter;
		if (gdk_rectangle_intersect (area, &theme->straights[i]->rect, &inter) == TRUE) {
			int repeats, extra_s, extra_end, d, j;

			d = inter.y / 16;
			extra_s = inter.y - (d * 16);
			
			if (extra_s > 0) {
				draw_pixbuf (theme->straights[i]->pixbuf,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->straights[i]->rect.x,
					     16 - extra_s,
					     inter.x, inter.y,
					     inter.width, extra_s);
			}

			repeats = (inter.height - extra_s) / 16;
			extra_end = (inter.height - extra_s) % 16;
			
			for (j = 0; j < repeats; j++) {
				draw_pixbuf (theme->straights[i]->pixbuf,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->straights[i]->rect.x,
					     0,
					     inter.x,
					     inter.y + (j * 16) + extra_s,
					     inter.width, 16);
			}

			if (extra_end > 0) {
				draw_pixbuf (theme->straights[i]->pixbuf,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->straights[i]->rect.x,
					     0,
					     inter.x,
					     inter.y + (repeats * 16) + extra_s,
					     inter.width, extra_end);
			}
		}
	}
							       
	/* Do the middle - combination of the above */
	{
		GdkRectangle inter;
		if (gdk_rectangle_intersect (area, &theme->middle->rect, &inter) == TRUE) {
			int repeats, extra_s, extra_end, j, d;

			d = inter.y / 16;
			extra_s = inter.y - (d * 16);
			
			if (extra_s > 0) {
				draw_pixbuf (theme->middle->scaled,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->middle->rect.x,
					     16 - extra_s,
					     inter.x, inter.y,
					     inter.width, extra_s);
			}

			repeats = (inter.height - extra_s) / 16;
			extra_end = (inter.height - extra_s) % 16;
			
			for (j = 0; j < repeats; j++) {
				draw_pixbuf (theme->middle->scaled,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->middle->rect.x,
					     0,
					     inter.x,
					     inter.y + (j * 16) + extra_s,
					     inter.width, 16);
			}

			if (extra_end > 0) {
				draw_pixbuf (theme->middle->scaled,
					     drawing_area->window,
					     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
					     inter.x - theme->middle->rect.x,
					     0,
					     inter.x,
					     inter.y + (repeats * 16) + extra_s,
					     inter.width, extra_end);
			}
		}
	}

	/* Do the info line */
	{
		GdkRectangle inter;
		CDImage *im;

		/* Track / CD? */
		im = (priv->playmode == GNOME_CDROM_SINGLE_TRACK ? priv->track : priv->cd);
		if (gdk_rectangle_intersect (area, &im->rect, &inter) == TRUE) {
			draw_pixbuf (im->pixbuf,
				     drawing_area->window,
				     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
				     inter.x - im->rect.x,
				     inter.y - im->rect.y,
				     inter.x, inter.y,
				     inter.width, inter.height);
		}

		im = (priv->loopmode == GNOME_CDROM_PLAY_ONCE ? priv->once : priv->loop);
		if (gdk_rectangle_intersect (area, &im->rect, &inter) == TRUE) {
			draw_pixbuf (im->pixbuf,
				     drawing_area->window,
				     drawing_area->style->bg_gc[GTK_STATE_NORMAL],
				     inter.x - im->rect.x,
				     inter.y - im->rect.y,
				     inter.x, inter.y,
				     inter.width, inter.height);
		}
	}
	
	context = pango_layout_get_context (priv->layout[0]->layout);
	base_dir = pango_context_get_base_dir (context);

	for (i = 0; i < CD_DISPLAY_END &&
		     height < area->y + area->height + Y_OFFSET; i++) {

		if (height + priv->layout[i]->height >= Y_OFFSET + area->y) {
			pango_layout_set_alignment (priv->layout[i]->layout,
						    base_dir == PANGO_DIRECTION_LTR ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT);

			gdk_draw_layout_with_colors (drawing_area->window,
						     drawing_area->style->text_gc[state],
						     X_OFFSET, 
						     Y_OFFSET + height,
						     priv->layout[i]->layout,
						     NULL, NULL);
		}

		if (i == CD_DISPLAY_LINE_INFO) {
			height += 16;
		} else {
			height += priv->layout[i]->height;
		}
	}

	return TRUE;
}

static void
finalize (GObject *object)
{
	CDDisplay *disp;
	CDDisplayPrivate *priv;
	int i;

	disp = CD_DISPLAY (object);
	priv = disp->priv;

	if (priv == NULL) {
		return;
	}

	/* FIXME: Free the CDImages here */
	
	for (i = 0; i < CD_DISPLAY_END; i++) {
		free_cd_text (priv->layout[i]);
	}

	g_free (priv);

	disp->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
realize (GtkWidget *widget)
{
	CDDisplay *disp;

	disp = CD_DISPLAY (widget);
#if 0
	GdkColormap *cmap;

	cmap = gtk_widget_get_colormap (widget);
	disp->priv->red.red = 65535;
	disp->priv->red.green = 0;
	disp->priv->red.blue = 0;
	disp->priv->red.pixel = 0;
	gdk_color_alloc (cmap, &disp->priv->red);

	disp->priv->blue.red = 0;
	disp->priv->blue.green = 0;
	disp->priv->blue.blue = 65535;
	disp->priv->blue.pixel = 0;
	gdk_color_alloc (cmap, &disp->priv->blue);
#endif
	gtk_widget_add_events (widget, GDK_BUTTON_PRESS_MASK |
			       GDK_KEY_PRESS_MASK);

	GTK_WIDGET_CLASS (parent_class)->realize (widget);
}

static int
button_press_event (GtkWidget *widget,
		    GdkEventButton *ev)
{
	CDDisplay *disp = CD_DISPLAY (widget);
	CDDisplayPrivate *priv = disp->priv;

	if (ev->button != 1) {
		return FALSE;
	}
	
	/* Check the play mode */
	if (ev->x >= priv->track->rect.x &&
	    ev->x <= (priv->track->rect.x + priv->track->rect.width) &&
	    ev->y >= priv->track->rect.y &&
	    ev->y <= (priv->track->rect.y + priv->track->rect.height)) {
		priv->playmode = (priv->playmode == GNOME_CDROM_SINGLE_TRACK ?
				  GNOME_CDROM_WHOLE_CD : GNOME_CDROM_SINGLE_TRACK);
		g_signal_emit (G_OBJECT (widget), display_signals[PLAYMODE_CHANGED],
			       0, priv->playmode);
		gtk_widget_queue_draw_area (widget, priv->track->rect.x,
					    priv->track->rect.y,
					    priv->track->rect.width,
					    priv->track->rect.height);
		return TRUE;
	}

	/* Check the loop mode */
	if (ev->x >= priv->once->rect.x &&
	    ev->x <= (priv->once->rect.x + priv->once->rect.width) &&
	    ev->y >= priv->once->rect.y &&
	    ev->y <= (priv->once->rect.y + priv->once->rect.height)) {
		priv->loopmode = (priv->loopmode == GNOME_CDROM_PLAY_ONCE ?
				  GNOME_CDROM_LOOP : GNOME_CDROM_PLAY_ONCE);
		g_signal_emit (G_OBJECT (widget), display_signals[LOOPMODE_CHANGED],
			       0, priv->loopmode);
		gtk_widget_queue_draw_area (widget, priv->track->rect.x,
					    priv->track->rect.y,
					    priv->track->rect.width,
					    priv->track->rect.height);
		return TRUE;
	}

	return FALSE;
}
		     
static void
class_init (CDDisplayClass *klass)
{
	GObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	widget_class = GTK_WIDGET_CLASS (klass);

	object_class->finalize = finalize;

	widget_class->size_allocate = size_allocate;
	widget_class->size_request = size_request;
	widget_class->expose_event = expose_event;
  	widget_class->realize = realize;
	widget_class->button_press_event = button_press_event;
	
	/* Signals */
	display_signals[PLAYMODE_CHANGED] = g_signal_new ("playmode-changed",
							  G_TYPE_FROM_CLASS (klass),
							  G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
							  G_STRUCT_OFFSET (CDDisplayClass, playmode_changed),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__INT,
							  G_TYPE_NONE,
							  1, G_TYPE_INT);
	display_signals[LOOPMODE_CHANGED] = g_signal_new ("loopmode-changed",
							  G_TYPE_FROM_CLASS (klass),
							  G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
							  G_STRUCT_OFFSET (CDDisplayClass, loopmode_changed),
							  NULL, NULL,
							  g_cclosure_marshal_VOID__INT,
							  G_TYPE_NONE,
							  1, G_TYPE_INT);
	
	parent_class = g_type_class_peek_parent (klass);
}

static CDImage *
cd_image_new (const char *filename,
	      int x,
	      int y)
{
	CDImage *image;
	char *fullname;
	
	image = g_new0 (CDImage, 1);

	if (filename[0] != '/') {
		fullname = gnome_program_locate_file (NULL,
			   GNOME_FILE_DOMAIN_PIXMAP, filename, TRUE, NULL);
		if (fullname == NULL) {
			/* If the elegant way doesn't work, try and brute force it */
			fullname = g_strconcat(GNOME_ICONDIR, "/", filename, NULL);
		}
		image->pixbuf = gdk_pixbuf_new_from_file (fullname, NULL);
		g_free (fullname);
	} else {
		image->pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
	}
	
	if (image->pixbuf == NULL) {
		g_warning ("Error loading %s", filename);
		g_free (image);
		return NULL;
	}

	image->scaled = image->pixbuf;
	g_object_ref (image->scaled);
	image->rect.x = x;
	image->rect.y = y;
	image->rect.width = gdk_pixbuf_get_width (image->pixbuf);
	image->rect.height = gdk_pixbuf_get_height (image->pixbuf);

	return image;
}

static void
init (CDDisplay *disp)
{
	int i;
	CDDisplayPrivate *priv;

	disp->priv = g_new0 (CDDisplayPrivate, 1);
	priv = disp->priv;

	GTK_WIDGET_UNSET_FLAGS (disp, GTK_NO_WINDOW);

	for (i = 0; i < CD_DISPLAY_END; i++) {
		PangoRectangle rect;
		
		priv->layout[i] = g_new0 (GnomeCDText, 1);
		
		priv->layout[i]->text = g_strdup (_(default_text[i]));
		priv->layout[i]->length = strlen (_(default_text[i]));
		priv->layout[i]->layout = gtk_widget_create_pango_layout (GTK_WIDGET (disp), priv->layout[i]->text);
		pango_layout_set_text (priv->layout[i]->layout,
				       priv->layout[i]->text,
				       priv->layout[i]->length);
		
		pango_layout_get_extents (priv->layout[i]->layout, NULL, &rect);
		priv->layout[i]->height = rect.height / 1000;
		
		priv->max_width = MAX (priv->max_width, rect.width / 1000);
		priv->height += priv->layout[i]->height;
	}

	priv->height += (Y_OFFSET * 2);
	priv->max_width += (X_OFFSET * 2);
	gtk_widget_queue_resize (GTK_WIDGET (disp));

	priv->playmode = GNOME_CDROM_WHOLE_CD;
	priv->loopmode = GNOME_CDROM_PLAY_ONCE;
	
	/* Don't know where these are to be placed yet */
	priv->track = cd_image_new ("gnome-cd/track.png", 16, 0);
	priv->cd = cd_image_new ("gnome-cd/disc.png", 16, 0);
	priv->loop = cd_image_new ("gnome-cd/repeat.png", 36, 0);
	priv->once = cd_image_new ("gnome-cd/once.png", 36, 0);
}

GType
cd_display_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (CDDisplayClass), NULL, NULL,
			(GClassInitFunc) class_init, NULL, NULL,
			sizeof (CDDisplay), 0, (GInstanceInitFunc) init,
		};

		type = g_type_register_static (GTK_TYPE_DRAWING_AREA, "CDDisplay", &info, 0);
	}

	return type;
}

CDDisplay *
cd_display_new (void)
{
	CDDisplay *disp;

	disp = g_object_new (cd_display_get_type (), NULL);
	return CD_DISPLAY (disp);
}

void
cd_display_set_style (CDDisplay *disp)
{
	CDDisplayPrivate *priv;
	CDDisplayLine line;
	GnomeCDText *text;
	PangoRectangle rect;
	int height, max_width = 0;

	priv = disp->priv;

	for (line = CD_DISPLAY_LINE_TIME; line < CD_DISPLAY_END; line++) {
		text = priv->layout[line];
		height = priv->height - text->height;

		pango_layout_set_text (text->layout, text->text, text->length);
		pango_layout_get_extents (text->layout, NULL, &rect);
		text->height = rect.height / 1000;

		priv->height = height + text->height;
		max_width = MAX (priv->max_width, rect.width / 1000);
	}

	priv->max_width = max_width;
	gtk_widget_queue_resize (GTK_WIDGET (disp));
}

const char *
cd_display_get_line (CDDisplay *disp,
		     int line)
{
	CDDisplayPrivate *priv;
	GnomeCDText *text;

	priv = disp->priv;
	text = priv->layout[line];

	return text->text;
}

void
cd_display_set_line (CDDisplay *disp,
		     CDDisplayLine line,
		     const char *new_str)
{
	CDDisplayPrivate *priv;
	GnomeCDText *text;
	PangoRectangle rect;
	int height;

	g_return_if_fail (disp != NULL);
	g_return_if_fail (new_str != NULL);

	priv = disp->priv;

	text = priv->layout[line];
	if (strcmp (new_str, text->text) == 0) {
		/* Same, do nothing */
		return;
	}

	height = priv->height - text->height;
	
	g_free (text->text);
	text->text = g_strdup (new_str);
	text->length = strlen (new_str);
	pango_layout_set_text (text->layout, text->text, text->length);
	pango_layout_get_extents (text->layout, NULL, &rect);
	text->height = rect.height / 1000;
	
	priv->height = height + text->height;
	priv->max_width = MAX (priv->max_width, rect.width / 1000);

	gtk_widget_queue_resize (GTK_WIDGET (disp));
}

void
cd_display_clear (CDDisplay *disp)
{
	CDDisplayPrivate *priv;
	CDDisplayLine line;
	int height, max_width = 0;

	g_return_if_fail (disp != NULL);

	priv = disp->priv;
	for (line = CD_DISPLAY_LINE_TIME; line < CD_DISPLAY_END; line++) {
		GnomeCDText *text;
		PangoRectangle rect;

		text = priv->layout[line];
		height = priv->height - text->height;

		g_free (text->text);
		text->text = g_strdup (" ");
		text->length = 1;
		pango_layout_set_text (text->layout, text->text, 1);
		pango_layout_get_extents (text->layout, NULL, &rect);
		text->height = rect.height / 1000;

		priv->height = height + text->height;
		max_width = MAX (max_width, rect.width / 1000);
	}

	priv->max_width = max_width;
	gtk_widget_queue_resize (GTK_WIDGET (disp));
}

static inline char *
make_fullname (const char *theme_name,
	       const char *name)
{
	char *image;

	image = g_build_filename (THEME_DIR, theme_name, name, NULL);
	
	return image;
}

void
cd_display_parse_theme (CDDisplay *disp,
			GCDTheme *cd_theme,
			xmlDocPtr doc,
			xmlNodePtr cur)
{
	CDDisplayPrivate *priv;
	CDDisplayTheme *theme;

	priv = disp->priv;
	/* Should probably destroy the old theme here */

	priv->theme = g_new0 (CDDisplayTheme, 1);
	theme = priv->theme;

	theme->need_resize = TRUE;
	while (cur != NULL) {
		if (xmlStrcmp (cur->name, (const xmlChar *) "image") == 0) {
			xmlChar *location;

			location = xmlGetProp (cur, (const xmlChar *) "location");
			if (location != NULL) {
				char *file, *full;

				file = xmlNodeListGetString (doc, cur->xmlChildrenNode, 1);
				full = make_fullname (cd_theme->name, file);
				xmlFree (file);
				
				if (xmlStrcmp (location, "top-left") == 0) {
					theme->corners[TOPLEFT] = cd_image_new (full, 0, 0);
				} else if (xmlStrcmp (location, "top-right") == 0) {
					theme->corners[TOPRIGHT] = cd_image_new (full, 48, 0);
				} else if (xmlStrcmp (location, "bottom-right") == 0) {
					theme->corners[BOTTOMRIGHT] = cd_image_new (full, 48, 32);
				} else if (xmlStrcmp (location, "bottom-left") == 0) {
					theme->corners[BOTTOMLEFT] = cd_image_new (full, 0, 32);
				} else if (xmlStrcmp (location, "left") == 0) {
					theme->straights[LEFT] = cd_image_new (full, 0, 16);
				} else if (xmlStrcmp (location, "right") == 0) {
					theme->straights[RIGHT] = cd_image_new (full, 48, 16);
				} else if (xmlStrcmp (location, "top") == 0) {
					theme->straights[TOP] = cd_image_new (full, 16, 0);
				} else if (xmlStrcmp (location, "bottom") == 0) {
					theme->straights[BOTTOM] = cd_image_new (full, 16, 32);
				} else if (xmlStrcmp (location, "middle") == 0) {
					theme->middle = cd_image_new (full, 16, 16);
				} else {
					/** Hmmmmm */
				}

				g_free (full);
			}
			xmlFree (location);
		}

		cur = cur->next;
	}

	/* Update sizes */
	cd_display_resize_images (disp, NULL);
}

GnomeCDText *
cd_display_get_layout(CDDisplay* disp,
		      int i)
{
	g_return_val_if_fail (disp != NULL, NULL);
	g_return_val_if_fail (i < CD_DISPLAY_END, NULL);

	return disp->priv->layout[i];
}

PangoLayout *
cd_display_get_pango_layout(CDDisplay *disp,
			    int i)
{
	g_return_val_if_fail (disp != NULL, NULL);
	g_return_val_if_fail (i < CD_DISPLAY_END, NULL);

	return disp->priv->layout[i]->layout;
}
