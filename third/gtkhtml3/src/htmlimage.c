/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
   Copyright (C) 1997 Torben Weis (weis@kde.org)
   Copyright (C) 1999 Red Hat Software
   Copyright (C) 1999, 2000 Helix Code, Inc.
   
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
#include <glib.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>
#include <gtk/gtk.h>

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "gtkhtml-stream.h"

#include "htmlclueflow.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmldrawqueue.h"
#include "htmlengine.h"
#include "htmlengine-save.h"
#include "htmlenumutils.h"
#include "htmlimage.h"
#include "htmlobject.h"
#include "htmlmap.h"
#include "htmlprinter.h"
#include "htmlgdkpainter.h"
#include "htmlplainpainter.h"

/* HTMLImageFactory stuff.  */

struct _HTMLImageFactory {
	HTMLEngine *engine;
	GHashTable *loaded_images;
	GdkPixbuf  *missing;
	gboolean    animate;
};


#define DEFAULT_SIZE 48
#define STRDUP_HELPER(i,j) if (i != j) {char *tmp = g_strdup (j); g_free(i); i = tmp;}

#define DA(x)

HTMLImageClass html_image_class;
static HTMLObjectClass *parent_class = NULL;

static HTMLImagePointer   *html_image_pointer_new               (const char *filename, HTMLImageFactory *factory);
static void                html_image_pointer_ref               (HTMLImagePointer *ip);
static void                html_image_pointer_unref             (HTMLImagePointer *ip);
static gboolean            html_image_pointer_timeout           (HTMLImagePointer *ip);
static gint                html_image_pointer_run_animation     (HTMLImagePointer *ip);
static void                html_image_pointer_start_animation   (HTMLImagePointer *ip);

static GdkPixbuf *         html_image_factory_get_missing       (HTMLImageFactory *factory);

guint
html_image_get_actual_width (HTMLImage *image, HTMLPainter *painter)
{
	GdkPixbufAnimation *anim = image->image_ptr->animation;
	gint pixel_size = painter ? html_painter_get_pixel_size (painter) : 1;
	gint width;

	if (image->percent_width) {
		/* The cast to `gdouble' is to avoid overflow (eg. when
                   printing).  */
		width = ((gdouble) HTML_OBJECT (image)->max_width
			 * image->specified_width) / 100;
	} else if (image->specified_width > 0) {
		width = image->specified_width * pixel_size;
	} else if (image->image_ptr == NULL || anim == NULL) {
		width = DEFAULT_SIZE * pixel_size;
	} else {
		width = gdk_pixbuf_animation_get_width (anim) * pixel_size;

		if (image->specified_height > 0 || image->percent_height) {
			double scale;

			scale =  ((double) html_image_get_actual_height (image, painter)) 
				/ (gdk_pixbuf_animation_get_height (anim) * pixel_size);
			
			width *= scale;
		}

	}

	return width;
}

guint
html_image_get_actual_height (HTMLImage *image, HTMLPainter *painter)
{
	GdkPixbufAnimation *anim = image->image_ptr->animation;
	gint pixel_size = painter ? html_painter_get_pixel_size (painter) : 1;
	gint height;
		
	if (image->percent_height) {
		/* The cast to `gdouble' is to avoid overflow (eg. when
                   printing).  */
		height = ((gdouble) html_engine_get_view_height (image->image_ptr->factory->engine)
			  * image->specified_height) / 100;
	} else if (image->specified_height > 0) {
		height = image->specified_height * pixel_size;
	} else if (image->image_ptr == NULL || anim == NULL) {
		height = DEFAULT_SIZE * pixel_size;
	} else {
		height = gdk_pixbuf_animation_get_height (anim) * pixel_size;

		if (image->specified_width > 0 || image->percent_width) {
			double scale;
			
			scale = ((double) html_image_get_actual_width (image, painter))
				/ (gdk_pixbuf_animation_get_width (anim) * pixel_size);
			
			height *= scale;
		} 
	}

	return height;
}


/* HTMLObject methods.  */

/* FIXME: We should close the stream here, too.  But in practice we cannot
   because the stream pointer might be invalid at this point, and there is no
   way to set it to NULL when the stream is closed.  This clearly sucks and
   must be fixed.  */
static void
destroy (HTMLObject *o)
{
	HTMLImage *image = HTML_IMAGE (o);

	html_image_factory_unregister (image->image_ptr->factory,
				       image->image_ptr, HTML_IMAGE (image));

	g_free (image->url);
	g_free (image->target);
	g_free (image->alt);
	g_free (image->usemap);
	g_free (image->final_url);

	if (image->color)
		html_color_unref (image->color);

	HTML_OBJECT_CLASS (parent_class)->destroy (o);
}

static void
copy (HTMLObject *self,
      HTMLObject *dest)
{
	HTMLImage *dimg = HTML_IMAGE (dest);
	HTMLImage *simg = HTML_IMAGE (self);

	/* FIXME not sure this is all correct.  */

	(* HTML_OBJECT_CLASS (parent_class)->copy) (self, dest);

	dimg->image_ptr = simg->image_ptr;
	dimg->color = simg->color;
	if (simg->color)
		html_color_ref (dimg->color);

	dimg->have_color = simg->have_color;

	dimg->border = simg->border;

	dimg->specified_width = simg->specified_width;
	dimg->specified_height = simg->specified_height;
	dimg->percent_width = simg->percent_width;
	dimg->percent_height = simg->percent_height;
	dimg->ismap = simg->ismap;

	dimg->hspace = simg->hspace;
	dimg->vspace = simg->vspace;

	dimg->valign = simg->valign;

	dimg->url = g_strdup (simg->url);
	dimg->target = g_strdup (simg->target);
	dimg->alt = g_strdup (simg->alt);
	dimg->usemap = g_strdup (simg->usemap);
	dimg->final_url = NULL;

	/* add dest to image_ptr interests */
	dimg->image_ptr->interests = g_slist_prepend (dimg->image_ptr->interests, dimg);
	html_image_pointer_ref (dimg->image_ptr);
}

static void 
image_update_url (HTMLImage *image, gint x, gint y)
{
	HTMLMap *map;
	HTMLObject *o = HTML_OBJECT (image);
	char *url = NULL;

	/* 
	 * FIXME this is a huge hack waiting until we implement events for now we write
	 * over the image->url for every point since we always call point before get_url
	 * it is sick, I know.
	 */
	if (image->usemap != NULL) {
		map = html_engine_get_map (image->image_ptr->factory->engine, 
					   image->usemap + 1);
		
		if (map) {
			url = html_map_calc_point (map, x - o->x , y - (o->y - o->ascent));
			
			if (url)
				url = g_strdup (url);
		}
	} else if (image->ismap) {
		if (image->url)
			url = g_strdup_printf ("%s?%d,%d", image->url, x - o->x, y - (o->y - o->ascent));
	} else {
		return;
	}
	
	g_free (image->final_url);
	image->final_url = url;
}

static HTMLObject *
check_point (HTMLObject *self,
	     HTMLPainter *painter,
	     gint x, gint y,
	     guint *offset_return,
	     gboolean for_cursor)
{
	if ((x >= self->x)
	    && (x < (self->x + self->width))
	    && (y >= (self->y - self->ascent))
	    && (y < (self->y + self->descent))) {
		if (offset_return != NULL)
			*offset_return = x - self->x < self->width / 2 ? 0 : 1;
		
		image_update_url (HTML_IMAGE (self), x, y);
		return self;
	}
	
	return NULL;
}

static gint
calc_min_width (HTMLObject *o,
		HTMLPainter *painter)
{
	HTMLImage *image = HTML_IMAGE (o);
	guint pixel_size;
	guint min_width;

	pixel_size = html_painter_get_pixel_size (painter);

	if (image->percent_width || image->percent_height)
		min_width = pixel_size;
	else
		min_width = html_image_get_actual_width (HTML_IMAGE (o), painter);

	min_width += (image->border * 2 + 2 * image->hspace) * pixel_size;

	return min_width;
}

static gint
calc_preferred_width (HTMLObject *o,
		      HTMLPainter *painter)
{
	HTMLImage *image = HTML_IMAGE (o);
	guint width;

	width = html_image_get_actual_width (HTML_IMAGE (o), painter)
		+ (image->border * 2 + 2 * image->hspace) * html_painter_get_pixel_size (painter);

	return width;
}

static gboolean
calc_size (HTMLObject *o, HTMLPainter *painter, GList **changed_objs)
{
	HTMLImage *image;
	guint pixel_size;
	guint width, height;
	gint old_width, old_ascent, old_descent;

	old_width = o->width;
	old_ascent = o->ascent;
	old_descent = o->descent;

	image = HTML_IMAGE (o);

	pixel_size = html_painter_get_pixel_size (painter);

	if (o->parent && HTML_IS_CLUEFLOW (o->parent)
	    && HTML_IS_PLAIN_PAINTER (painter) && image->alt && *image->alt) {
		GtkHTMLFontStyle style;
		gint lo = 0;

		style = html_clueflow_get_default_font_style (HTML_CLUEFLOW (o->parent));
		/* FIXME: cache items and glyphs? */
		html_painter_calc_text_size (painter, image->alt, g_utf8_strlen (image->alt, -1), NULL, NULL, 0, &lo,
					     style, NULL, &o->width, &o->ascent, &o->descent);
	} else {
		width = html_image_get_actual_width (image, painter);
		height = html_image_get_actual_height (image, painter);

		o->width  = width + (image->border + image->hspace) * 2 * pixel_size;
		o->ascent = height + (image->border + image->vspace) * 2 * pixel_size;
		o->descent = 0;
	}

	if (o->descent != old_descent
	    || o->ascent != old_ascent
	    || o->width != old_width)
		return TRUE;

	return FALSE;
}

static void
draw_plain (HTMLObject *o, HTMLPainter *p, gint x, gint y, gint width, gint height, gint tx, gint ty)
{
	HTMLImage *img = HTML_IMAGE (o);

	if (img->alt && *img->alt) {

		/* FIXME: cache items and glyphs? */
		if (o->selected) {
			html_painter_set_pen (p, &html_colorset_get_color_allocated
					      (p, p->focus ? HTMLHighlightColor : HTMLHighlightNFColor)->color);
			html_painter_fill_rect (p, o->x + tx, o->y + ty - o->ascent, o->width, o->ascent + o->descent);
			html_painter_set_pen (p, &html_colorset_get_color_allocated
					      (p, p->focus ? HTMLHighlightTextColor : HTMLHighlightTextNFColor)->color);
		} else { 
			html_painter_set_pen (p, &html_colorset_get_color_allocated (p, HTMLTextColor)->color);
		}
		html_painter_draw_text (p, o->x + tx, o->y + ty, img->alt, g_utf8_strlen (img->alt, -1), NULL, NULL, 0, 0);
	}
}

static void
draw (HTMLObject *o,
      HTMLPainter *painter,
      gint x, gint y,
      gint width, gint height,
      gint tx, gint ty)
{
	HTMLImage *image;
	HTMLImagePointer *ip;
	GdkPixbuf *pixbuf;
	gint base_x, base_y;
	gint scale_width, scale_height;
	GdkColor *highlight_color;
	guint pixel_size;
	GdkRectangle paint;

	/* printf ("Image::draw\n"); */

	if (!html_object_intersect (o, &paint, x, y, width, height))
		return;

	if (HTML_IS_PLAIN_PAINTER (painter)) {
		draw_plain (o, painter, x, y, width, height, tx, ty);
		return;
	}

	image = HTML_IMAGE (o);
	ip = image->image_ptr;

	if (ip->animation) {
		if (HTML_IS_GDK_PAINTER (painter) && !gdk_pixbuf_animation_is_static_image (ip->animation)) {
			pixbuf = gdk_pixbuf_animation_iter_get_pixbuf (ip->iter);
		} else {
			pixbuf = gdk_pixbuf_animation_get_static_image (ip->animation);
		}
	} else {
		pixbuf = NULL;
	}

	pixel_size = html_painter_get_pixel_size (painter);

	if (o->selected) {
		highlight_color = &html_colorset_get_color_allocated
			(painter, painter->focus ? HTMLHighlightColor : HTMLHighlightNFColor)->color;
	} else
		highlight_color = NULL;

	base_x = o->x + tx + (image->border + image->hspace) * pixel_size;
	base_y = o->y + ty + (image->border + image->vspace) * pixel_size - o->ascent;

	if (pixbuf == NULL) {
		gint vspace, hspace;

		hspace = image->hspace * pixel_size;
		vspace = image->vspace * pixel_size;

		if (image->image_ptr->loader && !image->image_ptr->stall) 
			return;

		if (o->selected) {
			html_painter_set_pen (painter, highlight_color);
			html_painter_fill_rect (painter, 
						o->x + tx + hspace,
						o->y + ty - o->ascent + vspace,
						o->width - 2 * hspace,
						o->ascent + o->descent - 2 * vspace);
		}
		html_painter_draw_panel (painter,
					 &((html_colorset_get_color (painter->color_set, HTMLBgColor))->color),
					 o->x + tx + hspace,
					 o->y + ty - o->ascent + vspace,
					 o->width - 2 * hspace,
					 o->ascent + o->descent - 2 * vspace,
					 GTK_HTML_ETCH_IN, 1);

		if (ip->factory)
			pixbuf = html_image_factory_get_missing (ip->factory);

		if (pixbuf && 
		    (o->width > gdk_pixbuf_get_width (pixbuf)) &&
		    (o->ascent  + o->descent > gdk_pixbuf_get_height (pixbuf)))
			html_painter_draw_pixmap (painter, pixbuf,
						  base_x, base_y,
						  gdk_pixbuf_get_width (pixbuf) * pixel_size,
						  gdk_pixbuf_get_height (pixbuf) * pixel_size,
						  highlight_color);
			
			
		return;
	}

	scale_width = html_image_get_actual_width (image, painter);
	scale_height = html_image_get_actual_height (image, painter);

	if (image->border) {
		if (image->have_color) {
			html_color_alloc (image->color, painter);
			html_painter_set_pen (painter, &image->color->color);
		}
		
		html_painter_draw_panel (painter,
					 &((html_colorset_get_color (painter->color_set, HTMLBgColor))->color),
					 base_x - image->border * pixel_size,
					 base_y - image->border * pixel_size,
					 scale_width + (2 * image->border) * pixel_size,
					 scale_height + (2 * image->border) * pixel_size,
					 GTK_HTML_ETCH_NONE, image->border);
		
	}
	
	image->animation_active = TRUE;
	html_painter_draw_pixmap (painter, pixbuf,
				  base_x, base_y,
				  scale_width, scale_height,
				  highlight_color);
}

gchar *
html_image_resolve_image_url (GtkHTML *html, gchar *image_url)
{
	gchar *url = NULL;

	/* printf ("html_image_resolve_image_url %p\n", html->editor_api); */
	if (html->editor_api) {
		GValue  *iarg = g_new0 (GValue, 1);
		GValue  *oarg;

		g_value_init (iarg, G_TYPE_STRING);
		g_value_set_string (iarg, image_url);

		oarg = (* html->editor_api->event) (html, GTK_HTML_EDITOR_EVENT_IMAGE_URL, iarg, html->editor_data);

		if (oarg) {
			if (G_VALUE_TYPE (oarg) == G_TYPE_STRING)
				url = (gchar *) g_strdup (g_value_get_string (oarg));
			g_value_unset (oarg);	
			g_free (oarg);
		}
		g_value_unset (iarg);
		g_free (iarg);
	}
	if (!url)
		url = g_strdup (image_url);
	/* printf ("image URL resolved to: %s (from: %s)\n", url, image_url); */

	return url;
}

static gboolean
save (HTMLObject *self,
      HTMLEngineSaveState *state)
{
	HTMLImage *image;
	gchar *url;
	gboolean result, link = FALSE;

	g_return_val_if_fail (self != NULL, FALSE);
	g_return_val_if_fail (state != NULL, FALSE);

	image  = HTML_IMAGE (self);

	if (image->url && *image->url) {
		url  = g_strconcat (image->url, image->target ? "#" : "", image->target, NULL);
		link = TRUE;
		result = html_engine_save_output_string (state, "<A HREF=\"%s\">", url);
		g_free (url);
		if (!result)
			return FALSE;	
	}

	url    = html_image_resolve_image_url (state->engine->widget, image->image_ptr->url);
	result = html_engine_save_output_string (state, "<IMG SRC=\"%s\"", url);
	g_free (url);
	if (!result)
		return FALSE;	

	if (image->percent_width) {
		if (!html_engine_save_output_string (state, " WIDTH=\"%d\%\"", image->specified_width))
			return FALSE;
	} else if (image->specified_width > 0) {
		if (!html_engine_save_output_string (state, " WIDTH=\"%d\"", image->specified_width))
			return FALSE;
	}

	if (image->percent_height) {
		if (!html_engine_save_output_string (state, " HEIGHT=\"%d\%\"", image->specified_height))
			return FALSE;
	} else if (image->specified_height > 0) {
		if (!html_engine_save_output_string (state, " HEIGHT=\"%d\"", image->specified_height))
			return FALSE;
	}

	if (image->vspace) {
		if (!html_engine_save_output_string (state, " VSPACE=\"%d\"", image->vspace))
			return FALSE;
	}

	if (image->hspace) {
		if (!html_engine_save_output_string (state, " HSPACE=\"%d\"", image->hspace))
			return FALSE;
	}

	if (image->vspace) {
		if (!html_engine_save_output_string (state, " VSPACE=\"%d\"", image->vspace))
			return FALSE;
	}

	if (image->valign != HTML_VALIGN_NONE) {
		if (!html_engine_save_output_string (state, " ALIGN=\"%s\"", html_valign_name (image->valign)))
			return FALSE;
	}

	if (image->alt) {
		if (!html_engine_save_output_string (state, " ALT=\"%s\"", image->alt))
			return FALSE;
	}

	/* FIXME this is the default set in htmlengine.c but there is no real way to tell
	 * if the usr specified it directly
	 */
	if (image->border != 2) {
		if (!html_engine_save_output_string (state, " BORDER=\"%d\"", image->border))
			return FALSE;
	}

	if (!html_engine_save_output_string (state, ">"))
		return FALSE;
	if (link && !html_engine_save_output_string (state, "</A>"))
		return FALSE;
	
	return TRUE;
}

static gboolean
save_plain (HTMLObject *self,
	    HTMLEngineSaveState *state,
	    gint requested_width)
{
	HTMLImage *image;
	gboolean rv = TRUE;

	image = HTML_IMAGE (self);
	
	if (image->alt)
		rv = html_engine_save_output_string (state, "%s", image->alt);

	return rv;
}

static const gchar *
get_url (HTMLObject *o)
{
	HTMLImage *image;

	image = HTML_IMAGE (o);
	return image->final_url ? image->final_url : image->url;
}

static const gchar *
get_target (HTMLObject *o)
{
	HTMLImage *image;

	image = HTML_IMAGE (o);
	return image->target;
}

static const gchar *
get_src (HTMLObject *o)
{
	HTMLImage *image;
	
	image = HTML_IMAGE (o);
	return image->image_ptr->url;
}

static HTMLObject *
set_link (HTMLObject *self, HTMLColor *color, const gchar *url, const gchar *target)
{
	HTMLImage *image = HTML_IMAGE (self);

	STRDUP_HELPER (image->url, url);
	STRDUP_HELPER (image->target, target);
	if (image->have_color)
		html_color_unref (image->color);
	image->color = color;
	if (color) {
		html_color_ref (color);
		image->have_color = TRUE;
	} else {
		image->have_color = FALSE;
	}

	return NULL;
}

static gboolean
accepts_cursor (HTMLObject *o)
{
	return TRUE;
}

static HTMLVAlignType
get_valign (HTMLObject *self)
{
	HTMLImage *image;

	image = HTML_IMAGE (self);

	return image->valign;
}

static gboolean
select_range (HTMLObject *self,
	      HTMLEngine *engine,
	      guint offset,
	      gint length,
	      gboolean queue_draw)
{
	/* printf ("IMAGE: select range\n"); */
	if ((*parent_class->select_range) (self, engine, offset, length, queue_draw)) {
		if (queue_draw) {
			html_engine_queue_draw (engine, self);
			/* printf ("IMAGE: draw queued\n"); */
		}
		return TRUE;
	} else
		return FALSE;
}


void
html_image_type_init (void)
{
	html_image_class_init (&html_image_class, HTML_TYPE_IMAGE, sizeof (HTMLImage));
}

void
html_image_class_init (HTMLImageClass *image_class,
		       HTMLType type,
		       guint size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (image_class);

	html_object_class_init (object_class, type, size);

	object_class->copy = copy;
	object_class->draw = draw;	
	object_class->destroy = destroy;
	object_class->calc_min_width = calc_min_width;
	object_class->calc_preferred_width = calc_preferred_width;
	object_class->calc_size = calc_size;
	object_class->check_point = check_point;
	object_class->get_url = get_url;
	object_class->get_target = get_target;
	object_class->get_src = get_src;
	object_class->set_link = set_link;
	object_class->accepts_cursor = accepts_cursor;
	object_class->get_valign = get_valign;
	object_class->save = save;
	object_class->save_plain = save_plain;
	object_class->select_range = select_range;

	parent_class = &html_object_class;
}

void
html_image_init (HTMLImage *image,
		 HTMLImageClass *klass,
		 HTMLImageFactory *imf,
		 const gchar *filename,
		 const gchar *url,
		 const gchar *target,
		 gint16 width, gint16 height,
		 gboolean percent_width, gboolean percent_height,
		 gint8 border,
		 HTMLColor *color,
		 HTMLVAlignType valign,
		 gboolean reload)
{
	HTMLObject *object;

	g_assert (filename);

	object = HTML_OBJECT (image);

	html_object_init (object, HTML_OBJECT_CLASS (klass));

	image->url = g_strdup (url);
	image->target = g_strdup (target);
	image->usemap = NULL;
	image->final_url = NULL;
	image->ismap = FALSE;

	image->specified_width  = width;
	image->specified_height = height;
	image->percent_width    = percent_width;
	image->percent_height   = percent_height;
	image->border           = border;

	if (color) {
		image->color = color;
		image->have_color = TRUE;
		html_color_ref (color);
	} else {
		image->color = NULL;
		image->have_color = FALSE;
	}

	image->alt = NULL;

	image->hspace = 0;
	image->vspace = 0;

	if (valign == HTML_VALIGN_NONE)
		valign = HTML_VALIGN_BOTTOM;
	image->valign = valign;

	image->image_ptr = html_image_factory_register (imf, image, filename, reload);
}

HTMLObject *
html_image_new (HTMLImageFactory *imf,
		const gchar *filename,
		const gchar *url,
		const gchar *target,
		gint16 width, gint16 height,
		gboolean percent_width, gboolean percent_height,
		gint8 border,	
		HTMLColor *color,
		HTMLVAlignType valign,
		gboolean reload)
{
	HTMLImage *image;

	image = g_new(HTMLImage, 1);

	html_image_init (image, &html_image_class,
			 imf,
			 filename,
			 url,
			 target,
			 width, height,
			 percent_width, percent_height,
			 border,
			 color,
			 valign,
			 reload);

	return HTML_OBJECT (image);
}

void
html_image_set_spacing (HTMLImage *image, gint hspace, gint vspace)
{
	gboolean changed = FALSE;

	if (image->hspace != hspace) {
		image->hspace = hspace;
		changed = TRUE;
	}

	if (image->vspace != vspace) {
		image->vspace = vspace;
		changed = TRUE;
	}

	if (changed) {
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_edit_set_url (HTMLImage *image, const gchar *url)
{
	if (url) {
		HTMLImageFactory *imf = image->image_ptr->factory;

		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_image_factory_unregister (imf, image->image_ptr, HTML_IMAGE (image));
		image->image_ptr = html_image_factory_register (imf, image, url, TRUE);
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (imf->engine);
	}
}

void
html_image_set_url (HTMLImage *image, const gchar *url)
{
	if (url && strcmp (image->image_ptr->url, url)) {
		HTMLImageFactory *imf = image->image_ptr->factory;

		html_image_factory_unregister (imf, image->image_ptr, HTML_IMAGE (image));
		image->image_ptr = html_image_factory_register (imf, image, url, FALSE);
	}
}

void
html_image_set_valign (HTMLImage *image, HTMLVAlignType valign)
{
	if (image->valign != valign) {
		image->valign = valign;
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_set_border (HTMLImage *image, gint border)
{
	if (image->border != border) {
		image->border = border;
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

void
html_image_set_alt (HTMLImage *image, gchar *alt)
{
	g_free (image->alt);
	image->alt = g_strdup (alt);
}

void
html_image_set_map (HTMLImage *image, gchar *usemap, gboolean ismap)
{
	char *url = NULL;

	g_free (image->usemap);

	if (usemap != NULL) {
		image->ismap = FALSE;
		url = g_strdup (usemap);
	} else {
		image->ismap = ismap;
	}
	image->usemap = url;
}

void
html_image_set_size (HTMLImage *image, gint w, gint h, gboolean pw, gboolean ph)
{
	gboolean changed = FALSE;

	if (pw != image->percent_width) {
		image->percent_width = pw;
		changed = TRUE;
	}

	if (ph != image->percent_height) {
		image->percent_height = ph;
		changed = TRUE;
	}

	if (w != image->specified_width) {
		image->specified_width = w;
		changed = TRUE;
	}

	if (h != image->specified_height) {
		image->specified_height = h;
		changed = TRUE;
	}

	if (changed) {
		html_object_change_set (HTML_OBJECT (image), HTML_CHANGE_ALL_CALC);
		html_engine_schedule_update (image->image_ptr->factory->engine);
	}
}

static char *fallback_image_content_types[] = {"image/*", NULL};

static char **
html_image_factory_types (GtkHTMLStream *stream,
			  gpointer user_data)
{
	static char**image_content_types = NULL;
	
#if 0
	/* this code should work in gtk+-2.2 but it is untested */
	if (image_content_types == NULL) {
		GSList *formats;
		GSList *cur;
		GSList *types = NULL;
		gint i;

		formats = gdk_pixbuf_get_formats ();
		
		for (cur = formats; cur; cur = cur->next) {
			GdkPixbufFormat *format = cur->data;
			char **mime;

			mime = gdk_pixbuf_formats_get_mime_types ();
			for (i = 0; mime && mime[i]; i++)
				g_slist_prepend (types, g_strdup (mime[i]));

		}
		g_slist_free (formats);

		if (types) {
			image_content_types = g_new0 (char *, g_slist_length (types) + 1);
			
			for (cur = types, i = 0; cur; cur = cur->next, i++) {
				image_content_types[i] = cur->data;
			}
			g_slist_free (types);
		} else {
			image_content_types = fallback_image_content_types;
		}
	}
#else 
	image_content_types = fallback_image_content_types;
#endif

	return image_content_types;
}

static void
update_or_redraw (HTMLImagePointer *ip)
{
	GSList *list;
	gboolean update = FALSE;

	for (list = ip->interests; list; list = list->next) {
		if (list->data == NULL)
			update = TRUE;
		else {
			HTMLImage *image = HTML_IMAGE (list->data);
			gint pixel_size = html_painter_get_pixel_size (ip->factory->engine->painter);
			gint w, h;

			w = html_image_get_actual_width (image, ip->factory->engine->painter)
				+ (image->border * 2 + 2 * image->hspace) * pixel_size;
			h = html_image_get_actual_height (image, ip->factory->engine->painter)
				+ (image->border * 2 + 2 * image->vspace) * pixel_size;

			/* printf ("%dx%d  <-->  %dx%d\n", w, h, HTML_OBJECT (list->data)->width,
			   HTML_OBJECT (list->data)->ascent + HTML_OBJECT (list->data)->descent); */

			if (w != HTML_OBJECT (list->data)->width
			    || h != HTML_OBJECT (list->data)->ascent + HTML_OBJECT (list->data)->descent) {
				html_object_change_set (HTML_OBJECT (list->data), HTML_CHANGE_ALL_CALC);
				update = TRUE;
			}
		}
	}

	if (ip->factory->engine->block && ip->factory->engine->opened_streams)
		return;

	if (!update) {
		/* printf ("REDRAW\n"); */
		for (list = ip->interests; list; list = list->next)
			if (list->data) /* && html_object_is_visible (HTML_OBJECT (list->data))) */
				html_engine_queue_draw (ip->factory->engine, HTML_OBJECT (list->data));
		if (ip->interests)
			html_engine_flush_draw_queue (ip->factory->engine);
	} else {
		/* printf ("UPDATE\n"); */
		html_engine_schedule_update (ip->factory->engine);
	}
}

static void
html_image_factory_end_pixbuf (GtkHTMLStream *stream,
			       GtkHTMLStreamStatus status,
			       gpointer user_data)
{
	HTMLImagePointer *ip = user_data;

	gdk_pixbuf_loader_close (ip->loader, NULL);

	if (!ip->animation) {
		ip->animation = gdk_pixbuf_loader_get_animation (ip->loader);

		if (ip->animation)
			g_object_ref (ip->animation);
	}
	html_image_pointer_start_animation (ip);

	g_object_unref (ip->loader);
	ip->loader = NULL;

	update_or_redraw (ip);
	if (ip->factory->engine->opened_streams)
		ip->factory->engine->opened_streams --;
	/* printf ("IMAGE(%p) opened streams: %d\n", ip->factory->engine, ip->factory->engine->opened_streams); */
	if (ip->factory->engine->opened_streams == 0 && ip->factory->engine->block)
		html_engine_schedule_update (ip->factory->engine);
	html_image_pointer_unref (ip);
}

static void
html_image_factory_write_pixbuf (GtkHTMLStream *stream,
				 const gchar *buffer,
				 size_t size,
				 gpointer user_data)
{
	HTMLImagePointer *p = user_data;

	/* FIXME ! Check return value */
	gdk_pixbuf_loader_write (p->loader, buffer, size, NULL);
}

static void
html_image_factory_area_updated (GdkPixbufLoader *loader, guint x, guint y, guint width, guint height)
{

}

static void
html_image_factory_area_prepared (GdkPixbufLoader *loader, HTMLImagePointer *ip)
{
	if (!ip->animation) {
		ip->animation = gdk_pixbuf_loader_get_animation (loader);
		g_object_ref (ip->animation);
		
		html_image_pointer_start_animation (ip);
	}
	update_or_redraw (ip);
}

static void
html_image_pointer_queue_animation (HTMLImagePointer *ip)
{
	gint delay = gdk_pixbuf_animation_iter_get_delay_time (ip->iter);

	if (delay >= 0 && !ip->animation_timeout && ip->factory && ip->factory->animate) {
		ip->animation_timeout = g_timeout_add (delay, 
						       (GtkFunction) html_image_pointer_run_animation, 
						       (gpointer) ip);
	}	
}

static gint
html_image_pointer_run_animation (HTMLImagePointer *ip)
{
	GdkPixbufAnimationIter  *iter = ip->iter;
	HTMLEngine              *engine = ip->factory->engine;
	
	g_return_val_if_fail (ip->factory != NULL, FALSE);
	ip->animation_timeout = 0;

	/* printf ("animation_timeout\n"); */
	if (gdk_pixbuf_animation_iter_advance (iter, NULL)) {
		GSList *cur;

		for (cur = ip->interests; cur; cur = cur->next) {
			HTMLImage           *image = cur->data;

			if (image && image->animation_active) {
				image->animation_active = FALSE;
				html_engine_queue_draw (engine, HTML_OBJECT (image));
			}
		}
	}
		
	html_image_pointer_queue_animation (ip);
	return FALSE;
}

static void
html_image_pointer_start_animation (HTMLImagePointer *ip)
{
	if (ip->animation && !gdk_pixbuf_animation_is_static_image (ip->animation)) {
		if (!ip->iter)
			ip->iter = gdk_pixbuf_animation_get_iter (ip->animation, NULL);

		html_image_pointer_queue_animation (ip);
	}
}

static void
html_image_pointer_stop_animation (HTMLImagePointer *ip)
{
	if (ip->animation_timeout) {
		g_source_remove (ip->animation_timeout);
		ip->animation_timeout = 0;
	}
}

static GdkPixbuf *
html_image_factory_get_missing (HTMLImageFactory *factory)
{
	if (!factory->missing)
		factory->missing = gtk_widget_render_icon (GTK_WIDGET (factory->engine->widget),
							  GTK_STOCK_MISSING_IMAGE,
							  GTK_ICON_SIZE_BUTTON, "GtkHTML.ImageMissing");
	return factory->missing;
}

HTMLImageFactory *
html_image_factory_new (HTMLEngine *e)
{
	HTMLImageFactory *retval;
	retval = g_new (HTMLImageFactory, 1);
	retval->engine = e;
	retval->loaded_images = g_hash_table_new (g_str_hash, g_str_equal);
	retval->missing = NULL;
	retval->animate = TRUE;

	return retval;
}

static gboolean
cleanup_images (gpointer key, gpointer value, gpointer free_everything)
{
	HTMLImagePointer *ip = value;

	/* free_everything means: NULL only clean, non-NULL free */
	if (free_everything){
		if (ip->interests != NULL) {
			g_slist_free (ip->interests);
			ip->interests = NULL;
		}
	}

	/* clean only if this image is not used anymore */
	if (!ip->interests){
		html_image_pointer_unref (ip);
		ip->factory = NULL;
		return TRUE;
	}

	return FALSE;
}

void
html_image_factory_cleanup (HTMLImageFactory *factory)
{
	g_return_if_fail (factory);
	g_hash_table_foreach_remove (factory->loaded_images, cleanup_images, NULL);
}

void
html_image_factory_free (HTMLImageFactory *factory)
{
	g_return_if_fail (factory);

	g_hash_table_foreach_remove (factory->loaded_images, cleanup_images, factory);
	g_hash_table_destroy (factory->loaded_images);

	if (factory->missing)
		gdk_pixbuf_unref (factory->missing);

	g_free (factory);
}

#define STALL_INTERVAL 1000

static HTMLImagePointer *
html_image_pointer_new (const char *filename, HTMLImageFactory *factory)
{
	HTMLImagePointer *retval;

	retval = g_new (HTMLImagePointer, 1);
	retval->refcount = 1;
	retval->url = g_strdup (filename);
	retval->loader = gdk_pixbuf_loader_new ();
	retval->iter = NULL;
	retval->animation = NULL;
	retval->interests = NULL;
	retval->factory = factory;
	retval->stall = FALSE;
	retval->stall_timeout = gtk_timeout_add (STALL_INTERVAL, 
						 (GtkFunction)html_image_pointer_timeout,
						 retval);
	retval->animation_timeout = 0;
	return retval;
}

static gboolean
html_image_pointer_timeout (HTMLImagePointer *ip)
{
	GSList *list;
	HTMLImage *image;

	ip->stall_timeout = 0;

	g_return_val_if_fail (ip->factory != NULL, FALSE);

	ip->stall = TRUE;

	list = ip->interests;
	/* 
	 * draw the frame now that we decided they've had enough time to
	 * load the image
	 */
	if (ip->animation == NULL) {
		while (list) {
			image = (HTMLImage *)list->data;

			if (image)
				html_engine_queue_draw (ip->factory->engine,
							HTML_OBJECT (image));
			
			list = list->next;
		}
	}
	return FALSE;
}

static void
html_image_pointer_ref (HTMLImagePointer *ip)
{
	ip->refcount++;
}

static void
free_image_ptr_data (HTMLImagePointer *ip)
{
	if (ip->loader) {
		gdk_pixbuf_loader_close (ip->loader, NULL);
		g_object_unref (ip->loader);
		ip->loader = NULL;
	}
	if (ip->animation) {
		g_object_unref (ip->animation);
		ip->animation = NULL;
	}
	if (ip->iter) {
		g_object_unref (ip->iter);
		ip->iter = NULL;
	}
}

static void
html_image_pointer_remove_stall (HTMLImagePointer *ip)
{
	if (ip->stall_timeout) {
		gtk_timeout_remove (ip->stall_timeout);
		ip->stall_timeout = 0;
	}
}

static void
html_image_pointer_unref (HTMLImagePointer *ip)
{
	g_return_if_fail (ip != NULL);

	ip->refcount--;
	/* printf ("unref(%p) %s --> %d\n", ip, ip->url, ip->refcount); */
	if (ip->refcount < 1) {
		/* printf ("freeing %s\n", ip->url); */
		html_image_pointer_remove_stall (ip);
		html_image_pointer_stop_animation (ip);
		g_free (ip->url);
		free_image_ptr_data (ip);
		g_free (ip);
	}
}

GtkHTMLStream *
html_image_pointer_load (HTMLImagePointer *ip)
{
	html_image_pointer_ref (ip);

	ip->factory->engine->opened_streams ++;
	return gtk_html_stream_new (GTK_HTML (ip->factory->engine->widget),
				    html_image_factory_types,
				    html_image_factory_write_pixbuf,
				    html_image_factory_end_pixbuf,
				    ip);
}

HTMLImagePointer *
html_image_factory_register (HTMLImageFactory *factory, HTMLImage *i, const char *url, gboolean reload)
{
	HTMLImagePointer *ip;
	GtkHTMLStream *stream = NULL;

	g_return_val_if_fail (factory, NULL);
	g_return_val_if_fail (url, NULL);

	ip = g_hash_table_lookup (factory->loaded_images, url);

	if (!ip) {
		ip = html_image_pointer_new (url, factory);
		g_hash_table_insert (factory->loaded_images, ip->url, ip);
		if (*url) {
			g_signal_connect (G_OBJECT (ip->loader), "area_prepared",
					  G_CALLBACK (html_image_factory_area_prepared),
					  ip);

			g_signal_connect (G_OBJECT (ip->loader), "area_updated",
					  G_CALLBACK (html_image_factory_area_updated),
					  ip);
			stream = html_image_pointer_load (ip);
		}
	} else {
		if (reload) {
			free_image_ptr_data (ip);
			ip->loader = gdk_pixbuf_loader_new ();
			stream = html_image_pointer_load (ip);
		}
	}

	if (stream)
		g_signal_emit_by_name (factory->engine, "url_requested", ip->url, stream);

	html_image_pointer_ref (ip);

	/* we add also NULL ptrs, as we dont want these to be cleaned out */
	ip->interests = g_slist_prepend (ip->interests, i);

	if (i) {
		i->image_ptr = ip;
	}

	return ip;
}

#if 0
HTMLEngine *
html_image_factory_get_engine (HTMLImageFactory *factory)
{
	return factory->engine;
}
#endif

void
html_image_factory_unregister (HTMLImageFactory *factory, HTMLImagePointer *pointer, HTMLImage *i)
{
	pointer->interests = g_slist_remove (pointer->interests, i);

	html_image_pointer_unref (pointer);

	if (pointer->refcount == 1) {
		g_assert (pointer->interests == NULL);
		/*
		 * FIXME The factory can be NULL if the image was from an iframe 
		 * that has been destroyed and the image is living in the cut buffer
		 * this isn't particularly clean and should be refactored.
		 *
		 * We really need a way to let cut objects know they are living outside
		 * the normal flow.
		 */
		if (factory) 
			g_hash_table_remove (factory->loaded_images, pointer->url);
		pointer->factory = NULL;
		html_image_pointer_unref (pointer);
	}
}

static void
stop_anim (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ip = value;
	html_image_pointer_remove_stall (ip);
	html_image_pointer_stop_animation (ip);
}

void
html_image_factory_stop_animations (HTMLImageFactory *factory)
{
	DA (g_warning ("stop animations");)
	g_hash_table_foreach (factory->loaded_images, stop_anim, NULL);
}

static void
start_anim (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ip = value;
	html_image_pointer_start_animation (ip);
}

void
html_image_factory_start_animations (HTMLImageFactory *factory)
{
	DA (g_warning ("start animations");)
	g_hash_table_foreach (factory->loaded_images, start_anim, NULL);
}

gboolean
html_image_factory_get_animate (HTMLImageFactory *factory)
{
	return factory->animate;
}

void
html_image_factory_set_animate (HTMLImageFactory *factory, gboolean animate)
{
	if (animate != factory->animate) {
		factory->animate = animate;

		if (animate)
			html_image_factory_start_animations (factory);
		else 
			html_image_factory_stop_animations (factory);
	}
}

static gboolean
move_image_pointers (gpointer key, gpointer value, gpointer data)
{
	HTMLImageFactory *dst = HTML_IMAGE_FACTORY (data);
	HTMLImagePointer *ip  = HTML_IMAGE_POINTER (value);

	ip->factory = dst;
	
	g_hash_table_insert (dst->loaded_images, ip->url, ip);
	g_signal_emit_by_name (ip->factory->engine, "url_requested", ip->url, html_image_pointer_load (ip));

	return TRUE;
}

void
html_image_factory_move_images (HTMLImageFactory *dst, HTMLImageFactory *src)
{
	g_hash_table_foreach_remove (src->loaded_images, move_image_pointers, dst);
}

static void
deactivate_anim (gpointer key, gpointer value, gpointer user_data)
{
	HTMLImagePointer *ip = value;
	GSList *cur = ip->interests;
	HTMLImage *image;
	

	while (cur) {
		if (cur->data) {
			image = (HTMLImage *) cur->data;
			image->animation_active = FALSE;
		}
		cur = cur->next;
	}
}

void
html_image_factory_deactivate_animations (HTMLImageFactory *factory)
{
	g_hash_table_foreach (factory->loaded_images, deactivate_anim, NULL);
}

static void
ref_image_ptr (gpointer key, gpointer val, gpointer data)
{
	html_image_pointer_ref (HTML_IMAGE_POINTER (val));
	/* printf ("ref(%p) %s --> %d\n", val, HTML_IMAGE_POINTER (val)->url, HTML_IMAGE_POINTER (val)->refcount); */
}

static void
unref_image_ptr (gpointer key, gpointer val, gpointer data)
{
	html_image_pointer_unref (HTML_IMAGE_POINTER (val));
}

void
html_image_factory_ref_all_images (HTMLImageFactory *factory)
{
	if (!factory->loaded_images)
		return;

	g_hash_table_foreach (factory->loaded_images, ref_image_ptr, NULL);
}

void
html_image_factory_unref_all_images (HTMLImageFactory *factory)
{
	if (!factory->loaded_images)
		return;

	g_hash_table_foreach (factory->loaded_images, unref_image_ptr, NULL);
}

void
html_image_factory_ref_image_ptr (HTMLImageFactory *factory, const gchar *url)
{
	HTMLImagePointer *ptr;

	if (!factory->loaded_images)
		return;

	ptr = HTML_IMAGE_POINTER (g_hash_table_lookup (factory->loaded_images, url));
	if (ptr)
		html_image_pointer_ref (ptr);
}

void
html_image_factory_unref_image_ptr (HTMLImageFactory *factory, const gchar *url)
{
	HTMLImagePointer *ptr;

	if (!factory->loaded_images)
		return;

	ptr = HTML_IMAGE_POINTER (g_hash_table_lookup (factory->loaded_images, url));
	if (ptr)
		html_image_pointer_unref (ptr);
}
